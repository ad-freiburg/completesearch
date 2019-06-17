// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <getopt.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include "./OntologyManager.h"
#include "./SUSIOutputWriter.h"
#include "./RelationsOutputWriterOld.h"
#include "./RelationsOutputWriter.h"

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[0m"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::vector;

using ad_semsearch::OntologyManager;
using ad_semsearch::OutputWriter;
using ad_semsearch::RelationsOutputWriter;
using ad_semsearch::RelationsOutputWriterOld;
using ad_semsearch::SUSIOutputWriter;

// Available options.
struct option options[] =
{
  {"susi-input", required_argument, NULL, 's'},
  {"paths-with-special-chars", required_argument, NULL, 'p'},
  {"relations", required_argument, NULL, 'r'},
  {"broccoli", required_argument, NULL, 'b'},
  {"ontology-ascii-output", required_argument, NULL, 'o'},
  {"redirect-map", required_argument, NULL, 'R'},
  {"wordnet-synonyms", required_argument, NULL, 'W'},
  {"wordnet-naming-map", required_argument, NULL, 'w'},
  {NULL, 0, NULL, 0}
};

// Main function calls the parser.
int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "OntologyManager, version " << __DATE__ << " "
      << __TIME__ << EMPH_OFF << endl << endl << flush;

  string susiOutputDir = "";
  string relationsOutputFile = "";
  string ontologyOutputFile = "";
  string wordnetNamingMapFile = "";
  string wordnetSynonymsFile = "";
  string redirectMapFile = "";
  bool pathsWithUnderscores = false;

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "s:r:b:o:w:W:p:R:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 's':
      susiOutputDir = optarg;
      break;
    case 'p':
      susiOutputDir = optarg;
      pathsWithUnderscores = true;
      break;
    case 'r':
      relationsOutputFile = optarg;
      break;
    case 'b':
      ontologyOutputFile = optarg;
      break;
    case 'o':
      ontologyOutputFile = optarg;
      break;
    case 'w':
      wordnetNamingMapFile = optarg;
      break;
    case 'W':
      wordnetSynonymsFile = optarg;
      break;
    case 'R':
      redirectMapFile = optarg;
      break;
    default:
      std::cout << std::endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << std::endl << std::endl;
      exit(1);
    }
  }

  if (wordnetNamingMapFile.size() > 0 && ontologyOutputFile.size() == 0
      && susiOutputDir.size() == 0)
  {
    cout << "Option -w aka --wordnet-naming-map currently is only supported"
        << " in conjunction with either -b aka --broccoli "
        << "or -s aka --susi-input" << endl;
    exit(1);
  }

  // File names.
  std::string ontologyFolder = optind < argc ? argv[optind++] : "ontology";
  OntologyManager om(ontologyFolder);
  cout << "Processing pending corrections now ..." << endl << flush;
  om.applyCorrections();
  cout << "No more pending corrections left." << endl << flush;
  cout << endl << flush;

  // Remeber those, because they have to be deleted.
  vector<OutputWriter*> registeredWriters;

  if (susiOutputDir.size() > 0)
  {
    SUSIOutputWriter* susiWriter = new SUSIOutputWriter(susiOutputDir,
        pathsWithUnderscores);
    if (wordnetNamingMapFile.size() > 0)
    {
      susiWriter->setOntologyCategoryNamesMap(wordnetNamingMapFile);
      susiWriter->readRedirectMap(redirectMapFile);
      if (wordnetNamingMapFile.size() > 0)
      {
        susiWriter->setOntologyNamesMapping(wordnetNamingMapFile);
      }
      if (wordnetSynonymsFile.size() > 0)
      {
        susiWriter->readWordnetSynonyms(wordnetSynonymsFile);
      }
    }
    registeredWriters.push_back(susiWriter);
    cout << "SUSI Mode." << endl << "Registering SUSI writer..." << flush;
    om.registerOutputWriter(susiWriter);
    cout << " registered." << endl << flush;
  }
  if (relationsOutputFile.size() > 0)
  {
    RelationsOutputWriterOld* relWriter = new RelationsOutputWriterOld(
        relationsOutputFile, false);
    registeredWriters.push_back(relWriter);
    cout << "Registering Relations writer (not treaing Is-A for old index)..."
        << flush;
    om.registerOutputWriter(relWriter);
    cout << " registered." << endl << flush;
  }
  if (ontologyOutputFile.size() > 0)
  {
    assert(redirectMapFile.size() > 0);
    RelationsOutputWriter* relWriter = new RelationsOutputWriter(
        ontologyOutputFile);
    relWriter->readRedirectMap(redirectMapFile);
    if (wordnetNamingMapFile.size() > 0)
    {
      relWriter->setOntologyNamesMapping(wordnetNamingMapFile);
    }
    if (wordnetSynonymsFile.size() > 0)
    {
      relWriter->readWordnetSynonyms(wordnetSynonymsFile);
    }
    registeredWriters.push_back(relWriter);
    cout << "Registering Relations writer..." << flush;
    om.registerOutputWriter(relWriter);
    cout << " registered." << endl << flush;
  }
  cout << "Generating output now using the registered writers. There are "
      << om.getNumberOfWriters() << " writers registered." << endl
      << "Writing outout ..." << endl << flush;
  om.generateOutput();
  cout << "Generating all output done." << endl << endl << flush;
  for (size_t i = 0; i < registeredWriters.size(); ++i)
  {
    delete registeredWriters[i];
  }
  return 0;
}

