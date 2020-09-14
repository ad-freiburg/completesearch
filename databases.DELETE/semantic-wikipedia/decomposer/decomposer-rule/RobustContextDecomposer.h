// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_ROBUSTCONTEXTDECOMPOSER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_ROBUSTCONTEXTDECOMPOSER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include "sentence/Sentence.h"
#include "base/ContextDecomposerBase.h"

// Main class for chunking. Sequentially reads the index file, builds the
// sentence structure and extracts facts.
class RobustContextDecomposer: public ContextDecomposerBase
{
  public:
    enum Mark
    {
      IGNORE,
      NONE,
      LIST,
      LISTV,
      SEP,
      REL,
      RELE,
      RED,
      VIO
    };
    explicit RobustContextDecomposer(bool writeToStdOut);
    virtual ~RobustContextDecomposer();
    virtual std::vector<ContextPtr> const decomposeToPtr(
        Sentence & sentence);
    virtual std::vector<Context> const
    decompose(Sentence & sentence);
    // the trivial chunking along boundaries
    void markRelativeClauses(Sentence & sentence);
    void markCommaAndVPs(Sentence & sentence);
    void correctLISTtoREL(Sentence & sentence);
    void extractAppositiveRELs(Sentence & sentence);
    void extractRELs(Sentence & sentence);
    void correctLISTtoRELbw(Sentence & sentence);
    void createSyntacticChunks(Sentence & sentence);
    void createSemanticRELChunks(Sentence & sentence);
    void createSemanticChunks(Sentence & sentence);
    void correctSEPtoREL(Sentence & sentence);
    void markSEPandLIST(Sentence & sentence);
    // print the chunks
    void printMarkedSentence(Sentence const & sentence);
    void printMark(Mark mark);
    void printChunks(std::vector <SyntacticChunk *> const & chunks,
        bool printGroups = false) const;
    void printChunks(boost::ptr_vector <SemanticChunk> const & chunks,
        bool printGroups = false) const;
  private:
    std::vector <std::pair <Mark, size_t> > _markers;
    // a vector telling using the same index as phrases
    std::vector <Mark> _markerVector;
    std::vector <Mark> _chunkMarkerVector;
    bool _writeToStdOut;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_ROBUSTCONTEXTDECOMPOSER_H_ // NOLINT
