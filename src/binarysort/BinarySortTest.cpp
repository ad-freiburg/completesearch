// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include "./BinarySort.h"

using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;
// _____________________________________________________________________________
// This Funktion write input array in temporary file "file.tmp"
void makeFile(int* input)
{
  fstream file("BinarySort.TMP.words-unsorted.binary",
               std::ios::out|std::ios::binary);
  assert(file.is_open());

  for (int i = 0; i < 12; i++)
  {
    int x = input[i];
    file.write(reinterpret_cast<char *>(&x), sizeof(x));
  }
  file.close();
}
// _____________________________________________________________________________
void writeStringToFile(const string& data, const string& fileName)
{
  ofstream oFile(fileName.c_str());
  ASSERT_TRUE(oFile.is_open()) << "Could not make file " << fileName << ".";
  oFile << data;
  oFile.close();
}
// _____________________________________________________________________________
// This Funktion tests if file.tmp has the same array like answer
bool checkOut(int* answer, int n)
{
  fstream file("BinarySort.TMP.words-unsorted.binary",
               std::ios::in | std::ios::binary);
  int buf;
  int counter = 0;

  file.read(reinterpret_cast<char *>(&buf), sizeof(buf));
  while ( !file.eof() )
  {
    // Test whether readed character and character from answer are equal
    if (answer[counter] != buf)
    {
      file.close();
      return false;
    }

    counter++;
    file.read(reinterpret_cast<char *>(&buf), sizeof(buf));
  }

  // Test whether number of readed character is correct
  if ((n > 0) && (counter != n))
  {
    file.close();
    return false;
  }

  file.close();
  return true;
}
// _____________________________________________________________________________
void removeFile(string file)
{
  string rmError = "Error deleting " + file;
  if ( remove(file.c_str()) != 0 )
    perror(rmError.c_str());
}
// _____________________________________________________________________________
TEST(BinarySort, sortBinaryWordsFile)
{
  writeStringToFile("disk=stxxl.disk,15,syscall", ".stxxl");
  // TEST 1
  // Write:         Answer:
  // 3, 9, 1, 5     1, 6, 0, 1
  // 2, 6, 9, 2     2, 6, 9, 2
  // 1, 6, 0, 1     3, 9, 1, 5

  // Testet Object
  BinarySort ad;
  int input1[12] = {3, 9, 1, 5, 2, 6, 9, 2, 1, 6, 0, 1};
  int answer1[12]   = {1, 6, 0, 1, 2, 6, 9, 2, 3, 9, 1, 5};

  // Make testfile with input1
  makeFile(input1);

  // Testing of SortBinary::sortBinaryWordsFile with first input
  ad.sortBinaryWordsFile("BinarySort.TMP.words-unsorted.binary");
  ASSERT_EQ(checkOut(answer1, 12), true);

  // TEST 2
  // Write:         Answer:
  // 5, 3, 1, 5     5, 1, 0, 1
  // 5, 2, 9, 2     5, 2, 9, 2
  // 5, 1, 0, 1     5, 3, 1, 5

  // Testet Object

  int input2[12] = {5, 3, 1, 5, 5, 2, 9, 2, 5, 1, 0, 1};
  int answer2[12]   = {5, 1, 0, 1, 5, 2, 9, 2, 5, 3, 1, 5};

  // Make testfile with input2
  makeFile(input2);

  // Testing of SortBinary::sortBinaryWordsFile with second input
  ad.sortBinaryWordsFile("BinarySort.TMP.words-unsorted.binary");
  ASSERT_EQ(checkOut(answer2, 12), true);

  // TEST 3
  // Write:         Answer:
  // 5, 2, 1, 5     5, 2, 9, 1
  // 5, 2, 9, 1     5, 2, 0, 2
  // 5, 2, 0, 2     5, 2, 1, 5

  // Testet Object
  int input3[12] = {5, 2, 1, 5, 5, 2, 9, 1, 5, 2, 0, 2};
  int answer3[12]   = {5, 2, 9, 1, 5, 2, 0, 2, 5, 2, 1, 5};

  // Make testfile with input3
  makeFile(input3);

  // Testing of SortBinary::sortBinaryWordsFile with third input
  ad.sortBinaryWordsFile("BinarySort.TMP.words-unsorted.binary");
  ASSERT_EQ(checkOut(answer3, 12), true);
  removeFile("BinarySort.TMP.words-unsorted.binary");
  removeFile(".stxxl");
  removeFile("stxxl.disk");
  removeFile("stxxl.log");
  removeFile("stxxl.errlog");
}

