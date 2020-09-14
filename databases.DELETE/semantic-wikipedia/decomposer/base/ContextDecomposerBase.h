// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTDECOMPOSERBASE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTDECOMPOSERBASE_H_

#include <vector>
#include <string>
#include "sentence/Sentence.h"
#include "sentence/Context.h"


namespace ad_decompose
{
// Abstract superclass for all decomposers.
// Decomposition consists of the tasks of
// marking and extraction.
template<class Token>
class ContextDecomposerBase
{
  public:
    virtual ~ContextDecomposerBase()
    {
    }
    // Decompose the previously constructed sentence but only return pointers
    // to the tokens for efficiency reasons.
    virtual Contexts<Token> const
    decomposeToPtr(Sentence<Token> & sentence) = 0;
    // Decompose the previously constructed sentence and just return
    // a set of strings instead of the wrapped contexts.
    virtual std::vector<string> const decompose(Sentence<Token> & sentence)
    {
      std::vector<string> result;
      return result;
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTDECOMPOSERBASE_H_
