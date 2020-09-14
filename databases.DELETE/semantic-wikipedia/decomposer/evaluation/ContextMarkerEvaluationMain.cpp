// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include <utility>
#include <iostream>
#include <vector>
#include "base/ContextDecomposerBase.h"
#include "base/ContextMarkerBase.h"
#include "./ContextMarkerEvaluation.h"
#include "decomposer-ml/MLContextMarker.h"
#include "decomposer-rule/RobustContextMarker.h"
#include "../codebase/server/Globals.h"

using std::string;
using std::endl;
using std::flush;
using std::cout;
using std::pair;

// Main class for context marker evaluation.
// Uses ContextMarkerEvaluation which evaluates
// a selected marker against a ground truth.


// Available options.
struct option options[] =
{
    { "featureMapFile", required_argument, NULL, 'm' },
    { "clauseFeatureMapFile", required_argument, NULL, 'l' },
    { "clauseClassifiers", required_argument, NULL, 'c' },
    { "wordClassifiers", required_argument, NULL, 'w' },
    { "openClassifiers", required_argument, NULL, 'o' },
    { "closeClassifiers", required_argument, NULL, 'k' },
    { "featureConfig", required_argument, NULL, 'f' },
    { NULL, 0, NULL, 0 }
};

// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./ContextMarkerEvaluationMain "
      << "<options> <groundTruthFile> <ouputFileName>\n"
      << "\t Required options:\n" << endl

  << "\t Other options:\n" << endl
      << "\t --verbose (v): provide verbose output\n" << endl << flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON << "Context Marker Evaluation, version "
      << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl;

  bool verbose = false;
  string wordClassifiers = "";
  string clauseClassifiers = "";
  string openClassifiers = "";
  string closeClassifiers = "";
  string featureConfig = "";
  string featureMapFile = "";
  string clauseFeatureMapFile = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "m:l:c:w:o:k:f:v", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case 'v':
        verbose = true;
        break;
      case('l'):
        clauseFeatureMapFile = string(optarg);
        break;
      case('o'):
        openClassifiers = string(optarg);
        break;
      case('k'):
        closeClassifiers = string(optarg);
        break;
      case('m'):
        featureMapFile = string(optarg);
        break;
      case('f'):
        featureConfig = string(optarg);
        break;
      case('c'):
        clauseClassifiers = string(optarg);
        break;
      case('w'):
        wordClassifiers = string(optarg);
        break;
      default:
        printUsage();
        exit(1);
    }
  }

  if (clauseClassifiers == "" || wordClassifiers == "" || featureConfig == ""
      || featureMapFile == "" || openClassifiers == "" || closeClassifiers
      == "" || clauseFeatureMapFile == "")
  {
    std::cerr << "You need to provide the word and clause classifiers, "
        << "a feature config file and a feature map file.\n";
    printUsage();
    exit(1);
  }

  if (optind + 2 > argc)
  {
    std::cout << "HERE";
    printUsage();
    exit(1);
  }

  // File names.
  std::string groundTruthFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(groundTruthFile.size() > 0);
  assert(outputFile.size() > 0);

  // Start the evaluation.
  cout << "Starting evaluation ..." << endl << flush;

  std::cout << "Using: \n";
  std::cout << "FeatureConfig: \t" << featureConfig << std::endl;
  std::cout << "FeatureMap: \t" << featureMapFile << std::endl;
  std::cout << "ClauseFeatureMap: \t" << clauseFeatureMapFile << std::endl;
  std::cout << "clauseClassifiers: \t" << clauseClassifiers << std::endl;
  std::cout << "wordClassifiers: \t" << wordClassifiers << std::endl;
  std::cout << "openClassifiers: \t" << openClassifiers << std::endl;
  std::cout << "closeClassifiers: \t" << closeClassifiers << std::endl;

  std::vector<pair<string, ContextMarkerBase<DefaultToken> *> > markers;
  ContextMarkerBase<DefaultToken> * mlMarker = new MLContextMarker
      (verbose, true,
      wordClassifiers, openClassifiers, closeClassifiers, clauseClassifiers,
      featureMapFile, clauseFeatureMapFile, featureConfig);
  markers.push_back(std::make_pair("MLMarker", mlMarker));
  ContextMarkerBase<DefaultToken> * ruleMarker = new RobustContextMarker(
      verbose);
  markers.push_back(std::make_pair("RuleMarker", ruleMarker));

  ContextMarkerEvaluation evalMarker(markers, groundTruthFile, outputFile);
  evalMarker.evaluate();
  cout << "Evaluation done. " << endl;
}
