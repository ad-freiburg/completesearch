// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "util/ContextDecomposerUtil.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "./SemanticWikipediaWriter.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::ostringstream;

namespace ad_decompose
{
// ____________________________________________________________________________
template<unsigned char MODE, class Token>
SemanticWikipediaWriter<MODE, Token>::SemanticWikipediaWriter(
    const std::string& wordsFileName, const std::string& docsFileName,
    const std::string& longDocsFileName, size_t minWordLength = 1) :
    _wordsFileName(wordsFileName), _docsFileName(docsFileName),
    _longDocsFileName(longDocsFileName), _minWordLength(
        minWordLength), _nTotalSentences(0), _nIgnoredSentences(0),
        _nTotalWords(0), _nTotalWordsIgnored(0)
{
  LOG(INFO)
  << "SemanticWikipediaWriter outputting words to: "
    << _wordsFileName << std::endl;
  LOG(INFO)
    << "SemanticWikipediaWriter outputting docs to: "
    << _docsFileName << std::endl;
  LOG(INFO)
    << "SemanticWikipediaWriter outputting long sentences to: "
    << _longDocsFileName << std::endl;

  // Create streams.
  _wordsStream = new std::ofstream(_wordsFileName.c_str(), std::ios::trunc);
  _wordsIgnoreStream = new std::ofstream((_wordsFileName+".ignored").c_str(),
      std::ios::trunc);
  _docsStream = new std::ofstream(_docsFileName.c_str(), std::ios::trunc);
  _longDocsStream = new std::ofstream(_longDocsFileName.c_str(),
      std::ios::trunc);
  // Check streams.
  assert(_wordsStream->is_open());
  assert(_wordsIgnoreStream->is_open());
  assert(_docsStream->is_open());
  assert(_longDocsStream->is_open());

  _docId = 1;
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
SemanticWikipediaWriter<MODE, Token>::~SemanticWikipediaWriter()
{
  // Close open streams.
  _wordsStream->close();
  _wordsIgnoreStream->close();
  _docsStream->close();
  _longDocsStream->close();
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::setStopWordsFile(
    string const stopWordsFileName)
{
  ContextDecomposerUtil::readFileToContainer(&_stopWords, stopWordsFileName);
  LOG(INFO)
    << "SemanticWikipediaWriter using " << _stopWords.size() << " stop words."
    << std::endl;
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::setCliticsFile(
    string const cliticsFileName)
{
  ContextDecomposerUtil::readFileToContainer(&_clitics, cliticsFileName);
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::writeLongSentence(
    Sentence<Token>const & sentence) const
{
  vector<Token *> const & words = sentence.getWords();
  ad_semsearch::Id docId = words.front()->completeSearchDocId;
  ostringstream os;
  for (size_t i = 0; i < words.size(); ++i)
  {
    os << words[i]->tokenString << " ";
  }
  os << std::endl;
  *_longDocsStream << docId << "\t" << "u:" << sentence.getDocUrl()
      << "\t" << "t:" << sentence.getDocTitle() << "\t" << "H:" << os.str();
}


// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::writeIgnoredWord(
    Token const * word) const
{
  *_wordsIgnoreStream << word->tokenString << "\t" << word->completeSearchDocId
      << "\t" << static_cast<int>(word->completeSearchScore) << "\t"
      << word->completeSearchPosition << std::endl;
}



// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::writeContexts(
    Contexts<Token> const & contexts, Sentence<Token>const & sentence) const
{
  vector<Token *> specialWords = sentence.getSpecialWords();
  for (size_t i = 0; i < specialWords.size(); ++i)
  {
    Token * token = specialWords[i];
    *_wordsStream << token->tokenString << "\t" << _docId << "\t"
        << static_cast<int>(token->completeSearchScore) << "\t"
        << token->completeSearchPosition << std::endl;
  }

  Entity<Token> const * lastEntity = NULL;
  for (size_t i = 0; i < contexts.size(); ++i)
  {
    // ad_utility::HashMap<string, bool > writtenEntities;
    // Make sure that we write at least one word
    bool wordWritten = false;

    for (size_t j = 0; j < contexts[i].size(); ++j)
    {
      // Context<Token> const & test = contexts[i];
      Token const * token = contexts[i][j];
      string word = token->tokenString;

      // If the next word is a clitic append it.
      //      if (j < contexts[i].size() - 1)
      //      {
      // if (_clitics.find(contexts[i][j + 1]->tokenString) != _clitics.end())
      //        {
      //          word += contexts[i][j + 1]->tokenString;
      //          ++j;
      //        }
      //      }

      // If the word is a clitic, ignore it.
      if (_clitics.find(word) != _clitics.end())
      {
        writeIgnoredWord(token);
        ++_nTotalWordsIgnored;
        continue;
      }

      // Only use lowercase from here on.
      word = ad_utility::getLowercase(word);

      // Ignore words shorter than mininum word length and stop words.
      if (!token->isPartOfEntity
          && (word.length() < _minWordLength
              || _stopWords.find(word) != _stopWords.end()))
      {
        writeIgnoredWord(token);
        ++_nTotalWordsIgnored;
        continue;
      }

      // Write entity stuff if necessary.
      if (token->isPartOfEntity && (lastEntity != token->_entity))
      {
        writeEntity(*token->_entity);
        lastEntity = token->_entity;
      }
      else
        lastEntity = NULL;

      // Output.
      *_wordsStream << word << "\t" << _docId << "\t"
          << static_cast<int>(token->completeSearchScore) << "\t"
          << token->completeSearchPosition << std::endl;
      _nTotalWords++;
      wordWritten = true;
    }
    // Write to docs file if wanted and needed.
    if ((MODE & WRITE_DOCS_FILE) && wordWritten)
    {
      writeSentenceToDocs(contexts, i, sentence, _docsStream);
    }
    // Next context -> next doc Id. But only if we actually wrote something.
    if (wordWritten)
    {
      ++_docId;
      ++_nTotalSentences;
    }
    // If we didn't write anything for this context.
    else
    {
      ++_nIgnoredSentences;
    }
  }
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::writeEntity(
    Entity<Token> const & entity) const
{
  std::vector<Token *> const & paths = entity.getPaths();
  for (size_t i = 0; i < paths.size(); ++i)
  {
    Token const * token = paths[i];
    *_wordsStream << token->tokenString << "\t" << _docId << "\t"
        << static_cast<int>(token->completeSearchScore) << "\t"
        << token->completeSearchPosition << std::endl;
  }
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
string SemanticWikipediaWriter<MODE, Token>::getDocsStringForEntity(
    Entity<Token> const & entity) const
{
  std::vector<Token *> const & paths = entity.getPaths();
  ostringstream os;
  string entityDocString;
  // Prefix the path and cut off last part.
  for (size_t i = 0; i < paths.size(); ++i)
  {
    os << ENTITY_DOCS_PREFIX;
    os
        << paths[i]->tokenString.substr(0,
            paths[i]->tokenString.find_last_of(':') + 1);
  }
  entityDocString = os.str();
  // Replace all ':' with 'x'.
  std::replace(entityDocString.begin(), entityDocString.end(), ':', 'x');
  return entityDocString;
}


// ____________________________________________________________________________
template<unsigned char MODE, class Token>
bool SemanticWikipediaWriter<MODE, Token>::decidePreceedingSpace(
    Sentence<Token>const & sentence, int index) const
{
  vector<Token *> const & words = sentence.getWords();

  // No space before first word.
  if (index == 0) return false;

  string const & currWord = words[index]->tokenString;
  string const & prevWord = words[index - 1]->tokenString;

  // Decide space after previous word. No space after opening bracket etc.
  if (prevWord.length() == 1
      && (SPACE_CHAR_MAP[static_cast<uint8_t>(prevWord[0])] == 'a'
          || SPACE_CHAR_MAP[static_cast<uint8_t>(prevWord[0])] == 'c'))
    return false;

  // Decide space before current word.
  else
  {
    // Word length = 1.
    if (currWord.length() == 1)
    {
      // No space before ',' etc.
      if (SPACE_CHAR_MAP[static_cast<uint8_t>(currWord[0])] == 'b'
          || SPACE_CHAR_MAP[static_cast<uint8_t>(currWord[0])] == 'c')
        return false;
    }
    // Word length > 1
    else
    {
      if (std::find(_clitics.begin(), _clitics.end(), currWord)
          != _clitics.end()) return false;
    }
  }
  // All other words are preceded by a space.
  return true;
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::writeDecompositionInfoToDocs(
    Contexts<Token> const & contexts, size_t contextIndex,
    Sentence<Token>const & sentence,
    std::ofstream * outStream) const
{
  vector<Token *> const & words = sentence.getWords();
  *outStream << " " << DECOMPOSE_INFO_START;
  for (size_t i = 0; i < words.size(); ++i)
  {
    Token const * token = words[i];
    *outStream << token->tokenString << "_" << token->posTag << "_"
        << token->npTag << "_" << token->cTag << "_" << token->brTag << " ";
  }
  // Limit the number of contexts written to DECOMPOSE_INFO_MAX_CONTEXTS.
  if (contexts.size() > DECOMPOSE_INFO_MAX_CONTEXTS)
  {
    for (size_t i = 0; i < DECOMPOSE_INFO_MAX_CONTEXTS; ++i)
    {
      *outStream << DECOMPOSE_INFO_CONTEXT_START << " ";
      *outStream << contexts[i].asString();
    }
    // The next context is just "...".
    *outStream << DECOMPOSE_INFO_CONTEXT_START << " ";
    *outStream << "...";
  }
  else
  {
    for (size_t i = 0; i < contexts.size(); ++i)
    {
      *outStream << DECOMPOSE_INFO_CONTEXT_START << " ";
      *outStream << contexts[i].asString();
    }
  }
  *outStream << " " << DECOMPOSE_INFO_END;
}

// ____________________________________________________________________________
template<unsigned char MODE, class Token>
void SemanticWikipediaWriter<MODE, Token>::writeSentenceToDocs(
    Contexts<Token> const & contexts, size_t contextIndex,
    Sentence<Token>const & sentence,
    std::ofstream * outStream)const
{
  vector<Token *> const & words = sentence.getWords();
  // Create a copy of the vector we can delete from.
  Context<Token> contextCopy = contexts[contextIndex];
  typename Context<Token>::iterator it;
  ostringstream os;
  Entity<Token> const * lastWrittenEntity = NULL;
  // Was the previous word written highlighted?
  bool previousWordHighlighted = false;
  // Is the previous written word a highlight-end tag?
  bool previousWordEndTag = false;
  for (size_t i = 0; i < words.size(); ++i)
  {
    Token const * token = words[i];
    // Highlight the token if it is part of the context.
    if ((it = std::find(contextCopy.begin(), contextCopy.end(), token))
        != contextCopy.end())
    {
      // Only insert the tag to start if the previous token was not highlighted.
      if (!previousWordHighlighted)
      {
        os << " " << CONTEXT_HIGHLIGHT_START << " ";
        previousWordEndTag = false;
      }
      contextCopy.erase(it);
      previousWordHighlighted = true;
    }
    // If the previous token was highlighted and this one is not, insert the
    // tag to end highlighting.
    else if (previousWordHighlighted)
    {
      os << " " << CONTEXT_HIGHLIGHT_END << " ";
      previousWordHighlighted = false;
      previousWordEndTag = true;
    }

    // If the token belongs to an entity special treatment is required.
    if (token->isPartOfEntity)
    {
      // A new entity - write the special path word and precede with a space.
      if (lastWrittenEntity != token->_entity)
      {
        // Precede first entity word with a space?
        if (!previousWordEndTag && decidePreceedingSpace(sentence, i))
        {
          os << " ";
        }
        os << getDocsStringForEntity(*(token->_entity));
        os << WORD_HL_DOCS_PREFIX;
        lastWrittenEntity = token->_entity;
      }
      // Same entity - word should be highlighted but the
      // special word was written before and no space is required.
      else if (lastWrittenEntity == token->_entity)
      {
        os << WORD_HL_DOCS_PREFIX;
      }
    }
    // Word is not part of an entity. Just decide if a space is required.
    else if (!previousWordEndTag && decidePreceedingSpace(sentence, i))
    {
      os << " ";
    }
    // Now append the token.
    os << token->tokenString;
    previousWordEndTag = false;
  }
  // If the last word was highlighted we still need to insert the end
  // tag.
  if (previousWordHighlighted)
  {
    os << " " << CONTEXT_HIGHLIGHT_END;
    previousWordEndTag = true;
  }

  // Write to the output stream.
  *outStream << _docId << "\t" << "u:" << sentence.getDocUrl() << "\t"
      << "t:" << sentence.getDocTitle() << "\t" << "H:" << os.str();
  if (MODE & WRITE_DECOMP_INFO)
    writeDecompositionInfoToDocs(contexts, contextIndex
        , sentence, outStream);
  *outStream << std::endl;
}

template class SemanticWikipediaWriter<WRITE_DOCS_FILE + WRITE_DECOMP_INFO,
    DeepToken>;
template class SemanticWikipediaWriter<WRITE_DOCS_FILE, DeepToken>;
template class SemanticWikipediaWriter<DONT_WRITE_DOCS_FILE, DeepToken>;
template class SemanticWikipediaWriter<WRITE_DOCS_FILE + WRITE_DECOMP_INFO,
    DefaultToken>;
template class SemanticWikipediaWriter<WRITE_DOCS_FILE, DefaultToken>;
template class SemanticWikipediaWriter<DONT_WRITE_DOCS_FILE, DefaultToken>;
}
