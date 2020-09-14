// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>


#include <fstream>
#include <string>
#include <vector>
#include "./SemanticWikipediaReader.h"
#include "../parser/EntityAwareWriter.h"
#include "util/ContextDecomposerUtil.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

namespace ad_decompose
{
// ____________________________________________________________________________
template <class Token>
SemanticWikipediaReader<Token>::SemanticWikipediaReader()
{
  init();
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::setInputFilename(
    const std::string& inputFileName)
{
  LOG(INFO) << "Opening " << inputFileName << ".\n";
  _indexStream = new std::ifstream(inputFileName.c_str(), std::ios::in);
  assert(_indexStream->good());
  init();
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::setInputStream(
    std::istringstream * istringstream)
{
  _indexStream = istringstream;
  assert(_indexStream->good());
  init();
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::init()
{
  _entityOpen = false;
  _currentEntity = NULL;
  _currentPhrase = NULL;
  _numSentencesRead = 0;
}


// ____________________________________________________________________________
template <class Token>
SemanticWikipediaReader<Token>::~SemanticWikipediaReader()
{
  // _indexStream->close();
}

// ____________________________________________________________________________
template <class Token>
bool SemanticWikipediaReader<Token>::parseNextSentence(
    Sentence<Token> * sentence)
{
  std::string line;
  ad_semsearch::Id currentDoc = 0;

  static Token * lastToken = NULL;
  if (_indexStream->eof())
    return false;
  if (lastToken != NULL)
  {
    sentence->storeToken(lastToken);
    addTokenToSentence(lastToken, sentence);
    lastToken = NULL;
  }
  while (std::getline(*_indexStream, line))
  {
    // std::cout << line << "-\n" << std::flush;
    Token * token = new Token();
    // Token * token = sentence->getNewToken();
    ContextDecomposerUtil::parseTokenLine(line, token);

    // a new sentence begins, also executed for the very first token
    if (token->completeSearchDocId != currentDoc)
    {
      // For the very first token proceed as normal.
      if (currentDoc == 0)
      {
        currentDoc = token->completeSearchDocId;
      }
      // Not the very first token. Finish the sentence.
      // And remember the token we read ahead.
      else
      {
        currentDoc = token->completeSearchDocId;
        finishSentence(sentence);
        lastToken = token;
        ++_numSentencesRead;
        reportStatus();
        return true;
      }
    }
    sentence->storeToken(token);
    addTokenToSentence(token, sentence);
    // std::cout << "docId: " << docId << " pos: "
    //          << position << " score: " << score
    //          << "posTag " << posTagStr << std::endl;
  }
  // No more lines - finish up and return.
  finishSentence(sentence);
  ++_numSentencesRead;
  reportStatus();
  return true;
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::reportStatus()
{
  if (_numSentencesRead % STATUS_SENTENCE_FREQUENCEY == 0)
  {
    LOG(INFO)
      << "SemanticWikipediaReader read : " << _numSentencesRead
      << " sentences.\n";
  }
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::parseSentenceFromTokens(Sentence<Token>*
    sentence, vector<Token *> tokens)
{
  for (size_t i = 0; i < tokens.size(); ++i)
    addTokenToSentence(tokens[i], sentence);
  finishSentence(sentence);
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::finishSentence(Sentence<Token> * sentence)
{
  // std::cout << "Sentencce done\n";
  // sentence->finishInput();
  _entityOpen = false;
  _currentEntity = NULL;
  _currentPhrase = NULL;
}

// ____________________________________________________________________________
template <class Token>
void SemanticWikipediaReader<Token>::addTokenToSentence(Token * token,
    Sentence<Token> * sentence)
{
  // Continuous commas will be ignored, there can only be one.
  if (token->tokenString == "," && sentence->getWords().size() > 0
      && sentence->getWords().back()->tokenString == ",")
  {
    // delete &token;
    return;
  }
  // Is this is a special word?
  if (token->tokenString[0] == ':' && token->tokenString != ":")
  {
    // This starts an entity.
    if (token->tokenString[1] == 'e')
    {
      if (!_entityOpen)
      {
        _entityOpen = true;
        Entity<Token> * newEntity = sentence->getNewEntity();
        sentence->appendEntity(newEntity);
        _currentEntity = newEntity;
      }
      // This is the path to an entity.
      _currentEntity->addPath(*token);
      // check if this is a person-entity
      if (!_currentEntity->isPersonEntity()
          && token->tokenString.find(":person:") != string::npos)
          _currentEntity->setIsPersonEntity(true);
      token->_entity = _currentEntity;
      token->isPartOfEntity = true;
    }
    // This is the document title the sentence belongs to.
    else if (token->tokenString[1] == 't')
    {
      sentence->setDocTitle(token->tokenString.substr(7));
    }
    // This is the document url the sentence belongs to.
    else if (token->tokenString[1] == 'u')
    {
      sentence->setDocUrl(token->tokenString.substr(5));
    }
    else
    {
      sentence->addSpecialWord(token);
    }
  }
  // TODO(elmar): workaround until the ct:.. special words are removed
  // from the index. For now we just ignore and delete them.
  else if (token->tokenString.size() > 3 &&
      token->tokenString[0] == 'c' &&
      token->tokenString[1] == 't' &&
      token->tokenString[2] == ':')
  {
    return;
    // delete &token;
  }
  else if (token->tokenString == ad_semsearch::ENTITY_END_MARKER)
  {
    _entityOpen = false;
    return;
    // If the entity was openend and closed immediately afterwards
    // we delete it.
    //   if (!_currentEntity->hasWord())
    // {
    // std::cout << "Removing wrong entity\n";
    // delete &_entities.back();
    // _entities.erase(_entities.back());
    // }
    // delete token here, we dont want to remember that one
    // delete &token;
    // _paths.push_back(&token);
  }
  else
  {
    // Beginning of a new phrase
    // or a token that is outside of any phrase
    if (token->npTag[0] == 'B' || token->npTag[0] == 'O')
    {
      Phrase<Token> * newPhrase = sentence->getNewPhrase(token->npTag);
      sentence->appendPhrase(newPhrase);
      _currentPhrase = newPhrase;
    }
    // A workaround for broken sentences. Some are not tokenized correctly and
    // start in the middle etc.
    else if (_currentPhrase == NULL)
    {
      Phrase<Token> * newPhrase = sentence->getNewPhrase(token->npTag);
      sentence->appendPhrase(newPhrase);
      _currentPhrase = newPhrase;
    }
    if (_entityOpen)
    {
      token->isPartOfEntity = true;
      token->_entity = _currentEntity;
      // add word as belonging to entity.
      _currentEntity->addWord(*token);
    }
    //   std::cout << "Token is at " << &token << "\n" << std::flush;
    token->_phrase = _currentPhrase;
    // std::cout << "Appending " << token->tokenString;
    sentence->appendWord(token);
    _currentPhrase->addWord(token);
    // std::cout << "Now Token is at " << &_words.back() << "\n" << std::flush;
  }
}

template class SemanticWikipediaReader<DefaultToken>;
template class SemanticWikipediaReader<DeepToken>;
}
