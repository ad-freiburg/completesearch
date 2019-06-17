// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <time.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <utility>
#include <string>

#include "util/GPOSTagger.h"
#include "./ContextDecomposerEvaluation.h"
#include "util/ContextDecomposerUtil.h"

using std::string;
using std::cerr;
using std::vector;

// _____________________________________________________________________________
template <class Token>
ContextDecomposerEvaluation<Token>::~ContextDecomposerEvaluation()
{
  delete _postagger;
}

// _____________________________________________________________________________
template <class Token>
void ContextDecomposerEvaluation<Token>::evaluate(
    ContextDecomposerBase<Token> & decomposer,
    string const & inputFileName, string const & outputFileName,
    bool taggedInput, bool ignoreBrTag)
{
  assert(inputFileName.size() > 0);
  assert(outputFileName.size() > 0);
  boost::ptr_vector<GroundTruthSentence<Token> > groundTruth;
  GroundTruthReader<Token> gtReader;
  // Read the ground truth file and put results into _groundTruthSentences.
  gtReader.readGroundTruthsFromFile(&groundTruth, inputFileName, taggedInput,
      ignoreBrTag);
  std::cout << "Evaluating " << groundTruth.size() << " sentences."
      << std::endl;
  // Process the ground truths. Create the contexts.
  for (size_t i = 0; i < groundTruth.size(); ++i)
  {
    groundTruth[i].contextsActual = decomposer.decompose(
        groundTruth[i].sentence);
    // Evaluate by comparing actual against expected contexts.
    evaluateResult(groundTruth[i]);
  }
  // Hack to get class name demangled.
  int status;
  char *decomposerName = abi::__cxa_demangle(typeid(decomposer).name(), 0, 0,
      &status);
  std::cout << "Done parsing sentences.\n";
  // Write the result to the output file.
  writeResultFile(groundTruth, outputFileName, decomposerName);
  groundTruth.clear();
}

// _____________________________________________________________________________
template <class Token>
void ContextDecomposerEvaluation<Token>::evaluateResult(
    GroundTruthSentence<Token> & sentence)
{
  ContextEquality eq(_contextEqualityPosTags);
  assert(sentence.contextsExpected.size()>0);
  assert(sentence.sentenceTokens.size()>0);
  // A POS tagger to POS-tag the expected contexts.


  // Work on copy of the expected and actual results
  std::vector<Context> actual = sentence.contextsActual;
  std::vector<Context> expected = sentence.contextsExpected;

  std::for_each(actual.begin(), actual.end(), *_postagger);
  std::for_each(expected.begin(), expected.end(), *_postagger);

  // Iterate through the actual results and decide whether it is part of
  // the expected results. If we find an expected result we delete it
  // from the list of expected results.
  for (size_t a = 0; a < actual.size(); ++a)
  {
    for (size_t e = 0; e < expected.size(); ++e)
    {
      if (eq.equals(actual[a], expected[e]))
      {
        sentence.contextResults.push_back(pair<Context,
            typename GroundTruthSentence<Token>::ContextResult> (actual[a],
            GroundTruthSentence<Token>::TRUE));
        sentence.contextResultSimilarity.push_back(0.0);
        // This has been matched, erase it.
        expected.erase(expected.begin() + e);
        actual.erase(actual.begin() + a);
        a--;
        break;
      }
    }
  }

  // Each of the remaining contexts is either false-positive or
  // false negative. Prefill the similarities with 1.0.
  sentence.contextResultSimilarity.insert(
      sentence.contextResultSimilarity.end(),
      actual.size() + expected.size(), 1.0);

  // Offset to the actual results in the similarity vector
  size_t offA = sentence.contextResults.size();
  // Offset to the expected results in the similarity vector
  size_t offE = actual.size();

  for (size_t a = 0; a < actual.size(); ++a)
  {
    // If we get here we didn't find the context in the expected ones.
    float dist;
    // Calculate the best jaccard distance
    for (size_t e = 0; e < expected.size(); ++e)
    {
      dist = eq.jaccardDistance(actual[a], expected[e]);
      if (dist < sentence.contextResultSimilarity[offA+a])
      {
        sentence.contextResultSimilarity[offA+a] = dist;
      }
      if (dist < sentence.contextResultSimilarity[offA+offE+e])
      {
        sentence.contextResultSimilarity[offA+offE+e] = dist;
      }
    }
    sentence.contextResults.push_back(pair<Context,
        typename GroundTruthSentence<Token>::ContextResult> (actual[a],
        GroundTruthSentence<Token>::FALSE_POS));
  }
  // All remaining results in expected are missing.
  for (size_t e = 0; e < expected.size(); ++e)
  {
    sentence.contextResults.push_back(pair<Context,
        typename GroundTruthSentence<Token>::ContextResult> (expected[e],
            GroundTruthSentence<Token>::FALSE_NEG));
  }
}

// _____________________________________________________________________________
ContextEquality::ContextEquality(std::string const & equalityTags)
{
  boost::split(_whitePosTagsSet, equalityTags, boost::is_any_of(" "));
}

float ContextEquality::jaccardDistance(Context const & a, Context const & b)
{
  std::set<string> aWords;
  std::set<string> bWords;
  std::vector<string> intersectAB;
  std::vector<string> unionAB;
  // std::cout << "Comparing: " << ContextDecomposerUtil::tokenVectorToString(a)
  //    << "\n";
  // std::cout << "against: " << ContextDecomposerUtil::tokenVectorToString(b)
  //    << "\n";

  for (size_t i = 0; i < a.size(); ++i)
  {
    if (_whitePosTagsSet.find((a[i]).posTag) != _whitePosTagsSet.end())
      aWords.insert((a[i]).tokenString);
    //  else {
    //    std::cout << "Ignoring " << a[i].tokenString << "_" << (a[i]).posTag
    // << "\n";
    //  }
  }

  for (size_t i = 0; i < b.size(); ++i)
  {
    if (_whitePosTagsSet.find((b[i]).posTag) != _whitePosTagsSet.end())
      bWords.insert((b[i]).tokenString);
  }

  std::set_intersection(aWords.begin(), aWords.end(), bWords.begin(),
      bWords.end(), std::inserter(intersectAB, intersectAB.end()));
  std::set_union(aWords.begin(), aWords.end(), bWords.begin(), bWords.end(),
      std::inserter(unionAB, unionAB.end()));

  float result = 1.0 - static_cast<float> (intersectAB.size())
      / static_cast<float> (unionAB.size());
  // std::cout << result << "\n";
  return result;
}

// _____________________________________________________________________________
bool ContextEquality::equals(Context const & a, Context const & b) const
{
  std::set<string> aWords;
  std::set<string> bWords;
  // std::cout << "Comparing: " << ContextDecomposerUtil::tokenVectorToString(a)
  //    << "\n";
  // std::cout << "against: " << ContextDecomposerUtil::tokenVectorToString(b)
  //    << "\n";

  for (size_t i = 0; i < a.size(); ++i)
  {
    if (_whitePosTagsSet.find((a[i]).posTag) != _whitePosTagsSet.end())
      aWords.insert((a[i]).tokenString);
    //  else {
    //    std::cout << "Ignoring " << a[i].tokenString << "_" << (a[i]).posTag
    // << "\n";
    //  }
  }

  for (size_t i = 0; i < b.size(); ++i)
  {
    if (_whitePosTagsSet.find((b[i]).posTag) != _whitePosTagsSet.end())
      bWords.insert((b[i]).tokenString);
  }

  std::set<string>::reverse_iterator rit;

  /*std::cout << "\naSet contains: ";
   for ( rit=aWords.rbegin() ; rit != aWords.rend(); rit++ )
   std::cout << " " << *rit;

   std::cout << "\nbSet contains: ";
   for ( rit=bWords.rbegin() ; rit != bWords.rend(); rit++ )
   std::cout << " " << *rit;

   std::cout << " Returning: " << (aWords == bWords ? "True" : "False") << "\n";*/
  return aWords == bWords;
}

// _____________________________________________________________________________
template <class Token>
void ContextDecomposerEvaluation<Token>::writeResultFile(boost::ptr_vector<
    GroundTruthSentence<Token> > const & gtSentences, string const & fileName,
    string const & className)
{
  FILE* outputFile = fopen(fileName.c_str(), "w");
  uint32_t totalTrue = 0;
  uint32_t totalFalse = 0;
  uint32_t totalMissing = 0;
  float dist_false_neg_sum = 0;
  float dist_false_pos_sum = 0;
  float n_false_pos_dist = 0;
  float n_false_neg_dist = 0;

  time_t rawtime;
  time(&rawtime);
  char buf[30];
  ctime_r(&rawtime, buf);
  fprintf(outputFile, "## ContextDecomposer Evaluation, %s", buf);
  fprintf(outputFile, "## Evaluating decomposer: %s\n\n", className.c_str());

  for (size_t i = 0; i < gtSentences.size(); ++i)
  {
    fprintf(outputFile, "S%i\t%s\n", gtSentences[i].sentenceNumber,
        ContextDecomposerUtil::tokenVectorToString(
            gtSentences[i].sentenceTokens).c_str());
    for (size_t j = 0; j < gtSentences[i].contextResults.size(); ++j)
    {
      Context context = gtSentences[i].contextResults[j].first;
      typename GroundTruthSentence<Token>::ContextResult result =
          gtSentences[i].contextResults[j].second;
      std::string resultString = "";
      switch (result)
      {
        case GroundTruthSentence<Token>::TRUE:
          resultString = "TRUE";
          totalTrue++;
          break;
        case GroundTruthSentence<Token>::FALSE_POS:
          resultString = "FALSE-POS";
          totalFalse++;
          dist_false_pos_sum += gtSentences[i].contextResultSimilarity[j];
          n_false_pos_dist++;
          break;
        case GroundTruthSentence<Token>::FALSE_NEG:
          resultString = "FALSE-NEG";
          totalMissing++;
          dist_false_neg_sum += gtSentences[i].contextResultSimilarity[j];
          n_false_neg_dist++;
          break;
        default:
          std::cerr << "Wrong result type: " << result << "\n";
          assert(false);
          break;
      }
      fprintf(outputFile, "C%i\t%s (%1.2f)\t%s\n",
          gtSentences[i].sentenceNumber, resultString.c_str(),
          gtSentences[i].contextResultSimilarity[j],
          ContextDecomposerUtil::tokenVectorToString(context).c_str());
    }
    fprintf(outputFile, "\n");
  }
  float avg_false_pos_distance = dist_false_pos_sum / n_false_pos_dist;
  float avg_false_neg_distance = dist_false_neg_sum / n_false_neg_dist;
  fprintf(outputFile, "\n");
  fprintf(outputFile, "## Total TRUE:   \t%3i\n", totalTrue);
  fprintf(outputFile,
      "## Total FALSE-POS:  \t%3i\tAverage Jaccard Distance: %1.3f\n",
      totalFalse, avg_false_pos_distance);
  fprintf(outputFile,
      "## Total FALSE-NEG:  \t%3i\tAverage Jaccard Distance: %1.3f\n",
      totalMissing, avg_false_neg_distance);
  fclose(outputFile);
}

template class ContextDecomposerEvaluation<DefaultToken>;
