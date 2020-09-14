// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_VOCABULARY_H_
#define SEMANTIC_WIKIPEDIA_SERVER_VOCABULARY_H_

#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./Identifiers.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
struct IdRange
{
    IdRange()
    : _first(), _last()
    {
    }

    IdRange(Id first, Id last)
    :_first(first), _last(last)
    {
    }

    Id _first;
    Id _last;
};

//! Stream operator for convenience.
inline std::ostream& operator<<(std::ostream& stream, const IdRange& idRange)
{
  return stream << '[' << idRange._first << ", " << idRange._last << ']';
}

class PrefixComparator
{
  public:
    explicit PrefixComparator(size_t prefixLength)
    : _prefixLength(prefixLength)
    {
    }

    bool operator()(const string& lhs, const string& rhs) const
    {
      return (lhs.size() > _prefixLength ? lhs.substr(0, _prefixLength) : lhs)
          < (rhs.size() > _prefixLength ? rhs.substr(0, _prefixLength) : rhs);
    }
  private:
    const size_t _prefixLength;
};
//! A vocabulary. Wraps a vector of strings
//! and provides additional methods for retrieval.
class Vocabulary
{
  public:
    Vocabulary();
    virtual ~Vocabulary();

    //! Read the vocabulary from file.
    void readFromFile(const string& fileName);

    //! Get a string version of the vocabulary that can be understood by humans.
    string asString(void) const;

    //! Append a word to the vocabulary. Wraps the std::vector method.
    void push_back(const string& word)
    {
      _words.push_back(word);
    }

    //! Get the word with the given id (as const, not as lvalue).
    const string& operator[](Id id) const
    {
      return _words[static_cast<size_t> (getPureValue(id))];
    }

    //! Get the number of words in the vocabulary.
    size_t size() const
    {
      return _words.size();
    }

    //! Reserve space for the given number of words.
    void reserve(unsigned int n)
    {
      _words.reserve(n);
    }

    //! Get an Id from the vocabulary for some ontology word.
    bool getIdForOntologyWord(const string& word, Id* id) const
    {
      Id index = lower_bound(word);
      *id = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + index;
      return _words[index] == word;
    }

    //! Get an Id from the vocabulary for some "normal" word.
    //! Return value signals if something was found at all.
    bool getIdForFullTextWord(const string& word, Id* id) const
    {
      *id = getFirstId(IdType::WORD_ID) + lower_bound(word);
      return _words[getPureValue(*id)] == word;
    }

    //! Get an Id range that matches a prefix.
    //! Should only be used with full-text vocabularies / words.
    //! Return value signals if something was found at all.
    bool getIdRangeForFullTextPrefix(const string& word, IdRange* range) const
    {
      AD_CHECK_EQ(word[word.size() - 1], PREFIX_CHAR);
      range->_first = getFirstId(IdType::WORD_ID) + lower_bound(
          word.substr(0, word.size() - 1));
      range->_last = getFirstId(IdType::WORD_ID) + upper_bound(
          word.substr(0, word.size() - 1), range->_first,
          PrefixComparator(word.size() - 1)) - 1;
      bool success = ad_utility::startsWith(
          _words[getPureValue(range->_first)],
          word.substr(0, word.size() - 1));
      if (success)
      {
        AD_CHECK_LT(range->_first, _words.size());
        AD_CHECK_LT(range->_last, _words.size());
      }
      return success;
    }

    //! Get an Id range that matches a prefix.
    //! Should only be used with full-text vocabularies / words.
    //! Return value signals if something was found at all.
    bool getIdRangeForOntologyPrefix(const string& prefix, IdRange* range) const
    {
      AD_CHECK_EQ(prefix[prefix.size() - 1], PREFIX_CHAR);
      range->_first = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + lower_bound(
          prefix.substr(0, prefix.size() - 1));
      range->_last = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + upper_bound(
          prefix.substr(0, prefix.size() - 1), getPureValue(range->_first),
          PrefixComparator(prefix.size() - 1)) - 1;
      bool success = ad_utility::startsWith(
          _words[getPureValue(range->_first)],
          prefix.substr(0, prefix.size() - 1));
      if (success)
      {
        AD_CHECK_LT(getPureValue(range->_first), _words.size());
        AD_CHECK_LT(getPureValue(range->_last), _words.size());
      }
      return success;
    }

  private:

    // Wraps std::lower_bound and returns an index instead of an iterator
    Id lower_bound(const string& word) const
    {
      return std::lower_bound(_words.begin(), _words.end(), word)
          - _words.begin();
    }

    // Wraps std::upper_bound and returns an index instead of an iterator
    // Only compares words that have at most word.size() or to prefixes of
    // that length otherwise.
    Id upper_bound(const string& word,
        size_t first, PrefixComparator comp) const
    {
      AD_CHECK_LT(first, _words.size());
      vector<string>::const_iterator it = std::upper_bound(
          _words.begin() + first, _words.end(), word, comp);
      Id retVal = (it == _words.end()) ? size() : it - _words.begin();
      AD_CHECK_LE(retVal, size());
      return retVal;
    }

    vector<string> _words;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_VOCABULARY_H_
