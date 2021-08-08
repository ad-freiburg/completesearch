// Copyright 2021, University of Freiburg,
// Chair of Algorithms and Data Structures
// Author: Hannah Bast <bast@cs.uni-freiburg.de>

#ifndef PARTIAL_WORDS_H
#define PARTIAL_WORDS_H

extern "C" {
#include "gsacak.h"
}

#include <limits>
#include <string>
#include <utility>
#include <vector>

const size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();
const int VERBOSITY = 0;
const bool PARTIAL_WORDS_OUTPUT_COMPACT = false;

// Class for determining which words in a given vocabulary are contained in
// other words.
//
// The class distinguishes between two vocabularies, called primary and
// secondary. Words from both vocabularies are candidates for partial words. But
// we compute and output partial words only for words from the primary vocabulary.
//
// The typical use is that the primary vocabulary is the vocabulary of the
// collection we want to search and the secondary vocabulary is a general
// dictionary of the language. That way, if the first vocabulary contains the
// word "dienstanweisung" and the word "anweisung" is not in the first
// vocabulary, but in the second, we find "anweisung" as partial word of
// "dienstanweisung" anyway.
//
// The secondary vocabulary can be empty.
class PartialWords {
 public:
  // Read primary and secondary vocabulary from the given files (see the comment
  // above why two vocabularies). If the second file name is empty, we only read
  // the primary vocabulary.
  void readVocabularies(std::string primary_vocabulary_file_name,
                        std::string secondary_vocabulary_file_name);

  // Get size of vocabulary.
  size_t vocabularySize() const { return _vocabulary.size(); }

  // Free memory allocated for suffix array input and output.
  ~PartialWords();

  // Build suffix array.
  void buildSuffixArray();

  // Find partial words from suffix array (we need: SA and DA).
  void findPartialWords(size_t min_word_length = 4);
 
  // Show input for suffix array computation (for testing).
  void showSuffixArrayInputText(size_t max_n = MAX_SIZE_T) const;

  // Show output from suffix array computation (for testing).
  void showSuffixArray(size_t max_n = MAX_SIZE_T) const;

  // Show partial word matches found.
  void showPartialWordMatches();

 private:
  // Vocabulary (first all words from the primary vocabulary, then all words
  // from the secondary vocabulary).
  std::vector<std::string> _vocabulary;

  // Index of the first word in the secondary vocabulary.
  size_t _primaryVocabularySize = 0;

  // Total size of vocabulary (sum of words lengths without terminating 0).
  size_t _vocabularyTotalSize = 0;

  // Partial word matches: the entry at position i are the partial matches for
  // the i-th word from the vocabulary. Each partial match is represented by its
  // index in the vocabulary.
  std::vector<std::vector<size_t>> _partialWordMatches;

  // Total number of partial word matches found.
  size_t _partialWordMatchesTotalNumber = 0;

  // Strings from suffix array.
  unsigned char* _text = NULL;
  uint_t* _SA = NULL;
  int_t* _LCP = NULL;
  int_t* _DA = NULL;
};

#endif  // PARTIAL_WORDS_H
