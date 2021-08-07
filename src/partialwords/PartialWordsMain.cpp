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
  if (argc != 2) {
    printf("Usage: ./PartialWordsMain <vocabulary file>\n");
    exit(1);
  }
  std::string file_name = argv[1];
  size_t max_n = 100;  // For showInput and showOutput only.

  // Read vocabulary from file and build suffix array.
  PartialWords pw;
  pw.readVocabulary(file_name);
  pw.buildSuffixArray();
  pw.showSuffixArrayInputText(max_n);
  pw.showSuffixArray(max_n);
  pw.findPartialWords(6);
  pw.showPartialWordMatches();
}
