#include "phonetik.h"
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::flush;

void printUsage()
{
  cout << "Usage: Call phonetikPerf without any arguments." << endl;
}

void evaluate(clock_t start, clock_t end, int bytes)
{ 
  cout << "-----------" << endl
       << "Evaluation." << endl
       << "-----------" << endl;

  clock_t diff = end - start;
  double time = diff / (double)CLOCKS_PER_SEC;
  double sizeKB = bytes / (double)1024;
  double sizeMB = sizeKB / (double)1024;
  double ratio = sizeMB / time;

  cout << "Read:\t\t"         << sizeMB << " MB." << endl
       << "Time needed:\t"  << time   << " s." << endl
       << "Ratio:\t\t"        << ratio  << " MB/s." << endl;
}

int randomWord(int length, string* result)
{
  char ch;

  // Fill result with length chars within ASCII code 97..122.
  for (int i = 0; i < length; ++i)
  {
    ch = (char) (rand() % 26) + 97; 
    (*result) += ch;
  }
 
  return 0;
}

int randomWordList(vector<string>* listOfWords)
{
  // Number of words to create.
  const int MAX = 10000000;
  
  // The words length;
  const int LENGTH = 10;

  string str = "";

  for (int i = 0; i < MAX; ++i)
  {
    randomWord(LENGTH, &str);
    (*listOfWords).push_back(str);
    str = "";
  }

  // Return number of bytes inside listOfWords.
  return LENGTH*MAX;
}

int main(int argc, char** argv)
{
  cout << "--------------------------------" << endl;
  cout << "Performance test of libtrapho.so" << endl;
  cout << "--------------------------------" << endl;
  

  if (argc != 1)
  {
    printUsage();
    exit(1);
  }

  vector<string> listOfWords;
  char result[20];

  cout << "Creating a list of random strings... " << flush;
  int bytes = randomWordList(&listOfWords);
  cout << "Done." << endl;

  cout  << "Starting performance test. " << endl
        << "The test asks for each word in the given list it's " << endl 
        << "phonetical expression returned by 'phonGerman_with'" << endl
        << "a library function of libtrapho.so... " << flush;

  clock_t start, end;
  // --------------------------------------------------------------------------
  start = clock();
  for (int i = 0; i < listOfWords.size(); ++i)
  {
    strcpy(result, phonGerman_with((unsigned char*)listOfWords[i].c_str()));
    // cout << "Word:\t\t"     << listOfWords[i] << "\t\t";
    // cout << "Phonetik:\t\t" << result << endl;
  }
  end = clock();
  // --------------------------------------------------------------------------
  cout << "Done." << endl;

  evaluate(start, end, bytes);

  return 0;
}
