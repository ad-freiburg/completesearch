// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTMARKERBASE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTMARKERBASE_H_

#include <vector>
#include <string>
#include "sentence/Sentence.h"
#include "base/ContextDecomposerBase.h"

class StatisticsProviderBase
{
  public:
    virtual ~StatisticsProviderBase()
    {
    }
    virtual std::string statsAsString()
    {
      return "";
    }
};

// Base class for marking up information for context extraction
// in a sentence using the provided markers.
// This includes marking of relative clauses, lists and
// separators.
template <class Token>
class ContextMarkerBase : public StatisticsProviderBase
{
  public:
    // Mark the sentence.
    virtual void markSentence(Sentence<Token> * sentence) = 0;
    virtual ~ContextMarkerBase()
    {}
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTMARKERBASE_H_
