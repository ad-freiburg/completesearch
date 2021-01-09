// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <gtest/gtest.h>
#include <stdarg.h>
#include <bits/basic_ios.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::ofstream;
using std::ifstream;
using std::vector;

class ConvertBinaryToAscii : public testing::Test
{
 public:

  typedef struct _poker
  {
    void init(int aP, int bP, int cP, int dP)
    {
      a = aP;
      b = bP;
      c = cP;
      d = dP;
    }
    int a;
    int b;
    int c;
    int d;
  } Poker;
 protected:

  ConvertBinaryToAscii()
  : _base("ConvertBinaryToAscii.TMP"),
  _extensionVocabulary(".vocabulary"),
  _extensionWords_sorted(".words"),
  _extensionBinary_sorted(".words-sorted.binary")
  {
  }

  void compareFileWithString(const string& expected, const string& fileName)
  {
    int fileLength;
    ifstream is;
    char* buffer;
    is.open(fileName.c_str(), std::ios::binary);
    ASSERT_TRUE(is.is_open()) << "Could not open file " << fileName << ".";
    // Determine file size.
    is.seekg(0, std::ios::end);
    fileLength = is.tellg();
    is.seekg(0, std::ios::beg);
    // Read the files content.
    buffer = new char[fileLength + 1];
    is.read(buffer, fileLength);
    is.close();
    buffer[fileLength] = '\0';
    ASSERT_EQ(strcmp(buffer, expected.c_str()), 0)
            << "File " << fileName << " has wrong content." << endl
            << "Read:  " << endl << buffer << endl
            << "Expected: " << endl << expected << endl;
    delete[] buffer;
  }

  void compareBinaryFileWithVector(const vector<Poker>& expected,
          const string& fileName)
  {
    vector<Poker> readed;
    writeBinaryFileToVector(&readed, fileName);
    for (unsigned i = 0; i < expected.size(); i++)
    {
      // column 1
      Poker buf = expected[i];
      ASSERT_EQ(buf.a, readed[i].a)
              << "File " << fileName << " has wrong content." << endl
              << "Read:  " << endl << readed[i].a << endl
              << "Expected: " << endl << buf.a << endl
              << "Line: " << i << " Column: 1" << endl;

      // column 2
      ASSERT_EQ(buf.b, readed[i].b)
              << "File " << fileName << " has wrong content." << endl
              << "Read:  " << endl << readed[i].b << endl
              << "Expected: " << endl << buf.b << endl
              << "Line: " << i << " Column: 2" << endl;

      // column 3
      ASSERT_EQ(buf.c, readed[i].c)
              << "File " << fileName << " has wrong content." << endl
              << "Read:  " << endl << readed[i].c << endl
              << "Expected: " << endl << buf.c << endl
              << "Line: " << i << " Column: 3" << endl;

      // column 4
      ASSERT_EQ(buf.d, readed[i].d)
              << "File " << fileName << " has wrong content." << endl
              << "Read:  " << endl << readed[i].d << endl
              << "Expected: " << endl << buf.d << endl
              << "Line: " << i << " Column: 4" << endl;
    }
  }

  void writeStringToFile(const string& data, const string& fileName)
  {
    ofstream oFile(fileName.c_str());
    ASSERT_TRUE(oFile.is_open()) << "Could not make file " << fileName << ".";
    oFile << data;
    oFile.close();
  }

  void writeFileToString(string *data, const string& fileName)
  {
    ifstream iFile(fileName.c_str());
    ASSERT_TRUE(iFile.is_open()) << "Could not read file " << fileName << ".";
    while (!iFile.eof())
    {
      data->push_back(iFile.get());
    }
    iFile.close();
  }

  void writeVectorToBinaryFile(vector<Poker>& data, const string& fileName)
  {
    ofstream oFile(fileName.c_str(), std::ios::binary);
    ASSERT_TRUE(oFile.is_open()) << "Could not make file " << fileName << ".";
    for (unsigned i = 0; i < data.size(); i++)
    {
      Poker buf = data[i];
      oFile.write(reinterpret_cast<char *> (&(buf.a)), sizeof(buf.a));
      oFile.write(reinterpret_cast<char *> (&(buf.b)), sizeof(buf.b));
      oFile.write(reinterpret_cast<char *> (&(buf.c)), sizeof(buf.c));
      oFile.write(reinterpret_cast<char *> (&(buf.d)), sizeof(buf.d));
    }
    oFile.close();
  }

  void writeBinaryFileToVector(vector<Poker>* data, const string& fileName)
  {
    ifstream iFile(fileName.c_str(), std::ios::binary);
    ASSERT_TRUE(iFile.is_open()) << "Could not read file " << fileName << ".";
    while (!(iFile.eof()))
    {
      Poker buf;
      iFile.read(reinterpret_cast<char *> (&(buf.a)), sizeof(buf.a));
      if (iFile.eof()) break;
      iFile.read(reinterpret_cast<char *> (&(buf.b)), sizeof(buf.b));
      iFile.read(reinterpret_cast<char *> (&(buf.c)), sizeof(buf.c));
      iFile.read(reinterpret_cast<char *> (&(buf.d)), sizeof(buf.d));
      data->push_back(buf);
    }
    iFile.close();
  }

  void removeFile(string file)
  {
    string rmError = "Error deleting " + file;
    if ( remove(file.c_str()) != 0 )
      perror(rmError.c_str());
  }

  // Variables the test uses.
  // int _argc;
  // char** _commandLine;
  const string _base;
  const string _extensionVocabulary;
  const string _extensionWords_sorted;
  const string _extensionBinary_sorted;
};
// _____________________________________________________________________________

TEST_F(ConvertBinaryToAscii, ConvertBinaryToAscii)
{
  vector<Poker> expVector;
  Poker buf;
  buf.init(0, 1, 1, 1);
  expVector.push_back(buf);
  buf.init(1, 1, 1, 2);
  expVector.push_back(buf);
  buf.init(1, 1, 1, 3);
  expVector.push_back(buf);
  buf.init(2, 1, 1, 1);
  expVector.push_back(buf);
  buf.init(3, 1, 1, 3);
  expVector.push_back(buf);
  writeVectorToBinaryFile(expVector, _base + _extensionBinary_sorted);
  string vocabulary = "wort0\nwort1\nwort3\nww\n";
  writeStringToFile(vocabulary, _base + _extensionVocabulary);

  // Execute ConvertBinaryToAscii with SortTest.words_ids
  string sysLine = string("./ConvertBinaryToAscii ")
                 + _base + _extensionBinary_sorted;
  ASSERT_EQ(system(sysLine.c_str()), 0);

  string expData = "wort0\t1\t1\t1\n"
          "wort1\t1\t1\t2\n"
          "wort1\t1\t1\t3\n"
          "wort3\t1\t1\t1\n"
          "ww\t1\t1\t3\n";

  compareFileWithString(expData, _base + _extensionWords_sorted);
  removeFile(_base + _extensionVocabulary);
  removeFile(_base + _extensionBinary_sorted);
  removeFile(_base + _extensionWords_sorted);
}

