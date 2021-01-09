// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include <stdlib.h>
#include <stdio.h>
#include <google/sparse_hash_map>
#include <google/dense_hash_map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <hash_set>
#include "../server/Globals.h"
// TODO(unknown): Change path.
#include "../binarysort/BinarySort.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::flush;
using google::dense_hash_map;

void printUsage()
{
  cout << "Specify the unsorted words file" << endl;
}

int fillHashSetWithVoc(FILE* file, hash_set<string, StringHashFunction>* vocSet)
{
  string line;
  string word;
  char buffer[256];
  int pos = 0;

  assert(file != NULL);
  assert(vocSet != NULL);

  while (fgets(buffer, 256, file))
  {
    // cout << "buffer: " << buffer << endl;
    line = string(buffer);
    pos = line.find('\t');
    word = line.substr(0, pos);
    // cout << "Insert word '" << word << "' to hash_map" << endl;
    vocSet->insert(word);
    word.clear();
  }
  return 0;
}

int hashSetToString(hash_set<string, StringHashFunction> set,
                    vector<string>* output)
{
  hash_set<string, StringHashFunction>::iterator it = set.begin();
  for (; it != set.end(); ++it)
  {
    output->push_back(*it);
  }

  return 0;
}

bool stringCompareFunction(string str1, string str2)
{
    return str1 < str2;
}

// This function creates a binary file in which it reads line by line from ascii
// file and exchanges words with their id's
void writeBinaryFile(dense_hash_map<string, int>* map,
                     const string& fileAscii,
                     const string& binaryFile)
{
  // InputAsciifile
  FILE* fileIn;
  int LINE_LENGTH = 10000000;

  fileIn = fopen(fileAscii.c_str(), "r");
  if (fileIn == NULL)
  {
    perror("fopen");
    exit(1);
  }

  // OutPutBinaryFile
  fstream fileOut(binaryFile.c_str(), ios::out|ios::binary);
  assert(fileOut.is_open());

  char* buffer = new char[LINE_LENGTH];

  char* pt;
  string a;
  int b;
  int c;
  int d;

  // Read lines from file, till there are no lines to read anymore.
  while (fgets(buffer, LINE_LENGTH , fileIn))
  {
    // Read from asciifile

    // read first column
    pt = strtok(buffer, " \t\n"); // NOLINT
    a =  (string)pt;

    // read second column
    pt = strtok(NULL, "\t\n"); // NOLINT
    b = atoi(pt);

    // read third column
    pt = strtok(NULL, "\t\n"); // NOLINT
    c = atoi(pt);

    // read forth column
    pt = strtok(NULL, "\t\n"); // NOLINT
    d = atoi(pt);

    // write in binaryfile

    // Id of readed word will be allocated
    int buf = map[a];

    // write first column
    fileOut.write(reinterpret_cast<char *>(&buf), sizeof(int)); // NOLINT

    // write second column
    fileOut.write(reinterpret_cast<char *>(&b), sizeof(int)); // NOLINT

    // write third column
    fileOut.write(reinterpret_cast<char *>(&c), sizeof(int)); // NOLINT

    // write fourth column
    fileOut.write(reinterpret_cast<char *>(&d), sizeof(int)); // NOLINT
  }

  fclose(fileIn);
  fileOut.close();
}

// This funktion converts sorted binary file in sorted ascii file.
//
void writeAsciiBinaryFile(vector<string>* voc,
                          const string& fileAscii,
                          const string& binaryFile)
{
  fstream fileIn(binaryFile.c_str(), ios::in | ios::binary);

  fstream fileOut(fileAscii.c_str(), ios::out);
  // int buf;

  int a;
  int b;
  int c;
  int d;

  while (!fileIn.eof())
  {
    // Read from binary file
    // Reinterpret, that convert readed number from char array in integer
    fileIn.read(reinterpret_cast<char *>(&a), sizeof(int)); // NOLINT

    if (fileIn.eof()) break;

    fileIn.read(reinterpret_cast<char *>(&b), sizeof(int)); // NOLINT
    fileIn.read(reinterpret_cast<char *>(&c), sizeof(int)); // NOLINT
    fileIn.read(reinterpret_cast<char *>(&d), sizeof(int)); // NOLINT

    //  write in ascii file

    string s = voc[a];

    fileOut << s << "\t" << b  << "\t" << c  <<"\t" << d << endl;
  }
  fileIn.close();
  fileOut.close();
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printUsage();
    exit(1);
  }

  string wordsFileName = string(argv[1]);
  cout << "Specified words file to sort: " << wordsFileName << endl;

  FILE* wordsFile = NULL;
  if ((wordsFile = fopen(wordsFileName.c_str(), "r")) == NULL)
  {
    cerr << wordsFileName << " could not be opened!" << endl;
    exit(1);
  }

  hash_set<string, StringHashFunction> vocabularySet;
  vector<string> vocabularyVec;

  cout << "Filling hash_map wiht vocabulary... ";
  fillHashSetWithVoc(wordsFile, &vocabularySet);
  cout << "done." << endl;

  // Convert set to string
  cout << "Convert hash_set to string... ";
  hashSetToString(vocabularySet, &vocabularyVec);
  cout << "done." << endl;

  // sort with std::sort
  cout << "Presorting vocabulary... " << flush;
  sort(vocabularyVec.begin(), vocabularyVec.end(), stringCompareFunction);
  cout << "done." << endl;

  dense_hash_map<string, int> vocabularyMap;
  vocabularyMap.set_empty_key("");

  cout << "Mapping sorted vocabulary... " << flush;
  // Fill the map
  // cout << endl;
  for (unsigned int i = 0; i < vocabularyVec.size(); ++i)
  {
     if (vocabularyMap.count(vocabularyVec[i]) == 0)
      {
        vocabularyMap[vocabularyVec[i]] = i;
      //  cout << vocabularyMap[vocabularyVec[i]] <<" = "<<vocabularyVec[i]
      //       << endl;
      }
  }
  cout << "done." << endl;

  cout << "Writing binary output... " << flush;
  writeBinaryFile(vocabularyMap, wordsFileName, wordsFileName + "_binary");
  cout << "done." << endl;

  cout << "Call binary sort... " << flush;
  SortBinary st;
  st.sortBinaryWordsFile(wordsFileName+"_binary");
  cout << "done." << endl;

  writeAsciiBinaryFile(vocabularyVec,
                       wordsFileName + "_sorted", wordsFileName + "_binary");
  // daraus hash_map<string, int> fuer ids

  return 0;
}
