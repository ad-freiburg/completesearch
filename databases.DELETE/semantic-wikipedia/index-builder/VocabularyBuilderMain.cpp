// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include "./VocabularyBuilder.h"

#include "../codebase/semantic-wikipedia-utils/Facets.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
  {"ontology-file", required_argument, NULL, 'o'},
  {"ontology-output", required_argument, NULL, 'O'},
  {NULL, 0, NULL, 0}
};

// Gets a default output filename for a giving input file.
string getOutputName(const string& inputName)
{
  // Find the first dot
  size_t posOfFirstDot = inputName.find('.');
  // Take the portion before the dot
  string baseName(posOfFirstDot == string::npos ?
      inputName : inputName.substr(0, posOfFirstDot));
  // Append ".vocabulary" and return
  return baseName + ".vocabulary";
}

// Main function.
int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "VocabularyBuilder version " << __DATE__ << " "
      << __TIME__ << EMPH_OFF << endl << endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string ontologyFile = "";
  string ontologyOutput = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "o:O:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'o':
      ontologyFile = optarg;
      break;
    case 'O':
      ontologyOutput = optarg;
      break;
    default:
      cout << endl << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  // Words files
  vector<string> wordsFiles;
  while (optind < argc)
  {
    wordsFiles.push_back(argv[optind]);
    ++optind;
  }

  // Imbue log with a locale that uses proper number formatting
  std::locale loc;
  ad_utility::ReadableNumberFacet facet;
  std::locale locWithNumberGrouping(loc, &facet);
  ad_utility::Log::imbue(locWithNumberGrouping);

  // Start the Program
  ad_semsearch::VocabularyBuilder vb;

  // Build vocabulary for the ontology part
  if (ontologyFile.size() > 0)
  {
    if (ontologyOutput.size() == 0)
    {
      // Use a generic name for the output
      ontologyOutput = getOutputName(ontologyFile);
    }
    vb.constructVocabularyFromOntology(ontologyFile);
    vb.writeVocabularyToOutputFiles(ontologyOutput);
  }
  else
  {
    assert(ontologyOutput.size() == 0);
  }

  // Build vocabulary for the words-file(s)
  for (size_t i = 0; i < wordsFiles.size(); ++i)
  {
    vb.constructVocabularyFromASCIIWordsFile(wordsFiles[i]);
    vb.writeVocabularyToOutputFiles(getOutputName(wordsFiles[i]));
  }

  return 0;
}
