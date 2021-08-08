// Copyright 2021, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast@cs.uni-freiburg.de>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "./PartialWords.h"


// Main function.
int main(int argc, char** argv) {
  // Parse command line argument and usage info.
  if (argc != 2 && argc != 3 && argc != 4) {
    printf("Usage: ./PartialWordsMain <primary vocabulary file>"
	   " [<secondary vocabulary file>] [<min_word_length]\n");
    exit(1);
  }
  std::string primary_vocabulary_file_name = argv[1];
  std::string secondary_vocabulary_file_name = argc >= 3 ? argv[2] : "";
  size_t min_word_length = argc >= 4 ? atoi(argv[3]) : 6;
  size_t max_n = VERBOSITY >= 2 ? MAX_SIZE_T : 100; // For show... below.

  // Read vocabulary from file and build suffix array.
  PartialWords pw;
  pw.readVocabularies(primary_vocabulary_file_name,
                      secondary_vocabulary_file_name);
  bool verbose = pw.vocabularySize() <= 100;
  pw.buildSuffixArray();
  if (VERBOSITY >= 1) pw.showSuffixArrayInputText(max_n);
  if (VERBOSITY >= 1) pw.showSuffixArray(max_n);
  pw.findPartialWords(min_word_length);
  pw.showPartialWordMatches();
}
