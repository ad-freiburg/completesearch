// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "./SimpleTextParser.h"

using std::endl;
using std::cout;

const size_t MAPLENGTH = 256;

// Testing function SimpleTextParser::setSeparators.
TEST(SimpleTextParserTest, setSeparators)
{
  SimpleTextParser stp;
  string delimiters = "!?.";
  stp.setSeparators(delimiters);
  for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
  {
    char c = static_cast<char>(i);
    bool isDelimiter = delimiters.find(c) != string::npos;
    ASSERT_EQ(isDelimiter, stp.isDelimiter(c))
      << "character: " << c << " (code: " << i << ")";
  }
}

// Testing function SimpleTextParser::parseText.
TEST(SimpleTextParserTest, parseText)
{
  // Use _defaultSeparators.
  SimpleTextParser stp;
  stp.setSeparators(" ,.:;$%§&/()=");

  // Create a text of n + 1 words (all "wort"), with one separator between
  // adjacent words, using each possible separator exactly once.
  string separators = stp.getSeparators();
  string text;
  for (size_t i = 0; i < separators.length() + 1; i++)
  {
    text += "wort";
    if (i < separators.length()) text += separators[i];
  }

  // Check that parseText does the right thing. Note that the initial value of
  // start does not matter, hence the value of 999.
  size_t start = 999;
  size_t end = 0;
  size_t expected_start = 0;
  size_t expected_end = 4;
  while (end < text.length())
  {
    stp.parseText(text, &start, &end);
    ASSERT_EQ(expected_start, start);
    ASSERT_EQ(expected_end, end);
    expected_start += 5;
    expected_end += 5;
  }

  // Test text with delimiters after last word. parseText should then return
  // start = end = text length.
  text = "wort";
  for (size_t i = 0; i < separators.length(); i++) text += separators[i];
  start = 999;
  end = 0;
  stp.parseText(text, &start, &end);
  ASSERT_EQ((unsigned) 0, start);
  ASSERT_EQ((unsigned) 4, end);
  stp.parseText(text, &start, &end);
  ASSERT_EQ(start, text.length());
  ASSERT_EQ(end, text.length());
}

