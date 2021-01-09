// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>

#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <iomanip>
#include "./StringConversion.h"

using std::endl;
using std::cout;
using std::flush;


const unsigned int TEXTSIZE = 100 * 1000 * 1000;

// Print comment, iff somebody called StringConversionPerf with arguments.
void printUsage()
{
  cout << "Usage: Call StringConversionPerf without arguments!" << endl;
}


// Class for performance testing. TODO: more details please.
class StringConversionPerf
{
 public:
  // Initialize parser (with default separators).
  StringConversionPerf() {}

  // Show statistics for given time and text length of utf8_tolower.
  void showStatistics(size_t textLength, double timeForEditingInMsecs);

  // Measure performance of utf8ToLower on various texts.
  void perf_utf8ToLower();

  // Measure performance of iso88591ToLower on various texts.
  void perf_iso88591ToLower();

  // Measure average time of a call to setlocale.
  void perfLocale();

 private:
  StringConversion _converter;
};

// _____________________________________________________________________________
void StringConversionPerf::showStatistics(size_t textLength,
                                               double timeForEditingInMsecs)
{
  cout.setf(std::ios::fixed);
  cout.precision(0);
  double textSizeInMb = textLength / (1024 * 1024);
  double mbPerSecond = 1000 * textSizeInMb / timeForEditingInMsecs;
  cout << "Length of string   : " << std::setw(6) << textSizeInMb
                                             << " MB" << endl;
  cout << "Time for editing   : " << std::setw(6) << timeForEditingInMsecs
                                             << " msecs" << endl;
  cout << "Editing rate       : " << std::setw(6) << mbPerSecond
                                             << " MB/second" << endl << endl;
}


// _____________________________________________________________________________
void StringConversionPerf::perf_iso88591ToLower()
{
  _converter.setEncoding(StringConversion::ISO_8859_1);
  string word = "Minze";

  // Time utf8_tolower for a large string with many normally-sized words.
  size_t numWords = TEXTSIZE / word.length();

  clock_t c_start, c_end;
  cout << endl;
  for (int i = 1; i <= 2; i++)
  {
    cout << i << ". Making all letters lower in a text with " << numWords
              << " words of size " << word.length()
              << " each ... " << endl;

    c_start = clock();
    for (size_t j = 0; j < numWords; j++)
    {
      _converter.iso88591ToLower(&word);
    }
    c_end = clock();
    double editTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

    // Show statistics.
    showStatistics(TEXTSIZE, editTimeInMsecs);
  }

  // Time utf8_tolower for a large string with a single very long word.
  word = "B";
  for (size_t i = 1; i < TEXTSIZE; i++)
  {
    word = word.append("a");
  }

  cout << endl;
  for (int i = 1; i <= 2; i++)
  {
    cout << i << ". Making all letters lower in a string with one word of size "
              << TEXTSIZE << " ... " << endl;

    // Performance Test by editing whole string "text".
    c_start = clock();
    _converter.iso88591ToLower(&word);
    c_end = clock();
    double editTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

    // Show statistics.
    showStatistics(TEXTSIZE, editTimeInMsecs);
  }
}

// _____________________________________________________________________________
void StringConversionPerf::perf_utf8ToLower()
{
  _converter.setEncoding(StringConversion::UTF_8);
  string word = "Minze"; // NOLINT

  // Time utf8_tolower for a large string with many normally-sized words.
  size_t numWords = TEXTSIZE / word.length();

  clock_t c_start, c_end;
  cout << endl;
  for (int i = 1; i <= 2; i++)
  {
    cout << i << ". Making all letters lower in a text with " << numWords
              << " words of size " << word.length()
              << " each ... " << endl;

    c_start = clock();
    for (size_t j = 0; j < numWords; j++)
    {
      _converter.utf8ToLower(&word);
    }
    c_end = clock();
    double editTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

    // Show statistics.
    showStatistics(TEXTSIZE, editTimeInMsecs);
  }

  // Time utf8_tolower for a large string with a single very long word.
  word = "B";
  for (size_t i = 1; i < TEXTSIZE; i++)
  {
    word = word.append("a");
  }

  cout << endl;
  for (int i = 1; i <= 2; i++)
  {
    cout << i << ". Making all letters lower in a string with one word of size "
              << TEXTSIZE << " ... " << endl;

    // Performance Test by editing whole string "text".
    c_start = clock();
    _converter.utf8ToLower(&word);
    c_end = clock();
    double editTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

    // Show statistics.
    showStatistics(TEXTSIZE, editTimeInMsecs);
  }
}


// _____________________________________________________________________________
void StringConversionPerf::perfLocale()
{
  unsigned int n = 1000 * 1000;
  cout << "Calling setlocale " << n << " times ... " << flush;
  clock_t t = clock();
  for (unsigned int i = 0; i < n; ++i)
  {
    setlocale(LC_ALL, "en_US.utf-8");
    setlocale(LC_ALL, "POSIX");
  }
  size_t usecs = (size_t)(clock() - t);
  cout << "done in " << usecs / 1000 << " msecs"
       << " (" << usecs / (2 * n) << " usecs / setlocale)" << endl;
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

  cout << "----------------" << endl;
  cout << "Performance Test" << endl;
  cout << "----------------" << endl;

  StringConversionPerf perf;

  cout << "Performance test for StringConversionPerf::iso88591ToLower" << endl
       << endl;
  perf.perf_iso88591ToLower();
  cout << "Performance test for StringConversionPerf::utfToLower" << endl
       << endl;
  perf.perf_utf8ToLower();
}
