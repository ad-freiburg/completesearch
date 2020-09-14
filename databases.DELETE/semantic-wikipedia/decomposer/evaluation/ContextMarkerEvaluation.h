// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_CONTEXTMARKEREVALUATION_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_CONTEXTMARKEREVALUATION_H_

#include <string>
#include <vector>
#include <set>
#include <utility>
#include "base/ContextDecomposerBase.h"
#include "base/ContextMarkerBase.h"
#include "./GroundTruthReader.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"


using std::string;

// This class currently only uses the DefaultToken class.
// It is already specific about the markings it expects
// in the ground-truth.
class ContextMarkerEvaluation
{
  public:
    // Carries the simple ints for the results.
    class TagResult
    {
      public:
        TagResult():correct(0), false_pos(0), false_neg(0)
        {}
        int correct;
        int false_pos;
        int false_neg;
    };

    // Result statistics for each marker.
    class MarkerResult
    {
      public:
      MarkerResult(): totalTokensCorrect(0), totalTokensinCorrect(0)
      {}
        int totalTokensCorrect;
        int totalTokensinCorrect;
        ad_utility::HashMap<int, TagResult > brTagResults;
        ad_utility::HashMap<int, TagResult > brClauseResults;
    };

    // Pass the data-dir of the POS tagger as well as a space-separated
    // string of all POS-tags that are compared for context equality.
    ContextMarkerEvaluation(
        std::vector<pair<string, ContextMarkerBase<DefaultToken> *> > markers,
        string const & inputFilename, string const & outputFilename);

    ~ContextMarkerEvaluation();

    // Evaluate a decomposer.
    // inputFilename is of the following format:
    // S1<TAB>Albert and Milena went to the movies.
    // P1<TAB>POSTAG POSTAG POSTAG POSTAG POSTAG POSTAG POSTAG
    // N1<TAB>NPTAG NPTAG NPTAG NPTAG NPTAG NPTAG NPTAG
    // K1<TAB>Albert went to the movies
    // K1<TAB>Milena went to the movies
    // S2<TAB>[next sentence]
    // outputFilename is of the following format:
    // S1<TAB>Albert and Milena went to the movies.
    // K1<TAB>RESULT<TAB>Albert went to the movies
    // K1<TAB>RESULT<TAB>Milena went to the movies
    // S2<TAB>[next sentence]
    // where RESULT is of the form
    //   true (the determined context is part of the ground truth)
    //   false (the determined context is not part of the ground truth)
    //   missing (this context was not part of the determined contexts)
    // If taggedInput is true POSTags and NPTags are required. Else
    // they will be read if they exist - but missing lines will not cause
    // an error.
    void evaluate();

    static const int CORRECT = 0;
    static const int FALSE_POS = 1;
    static const int FALSE_NEG = 2;

  private:

    vector<DefaultTokenMarks::TypeEnum> _allBrTags;
    vector<pair<DefaultTokenMarks::TypeEnum, DefaultTokenMarks::TypeEnum> >
    _allBrPairs;

    std::vector<pair<string, ContextMarkerBase<DefaultToken> *> > _markers;
    string _inputFilename;
    string _outputFilename;
    // Write results to output file.
    void writeResultFile(boost::ptr_vector<
      GroundTruthSentence<DefaultToken> > const & correctSentences,
      boost::ptr_vector<boost::ptr_vector<GroundTruthSentence<DefaultToken> > >
        const & markerSentences, vector<MarkerResult> & results);

    // Evaluate the actual versus the expected contexts received
    // via decomposition and provide the results in contextsResult
    // of the passed GroundTruthSentence.
    void evaluateResult(boost::ptr_vector<
      GroundTruthSentence<DefaultToken> > const & correctSentences,
      boost::ptr_vector<boost::ptr_vector<GroundTruthSentence<DefaultToken> > >
      const & markerSentences, vector<MarkerResult> * results);
    void getConstituents(vector<pair<size_t, size_t> > * constituents,
        DefaultTokenMarks::TypeEnum startMark,
        DefaultTokenMarks::TypeEnum endMark,
        Sentence<DefaultToken> const & sentence);
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_CONTEXTMARKEREVALUATION_H_
