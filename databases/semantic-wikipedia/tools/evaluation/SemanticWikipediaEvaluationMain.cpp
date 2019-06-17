// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <string>
#include <iostream>
#include "./SemanticWikipediaEvaluation.h"

using std::cout;
using std::flush;
using std::endl;
using std::string;

// Available options.
struct option options[] =
{
    { "server-port", required_argument, NULL, 'p' },
    {"top-k", required_argument, NULL, 't' },
    { "host-ip", required_argument, NULL, 'i' },
    { "redirect-map", required_argument, NULL, 'r' },
    { NULL, 0, NULL, 0 }
};

// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./SemanticWikipediaEvaluationMain "
      << "<options> <groundTruthFile> <ouputFileName>\n"
      << "\t Available options:\n"
      << "\t\t --top-k (-t) <int>: Enable top k retrieval "
      << "to process the first k completions only" << endl
      << "\t\t --server-port (-p): The port the completesearch server with "
      << "semantic wikipedia database running. Default is 8887." << endl
      << "\t\t --host-ip (-i): The ip address of the completesearch server "
      << " with semantic wikipedia database running. Default is 127.0.0.1."
      << endl << "\t\t --redirect-map (-r): The redirect map" << endl << flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << "Semantic Wikipedia Evaluation. \n" << endl << flush;

  int port = 8887;
  int k = -1;
  string ip = "127.0.0.1";
  string redirectMap = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "p:i:t:r:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
    case 'p':
      port = atoi(optarg);
      break;
    case 't':
      k = atoi(optarg);
      break;
    case 'i':
      ip = optarg;
      break;
    case 'r':
      redirectMap = optarg;
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

  // File names.
  std::string groundTruthFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(groundTruthFile.size() > 0);
  assert(outputFile.size() > 0);

  // Start the evaluation.
  cout << "Starting evaluation... " << endl << flush;

  SemanticWikipediaEvaluation eval(ip, port);
  if (redirectMap.length() > 0)
    eval.readRedirectMap(redirectMap);
  if (k > 0)
    eval.enableTopK(k);
  eval.evaluate(groundTruthFile, outputFile);

  cout << "Evaluation done. " << endl;
}
