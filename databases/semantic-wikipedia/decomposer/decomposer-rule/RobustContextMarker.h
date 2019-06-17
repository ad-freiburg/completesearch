// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_ROBUSTCONTEXTMARKER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_ROBUSTCONTEXTMARKER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include "sentence/Sentence.h"
#include "base/ContextDecomposerBase.h"
#include "base/ContextMarkerBase.h"

class RobustContextMarker: public ContextMarkerBase<DefaultToken>
{
  public:
    static const bool EVAL_RUN = false;
    explicit RobustContextMarker(bool verbose);
    // Mark the sentence.
    void markSentence(Sentence<DefaultToken> * sentence);
    virtual ~RobustContextMarker();
    enum InternalMark
    {
      IGNORE,
      NONE,
      LIST,
      LISTV,
      SEP,
      REL,
      RELE,
      RELA,
      RED,
      VIO,
      CLOSE
    };

  private:
    // Mark opening "(" and closing ")" brackets
    // as relative clauses.
    void markBrackets(Sentence<DefaultToken> const & sentence);
    // Find matching endings to the discovered starts of
    // constituents.
    void markEndings(Sentence<DefaultToken> * sentence);
    // Mark non-restrictive relative clauses.
    void markRelativeClauses(Sentence<DefaultToken> const & sentence);
    // Mark commas and verb-phrases.
    void markCommaAndVPs(Sentence<DefaultToken> const & sentence);
    // Correct mistakes (LIST to REL)
    void correctLISTtoRELbw(Sentence<DefaultToken> const & sentence);
    // Mark SEPs and LISTs.
    void markSEPandLIST(Sentence<DefaultToken> const & sentence);
    // Mark the missed first items of enumerations.
    void markListStarts(Sentence<DefaultToken> * sentence);
    // Get a string representation of the sentence and its internal marks.
    string internalMarkedSentenceAsString(
        Sentence<DefaultToken> const & sentence);
    // Convert an InternalMark to a string.
    string internalMarkToString(InternalMark mark);
    // Correct the marking of the sentence.
    void correctMarks(Sentence<DefaultToken> * sentence);
    std::vector <std::pair <InternalMark, size_t> > _markers;
    // a vector telling using the same index as phrases
    std::vector <InternalMark> _markerVector;
    // std::vector <InternalMark> _chunkMarkerVector;
    bool _verbose;
};

#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_DECOMPOSER_RULE_ROBUSTCONTEXTMARKER_H_
