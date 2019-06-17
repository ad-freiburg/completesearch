// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <getopt.h>
#include <string>
#include <iostream>
#include "./RedirectMapBuilder.h"

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
{ NULL, 0, NULL, 0 } };

// Prints usage hints.
void printUsage()
{
  std::cerr << "Usage: ./RedirectMapBuilderMain <DB>" << std::endl
      << std::flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON << "Redirect-Map Builder, version "
      << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      default:
        printUsage();
    }
  }

  // File names.
  std::string dbName = optind < argc ? argv[optind++]
      : "semantic-wikipedia-test";
  std::string xmlFileName = dbName + ".xml";
  std::string outputFileName = dbName + ".redirect-map-unsorted";

  // Parse the xml file.
  semsearch::RedirectMapBuilder rmb(outputFileName);
  rmb.process(xmlFileName);
  std::cout << std::endl;
}
