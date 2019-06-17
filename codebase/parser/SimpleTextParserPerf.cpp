// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>

#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <iomanip>
#include "./SimpleTextParser.h"

using std::endl;
using std::cout;
using std::flush;

// Print comment, iff somebody called SimpleTextParserPerf with arguments.
void printUsage()
{
  cout << "Usage: Call SimpleTextParserPerf without arguments!" << endl;
}


// Class for performance testing. TODO: more details please.
class SimpleTextParserPerf
{
 public:
  // Initialize parser (with default separators).
  SimpleTextParserPerf() {}

  // Create random text with given number of words of given word length.
  void randomText(size_t numWords, size_t wordLength, string* text);

  // Show statistics for given time and text length.
  void showStatistics(size_t textLength, double timeForParsingInSeconds);

  // Measure performance of parseText function on the given text.
  void perfParseText(const string& text);

  // Measure performance of parseText on various texts.
  void perfParseText();

  private:
  SimpleTextParser _parser;
};


// _____________________________________________________________________________
void SimpleTextParserPerf::showStatistics(size_t textLength,
                                          double timeForParsingInMsecs)
{
  cout.setf(std::ios::fixed);
  cout.precision(0);
  double textSizeInMb = textLength / (1024 * 1024);
  double mbPerSecond = 1000 * textSizeInMb / timeForParsingInMsecs;
  cout << "Length of string   : " << std::setw(6) << textSizeInMb
                                             << " MB" << endl;
  cout << "Time for parsing   : " << std::setw(6) << timeForParsingInMsecs
                                             << " msecs" << endl;
  cout << "Parsing rate       : " << std::setw(6) << mbPerSecond
                                             << " MB/second" << endl << endl;
}

// _____________________________________________________________________________
void SimpleTextParserPerf::randomText(size_t numWords,
                                      size_t wordLength, string* text)
{
  assert(text != NULL);
  assert(text->size() == 0);
  cout << "Generating random text with " << numWords
       << (numWords == 1 ? " word" : " words") << " of size "
       << wordLength << " each ... " << std::flush;
  const string& separators = _parser.getSeparators();
  size_t separatorsLength = separators.length();
  char ch = 0;
  unsigned int seed = 1;

  for (size_t i = 0; i < numWords; ++i)
  {
    for (size_t j = 0; j < wordLength; ++j)
    {
      size_t up_or_low = (rand_r(&seed) % 2);
      switch (up_or_low)
      {
        // Uppercase
        case 0: ch = static_cast<char>((rand_r(&seed) % 26) + 65); break;
        // Lowercase
        case 1: ch = static_cast<char>((rand_r(&seed) % 26) + 97); break;
        default: cout << "Something went wrong." << endl;
      }
      (*text) += ch;
    }
    char randomSeparator = separators[rand_r(&seed) % separatorsLength];
    (*text) += randomSeparator;
  }
  cout << "done." << endl;
}


// _____________________________________________________________________________
void SimpleTextParserPerf::perfParseText(const string& text)
{
  // Initialising variables for parsing.
  size_t textLength = text.length();
  size_t start = 0;
  size_t end = 0;
  clock_t c_start, c_end;

  // Performance Test by parsing whole string "text".
  c_start = clock();
  while (end < textLength) _parser.parseText(text, &start, &end);
  c_end = clock();
  double parseTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

  // Show statistics.
  showStatistics(textLength, parseTimeInMsecs);
}


// _____________________________________________________________________________
void SimpleTextParserPerf::perfParseText()
{
  unsigned int textSize = 100 * 1000 * 1000;
  unsigned int wordLength = 5;
  string text;

  // Time parseText for a large string with many normally-sized words.
  size_t numWords = textSize / wordLength;
  randomText(numWords, wordLength, &text);
  cout << endl;
  for (int i = 1; i <= 3; i++)
  {
    cout << i << ". Parsing text with " << numWords
              << " words of size " << wordLength
              << " each ... " << endl;
    perfParseText(text);
  }

  // Time parseText for a large string with a single very long word.
  text.clear();
  randomText(1, textSize, &text);
  cout << endl;
  for (int i = 1; i <= 3; i++)
  {
    cout << i << ". Parsing text with one word of size "
              << textSize << " ... " << endl;
    perfParseText(text);
  }
}


// _____________________________________________________________________________
int main(int argc, char** argv)
{
  // Check for arguments.
  if (argc != 1)
  {
    printUsage();
    exit(1);
  }

  cout << "--------------------" << endl;
  cout << "SimpleTextParserPerf" << endl;
  cout << "--------------------" << endl;

  SimpleTextParserPerf perf;

  cout << "Performance test for SimpleTextParser::parseText" << endl << endl;
  perf.perfParseText();
}
