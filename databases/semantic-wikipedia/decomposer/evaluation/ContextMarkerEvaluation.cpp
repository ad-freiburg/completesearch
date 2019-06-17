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
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "./ContextMarkerEvaluation.h"
#include "util/ContextDecomposerUtil.h"
#include "./GroundTruthReader.h"

// printf macros for size_t, in the style of inttypes.h
#ifdef _LP64
#define __PRIS_PREFIX "z"
#else
#define __PRIS_PREFIX
#endif

// Use these macros after a % in a printf format string
// to get correct 32/64 bit behavior, like this:
// size_t size = records.size();
// printf("%"PRIuS"\n", size);

#define PRIdS __PRIS_PREFIX "d"
#define PRIxS __PRIS_PREFIX "x"
#define PRIuS __PRIS_PREFIX "u"
#define PRIXS __PRIS_PREFIX "X"
#define PRIoS __PRIS_PREFIX "o"

using std::string;
using std::cerr;
using std::make_pair;
using std::pair;
using std::vector;

// _____________________________________________________________________________
ContextMarkerEvaluation::ContextMarkerEvaluation(std::vector<pair<string,
    ContextMarkerBase<DefaultToken> *> > markers, string const & inputFilename,
    string const & outputFilename) :
  _markers(markers), _inputFilename(inputFilename), _outputFilename(
      outputFilename)
{
  _allBrTags.push_back(DefaultTokenMarks::LIT_OPEN);
  _allBrTags.push_back(DefaultTokenMarks::LIT_CLOSE);
  _allBrTags.push_back(DefaultTokenMarks::REL_OPEN);
  _allBrTags.push_back(DefaultTokenMarks::REL_CLOSE);
  _allBrTags.push_back(DefaultTokenMarks::RELA_OPEN);
  _allBrTags.push_back(DefaultTokenMarks::RELA_CLOSE);
  _allBrTags.push_back(DefaultTokenMarks::SEP);

  _allBrPairs.push_back(make_pair(DefaultTokenMarks::LIT_OPEN,
      DefaultTokenMarks::LIT_CLOSE));
  _allBrPairs.push_back(make_pair(DefaultTokenMarks::REL_OPEN,
      DefaultTokenMarks::REL_CLOSE));
  _allBrPairs.push_back(make_pair(DefaultTokenMarks::RELA_OPEN,
      DefaultTokenMarks::RELA_CLOSE));
}

// _____________________________________________________________________________
ContextMarkerEvaluation::~ContextMarkerEvaluation()
{
}

// _____________________________________________________________________________
void ContextMarkerEvaluation::evaluate()
{
  GroundTruthReader<DefaultToken> gtReader;
  boost::ptr_vector<boost::ptr_vector<GroundTruthSentence<DefaultToken> > >
    markerSentences;
  size_t nSentences = 0;
  boost::ptr_vector<GroundTruthSentence<DefaultToken> > correctSentences;
  vector<MarkerResult> results;
  gtReader.readGroundTruthsFromFile(&correctSentences, _inputFilename, true,
      false);
  for (size_t i = 0; i < _markers.size(); ++i)
  {
    MarkerResult res;
    results.push_back(res);
    markerSentences.push_back(
        new boost::ptr_vector<GroundTruthSentence<DefaultToken> >);
    gtReader.readGroundTruthsFromFile(&markerSentences.back(), _inputFilename,
        true, true);
    nSentences = markerSentences.back().size();
  }

  for (size_t i = 0; i < nSentences; ++i)
  {
    std::cout << markerSentences[0][i].sentence.asStringWithPhrases();
    for (size_t m = 0; m < _markers.size(); ++m)
    {
      _markers[m].second->markSentence(&(markerSentences[m][i].sentence));
    }
  }

  evaluateResult(correctSentences, markerSentences, &results);
  writeResultFile(correctSentences, markerSentences, results);
}

// _____________________________________________________________________________
void ContextMarkerEvaluation::evaluateResult(boost::ptr_vector<
    GroundTruthSentence<DefaultToken> > const & correctSentences,
    boost::ptr_vector<boost::ptr_vector<GroundTruthSentence<DefaultToken> > >
    const & markerSentences, vector<MarkerResult> * results)
{
  // First the overall equality.

  // For each sentence.
  for (size_t s = 0; s < correctSentences.size(); ++s)
  {
    // For each token.
    for (size_t t = 0; t < correctSentences[s].sentenceTokens.size(); ++t)
    {
      // For each marker.
      for (size_t m = 0; m < markerSentences.size(); ++m)
      {
        std::set<DefaultTokenMarks::TypeEnum> actual;
        actual.insert(markerSentences[m][s].sentenceTokens[t]->marks.begin(),
            markerSentences[m][s].sentenceTokens[t]->marks.end());
        std::set<DefaultTokenMarks::TypeEnum> expected;
        expected.insert(correctSentences[s].sentenceTokens[t]->marks.begin(),
            correctSentences[s].sentenceTokens[t]->marks.end());
        if (actual == expected)
          (*results)[m].totalTokensCorrect++;
        else
          (*results)[m].totalTokensinCorrect++;
      }
    }
  }

  // Now ordered by br tag.
  // For each sentence.
  for (size_t s = 0; s < correctSentences.size(); ++s)
  {
    // For each token.
    for (size_t t = 0; t < correctSentences[s].sentenceTokens.size(); ++t)
    {
      // For each marker.
      for (size_t m = 0; m < markerSentences.size(); ++m)
      {
        for (size_t br = 0; br < _allBrTags.size(); ++br)
        {
          int actualCount = std::count(
              markerSentences[m][s].sentenceTokens[t]->marks.begin(),
              markerSentences[m][s].sentenceTokens[t]->marks.end(),
              _allBrTags[br]);
          int expectedCount = std::count(
              correctSentences[s].sentenceTokens[t]->marks.begin(),
              correctSentences[s].sentenceTokens[t]->marks.end(),
              _allBrTags[br]);
          // compute difference actualnum - expectednum
          // if difference < 0 add abs to false-negative
          if (actualCount - expectedCount < 0)
          {
            (*results)[m].brTagResults[_allBrTags[br]].false_neg += -1
                * (actualCount - expectedCount);
          }
          // if difference > 0 add abs to false-positive
          else if (actualCount - expectedCount > 0)
          {
            (*results)[m].brTagResults[_allBrTags[br]].false_pos
                += (actualCount - expectedCount);
          }
          // add smaller number of actual and expected tags set to correct
          // number for this tag
          (*results)[m].brTagResults[_allBrTags[br]].correct += std::min(
              actualCount, expectedCount);
        }
      }
    }
  }

  // For each sentence.
  for (size_t s = 0; s < correctSentences.size(); ++s)
  {
    // correctSentences[s].sentence.printSentence();
    // For each brTag-pair
    for (size_t br = 0; br < _allBrPairs.size(); ++br)
    {
      // Collect pairs of brackets in ground truth
      vector<pair<size_t, size_t> > expectedConstituents;
      getConstituents(&expectedConstituents, _allBrPairs[br].first,
          _allBrPairs[br].second, correctSentences[s].sentence);
      // For each marker
      for (size_t m = 0; m < markerSentences.size(); ++m)
      {
        vector<pair<size_t, size_t> > exp = expectedConstituents;
        vector<pair<size_t, size_t> > actualConstituents;
        getConstituents(&actualConstituents, _allBrPairs[br].first,
            _allBrPairs[br].second, markerSentences[m][s].sentence);
        for (size_t a = 0; a < actualConstituents.size(); ++a)
        {
          vector<pair<size_t, size_t> >::iterator it;
          // For each pair check if it is in expected set
          // if it is remove from expected
          if ((it = find(exp.begin(), exp.end(), actualConstituents[a]))
              != exp.end())
          {
            exp.erase(it);
            (*results)[m].brClauseResults[_allBrPairs[br].first].correct++;
          }
          // if it is not add false-positive
          else
          {
            (*results)[m].brClauseResults[_allBrPairs[br].first].false_pos++;
          }
        }
        // whatever remains in expected is false-negative
        (*results)[m].brClauseResults[_allBrPairs[br].first].false_neg
            += exp.size();
      }
    }
  }
}

// _____________________________________________________________________________
void ContextMarkerEvaluation::getConstituents(
    vector<pair<size_t, size_t> > * constituents,
    DefaultTokenMarks::TypeEnum startMark, DefaultTokenMarks::TypeEnum endMark,
    Sentence<DefaultToken> const & sentence)
{
  vector<size_t> starts;
  vector<DefaultToken *> const words = sentence.getWords();
  for (size_t i = 0; i < words.size(); ++i)
  {
    for (size_t m = 0; m < words[i]->marks.size(); ++m)
    {
      if (words[i]->marks[m] == startMark)
        starts.push_back(i);
      else if (words[i]->marks[m] == endMark)
      {
        constituents->push_back(make_pair(starts.back(), i));
        starts.erase(starts.end() - 1);
      }
    }
  }
}

// _____________________________________________________________________________
void ContextMarkerEvaluation::writeResultFile(boost::ptr_vector<
    GroundTruthSentence<DefaultToken> > const & correctSentences,
    boost::ptr_vector<boost::ptr_vector<GroundTruthSentence<DefaultToken> > >
    const & markerSentences, vector<MarkerResult> & results)
{
  FILE* outputFile = fopen(_outputFilename.c_str(), "w");
  time_t rawtime;
  time(&rawtime);
  char buf[30];
  ctime_r(&rawtime, buf);

  fprintf(outputFile, "## ContextMarker Evaluation, %s", buf);

  for (size_t s = 0; s < correctSentences.size(); ++s)
  {
    fprintf(outputFile, "S%"PRIuS"\t\t%s\n", s + 1,
        ContextDecomposerUtil::tokenVectorToString(
            correctSentences[s].sentenceTokens).c_str());
    // Output golden marks.
    fprintf(outputFile, "%-10s\t", "GoldMarks");
    for (size_t t = 0; t < correctSentences[s].sentenceTokens.size(); ++t)
    {
      fprintf(outputFile, "%s ",
          (correctSentences[s].sentenceTokens[t]->brTag).c_str());
    }
    fprintf(outputFile, "\n");

    // Output result of each marker.
    for (size_t m = 0; m < _markers.size(); ++m)
    {
      fprintf(outputFile, "%-10s\t", (_markers[m].first).c_str());
      for (size_t t = 0; t < correctSentences[s].sentenceTokens.size(); ++t)
      {
        fprintf(outputFile, "%s ",
            (markerSentences[m][s].sentenceTokens[t]->brTag).c_str());
      }
      fprintf(outputFile, "\n");
    }
    fprintf(outputFile, "\n");
  }

  // Output summary.
  for (size_t m = 0; m < _markers.size(); ++m)
  {
    fprintf(outputFile, "----------------------------------------------------"
      "----------------------------\n");
    fprintf(outputFile, "--- Summary for %s\n", _markers[m].first.c_str());
    fprintf(outputFile, "----------------------------------------------------"
      "----------------------------\n\n");
    fprintf(outputFile, "Total Tokens marked correctly:  \t%i\n",
        results[m].totalTokensCorrect);
    fprintf(outputFile, "Total Tokens marked incorrectly:\t%i\n",
        results[m].totalTokensinCorrect);
    fprintf(outputFile, "\n");
    fprintf(outputFile, "Single Tag Results:\n\n");
    for (size_t br = 0; br < _allBrTags.size(); ++br)
    {
      int correct = results[m].brTagResults[_allBrTags[br]].correct;
      int false_pos = results[m].brTagResults[_allBrTags[br]].false_pos;
      int false_neg = results[m].brTagResults[_allBrTags[br]].false_neg;

      float precision = ContextDecomposerUtil::computePrecision(correct,
          false_pos);
      float recall = ContextDecomposerUtil::computeRecall(correct, false_neg);
      float f_measure = ContextDecomposerUtil::computeFMeasure(precision,
          recall);
      fprintf(outputFile, "\t%s:\n", (DefaultToken::markToString(
          _allBrTags[br])).c_str());
      fprintf(outputFile, "\t\tcorrect:\t%i\n", correct);
      fprintf(outputFile, "\t\tfalse-positive:\t%i\n", false_pos);
      fprintf(outputFile, "\t\tfalse-negative:\t%i\n", false_neg);
      fprintf(outputFile, "\t\tprecision:\t%.5f\n", precision);
      fprintf(outputFile, "\t\trecall:\t%.5f\n", recall);
      fprintf(outputFile, "\t\tf-measure:\t%.5f\n", f_measure);
      fprintf(outputFile, "\n");
    }
    fprintf(outputFile, "\n");
    fprintf(outputFile, "Pair Results:\n");
    for (size_t br = 0; br < _allBrPairs.size(); ++br)
    {
      int correct = results[m].brClauseResults[_allBrPairs[br].first].correct;
      int false_pos =
          results[m].brClauseResults[_allBrPairs[br].first].false_pos;
      int false_neg =
          results[m].brClauseResults[_allBrPairs[br].first].false_neg;

      float precision = ContextDecomposerUtil::computePrecision(correct,
          false_pos);
      float recall = ContextDecomposerUtil::computeRecall(correct, false_neg);
      float f_measure = ContextDecomposerUtil::computeFMeasure(precision,
          recall);

      fprintf(outputFile, "\t%s:\n", ((DefaultToken::markToString(
          _allBrPairs[br].first)) + "-" + (DefaultToken::markToString(
          _allBrPairs[br].second))).c_str());
      fprintf(outputFile, "\t\tcorrect:\t%i\n", correct);
      fprintf(outputFile, "\t\tfalse-positive:\t%i\n", false_pos);
      fprintf(outputFile, "\t\tfalse-negative:\t%i\n", false_neg);
      fprintf(outputFile, "\t\tprecision:\t%.5f\n", precision);
      fprintf(outputFile, "\t\trecall:\t%.5f\n", recall);
      fprintf(outputFile, "\t\tf-measure:\t%.5f\n", f_measure);
      fprintf(outputFile, "\n");
    }
    fprintf(outputFile, "\n\n");
  }

  fclose(outputFile);
}

