// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <google/dense_hash_map>
#include <google/sparse_hash_map>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include "../server/Globals.h"

using google::dense_hash_map;

void printUsage()
{
  cout << "Specify the ascii words file" << endl;
}

// Bjoern(26Jul10): Changed type of pos to string::size_type.
// using the file name
// data/semantic-wikipedia-scientists.words-unsorted.ascii
// had problems before. Somehow this method returned "dat" omly.
string parseFileName(const string& fileName)
{
  string::size_type pos = 0;
  pos = fileName.rfind(".TMP.");
  if (pos == string::npos)
  {
    pos = fileName.find('.');
    if (pos == string::npos)
      return fileName;
  } else
    pos += 4;
  return fileName.substr(0, pos);
}

// Bjoern(26Jul10): Added one argument to support different buffer
// sizes. See comment in main method for motivation of this change.
int fillHashSetWithVoc(const string& unsortedFile,
    dense_hash_map<string, int>& map, size_t vocabBufferSize)
{
  FILE* wordsFile = NULL;
  if ((wordsFile = fopen(unsortedFile.c_str(), "r")) == NULL)
  {
    cerr << unsortedFile << " could not be opened!" << endl;
    exit(1);
  }

  string line;
  string word;
  const size_t BUFFER_SIZE = vocabBufferSize;
  char buffer[BUFFER_SIZE];

  assert(wordsFile != NULL);

  while (fgets(buffer, vocabBufferSize, wordsFile))
  {
    line = string(buffer);

    // Björn(01Aug10):
    // Changed from int pos = 0
    // to string::size_type pos = string::npos.
    string::size_type pos = string::npos;

    // Björn(01Aug10):
    // Replaced comparison v.s. 0 with a comparison v.s. string::npos.
    // Refined the error message.
    // Also added a check for the line length vs buffer size.
    pos = line.find('\n');
    if (pos == string::npos)
    {
      cout << "No newline found at the end of this line!" << endl
          << "Please make sure that the --vocab-buffer-size "
          << "is specified specified correctly if "
          << "there are words with more than 256 characters." << endl;
      exit(1);
    }

    pos = line.find('\t');
    if (pos == string::npos)
    {
      cout << "Wrong format! Use TAB as separator! " << endl;
      exit(1);
    }
    word = line.substr(0, pos);
    map[word] = 0;
    word.clear();
  }
  fclose(wordsFile);
  return 0;
}

bool stringCompareFunction(string str1, string str2)
{
  return str1 < str2;
}

void makeVocabulary(const string& vocFile, dense_hash_map<string, int>& map,
    vector<string>& set)
{
  dense_hash_map<string, int>::iterator it = map.begin();

  // fill vocabulary vector with mapped words
  for (; it != map.end(); ++it)
  {
    set.push_back(it->first);
  }

  // sorting vocabulary vector
  cout << "Presorting vocabulary... " << flush;
  sort(set.begin(), set.end(), stringCompareFunction);
  cout << "done." << endl;

  FILE* outFile;
  outFile = fopen(vocFile.c_str(), "w");

  //    fill vocabulary File with sorted words
  cout << "Writing vocabulary file... " << flush;
  for (unsigned int i = 0; i < set.size(); i++)
  {
    fputs((set[i] + "\n").c_str(), outFile);
    map[set[i]] = i;
  }
  cout << "done." << endl;

  fclose(outFile);
}

void writeBinaryFile(const string& asciiFile, const string& binaryFile,
    dense_hash_map<string, int>& map)
{
  // InputAsciifile
  FILE* fileIn;
  int LINE_LENGTH = 10000000;

  fileIn = fopen(asciiFile.c_str(), "r");
  if (fileIn == NULL)
  {
    perror(string("Could not open file : " + asciiFile).c_str());
    exit(1);
  }

  // OutPutBinaryFile
  fstream fileOut(binaryFile.c_str(), ios::out | ios::binary);
  assert(fileOut.is_open());

  char* buffer = new char[LINE_LENGTH];

  char *pt, *p;
  string a;
  int b;
  int c;
  int d;

  // Read lines from file, till there are no lines to read anymore.
  while (fgets(buffer, LINE_LENGTH, fileIn))
  {
    // Read from asciifile

    // read first column
    pt = strtok_r(buffer, " \t\n", &p);
    a = (string) pt;

    // read second column
    pt = strtok_r(NULL, " \t\n", &p);
    b = atoi(pt);

    // read third column
    pt = strtok_r(NULL, " \t\n", &p);
    c = atoi(pt);

    // read forth column
    pt = strtok_r(NULL, " \t\n", &p);
    d = atoi(pt);

    // write in binaryfile

    // Id of readed word will be allocated
    int buf = map[a];

    // write first column
    fileOut.write(reinterpret_cast<char *> (&buf), sizeof(buf));

    // write second column
    fileOut.write(reinterpret_cast<char *> (&b), sizeof(b));

    // write third column
    fileOut.write(reinterpret_cast<char *> (&c), sizeof(c));

    // write fourth column
    fileOut.write(reinterpret_cast<char *> (&d), sizeof(d));
  }

  fclose(fileIn);
  fileOut.close();
}
int main(int argc, char** argv)
{
  cout << "~~~~~~~~~~~~~~~~~~~~" << endl << "ConvertAsciiToBinary" << endl
      << "~~~~~~~~~~~~~~~~~~~~" << endl;

  // Bjoern(26Jul10): Added this variable to allow setting different buffer
  // sizes when making the vocabulary map. Had problems earlier with really
  // long, artificial words written to an index.
  // 256 is kept as default value and other values can be set if some
  // index requires longer words.
  size_t vocabBufferSize = 256;

  // Bjoern(26Jul10): Added support for the --vocab-buffer-size option.

  // Available options.
  struct option options[] =
  {
  { "vocab-buffer-size", required_argument, NULL, 'b' },
  { NULL, 0, NULL, 0 } };

  optind = 1;

  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "b:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case 'b':
        vocabBufferSize = atoi(optarg);
        break;
      default:
        std::cout << std::endl
            << "! ERROR in processing options (getopt returned '" << c
            << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
            << std::endl << std::endl;
        exit(1);
    }
  }

  // File names.
  if (optind >= argc)
  {
    printUsage();
    exit(1);
  }

  string wordsFileName = string(argv[optind++]);

  cout << "Specified ASCII file: " << wordsFileName << endl;

  string wordsFileNameBase = parseFileName(wordsFileName);
  cout << "Base: " << wordsFileNameBase << endl;

  dense_hash_map<string, int> vocabularyMap;
  vocabularyMap.set_empty_key("");
  vector<string> vocabularySet;

  cout << "Mapping words... " << flush;
  fillHashSetWithVoc(wordsFileName, vocabularyMap, vocabBufferSize);
  cout << "done." << endl;
  makeVocabulary(wordsFileNameBase + ".vocabulary", vocabularyMap,
      vocabularySet);
  cout << "Writing binary file (" << wordsFileNameBase
      + ".words-unsorted.binary" << ")... " << flush;
  writeBinaryFile(wordsFileName,
      wordsFileNameBase + ".words-unsorted.binary", vocabularyMap);
  cout << "done." << endl;
  return 0;
}
