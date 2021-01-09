#include <iostream>
#include <string>
#include <ext/hash_map>
#include "Globals.h"
#include "WordsFile.h"
#include "Timer.h"
#include <sys/stat.h>

using namespace std;
using namespace __gnu_cxx; /* for hash_map */

void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: word2wordids words-file vocabulary-file wordids-file" << EMPH_OFF << endl
       << endl;
}

typedef pair<WordId,DocId> WordIdDocIdPair;

//
// MAIN
//
int main(int argc, char** argv)
{
  if (argc != 4) { printUsage(); exit(1); }
  string wordsFileName = argv[1];
  string vocFileName = argv[2];
  string wordidsFileName = argv[3];
  struct stat buf; 
  if (stat(wordidsFileName.c_str(), &buf) == 0) { cout << endl << "! ERROR : file \"" << wordidsFileName 
                                                       << "\" exists, first (re)move" << endl << endl; exit(1); }
  Timer timer;

  cout << endl 
       << EMPH_ON << "WORDS to WORDIDS : " << EMPH_OFF << wordsFileName << " -> " << wordidsFileName << endl

       << endl
       << "! will read word-docid pairs from file \"" << wordsFileName << "\" (expecting SHORT_FORMAT)" << endl
       << "! will write file of (unsorted) wordid-docid pairs to file \"" << wordidsFileName << "\" (" 
                                                                        << sizeof(WordIdDocIdPair) << " bytes per pair)" << endl
       << endl;

  
  //
  // READ VOCABULARY INTO HASH   
  //
  hash_map<string, WordId, StringHashFunction> vocabulary; /* words -> frequencies */
  {
    FILE* vocFile = fopen(vocFileName.c_str(), "r");
    char vocBuffer[FILE_BUFFER_SIZE];
    setbuffer(vocFile, vocBuffer, FILE_BUFFER_SIZE);
    if (vocFile == NULL) { cout << endl << "! ERROR opening file \"" << vocFileName 
                             << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
    cout << "* reading vocabulary from file \"" << vocFileName << "\" ... " << flush;
    char word[MAX_WORD_LENGTH+1];
    WordId wordId = 0;
    while (fgets(word, MAX_WORD_LENGTH, vocFile))
    {
      wordId++;
      int len = strlen(word);
      if (len == MAX_WORD_LENGTH - 1)
        { cout << endl << "! ERROR reading from file \"" << vocFileName
                       << "\" (line too long)" << endl << endl; exit(1); }
      while (len > 0 && iswspace(word[len-1])) word[--len] = 0; /* remove trailing whitespace (ip newline) */
      if (len == 0) continue; /* ignore empty lines */
      vocabulary[word] = wordId;
    }
    cout << "done (" << commaStr(vocabulary.size()) << " words)" << endl << endl;
    fclose(vocFile);
  }

  //
  // WORDS FILE -> WORDIDS FILE
  //
  cout << "* reading word-docid pairs and writing corresponding wordid-docid pairs ... " << flush;
  WordsFile wordsFile(wordsFileName, "SHORT", DONT_MAINTAIN_MAX_DOC_ID);
  FILE* wordidsFile = fopen(wordidsFileName.c_str(), "w");
  char wordidsBuffer[FILE_BUFFER_SIZE];
  setbuffer(wordidsFile, wordidsBuffer, FILE_BUFFER_SIZE);
  if (wordidsFile == NULL) { cout << endl << "! ERROR opening file \"" << wordidsFileName 
                                  << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  off_t nofWordsNotInVocabulary = 0;
  off_t linesWritten = 0;
  timer.start();
  {
    string word;
    DocId docId;
    DiskScore score; /* not used, but needed as argument to getNextLine */ // NEW 09Oct06 (Holger): Score -> DiskScore
    Position pos; /* same as for score */
    WordIdDocIdPair wordIdDocIdPair;
    while (wordsFile.isEof() == false)
    {
      // NOTE: cannot skip same word-docid pairs, beacuse the words file is not sorted!
      if ( wordsFile.getNextLine(word, docId, score, pos) == false ) continue;
      // ignore words which are not in the vocabulary 
      if ( vocabulary.count(word) == 0 ) { nofWordsNotInVocabulary++; continue; }
      //
      wordIdDocIdPair.first = vocabulary[word];
      wordIdDocIdPair.second = docId;
      fwrite(&wordIdDocIdPair, sizeof(WordIdDocIdPair), 1, wordidsFile);
      ++linesWritten;
      if ( (wordsFile.lineNumber() % 1000000) == 0 ) 
      {
        timer.stop();
        if (timer.usecs() > 0) /* just to prevent crash from division by zero */
        cout << "[" << docId << "|" 
             << (wordsFile.totalNofBytesRead() + sizeof(WordIdDocIdPair))/timer.usecs() 
             << " MiB/s]" << flush;
        timer.cont();
      }
    } 
  }
  timer.stop();
  cout << " ... done in " << timer 
       << " (read " << commaStr(wordsFile.lineNumber()) << " word-docid pairs" 
       << ", written " << commaStr(linesWritten) << " wordid-docid pairs)" 
       << endl << endl;
  if (nofWordsNotInVocabulary > 0)
  cout << "! dropped " << commaStr(nofWordsNotInVocabulary) 
       << " word-docid pairs, because word was not in vocabulary" << endl << endl;
  wordsFile.close();
  fclose(wordidsFile);

}
