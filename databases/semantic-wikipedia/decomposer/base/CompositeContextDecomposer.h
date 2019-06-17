  // Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_COMPOSITECONTEXTDECOMPOSER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_COMPOSITECONTEXTDECOMPOSER_H_

#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include "sentence/Sentence.h"
#include "base/ContextDecomposerBase.h"
#include "base/ContextExtractorBase.h"
#include "base/ContextMarkerBase.h"

namespace ad_decompose
{
template <class Token>
class CompositeContextDecomposer: public ContextDecomposerBase<Token>
{
  public:
    static const size_t MAX_DECOMPOSER_SENTENCE_SIZE = 150;

    explicit CompositeContextDecomposer();
    virtual ~CompositeContextDecomposer();
    virtual Contexts<Token> const decomposeToPtr(
        Sentence<Token> & sentence);
    // Set the ContextMarker to use.
    void setContextMarker(ContextMarkerBase<Token> * marker);
    // Set the ContextExtractor to use.
    void setContextExtractor(ContextRecombinerBase<Token> * extractor);
  private:

    ContextMarkerBase<Token> * _marker;
    ContextRecombinerBase<Token> * _extractor;
    // Counters to keep track.
    mutable int _nIgnoredSentences;
    mutable int _nTotalSentences;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_COMPOSITECONTEXTDECOMPOSER_H_
