// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTEXTRACTORBASE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTEXTRACTORBASE_H_

#include <vector>
#include <string>
#include "sentence/Sentence.h"
#include "sentence/Context.h"

namespace ad_decompose
{
// Class for extracting contexts from a sentence. The
// sentence already contains markings, relevant for
// context extraction.
template <class Token>
class ContextRecombinerBase
{
  public:
    virtual ~ContextRecombinerBase()
    {
    }
    // Extract contexts from the previously marked sentence,
    // but only return pointers
    // to the tokens for efficiency reasons.
    virtual Contexts<Token> const
        extractContextsPtr(Sentence<Token> & sentence) = 0;
    // Extract from the previously constructedand identified sentence and just
    // return a set of strings instead of the wrapped contexts.
    virtual std::vector<string> const extractContexts(
        Sentence<Token> & sentence)
    {
      std::vector<string> result;
      return result;
    }
};
}

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTEXTRACTORBASE_H_
