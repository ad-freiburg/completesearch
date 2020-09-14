// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#ifndef SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_SENTENCE_H_
#define SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_SENTENCE_H_

#include <gtest/gtest.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/regex.hpp>
#include <stdint.h>
#include <vector>
#include <utility>
#include <string>
#include "sentence/Token.h"

using std::string;
using std::vector;

template <class Token>
class Phrase
{
  public:
    explicit Phrase(string type, Sentence<Token> * sentence);
    ~Phrase();
    // Add a word, referenced in the words vector, to this subsentence
    void addWord(Token * word);
    std::vector <Token const *> const & getWords() const;
    string getType() const;
    std::vector <Entity<Token> const *>  const & getEntities() const;
    // return the phrase in a stringrepresentation, this contains special words
    // to mark the phrase type
    void setPhrasePosition(size_t phrasePosition);
    string asString() const;
    Token const & getFirstWord() const;
    Token const & getLastWord() const;
    bool isStopPhrase() const;
    // Return the index of the phrase in the vector of all phrases
    // of the sentence.
    size_t getPhraseIndexPosition() const;
    // Return the index of the first word in the vector of all words
    // of the sentence.
    size_t getWordsStartIndex() const;
    // Return the index of the first word in the vector of all words
    // of the sentence.
    size_t getWordsEndIndex() const;
    bool startsRelativeClause() const;
    bool startsRelativeClausePerson() const;
    bool containsSayingWord() const;
    // Match a regular expression against the words of the phrase.
    // The regular expression must match all words (it is not a search).
    bool matchWordsRegex(string regex) const;
    // Match a regular expression against the POS-tags of the phrase.
    // The regular expression must match all words (it is not a search).
    bool matchPOSRegex(string regex) const;
  private:
    // index where the first word of this phrase starts
    size_t _wordStartIndex;
    // index where the last word of this phrase ends
    size_t _wordEndIndex;
    // length of this phrase
    size_t _phraseLength;
    // index of this phrase in the overall phrase vector
    size_t _phrasePosition;
    // Vector of all entities in the phrase
    std::vector <Entity<Token> const *> _entities;
    // the type of phrase, i.e. NP, PP etc.
    // TODO(elmar): replace with ENUM
    string _type;
    std::vector <Token const * > _words;
    bool _startsRelativeClause;
    bool _startsRelativeClausePerson;
    // pointer back to the whole sentence
    Sentence<Token> * _sentence;
};

typedef Phrase<DefaultToken> DefaultPhrase;

// A complete sentence consisting of a vector of
// subsentences, as well as a vector of all words.
template <class Token>
class Sentence
{
  public:
    Sentence();
    ~Sentence();

    // Add a token to the sentence. Must be called in the sequence the
    // tokens appear in the sentence. This will build the structure of
    // the sentence. A token can also be an entity path, following tokens
    // are then identified as being part of a specific entity.
    // (thus Token is non-const). Ownership is transferred to the sentence
    // and pointers may be invalid after calling this function.
    // void addAndStoreToken(Token & token);

    // Return a string representation that includes the phrases of
    // the sentence. The phrase tags are on a seperate line.
    string asStringWithPhrases() const;
    // Return a string representation that includes the marks of
    // each word of the sentence. Each word is on a new line.
    string asStringWithWordMarks() const;
    // Get the sentence as string. Will only output the words (no entity
    // definitions).
    string asString() const;
    // Get all the POS tags of the words. Will ignore the entity tokens.
    string asPOSTagString() const;
    // Get all the Phrase tags of the phrases as a single string.
    string asPhraseTagString() const;
    // Finish the input. Must be called after the last token has been added.
    void finishInput();
    // Get the vector of all words.
    vector <Token *> const & getWords() const;
    // return all phrases
    vector <Phrase<Token> *> const & getPhrases() const;
    // TODO(elmar): these should return const and we need other methods for
    // modification
    // return all syntactic chunks
    // Append the provided tag to the word at position pos.
    // Needed in classification tasks.
    void appendBrTag(size_t pos, string const & tag) const;
    void clearBrTag(size_t pos) const;

    // Transfer ownership of this single token
    // to the sentence.
    void storeToken(Token * token)
    {
      _tokens.push_back(token);
    }

    // Return the document title of this sentence.
    string getDocTitle() const
    {
      return _docTitle;
    }

    // Return the document URL of this sentence.
    string getDocUrl() const
    {
      return _docUrl;
    }

    // Return the document title of this sentence.
    void setDocTitle(string const & title)
    {
      _docTitle = title;
    }

    // Return the document URL of this sentence.
    void setDocUrl(string const & url)
    {
      _docUrl = url;
    }

    // Create a new Token object. Ownership remains with the sentence.
    Token * getNewToken();
    // Create a new Entity object. Ownership remains with the sentence.
    Entity<Token> * getNewEntity();
    // Create a new Phrase object. Ownership remains with the sentence.
    Phrase<Token> * getNewPhrase(string const & npTag);

    // Append the word to the sequence of words.
    void appendWord(Token * const word);
    // Append the phrase to the sequence of phrases.
    void appendPhrase(Phrase<Token> * const phrase);
    // Append the entity to the sequence of entities.
    void appendEntity(Entity<Token> * const entity);

    // Add a special word to the sentence.
    void addSpecialWord(Token * specialWord)
    {
      _specialWords.push_back(specialWord);
    }

    vector<Token *> const & getSpecialWords() const
    {
      return _specialWords;
    }

  private:
    Sentence(Sentence & s);
    boost::ptr_vector<Phrase<Token> > _phrases;
    boost::ptr_vector<Token> _tokens;
    boost::ptr_vector<Entity<Token> > _entities;

    // Vector of all phrases in sequence.
    vector <Phrase<Token> *> _phraseSequence;
    // Vector of all entities in sequence added.
    vector <Entity<Token> *> _entitiesSequence;
    // Vector of all words in sequence.
    vector <Token *> _wordSequence;

    string _completeSentence;
    // All the POS tags of  words of the sentence as string.
    string _completePOSSentence;
    // All the Phrase tags of  words of the sentence as string.
    string _completePhraseTagSentence;
    // A marker if currently an entity is described.
    // bool _entityOpen;
    // A marker if this is a translation sequence
    // bool _isTranslationSentence;
    // The next character index a token would start at in the complete sentence
    // string
    size_t _nextCharacterIndex;
    // The title of the document the sentence belongs to.
    // Needed for documents file.
    string _docTitle;
    // The URL of the document the sentence belongs to.
    // Needed for documents file.
    string _docUrl;
    vector<Token *> _specialWords;
};


#endif  // SEMANTIC_WIKIPEDIA_DECOMPOSER_SENTENCE_SENTENCE_H_
