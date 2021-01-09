#include <iostream>
#include <string>
#include "Globals.h"
#include "WordsFile.h"
#include "Timer.h"
#include <stxxl>

using namespace std;

//
// USAGE INFO
//
void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: sortWordidsFile wordids-file-unsorted wordids-file-sorted main-memory-for-sort-in-MB" << EMPH_OFF << endl
       << endl
       << "Reads a wordids-file containing a sequence of 8-bytes wordid-docid pairs, sorts it, and then writes" << endl
       << "the sorted variant, with duplicates removed, to a file with the given name (size will shrink by about 1/3)."  << endl
       << endl;
}

//
// COMPARISON CLASS FOR STXXL::SORT
//
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
// MAIN
//
int main(int argc, char** argv)
{
  if (argc != 4 or atoi(argv[3]) == 0) { printUsage(); exit(1); }
  string wordidsUnsortedFileName = argv[1];
  string wordidsSortedFileName = argv[2];
  unsigned int M = atoi(argv[3])*1024*1024;
  Timer timer;

  cout << endl 
       << EMPH_ON << "SORT WORDIDS FILE IN CHUNKS" << EMPH_OFF << endl
       << endl
       << "! will read unsorted wordid-docid pairs from file \"" << wordidsUnsortedFileName << "\"" << endl
       << "! will sort with stxxl::sort provided with " << M/(1024*1024) << " MB of main memory" << endl
       << "! will write sorted pairs,with duplicates removed, to file \"" << wordidsSortedFileName << "\"" << endl
       << endl;

  stxxl::vector<WordIdDocIdPair> pairs;
  cout << endl;

  //
  // READ PAIRS FROM DISK
  //
  string filename = wordidsUnsortedFileName;
  cout << "* reading pairs from \"" << filename << "\" ... " << flush;
  FILE* file = fopen(filename.c_str(), "r");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << filename 
                           << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  off_t nofPairsRead = 0;
  WordIdDocIdPair pair;
  timer.start();
  while (feof(file) == 0)
  {
    fread(&pair, sizeof(WordIdDocIdPair), 1, file);
    pairs.push_back(pair);
    if ( (++nofPairsRead % 100000) == 0 ) cout << "[" << nofPairsRead << "]" << flush;
  }
  timer.stop();
  cout << " done in " << timer << " (read " << commaStr(nofPairsRead) << " wordid-docid pairs, "
       << megsPerSecond(nofPairsRead*sizeof(WordIdDocIdPair),timer.usecs()) 
       << ")" << endl << endl;
  fclose(file);


  //
  // SORT THE STXXL VECTOR (primary key: word id, secondary key: doc id)
  //
  cout << "* sorting " << commaStr(pairs.size()) << " wordid-docid pairs ("
       << commaStr((sizeof(WordIdDocIdPair)*pairs.size())/(1024*1024)) << " MB)"
       << " in inverted-index order ... " << flush;
  timer.start();
  stxxl::sort(pairs.begin(),pairs.end(),CompareWordIdDocIdPair(),M);
  timer.stop();
  cout << "done in " << timer << " (" 
       << megsPerSecond(sizeof(WordIdDocIdPair)*pairs.size(), timer.usecs()) 
       << ")" << endl << endl;

  //
  // WRITE SORTED VECTOR TO DISK (without duplicates)
  //
  filename = wordidsSortedFileName;
  cout << "* writing sorted vector to \"" << filename << "\" ... " << flush;
  file = fopen(filename.c_str(), "w");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << filename 
                           << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  off_t linesWritten = 0;
  timer.start();
  for (off_t i = 0; i < pairs.size(); ++i)
    if (i == 0 || pairs[i].first != pairs[i-1].first || pairs[i].second != pairs[i-1].second)
    {
      fwrite((void*)&(pairs[i]), sizeof(WordIdDocIdPair), 1, file);
      if ( (++linesWritten % 100000) == 0 ) cout << "[" << linesWritten << "]" << flush;
    }
  timer.stop();
  cout << " done in " << timer << " (written " << commaStr(linesWritten) << " wordid-docid pairs, "
       << megsPerSecond((linesWritten+pairs.size())*sizeof(WordIdDocIdPair), timer.usecs()) << ")" 
       << endl << endl;
  fclose(file);

}
