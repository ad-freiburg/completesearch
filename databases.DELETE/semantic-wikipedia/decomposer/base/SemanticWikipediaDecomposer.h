// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIADECOMPOSER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIADECOMPOSER_H_

#include <gtest/gtest.h>
#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <string>
#include <hash_set>
#include "base/ContextDecomposerBase.h"
#include "base/SemanticWikipediaReader.h"
#include "base/ContextWriterBase.h"
#include "sentence/Sentence.h"


namespace ad_decompose
{
// Main class for chunking. Sequentially reads the index file, builds the
// sentence structure and extracts facts.
template <class Token>
class SemanticWikipediaDecomposer
{
  public:
    static const size_t MAX_SENTENCE_SIZE = 150;
    // Create a new chunker. The inputFileName is the file name of the POS
    // tagged index.
    explicit SemanticWikipediaDecomposer(bool writeToStdOut,
        ContextDecomposerBase<Token> & decomposer,
        ContextWriterBase<Token> const & writer,
        SemanticWikipediaReader<Token> & reader);
    // Parse the index file and create the sentence structure.
    void parseAndChunk();
    // Parse a single line into a token, it does not set the
    // wordPosition of the token!
    Token * parseTokenLine(std::string line);
  private:
    bool _verbose;
    ContextDecomposerBase<Token> & _decomposer;
    ContextWriterBase<Token> const & _writer;
    SemanticWikipediaReader<Token> & _reader;
    std::string _inputFileName;
    std::vector<boost::regex> _factRegexes;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIADECOMPOSER_H_
