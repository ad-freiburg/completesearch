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
       << EMPH_ON << "Usage: sortWordidsFileInPlace wordids-file memory-for-sort-in-MB" << EMPH_OFF << endl
       << endl
       << "Sorts the given file of 8-byte wordid-docid pairs IN PLACE. DANGER: if some error happens" << endl
       << "(as it did in the past) the file might be corrupted afterward, so always RUN ON A COPY!" << endl
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
  if (argc != 3 or atoi(argv[2]) == 0) { printUsage(); exit(1); }
  string wordidsFileName = argv[1];
  unsigned int M = atoi(argv[2])*1024*1024;
  Timer timer;

  cout << endl 
       << EMPH_ON << "SORT WORDIDS FILE *IN PLACE*" << EMPH_OFF << endl
       << endl
       << "! will replace file \"" << wordidsFileName << "\" by a sorted version" << endl
       << "! will sort with stxxl::sort provided with " << M/(1024*1024) << " MB of main memory" << endl
       << "! no duplicates are removed" << endl
       << endl;

  //
  // MAKE FILE AN STXXL::VECTOR (no copying involved)
  //
  #ifdef STXXL_DIRECT_IO_OFF 
  stxxl::syscall_file f(wordidsFileName,stxxl::file::RDWR);
  #else
  stxxl::syscall_file f(wordidsFileName,stxxl::file::DIRECT|stxxl::file::RDWR);
  #endif
  stxxl::vector<WordIdDocIdPair> pairs(&f);
  cout << endl; /* vector creation will produce an [STXXL-MSG] message */

  //
  // SORT WORDID_DOCID PAIRS IN PLACE
  //
  cout << "* sorting file \"" << wordidsFileName << "\" in place ... " << flush;
  timer.start();
  stxxl::sort(pairs.begin(),pairs.end(),CompareWordIdDocIdPair(),M);
  timer.stop();
  cout << "done in " << timer << " (" << commaStr(pairs.size()) << " wordid-docid pairs, "
       << megsPerSecond(sizeof(WordIdDocIdPair)*pairs.size(), timer.usecs()) 
       << ")" << endl << endl;

}
