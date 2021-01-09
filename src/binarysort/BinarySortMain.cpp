// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include "./BinarySort.h"

using std::cout;
using std::endl;
using std::flush;
using std::cerr;

// _____________________________________________________________________________
// Usage information
void printUsage()
{
  cout << endl
       << "USAGE : ./BinarySort -f [binary-words-file] [other options]"
       << endl
       << endl
       << "Options are:"
       << endl
       << "\t--binary-words-(f)ile: name of the file to be sorted"
       << " (REQUIRED)."
       << endl
       << "\t--stxxl-(p)ath:        path for stxxl config and disk"
       << " (default: path of words file)."
       << endl
       << "\t--words_file_(b)ase:   name's base of the file to be sorted"
       << endl
       << "\t--stxxl-(d)isk-file:   name of stxxl disk"
       << " (default: ./stxxl.disk)."
       << endl
       << "\t--stxxl-disk-(s)ize:   the size of stxxl disk  Exp: 130M, 2G etc."
       << " (default: the size of words file + 10%)."
       << endl
       << "\t--(u)nique:   if the flag is set, duplicates are removed."
       << " (default: false)."
       << endl
       << endl;
}
// _____________________________________________________________________________
// Return base of given file name
string parseFileName(const string& fileName)
{
  int pos = 0;
  pos = fileName.find('.');
  if (pos == 0)
  {
    cout << "bad file name." << endl;
    exit(1);
  }
  return fileName.substr(0, pos);
}
// _____________________________________________________________________________
// Return int digit for charakter digit
int isDigit(char digit)
{
  switch (digit)
  {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    default: return -1;
  }
}
// _____________________________________________________________________________
// parsing of given Size for stxxl disk from --stxxl-disk-size option
int64_t parseSTXXLDiskSize(const char* size)
{
  int n = strlen(size) - 1;
  int64_t res = 0;
  int64_t base = 1;
  if (n > 0)
  {
    switch (size[n])
    {
      case 'B':
        base = 1;
        break;
      case 'K':
        base = 1024;
        break;
      case 'M':
        base = 1024 * 1024;
        break;
      case 'G':
        base = 1024 * 1024 * 1024;
        break;
      default:
        return -1;
    }
  } else
  {
    return -1;
  }

  n--;
  int mul = 1;
  int digit;
  for (int i = n; i >= 0; i--)
  {
    digit = isDigit(size[i]);
    if (digit != -1)
    {
      res += mul * digit;
      mul *= 10;
    } else
    {
      return -1;
    }
  }

  // It can be that the user specify too small disk Size
  // (It must be minimum 1Mb)
  // In this fall return 1Mb
  if (res * base < 1024 * 1024)
    return 1024 * 1024;
  else
    return res * base;
}
// _____________________________________________________________________________
// Check if file exists
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
// Return the size of given file
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
// Parse int number to string
string intToString(int64_t num)
{
  std::stringstream sstr;
  sstr << num;
  string buf;
  sstr >> buf;
  return buf;
}

// _____________________________________________________________________________
// Create file of given size with given name
void createDisk(const string& diskFileName, const int64_t &diskSize)
{
  int64_t size = 0;

  if (diskSize < 1024 * 1024)
    size = 1;
  else
    size = diskSize / 1024 / 1024;

  string buf = intToString(size);
  string sysLine = "dd if=/dev/zero bs=1048576 count=" +
          buf + " of=" + diskFileName;

  // Create file with dd programe
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
// Write config for stxxl sort. Name of the config file is ".stxxl"
void writeSTXXLConfig(const string &path,
        string *wordsFileName,
        string *diskFileName,
        const int64_t &diskSize)
{
  if (wordsFileName->length() == 0)
  {
    cerr << "wordsFile was not defined!" << endl;
    printUsage();
    exit(1);
  }

  // If stxxl disk path was not given, then create disk under the path, where
  // words file is
  if (diskFileName->length() == 0)
  {
    *diskFileName = path + "stxxl.disk";
  }

  FILE* stxxlConfig = fopen(string(path + ".stxxl").c_str(), "w");
  if (stxxlConfig == NULL)
  {
    cerr << "STXXL Config File could not be writen!"
            << string(path + ".stxxl")
            << endl;
    exit(1);
  }

  // Print entry in .stxxl
  if (checkFile(*diskFileName))
  {
    // If the disk exists and size of disk was not set,
    // than just use the disk, wich already exists
    // If the disk exists and size was set,
    // than create a disk with given size
    if (diskSize == -1)
    {
      fprintf(stxxlConfig, "disk=%s,%d,syscall",
              diskFileName->c_str(),
              static_cast<int> (getSizeOfFile(*diskFileName)
              / 1024 / 1024));
    } else
    {
      if (diskSize != getSizeOfFile(*diskFileName))
        createDisk(*diskFileName, diskSize);

      fprintf(stxxlConfig, "disk=%s,%d,syscall",
              diskFileName->c_str(),
              static_cast<int> (diskSize / 1024 / 1024));
    }
  } else
  {
    // If the disk does not exists and size of disk was not set,
    // than create the disk with the size of wordsFile
    // If the disk does not exists and size was set,
    // than create a disk with given size
    if (diskSize == -1)
    {
      int64_t wordsFileSize = getSizeOfFile(*wordsFileName);
      wordsFileSize += (wordsFileSize * 10) / 100;
      if (wordsFileSize < 1024 * 1024)
        wordsFileSize = 1024 * 1024;

      createDisk(*diskFileName, wordsFileSize);

      fprintf(stxxlConfig, "disk=%s,%d,syscall",
              diskFileName->c_str(),
              static_cast<int> (wordsFileSize / 1024 / 1024));
    } else
    {
      createDisk(*diskFileName, diskSize);

      fprintf(stxxlConfig, "disk=%s,%d,syscall",
              diskFileName->c_str(),
              static_cast<int> (diskSize / 1024 / 1024));
    }
  }

  fclose(stxxlConfig);
}
// _____________________________________________________________________________
// Return the pathe of given file
string getPath(const string &file)
{
  string sysLine = "find $PWD -name " + file + "> path.txt";
  int ret = system(sysLine.c_str());
  if (ret != 0)
  {
    cerr << "Could not identify path of wordsFile\n"
         << "Use option --words-file-path for manual specification"
         << endl;
    exit(1);
  }
  std::fstream pathFile("path.txt", std::ios::in);
  if (!pathFile.is_open())
  {
    cerr << "Could not identify path of wordsFile\n"
         << "Use option --words-file-path for manual specification"
         << endl;
    exit(1);
  }
  char buf[256];
  pathFile.getline(buf, sizeof(buf));
  if (string(buf).length() == 0)
  {
    cerr << "Could not identify path of wordsFile\n"
         << "Use option --words-file-path for manual specification"
         << endl;
    exit(1);
  }

  size_t found = string(buf).find_last_of("/");

  if (found == string::npos)
  {
    cerr << "Could not identify path of wordsFile\n"
         << "Use option --words-file-path for manual specification"
         << endl;
    exit(1);
  }

  string path = string(buf).substr(0, found+1);

  if (path.length() == 0)
  {
    cerr << "Could not identify path of wordsFile\n"
         << "Use option --words-file-path for manual specification"
         << endl;
    exit(1);
  }

  sysLine = "rm -f path.txt";
  ret = system(sysLine.c_str());
  if (ret != 0)
  {
    cerr << "Could not delete path.txt"
         << endl;
    exit(1);
  }
  return path;
}
// _____________________________________________________________________________

int main(int argc, char** argv)
{
  cout << endl << "\x1b[1mBINARY SORT of words file\x1b[0m" << endl << endl;
  if (argc < 2)
  {
    printUsage();
    exit(1);
  }

  // Needed for getopt_long.
  int optChr;
  // Name of words file to be sorted
  string binaryWordsFile = "";
  // Name of stxxl disk file
  string stxxlDiskFile = "";
  // The pathe, wehre words file is
  string path ="";
  // The base of words file name
  string wordsFileNameBase = "";

  // flag if duplicates should be removed
  bool unique = false;

  // The size of stxxl disk
  int64_t stxxlDiskSize = -1;

  while (1)
  {
    int optionIndex = 0;

    // Define command-line options.
    static struct option longOptions[] =
    {
      {"help", no_argument, 0, 'h'},
      {"binary-words-file", required_argument, 0, 'f'},
      {"stxxl-path", required_argument, 0, 'p'},
      {"words-file-base", required_argument, 0, 'b'},
      {"stxxl-disk-file", required_argument, 0, 'd'},
      {"stxxl-disk-size", required_argument, 0, 's'},
      {"unique", no_argument, 0, 'u'},
      {0, 0, 0, 0}
    };

    optChr = getopt_long(argc, argv, "hf:p:b:d:s:u", longOptions, &optionIndex);

    if (optChr == -1) break;

    switch (optChr)
    {
      case 0:
        break;
      case 'h':
        printUsage();
        exit(1);
        break;
      case 'u':
        cout << "Enabled unique-option!" << endl;
        unique = true;
        break;
      case 'f':
        binaryWordsFile = string(optarg);
        cout << "Specified binary file:               "
                << binaryWordsFile << endl;
        break;
      case 'p':
        path = string(optarg);

        if (optarg[strlen(optarg)-1] != '/')
          path += "/";
        cout << "Path for stxxl and words Files:      " << path << endl;
        break;
      case 'b':
        wordsFileNameBase = string(optarg);
        cout << "Specified base of binary file:       "
                << wordsFileNameBase << endl;
        break;
      case 'd':
        stxxlDiskFile = string(optarg);
        cout << "Specified stxxl disk file:           "
                << stxxlDiskFile << endl;
        break;
      case 's':
        stxxlDiskSize = parseSTXXLDiskSize(optarg);
        if (stxxlDiskSize == -1)
        {
          cerr << "False format of given stxxl disk size: "
                  << string(optarg)
                  << endl;
          printUsage();
          exit(1);
        }
        cout << "Specified size of stxxl disk file:   "
                << stxxlDiskSize / 1024 / 1024
                << "M" << endl;
        break;
      default:
        cerr << "unknown option: " << optChr << " ??" << endl;
        printUsage();
        exit(1);
        break;
    }
  }

  if ((path.length() == 0) && checkFile(binaryWordsFile))
  {
    path = getPath(binaryWordsFile);
    cout << "Path for stxxl and words Files:      " << path << endl;
  }

  cout << endl << endl;

  // Wite config file for stxxl sort
  writeSTXXLConfig(path, &binaryWordsFile, &stxxlDiskFile, stxxlDiskSize);

  if (wordsFileNameBase.length() == 0)
  {
    wordsFileNameBase = parseFileName(binaryWordsFile);
  }

  BinarySort s;
  s.setRemoveDuplicates(unique);
  timeval start, end;
  gettimeofday(&start, 0);

  // Sorting with stxxl::sort
  cout << "Sorting of " << binaryWordsFile << " and making "
          << wordsFileNameBase + ".words-sorted.binary"
          << "... " << flush;
  s.sortBinaryWordsFile(binaryWordsFile);

  gettimeofday(&end, 0);

  cout << "done" << endl;
  int time = end.tv_sec - start.tv_sec;
  int h = (time / 60) / 60;
  int min = time / 60 - h * 60;
  int sec = time - (h * 60 + min) * 60;
  cout << endl << "Sorted in " << h << "h "
          << min << "m "
          << sec << "s "
          << endl << endl;

  // Rename: *.words_unsorted_binary -> words_sorted_binar

  int result = rename(binaryWordsFile.c_str(),
          (wordsFileNameBase + ".words-sorted.binary").c_str());
  if (result == 0)
    cout << "File successfully renamed in "
          << wordsFileNameBase << ".words-sorted.binary"
          << endl << endl;
  else
    perror("Error renaming file");
  return 0;
}
