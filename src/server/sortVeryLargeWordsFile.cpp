#include <iostream>
#include <string>
#include <ext/hash_map>
#include "Globals.h"
#include "WordsFile.h"
#include "Timer.h"
#include <stxxl>

using namespace std;
using namespace __gnu_cxx; /* for hash_map */

void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: sortVeryLargeWordsFile words-file vocabulary-file memory-for-sorting-in-MB" << EMPH_OFF << endl
       << endl
       << "Scans the file <db>.words and pushes lines to an stxxl::vector< pair<DocId,WordId> >," << endl
       << "and then sorts the vector using stxxl::sort." << endl
       << endl
       << "The required space lies *significantly* below the space taken by the words-file" << endl
       << "(about 10% if we ignore positions, as we currently do)." << endl
       << endl;
}

//! CLASSES FOR STXXL-SORTING vector of wordid-docid pairs
class KeyExtractor
{
  public:
    typedef stxxl::uint64 key_type;    
    key_type operator()(const pair<WordId,DocId>& x) const { return *((stxxl::uint64*)(&x)); }
    key_type min_value() const { return 0; }
    key_type max_value() const { return (stxxl::uint64)(-1); } /* gives 18,446,744,073,709,551,615 */
};

typedef pair<WordId,DocId> WordIdDocIdPair;
WordIdDocIdPair MIN_WORD_ID_DOC_ID_PAIR = pair<WordId,DocId>(MIN_WORD_ID,MIN_DOC_ID);
WordIdDocIdPair MAX_WORD_ID_DOC_ID_PAIR = pair<WordId,DocId>(MAX_WORD_ID,MAX_DOC_ID);
class CompareWordIdDocIdPair 
{
  public:
    bool operator()(const WordIdDocIdPair& x, const WordIdDocIdPair& y) const 
      { return x.first < y.first || (x.first == y.first && x.second < y.second) ; } 
    WordIdDocIdPair min_value() const { return MIN_WORD_ID_DOC_ID_PAIR; }
    WordIdDocIdPair max_value() const { return MAX_WORD_ID_DOC_ID_PAIR; }
};

//
// WRITE STXXL::VECTOR OF WORDID-DOCID PAIRS TO DISK 
//
#define LEAVE_DUPLICATE_LINES 0
#define REMOVE_DUPLICATE_LINES 1
void writeWordIdDocIdPairsToDisk(stxxl::vector<WordIdDocIdPair>& pairs, 
                                 string filename, 
                                 bool MODE = 0,
                                 string nameOfPairs = "vector")
{
  cout << "* writing " << nameOfPairs << " to \"" << filename << "\" ... " << flush;
  FILE* file = fopen(filename.c_str(), "w");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << filename 
                           << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  off_t linesWritten = 0;
  Timer timer;
  timer.start();
  for (unsigned int i = 0; i < pairs.size(); ++i)
    if (MODE == LEAVE_DUPLICATE_LINES || i == 0 || pairs[i].first != pairs[i-1].first || pairs[i].second != pairs[i-1].second)
    {
      fwrite((void*)&(pairs[i]), sizeof(WordIdDocIdPair), 1, file);
      //fprintf(file, "%u\t%lu\n", words[i].first, words[i].second);
      linesWritten++;
    }
  timer.stop();
  cout << "done in " << timer << " (" << commaStr(linesWritten) << " wordid-docid pairs, "
       << (linesWritten*sizeof(WordIdDocIdPair))/timer.usecs() << " MiB/sec)" << endl << endl;
  fclose(file);
}

//
// MAIN
//
int main(int argc, char** argv)
{
  if (argc != 4) { printUsage(); exit(1); }
  string wordsFileName = argv[1];
  string vocFileName = argv[2];
  unsigned int M = atoi(argv[3]) * 1024 * 1024; /* main memory for sort */
  char* db_name = basename(strdup(wordsFileName.c_str())); /* strip of path */
  string dbName = db_name != NULL ? db_name : "DEFAULT";
  dbName.erase(dbName.rfind('.')); /* strip of suffix (presumably .words) */
  string wordidsUnsortedFileName = dbName + ".wordids_unsorted";
  string wordidsSortedFileName = dbName + ".wordids_sorted";
  Timer timer;

  cout << endl 
       << EMPH_ON << "SORT VERY LARGE WORDS FILE" << EMPH_OFF << endl
       << endl
       << "! will read from file \"" << wordsFileName << "\" (SHORT_FORMAT)" << endl
       << "! will write file of unsorted wordid-docid pairs to file \"" << wordidsUnsortedFileName << "\" (" 
                                                                        << sizeof(WordIdDocIdPair) << " bytes per pair)" << endl
       << "! will write file of sorted wordid-docid pairs to file \"" << wordidsSortedFileName << "\" (" 
                                                                      << sizeof(WordIdDocIdPair) << " bytes per pair)" << endl
       << "! amount of main memory provided for stxxl::sort is " << M/(1024*1024) << " MB" << endl
       << endl;

  
  //
  // READ VOCABULARY INTO HASH   void readWordsFromFile(const string fileName, vector<string>& words)
  //
  hash_map<string, WordId, StringHashFunction> vocabulary; /* words -> frequencies */
  {
    FILE* vocFile = fopen(vocFileName.c_str(), "r");
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
  // WORDS FILE -> STXXL VECTOR (with words -> word ids)
  //
  stxxl::vector<WordIdDocIdPair> words; cout << endl; /* STXXL will print message */
  cout << "* reading word-docid pairs and storing in stxxl::vector of wordid-docid pairs ... " << flush;
  WordsFile wordsFile(wordsFileName, SHORT_FORMAT);
  off_t nofWordsNotInVocabulary = 0;
  timer.start();
  {
    string word;
    DocId docId;
    Position pos;
    WordIdDocIdPair wordIdDocIdPair;
    while (wordsFile.isEof() == false)
    {
      // NOTE: cannot skip same word-docid pairs, beacuse the words file is not sorted!
      if ( wordsFile.getNextLine(word, docId, pos) == false ) continue;
      // ignore words which are not in the vocabulary 
      if ( vocabulary.count(word) == 0 ) { nofWordsNotInVocabulary++; continue; }
      wordIdDocIdPair.first = vocabulary[word];
      wordIdDocIdPair.second = docId;
      words.push_back(wordIdDocIdPair);
      if ( (wordsFile.lineNumber() % 1000000) == 0 ) cout << "[" << docId << "]" << flush;
    } 
  }
  timer.stop();
  cout << " ... done in " << timer << " (" << commaStr(words.size()) << " wordid-docid pairs" << flush;
  if (timer.usecs() == 0) cout << endl; else
  cout << ", " << wordsFile.nofBytesRead()/timer.usecs() << " MiB/second)" << endl << endl;
  if (nofWordsNotInVocabulary > 0)
  cout << "! dropped " << commaStr(nofWordsNotInVocabulary) 
       << " word-docid pairs, because word was not in vocabulary" << endl << endl;
  wordsFile.close();

  //
  // WRITE THE UNSORTED VECTOR TO DISK (in case the sorting crashes)
  //
  writeWordIdDocIdPairsToDisk(words, wordidsUnsortedFileName, LEAVE_DUPLICATE_LINES, "unsorted stxxl::vector");

  //
  // SORT THE STXXL VECTOR (primary key: word id, secondary key: doc id)
  //
  cout << "* sorting " << commaStr(words.size()) << " wordid-docid pairs ("
       << commaStr((sizeof(WordIdDocIdPair)*words.size())/(1024*1024)) << " MB)"
       << " in inverted-index order ... " << flush;
  timer.start();
  stxxl::sort(words.begin(),words.end(),CompareWordIdDocIdPair(),M);
  //std::sort(words.begin(),words.end(),CompareWordIdDocIdPair());
  timer.stop();
  cout << "done in " << timer << flush;
  if (timer.usecs() == 0) cout << endl; else
  cout << " (" << (sizeof(WordIdDocIdPair)*words.size())/timer.usecs() << " MiB/sec)" << endl << endl;


  //
  // WRITE THE SORTED VECTOR TO DISK (leaving out duplicates!)
  //
  writeWordIdDocIdPairsToDisk(words, wordidsSortedFileName, REMOVE_DUPLICATE_LINES, "sorted stxxl::vector");

}
