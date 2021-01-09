// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

void printUsage()
{
  cout << "Specify the binary words file" << endl;
}

string parseFileName(const string& fileName)
{
    unsigned pos = 0;
    pos = fileName.rfind(".TMP.");
    if (pos == string::npos)
    {
       pos = fileName.find('.');
       if (pos == string::npos)
           return fileName;
    }
    else
       pos += 4;
    return fileName.substr(0, pos);
}

void fillVocVector(vector<string>& voc, const string& vocFile)
{
  FILE* wordsFile = NULL;
  if ((wordsFile = fopen(vocFile.c_str(), "r")) == NULL)
  {
    std::cerr << vocFile << " could not be opened!" << endl;
    exit(1);
  }

  string line;
  string word;
  string buf;
  char buffer[256];
  int pos = 0;

  while (fgets(buffer, 256, wordsFile))
  {
    line = string(buffer);
    pos = line.find('\n');
    word = line.substr(0, pos);
    voc.push_back(word);
    word.clear();
  }

  fclose(wordsFile);
}

void writeAsciiFile(vector<string>* voc,
                    const string& fileAscii,
                    const string& binaryFile)
{
  FILE* fileIn;
  fileIn = fopen(binaryFile.c_str(), "rb");
  if (fileIn == NULL)
  {
      perror("opening binary file");
      exit(1);
  }

  std::fstream fileOut(fileAscii.c_str(), std::ios::out);

  int a[4];
  unsigned int recordCount = 0;
  while (!feof(fileIn))
  {
    int ret = fread(&a, 1, sizeof(a), fileIn);
    if ((ret != 16) && (!feof(fileIn)))
    {
      std::cerr << "Error by reading of file: "
                << binaryFile
                << endl;
      exit(1);
    }
    if (feof(fileIn)) break;
    unsigned wordId = a[0];
    if (wordId < 0 || wordId >= voc->size())
    {
      std::cerr << "! word id out of range (" << wordId << ") at record #"
                << recordCount << endl;
      exit(1);
    }
    string s = voc->operator[](wordId);
    fileOut << s << "\t" << a[1]  << "\t" << a[2]  <<"\t" << a[3] << endl;
    ++recordCount;
  }
  fclose(fileIn);
  fileOut.close();
}

int main(int argc, char** argv)
{
  cout << "~~~~~~~~~~~~~~~~~~~~" << endl
       << "ConvertBinaryToAscii" << endl
       << "~~~~~~~~~~~~~~~~~~~~" << endl;

  if (argc != 2)
  {
    printUsage();
    exit(1);
  }

  string wordsFileName = string(argv[1]);

  cout << "Specified words file to sort: " << wordsFileName << endl;

  string wordsFileNameBase = parseFileName(wordsFileName);



  vector<string> vocabulary;
  cout << "Reading vocabulary ... " << flush;
  fillVocVector(vocabulary, wordsFileNameBase + ".vocabulary");
  cout << "done (" << vocabulary.size() << " words)" << endl;

  cout << "Writing ascii file " << wordsFileNameBase + ".words"
       << " ... " << flush;
  writeAsciiFile(&vocabulary, wordsFileNameBase + ".words", wordsFileName);
  cout << "done" << endl;
}
