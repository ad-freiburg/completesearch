// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/ptr_container/ptr_vector.hpp>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <iostream>
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "evaluation/GroundTruthReader.h"
#include "./FeatureExtractor.h"
#include "util/ContextDecomposerUtil.h"
#include "./MLExampleWriter.h"

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


// Available options.
struct option options[] =
{
    { "verbose", no_argument, NULL, 'v' },
    { "brTag", required_argument, NULL, 'b' },
    { "phraseBoundaries", required_argument, NULL, 'p' },
    { "fmap", required_argument, NULL, 'm' },
    { "lockFmap", required_argument, NULL, 'l' },
    { "feature_conf", required_argument, NULL, 'f' },
    { NULL, 0, NULL, 0 }
};



// Prints usage hints.
void printUsage()
{
  cout << "Usage:\n" << "\t ./GroundTruthFeatureExtractorMain "
      << "<options> <groundTruthFile> <ouputFileName>\n"
      << "\t Required options:\n" << endl
      << "\t --brTagR (-b): the br tag to classify for\n" << endl
      << "\t --brTypes (-t): golden br tags consider, a space "
      << "separated string\n"
      << endl
      << "\t --fmap (-m): file to read the feature map from\n" << endl
      << "\t --feature_conf (-f): feature configuration file\n" << endl
      << "\t Optional options:\n" << endl
      << "\t --phraseBoundaries (-p): 0|1 consider only phrase boundaries\n"
      << endl
      << "\t --lockFmap (-l): 0|1 lock the feature map\n" << endl
      << flush;
}

int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "Ground Truth Feature Extractor, version " << __DATE__ << " "
      << __TIME__ << ad_utility::EMPH_OFF << std::endl;
  optind = 1;
  std::string tagStr = "";
  std::string featureMapFile = "";
  std::string brTypesStr = "";
  std::string featureConfigFile = "";
  std::string phraseBoundaries = "0";
  bool lockFmap = false;

  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "b:m:t:f:p:l:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case('l'):
        if (string(optarg) == "1")
          lockFmap = true;
        break;
      case('p'):
        phraseBoundaries = string(optarg);
        break;
      case('f'):
        featureConfigFile = string(optarg);
        break;
      case('m'):
        featureMapFile = string(optarg);
        break;
      case('b'):
        tagStr = string(optarg);
        break;
      case('t'):
        brTypesStr = string(optarg);
        break;
      default:
        printUsage();
        exit(1);
    }
  }

  if (optind + 2 > argc)
  {
    printUsage();
    exit(1);
  }

  if (tagStr == "")
  {
    std::cout << "Missing brTag.";
    printUsage();
    exit(1);
  }

  // File names.
  std::string groundTruthFile = argv[optind++];
  std::string outputFile = argv[optind++];

  assert(groundTruthFile.size() > 0);
  assert(outputFile.size() > 0);
  assert(featureConfigFile.size() > 0);

  // Start the extraction.
  cout << "Starting feature extraction... " << endl << flush;
  // Our feature map and the ground truth reader.
  // FeatureMap fMap(groundTruthFile + ".map", lockFmap);
  GroundTruthReader<DefaultToken> reader;

  if (featureMapFile == "")
  {
    std::cerr << "Missing feature map file in parameters\n";
  }

  FeatureMap fMap(featureMapFile, lockFmap);


  // Update the configuration with the passed parameter.
  FeatureExtractorConfig config = ContextDecomposerUtil::parseFeatureConfig(
      featureConfigFile);
  config.brTypes = brTypesStr;
  config.dynbrType = tagStr;

  FeatureExtractor fex(&fMap, config);
  boost::ptr_vector<GroundTruthSentence<DefaultToken> > sentences;
  reader.readGroundTruthsFromFile(&sentences, groundTruthFile, true);
  std::vector<MLExample> mlExamples;

  // Consider all words as candidates.
  if (phraseBoundaries != "1")
  {
    // Extract features for each sentence and each word.
    for (size_t i = 0; i < sentences.size(); ++i)
    {
      LOG(INFO)
        << "Extracting features from sentence " << i << "\n";
      fex.newSentence(sentences[i].sentence);
      for (size_t j = 0; j < sentences[i].sentenceTokens.size(); ++j)
      {
        double mlClass = -1.0;
        // Classify as +1 if the tag we classify for is contained.
        if (sentences[i].sentenceTokens[j]->brTag.find(tagStr)
            != std::string::npos)
        {
          mlClass = +1.0;
        }
        mlExamples.push_back(MLExample(mlClass, fex.extractWordFeatures(j)));
      }
    }
  }
  // Consider only first and last word of a phrase as candidate
  else if (phraseBoundaries == "1")
  {
    // Extract features for first and last word of each phrase.

    for (size_t i = 0; i < sentences.size(); ++i)
    {
      LOG(INFO)
        << "Extracting features from sentence " << i << "\n";
      fex.newSentence(sentences[i].sentence);
      vector<Phrase<DefaultToken> *> const & phrases =
          sentences[i].sentence.getPhrases();
      vector<DefaultToken *> const & words =
          sentences[i].sentence.getWords();
      for (size_t j = 0; j < phrases.size(); ++j)
      {
        double mlClass = -1.0;
        DefaultToken const & firstToken =
            *words[phrases[j]->getWordsStartIndex()];
        DefaultToken const & lastToken = *words[phrases[j]->getWordsEndIndex()];
        // Classify as +1 if the tag we classify for is contained.
        if (firstToken.brTag.find(tagStr) != std::string::npos)
        {
          mlClass = +1.0;
        }
        mlExamples.push_back(MLExample(mlClass, fex.extractWordFeatures(
            phrases[j]->getWordsStartIndex())));

        // If the phrase consists of more than one word we also consider the
        // last word of the phrase.
        if (phrases[j]->getWords().size() > 1)
        {
          mlClass = -1.0;
          if (lastToken.brTag.find(tagStr) != std::string::npos)
          {
            mlClass = +1.0;
          }
          mlExamples.push_back(MLExample(mlClass, fex.extractWordFeatures(
              phrases[j]->getWordsEndIndex())));
        }
      }
    }
  }
  // Write the map if there were changes.
  fMap.writeFeatureMap();
  // Write the results.
  SVMLightExampleWriter svmLEW;
  svmLEW.writeMappedExamples(outputFile + ".features.mapped", fMap, mlExamples);
  svmLEW.writeExamples(outputFile + ".features", mlExamples);
  cout << "Extraction done. " << endl;
}
