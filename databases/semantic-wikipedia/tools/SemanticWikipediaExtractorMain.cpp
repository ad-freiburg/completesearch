// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <getopt.h>
#include <string>
#include <iostream>
#include "./SemanticWikipediaExtractor.h"

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
{ "in-text-substring", required_argument, NULL, 's' },
{ "regex-match", required_argument, NULL, 'r' },
{ "titles-file", required_argument, NULL, 't' },
{ "input-file", required_argument, NULL, 'i' },
{ "output-file", required_argument, NULL, 'o' },
{ "match-all", no_argument, NULL, 'a'},
{ NULL, 0, NULL, 0 } };

// Prints usage hints.
void printUsage()
{
  std::cerr << "Usage Problem!" << std::endl;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON << "Semantic-Wikipedia Extractor, version "
        << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;

  std::string inTextPattern = "";
  std::string regex = "";
  std::string titlesFile = "";
  std::string inputFile = "";
  std::string outputFile = "";
  bool unionMode = true;

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "s:r:t:i:o:a", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case 's':
        inTextPattern = optarg;
        break;
      case 'r':
        regex = optarg;
        break;
      case 't':
        titlesFile = optarg;
        break;
      case 'i':
        inputFile = optarg;
        break;
      case 'o':
        outputFile = optarg;
        break;
      case 'a':
        unionMode = false;
      default:
        printUsage();
        exit(1);
    }
  }

  if (inputFile.size() == 0)
  {
    std::cerr << "--input-file has to be specified!" << std::endl;
    exit(1);
  }

  if (outputFile.size() == 0)
  {
    outputFile = inputFile + "-extracted";
  }

  // Parse the xml file.
  if (unionMode)
  {
    SemanticWikipediaExtractor<MATCH_ANY_MODE> de(inTextPattern,
        regex, titlesFile);
    de.extract(inputFile, outputFile);
  } else
  {
    SemanticWikipediaExtractor<MATCH_ALL_MODE> de(inTextPattern,
        regex, titlesFile);
    de.extract(inputFile, outputFile);
  }
  std::cout << std::endl;
}
