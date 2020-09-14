// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_CONTEXTDECOMPOSEREVALUATION_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_CONTEXTDECOMPOSEREVALUATION_H_

#include <string>
#include <vector>
#include <set>
#include <utility>
#include "base/ContextDecomposerBase.h"
#include "util/GPOSTagger.h"
#include "./GroundTruthReader.h"

class ContextEquality
{
  public:
    explicit ContextEquality(std::string const & equalityTags);
    // Return true if the two contexts are identical, false otherwise.
    // The definition of identical is:
    bool equals(Context const & a, Context const & b) const;

    // Calculate the Jaccard distance between two contexts
    float jaccardDistance(Context const & a, Context const & b);


  private:
    std::set<string> _whitePosTagsSet;
};

template <class Token>
class ContextDecomposerEvaluation
{
  public:
    // Pass the data-dir of the POS tagger as well as a space-separated
    // string of all POS-tags that are compared for context equality.
    ContextDecomposerEvaluation(std::string const & gposDataDir,
        std::string const & contextEqualityPosTags) :
      _postagger(new GPOSTagger(gposDataDir)), _contextEqualityPosTags(
          contextEqualityPosTags)
    {
    }

    ~ContextDecomposerEvaluation();

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
    void evaluate(ContextDecomposerBase<Token> & decomposer,
        string const & inputFilename, string const & outputFilename,
        bool taggedInput, bool ignoreBrTag);

  private:

    // The POS-tagger.
    GPOSTagger * _postagger;

    // Write results to output file.
    void writeResultFile(
        boost::ptr_vector<GroundTruthSentence<Token> > const & sentences,
        string const & fileName, string const & className);

    // Evaluate the actual versus the expected contexts received
    // via decomposition and provide the results in contextsResult
    // of the passed GroundTruthSentence.
    void evaluateResult(GroundTruthSentence<Token> & sentence);



    // A string containing the equality POS tags.
    std::string _contextEqualityPosTags;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_CONTEXTDECOMPOSEREVALUATION_H_ // NOLINT
