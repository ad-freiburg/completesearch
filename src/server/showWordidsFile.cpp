#include <iostream>
#include <string>
#include <vector>
#include "Globals.h"
#include "INVCompleter.h"

using namespace std;

void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: showWordidsFile wordids-file vocabulary-file" << EMPH_OFF << endl
       << endl
       << "Scans the wordids file, and by translating the wordids to words via the vocabulary file," << endl
       << "outputs an ordinary words file (line by line, to STDOUT)" << endl
       << endl;
}

int main(int argc, char** argv)
{

  if (argc != 3) { printUsage(); exit(1); }
  string wordidsFileName = argv[1];
  string vocFileName = argv[2];

  cout << endl 
       << EMPH_ON << "FILE OF WORDID-DOCID PAIRS -> WORD-DOCID LINES" << EMPH_OFF << " (" << wordidsFileName << ")" << endl 
       << endl;
  
  // READ VOCABULARY
  vector<string> vocabulary;
  INVCompleter<NO_DUPS_NO_POS> completer; /* just a dummy object, to have access to the following method */
  completer.readWordsFromFile(vocFileName, vocabulary);
  cout << endl;

  // SCAN WORIDS FILE AND OUPUT LINES OF WORD-DOCID PAIRS
  FILE* file = fopen(wordidsFileName.c_str(), "r");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << wordidsFileName 
                           << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  unsigned int linesRead = 0;
  typedef pair<WordId,DocId> WordIdDocIdPair;
  WordIdDocIdPair wordIdDocIdPair;
  while (true)
  {
    fread(&wordIdDocIdPair, sizeof(WordIdDocIdPair), 1, file);
    if (feof(file) != 0) break; /* last fread read EOF */
    cout << vocabulary[wordIdDocIdPair.first-1] << "\t" << wordIdDocIdPair.second << endl;
    linesRead++;
  }
  fclose(file);

}
