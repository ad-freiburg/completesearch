// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>

#ifndef PARSER_SIMPLETEXTPARSER_H_
#define PARSER_SIMPLETEXTPARSER_H_

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <vector>

using std::string;
using std::wstring;


class SimpleTextParser
{
 public:

  // Default constructor. Uses default separators.
  SimpleTextParser();

  // Parse text starting from word_end. Will set word_start to the position of
  // the first character of the next word, an word_end to one after the last
  // character of that word. Note that this way the function can be easily
  // called repeatedly to extract all words from the given text.
  void parseText(const string& text, size_t* word_start, size_t* word_end);

  // Set separators.
  // Each character of the given string is taken as a word separator.
  void setSeparators(const string& separators);

  // Get separators.
  string getSeparators();

  // Return true iff given character is delimiter.
  bool isDelimiter(char ch);

 private:
  // Default set of separators.
  static const char* _defaultSeparators;

  // Map each possible character to a value saying
  // whether it's a separator or a word character.
  bool _map[256];
};

#endif  // PARSER_SIMPLETEXTPARSER_H_
