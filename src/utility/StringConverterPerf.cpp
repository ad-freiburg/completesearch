// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>, Wolfgang Bartsch <weitkaemper>

#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <iomanip>
#include "./StringConverter.h"

using std::endl;
using std::cout;
using std::flush;


const unsigned int TEXTSIZE = 100 * 1000 * 1000;

// Print comment, iff somebody called StringConverterPerf with arguments.
void printUsage()
{
  cout << "Usage: Call StringConverterPerf without arguments!" << endl;
}


// Class for performance testing. TODO: more details please.
class StringConverterPerf
{
 public:
  // Initialize parser (with default separators).
  StringConverterPerf() {}

  // Show statistics for given time and text length of utf8_tolower.
  void showStatistics(size_t textLength, double timeForEditingInMsecs);


  // Measure performance of utf8ToLower on various texts.
  void perf_StringConverter_utf8ToLower();

  // Measure performance of iso88591ToLower on various texts.
  void perf_StringConverter_iso88591ToLower();


  StringConverter  _stringConverter;
};

// _____________________________________________________________________________
void StringConverterPerf::showStatistics(size_t textLength,
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
void StringConverterPerf::perf_StringConverter_utf8ToLower()
{
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
      // _converter.utf8ToLower(&word);

      word = _stringConverter.convert(word,
                                      StringConverter::ENCODING_UTF8,
                                      StringConverter::CONVERSION_TO_LOWER);
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
    //  _converter.utf8ToLower(&word);

    word = _stringConverter.convert(word,
                                    StringConverter::ENCODING_UTF8,
                                    StringConverter::CONVERSION_TO_LOWER);

    c_end = clock();
    double editTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

    // Show statistics.
    showStatistics(TEXTSIZE, editTimeInMsecs);
  }
}



// _____________________________________________________________________________
void StringConverterPerf::perf_StringConverter_iso88591ToLower()
{
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
      // _converter.iso88591ToLower(&word);
      word = _stringConverter.convert(word,
                                      StringConverter::ENCODING_ISO8859_1,
                                      StringConverter::CONVERSION_TO_LOWER);
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
    //  _converter.iso88591ToLower(&word);

    word = _stringConverter.convert(word,
                                    StringConverter::ENCODING_ISO8859_1,
                                    StringConverter::CONVERSION_TO_LOWER);

    c_end = clock();
    double editTimeInMsecs = 1000 * (c_end - c_start) / CLOCKS_PER_SEC;

    // Show statistics.
    showStatistics(TEXTSIZE, editTimeInMsecs);
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

  cout << "----------------" << endl;
  cout << "Performance Test" << endl;
  cout << "----------------" << endl;

  StringConverterPerf perf;
  perf._stringConverter.init();

  cout << "Performance test for  "
       << "StringConverterPerf::perf_StringConverter_utf8ToLower"
       << endl << endl;
  perf.perf_StringConverter_utf8ToLower();

  cout << "Performance test for "
       << "StringConverterPerf::StringConverter_iso88591ToLower"
       << endl << endl;
  perf.perf_StringConverter_iso88591ToLower();
}
