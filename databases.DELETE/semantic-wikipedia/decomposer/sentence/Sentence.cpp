// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <ext/hash_set>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "util/ContextDecomposerUtil.h"
#include "../parser/EntityAwareWriter.h"
#include "sentence/Sentence.h"

// For internal representation of strings of POS tags
// and phrase tags.
#define POSTAG_LENGTH 5
#define POSTAG_FORMAT "%-5s"
#define PHRASETAG_LENGTH 6
#define PHRASETAG_FORMAT "%-6s"

using std::string;

// ____________________________________________________________________________
template <class Token>
Sentence<Token>::Sentence()
:_nextCharacterIndex(0)
{
  // _currentPhrase = NULL;
  // _entityOpen = false;
  // _isTranslationSentence = false;

  _docTitle = "empty";
  _docUrl  = "http://empty";
}

// ____________________________________________________________________________
template <class Token>
Sentence<Token>::~Sentence()
{
}

// ____________________________________________________________________________
template <class Token>
void Sentence<Token>::appendBrTag(size_t pos, string const & brTag) const
{
  assert(pos < _wordSequence.size());
  _wordSequence[pos]->appendMark(Token::stringToMark(brTag));
}

// ____________________________________________________________________________
template <class Token>
void Sentence<Token>::clearBrTag(size_t pos) const
{
  assert(pos < _wordSequence.size());
  _wordSequence[pos]->clearBrTags();
}

// ____________________________________________________________________________
template <class Token>
string Sentence<Token>::asStringWithPhrases() const
{
  std::ostringstream s;
  string wordString;
  string typeString;
  for (size_t i = 0; i < _phrases.size();i++)
  {
    string typeStringSub = "";
    std::vector <Token const *> const & words = _phrases[i].getWords();
    for (size_t j = 0; j < words.size(); j++)
    {
      wordString += (words[j]->tokenString) + " ";
      typeStringSub.append(words[j]->tokenString.length() + 1, ' ');
    }
    typeStringSub.replace(0, _phrases[i].getType().length(),
        _phrases[i].getType());
    typeString += typeStringSub;
  }
  s << wordString << std::endl;
  s << typeString << std::endl << std::endl;

  return s.str();
}

// ____________________________________________________________________________
template <class Token>
string Sentence<Token>::asStringWithWordMarks() const
{
  std::ostringstream s;
  for (size_t i = 0; i < _wordSequence.size(); i++)
  {
    size_t mark = 0;
    for (mark = 0; mark < _wordSequence[i]->marks.size(); ++mark)
    {
      if (Token::isCloseMark(_wordSequence[i]->marks[mark]))
        break;
      s << Token::markToString(_wordSequence[i]->marks[mark]);
    }
    s << " " << _wordSequence[i]->tokenString << " ";
    for (; mark < _wordSequence[i]->marks.size(); ++mark)
    {
      s << Token::markToString(_wordSequence[i]->marks[mark]);
    }
  }
  return s.str();
}


// ____________________________________________________________________________
template <class Token>
string Sentence<Token>::asString() const
{
  return _completeSentence;
}

// ____________________________________________________________________________
template <class Token>
vector <Phrase<Token> *> const & Sentence<Token>::getPhrases() const
{
  return _phraseSequence;
}

// ____________________________________________________________________________
template <class Token>
string Sentence<Token>::asPOSTagString() const
{
  return _completePOSSentence;
}

// ____________________________________________________________________________
template <class Token>
string Sentence<Token>::asPhraseTagString() const
{
  return _completePhraseTagSentence;
}

// ____________________________________________________________________________
template <class Token>
Token * Sentence<Token>::getNewToken()
{
  _tokens.push_back(new Token());
  return &(_tokens.back());
}

// ____________________________________________________________________________
template <class Token>
Entity<Token> * Sentence<Token>::getNewEntity()
{
  Entity<Token> * e = new Entity<Token>();
  _entities.push_back(e);
  return &(_entities.back());
}

// ____________________________________________________________________________
template <class Token>
Phrase<Token> * Sentence<Token>::getNewPhrase(string const & npTag)
{
  Phrase<Token> * p = new Phrase<Token>(npTag, this);
  _phrases.push_back(p);
  return &(_phrases.back());
}

// ____________________________________________________________________________
template <class Token>
void Sentence<Token>::appendWord(Token * const token)
{
  token->tokenWordPosition = _wordSequence.size();
  _wordSequence.push_back(token);

  token->tokenCharPosition = _nextCharacterIndex;

  if (_wordSequence.size() == 1)
      _completeSentence += (token->tokenString);
  else
    _completeSentence += " " + (token->tokenString);

  _nextCharacterIndex += token->tokenString.length()+1;
  char buffer[POSTAG_LENGTH+1];
  snprintf(buffer, POSTAG_LENGTH+1, POSTAG_FORMAT , token->posTag.c_str());
  _completePOSSentence += (string(buffer) + " ");
}

// ____________________________________________________________________________
template <class Token>
void Sentence<Token>::appendPhrase(Phrase<Token> * const phrase)
{
  phrase->setPhrasePosition(_phraseSequence.size());
  _phraseSequence.push_back(phrase);

  char buffer[PHRASETAG_LENGTH + 1];
  snprintf(buffer, PHRASETAG_LENGTH + 1, PHRASETAG_FORMAT,
      phrase->getType().c_str());
  _completePhraseTagSentence += (string(buffer) + " ");
}

// ____________________________________________________________________________
template <class Token>
void Sentence<Token>::appendEntity(Entity<Token> * const entity)
{
  _entitiesSequence.push_back(entity);
}

// ____________________________________________________________________________
template <class Token>
vector <Token *> const & Sentence<Token>::getWords() const
{
  return _wordSequence;
}

// ____________________________________________________________________________
template <class Token>
void Sentence<Token>::finishInput()
{
  // _completeSentence[_completeSentence.size()-1]='.';
}

// ____________________________________________________________________________
template <class Token>
Phrase<Token>::~Phrase()
{
}


// ____________________________________________________________________________
template <class Token>
void Phrase<Token>::setPhrasePosition(size_t phrasePosition)
{
  _phrasePosition = phrasePosition;
}

// ____________________________________________________________________________
template <class Token>
Phrase<Token>::Phrase(string type, Sentence<Token> * sentence)
{
  _wordStartIndex = 0;
  _sentence = sentence;
  _wordEndIndex = 0;
  _phrasePosition = 0;
  _phraseLength = 0;
  _startsRelativeClause = false;
  _startsRelativeClausePerson = false;
  string::size_type indexOf = type.find('-');
  if (indexOf > 0)
  {
    _type = type.substr(indexOf+1);
  }
  else
  {
    _type = type;
  }
}

// ____________________________________________________________________________
template <class Token>
string Phrase<Token>::getType() const
{
  return _type;
}

// ____________________________________________________________________________
template <class Token>
bool Phrase<Token>::isStopPhrase() const
{
  if (_startsRelativeClause || _type == "O" || _type == "SBAR")
    return true;
  else
    return false;
}

// ____________________________________________________________________________
template <class Token>
Token const & Phrase<Token>::getFirstWord() const
{
  return *_words.front();
}

// ____________________________________________________________________________
template <class Token>
Token const & Phrase<Token>::getLastWord() const
{
  return *_words.back();
}

// ____________________________________________________________________________
template <class Token>
bool Phrase<Token>::startsRelativeClause() const
{
  if (_startsRelativeClause || _startsRelativeClausePerson)
    return true;
  // we can also start a relativeclause if the first words are in gerund form
  // else if (_words.size() > 1 && _words[0]->posTag == "VBG"
  //    && _words[1]->posTag == "VBN")
  //  return true;
  return false;
}

// ____________________________________________________________________________
template <class Token>
bool Phrase<Token>::containsSayingWord() const
{
  if (_type != "VP")
    return false;
  // TODO(elmar): implement list of saying verbs and their recognition
  else
    return false;
}

// ____________________________________________________________________________
template <class Token>
void Phrase<Token>::addWord(Token * word)
{
  _wordEndIndex = word->tokenWordPosition;
  if (_phraseLength == 0)
  {
    _wordStartIndex = word->tokenWordPosition;
    _phraseLength += 1;
  }

  _words.push_back(word);
  if (_words.size() == 1)
  {
    if (_words[0]->posTag == "WDT")
    {
      _startsRelativeClause = true;
    }
    else if (_words[0]->posTag == "WP$" || _words[0]->posTag == "WP")
    {
      _startsRelativeClausePerson = true;
    }
  }
  // this is the wrong place, PP WDT will not occur in the same phrase!
  // a relative clause can also start with PP WDT, or IN WDT
//  else if (_words.size() ==2)
//  {
//    if ((_words[0]->posTag == "PP" && _words[1]->posTag == "WDT") ||
//       (_words[0]->posTag == "IN" && _words[1]->posTag == "WDT"))
//      _startsRelativeClause = true;
//    else if ((_words[0]->posTag == "PP" && _words[1]->posTag == "WDT") ||
//            (_words[0]->posTag == "IN" && _words[1]->posTag == "WDT"))
//      _startsRelativeClausePerson = true;
//  }

  // add to the list of entities only if the previously added entity is
  // different
  if (word->isPartOfEntity && _entities.size() == 0)
    _entities.push_back(word->_entity);
  else if (word->isPartOfEntity && _entities.size() > 0
      && !(*(_entities.back()) == *(word->_entity)))
  {
    _entities.push_back(word->_entity);
  }
}

// ____________________________________________________________________________
template <class Token>
string Phrase<Token>::asString() const
{
  string sentence = _sentence->asString();
  vector<Token *> const & words = _sentence->getWords();
  const char * start = (sentence.c_str()
      + words[_wordStartIndex]->tokenCharPosition);
  const char * end = (sentence.c_str() + words[_wordEndIndex]->tokenCharPosition
      + words[_wordEndIndex]->tokenString.length());
  return string(start, end-start);
}

// ____________________________________________________________________________
template <class Token>
std::vector <Entity<Token> const *> const &
  Phrase<Token>::getEntities() const
{
  return _entities;
}

// ____________________________________________________________________________
template <class Token>
std::vector<Token const *> const & Phrase<Token>::getWords() const
{
  return _words;
}

// ____________________________________________________________________________
template <class Token>
size_t Phrase<Token>::getPhraseIndexPosition() const
{
  return _phrasePosition;
}

// ____________________________________________________________________________
template <class Token>
size_t Phrase<Token>::getWordsStartIndex() const
{
  return _wordStartIndex;
}

// ____________________________________________________________________________
template <class Token>
size_t Phrase<Token>::getWordsEndIndex() const
{
  return _wordEndIndex;
}

// ____________________________________________________________________________
template <class Token>
bool Phrase<Token>::matchWordsRegex(string regex) const
{
  boost::regex searchRegex(regex);
  string sentence = _sentence->asString();
  vector<Token *> const & words = _sentence->getWords();
  const char * start = (sentence.c_str()
      + words[_wordStartIndex]->tokenCharPosition);
  const char * end = (sentence.c_str() + words[_wordEndIndex]->tokenCharPosition
      + words[_wordEndIndex]->tokenString.length());
  boost::cmatch what;
  bool res = regex_match(start, end, what, searchRegex);
  // std::cout << what[0];
  return res;
}

// ____________________________________________________________________________
template <class Token>
bool Phrase<Token>::matchPOSRegex(string regex) const
{
  boost::regex searchRegex(regex);
  string sentence = _sentence->asPOSTagString();
  const char * start = (sentence.c_str() + _wordStartIndex
      * (POSTAG_LENGTH+1));
  const char * end = (sentence.c_str() + _wordEndIndex * (POSTAG_LENGTH+1)
      + POSTAG_LENGTH);
  boost::cmatch what;
  bool res = regex_match(start, end, what, searchRegex);
  return res;
}

template class Phrase<DefaultToken>;
template class Phrase<DeepToken>;
template class Sentence<DefaultToken>;
template class Sentence<DeepToken>;

