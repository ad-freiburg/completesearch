// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLCONTEXTMARKER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLCONTEXTMARKER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include "sentence/Sentence.h"
#include "base/ContextMarkerBase.h"
#include "./LibSVMClassifier.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "./MLInference.h"


using std::string;
namespace ad_decompose
{
class MLContextMarker: public ContextMarkerBase<DefaultToken>
{
  public:
    // The marker class will first apply all the wordClassifiers in order
    // of increasing index.
    // Then each pair of wordOpenClassifiers and wordCloseClassifiers
    // at same, increasing index will be applied.
    // Thus they have to be the same amount.
    // Application of each classifier will always be on each word/phrase
    // of the whole sentence. The clauseClassifiers are then used for
    // inference.
    explicit MLContextMarker(bool writeToStdOut, bool onlyPhraseBoundaries,
        string wordClassifiers, string wordOpenClassifiers,
        string wordCloseClassifiers, string clauseClassifiers,
        string wordFeatureMapFile, string clauseFeatureMapFile,
        string featureConfig);
    virtual ~MLContextMarker();
    // Mark the sentence.
    void markSentence(Sentence<DefaultToken> * sentence);
  private:
    void printMarkedWords(Sentence<DefaultToken> const & sentence) const;
    std::string marksAsString(
        std::vector<DefaultTokenMarks::TypeEnum> & marks) const;
    bool _writeToStdOut;
    bool _onlyPhraseBoundaries;
    // Mark the sentence using the provided classifiers at all
    // phrase boundaries.
    void markSentenceAtPhraseBoundaries(
        Sentence<DefaultToken> * sentence) const;
    // Mark the sentence using the provided classifiers at all
    // words.
    void markSentenceAtWords(Sentence<DefaultToken> * sentence) const;
    // These will be applied on the first word of each phrase.
    std::vector<std::pair<string, SVMClassifier *> > _wordOpenClassifiers;
    // These will be applied on the last word of each phrase.
    std::vector<std::pair<string, SVMClassifier *> > _wordCloseClassifiers;
    // These will be applied on the first and last word of each phrase.
    std::vector<std::pair<string, SVMClassifier *> > _wordClassifiers;
    // std::vector<std::pair<string, SVMClassifier *> > ;
    mutable ad_utility::HashMap<string, SVMClassifier * > _clauseClassifiers;

    FeatureMap * _mfMap;
    FeatureMap * _cfMap;
    FeatureExtractor * _wordFex;
    FeatureExtractor * _clauseFex;

    // Go through the up to sentence and collect opened
    // and closed brackets. For each complete seperated
    // sentence call the inference method.
    void correctBracketing(Sentence<DefaultToken> * sentence) const;

    void classifyPhraseStartAndEnds(Sentence<DefaultToken> * sentence,
        SVMClassifier const & classifier, string mark) const;
    void classifyPhraseEnds(Sentence<DefaultToken> * sentence,
        SVMClassifier const & classifier, string mark) const;
    void classifyPhraseStarts(Sentence<DefaultToken> * sentence,
        SVMClassifier const & classifier, string mark) const;
    void classifyAllWords(Sentence<DefaultToken> * sentence,
        SVMClassifier const & classifier, string mark) const;

    // Infer the correct bracketing. The passed vectors
    // contain the word indice positions of the respective
    // open/close bracket. The method constructs all possible
    // combinations of bracketings and uses the MLInference class
    // which constructs a graph of it and solves it.
    // endIndex describes the last possible index all brackets
    // can close.
    void inferBracketing(Sentence<DefaultToken> * sentence,
        std::vector<size_t> const & rel_open,
        std::vector<size_t> const & rel_close,
        std::vector<size_t> const & lit_open,
        std::vector<size_t> const & lit_close,
        std::vector<size_t> const & rela_open,
        std::vector<size_t> const & rela_close) const;

    MLInference _inference;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_ML_MLCONTEXTMARKER_H_
