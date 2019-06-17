// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIAWRITER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIAWRITER_H_

#include <gtest/gtest.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <set>
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "sentence/Sentence.h"
#include "base/ContextWriterBase.h"

#define DONT_WRITE_DOCS_FILE  1
#define WRITE_DOCS_FILE 2
#define WRITE_DECOMP_INFO 4
#define DECOMPOSE_INFO_MAX_CONTEXTS 20
#define CONTEXT_HIGHLIGHT_START "$hlct$"
#define CONTEXT_HIGHLIGHT_END   "$/hlct$"
#define DECOMPOSE_INFO_START   "$addinfo$"
#define DECOMPOSE_INFO_END   "$/addinfo$"
#define DECOMPOSE_INFO_CONTEXT_START "$context$"
#define ENTITY_DOCS_PREFIX   "^^"
#define WORD_HL_DOCS_PREFIX   "^^^"

namespace ad_decompose
{
// 0 - default, space before this character
// a - no space after this character
// b - no space before this character
// c - no space after and before this character
static const char
    SPACE_CHAR_MAP[257] =
       //                                 !"  % '()  , ./          :;   ?                                                   s // NOLINT
        "000000000000000000000000000000000b000b00ab00b0bc0000000000bb000b000000000000000000000000000000000000000000000000000b00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"; //NOLINT

// Main class for chunking. Sequentially reads the index file, builds the
// sentence structure and extracts facts.
template<unsigned char MODE, class Token>
class SemanticWikipediaWriter : public ContextWriterBase<Token>
{
  public:
    // Create a new chunker. The inputFileName is the file name of the POS
    // tagged index.
    explicit SemanticWikipediaWriter(const std::string& wordsFileName,
        const std::string& docsFileName, const std::string& longDocsFileName,
        size_t minWordLength);
    virtual ~SemanticWikipediaWriter();

    string statsAsString()
    {
      std::stringstream s;
      s << "SemanticWikipediaWriter wrote " << _nTotalWords << " words in "
          << _nTotalSentences << " documents and ignored "
          << _nTotalWordsIgnored << " words and " << _nIgnoredSentences
          << " documents due to stop words" << " and clitics." << std::endl;
      return s.str();
    }

    // Write the provided contexts to the output file.
    virtual void writeContexts(Contexts<Token> const & contexts,
        Sentence<Token> const & sentence) const;

    // Write a long sentence to a seperate file.
    virtual void writeLongSentence(Sentence<Token> const & sentence) const;

    // Set and read the stop-words file.
    void setStopWordsFile(string const stopWordsFileName);

    // Set and read the clitics file.
    void setCliticsFile(string const cliticsFileName);

  private:
    // Write an entity to the output file. This writes all paths that have
    // been found for that entity.
    void writeEntity(Entity<Token> const & entity) const;

    // Write a word that was ignored to the special file.
    void writeIgnoredWord(Token const * word) const;


    // Write the sentence with highlighted context to the stream given.
    // The context to highlight is provided as index.
    void writeSentenceToDocs(Contexts<Token> const & contexts,
        size_t contextIndex, Sentence<Token> const & sentence,
        std::ofstream * outStream) const;


    // If wanted write additional information about the decomposition process
    // to the stream given.
    void writeDecompositionInfoToDocs(Contexts<Token> const & context,
        size_t contextIndex, Sentence<Token> const & sentence,
        std::ofstream * outStream) const;

    // Decide whether the given word at index is preceded by a space or not.
    bool decidePreceedingSpace(Sentence<Token> const & sentence,
        int index) const;

    // For a given entity construct the markup string required for the docs
    // file.
    std::string getDocsStringForEntity(Entity<Token> const & entity) const;

    // Sequences cut off at the end of a word.
    std::set<string> _clitics;

    // Stop words not written to index.
    std::set<string> _stopWords;

    // Filename of words-unsorted file.
    std::string _wordsFileName;
    // Filename of docs-unsorted file.
    std::string _docsFileName;
    // Filename of very-long-sentences file.
    std::string _longDocsFileName;

    // IO-Stream of words-unsorted file.
    std::ofstream * _wordsStream;
    // IO-Stream of words-unsorted.ignored file.
    std::ofstream * _wordsIgnoreStream;
    // IO-Stream of docs-unsorted file.
    std::ofstream * _docsStream;
    // IO-Stream of very-long-sentences file.
    std::ofstream * _longDocsStream;


    // Document ID - reassigned during output.
    mutable int _docId;

    // Minimum lenth of a word written to index.
    size_t _minWordLength;

    // Total number of sentences written.
    mutable int _nTotalSentences;

    // Total number of sentences ignored.
    mutable int _nIgnoredSentences;

    // Total number of words written.
    mutable int _nTotalWords;

    // Total number of words ignored.
    mutable int _nTotalWordsIgnored;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIAWRITER_H_
