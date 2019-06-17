// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <google/dense_hash_set>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include "./UserDefinedIndexWords.h"

using google::dense_hash_set;
using std::endl;
using std::cout;
using std::flush;
using std::setw;
using std::ios;

// _____________________________________________________________________________
void printUsage()
{
  cout << endl
       << "Usage: UserDefinedIndexWordsPerf "
          "<multiplicate factor for given text>" << endl
       << endl
       << "Example usage: UserDefinedIndexWordsPerf 500" << endl
       << endl
       << "1. Performance test of class UserDefinedIndexWords::init that parses"
          " the file containing the proper words." << endl
       << "2. Performance test of class SimpleTextParser::parse without parsing"
          " any proper names." << endl
       << "3. Performance test of class "
          "UserDefinedIndexWords::extendToUserDefinedWord." << endl
       << endl << endl;
}

// _____________________________________________________________________________
void testProperInit(unsigned fileSize, UserDefinedIndexWords* pN)
{
  timeval start, end;
  double t1, t2, elapsed, speed;

  cout << endl;
  cout << "Testing of UserDefinedIndexWords::Init()... " << flush;

  // Measure.
  gettimeofday(&start, NULL);
  pN->init("propers.perf");
  gettimeofday(&end, NULL);

  cout << "done." << endl;
  t1 = start.tv_sec + (start.tv_usec/1000000.0);
  t2 = end.tv_sec + (end.tv_usec/1000000.0);
  elapsed = t2 - t1;
  speed = (static_cast<double>(fileSize) / (1024 * 1024)) / elapsed;
  cout << "  measured time : " << setw(4) << elapsed << " sec" << endl;
  cout << "; speed : "<< setw(5) << speed << " MB/sec" << endl;
}

// _____________________________________________________________________________
bool isProper(const string& text, size_t wordStart, size_t wordEnd)
{
  for (size_t i = wordStart; i < wordEnd; i++)
  {
    if (!(text[i] >= 65 && text[i] <= 90)
     && !(text[i] >= 97 && text[i] <= 122)
     && !(text[i] >= 48 && text[i] <= 57)
     // && !(text[i] >= 192 && text[i] <= 255))
     && !(text[i] < 0))
    {
    //    cout << text[i] << ":"<<(int)text[i]
    //         << ":" << text.substr(wordStart,wordEnd-wordStart) << endl;
        return true;
    }
  }
  return false;
}

// _____________________________________________________________________________
void makeTestData(unsigned fileSize, int loop, string  textFile,
                                               string  propersFile,
                                               string  quelFile)
{
  SimpleTextParser simpleTextParser;
  simpleTextParser.setSeparators(" \t\n.;,:!?");

  dense_hash_set<string> properSet;
  properSet.set_empty_key("");

  size_t word_start = 0;
  size_t word_end = 0;

  FILE* tFile = fopen(textFile.c_str(), "w");
  FILE* pFile = fopen(propersFile.c_str(), "w");
  FILE* qFile = fopen(quelFile.c_str(), "r");
  // cout << quelFile << endl;
  assert(tFile != NULL);
  assert(pFile != NULL);
  assert(qFile != NULL);

  srand(time(NULL));
  clock_t start, end;
  // int wLength;
  // char charId;
  string word;
  char buffer[10000];
  string line;
  int ct = 0;

  start = clock();
  cout << endl << "Making testfile... " << flush;

  while (fgets(buffer, 10000, qFile) != NULL)
  {
    line = string(buffer);
    ct++;
    fileSize = fileSize + line.size();

    // cout << "line "<< ct <<"size "<< line.size() << "inhalt "<< line<< endl;
    for (int i = 0; i < loop; i++)
    {
     fputs(buffer, tFile);
    }
    word_start = 0;
    word_end = 0;

    while (word_end < line.length())
    {
      simpleTextParser.parseText(line, &word_start, &word_end);
      if (isProper(line, word_start, word_end))
      {
        word = line.substr(word_start, word_end - word_start) + "\n";
        if ((properSet.find(word.c_str()) == properSet.end())
        && ((word_end - word_start) > 1))
        {
          fputs(word.c_str(), pFile);
          properSet.insert(word.c_str());
        }
      }
    }
  }
  end = clock();
  cout << "done." << endl;
  // cout << "FileSize="<< fileSize << " loop=" << loop << endl;
  double time = (end - start) / static_cast<double>(CLOCKS_PER_SEC);
  cout << "  created " << setw(4)
       << static_cast<double>(fileSize*loop / (1024 * 1024)) << " MB";
  cout << " in " << setw(4) << time << " sec" << endl;
  fclose(qFile);
  fclose(pFile);
  fclose(tFile);
}

// _____________________________________________________________________________
void  parsWithoutUserDefinedIndexWords(int fileSize, string  textFile)
{
  cout << "Here we go!!!" << endl;

  SimpleTextParser simpleTextParser;
  FILE* file = fopen(textFile.c_str(), "r");
  char buffer[10000];
  assert(file != NULL);
  srand(time(NULL));
  clock_t start, end;
  string text;
  string word;
  start = clock();
  size_t word_start = 0;
  size_t word_end = 0;
  int namesCnt = 0;

  cout << endl << "Parsing without UserDefinedIndexWord... " << flush;
  start = clock();

  while (fgets(buffer, 10000, file) != NULL)
  {
    text = string(buffer);
    word_start = 0;
    word_end = 0;
    while (word_end < text.length())
    {
      namesCnt++;
      simpleTextParser.parseText(text, &word_start, &word_end);
      word = text.substr(word_start, word_end - word_start);
      // cout << "simpleprser hat gelesen: "<< word << endl;
      if (word_start == word_end) break;
    }
  }

  end = clock();
  cout << "done." << endl;

  double time = (end - start) / static_cast<double>(CLOCKS_PER_SEC);
  double speed = (static_cast<double>(fileSize) / (1024 * 1024)) / time;
  cout << "  measured time : " << setw(4) << time << " sec";
  cout << "; speed : "<< setw(5) << speed << " MB/sec" << endl;
  cout << "  parsed words: " << namesCnt << endl;

  fclose(file);
}

// _____________________________________________________________________________
void parsWithUserDefinedIndexWords(UserDefinedIndexWords* pN,
                                         int fileSize,
                                         string textFile)
{
  SimpleTextParser simpleTextParser;
  FILE* file = fopen(textFile.c_str(), "r");
  char buffer[10000];
  assert(file != NULL);
  srand(time(NULL));
  clock_t start, end;
  string text;
  string word;
  start = clock();
  size_t word_start = 0;
  size_t word_end = 0;
  int namesCnt = 0;
  int properNamesCnt = 0;

  cout << endl << "Parsing with UserDefinedIndexWord... " << flush;
  start = clock();

  while (fgets(buffer, 10000, file) != NULL)
  {
    text = string(buffer);
    word_start = 0;
    word_end = 0;
    while (word_end < text.length())
    {
      namesCnt++;
      simpleTextParser.parseText(text, &word_start, &word_end);
      word = text.substr(word_start, word_end - word_start);
      // cout << "simpleprser hat gelesen: "<< word << endl;
      if (word_start == word_end) break;

      if (pN->extendToUserDefinedWord(text, &word_start, &word_end))
      {
          properNamesCnt++;
      }
    }
  }

  end = clock();
  cout << "done." << endl;

  double time = (end - start) / static_cast<double>(CLOCKS_PER_SEC);
  double speed = static_cast<double>(fileSize) / (1024 * 1024) / time;
  cout << "  measured time : " << setw(4) << time << " sec";
  cout << "; speed : "<< setw(5) << speed << " MB/sec" << endl;
  cout << "  parsed words: " << namesCnt << "; parsed propernames: "
       << properNamesCnt << endl << endl;

  fclose(file);
}

// _____________________________________________________________________________
int main(int argc, char** argv)
{
  size_t loop;
  if (argc != 2)
  {
    printUsage();
    loop = 500;
  }
  else
    loop = atoi(argv[1]);

  unsigned fileSize = 0;

  cout.setf(ios::fixed);

  cout.precision(1);

  // Object of UserDefinedIndexWords
  UserDefinedIndexWords pN;

  char textFile[] ="text.perf";
  char propersFile[] ="propers.perf";
  char quelFile[] ="quel_text.perf";

  cout << "makeTestDate... " << flush;
  makeTestData(fileSize, loop, textFile, propersFile, quelFile);
  cout << "done.\n";

  cout << "testProperInit... " << flush;
  testProperInit(fileSize, &pN);
  cout << "done.\n";

  cout << "parseWithoutUserDefinedIndexWords... " << flush;
  parsWithoutUserDefinedIndexWords(fileSize*loop, textFile);
  cout << "done\n";

  cout << "parseWithUserDefinedIndexWords... " << flush;
  parsWithUserDefinedIndexWords(&pN, fileSize*loop, textFile);
  cout << "done.\n";
  return 0;
}
