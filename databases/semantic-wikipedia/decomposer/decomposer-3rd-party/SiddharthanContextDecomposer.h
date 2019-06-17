// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Hannah Bast <bast>, Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_3RD_PARTY_SIDDHARTHANCONTEXTDECOMPOSER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_3RD_PARTY_SIDDHARTHANCONTEXTDECOMPOSER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include <hash_set>
#include "./sentence/Sentence.h"
#include "./ContextDecomposerBase.h"

// Main class for chunking. Sequentially reads the index file, builds the
// sentence structure and extracts facts.
class SiddharthanContextDecomposer: public ContextDecomposerBase
{
  public:
    enum Mark
    {
      NONE,
      COMMA,
      INT_COMMA,
      NR_REL_START,
      NR_REL_END,
      R_REL_START,
      R_REL_END,
      APPOS,
      INT_CLAUSE,
      RED,
      VIO,
      REL,
      LIST,
      SEP,
      IGNORE
    };
    explicit SiddharthanContextDecomposer(bool writeToStdOut);
    virtual ~SiddharthanContextDecomposer();
    virtual std::vector<ContextPtr> const decomposeToPtr(
        Sentence & sentence);
    virtual std::vector<Context> const
        decompose(Sentence & sentence);
    // the trivial chunking along boundaries
    void
        markNonRestrictiveRelativeClausesStart(
            Sentence & sentence);
    size_t decideNonRestrictiveRelativeClauseEnd(
        Sentence& sentence, size_t nrRelClauseStart);
    void markRestrictiveRelativeClausesStart(
        Sentence& sentence);
    size_t decideRestrictiveRelativeClauseEnd(
        Sentence& sentence, size_t rRelClauseStart);
    // mark all commas in the sentence
    void markCommas(Sentence & sentence);
    void createSyntacticChunks(Sentence & sentence);
    void createSemanticRELChunks(Sentence & sentence);
    void createSemanticChunks(Sentence & sentence);
    void markSEPandLIST(Sentence & sentence);

    // print the chunks
    void printMarkedSentence(Sentence const & sentence);
    void printMark(Mark mark);
    void printChunks(std::vector <SyntacticChunk *> const & chunks,
        bool printGroups = false) const;
    void printChunks(boost::ptr_vector <SemanticChunk> const & chunks,
        bool printGroups = false) const;
  private:
    // a vector of markerss
    std::vector <std::pair <Mark, size_t> > _markers;
    // a vector with a marker for each phrase
    std::vector <Mark> _phraseMarkerVector;
    // a vector with a marker for each chunk
    std::vector <Mark> _chunkMarkerVector;
    // do we write to stdout
    bool _writeToStdOut;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_3RD_PARTY_SIDDHARTHANCONTEXTDECOMPOSER_H_
