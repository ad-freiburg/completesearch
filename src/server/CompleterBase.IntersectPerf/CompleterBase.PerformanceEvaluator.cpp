// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#include <string>
#include <vector>
#include <utility>
#include "./CompleterBase.PerformanceEvaluator.h"


extern int logVerbosity;
namespace IntersectPerformanceEvaluation
{
// make random seed for random lists...
unsigned randomSeed;

  // Performance Evaluator Implementation
  // ___________________________________________________________________________
  // ___________________________________________________________________________
  // ___________________________________________________________________________

  // ___________________________________________________________________________
  // Constructor
  PerformanceEvaluator::PerformanceEvaluator(const string& indexTarget,
                                             const string& fileBaseName,
                                             const string& queriesBaseName,
                                             const string& queriesFileTarget,
                                             const string& statTarget,
                                             int verbosity,
                                             int loop)
  {
    this->_verbosity = verbosity;
    this->_loop = loop;
    cout << endl << endl << "\033[1mCREATING COMPLETER:\033[0m" << endl << endl;
    this->_fileBaseName = fileBaseName;
    this->_indexTarget = indexTarget;
    this->_queriesBaseName = queriesBaseName;
    this->_queriesFileTarget = queriesFileTarget;
    this->_statTarget = statTarget;
    string indexFileName = _indexTarget + _fileBaseName + ".hybrid";
    string vocabularyFileName = _indexTarget + _fileBaseName + ".vocabulary";
    const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;

    // Create completer object from these files.
    HYBIndex *index = new HYBIndex(indexFileName, vocabularyFileName, MODE);
    History *history = new History();
    FuzzySearch::FuzzySearcherUtf8 *fuzzySearcher =
                                           new FuzzySearch::FuzzySearcherUtf8();
    _completer = new HybCompleter<MODE>(*index, *history, *fuzzySearcher);
    _completer->_metaInfo.show();

    // set global verbosity of log to zero, see ConcurrentLog.h,
    // this is to suppress completer object output
    if (this->_verbosity <= 1)
      logVerbosity = LogVerbosity::ZERO;

    cout << endl << endl << "\033[1mMAIN TESTING:\033[0m" << endl << endl;
  }


    // //using IntersectPerformanceEvaluation::PerformanceEvaluator;
  // ___________________________________________________________________________
  // Print Usage
  void PerformanceEvaluator::printUsage()
  {
    cout << endl
         << "USAGE : ./CompleterBase.IntersectPerf [options]"
         << endl
         << endl
         << "Options are:"
         << endl
         << endl
         << " --(i)ndex-path:               Pathe for index files (*.hybrid, "
            "*.vocabulary) (FORMATE: directory + BaseName) REQUIRED"
         << endl
         << endl
         << " --(q)ueries-path:             Pathe for queries file"
         << " (FORMATE: directory + BaseName) (default = ./index-Base-Name)."
         << endl
         << endl
         << " --(s)tatistic-directory:      Directory for statistics files"
         << " (*.perf-statistics, *.perf-statistics-summary) "
         << " (default: queries-directory)."
         << endl
         << endl
         << " --(v)erbosity:                Level of verbosity"
         << " (1 for progress output only, 2 for detailed output)"
         << " (default: 1)."
         << endl
         << endl
         << " --(r)epeat:                   Level of repeating."
         << " How often each query will be tested"
         << " (default: 1)."
         << endl
         << endl;
  }


  // ___________________________________________________________________________
  // Count number of lines of given File
  size_t PerformanceEvaluator::getFilesLineNumber(const string &fileName)
  {
    const int LINE_SIZE = 1000;
    char buf[LINE_SIZE];
    size_t counter = 0;
    FILE* file = fopen(fileName.c_str(), "r");
    if (file == NULL)
    {
      cout << "Can not read File: " << fileName << endl;
      exit(1);
    }
    while (fgets(buf, LINE_SIZE, file) != NULL)
    {
      if (strlen(buf) > 2)
        counter++;
    }
    fclose(file);
    return counter;
  }

  // ___________________________________________________________________________
  // Parse the strings from artificial words to the number
  // Exp "100T" -> 100000, "10M" -> 10000000 "1" -> 100
  size_t PerformanceEvaluator::stringToInt(const string &str)
  {
    int n = str.length()-1;
    switch (str.c_str()[n])
    {
      case 'M' : return 1000000 * atoi(str.substr(0, n).c_str());
      case 'T' : return 1000 * atoi(str.substr(0, n).c_str());
      default  : return atoi(str.c_str());
    }
  }

  // ___________________________________________________________________________
  // Create queryList for given query word.
  // if artificial word, then create random list, else use loaded database
  void PerformanceEvaluator::getListForQuery(const string& queryStr,
                                             QueryResult*& result)
  {
    queryTypes queryType = getQueryType(queryStr);
    if (queryType != REALWORDS)
    {
      // Parse artificial word
      size_t postings = 0;
      size_t docIds = 0;
      size_t found;
      double docGap;
      found = queryStr.rfind("D");
      if (found == string::npos)
      {
        cout << queryStr << " is not artificial word !!!";
        return;
      }
      string str1 = queryStr.substr(2, found - 2);
      string str2 = queryStr.substr(found + 1);
      docIds = stringToInt(str2);
      if (queryType == RANDOMLISTS)
        postings = stringToInt(str1);
      else
      {
        docGap = atof(str1.c_str());
        postings = static_cast<size_t>(docIds / docGap);
      }
      // cout << "postings " << postings << endl;
      // cout << "docIds " << docIds << endl;
      // cout << "docGap " << docGap << endl;

      if (this->_verbosity == 2)
      {
        if (queryType == RANDOMLISTS)
          cout << "Randomized queryList for  query " << queryStr
               << " will be created..." << flush;
        else
          cout << "Pseudo-randomized queryList for  query " << queryStr
               << " will be created..." << flush;
      }

      // Create docs list
      DocList docList;
      WordList wordList;
      Vector <Score> scoreList;
      Vector <Position> positionList;
      for (size_t i = 1; i <= postings; i++)
      {
        DocId docId;
        WordId wordId;
        Score score;
        Position position;
        if (queryType == RANDOMLISTS)
          docId = rand_r(&randomSeed) % docIds + 1;
        else
        {
          docId = static_cast<size_t>(docGap * i);
        }

        wordId = i;
        score = 1;
        position = 0;
        docList.push_back(docId);
        wordList.push_back(wordId);
        scoreList.push_back(score);
        positionList.push_back(position);
      }
      // //TODO(Robin): is this a memory leak problem?
      result = new QueryResult();
      result->setMatchingWordsWithDups(wordList);
      result->setMatchingDocsWithDups(docList);
      result->setScorelist(scoreList);
      result->setPositionlist(positionList);

      result->sortLists();
      // print status if verbosity is high
      if (this->_verbosity == 2)
        cout << " done." << endl;
    }
    else
    {
      // print status if verbosity is high
      if (this->_verbosity == 2)
      {
        cout << "Real word queryList for  query " << queryStr
             << " will be created..." << flush;
      }
      Query query = queryStr;
      _completer->processQuery(query, result);

      // print status if verbosity is high
      if (this->_verbosity == 2)
        cout << " done." << endl;
    }
  }

  // ___________________________________________________________________________
  void PerformanceEvaluator::getAllQueries(vector<vector<string> >* queries)
  {
    const int LINE_SIZE = 1000;
    char buf[LINE_SIZE];
    char *pt, *p;

    string queryFileName = this->_queriesFileTarget
                         + this->_queriesBaseName + ".perf-queries";
    // Open query File
    FILE* queryFile = fopen(queryFileName.c_str(), "r");

    if (queryFile == NULL)
    {
      cout << endl
           << "Can not open file with the queries: " << queryFileName
           << endl;
      exit(1);
    }

    // Read line by line query file
    while ((fgets(buf, LINE_SIZE, queryFile) != NULL))
    {
      // Read word by word query line
      // queries vector stores queries of one line of the file
      // (currently always 2)

      // vector from the queries read by the file in this line...
      vector <string> queriesVector;
      string queryW;
      pt = strtok_r(buf, " \t\n", &p);
      while (pt != NULL)
      {
        queryW = (string) pt;
        pt = strtok_r(NULL, " \t\n", &p);
        queriesVector.push_back(queryW);
      }
      // HACK!!! At this time just the case with 2 queries pro line
      if (queriesVector.size() >= 2)
      {
        queries->push_back(queriesVector);
      }
    }
  }
  // ___________________________________________________________________________
  // Read line by line query file and process them, then create lists from the
  // readed queries and intersect the lists
  void PerformanceEvaluator::processIntersectQueries
                             (PerformanceEvaluator::AlgorithmId algorithmId)
  {
    const int LINE_SIZE = 1000;
    char buf[LINE_SIZE];

    char *pt, *p;
    // start randomizing
    srand(time(NULL));
    // the statistics pool for recording times and writing output statistics
    StatisticsPool statPool;

    // vector from the queries read by the file
    vector <string> queriesVector;
    vector <QueryResult> queryLists;

    string queryFileName = this->_queriesFileTarget
                         + this->_queriesBaseName + ".perf-queries";
    // Open query File
    FILE* queryFile = fopen(queryFileName.c_str(), "r");

    if (queryFile == NULL)
    {
      cout << endl
           << "Can not open file with the queries: " << queryFileName
           << endl;
      exit(1);
    }
    size_t numberOfQueries = getFilesLineNumber(queryFileName);
    // variable for counting the iterations, so they can be printed..
    size_t queryNr = 1;
    // make newline for main testing text, save cursor position at start of line
    printf("\n\033[s");
    // Read line by line query file
    while ((fgets(buf, LINE_SIZE, queryFile) != NULL))
    {
      // Read word by word query line
      // queries vector stores queries of one line of the file
      // (currently always 2)
      queriesVector.clear();

      // delete the lists
      /* NO EFFECT(ROBINS)
      for (size_t i = 0; i < queryLists.size(); i++)
      {
        queryLists[i].clear();
        queryLists[i].freeExtraSpace();
      }
       * * */
      queryLists.clear();

      string queryW;
      pt = strtok_r(buf, " \t\n", &p);

      while (pt != NULL)
      {
        queryW = (string) pt;
        pt = strtok_r(NULL, " \t\n", &p);
        queriesVector.push_back(queryW);
      }
      // Create lists from the read queries
      for (unsigned i = 0; i < queriesVector.size(); i++)
      {
        QueryResult *qList = NULL;
        getListForQuery(queriesVector[i], qList);
        if (qList != NULL)
          queryLists.push_back(*qList);
      }

      // HACK!!! At this time just the case with 2 queries pro line
      if (queryLists.size() >= 2)
      {
        statPool.startMeasurement();
        // Remember attributes of this measurement for statistics
        statPool.setAttributeForThisMeasurement("Word 1", queriesVector[0]);
        statPool.setAttributeForThisMeasurement("Word 2", queriesVector[1]);
        statPool.setNrAttributeForThisMeasurement("Listsize1",
                                       queryLists[0]._docIds.size() / 1000000.0,
                                                  2);
        statPool.setNrAttributeForThisMeasurement("Listsize2",
                                       queryLists[1]._docIds.size() / 1000000.0,
                                                  2);
        // set dummy value so that result appears after list size
        statPool.setAttributeForThisMeasurement("Result",
                                              "unknown");

        // Calculate gap Avarage by list intersektion
        double gapAvarage1 = 0;
        double gapAvarage2 = 0;
        double gapAvarage3 = 0;
        calculateIntersectGaps(queryLists[0], queryLists[1],
                               gapAvarage1, gapAvarage2, gapAvarage3);
        statPool.setNrAttributeForThisMeasurement("Gap1", gapAvarage1, 1);
        statPool.setNrAttributeForThisMeasurement("Gap2", gapAvarage2, 1);
        statPool.setNrAttributeForThisMeasurement("Gap3", gapAvarage3, 1);
        // print progress info if verbosity level is low
        // (else all output us in processSingleIntersectQuery)

        int progressBarWidth = 30;
        for (int loop = 0; loop < _loop; loop++)
          {
            // start next run
            statPool.startRun();

            if (this->_verbosity == 1)
            {
              // show which list is being intersected, always erase line
              // and jump back to saved cursor position
              // (see above before outer loop) at start of line
              // also jump up one line due to new line for progress bar!
              printf("\033[u\033[1A\033[2KIntersecting List %zu/%zu "
                      "(loop %d/%d) ...",
                                              queryNr,
                                              numberOfQueries,
                                              loop + 1,
                                              _loop);
              // draw first progressbar
              if (loop == 0 && queryNr == 1)
              {
                printf("\n");
                displayProgressBar(0, progressBarWidth);
                // go up one line so progressbar is overwritten again..
                printf("\033[1A");
              }
              fflush(stdout);
            }

            processSingleIntersectQuery(queryLists[0],
                    queryLists[1],
                    algorithmId,
                    &statPool);

            // print progress
            if (this->_verbosity == 1)
            {
              printf("\n");
                  // the progress in percent ie 0.1 for 10% progress
              // queryNr -1 because queryNr starts with 1, loop + 1 because loop
              // starts with 0..... this way it makes sense and its 1 at the end
              // :)
              float progressPerCent = (static_cast<float>(
                                      (((queryNr - 1) * _loop) + loop + 1)))
                                      /
                                      (static_cast<float>(
                                        (numberOfQueries * _loop)));
              displayProgressBar(progressPerCent, progressBarWidth);
            }
          }
        queryNr++;
      }
    }
    // Close query File
    fclose(queryFile);

    // print newline before statistical summary if verbosity is not high...
    if (this->_verbosity == 1)
      printf("\n\n");

    // write statistics to File...
    // //TEST, to be removed by proper instance variables!!
    string baseFileName = this->_statTarget + this->_queriesBaseName;

    statPool.writeStatisticsToFile(baseFileName + ".perf-statistics");
  }
  // ___________________________________________________________________________
  // Calculate gap Avarage by list intersektion
  // simulation naitives algorithms of completesearch
  void PerformanceEvaluator::calculateIntersectGaps(const QueryResult& list1,
                                                    const QueryResult& list2,
                                                    double& gapSum1,
                                                    double& gapSum2,
                                                    double& gapSum3)
  {
    const DocList& docIds1 = list1._docIds;
    const DocList& docIds2 = list2._docIds;
    unsigned int len1 = docIds1.size();
    unsigned int len2 = docIds2.size();
    unsigned int i = 0;
    unsigned int j = 0;
    size_t gap = 0;
    size_t gapCounter1 = 0;
    size_t gapCounter2 = 0;
    size_t gapCounter3 = 0;
    while (true)
    {
      if (j == len2) break;
      gap = 0;
      while (i < len1 && docIds1[i] < docIds2[j])
      {
        gap++;
        ++i;
      }
      gapSum1 += gap;
      if (gap > 0)
        gapCounter1++;
      gap = 0;
      if (i == len1) break;
      while (j < len2 && docIds2[j] < docIds1[i])
      {
        gap++;
        ++j;
      }
      gapSum2 += gap;
      if (gap > 0)
        gapCounter2++;
      gap = 0;
      while (j < len2 && docIds2[j] == docIds1[i])
      {
        gap++;
        ++j;
      }
      gapSum3 += gap;
      if (gap > 0)
        gapCounter3++;
      if (j == len2) break;
    }
    // //return static_cast<double>(gapSum) / static_cast<double>(gapCounter);
    if (gapCounter1 > 0)
      gapSum1 = gapSum1 / static_cast<double>(gapCounter1);
    if (gapCounter2 > 0)
      gapSum2 = gapSum2 / static_cast<double>(gapCounter2);
    if (gapCounter3 > 0)
      gapSum3 = gapSum3 / static_cast<double>(gapCounter3);
  }

// _____________________________________________________________________________
  void PerformanceEvaluator::processSingleIntersectQuery
                                 (const QueryResult& list1,
                                  const QueryResult& list2,
                                  PerformanceEvaluator::AlgorithmId algorithmId,
                                  StatisticsPool* statPool)
  {
    // give status info if verbosity level is high
    if (this->_verbosity == 2)
      printf("\n%c[1;34mIntersecting Lists...%c[0m\n", 0x1B, 0x1B);
    // Now intersect the two lists / blocks.
    Separator separator(" ", pair<signed int, signed int>(-1, -1), 0);
    QueryResult list3;

    // start the measurement, record time needed by intersection procedure...
    Timer timer;
    timer.start();
    patternAlgorithmIntersect(list1, list2, list3);
    timer.stop();

    // remember time and result list size for statistics
    off_t uSecTime = timer.usecs();
    double mSecTime = uSecTime / 1000.0;
    statPool->addTime("simple", mSecTime);
    statPool->setNrAttributeForThisMeasurement("Result",
                                              list3._docIds.size() / 1000000.0,
                                              2);

    QueryResult hackList;
    Timer hackTimer;
    hackTimer.start();
    patternAlgorithmIntersectMod2(list1, list2, hackList);
    hackTimer.stop();

    off_t hackUSecTime = hackTimer.usecs();
    double hackMSecTime = hackUSecTime / 1000.0;
    statPool->addTime("mod2", hackMSecTime);

    // delete old query result data
    /* NO EFFECT(ROBINS)
    list3.clear();
    list3.freeExtraSpace();
     * */
    // if verbosity has level 2 print out detailed information
    if (this->_verbosity == 2)
    {
      // print out the list with query names and sizes
      // use streams, so that whole string (e.g. "List 1 (Rob* and Sim*):")
      // can be aligned properly by printf
      stringstream leftStream;
      // TODO(Robin): reput words by adding statpool method
      leftStream << " List 1 (" << "TODOword1" << "): ";

      // print out leftStream with width 45, adding spaces on the right!
      printf("%-45s %11zu elements\n", leftStream.str().c_str(),
                                       list1._docIds.size());

      leftStream.str("");
      leftStream << " List 2 (" << "TODOword2" << "): ";
      printf("%-45s %11zu elements\n", leftStream.str().c_str(),
                                       list2._docIds.size());

      leftStream.str("");
      leftStream << " Resultlist (" << "TODOword1"<< " and " <<
               "TODOword2" << "): ";
      printf("%-45s %11zu elements\n", leftStream.str().c_str(),
                                       list3._docIds.size());

      // print intersection time
      string leftString = "Needed Time:";
      printf("%c[2m%-45s %11.1f ms%c[0m\n", 0x1B,
            leftString.c_str(), mSecTime, 0x1B);

      printf("\n");
    }
  }

  // //________________________________GETTER METHODS___________________________

    string PerformanceEvaluator::getStatBaseName()
    {
      // return copy of the string
      return string(this->_statTarget + this->_queriesBaseName);
    }

    int PerformanceEvaluator::getRepetitions()
    {
      return this->_loop;
    }
    int PerformanceEvaluator::getVerbosity()
    {
      return this->_verbosity;
    }

  // //____________________________END GETTER METHODS___________________________
}
