// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
  {"ontology-ascii-input-file", required_argument, NULL, 'i'},
  {"output-file-name", required_argument, NULL, 'o'},
  {NULL, 0, NULL, 0}
};

struct EntityFlags
{
    bool _isPerson;
};
// Main function.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON
      << "CreateOntologyEntityListMain, version " << __DATE__ << " "
      << __TIME__ << EMPH_OFF << std::endl << std::endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string inputFile = "";
  string outputFile = "ontology.entity-list";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "i:o:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'i':
      inputFile = optarg;
      break;
    case 'o':
      outputFile = optarg;
      break;
    default:
      cout << endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  if (inputFile.size() == 0)
  {
    LOG(ERROR) << "Required argument --ontology-ascii-input-file (-i)!" << endl;
    exit(1);
  }

  LOG(INFO)
  << "Creating ontology entity-list from " << inputFile << "..." << endl;
  std::fstream input(inputFile.c_str(), std::ios_base::in);


  string line;

  ad_utility::HashMap<string, EntityFlags> entities;
  while (std::getline(input, line))
  {
    size_t indexOfTab1 = line.find('\t');
    size_t indexOfTab2 = line.find('\t', indexOfTab1 + 1);
    size_t indexOfTab3 = line.find('\t', indexOfTab2 + 1);
    size_t indexOfTab4 = line.find('\t', indexOfTab3 + 1);
    if (line.substr(0, indexOfTab1) != string(ad_semsearch::RELATION_PREFIX)
        + ad_semsearch::IS_A_RELATION)
    {
      continue;
    }
    string fourthColumn = line.substr(indexOfTab3 + 1,
        indexOfTab4 - (indexOfTab3 + 1));
    string fifthColumn = line.substr(indexOfTab4 + 1);
    ad_utility::HashMap<string, EntityFlags>::iterator it = entities.find(
        fourthColumn);
    if (it == entities.end())
    {
      EntityFlags flags;
      flags._isPerson = (ad_utility::getLastPartOfString(fifthColumn, ':')
          == "Person");
      entities[fourthColumn] = flags;
    }
    else
    {
      if (ad_utility::getLastPartOfString(fifthColumn, ':') == "Person")
      {
        entities[fourthColumn]._isPerson = true;
      }
    }
    it = entities.find(fifthColumn);
    if (it == entities.end())
    {
      EntityFlags flags;
      flags._isPerson = (ad_utility::getLastPartOfString(fifthColumn, ':')
          == "Person");
      entities[fifthColumn] = flags;
    }
    else
    {
      if (ad_utility::getLastPartOfString(fifthColumn, ':') == "Person")
      {
        entities[fifthColumn]._isPerson = true;
      }
    }
  }

  LOG(INFO) << "File IO for writing..." << endl;
  std::fstream output(outputFile.c_str(), std::ios_base::out);
  for (ad_utility::HashMap<string, EntityFlags>::const_iterator it =
      entities.begin(); it != entities.end(); ++it)
  {
    output << it->first << '\t' << (it->second._isPerson ? "1" : "0") << endl;
  }
  LOG(INFO)
    << "Creation of ontology entity-list done." << endl;
  return 0;
}
