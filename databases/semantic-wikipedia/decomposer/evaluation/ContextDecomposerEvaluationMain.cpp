// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include "evaluation/ContextDecomposerEvaluation.h"
#include "util/ContextDecomposerUtil.h"
#include "base/CompositeContextDecomposer.h"
#include "decomposer-ml/MLContextMarker.h"
#include "decomposer-rule/RobustContextMarker.h"
#include "base/ContextRecombiner.h"


using std::string;
using std::endl;
using std::flush;
using std::cout;

// Main class for context decomposition evaluation.
// Uses ContextDecomposerEvaluation which evaluates
// a selected decomposer against a ground truth.

// Available options.
struct option options[] =
{
    { "decomposer", required_argument, NULL, 'd' },
    { "gpos_datadir", required_argument, NULL, 'g' },
    { "simplify_bin", required_argument, NULL, 's' },
    { "equality_pt", required_argument, NULL, 'e' },
    { "featureMapFile", required_argument, NULL, 'm' },
    { "clauseFeatureMapFile", required_argument, NULL, 'l' },
    { "clauseClassifiers", required_argument, NULL, 'c' },
    { "wordClassifiers", required_argument, NULL, 'w' },
    { "openClassifiers", required_argument, NULL, 'o' },
    { "closeClassifiers", required_argument, NULL, 'k' },
    { "featureConfig", required_argument, NULL, 'f' },
    { "verbose", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
};

// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./ContextDecomposerEvaluationMain "
      << "<options> <groundTruthFile> <ouputFileName>\n"
      << "\t Required options:\n"
      << "\t\t --decomposer (-d): choose the decomposer to "
      << "evaluate: \"robust\" \"ml\" \"simplify\"" << endl
      << "\t\t If you select \"simplify\" you also need to provide "
      << "the location to the binary with --simplify_bin (-s)" << endl
      << "\t\t If you select \"ml\" you also need to provide "
      << "the classifier model files with --wordClassifiers (-w)" << endl
      << ",--clauseClassifiers (-c), --openClassifiers (-o)" << endl
      << ",--closeClassifiers (-k), a feature config file (-f)" << endl
      << "a clause feature map config file (-l)" << endl
      << "and a feature map file (-m)" << endl
      << "\t\t --gpos_datadir (-g): path to gposttl's data directory"
      << "\t\t --verbose (-v): enable verbose output (default:disabled)"
      << endl << "\t\t --equality_pt (-e): a string of POS tags that "
      << "define equality\n" << endl << flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "Context Decomposer Evaluation, version " << __DATE__ << " "
      << __TIME__ << ad_utility::EMPH_OFF << std::endl;
  string decomposer = "";
  string gposDataDir = "";
  string equalityPosTags = "";
  string simplifyBin = "";
  string wordClassifiers = "";
  string clauseClassifiers = "";
  string openClassifiers = "";
  string closeClassifiers = "";
  string featureConfig = "";
  string featureMapFile = "";
  string clauseFeatureMapFile = "";

  bool verbose = false;
  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "s:d:g:e:vw:c:f:m:o:k:l:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
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
      case 'd':
        decomposer = string(optarg);
        break;
      case 'e':
        equalityPosTags = string(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'g':
        gposDataDir = string(optarg);
        break;
      case 's':
        simplifyBin = string(optarg);
        break;
      default:
        printUsage();
        exit(1);
    }
  }

  if (gposDataDir == "")
  {
    std::cerr << "No gposttl data directory specified." << std::endl;
    printUsage();
    exit(1);
  }
  else if (decomposer == "")
  {
    std::cerr << "You need to specify which decomposer to evaluate"
        << " using --decomposer (-d)." << std::endl;
    printUsage();
    exit(1);
  }
  else if (equalityPosTags == "")
  {
    std::cerr << "You need to specify a list of POS tags for equality"
        << " using --equality_pt (-pt)." << std::endl;
    printUsage();
    exit(1);
  }

  if (optind + 2 > argc)
  {
    printUsage();
    exit(1);
  }

  // File names.
  std::string groundTruthFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(groundTruthFile.size() > 0);
  assert(outputFile.size() > 0);

  // Start the evaluation.
  cout << "Starting evaluation for " << decomposer << "... " << endl << flush;
  CompositeContextDecomposer<DefaultToken> evalDecomp;
  ContextMarkerBase<DefaultToken> * marker = NULL;

  if (decomposer == "robust")
  {
    // evalDecomp = new RobustContextDecomposer(verbose);
    marker = new RobustContextMarker(verbose);
  }
  else if (decomposer == "simplify")
  {
    if (simplifyBin == "")
    {
      std::cerr << "You need to provide the interface binary to the"
          << " simplify system with --simplify_bin (-s) \n";
      printUsage();
      exit(1);
    }
// evalDecomp = new SiddharthanSimplifier(simplifyBin, verbose);
// ContextDecomposerEvaluation contextEval(gposDataDir, equalityPosTags);
// contextEval.evaluate(*evalDecomp, groundTruthFile, outputFile, false, true);
  }
  else if (decomposer == "ml")
  {
    if (clauseClassifiers == "" || wordClassifiers == "" || featureConfig == ""
        || featureMapFile == "" || openClassifiers == "" || closeClassifiers
        == "" || clauseFeatureMapFile == "")
    {
      std::cerr << "You need to provide the word and clause classifiers, "
          << "a feature config file and a feature map file.\n";
      printUsage();
      exit(1);
    }
    std::cout << "Using: \n";
    std::cout << "FeatureConfig: " << featureConfig << std::endl;
    std::cout << "FeatureMap: " << featureMapFile << std::endl;
    std::cout << "ClauseFeatureMap: " << clauseFeatureMapFile << std::endl;
    std::cout << "clauseClassifiers: " << clauseClassifiers << std::endl;
    std::cout << "wordClassifiers: " << wordClassifiers << std::endl;
    std::cout << "openClassifiers: " << openClassifiers << std::endl;
    std::cout << "closeClassifiers: " << closeClassifiers << std::endl;

    marker = new MLContextMarker(verbose, true,
        wordClassifiers, openClassifiers, closeClassifiers, clauseClassifiers,
        featureMapFile, clauseFeatureMapFile, featureConfig);
  }
  else
  {
    std::cerr << "Undefined decomposer: " << decomposer << "\n";
    printUsage();
    exit(1);
  }

  ad_decompose::ContextRecombiner<DefaultToken> extractor;
  evalDecomp.setContextExtractor(&extractor);
  evalDecomp.setContextMarker(marker);
  ContextDecomposerEvaluation<DefaultToken> contextEval(gposDataDir,
      equalityPosTags);
  contextEval.evaluate(evalDecomp, groundTruthFile, outputFile, true, true);
  delete marker;
  cout << "Evaluation done. " << endl;
}
