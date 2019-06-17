// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTWRITERBASE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTWRITERBASE_H_

#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <string>
#include <hash_set>
#include "sentence/Sentence.h"
#include "sentence/Context.h"

namespace ad_decompose
{
// Abstract superclass for all writers.
template <class Token>
class ContextWriterBase
{
  public:
    virtual ~ContextWriterBase() {}
    // Write the contexts of a given sentence.
    virtual void writeContexts(Contexts<Token> const & contexts,
        Sentence<Token> const & sentence) const = 0;
    // A function to write a very long sentence that should
    // not be decomposed because of that.
    virtual void writeLongSentence(Sentence<Token> const & sentence) const = 0;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_CONTEXTWRITERBASE_H_
