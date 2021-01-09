// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>

#include "./SimpleTextParser.h"
#include <ctype.h>   // for tolower
#include <string>



// bartsch: deleted low line '_' from separaror list
const char* SimpleTextParser::_defaultSeparators
= "!\"#$%&'()*+,-./@[\\]^{|}~:;<=>? ";

// const char* SimpleTextParser::_defaultSeparators
// = "!\"#$%&'()*+,-./@[\\]^_{|}~:;<=>? ";

// Constructor.
SimpleTextParser::SimpleTextParser()
{
  setSeparators(_defaultSeparators);
}

// Read delimiters from _map[256].
string SimpleTextParser::getSeparators()
{
  string delimiters = "";
  size_t maplength = sizeof(_map) / sizeof(_map[0]);
  for (size_t i = 0; i < maplength; i++)
  {
    if (_map[i]) delimiters += static_cast<char>(i);
  }
  return delimiters;
}

// Read delimiters to char _map[256].
void SimpleTextParser::setSeparators(const string& separators)
{
  // Reset _map. Everything is 0.
  size_t mapLength = sizeof(_map) / sizeof(_map[0]);
  for (size_t i = 0; i < mapLength; i++) _map[i] = 0;

  // Fill _map with delimiters.
  // Get ascii-code and set map[ascii-code] to 1
  for (size_t i = 0; i < separators.length(); i++)
  {
    unsigned char sep = static_cast<unsigned char>(separators[i]);
    int asciiCode = static_cast<int>(sep);
    assert(asciiCode >= 0);
    assert(asciiCode < 256);
    _map[asciiCode] = 1;
  }
}


// Test iff char is delimiter.
bool SimpleTextParser::isDelimiter(char c)
{
  int asciiCode = static_cast<int>(static_cast<unsigned char>(c));
  return _map[asciiCode];
}


// Parse string -> return values: word_start, word_end.
void SimpleTextParser::parseText(const string& text,
                                 size_t* word_start,
                                 size_t* word_end)
{
  assert(*word_end >= 0);
  if (*word_end >= text.length())
  {
    fprintf(stderr, "85:parseText() failed on string \"%s\" (%zu >= %zu)\n",
            text.c_str(), *word_end, text.length());
  }
  assert(*word_end < text.length());

  size_t textLength = text.length();
  size_t pos = *word_end;
  while (pos < textLength && isDelimiter(text[pos])) pos++;
  *word_start = pos;
  while (pos < textLength && !isDelimiter(text[pos])) pos++;
  *word_end = pos;
}

