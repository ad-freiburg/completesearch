// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <getopt.h>
#include <string>
#include <iomanip>
#include "./SemanticWikipediaWriter.h"
#include "./SemanticWikipediaReader.h"
#include "./SemanticWikipediaDecomposer.h"
#include "base/CompositeContextDecomposer.h"
#include "decomposer-ml/MLContextMarker.h"
#include "decomposer-rule/RobustContextMarker.h"
#include "decomposer-rule-deep/DeepContextMarker.h"
#include "decomposer-rule-deep/DeepContextRecombiner.h"
#include "util/ContextDecomposerUtil.h"
#include "base/ContextRecombiner.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"



using std::string;
using ad_decompose::MLContextMarker;
using ad_decompose::CompositeContextDecomposer;
using ad_decompose::SemanticWikipediaReader;
using ad_decompose::SemanticWikipediaWriter;
using ad_decompose::SemanticWikipediaDecomposer;

// Available options.
struct option options[] =
{
{ "decomposer", required_argument, NULL, 'd' },
{ "wordsUnsortedFile", required_argument, NULL, 'x' },
{ "verbose", no_argument, NULL, 'v' },
{ "featureMapFile", required_argument, NULL, 'm' },
{ "clauseFeatureMapFile", required_argument, NULL, 'l' },
{ "clauseClassifiers", required_argument, NULL, 'c' },
{ "wordClassifiers", required_argument, NULL, 'w' },
{ "openClassifiers", required_argument, NULL, 'o' },
{ "closeClassifiers", required_argument, NULL, 'k' },
{ "featureConfig", required_argument, NULL, 'f' },
{ "minimumWordLength", required_argument, NULL, 'n' },
{ "stopWordsFile", required_argument, NULL, 's' },
{ "cliticsFile", required_argument, NULL, 'i' },
{ "writeDecompInfo", no_argument, NULL, 'y' },


{ NULL, 0, NULL, 0 } };

// Print usage
void printUsage()
{
  std::cout << "Usage:\n" << "\t ./SemanticWikipediaDecomposerMain "
      << "<indexFile>\n" << std::endl << std::flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON << "Wikipedia chunker, version "
      << __DATE__ << " " << __TIME__ << ad_utility::EMPH_OFF
      << std::endl << std::endl;

  optind = 1;
  string wordClassifiers = "";
  string clauseClassifiers = "";
  string openClassifiers = "";
  string closeClassifiers = "";
  string featureConfig = "";
  string decomposer = "";
  string featureMapFile = "";
  string clauseFeatureMapFile = "";
  string stopWordsFile = "";
  string cliticsFile = "";
  size_t minWordLength = 1;
  string dbName;
  string wordsUnsortedfileName;

  bool verbose = false;
  bool writeDecompInfo = false;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "i:s:d:g:e:vw:c:f:m:o:k:l:n:s:x:y", options,
        NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case('y'):
        writeDecompInfo = true;
        break;
      case('x'):
        wordsUnsortedfileName = string(optarg);
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
      case 'd':
        decomposer = string(optarg);
        break;
      case 'v':
        verbose = true;
        break;
      case 'n':
        minWordLength = atoi(optarg);
        break;
      case 's':
        stopWordsFile = string(optarg);
        break;
      case 'i':
        cliticsFile = string(optarg);
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

  // Default file names
  dbName = argv[optind++];
  // std::string  wordsOutFileName = argv[optind++];

  std::string xmlFileName = dbName + ".xml";
  std::string docsFileName;
  std::string longSentenceFileName;

  wordsUnsortedfileName == "" ? docsFileName = dbName + ".docs-unsorted"
      : docsFileName = wordsUnsortedfileName + ".docs-unsorted";
  wordsUnsortedfileName == "" ? longSentenceFileName = dbName
      + ".very-long-sentences" : longSentenceFileName =
          wordsUnsortedfileName + ".very-long-sentences";
  std::string wordsFileName;
  wordsUnsortedfileName == "" ? wordsFileName = dbName
      + ".words-unsorted.ascii" : wordsFileName = wordsUnsortedfileName
      + ".decomposed";
  std::string taggedWordsFileName;
  wordsUnsortedfileName == "" ? taggedWordsFileName = dbName
      + ".words-unsorted.prechunk.tagged.ascii" : taggedWordsFileName
      = wordsUnsortedfileName;
  std::string vocabularyFileName = dbName + ".vocabulary";
  std::string entitiesFileName = dbName + ".words-unsorted.entities";
  std::string logFileName = dbName + ".parse-log";

  if (decomposer == "ml")
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

    MLContextMarker marker(verbose, true, wordClassifiers,
        openClassifiers, closeClassifiers, clauseClassifiers, featureMapFile,
        clauseFeatureMapFile, featureConfig);
    ContextMarkerBase<DefaultToken> * markerBase = &marker;
    CompositeContextDecomposer<DefaultToken> decomp;
    ad_decompose::ContextRecombiner<DefaultToken> extractor;
    decomp.setContextExtractor(&extractor);
    decomp.setContextMarker(markerBase);
    // Read from words file.
    SemanticWikipediaReader<DefaultToken> reader;
    reader.setInputFilename(taggedWordsFileName);
    // Write to words-out and docs-out.
    SemanticWikipediaWriter<WRITE_DOCS_FILE + WRITE_DECOMP_INFO,
    DefaultToken> writer(wordsFileName, docsFileName, longSentenceFileName,
        minWordLength);
    writer.setStopWordsFile(stopWordsFile);
    writer.setCliticsFile(cliticsFile);
    // Combine decomposer, writer and reader.
    SemanticWikipediaDecomposer<DefaultToken> xc(verbose, decomp,
        writer, reader);
    xc.parseAndChunk();
    LOG(INFO) << "Finishing..." << std::endl;
  }
  else if (decomposer == "deep")
  {
    ad_decompose::DeepContextMarker marker;
    ContextMarkerBase<DeepToken> * markerBase = &marker;
    CompositeContextDecomposer<DeepToken> decomp;
    ad_decompose::DeepContextRecombiner<DeepToken> extractor;
    decomp.setContextExtractor(&extractor);
    decomp.setContextMarker(markerBase);
    // Read from words file.
    SemanticWikipediaReader<DeepToken> reader;
    reader.setInputFilename(taggedWordsFileName);
    // Write to words-out and docs-out.
    SemanticWikipediaWriter<WRITE_DOCS_FILE + WRITE_DECOMP_INFO, DeepToken>
      writer(
        wordsFileName, docsFileName, longSentenceFileName, minWordLength);
    writer.setStopWordsFile(stopWordsFile);
    writer.setCliticsFile(cliticsFile);
    // Combine decomposer, writer and reader.
    SemanticWikipediaDecomposer<DeepToken>
      xc(verbose, decomp, writer, reader);
    xc.parseAndChunk();
    LOG(INFO) << "Finishing..." << std::endl;
  }
  else
  {
    RobustContextMarker marker(verbose);
    CompositeContextDecomposer<DefaultToken> decomp;
    ad_decompose::ContextRecombiner<DefaultToken> extractor;
    ContextMarkerBase<DefaultToken> * markerBase = &marker;
    decomp.setContextExtractor(&extractor);
    decomp.setContextMarker(markerBase);
    // Read from words file.
    SemanticWikipediaReader<DefaultToken> reader;
    reader.setInputFilename(taggedWordsFileName);
    // Write to words-out and docs-out.
    SemanticWikipediaWriter<WRITE_DOCS_FILE + WRITE_DECOMP_INFO, DefaultToken>
      writer(wordsFileName, docsFileName, longSentenceFileName, minWordLength);
    writer.setStopWordsFile(stopWordsFile);
    writer.setCliticsFile(cliticsFile);
    // Combine decomposer, writer and reader.
    SemanticWikipediaDecomposer<DefaultToken>
      xc(verbose, decomp, writer, reader);
    xc.parseAndChunk();
    LOG(INFO) << "Finishing..." << std::endl;
  }
}
