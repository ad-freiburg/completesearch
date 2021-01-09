#include "Globals.h"
#include "HybCompleter.h"
#include "ScoreAggregators.h"
#include <fstream>
#include "Intersect.h"

void test1(int argc, char** argv);
void test2(int argc, char** argv);
void test3(int argc, char** argv);

bool showResults = true;

//! Main: run one of the tests
int main(int argc, char** argv)
{
  cout << endl
       << "\033[1mTIME INTERSECT OF TWO POSTING LISTS (via CompleteBaser::intersect)\033[21m" << endl
       << endl
       << "Usage: try-intersect [-t <which test>] <db name> <queries file>" << endl
       << endl;

  runMultithreaded = false;

  int whichTest = 1;
  while (true)
  {
    int c = getopt(argc, argv, "t:");
    if (c == -1) break;
    if (c == 't') whichTest = atoi(optarg);
  }

  try {
    if      (whichTest == 1) test1(argc, argv);
    else if (whichTest == 2) test2(argc, argv);
    else if (whichTest == 3) test3(argc, argv);
    else cout << "Invalid test (" << whichTest << ")" << endl; exit(1);
  } catch (Exception e) {
    cout << e.errorMessage() << endl << endl;
    exit(1);
  } catch (exception e) {
    cout << "STD exception: " << e.what() << endl << endl;
    exit(1);
  }
}


//! TEST 1
/*!
 *   Time the internal intersect routine against one rewritten by myself
 */
void test1(int argc, char** argv)
{
  if (argc <= 2) exit(1);

  string baseName = argv[optind++];
  string indexFileName = baseName + ".hybrid";
  string vocabularyFileName = baseName + ".vocabulary";
  //string excerptsDBFileName = baseName + ".docs.db";
  string queriesFileName  = argv[optind++];
  unsigned int R = 3;
  Timer timer;

  // initialize index
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HYBIndex index(indexFileName, vocabularyFileName, MODE);
  History history;
  HybCompleter<MODE> completer(index, history);
  completer._metaInfo.show();
  cout << endl;

  // process queries
  ifstream queriesFile(queriesFileName.c_str());  
  string queryString;
  unsigned int queryCount = 0;
  queriesFile >> noskipws;
  while (getline(queriesFile, queryString))
  {
    if (queryString.length() == 0 || queryString[0] == '#') continue;
    ++queryCount;
    cout << "\033[1m" << queryString << "\033[21m" << endl << endl;
    Query query(queryString);
    Query part1;
    Query part2;
    Separator separator;
    query.splitAtLastSeparator(part1, part2, separator);
    if (part1.length() == 0 || part2.length() == 0)
    {
      cout << "! Skipping query with less than two parts ...\n\n";
      continue;
    }
      //IntersectMode intersectMode(separator, OUTPUT_MATCHES);
    QueryResult* list1 = NULL;
    QueryResult* list2 = NULL;
    {
      completer.processQuery(part1, list1);
      completer.processQuery(part2, list2);
      if (list1 == NULL || list2 == NULL) 
      {
        cout << "! Empty query result, something went wrong, skipping ...\n\n";
        continue;
      }
    }
    cout << "List length 1  : " << list1->_docIds.size() << endl;
    cout << "List length 2  : " << list2->_docIds.size() << endl;
    cout << "Separator      : " << separator.infoString()      << endl;
    cout.setf(ios::fixed);
    cout.precision(3);
    for (unsigned int rounds = 1; rounds <= R; ++rounds)
    {
      QueryResult list3a;
      timer.start();
      completer.intersectTwoPostingListsOld(*list1, *list2, list3a, separator);
      timer.stop();
      cout << "List length 3a : " << list3a._docIds.size() 
           << " ... computed in " << setw(6) << timer.usecs() / 1000.0 << " msecs" << endl;
    }
    for (unsigned int rounds = 1; rounds <= R; ++rounds)
    {
      QueryResult list3b;
      timer.start();
      completer.intersectTwoPostingListsNew(*list1, *list2, list3b, separator);
      timer.stop();
      cout << "List length 3b : " << list3b._docIds.size()
           << " ... computed in " << setw(6) << timer.usecs() / 1000.0 << " msecs" << endl;
    }
    QueryResult list3a;
    QueryResult list3b;
    completer.intersectTwoPostingListsOld(*list1, *list2, list3a, separator);
    completer.intersectTwoPostingListsNew(*list1, *list2, list3b, separator);
    completer.checkCorrectnessOfIntersectionResult(*list1, *list2, list3b);
    completer.checkEqualityOfTwoPostingLists(list3a, list3b);
    //list3a.show();
    //list3b.show();
    completer.topMatchesFromAllMatches
     (list3a,
      WITHIN_DOC_AGGREGATION, 
      BETWEEN_DOCS_AGGREGATION,
      WITHIN_COMPLETION_AGGREGATION, 
      BETWEEN_COMPLETIONS_AGGREGATION,
      100,
      100);
    completer.topMatchesFromAllMatches
     (list3b,
      WITHIN_DOC_AGGREGATION, 
      BETWEEN_DOCS_AGGREGATION,
      WITHIN_COMPLETION_AGGREGATION, 
      BETWEEN_COMPLETIONS_AGGREGATION,
      100,
      100);
    cout << "Number of distinct doc ids in list 3a: " << list3a.nofTotalHits << endl;
    cout << "Number of distinct doc ids in list 3b: " << list3b.nofTotalHits << endl;

    cout << endl;
  }
}

//! TEST 2
/*!
 *
 *   Test intersect on some self-created queries
 */
void test2(int argc, char** argv)
{
  if (argc < 2)
  {
    cout << "Usage: try-intersect <file with lists>" << endl << endl;
    exit(1);
  }
  
  // Need a (dummy) completer object for call to intersect method below (e.g.,
  // statistics on times and volumes are part of the obect)
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;

  // 1. Read posting lists from give file, format is
  //
  //     10  10  10  10  20  30  30  30  30  30
  //      1   2   2   1   3   4   1   2   1   4
  //     18  19  29  35  11  11  12  13  14  15
  //      1   1   1   1   1   1   1   1   1   1
  //
  // with arbitrary comment lines (starting with #)
  //
  string listsFileName = argv[optind++];
  ifstream listsFile(listsFileName.c_str());
  string line;
  vector<vector<int> > fourLists(4);
  unsigned int lineNo = 0;
  int l = -1;
  vector<QueryResult> postingLists;
  vector<Separator> separators;
  while (getline(listsFile, line))
  {
    lineNo++;
    // remove trailing whitespace
    while (line.length() > 0 && isspace(line[line.length()-1])) 
      line.resize(line.length()-1);
    if (line.length() == 0 || line[0] == '#') continue;
    l++;
    // Read separator
    if (l % 9 == 0)
    {
      // get output mode 
      Separator::OutputMode outputMode;
      ASSERT(line.length() > 0);
      char c = toupper(line[line.length()-1]);
        //cout << lineNo << "." << line << endl;
      switch (c)
      {
	case 'M' : outputMode = Separator::OUTPUT_MATCHES; break;
	case 'N' : outputMode = Separator::OUTPUT_NON_MATCHES; break;
	case 'A' : outputMode = Separator::OUTPUT_ALL; break;
        default  : cout << "Invalid output mode ..." << endl; continue;
      }
      // remove output mode char and trailing whitespace
      line.resize(line.length()-1);
      while (line.length() > 0 && isspace(line[line.length()-1])) 
        line.resize(line.length()-1);
      // get separator
      Query query(line);
      Query part1;
      Query part2;
      Separator separator;
      query.splitAtLastSeparator(part1, part2, separator);
      separator.setOutputMode(outputMode);
      separators.push_back(separator);
      continue;
    }

    // Read a list
    unsigned int ll = ((l % 9) - 1) % 4;
    ASSERT(ll < 4);
    fourLists[ll].clear();
    istringstream is(line);
    int x;
    while (is.eof() == false)
    {
      is >> x;
      fourLists[ll].push_back(x);
    }

    // Collected four lists -> push a new QueryResult (if lists are of equal lengths)
    if ((l % 9) == 4 || (l % 9) == 8)
    {
      unsigned int l0 = fourLists[0].size();
      unsigned int l1 = fourLists[1].size();
      unsigned int l2 = fourLists[2].size();
      unsigned int l3 = fourLists[3].size();
      if (l0 != l1 || l0 != l2 || l0 != l3)
      {
        cout << "Four lists up to line no " << lineNo
             << " are not of same length ("
             << l0 << ", " << l1 << ", " << l2 << ", " << l3 << ")"
             << ", skipping all four lists ..." << endl;
        continue;
      }
      QueryResult postingList;
      postingList._docIds.resize(l0);
      postingList._wordIds.resize(l1);
      postingList._positions.resize(l2);
      postingList._scores.resize(l3);
      memcpy(&postingList._docIds[0],    &fourLists[0][0], l0 * sizeof(int));
      memcpy(&postingList._wordIds[0], &fourLists[1][0], l1 * sizeof(int));
      memcpy(&postingList._positions[0],          &fourLists[2][0], l2 * sizeof(int));
      memcpy(&postingList._scores[0],             &fourLists[3][0], l3 * sizeof(int));
      postingLists.push_back(postingList);
    }
  }

  cout << "Found " << postingLists.size() / 2 << " list pair(s) ..." << endl << endl;
  ASSERT(separators.size() >= postingLists.size() / 2);
  for (unsigned int i = 0; i < postingLists.size(); i += 2)
  {
    cout << "\033[1m***** Intersecting lists " << i+1 << " and " << i+2 << "\033[21m" << endl;
    cout << endl;
    Separator separator = separators[i/2];
    cout << "Separator is : " << separator.infoString() << endl;
    cout << endl;
    QueryResult& list1 = postingLists[i];
    QueryResult& list2 = postingLists[i+1];
    QueryResult  list3;
    list1.show();
    list2.show();
    completer.intersectTwoPostingListsNew(list1, list2, list3, separator);
    list3.show();
  }
}



//! TEST 3
/*!
 *   Create index and process 3-word queries. Some old thing for testing the
 *   internal intersect routine.
 */
void test3(int argc, char** argv)
{
  if (argc <= 2) exit(1);

  string baseName = argv[1];
  string indexFileName = baseName + ".hybrid";
  string vocabularyFileName = baseName + ".vocabulary";
  //string excerptsDBFileName = baseName + ".docs.db";
  string queriesFileName  = argv[2];
  Timer timer;

  // initialize index
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HYBIndex index(indexFileName, vocabularyFileName, MODE);
  History history;
  HybCompleter<MODE> completer(index, history);
  completer._metaInfo.show();
  cout << endl;

  // process queries
  ifstream queriesFile(queriesFileName.c_str());  
  string query;
  unsigned int queryCount = 0;
  queriesFile >> noskipws;
  while (getline(queriesFile, query))
  {
    istringstream is(query);
    string word1, word2, word3;
    is >> word1 >> word2 >> word3;
    ++queryCount;
    cout << queryCount << ". " << "\"" << word1 << "\" \"" << word2 << "\" \"" << word3 << "\"" << endl;
    QueryResult* list1 = NULL;
    QueryResult* list2 = NULL;
    QueryResult* list3 = NULL;
    completer.processQuery(word1, list1);
    if (showResults) list1->show();
    completer.processQuery(word2, list2);
    if (showResults) list2->show();
    completer.processQuery(word3, list3);
    if (showResults) list3->show();
    QueryResult result12, result123;
    SumProxAggregation sumProxAggregation;
    WordRange wordRange(-1,0);
    Score score;
    try {
      // BEWARE: if sixth argument is a pointer, copy constructor QueryResult(bool) will be called!
      completer.intersect(SameDoc,
                          doNotCheckWordIds,
                          andIntersection,
                          sumProxAggregation,
                          score,
                          *list1,
                          list2->_docIds,
                          list2->_positions,
                          list2->_scores,
                          list2->_wordIds,
                          result12,
                          wordRange);
      completer.topMatchesFromAllMatches(result12,
                                         WITHIN_DOC_AGGREGATION, 
                                         BETWEEN_DOCS_AGGREGATION,
                                         WITHIN_COMPLETION_AGGREGATION, 
                                         BETWEEN_COMPLETIONS_AGGREGATION,
                                         10,
                                         10);
    }
    catch (Exception e) {
      cout << "! ERROR in intersect 12: " << e.getFullErrorMessage() << endl;
    }
    result12._status = IS_FINISHED;
    if (showResults) result12.show();
    try {
      completer.intersect(SameDoc,
			    		            doNotCheckWordIds,
			    		            andIntersection,
			    		            sumProxAggregation,
			    		            score,
			    		            result12,
			    		            list3->_docIds,
			    		            list3->_positions,
			    		            list3->_scores,
			    		            list3->_wordIds,
			    		            result123,
			    		            wordRange);
      completer.topMatchesFromAllMatches(result123,
                                         WITHIN_DOC_AGGREGATION, 
                                         BETWEEN_DOCS_AGGREGATION,
                                         WITHIN_COMPLETION_AGGREGATION, 
                                         BETWEEN_COMPLETIONS_AGGREGATION,
                                         10,
                                         10);
    }
    catch (Exception e) {
      cout << "! ERROR in intersect 123: " << e.getFullErrorMessage() << endl;
    }
    result123._status = IS_FINISHED;
    if (showResults) result123.show();
  }
  cout << endl;
  history.show();
} 


