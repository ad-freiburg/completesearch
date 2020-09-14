// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_GROUNDTRUTHREADER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_GROUNDTRUTHREADER_H_

#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <map>
#include <string>
#include "base/ContextDecomposerBase.h"
#include "sentence/Sentence.h"

using std::pair;

namespace ad_decompose
{
// A simple structure to contain a ground truth sentence.
template <class Token>
class GroundTruthSentence
{
  public:
    enum ContextResult
    {
      TRUE, FALSE_POS, FALSE_NEG
    };
    // GroundTruthSentence();
    // ~GroundTruthSentence();
    // The number of the sentence as given in the input file.
    uint32_t sentenceNumber;
    // These are the tokens of the sentence.
    std::vector<Token *> sentenceTokens;
    // This is the sentence structure.
    Sentence<Token> sentence;
    // These are the expected contexts for the sentence.
    Contexts<Token> contextsExpected;
    // These are the actual contexts for the sentence.
    Contexts<Token> contextsActual;
    // These are results of evaluating actual against expected contexts.
    std::vector<pair<Context<Token>, ContextResult> > contextResults;
    // For each of the actual results store the best similarity to the
    // expected results.
    std::vector<float> contextResultSimilarity;
};

// Read a ground truth from input file and return a vector of all
// ground truth sentences.
template <class Token>
class GroundTruthReader
{
  public:
    // Read the ground truth file and put results into _groundTruthSentences.
    // If tagsRequired is true the provided file in fileName needs to include
    // POS as well as NP-tags for each sentence to decompose.
    void readGroundTruthsFromFile(
        boost::ptr_vector <GroundTruthSentence<Token> > * groundTruth,
        string const & fileName, bool tagsRequired = false, bool ignoreBrTag =
            false) const;
  private:
    // Check if the line is of expected format, bail out if it isn't.
    void verifyLineFormat(string const & line) const;
    // Return tokens from a single line. Performs line validation etc.
    // Validate that numTokens tokens are returned.
    // Pass 0 as numTokens to ignore the number of tokens returned.
    std::vector<string> getTokensFromLine(string const & line,
        uint32_t numTokens, int sentenceNumber) const;
    void
    verifyLineNumber(string const & numberStr, uint32_t sentenceNumber) const;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_EVALUATION_GROUNDTRUTHREADER_H_
