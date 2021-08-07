// Copyright 2021, University of Freiburg,
// Chair of Algorithms and Data Structures
// Author: Hannah Bast <bast@cs.uni-freiburg.de>

#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include "./PartialWords.h"

// ____________________________________________________________________________
PartialWords::~PartialWords() {
  delete[] _text;
  delete[] _SA;
  delete[] _LCP;
  delete[] _DA;
}

// ____________________________________________________________________________
void PartialWords::readVocabulary(std::string file_name) {
  std::ifstream file(file_name.c_str());
  std::string line;
  while (getline(file, line)) {
    _vocabulary.push_back(line);
    _vocabularyTotalSize += line.size();
    // count++; printf("Line %5d: %s\n", count, line.c_str());
  }
}

// ____________________________________________________________________________
void PartialWords::buildSuffixArray() {
  // Concatenate words from vocabulary, with '\x01' after each word and one
  // '\x00' the very end.
  size_t n = _vocabularyTotalSize + _vocabulary.size() + 1;
  _text = new unsigned char[n];
  size_t sizeCopied = 0;
  for (const auto& word : _vocabulary) {
    word.copy((char*)(_text + sizeCopied), word.size());
    sizeCopied += word.size();
    _text[sizeCopied] = '\x01';
    sizeCopied++;
  }
  _text[sizeCopied] = '\x00';
  assert(sizeCopied + 1 == n);
  // Allocate space for the output arrays.
  _SA = new uint_t[n];
  _LCP = new int_t[n];
  _DA = new int_t[n];
  // Compute the the arrays.
  gsacak(_text, _SA, _LCP, _DA, n);
  // Throw away LCP (we don't need it).
  delete[] _LCP;
  _LCP = NULL;
}

// ____________________________________________________________________________
void PartialWords::showSuffixArrayInputText(size_t max_n) const {
  size_t n = _vocabularyTotalSize + _vocabulary.size() + 1;
  for (size_t i = 0; i < n - 1; i++) {
    if (i > max_n) { printf("..."); break; }
    printf("%c", _text[i] != 1 ? _text[i] : '$');
  }
  printf("#\n");
}

// ____________________________________________________________________________
void PartialWords::showSuffixArray(size_t max_n) const {
  size_t n = _vocabularyTotalSize + _vocabulary.size() + 1;
  printf("i\tSA\tDA\tBWT\tsuffixes\n");
  // printf("i\tSA\tDA\tLCP\tBWT\tsuffixes\n");
  for(int i = 0; i < n; i++) {
    if (i > max_n) { printf("...\n"); break; }
    char c = _SA[i] != 0 ? _text[_SA[i] - 1] : '#';
    if (c == '\x01') c = '$';
    printf("%d\t%d\t%d\t%c\t", i, _SA[i], _DA[i], c);
    int j;
    for (j = _SA[i]; j < n - 1; j++) {
      printf("%c", _text[j] != '\x01' ? _text[j] : '$');
      if (j - _SA[i] > 50) break;
    }
    if (j < n - 1) printf("...");
    printf("#\n");
  }
}

// ____________________________________________________________________________
void PartialWords::findPartialWords(size_t min_word_length) {
  // Iterate over suffix array and stop at each beginning of a new word. Omit
  // the very first entry (which is the hypothetical empty word at the very end,
  // before the terminating zero, hence int i = 1 and not 0 in the for loop).
  //
  // NOTE that by definition of the suffix array, this will produce the words
  // from the vocabulary in sorted order.
  bool verbose = false;
  _partialWordMatches.resize(_vocabulary.size());
  size_t n = _vocabularyTotalSize + _vocabulary.size() + 1;
  for (int i = 1; i < n; i++) {
    // Get BWT[i]. This is simply the character in _text that precedes the
    // suffix with rank i.
    char bwt_i = _SA[i] == 0 ? '\x00' : _text[_SA[i] - 1];
    if (bwt_i == '\x00' || bwt_i == '\x01') {
      // Compute length of word (we are at the beginning of a word).
      if (verbose) printf("Word #%d: ", _DA[i]);
      int j = _SA[i];
      while (j < n - 1 && _text[j] != '\x01') {
	if (verbose) printf("%c", _text[j]);
	j++;
      }
      size_t length = j - _SA[i];
      if (verbose) printf(" [length: %zu; vocabulary check: %s]\n",
	  length, _vocabulary[_DA[i]].c_str());

      // Partial words should have at least four four characters.
      if (length < min_word_length) continue;

      // Lambda that checks whether _SA[ii] is inside of a word (1) and if yes
      // whether the word starting at _SA[i] is a prefix (2). If both (1) and
      // (2) are true, _partialWordMatches is updated accordingly. Returns false 
      // only if (1) is true, but (2) is false.
      //
      // NOTE: We are not interested in finding prefix matches because we
      // already find those via prefix search.
      auto checkPartialMatch = [&](size_t ii) {
	// Skip suffixes at the start of a word.
	if (_SA[ii] == 0 || _text[_SA[ii] - 1] == '\x01') return true;
	// Prefix check -> abort loop once this fails.
	int max_length = std::min(length, n - _SA[ii]);
	const char* word_1 = (char*)(_text + _SA[i]);
	const char* word_2 = (char*)(_text + _SA[ii]);
	if (strncmp(word_1, word_2, max_length) == 0) {
	  // We have a match: word _DA[i] is contained in word _DA[ii].
	  _partialWordMatches[_DA[ii]].push_back(_DA[i]);
	  _partialWordMatchesTotalNumber++;
	  if (verbose) printf("PARTIAL WORD MATCH FOUND: %s contained in %s\n",
	      _vocabulary[_DA[i]].c_str(), _vocabulary[_DA[ii]].c_str());
	  return true;
	} else {
	  return false;
	}
      };
      // Search both forward and backward.
      for (int ii = i + 1; ii < n && checkPartialMatch(ii); ii++) {}
      for (int ii = i - 1; ii > 0 && checkPartialMatch(ii); ii--) {}
    }
  }
}

// ____________________________________________________________________________
void PartialWords::showPartialWordMatches() {
  assert(_partialWordMatches.size() == _vocabulary.size());
  for (size_t i = 0; i < _partialWordMatches.size(); i++) {
    if (_partialWordMatches[i].size() == 0) continue;
    printf("%s contains:", _vocabulary[i].c_str());
    for (auto j : _partialWordMatches[i])
      printf(" %s", _vocabulary[j].c_str());
    printf("\n");
  }
  printf("Total number of partial word matches: %zu\n",
      _partialWordMatchesTotalNumber);
}
