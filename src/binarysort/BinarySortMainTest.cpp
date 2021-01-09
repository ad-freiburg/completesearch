// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/stat.h>
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

class BinarySortMain : public testing::Test
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

  BinarySortMain()
  : _base("BinarySortMain.TMP"),
  _extensionVocabulary(".vocabulary"),
  _extensionWords_unsorted(".words-unsorted"),
  _extensionWords_sorted(".words"),
  _extensionBinary_unsorted(".words-unsorted.binary"),
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

  string getPath()
  {
    string path = "";
    pid_t pid = getpid();
    char buf[20] = {0};
    snprintf(buf, sizeof(buf), "%d", pid);
    string _link = "/proc/";
    _link.append(buf);
    _link.append("/exe");
    char proc[512];
    int ch = readlink(_link.c_str(), proc, 512);
    if (ch != -1)
    {
      proc[ch] = 0;
      path = proc;
      string::size_type t = path.find_last_of("/");
      path = path.substr(0, t);
    }
    return path + string("/");
  }

  bool checkFile(const string& file)
  {
    std::fstream bFile(file.c_str(), std::ios::in);
    if (!bFile.is_open())
    {
      return false;
    }
    bFile.close();
    return true;
  }

  int64_t getSizeOfFile(const string& diskFileName)
  {
    struct stat buf;
    int ret = stat(diskFileName.c_str(), &buf);
    if (ret != 0)
    {
      std::cerr << "Could not check size of " << diskFileName << endl;
      exit(1);
    }
    return buf.st_size;
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
    if (remove(file.c_str()) != 0)
      perror(rmError.c_str());
  }

  // Variables the test uses.
  // int _argc;
  // char** _commandLine;
  const string _base;
  const string _extensionVocabulary;
  const string _extensionWords_unsorted;
  const string _extensionWords_sorted;
  const string _extensionBinary_unsorted;
  const string _extensionBinary_sorted;
};
// _____________________________________________________________________________

TEST_F(BinarySortMain, BinarySortMain)
{
  // Testing of created binary File
  Poker buf;
  vector<Poker> inVector;

  buf.init(2, 1, 1, 1);
  inVector.push_back(buf);
  buf.init(1, 1, 1, 3);
  inVector.push_back(buf);
  buf.init(1, 1, 1, 2);
  inVector.push_back(buf);
  buf.init(3, 1, 1, 3);
  inVector.push_back(buf);
  buf.init(0, 1, 1, 1);
  inVector.push_back(buf);

  writeVectorToBinaryFile(inVector, _base + _extensionBinary_unsorted);
  writeStringToFile(string("disk="
          + getPath()
          + "stxxl.disk,15,syscall").c_str(), ".stxxl");
  // Test of BinarySortMain for --stxxl-disk-size options
  string sysLine = string("./BinarySortMain -f ")
          + _base + _extensionBinary_unsorted
          + " -b " + _base
          + " -s 15M";
  ASSERT_EQ(system(sysLine.c_str()), 0);

  // Testing of sorted binary File
  vector<Poker> expVector;
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
  compareFileWithString(string("disk="
          + getPath()
          + "stxxl.disk,15,syscall").c_str(),
          ".stxxl");
  ASSERT_EQ(getSizeOfFile("stxxl.disk"), 15 * 1024 * 1024);
  compareBinaryFileWithVector(expVector, _base + _extensionBinary_sorted);

  // Test of BinarySortMain for default options
  writeVectorToBinaryFile(inVector, _base + _extensionBinary_unsorted);

  sysLine = string("./BinarySortMain -f ")
          + _base + _extensionBinary_unsorted
          + " -b " + _base;
  ASSERT_EQ(system(sysLine.c_str()), 0);

  compareFileWithString(string("disk="
          + getPath()
          + "stxxl.disk,15,syscall").c_str(),
          ".stxxl");

  ASSERT_EQ(getSizeOfFile("stxxl.disk"), 15 * 1024 * 1024);
  compareBinaryFileWithVector(expVector, _base + _extensionBinary_sorted);

  // Test of BinarySortMain for --stxxl-disk-file options
  writeVectorToBinaryFile(inVector, _base + _extensionBinary_unsorted);

  sysLine = string("./BinarySortMain -f ")
          + _base + _extensionBinary_unsorted
          + " -d " + getPath() + "stxxl.TMP.disk"
          + " -s 15M";
  ASSERT_EQ(system(sysLine.c_str()), 0);

  compareFileWithString(string("disk="
          + getPath()
          + "stxxl.TMP.disk,15,syscall").c_str(),
          ".stxxl");

  ASSERT_EQ(checkFile(getPath() + "stxxl.TMP.disk"), true);

  ASSERT_EQ(getSizeOfFile(getPath() + "stxxl.TMP.disk"), 15 * 1024 * 1024);
  compareBinaryFileWithVector(expVector, _base + _extensionBinary_sorted);
  removeFile(_base + _extensionBinary_sorted);
  removeFile(getPath() + "stxxl.TMP.disk");
  removeFile("stxxl.disk");
  removeFile(".stxxl");
  removeFile("stxxl.log");
  removeFile("stxxl.errlog");
  removeFile("BinarySortMain.words-sorted.binary");
}

