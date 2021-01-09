#include <iostream>
#include <string>
#include <ext/hash_map>
#include "Globals.h"
#include "WordsFile.h"

using namespace std;
using namespace __gnu_cxx; /* for hash_map */

void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: buildVocabulary words-file" << EMPH_OFF << endl
       << endl
       << "Scans a file <db>.words files and produces a file <db>.vocabulary_with_freqs" << endl
       << "with lines of the form" << endl
       << endl
       << "aachen     873456" << endl
       << "and        2364746" << endl
       << "..." << endl
       << endl
       << "where the second column is the number of documents containing the word in the first column" << endl
       << endl;
}

int main(int argc, char** argv)
{
  if (argc != 2) { printUsage(); exit(1); }
  string wordsFileName = argv[1];
  char* db_name = basename(strdup(wordsFileName.c_str())); /* strip of path */
  string dbName = db_name != NULL ? db_name : "DEFAULT";
  dbName.erase(dbName.rfind('.')); /* strip of suffix (presumably .words) */

  cout << endl 
       << EMPH_ON << "BUILD VOCABULARY" << EMPH_OFF 
       << " from file \"" << wordsFileName << "\"" << endl 
       << endl;
  
  //
  // PROCESS WORDS FILE LINE BY LINE building vocabulary with frequencies
  //
  WordsFile wordsFile(wordsFileName, "HTDIG"); // NEW 09Oct06 (Holger): was SHORT_FORMAT
  hash_map<string, unsigned int, StringHashFunction> vocabulary; /* words -> frequencies */
  string word;
  DocId docId;
  DiskScore score;
  Position pos;
  while (wordsFile.isEof() == false)
  {
    // read next word, doc id, position triple
    // NEW 09Oct06 (Holger): interface changed, now also have to pass score
    if ( wordsFile.getNextLine(word, docId, score, pos) == false ) continue;
      //cout << "--> " << word << "\t" << docId << "\t" << pos << "\n"; continue;
    // increase frequency for that word
    vocabulary[word]++;
    if ( (wordsFile.lineNumber() % 1000000) == 0 ) cout << "[" << docId << "]" << flush;
  } 
  cout << endl << endl;
  wordsFile.close();

  //
  // WRITE VOCABULARY TO FILE
  //
  string vocFileName = dbName + ".vocabulary_with_freqs";
  cout << "* writing vocabulary to file \"" << vocFileName << "\" ... " << flush;
  FILE* voc_file = fopen(vocFileName.c_str(), "w");
  unsigned int nofWords = 0;
  off_t nofWordAtPosPairs = 0;
  if (voc_file == NULL) 
    { cout << endl << "! ERROR opening file \"" << vocFileName 
                   << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  for (hash_map<string, unsigned int, StringHashFunction>::iterator it = vocabulary.begin(); it != vocabulary.end(); ++it)
  {
    if ( fprintf(voc_file, "%s\t%u\n", it->first.c_str(), it->second) < 0 )
      { cout << endl << "! WARNING : fprintf to \"" << vocFileName 
                    << "\" failed (" << strerror(errno) << ")" << endl << endl; exit(1); }
    nofWords++;
    nofWordAtPosPairs += it->second;
  }
  fclose(voc_file);
  cout << "done (" << commaStr(nofWords) << " words; " << commaStr(nofWordAtPosPairs) << " word-at-pos pairs)" << endl << endl;

} 
