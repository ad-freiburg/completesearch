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
#include <vector>

const size_t MAX_SIZE_T = std::numeric_limits<size_t>::max();

// Class for determining which words in a given vocabulary are contained in
// other words.
class PartialWords {
 public:
  // Read vocabulary from given file.
  void readVocabulary(std::string file_name);

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
  // Vocabulary.
  std::vector<std::string> _vocabulary;

  // Total size of vocabulary (sum of words lengths without terminating 0).
  size_t _vocabularyTotalSize = 0;

  // Partial word matches: the entry at position i pertains to the i-th word
  // from the vocabulary and contains the indices of all words contained in that
  // word.
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
