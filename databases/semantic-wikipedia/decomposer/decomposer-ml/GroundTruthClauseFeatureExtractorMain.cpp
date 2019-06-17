// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <boost/ptr_container/ptr_vector.hpp>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include "evaluation/GroundTruthReader.h"
#include "decomposer-ml/FeatureExtractor.h"
#include "util/ContextDecomposerUtil.h"
#include "decomposer-ml/MLExampleWriter.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::string;
using std::endl;
using std::vector;
using std::flush;
using std::cout;
using std::pair;
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
    { "clauseType", required_argument, NULL, 'c' },
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
      << "\t --clauseType (-c): the br tag pair to classify for"
      << "(e.g. LIT, REL etc.)\n"
      << endl << "\t --fmap (-m): file to read the feature map from\n" << endl
      << "\t --feature_conf (-f): feature configuration file\n" << endl
      << "\t Optional options:\n" << endl
      << "\t --lockFmap (-l): 0|1 lock the feature map\n" << endl << flush;
}

void getCorrectAndIncorrectPairs(
    Sentence<DefaultToken> const & sentence, vector<pair<size_t,
    size_t> > * incorrect, vector<pair<size_t, size_t> > * correct,
    DefaultTokenMarks::TypeEnum openMark, DefaultTokenMarks::TypeEnum closeMark)
{
  vector<DefaultToken *> const & words = sentence.getWords();
  vector<size_t> correctOpens;
  vector<size_t> opens;
  vector<size_t> closes;
  for (size_t j = 0; j < words.size(); ++j)
  {
    for (size_t m = 0; m < words[j]->marks.size(); ++m)
    {
      DefaultTokenMarks::TypeEnum mark = words[j]->marks[m];
      // A seperator is neither open nor close.
      if (mark == DefaultTokenMarks::SEP)
        continue;
      if (DefaultToken::isOpenMark(mark))
      {
        // Avoid duplicate indices, if several marks open
        // here.
        if (opens.empty() || opens.back() != j)
          opens.push_back(j);
        if (mark == openMark)
        {
          correctOpens.push_back(j);
        }
      }
      else if (DefaultToken::isCloseMark(mark))
      {
        // Avoid duplicate indices, if several marks close
        // here.
        if (closes.empty() || closes.back() != j)
          closes.push_back(j);
        if (mark == closeMark)
        {
          assert(!correctOpens.empty());
          correct->push_back(std::make_pair(correctOpens.back(), j));
          correctOpens.erase(correctOpens.end() - 1);
        }
      }
    }
  }

//  boost::ptr_vector<Phrase> const & phrases = sentence.getPhrases();
//  //
//  for (size_t i = 0; i < phrases.size(); i += 3)
//  {
//    size_t open = phrases[i].getWordsStartIndex();
//    size_t end = phrases[i].getWordsEndIndex();
//    opens.push_back(open);
//    closes.push_back(end);
//  }

  // We have to open as often as we close and then all opens
  // are removed.
  assert(correctOpens.empty());

  for (size_t j = 0; j < opens.size(); ++j)
  {
    for (size_t k = 0; k < closes.size(); ++k)
    {
      if (opens[j] <= closes[k])
      {
        pair<size_t, size_t> thisPair = std::make_pair(opens[j], closes[k]);
        if (std::find(correct->begin(), correct->end(), thisPair)
            == correct->end())
          incorrect->push_back(thisPair);
      }
    }
  }
}

int main(int argc, char** argv)
{
  std::cout << std::endl << ad_utility::EMPH_ON
      << "Ground Truth Clause Feature Extractor, version " << __DATE__ << " "
      << __TIME__ << ad_utility::EMPH_OFF << std::endl;
  optind = 1;
  std::string clauseType = "";
  std::string featureMapFile = "";
  std::string featureConfigFile = "";
  bool lockFmap = false;

  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "c:m:f:l:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case('l'):
        if (string(optarg) == "1")
          lockFmap = true;
        break;
      case('f'):
        featureConfigFile = string(optarg);
        break;
      case('m'):
        featureMapFile = string(optarg);
        break;
      case('c'):
        clauseType = string(optarg);
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

  if (clauseType == "")
  {
    std::cout << "Missing clause type.";
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
  GroundTruthReader<DefaultToken> reader;

  if (featureMapFile == "")
  {
    std::cerr << "Missing feature map file in parameters\n";
  }

  FeatureMap fMap(featureMapFile, lockFmap);

  DefaultTokenMarks::TypeEnum openMark = DefaultTokenMarks::LIT_CLOSE;
  DefaultTokenMarks::TypeEnum closeMark = DefaultTokenMarks::LIT_CLOSE;
  if (clauseType == "LIT")
  {
    openMark = DefaultTokenMarks::LIT_OPEN;
    closeMark = DefaultTokenMarks::LIT_CLOSE;
  }
  else if (clauseType == "REL")
  {
    openMark = DefaultTokenMarks::REL_OPEN;
    closeMark = DefaultTokenMarks::REL_CLOSE;
  }
  else if (clauseType == "RELA")
  {
    openMark = DefaultTokenMarks::RELA_OPEN;
    closeMark = DefaultTokenMarks::RELA_CLOSE;
  }

  // Update the configuration with the passed parameter.
  FeatureExtractorConfig config = ContextDecomposerUtil::parseFeatureConfig(
      featureConfigFile);

  FeatureExtractor fex(&fMap, config, true, false);
  boost::ptr_vector<GroundTruthSentence<DefaultToken> > sentences;
  reader.readGroundTruthsFromFile(&sentences, groundTruthFile, true);
  std::vector<MLExample> mlExamples;

  for (size_t i = 0; i < sentences.size(); ++i)
  {
    LOG(INFO)
      << "Extracting features from sentence " << i << "\n";
    Sentence<DefaultToken> & sentence = sentences[i].sentence;
    std::vector<pair<size_t, size_t> > correct;
    std::vector<pair<size_t, size_t> > incorrect;
    // Get correct and incorrect start/end pairs
    getCorrectAndIncorrectPairs(sentence, &incorrect, &correct, openMark,
        closeMark);
//  std::cout << "Correct: " << correct.size() << "\n";
//      for (size_t j = 0; j < correct.size(); ++j)
//      {
//        std::cout << "pair: " << correct[j].first << ", "
//    << correct[j].second
//    << std::endl;
//      }
//      std::cout << "Incorrect: " << incorrect.size() << "\n";
//      for (size_t j = 0; j < incorrect.size(); ++j)
//      {
//        std::cout << "pair: " << incorrect[j].first << ", "
//     << incorrect[j].second
//     << std::endl;
//      }
    fex.newSentence(sentence);
    // Add all correct ones.
    for (size_t j = 0; j < correct.size(); ++j)
      mlExamples.push_back(MLExample(1.0, fex.extractClauseFeatures(
          correct[j].first, correct[j].second)));
    // Add all incorrect ones.
    for (size_t j = 0; j < incorrect.size(); ++j)
      mlExamples.push_back(MLExample(-1.0, fex.extractClauseFeatures(
          incorrect[j].first, incorrect[j].second)));
  }
  // Write the map if there were changes.
  fMap.writeFeatureMap();
  // Write the results.
  SVMLightExampleWriter svmLEW;
  svmLEW.writeMappedExamples(outputFile + ".features.mapped", fMap, mlExamples);
  svmLEW.writeExamples(outputFile + ".features", mlExamples);
  cout << "Extraction done. " << endl;
}
