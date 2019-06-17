// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <locale>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "./SemsearchIndexBuilder.h"
#include "../codebase/semantic-wikipedia-utils/Timer.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Facets.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::flush;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
{"ontology-vocab", required_argument, NULL, 'v'},
{"ontology-blocks", required_argument, NULL, 'b'},
{"ontology-input", required_argument, NULL, 'o'},
{"class-instances-counts", required_argument, NULL, 'c'},
{NULL, 0, NULL, 0}
};

void printUsage()
{
  cout << " ---- Usage: ----" << endl
      << "IndexBuilderMain <OPTIONS> <FULLTEXT INPUT>"
      << endl << endl;
  cout << "<OPTIONS>:" << endl
      << "\t--ontology-vocab <ONTOLOGY VOCABLARY> (mandatory)" << endl
      << "\t--ontology-blocks <ONTOLOGY BLOCK BOUNDARIES> (mandatory)" << endl
      << "\t--ontology-input <ONTOLOGY ASCII FILE> (optional)" << endl
      << "\t--class-instances-counts <FILE NAME> (optional)" << endl
      << endl;
  cout << "<FULLTEXT INPUT>:" << endl
      << "\tList of base names. For each base there have to be three files:"
        << "\t\t<base>.words-by-contexts.ascii" << endl
        << "\t\t<base>.vocabulary" << endl
        << "\t\t<base>.block-boundaries" << endl
        << endl << endl;
}

// Write a .stxxl config-file.
// All we want is sufficient space somewhere with enough space.
// We can use the location of input files and use a constant size for now.
// The required size can only ben estimation anyway, since index size
// depends on the structure of words files rather than their size only,
// because of the "multiplications" performed.
void writeStxxlConfigFile(string location)
{
  ad_utility::File stxxlConfig(".stxxl", "w");
  std::ostringstream config;
  config << "disk=" << location << "stxxl.disk,"
      << ad_semsearch::STXXL_DISK_SIZE_INDEX_BUILDER << ",syscall";
  stxxlConfig.writeLine(config.str());
}

// Main function.
int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "Index-Builder, version " << __DATE__ << " "
      << __TIME__ << EMPH_OFF << endl << endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string ontologyVocabulary = "";
  string ontologyBlockBoundaries = "";
  string ontologyASCII = "";
  string classInstancesCountsFileName = "";
  vector<string> relationsToBySplitIntoBlocksByLhs;

  // TODO(buchholb): Replace this hardcoded information by something
  // configurable and more flixible.
  relationsToBySplitIntoBlocksByLhs.push_back(
      string(ad_semsearch::RELATION_PREFIX) + ad_semsearch::IS_A_RELATION
          + ad_semsearch::REVERSED_RELATION_SUFFIX);

  // Imbue log with a locale that uses proper number formatting
  std::locale loc;
  ad_utility::ReadableNumberFacet facet;
  std::locale locWithNumberGrouping(loc, &facet);
  ad_utility::Log::imbue(locWithNumberGrouping);


  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "v:b:o:c:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'v':
      ontologyVocabulary = optarg;
      break;
    case 'b':
      ontologyBlockBoundaries = optarg;
      break;
    case 'o':
      ontologyASCII = optarg;
      break;
    case 'c':
      classInstancesCountsFileName = optarg;
      break;
    default:
      cout << endl << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      printUsage();
      exit(1);
    }
  }
  vector<string> fulltextBases;
  while (optind < argc)
  {
    fulltextBases.push_back(argv[optind++]);
  }

  ad_utility::Timer timer;
  // Verify input
  if (ontologyVocabulary.size() == 0 || ontologyVocabulary.size() == 0)
  {
    printUsage();
    exit(1);
  }

  //! Write a .stxxl config file.
  //! Use a disk at the location of the input files.
  //! If there are no input files, there is no need for a config file, anyways.
  string input;
  if (fulltextBases.size() > 0)
  {
    input = fulltextBases[0];
  }
  else
  {
    if (ontologyASCII.size() > 0)
    {
      input = ontologyASCII;
    }
  }
  if (input.size() > 0)
  {
    string location = input.substr(0, input.rfind('/') + 1);
    writeStxxlConfigFile(location);
  }

  // Start the Program
  // ad_semsearch::BasicIndexBuilder ib;
  ad_semsearch::SemsearchIndexBuilder ib;
  cout << "Reading ontology vocabulary ..." << flush;
  ib.setOntologyVocabulary(ontologyVocabulary);
  cout << " done." << endl << endl << flush;

  if (ontologyASCII.size() > 0)
  {
    string outputName = ontologyASCII.substr(0, ontologyASCII.rfind('.'));
    outputName += ".index";
    cout << "Building ontology index \"" << outputName << "\"..." << endl
        << endl << flush;
    timer.start();
    ib.buildOntologyIndex(ontologyASCII, relationsToBySplitIntoBlocksByLhs,
        classInstancesCountsFileName, outputName);
    timer.stop();
    cout << "Building ontology index done." << endl << "The whole thing took "
        << timer.secs() << " seconds" << endl << endl;
  }

  for (size_t i = 0; i < fulltextBases.size(); ++i)
  {
    string asciiFulltext = fulltextBases[i] + ".words-by-contexts.ascii";
    string vocab = fulltextBases[i] + ".vocabulary";
    string blocks = fulltextBases[i] + ".block-boundaries";
    string output = fulltextBases[i] + ".index";
    cout << "Building fulltext index \"" << output << "\"..." << endl << endl
        << flush;
    timer.start();
    ib.buildFulltextIndex(asciiFulltext, vocab, blocks,
        ontologyBlockBoundaries, output);
    timer.stop();
    cout << "Building fulltext index \"" << output << "\" done. Took "
        << timer.secs() << " seconds" << endl << endl;
  }

  cout << "Everything done." << endl << "Built "
      << (ontologyASCII.size() > 0 ? "an " : "no ") << "ontology index and"
      << endl << fulltextBases.size() << " fulltext " << (fulltextBases.size()
      == 1 ? "index" : "indexes.") << endl << endl;
  return 0;
}
