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
void PartialWords::readVocabularies(
    std::string primary_vocabulary_file_name,
    std::string secondary_vocabulary_file_name) {
  // Lambda that reads words from `file_name` and appends them to
  // `_vocabulary'.
  auto readVocabulary = [&](std::string file_name) {
    std::ifstream file(file_name.c_str());
    std::string line;
    while (getline(file, line)) {
      _vocabulary.push_back(line);
      _vocabularyTotalSize += line.size();
      // count++; printf("Line %5d: %s\n", count, line.c_str());
    }
  };
  // Read first the primary vocabulary, then the secondary vocabulary and
  // remember the start of the secondary vocabulary. If the file name for the
  // secondary vocabulary is empty, skip that part.
  assert(primary_vocabulary_file_name != "");
  readVocabulary(primary_vocabulary_file_name);
  _primaryVocabularySize = _vocabulary.size();
  if (secondary_vocabulary_file_name != "")
    readVocabulary(secondary_vocabulary_file_name);
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
  size_t n = _vocabularyTotalSize + _vocabulary.size() + 1;
  _partialWordMatches.resize(_primaryVocabularySize);
  for (int i = 1; i < n; i++) {
    // Get BWT[i]. This is simply the character in _text that precedes the
    // suffix with rank i.
    char bwt_i = _SA[i] == 0 ? '\x00' : _text[_SA[i] - 1];
    if (bwt_i == '\x00' || bwt_i == '\x01') {
      // Compute length of word (we are at the beginning of a word).
      if (VERBOSITY >= 2) printf("Word #%d: ", _DA[i]);
      int j = _SA[i];
      while (j < n - 1 && _text[j] != '\x01') {
	if (VERBOSITY >= 2) printf("%c", _text[j]);
	j++;
      }
      size_t length = j - _SA[i];
      if (VERBOSITY >= 2) printf(" [length: %zu; vocabulary check: %s]\n",
	  length, _vocabulary[_DA[i]].c_str());

      // Skip words that are shorter than the specified threshold.
      if (length < min_word_length) continue;

      // Lambda that checks whether _SA[ii] fulfills the following three
      // conditions:
      //
      // (1) The word _DA[ii] is from the primary vocabulary. REASON: we
      // consider words from the secondary vocabulary (typically: a dictionary
      // of words) as partial word matches, but we don't want to find partial
      // word matches in words from the secondary vocabulary.
      //
      // (2) _SA[ii] is not at the beginning of word _DA[i] . REASON: we are
      // not interested in finding partial word matches at the beginning of a
      // word (for example, "mittel" as partial word of "mittelzuweisung"
      // because we already find those via prefix search).
      //
      // (3) The word starting at _SA[i] is a prefix of the suffix starting at
      // _SA[ii]. REASON: this is the definition of a partial word match.
      //
      // If (1) or (2) are not fulfilled, don't record a partial match, but
      // return `true` indicating that we continue searching for partial matches.
      //
      // If (1) and (2) are fulfilled, but (3) is false, don't record a partial
      // match and return `false` because we know that all suffixes after this
      // (in forward search) or before this (in backward search) cannot be
      // matches anymore.
      //
      auto checkPartialMatch = [&](size_t ii) {
	// Skip suffixes at the start of a word (see comment above).
	if (_SA[ii] == 0 || _text[_SA[ii] - 1] == '\x01') return true;

	// Skip matches in words from the secondary vocabulary.
	if (_DA[ii] >= _primaryVocabularySize) return true;

        // HACK: Also skip CompleteSearch's special words (starting with a colon).
	//
        // TODO: This should be properly congifurable, it is not necessarily a
        // colon and we might not only use this code for CompleteSearch.
	//
	// TODO: Does not work, why?
        // if (_text[_SA[ii]] == ':') return true;

	// Check whether word _DA[i] is contained in word _DA[ii].
	int strncmp_max_length = std::min(length, n - _SA[ii]);
	const char* word_1 = (char*)(_text + _SA[i]);
	const char* word_2 = (char*)(_text + _SA[ii]);
	if (strncmp(word_1, word_2, strncmp_max_length) == 0) {

	  // First check whether word _DA[i] (which starts at _text + _SA[i])
	  // is a continuation of the previously found partial word match for
	  // word _DA[ii]. For example, we have previously found "möglich" and
	  // now find "möglichkeiten".
	  // 
	  // NOTE: If a word W has k partial matches P1, ..., Pk, where Pi+1 is
	  // a continuation of Pi, then our algorithm encouters them in this
	  // order (1) and without any other partial match inbetween that does
	  // not fit into this sequence (2).
	  //
	  // (1) This is obvious because the end-of-word symbol '\x01' is
	  // smaller than any character.
	  //
	  // (2) For example, a partial word match that is lexicographically
	  // between "möglich" and "möglichkeiten", has to be a continuation C
	  // of "möglich". If we assume that W has only one occurrence of
	  // "möglich" (and hence "möglichkeiten"), then C has to be a
	  // prefix of "möglichkeiten" and would hence fit in the sequence
	  // between "mögliche" and "möglichkeiten".
	  //
	  // NOTE: Since some words can be contained in both vocabularies, it
	  // can also happen that we find the exact same partial match twice.
	  // We can find those by Cnot looking for strict prefixe (hence the
	  // length >= previousMatch.size() below, and not ... > ...).
	  bool previousMatchIsPrefix = false;
	  size_t numMatches = _partialWordMatches[_DA[ii]].size();
	  if (numMatches > 0) {
	    const std::string& previousMatch =
	      _vocabulary[_partialWordMatches[_DA[ii]][numMatches - 1]];
	    if (length >= previousMatch.size() &&
		strncmp((char*)(_text + _SA[i]), previousMatch.c_str(),
		  previousMatch.size()) == 0)
	      previousMatchIsPrefix = true;
	  }

	  // If previous partial match for this word is a prefix, replace it,
	  // otherwise add the new partial match.
	  if (previousMatchIsPrefix) {
	    _partialWordMatches[_DA[ii]][numMatches - 1] = _DA[i];
	  } else {
	    _partialWordMatches[_DA[ii]].push_back(_DA[i]);
	    _partialWordMatchesTotalNumber++;
	  }
	  if (VERBOSITY >= 2)
	    printf("PARTIAL WORD MATCH FOUND: %s contained in %s\n",
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
  assert(_partialWordMatches.size() == _primaryVocabularySize);
  for (size_t i = 0; i < _partialWordMatches.size(); i++) {
    if (_partialWordMatches[i].size() == 0) continue;
    if (PARTIAL_WORDS_OUTPUT_COMPACT) {
      // COMPACT OUTPUT: all partial matches for a word in one line.
      printf("%s contains:", _vocabulary[i].c_str());
      for (auto j : _partialWordMatches[i])
        printf(" %s", _vocabulary[j].c_str());
      printf("\n");
    } else {
      // STANDARD OUTPUT: one line per partial match.
      for (auto j : _partialWordMatches[i]) {
        printf("%s\t%s:%s\n", _vocabulary[i].c_str(),
            _vocabulary[j].c_str(), _vocabulary[i].c_str());
      }
    }
  }
  if (VERBOSITY >= 1) printf("Total number of partial word matches: %zu\n",
      _partialWordMatchesTotalNumber);
}
