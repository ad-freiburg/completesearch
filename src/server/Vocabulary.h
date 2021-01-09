// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#ifndef SERVER_VOCABULARY_H_
#define SERVER_VOCABULARY_H_

#include <gtest/gtest.h>
#include <limits.h>
#include <vector>
#include <string>
#include "./Globals.h"

using std::vector;
using std::string;

/// The vocabulary of a document collection. Implemented as a simple vector of
/// strings, sorted lexicographically.
class Vocabulary
{
 public:

  /// Dump object to string.
  std::string asString(void) const;

  /// Append a word to the vocabulary.
  void push_back(const string& word);

  /// Get the word with the given id (as const, not as lvalue).
  const string& operator[](unsigned int id) const;

  /// Get the number of words in the vocabulary.
  unsigned int size() const { return _words.size(); }

  /// Reserve space for the given number of words.
  void reserve(unsigned int n) { _words.reserve(n); }

  /// Get last word (empty string if vocabulary is empty).
  string getLastWord() { return _words.size() > 0 ? _words.back() : ""; }

  /// Return true iff x is strictly less than y, but also "helloanything" <
  /// "hello*".
  bool lessThan(const string& x, const string& y) const;

  /// Search for a word in the dictionary. Returns index of the first word in
  /// the given range that is not strictly smaller. The default range is the
  /// whole dictionary.
  unsigned int findWord(const    string& word,
                        unsigned int     low  = 0,
                        unsigned int     high = UINT_MAX) const;
  FRIEND_TEST(VocabularyTest, findWord);

  /// Precompute the word id map. Checks that _wordIdMap is empty, assert
  /// failure otherwise.
  void precomputeWordIdMap();
  FRIEND_TEST(VocabularyTest, precomputeWordIdMap);

  /// Get the word id which the given word id is mapped to. If _wordIdMap is empty, this is
  /// simply the identity function, that is the given word id is returned.
  WordId mappedWordId(WordId wordId);
  FRIEND_TEST(VocabularyTest, mappedWordId);

  /// Return true if we have a non-trivial word id map (_wordIdMap.size() > 0)
  /// and false otherwise.
  bool isWordMapTrivial() { return _wordIdMap.size() == 0; }

 private:
  /// The words of the vocabulary, sorted in lexicogaphic order.
  vector<string> _words;

  /// A mapping of word ids to words ids. For example, used to map the word id
  /// of C:1234:algorithm to the word id of algorithm. Used in
  /// CompleterBase::computeTopCompletion.
  vector<WordId> _wordIdMap;
  FRIEND_TEST(CompleterBaseTest, remapWordIds);

  friend class VocabularyTest;
};

#endif  // SERVER_VOCABULARY_H_
