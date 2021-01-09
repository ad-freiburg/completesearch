#include "Globals.h"
#include "HYBCompleter.h"
#include "CompletionServer.h"

void printUsage() { cout << "Usage: test-asynchronous <db-name>" << endl; }
   
//! MAIN 
int main(char argc, char** argv)
{
  cout << endl << EMPH_ON << "TEST ASYNCHRONOUS IO (" << VERSION << ")" << EMPH_OFF << endl << endl;

  if (optind >= argc) { printUsage(); exit(1); }
  string baseName = argv[optind++];
  string indexFileName = baseName + ".hybrid";
  string excerptsDBFileName = baseName + ".docs.db";
  string vocabularyFileName = baseName + ".vocabulary";
  string logFileName = baseName + ".log";
  string prefix = optind < argc ? argv[optind++] : "b";
  Timer timer;

  unsigned long int initIntBuffSize = INTERSECTION_BUFFER_INIT;
  unsigned long int initUniBuffSize = UNION_BUFFER_INIT;
  HYBCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> completer(indexFileName, vocabularyFileName, initIntBuffSize, initUniBuffSize);
  cout << endl;
  completer._metaInfo.show();
  cout << endl;

  bool notIntersectionMode;
  const WordRange wordRange = completer.prefixToRange(prefix, notIntersectionMode);
  BlockId firstBlockId, lastBlockId;
  completer.blockRangeForNonEmptyWordRange(wordRange, firstBlockId, lastBlockId);

  BlockId blockId = firstBlockId;
  DocList docList;
  WordList wordList;
  Vector<Position> positionList;
  Vector<DiskScore> scoreList;
  for (unsigned int run = 1; run <= 3; ++run)
  {
    cout << "get block for \"" << prefix << "\" ... " << flush;
    timer.start();
    completer.getDataForBlockId(blockId, docList, positionList, scoreList, wordList);
    timer.stop();
    cout << "done in " << timer <<  endl;
  }
  cout << endl;
 
} // end of startServer
