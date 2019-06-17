// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <assert.h>
#include <time.h>
#include <fstream>
#include <string>
#include "./BinarySort.h"

using std::fstream;
using std::ofstream;
using std::flush;

// _____________________________________________________________________________
void printUsage()
{
  cout << endl
       << "Usage: sortBinaryPerf <number of words>" << endl << endl
       << "Example usage: sortBinaryPerf 10000000" << endl << endl
       << "1. Timetest of SortBinary::sortBinaryWordsFile "
       << "with random file and different range of numbers" << endl
       << endl
       << "2. Timetest of SortBinary::sortBinaryWordsFile with sorted file"
       << endl;
}

// _____________________________________________________________________________
string intToString(int64_t num)
{
  std::stringstream sstr;
  sstr << num;
  string buf;
  sstr >> buf;
  return buf;
}

// _____________________________________________________________________________
void writeStringToFile(const string& data, const string& fileName)
  {
    ofstream oFile(fileName.c_str());
    if (!oFile.is_open())
    {
      cout << "Could not write to file: " << fileName;
    }

    oFile << data;
    oFile.close();
  }

// _____________________________________________________________________________
void removeFile(const string &file)
{
  string sysLine = "rm -f " + file;
  int ret = system(sysLine.c_str());
  if (ret != 0)
    cout << "Error by removing of file" << file << endl;
}

// _____________________________________________________________________________
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

// _____________________________________________________________________________
int64_t getSizeOfFile(const string& fileName)
{
  if (!checkFile(fileName))
  {
    {
    cerr << "Could not get size of " << fileName << endl;
    exit(1);
  }
  }
  struct stat buf;
  int ret = stat(fileName.c_str(), &buf);
  if (ret != 0)
  {
    cerr << "Could not get size of " << fileName << endl;
    exit(1);
  }
  return buf.st_size;
}

// _____________________________________________________________________________
void createDisk(const string& diskFileName, const int64_t &diskSize)
{
  int64_t size = 0;

  if (diskSize < 1024 * 1024)
    size = 1;
  else
    size = diskSize / 1024 / 1024;

  std::stringstream sstr;
  sstr << size;
  string buf;
  sstr >> buf;
  cout << buf << endl;
  string sysLine = "dd if=/dev/zero bs=1048576 count=" +
          buf + " of=" + diskFileName;
  cout << sysLine << endl;

  cout << "Create stxxl disk ... " << flush;
  int ret = system(sysLine.c_str());
  if (ret != 0)
  {
    cerr << "Could not create stxxl disk: " << diskFileName << endl;
    exit(1);
  }
  cout << "done." << endl;
}

// _____________________________________________________________________________
// This funktion make a random binaryfile, wich contains numLines lines
// and even line has 4 random numbers between 0 and 9999
void makeRandomBinaryFile(unsigned numLines, int nSpan)
{
  fstream file("BinarySortPerf.TMP.binary", std::ios::out|std::ios::binary);
  assert(file.is_open());
  srand(time(NULL));
  unsigned count = 0;
  int x;
  clock_t start, end;

  cout << "     Testfile will created... " << flush;
  // start measure
  start = clock();
  while (count < numLines)
  {
    unsigned pt;
    // Write random number in first column
    x = rand_r(&pt) % nSpan;
    file.write(reinterpret_cast<char *>(&x), sizeof(x));

    // Write random number in second column
    x = rand_r(&pt) % nSpan;
    file.write(reinterpret_cast<char *>(&x), sizeof(x));

    // Write random number in third column
    x = rand_r(&pt) % nSpan;
    file.write(reinterpret_cast<char *>(&x), sizeof(x));

    // Write random number in fourth column
    x = rand_r(&pt) % nSpan;
    file.write(reinterpret_cast<char *>(&x), sizeof(x));

    // next line
    count++;
  }
  // end measure
  end = clock();
  file.close();
  double time = (end - start);
  time = time / static_cast<double>(CLOCKS_PER_SEC);
  cout << "created " << std::setw(4)
       << static_cast<double>(4 * 4 * ((numLines) / (1024 * 1024))) << " MB";
  cout << " in " << std::setw(4) << time << " sec" << endl;
}

// _____________________________________________________________________________
// Test of SortBinary::sortBinaryWordsFile with created temp file
void makeTest(unsigned numLines)
{
  // Make instance of SortBinary
  BinarySort os;
  time_t start, end;

  cout << "     Test SortBinary::sortBinaryWordsFile ..." << endl;
  // Start measure
  time(&start);
  // start = clock();

  // Execute SortBinary::sortBinaryWordsFile
  os.sortBinaryWordsFile("BinarySortPerf.TMP.binary");

  // End measure
  time(&end);
  double time = difftime(end, start);
  double speed = static_cast<double>(4 * 4 * ((numLines)
               / (1024 * 1024))) / time;
  cout << "      measured time : " << std::setw(4) << time << " sec";
  cout << "; speed : "<< std::setw(5) << speed << " MB/sec" << endl;
}

// _____________________________________________________________________________
void checkSortOrder(unsigned numLines)
{
  cout << "     Correctness test of sorted file ..." << endl;

  fstream bFile("BinarySortPerf.TMP.binary", std::ios::in | std::ios::binary);

  int a = 0;
  int _a;
  int b = 0;
  int _b;
  int d = 0;
  int _d;
  unsigned count = 0;

  while ( !bFile.eof() )
  {
    // Test of ordeering in column 1
    bFile.read(reinterpret_cast<char *>(&_a), sizeof(_a));

    if (_a < a)
      cout << "         Error in sort order!!!" << endl;

    // Check if end of file
    if ( bFile.eof() )
    {
      cout << "      Sort order is OK!" << endl;
      cout << "      Sizedata befor sorting "
           << static_cast<double>(numLines) * 4 * 4 << " Byte"<< endl;
      cout << "      Sizedata after sorting "
           << static_cast<double>(count) * 4 * 4 << " Byte"<< endl;
      break;
    }

    // Test of ordeering in column 2
    bFile.read(reinterpret_cast<char *>(&_b), sizeof(_b));

    if ((_a = a) && (_b < b))
      cout << "Error in sort order!!!" << endl;

    // Check if and of file
    if ( bFile.eof() )
    {
      cout << "Sizedata after sorting"
           << static_cast<double>(count) * 4 * 4 + 1
           << endl;
      break;
    }

    // Test of ordeering in column 4
    // Two time read because of column 3, wich is not needed
    bFile.read(reinterpret_cast<char *>(&_d), sizeof(_d));

    // Check if and of file
    if ( bFile.eof() )
    {
      cout << "Sizedata after sorting"
           << static_cast<double>(count) * 4 * 4 + 2
           << endl;
      break;
    }

    bFile.read(reinterpret_cast<char *>(&_d), sizeof(_d));

    if ((_a = a) && (_b = b) && (_d < d))
      cout << "Error in sort order!!!" << endl;

    // Check if and of file
    if ( bFile.eof() )
    {
      cout << "Sizedata after sorting"
           << static_cast<double>(count) * 4 * 4 + 3
           << endl;
      break;
    }

    // Safe of new values
    a = _a;
    b = _b;
    d = _d;
    count++;
  }
}

// _____________________________________________________________________________
// 1. Timetest of SortBinary::sortBinaryWordsFile with random file
// and different range of numbers
// 2. Timetest of SortBinary::sortBinaryWordsFile with sorted file
void perfSortBinaryWordsFile(unsigned numLines)
{
  int numSpan = 1000;
  int64_t diskSize = (numLines * 4 * 4) + (numLines * 4 * 4) / 100 * 10;
  char testId = 'A';

  cout << "------------------------------------------------"
       <<"----------------------------------------------" << endl;
  cout << "1. Timetest of SortBinary::sortBinaryWordsFile "
       <<"with random file and different range of numbers" << endl;
  cout << "(WITHOUT predefined stxxl.disk)" << endl;
  cout << "-----------------------------------------------"
       <<"-----------------------------------------------" << endl;
  for (int i = 0; i < 1; i++)
  {
    cout << endl;
    cout << " TEST " << testId <<" , range " << numSpan << ":" << endl;

    // Make a random binaryfile, wich contains numLines lines and even line has
    // 4 random numbers between 0 and 9/ 0 and 999/ 0 and 99999
    for (int j = 0; j < 1; j++)
    {
      cout << "   try "<< j+1 <<":" << endl;
      cout << "     ---------" << endl;
      makeRandomBinaryFile(numLines, numSpan);
      cout << "     ---------" << endl;
      // Create .stxxl
      if (checkFile("stxxl.disk")) removeFile("stxxl.disk");


      writeStringToFile(string("disk=stxxl.disk,"
                               + intToString(diskSize / 1024 / 1024)
                               + ",syscall"),
                        ".stxxl");
      makeTest(numLines);
      cout << "     ---------" << endl;
      checkSortOrder(numLines);
      cout << "     ---------" << endl;
      numSpan = numSpan * 100;
    }

    testId++;
  }

  cout << endl;
  cout << "----------------------------------------------------------------"
       << endl;
  cout << "2. Timetest of SortBinary::sortBinaryWordsFile with sorted file "
       << endl;
  cout << "(WITHOUT predefined stxxl.disk)"
       << endl;
  cout << "----------------------------------------------------------------"
       << endl;

  // Create .stxxl
  if (checkFile("stxxl.disk")) removeFile("stxxl.disk");
  writeStringToFile(string("disk=stxxl.disk,"
                          + intToString(diskSize / 1024 / 1024) + ",syscall"),
                    ".stxxl");
  makeTest(numLines);

  cout << endl;
  cout << "----------------------------------------------------------------"
       << endl;
  cout << "3. Timetest of SortBinary::sortBinaryWordsFile with sorted file "
       << endl;
  cout << "(WITH predefined stxxl.disk)"
       << endl;
  cout << "----------------------------------------------------------------"
       << endl;

  // Create .stxxl
  if (checkFile("stxxl.disk")) removeFile("stxxl.disk");
  writeStringToFile(string("disk=stxxl.disk,"
                          + intToString(diskSize / 1024 / 1024) + ",syscall"),
                    ".stxxl");
  // Create stxxl.disk
  createDisk("stxxl.disk", diskSize);
  if (checkFile("stxxl.disk"))
  {
    makeTest(numLines);
    removeFile("stxxl.disk");
  }
  else
  {
    cout << "Could not create stxxl.disk for Test 3" << endl;
  }
}

// _____________________________________________________________________________
int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printUsage();
    exit(1);
  }
  size_t numLines = atoi(argv[1]);

  cout.setf(std::ios::fixed);
  cout.precision(1);
  cout << endl;
  perfSortBinaryWordsFile(numLines);
  // Remove tested file file.tmp
  removeFile("BinarySortPerf.TMP.binary");
  return 0;
}
