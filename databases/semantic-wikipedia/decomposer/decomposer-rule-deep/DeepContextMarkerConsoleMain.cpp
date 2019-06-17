// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <getopt.h>
#include <string>
#include <iomanip>
#include "base/CompositeContextDecomposer.h"
#include "decomposer-rule-deep/DeepContextMarker.h"
#include "decomposer-rule-deep/DeepContextRecombiner.h"
#include "util/ContextDecomposerUtil.h"
#include "base/ContextRecombiner.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"



using std::string;
using namespace ad_decompose; // NOLINT

typedef DeepContextMarker::DeepParserTerminalNode TNode;


// Available options.
struct option options[] =
{
{ "verbose", required_argument, NULL, 'v' },
{ NULL, 0, NULL, 0 } };

// Print usage
void printUsage()
{
  std::cout << "Usage:\n" << "\t ./DeepContextMarkerConsoleMain "
      << "<parse-string>\n" << std::endl << std::flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "DeepContextMarkerConsoleMain, version "
      << __DATE__ << " " << __TIME__ << ad_utility::EMPH_OFF
      << std::endl << std::endl;

  optind = 1;

  bool verbose = false;
  bool writeDecompInfo = false;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "v", options,
        NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case 'v':
        verbose = true;
        break;
      default:
        printUsage();
        exit(1);
    }
  }

  if (optind + 1 > argc)
  {
    printUsage();
    exit(1);
  }

  string inputString = argv[optind++];


  std::cout << "CSD for sentence: " << inputString << std::endl << std::endl;
  // SCI
  DeepContextMarker marker;
  // Parse deep-parsed sentence into a tree structure.
  DeepParserSentenceParser parser;
  // Where we keep the tree.
  DeepContextMarker::DeepParserNodeList rootList;
  // Where we keep the tokens.
  Sentence<DeepToken> sentence;
  // Parse it.
  bool parseResult = parser.parseSentenceFromString<TNode>(inputString,
      &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  if (!parseResult)
  {
    std::cerr << "String could not be parsed correctly." << std::endl;
    exit(1);
  }
  std::cout << "Sentence Tree as string: ";
  std::cout << rootList[0].treeAsString() << std::endl << std::endl;
  // Perform SCI.
  marker.markTree(rootList);


  // Now all the steps for SCR.
  // Parse the SCI output.
  DeepMarkerSentenceParser markedParser;
  // Where we keep the SCI-tree.
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListB;
  markedParser.parseSentence(sentence, &rootListB);
  // Simplify the SCI tree.
  SimplifyDeepMarkerTree simplyfier;
  simplyfier.apply(&rootListB[0]);
  std::cout << "SCI Tree as string: ";
  std::cout << rootListB[0].treeAsString() << std::endl << std::endl;
  // Perform SCR.
  DeepRecombiner<DeepToken> recombiner;
  Contexts<DeepToken> res = recombiner.apply(&rootListB[0]);
  std::cout
    << "Resulting contexts: " << std::endl << std::endl
    << res.asString() << std::endl;
  return 0;
}
