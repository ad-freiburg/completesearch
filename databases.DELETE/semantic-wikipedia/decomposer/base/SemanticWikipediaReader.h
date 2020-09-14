// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIAREADER_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIAREADER_H_

#include <string>
#include <hash_set>
#include <vector>
#include "sentence/Sentence.h"

namespace ad_decompose
{
// Sequentially reads the index file, builds the
// sentence structure and returns each sentence.
template <class Token>
class SemanticWikipediaReader
{
  public:

    // Report all STATUS_SENTENCE_FREQUENCEY read sentences.
    static const int STATUS_SENTENCE_FREQUENCEY = 10000;

    // Construct a new reader w/o initialistation.
    // Must call setInputFilename or setInputStream afterwards
    // or only use getSentenceFromTokens.
    explicit SemanticWikipediaReader();
    ~SemanticWikipediaReader();
    // Create the next sentence in the provided object.
    bool parseNextSentence(Sentence<Token> * sentence);
    // Base the reader on a file with name inputFileName.
    void setInputFilename(const std::string& inputFileName);
    // Base the reader on a stringstream.
    void setInputStream(std::istringstream * istringstream);
    // Add the tokens given to the sentence given as parameter.
    void parseSentenceFromTokens(Sentence<Token> * sentence,
        vector<Token *> tokens);
  private:
    // Initialise default values.
    void init();
    // Finish current sentence, reset values.
    void finishSentence(Sentence<Token> * sentence);
    // Add a single token to the sentence passed.
    void addTokenToSentence(Token * token, Sentence<Token> * sentence);
    // Print current status.
    void reportStatus();
    // If given the input filename.
    std::string _inputFileName;
    // The stream the reader uses to read.
    std::istream * _indexStream;
    // Variables for parsing.
    // Is an entity opened currently?
    bool _entityOpen;
    // A pointer to the current entity opened.
    Entity<Token> * _currentEntity;
    // A pointer to the phrase currently opened.
    Phrase<Token> * _currentPhrase;
    // Number of sentences read.
    int _numSentencesRead;
};
}
#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_BASE_SEMANTICWIKIPEDIAREADER_H_
