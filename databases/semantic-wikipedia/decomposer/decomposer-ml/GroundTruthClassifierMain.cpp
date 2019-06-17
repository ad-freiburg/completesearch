// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>


#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <iostream>
#include "./SVMClassifier.h"
#include "./LibSVMClassifier.h"
#include "./SVMLightClassifier.h"
#include "evaluation/GroundTruthReader.h"
#include "./FeatureExtractor.h"
#include "./MLExampleWriter.h"
#include "util/ContextDecomposerUtil.h"

using std::string;
using std::endl;
using std::flush;
using std::cout;

using ad_decompose::FeatureExtractorConfig;
using ad_decompose::FeatureMap;
using ad_decompose::FeatureExtractor;
using ad_decompose::GroundTruthReader;
using ad_decompose::GroundTruthSentence;
using ad_decompose::ContextDecomposerUtil;
using ad_decompose::SVMLightExampleWriter;
using ad_decompose::MLExample;
using ad_decompose::SVMClassifier;
using ad_decompose::LibSVMClassifier;
using ad_decompose::SVMLightClassifier;


// Available options.
struct option options[] =
{
    { "verbose", no_argument, NULL, 'v' },
    { "classifier", required_argument, NULL, 'c' },
    { "phraseBoundaries", required_argument, NULL, 'p' },
    { "feature_conf", required_argument, NULL, 'f' },
    { NULL, 0, NULL, 0 }
};

// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./SVMLightClassifierMain "
      << "<options> <featureMapFile> <modelFile>,<brTag>[...] "
      << "<groundTruthFile> "
      << "<outputFile>\n"
      << "\t Required options:\n" << endl
      << "\t --classifier (-c): the classifier to use: lib|light\n" << endl
      << "\t --feature_conf (-f): feature configuration file\n" << endl
      << "\t Optional options:\n" << endl
      << "\t --phraseBoundaries (-p): 0|1 consider only phrase boundaries\n"
      << endl
      << flush;
}

// Main function calls the parser.
int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "SVMLight Ground Truth Classifier, version " << __DATE__ << " "
      << __TIME__ << ad_utility::EMPH_OFF << std::endl;
  optind = 1;
  std::string classifierStr = "";
  std::string featureConfigFile = "";
  std::string phraseBoundaries = "0";

  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "c:f:p:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case('p'):
        phraseBoundaries = string(optarg);
        break;
      case('c'):
        classifierStr = string(optarg);
        break;
      case('f'):
        featureConfigFile = string(optarg);
        break;
      default:
        printUsage();
        exit(1);
    }
  }

  if (optind + 3 > argc)
  {
    std::cerr << "Wrong number of arguments." << std::endl;
    printUsage();
    exit(1);
  }

  // File names.
  std::string featureMapFile = argv[optind++];
  std::string modelFilesTagStr = argv[optind++];
  std::string indexFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(indexFile.size() > 0);
  assert(featureMapFile.size() > 0);
  assert(modelFilesTagStr.size() > 0);
  assert(outputFile.size() > 0);
  assert(classifierStr.size() > 0);
  assert(featureConfigFile.size() > 0);

  std::vector<string> modelFilesTag;
  boost::split(modelFilesTag, modelFilesTagStr, boost::is_any_of(","));
  if (modelFilesTag.size() % 2 != 0 || modelFilesTag.size() == 0)
  {
    std::cerr << "Incorrect format of modelFile,tag sequence.";
    printUsage();
    exit(1);
  }
  // Load the feature map.
  FeatureMap mfMap(featureMapFile, true);

  // Initialize the extractor configuration.
  // FeatureExtractorConfig config =
  //                 ContextDecomposerUtil::getDefaultFexConfig();
  FeatureExtractorConfig config = ContextDecomposerUtil::parseFeatureConfig(
      featureConfigFile);
  config.dynbrLeft = 0;
  config.dynbrRight = 0;
  FeatureExtractor fex(&mfMap, config, true);

  std::vector<SVMClassifier *> classifiers;

  // Initialize the classifiers.
  if (classifierStr == "lib")
  {
    for (size_t i = 0; i < modelFilesTag.size(); ++i)
    {
      LibSVMClassifier * svm = new LibSVMClassifier(modelFilesTag[i]);
      classifiers.push_back(svm);
      ++i;
    }
  }
  else if (classifierStr == "light")
  {
    for (size_t i = 0; i < modelFilesTag.size(); ++i)
    {
      SVMLightClassifier * svm = new SVMLightClassifier(modelFilesTag[i]);
      classifiers.push_back(svm);
      ++i;
    }
  }
  else
  {
    std::cout << "Unknown classifier: " << classifierStr << endl;
    printUsage();
    exit(1);
  }

  // Start the classification
  cout << "Starting classification... " << endl << flush;

  // Evaluation currently only works with DefaultToken.
  GroundTruthReader<DefaultToken> gtReader;

  boost::ptr_vector<GroundTruthSentence<DefaultToken> > sentences;
  gtReader.readGroundTruthsFromFile(&sentences, indexFile, true, true);
  boost::ptr_vector<GroundTruthSentence<DefaultToken> > correctSentences;
  gtReader.readGroundTruthsFromFile(&correctSentences, indexFile, true, false);

  // SemanticWikipediaWriter sww(outputFile);


  for (size_t j = 0; j < sentences.size(); ++j)
  {
    // The br tags are not part of the counts, so it is enough
    // to initialize once.
    fex.newSentence(sentences[j].sentence);
    for (size_t k = 0; k < classifiers.size(); ++k)
    {
      // Classify all words.
      if (phraseBoundaries != "1")
      {
        for (size_t i = 0; i < sentences[j].sentence.getWords().size(); ++i)
        {
          if (classifiers[k]->classifyFV(fex.extractWordFeatures(i)))
          {
            fex.appendBrTag(i, modelFilesTag[2 * k + 1]);
            sentences[j].sentence.appendBrTag(i, modelFilesTag[2 * k + 1]);
          }
        }
      }
      // Only classify words on chunk boundaries.
      else if (phraseBoundaries == "1")
      {
        vector<Phrase<DefaultToken> *> const & phrases =
            sentences[j].sentence.getPhrases();
        for (size_t i = 0; i < phrases.size(); ++i)
        {
          size_t start = phrases[i]->getWordsStartIndex();
          size_t end = phrases[i]->getWordsEndIndex();
          if (classifiers[k]->classifyFV(fex.extractWordFeatures(start)))
          {
            fex.appendBrTag(start, modelFilesTag[2 * k + 1]);
            sentences[j].sentence.appendBrTag(start, modelFilesTag[2 * k + 1]);
          }
          if (start != end)
          {
            if (classifiers[k]->classifyFV(fex.extractWordFeatures(end)))
            {
              fex.appendBrTag(end, modelFilesTag[2 * k + 1]);
              sentences[j].sentence.appendBrTag(end, modelFilesTag[2 * k + 1]);
            }
          }
        }
      }
    }
    for (size_t i = 0; i < sentences[j].sentence.getWords().size(); ++i)
    {
      std::cout << sentences[j].sentence.getWords()[i]->tokenString << "\t"
          << correctSentences[j].sentence.getWords()[i]->brTag << "\t"
          << sentences[j].sentence.getWords()[i]->brTag << "\n";
    }
    std::cout << "\n";
  }
  for (size_t i = 0; i < classifiers.size(); ++i)
    delete classifiers[i];
}
