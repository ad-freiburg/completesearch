// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_SIMPLECONTEXTDECOMPOSER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_SIMPLECONTEXTDECOMPOSER_H_

#include <gtest/gtest.h>
#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <string>
#include <hash_set>
#include "sentence/Sentence.h"
#include "base/ContextDecomposerBase.h"

// Main class for chunking. Sequentially reads the index file, builds the
// sentence structure and extracts facts.
class SimpleContextDecomposer: public ContextDecomposerBase
{
  public:
    SimpleContextDecomposer();
    virtual ~SimpleContextDecomposer();
    // Extract facts from the provided sentence.
    int extractFacts(Sentence const & sentence) const;
    virtual std::vector<Context> const decompose(
        Sentence & sentence);
    virtual std::vector<ContextPtr> const
        decomposeToPtr(Sentence & sentence);
    // the trivial chunking along boundaries
    void simpleChunkSentence(Sentence & sentence);
    // mark the enumerations in the sentence
    void markEnumerations(Sentence & sentence);
    // remove the relative clauses from the sentence
    void extractRelativeClauses(Sentence & sentence);
    // remove the apposition clauses from the sentence
    void extractAppositions(Sentence & sentence);
    // identify and combine the phrasal groups in the chunks
    void identifyAndCombinePhraseGroups(Sentence & sentence);
    void contractConjunctions(Sentence & sentence);
    // contract other sayings like "in which" or "just as" ...
    void contractOtherCollocations(Sentence & sentence);
    // print the chunks
    void printChunks(std::vector <SyntacticChunk *> const & chunks,
        bool printGroups = false) const;
    void printChunks(boost::ptr_vector <SemanticChunk> const & chunks,
        bool printGroups = false) const;
  private:
    std::vector <boost::regex> _factRegexes;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_SIMPLECONTEXTDECOMPOSER_H_ // NOLINT
