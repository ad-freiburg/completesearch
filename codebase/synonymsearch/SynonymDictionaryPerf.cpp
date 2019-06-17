#include "SynonymDictionary.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <google/dense_hash_map>
#include <google/sparse_hash_map>
#include <ext/hash_map>
#include <getopt.h>

using namespace std;
using namespace __gnu_cxx;

// Fix for ext::hash_map
/** BEGIN FIX **/
namespace __gnu_cxx
{
  template<> struct hash< std::string >
  {
    size_t operator()( const std::string& x ) const
    {
      return hash< const char* >()( x.c_str() );
    }
  };
}
/** END FIX **/

using google::dense_hash_map;
using google::sparse_hash_map;


char LINE[] = "12345 12345 12345 12345 12345 \n";


void printUsage()
{
  cout << endl
       << "Usage: SynonymDictionaryPerf [options]" 
       << endl << endl
       << "Options:"
       << endl << endl
       << "-w words number            Choose number of the words to be tested for test of different hash_maps (default 500000)" << endl
          << endl
       << "-l lines number            The testfile for the test SynonymDictionary::readFromFile will have this number of lines (default 100000)" << endl
       << "                           each line looks like this \"12345 12345 12345 12345\"" << endl
          << endl 
       << "-r number of repetitions   How often each test should be repeated (default 1)" << endl
       << endl;
}

void outTest(double time, double dataSize, int wordN){

  double speed = ((double)dataSize / (1024 * 1024)) / time;
  int wSpeed = wordN / time;

  cout << "measured time : " << setw(4) << time << " sec";
  cout << "; speed : "<< setw(5) << speed << " MB/sec, ";
  cout << setw(9) << wSpeed << " Words/sec" << endl;

}

void perfReadFromFile(int numLines, int numberOfRepeat){

  cout << "1. Time SynonymDictionary::readFromFile." << endl << endl;

  // Create temporary filename.
  // char tmpFileName[] = "XXXXXXXX";
  char tmpFileName[] = "tmp";
  // int fileDescriptor = mkstemp(tmpFileName);

  // assert(fileDescriptor != -1);

  // Create filestream.
  FILE* file;
  // file = fdopen(fileDescriptor, "w+");
  file = fopen(tmpFileName, "w");

  assert(file);

  // Write LINE MAX_WRITE times into the stream file.
  cout << "Creating temporary SynonymDictionary file with " 
       << numLines << " lines (all identical) ... " << flush;
  for (int i = 0; i < numLines; i++)
  {
    fputs(LINE, file);
  }
  fclose(file);
  cout << "done." << endl;

  // Create instance of class SynonymDictionary.
  SynonymDictionary sd;

  // Determine file size.
  ifstream is;
  is.open(tmpFileName);

  assert(is.is_open());

  int fileSize;
  {
    int start = is.tellg();
    is.seekg(0, ios::end);
    int end = is.tellg();
    fileSize = end - start;
  }
  double sizeMb = (double)fileSize / 1024 / 1024;
  double sizeKb = (double)fileSize / 1024;
  cout << "  size of file : "<< sizeMb << " MB or " << sizeKb
       << " KB or " << fileSize << " Byte" << endl;
  cout << endl;

  is.close();

  // Measure clock cycles needed to map the content in tmpFileName.
  cout << "SynonymDictionary::readFromFile" << endl;
  clock_t start, end;

  for (int r = 1; r <= numberOfRepeat; r++)
  {
    cout << "   " << r << " : " << flush;
    cout << "  Reading  ... " << flush;
    start = clock();
    sd.readFromFile(tmpFileName);
    end = clock();
    int cycles = end - start;
    double time = cycles / (double)CLOCKS_PER_SEC;
    int wordNumber = 5 * numLines;
    outTest(time, fileSize, wordNumber);
  }
  // Cleaning up.
  remove(tmpFileName);
}
// Randomly create a word collection to be tested on different hash strategies
int makeWordCollection(vector<string>& collection, int numberOfWords)
{
  char randomizer[] = "qwertzuiop";
  int dataSize = 0;
  int wLength = 0;
  int character = 0;
  string word = "";
  srand ( time(NULL) );

  // creating of test data
  for( int i = 0; i < numberOfWords; i++ )
  {
    wLength = rand() % 7 + 3;

    for (int k = 0; k <= wLength; k++)
    {
      character = rand() % 10;
      word = word + randomizer[character];
      
    }
    dataSize = dataSize + word.length();
    collection.push_back(word);
    word = "";
  }
  return dataSize;
}

// Function for testing different hash stratagies, templated by type of hash
// function. It will write all words from test data inside the evaluated map.
// Then it will read writing words from map and at the end it will try
// to read the words from map, wich not exist
template <class T>
void fillHashMap(T map, const vector<string>& collection, int dataSize,int numberOfRepeat)
{
  int wordN = collection.size();

  for (int r = 1; r <= numberOfRepeat; r++)
  {
    cout << "   " << r << " : " << flush;
    
    // Speedtest of writing data in map
    cout << "  Insert   ... " << flush;
    map.clear();
    clock_t start, end;
    start = clock();

    for (int i = 0; i < wordN; i++)
    {
      map[collection[i]] = i+1;
    }

    end = clock();

    outTest((end - start) / (double)CLOCKS_PER_SEC, dataSize, wordN);


    // Speedtest of reading data from map, word inside of map
    cout << "         Lookup T ... " << flush;

   
    start = clock();

    for (int i = 0; i < wordN; i++)
    {
      map.count(collection[i]);
    }
    
    end = clock();

    outTest((end - start) / (double)CLOCKS_PER_SEC, dataSize, wordN);

    // Speedtest of reading data from map, word is not inside of map
    cout << "         Lookup F ... " << flush;

   
    start = clock();

    for (int i = 0; i < wordN; i++)
    {
      map.count("abcdef");
    }

    end = clock();

    outTest((end - start) / (double)CLOCKS_PER_SEC, dataSize, wordN);

  }

  cout << endl;
}

// main funktion of hashmaps testing
void perfHashMaps(int numberOfWords, int numberOfRepeat)
{
  cout << endl;
  cout << "2. Compare map, ext/hash_map, google/dense_hash_map, and google/sparse_hash_map." << endl
       << "Measure time for insert, lookup of words contained in map (Lookup T), and lookup" << endl
       << "of words not contained in map (Lookup F)." << endl;
  cout << endl;

  //tested ext::map
  map<string, int> simpleMap;

  //tested ext::hash_map
  hash_map<string, int> hashMap;
  
  //tested google::dense_hash_map
  sparse_hash_map<string, int> sparseMap;


  //tested google::dense_hash_map
  dense_hash_map<string, int> denseMap;
  denseMap.set_empty_key("");
 
  //test collection
  vector<string> collection;

  //--------------------CREATING TEST DATA----------------------

  cout << "Create test data ... " << flush;
  int dataSize = makeWordCollection(collection, numberOfWords);
  double sizeMb = (double)dataSize / 1024 / 1024;
  double sizeKb = (double)dataSize / 1024;
  if (collection.size() == 0)
  {
    cout << "  collection is empty." << endl;
  }
  else
  {
   cout << "done, an array of " << numberOfWords << " random words was created" << endl;
   cout << "  size of data : "<< sizeMb << " MB or " << sizeKb
        << " KB or " << dataSize << " Byte" << endl; 
   cout << endl;

   // -------------------TESTING------------------------------

   // testing of MAP without HASH Funktion
   cout << "ext::map without hashing" << endl;
   fillHashMap(simpleMap, collection, dataSize, numberOfRepeat);

   // testing of ext::hash_map
   cout << "ext::hash_map" << endl;
   fillHashMap(hashMap, collection, dataSize, numberOfRepeat);

   // testing of DENSE_HASH_MAP from google
   cout << "google::dense_hash_map" << endl;
   fillHashMap(denseMap, collection, dataSize, numberOfRepeat);

   // testing of SPARSE_HASH_MAP from google
   cout << "google::sparse_hash_map" << endl;
   fillHashMap(sparseMap, collection, dataSize, numberOfRepeat);

  }
}

int main(int argc, char** argv)
{
  // default setting, if argument was not set
  size_t numWords = 500000;
  size_t numLines = 100000;
  size_t numRepeat = 1;

  // Arguments : -w words number, -r repetitions number, -l lines number,
  //             -h help
  cout << "----------------------- USAGE -----------------------" << endl;
  printUsage();
  cout << "----------------------- TESTS -----------------------" << endl;
  while (true)
  {
    int c = getopt(argc, argv, "w:l:r:");
    if (c == -1) break;
    switch(c)
    {
        case 'w': numWords = atoi(optarg);  break;
        case 'l': numLines = atoi(optarg);  break;
        case 'r': numRepeat = atoi(optarg); break;
     // case 'h': printUsage(); exit(0);    break;
        default : exit(1);                  break;
    }
  }

  cout.setf(ios::fixed);
  cout.precision(1);
  cout << endl;
  perfReadFromFile(numLines, numRepeat);
  perfHashMaps(numWords, numRepeat);

  return 0;
}
