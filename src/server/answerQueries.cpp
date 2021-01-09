#include <iostream>
#include <getopt.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "CompleterBase.h"
#include "HYBCompleter.h"
#include "INVCompleter.h"

#include "../fuzzysearch/FuzzySearcher.h"
#include "../fuzzysearch/WordClusteringBuilder.h"

using namespace std;
using FuzzySearch::WordClusteringBuilder;

int atoi_ext(string s);

void printUsage()
{
  cout
      << "Usage: answerQueries [INV|HYB] index-file [-q query_file] [queries] [all kinds of options]"
      << endl << endl << "-q query_file [queries]" << endl
      << "     Read queries from file (each line a query, trailing whitespace ignored)"
      << endl
      << "     if not specified, all non-option arguments following index-file are taken as queries"
      << endl << endl << "-p min_prefix_size" << endl
      << "     For each query, also consider all of its prefixes with length of at least min_prefix_size"
      << endl
      << "     With argument 0, all whole-word queries are added. By default, no queries are added at all."
      << endl << endl << "-H" << endl
      << "     Clear history before executing a query (interesting for timing repeated execution of same query)."
      << endl
      << "     Will keep history for each run though (run = sequence of queries, where each is a prefix of the next)."
      << endl
      << "     NEW HACK 27May06: clear when query is shorter than predecessor (needed it for FF:* queries)."
      << endl << endl << "-r nof_repeats" << endl
      << "     Repeat each query for the specified number of times." << endl
      << endl << "-O" << endl
      << "     Do not show statistics for each individual query, but only the summary line."
      << endl << endl << "-L" << endl
      << "     Index was built without location information." << endl << endl
      << "-S" << endl << "     Index was built without score information."
      << endl << endl << "-F" << endl
      << "     Do not filter results from prefix in the history (filtering is done by default)."
      << endl << endl << "-C" << endl << "     Show completions for each query"
      << endl << endl << "-n nof_query_words" << endl
      << "     Summary only for queries with the specified number of query words. A number means:"
      << endl
      << "     exactly this number. A number followed by a plus (e.g. 3+), means at least this many."
      << endl << endl << "-d prefix_length" << endl
      << "     Summary only for queries with specified prefix length. Format as for -n options, e.g."
      << endl << "     can specify something like 3+" << endl << endl
      << "-v volume" << endl
      << "     Summary only for queries with specified volume. Format as for -n and -d options."
      << endl << endl << "-s statistics_file" << endl
      << "     Write statistics to file with this name; default name is \"STATISTICS\""
      << endl << endl << "-y yago_result_file" << endl
      << "     Write result completions to file with this name (for SIGIR'07 paper, default is not to do it at all)"
      << endl << endl << "-Y" << endl << "     Enable fuzzysearch" << endl
      << "-E encoding" << endl << endl << "-f" << endl
      << "     Show statistics for fuzzysearch" << endl << endl << "-e" << endl
      << "     Exclude time for reading, compressing and scoring (for comparison with the WWW fuzzysearch paper)"
      << endl << "-h <historySizeInByte>" << endl
      << "     Specify a MAX_HISTORY_SIZE for the CompletionServer" << endl
      << "Performs an additional first round which is ignored when building the statistics. "
      << endl << endl;
}

//
// GLOBAL VARIABLES (all set in main, many used in answerQueries)
//
const char WARMUP_QUERY_PREFIX = '&';
string method;
vector<string> queries;
string indexFileName;
string vocFileName;
string statisticsFileName = "STATISTICS";
string yagoResultFileName = "";
char* query_file = NULL;
int min_prefix_length = -1; /* 0 = add whole-word queries; -1 = add nothing at all */
unsigned int nof_query_repeats = 1;
bool showStatisticsPerQuery = true;
bool clearHistory = false;
bool filterResults = true;
bool useLocations = true;
bool useScores = true;
bool showCompletions = false;
string prefixLengthRange = "0+"; /* for summary, consider only queries with this prefix length */
string querySizeRange = "0+"; /* for summary, consider only queries with this many words */
string volumeRange = "0+"; /* for summary, consider only queries with this many words */
extern bool fuzzySearchEnabled;
extern string baseName;
bool showFuzzySearchStats = false;
bool excludeTimesWWWPaper = false; // for excluding disk, compression and scoring time

FuzzySearch::FuzzySearcherBase* fuzzySearcher;
extern bool fuzzySearchEnabled;

bool isInRange(int x, string range);

//
// ANSWER A SET OF QUERIES (queries, index files, etc. from global variables)
//
template<class Completer, class Index>
void answerQueries()
{
  //TimedHistory emptyHistory;
  Index index(indexFileName, vocFileName, Completer::mode());
  index.read();

  Completer completer(&index, &emptyHistory, &nullFuzzySearcher);
  QueryResult *result = NULL;
  ////// result = new QueryResult();


  cout << "* read meta information" << endl << endl;
  completer.showMetaInfo();
  cout << endl;

  cout << "* writing statistics to file \"" << statisticsFileName << "\""
      << endl;
  FILE* statistics_file = fopen(statisticsFileName.c_str(), "w");
  if (statistics_file == NULL)
  {
    cerr << endl << "! ERROR opening statistics file \"" << statisticsFileName
        << "\" (" << strerror(errno) << ")" << endl << endl;
    exit(1);
  }

  FILE* yago_result_file = NULL;
  ostringstream yago_result_stream_buffer; // pass this to the query Result method, that write the result to it
  if (yagoResultFileName != "")
  {
    cout << "* writing result completions to file \"" << yagoResultFileName
        << "\"" << endl;
    yago_result_file = fopen(yagoResultFileName.c_str(), "w");
    if (yago_result_file == NULL)
    {
      cerr << endl << "! ERROR opening yago result file \""
          << yagoResultFileName << "\" (" << strerror(errno) << ")" << endl
          << endl;
      exit(1);
    }
  }

  ostringstream report;
  cout << "* processing " << numberAndNoun(queries.size(), "query", "queries");
  if (nof_query_repeats > 1)
    cout << " (repeating each query " << nof_query_repeats << " times)";
  cout << endl << endl;

  // COUNTERS FOR GLOBAL STATISTICS (initialize times to 1 usec to avoid division by zero)
  unsigned int nofQueries = 0; /* queries executed */
  unsigned int nofQueriesWithStatistics = 0; /* queris considered in summary (-n and -d option) */
  off_t maxCompletionTime = 1;
  off_t totalCompletionTime = 1;
  //off_t totalGetDataVolume = 0;
  //off_t totalGetDataTime = 1;
  off_t totalReadVolume = 0;
  off_t totalReadTime = 1;
  off_t totalResizeAndReserveTime = 1;
  off_t totalDoclistDecompressionVolume = 0;
  off_t totalWordlistDecompressionVolume = 0;
  off_t totalPositionlistDecompressionVolume = 0;
  off_t totalDoclistDecompressionTime = 1;
  off_t totalWordlistDecompressionTime = 1;
  off_t totalPositionlistDecompressionTime = 1;
  off_t totalTimerTime = 1;
  off_t totalIntersectVolume = 0;
  off_t totalIntersectTime = 1;
  off_t totalIntersectWordlistsTime = 1;
  off_t totalIntersectWordlistsTime1 = 1;
  off_t totalIntersectWordlistsTime2 = 1;
  off_t totalIntersectWordlistsTime3 = 1;
  off_t totalIntersectWordlistsTime4 = 1;
  off_t totalIntersectWordlistsTime5 = 1;
  off_t totalSortVolume = 0;
  off_t totalInvMergeTime = 0;
  off_t totalSortTime = 1;// includes time for scoring/duplicate removal
  off_t totalScoreDocsTime = 1;
  off_t totalScoreDocsTime1 = 1;
  off_t totalScoreDocsTime2 = 1;
  off_t totalScoreDocsTime3 = 1;
  off_t totalScoreDocsTime4 = 1;
  off_t totalBroadHistoryTime = 1;
  off_t totalBroadHistoryTime0 = 1;
  off_t totalBroadHistoryTime1 = 1;
  off_t totalBroadHistoryTime2 = 1;
  off_t totalBroadHistoryTime3 = 1;
  off_t totalBroadHistoryTime4 = 1;
  off_t totalBroadHistoryTime5 = 1;
  off_t totalBroadHistoryTime6 = 1;
  off_t totalBroadHistoryTime7 = 1;
  off_t totalSetCompletionsTime = 1;
  off_t totalSetCompletionsTime1 = 1;
  off_t totalSetCompletionsTime2 = 1;
  //off_t totalprefixToRangeTime = 1;
  //off_t totalReserveTime = 1;
  //off_t totalInLoopTime = 1;
  off_t totalAppendTime = 1;
  off_t totalScoreWordsTime = 1;
  //off_t totalWriteBufferTime = 1;
  off_t totalHistoryTime = 1;

  off_t totalFuzzySearchDistanceComputationsTime = 0;
  off_t totalFuzzySearchProcessQueryTime = 0;
  off_t totalfuzzySearchTotalTime = 0;

  double totalFuzzySearchRecall = 0;
  double totalFuzzySearchPrecision = 0;
  off_t totalFuzzySearchClusters = 0;
  off_t totalNumOfSimilarWords = 0;

  //ConcurrentLog log; (using completer.log)
  Query query;
  QueryParameters queryParameters;
  queryParameters.nofTopHitsToCompute = UINT_MAX;
  queryParameters.nofTopCompletionsToCompute = UINT_MAX;
  queryParameters.useFiltering = filterResults;
  queryParameters.howToJoin = QueryParameters::HASH_JOIN;
  completer.setQueryParameters(queryParameters);

  // PROCESS ONE QUERY AFTER THE OTHER
  for (unsigned int i = 0; i < queries.size(); ++i)
  {
    // NEW 25th Oct. 10(Björn):
    // Introduced cache-warming queries that are indicated by
    // a special symbol (WARMUP_QUERY_PREFIX) in the beginning of
    // a query. Those queries are not considered in the statistics
    // hence, we continue the loop after execution and before
    // measurements are taken.
    bool isWarmupQuery = false;
    string currentQuery = queries[i];
    // Strip the prefix
    if (currentQuery[0] == WARMUP_QUERY_PREFIX)
    {
      isWarmupQuery = true;
      queries[i] = currentQuery.substr(1, currentQuery.size() - 1);
    }

    unsigned int repeat;
    for (repeat = 1; repeat < nof_query_repeats; ++repeat)
    {
      query.setQueryString(queries[i]);
      result = NULL;
      try
      {
        completer.resetTimersAndCounters();
        completer.processQuery(query, result);
        cout << "XXXX: " << result->_docIds.size() << endl;
        completer.history->cleanUp();
      } catch (Exception e)
      {
        cout << "! " << e.getFullErrorMessage() << endl << endl;
        continue;
      }
      assert(result);
      completer.removeFromHistory(query);
      cout << "! repeated query " << endl;
    }

    // CLEAR HISTORY if -H option given and unless previous query was a prefix
    bool previousQueryIsShorter = i > 0 && repeat == 1
        && queries[i - 1].length() < queries[i].length();
    /* bool previousQueryIsPrefix = previousQueryIsShorter
     && queries[i].substr(0,queries[i-1].length()-1)
     == queries[i-1].substr(0,queries[i-1].length()-1); // take into acocunt "*" at end */
    if (previousQueryIsShorter == false)
    {
      if (clearHistory == true)
      {
        completer.history->cutToSizeAndNumber(1, UINT_MAX);
        // completer.history.clear();
        cout << "! cleared history" << endl << endl;
      }
      else
      {
        bool wasHistoryCutDown = completer.history->cutToSizeAndNumber(
            historyMaxSizeInBytes, historyMaxNofQueries);
        if (wasHistoryCutDown == true)
          cout << "! cut down history to at most " << historyMaxSizeInBytes
              / (1024 * 1024) << " MB" << " / " << historyMaxNofQueries
              << " queries";
      }
    }

    // TODO: Was soll denn die Schei�e, dasselbe nochmal?? Tods�nde, b�se, niemals, nein, aua, ...

    query = queries[i];
    result = NULL;
    try
    {
      completer.resetTimersAndCounters();
      completer.processQuery(query, result);
      cout << "XXXX: " << result->_docIds.size() << endl;
      Timer timer;
      timer.start();
      completer.history->cleanUp();
      timer.stop();
      completer.log << "! *** TIME TO CLEAN UP HISTORY: " << timer << endl;
    } catch (Exception e)
    {
      cout << "! " << e.getFullErrorMessage() << endl << endl;
      continue;
    }

    // Continue the loop without taking measurements
    // for the statistics if the current query is a warmup query.
    if (isWarmupQuery)
      continue;

    unsigned int completionTime = completer.processQueryTimer.usecs();
    bool writeResultWordsNotDocs = true;
    if (yago_result_file)
    {
      yago_result_stream_buffer << "query: " << queries[i] << endl;
      if (queries[i].substr(0, 2) == "d ")
      {
        writeResultWordsNotDocs = false;
      }
      else
      {
        writeResultWordsNotDocs = true;
      }
      if (writeResultWordsNotDocs)
      {
        result->dumpResultToStream(yago_result_stream_buffer,
            (*completer._vocabulary));
        result->writeRankedResultToStream(yago_result_stream_buffer);
      }
      else
      {
        result->writeRankedResultToStream(yago_result_stream_buffer,
            writeResultWordsNotDocs);
      }
      yago_result_stream_buffer << flush;
    }
    else
    {
      //          cout << "\n apparently no yago option chosen " << endl;
    }
    nofQueries++;

    // SHOW ONE LINE SUMMARY OF RESULTS
    cout << endl;
    completer.log.setId(nofQueries);
    completer.showOneLineSummary(completer.log, query, result, completionTime);
    cout << endl;
    /*
     cout << EMPH_ON << setw(26) << left << ( string("\"") + queries[i] + string("\"") ) << EMPH_OFF
     << " " << EMPH_ON << setw(4) << right << completer.topCompletionsAndHitsTimer.msecs() << " millisecs" << EMPH_OFF
     << "; " << setw(10) << commaStr((*result)._topDocIds.size()) << " hits"
     << "; " << setw(4) << commaStr((*result)._topCompletions.size()) << " completions" << flush;
     if (completer.nofBlocksReadFromFile > 0)
     cout << "; " << numberAndNoun(completer.nofBlocksReadFromFile,"block","blocks")
     << " of volume " << commaStr(completer.doclistVolumeDecompressed/sizeof(DocId)) << " read" << endl;
     else if (result->wasInHistory)
     cout << "; [was in history]" << endl;
     else if (result->resultWasFilteredFrom != "")
     cout << "; [was filtered]" << endl;
     cout << endl;
     */
    // COMPUTE QUERY LENGTH, PREFIX LENGTH, VOLUME
    unsigned int prefixLength =
        queries[i].rfind(' ') == string::npos ? queries[i].length()
            : queries[i].length() - queries[i].rfind(' ') - 1;
    if (queries[i].operator[](prefixLength - 1) == '*')
      prefixLength--;

    unsigned int querySize = 1;
    for (unsigned int l = 1; l < queries[i].length(); ++l)
      if (queries[i][l] == ' ' && queries[i][l - 1] != ' ')
        ++querySize;
    // skip statistics if query is not in specified range
    //cout << "--> prefix length = " << prefixLength << " | "
    //     << prefixLengthRange << " : " << isInRange(prefixLength,prefixLengthRange) << endl;
    //cout << "--> query size = " << querySize << " | "
    //     << querySizeRange << " : " << isInRange(querySize,querySizeRange) << endl;
    unsigned int queryVolume = completer.doclistVolumeDecompressed
        / sizeof(DocId); /* nof word-doc/pos pairs */

    // WRITE SOME INFO TO STATISTICS FILE
    //bool doNotCountSort = (method == "HYB" && querySize == 1); // uncommented on 27 April 2006
    //bool doNotCountSort = false;
    //if (doNotCountSort) completionTime -= completer.uniteTimer.usecs();
    string q = queries[i];
    unsigned int nofQueryWords = 1;
    for (unsigned int j = 0; j < q.length(); ++j)
      if (q[j] == ' ')
      {
        q[j] = '_';
        nofQueryWords++;
      }

    /*
     fprintf(statistics_file, "%s\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n",
     q.c_str(),
     completionTime,
     nofQueryWords,
     prefixLength,
     completer.nofBlocksReadFromFile,
     queryVolume,
     (unsigned int) completer::prefixToRangeTimer.usecs(),
     (unsigned int) completer.fileReadTimer.usecs(),
     (unsigned int) completer.resizeTimer.usecs(),
     (unsigned int) GlobalTimers::reserveTimer.usecs(),
     (unsigned int) completer.doclistDecompressionTimer.usecs(),
     (unsigned int) completer.wordlistDecompressionTimer.usecs(),
     (unsigned int) completer.positionlistDecompressionTimer.usecs(),
     (unsigned int) completer.intersectionTimer.usecs(),
     (unsigned int) GlobalTimers::appendTimer.usecs(),
     (unsigned int) completer.uniteTimer.usecs(),
     (unsigned int) GlobalTimers::stlSortTimer.usecs(),
     (unsigned int) GlobalTimers::scoreDocsTimer.usecs(),
     (unsigned int) GlobalTimers::scoreWordsTimer.usecs(),
     (unsigned int) completer.writeBuffersToQueryResultTimer.usecs(),
     (unsigned int) completer.volumeReadFromFile,
     (unsigned int) completer.historyTimer.usecs());
     */

    // SELECT QUERY FOR STATISTICS?
    if (not isInRange(queryVolume, volumeRange))
      completer.removeFromHistory(queries[i]); /* clear history for high-volume queries */
    if (not isInRange(prefixLength, prefixLengthRange) || not isInRange(
        querySize, querySizeRange) || not isInRange(queryVolume, volumeRange))
      continue;
    //cout << "! ignoring query \"" << queries[i] << "\" for global statistics" << endl << endl;

#ifndef NDEBUG
    (*result).show(); // for debugging/testing
#endif

    if (showStatisticsPerQuery == true)
    {
      // DETAILED STATISTICS
      completer.showStatistics(completer.log, completionTime, ""); //"   ");
      cout << endl;

      // SHOW (SOME) COMPLETIONS
      if (showCompletions == true)
      {
        cout << "   Completions are :" << flush;
        if (result == NULL)
        {
          cout << " [result == NULL]" << endl << endl;
        }
        else
        {
          //(*result)._topCompletions.sortByScore(10);
          for (unsigned int i = 0; i < (*result)._topCompletions.size(); ++i)
          {
            if (i == 10)
            {
              cout << " ..." << flush;
              break;
            }
            cout << " " << (*result)._topCompletions[i].first << " ("
                << (*result)._topCompletions[i].second << ") " << flush;
          }
          cout << endl << endl;
        }
      }

    } //end : if (showStatisticsPerQuery == true)

    off_t timeToBeExcluded = 0;
    if (excludeTimesWWWPaper)
    {
      timeToBeExcluded = completer.fileReadTimer.usecs(); // + completer.doclistDecompressionTimer.usecs()
      // + completer.wordlistDecompressionTimer.usecs() + completer.positionlistDecompressionTimer.usecs();
      // + completer.scoreWordsTimer.usecs()
      //+ completer.scoreDocsTimer.usecs()
      // + completer.fuzzySearchAuxTimer.usecs()
      // + completer.fuzzySearchDistanceComputationsTimer.usecs();
      // + completer.historyTimer.usecs();
      // completionTime = timeToBeExcluded > completionTime ? 0 : completionTime - timeToBeExcluded;
    }
    if (fuzzySearchEnabled)
    {
      cout << ">>> Fuzzysearch Query Processing Time : "
          << completer.fuzzySearchProcessQueryTimer << endl;
      cout << ">>> Fuzzysearch Total Time            : "
          << completer.fuzzySearchTotalTimer << endl;
      cout << ">>> Fuzzysearch Cluster Time          : "
          << completer.fuzzySearchDistanceComputationsTimer << endl;
      cout << ">>> Fuzzysearch Aux Time              : "
          << completer.fuzzySearchAuxTimer << endl;
      // cout << ">>> Execution time with exclusion     : " << 1.0 * completionTime / 1000 << " milliseconds"<< endl;
      cout << endl;
    }
    // MAINTAIN GLOBAL STATISTICS
    nofQueriesWithStatistics++;
    if (completionTime > maxCompletionTime)
      maxCompletionTime = completionTime;
    totalCompletionTime += completionTime;
    //totalGetDataVolume += completer.doclistVolumeDecompressed + completer.wordlistVolumeDecompressed;
    //totalGetDataTime += completer.getDataTimer.usecs();
    if (showFuzzySearchStats)
    {
      totalFuzzySearchClusters += completer.fuzzySearchCoverIndex;
      totalNumOfSimilarWords
          += completer.fuzzySearchNumDistinctWordsInSelectedClusters;
      if (completer.fuzzySearchNumRelevantWordsSelected > 0)
      {
        totalFuzzySearchRecall += 1.0
            * completer.fuzzySearchNumRelevantWordsSelected
            / completer.fuzzySearchNumRelevantWords;
        totalFuzzySearchPrecision += 1.0
            * completer.fuzzySearchNumRelevantWordsSelected
            / completer.fuzzySearchNumDistinctWordsInSelectedClusters;
      }
      else
      {
        totalFuzzySearchRecall += 1;
        totalFuzzySearchPrecision += 1;
      }
      totalFuzzySearchDistanceComputationsTime
          += completer.fuzzySearchDistanceComputationsTimer.usecs();
      totalFuzzySearchProcessQueryTime
          += completer.fuzzySearchProcessQueryTimer.usecs();
      totalfuzzySearchTotalTime += completer.fuzzySearchTotalTimer.usecs();
      if (excludeTimesWWWPaper)
      {
        totalFuzzySearchProcessQueryTime -= timeToBeExcluded;
        totalfuzzySearchTotalTime -= timeToBeExcluded;
      }
    }
    totalReadVolume += completer.volumeReadFromFile;
    totalReadTime += completer.fileReadTimer.usecs();
    totalResizeAndReserveTime += completer.resizeAndReserveTimer.usecs();
    totalTimerTime += completer.timerTimer.usecs();
    totalBroadHistoryTime += completer.broadHistoryTimer.usecs();
    totalInvMergeTime += completer.invMergeTimer.usecs();
    totalBroadHistoryTime0 += completer.broadHistoryTimer0.usecs();
    totalBroadHistoryTime1 += completer.broadHistoryTimer1.usecs();
    totalBroadHistoryTime2 += completer.broadHistoryTimer2.usecs();
    totalBroadHistoryTime3 += completer.broadHistoryTimer3.usecs();
    totalBroadHistoryTime4 += completer.broadHistoryTimer4.usecs();
    totalBroadHistoryTime5 += completer.broadHistoryTimer5.usecs();
    totalBroadHistoryTime6 += completer.broadHistoryTimer6.usecs();
    totalBroadHistoryTime7 += completer.broadHistoryTimer7.usecs();
    totalSetCompletionsTime += completer.setCompletionsTimer.usecs();
    totalSetCompletionsTime1 += completer.setCompletionsTimer1.usecs();
    totalSetCompletionsTime2 += completer.setCompletionsTimer2.usecs();
    totalDoclistDecompressionVolume += completer.doclistVolumeDecompressed;
    totalWordlistDecompressionVolume += completer.wordlistVolumeDecompressed;
    totalPositionlistDecompressionVolume
        += completer.positionlistVolumeDecompressed;
    totalDoclistDecompressionTime
        += completer.doclistDecompressionTimer.usecs();
    totalWordlistDecompressionTime
        += completer.wordlistDecompressionTimer.usecs();
    totalPositionlistDecompressionTime
        += completer.positionlistDecompressionTimer.usecs();
    totalIntersectVolume += completer.intersectedVolume;//does currently (29Jan06) NOT count scores
    totalIntersectTime += (off_t) completer.intersectionTimer.usecs();
    totalIntersectWordlistsTime += completer.intersectWordlistsTimer.usecs();
    totalIntersectWordlistsTime1 += completer.intersectWordlistsTimer1.usecs();
    totalIntersectWordlistsTime2 += completer.intersectWordlistsTimer2.usecs();
    totalIntersectWordlistsTime3 += completer.intersectWordlistsTimer3.usecs();
    totalIntersectWordlistsTime4 += completer.intersectWordlistsTimer4.usecs();
    totalIntersectWordlistsTime5 += completer.intersectWordlistsTimer5.usecs();
    totalSortVolume += completer.doclistUniteVolume
        + completer.wordlistUniteVolume + completer.positionlistUniteVolume
        + completer.scorelistUniteVolume;
    //      if (!doNotCountSort) totalSortTime += completer.uniteTimer.usecs();
    totalSortTime += completer.stlSortTimer.usecs();
    //totalStringSearchTime += completer::stringSearchTimer.usecs();
    totalScoreDocsTime += completer.scoreDocsTimer.usecs();
    totalScoreDocsTime1 += completer.scoreDocsTimer1.usecs();
    totalScoreDocsTime2 += completer.scoreDocsTimer2.usecs();
    totalScoreDocsTime3 += completer.scoreDocsTimer3.usecs();
    totalScoreDocsTime4 += completer.scoreDocsTimer4.usecs();
    totalScoreWordsTime += completer.scoreWordsTimer.usecs();
    totalAppendTime += completer.appendTimer.usecs();
    //totalInLoopTime += GlobalTimers::inLoopTimer.usecs();
    //totalReserveTime += GlobalTimers::reserveTimer.usecs();
    //totalWriteBufferTime += completer.writeBuffersToQueryResultTimer.usecs();
    totalHistoryTime += completer.history->getTimer().usecs();
    /*
     if (excludeTimesWWWPaper)
     {
     // totalFuzzySearchProcessQueryTime -= timeToBeExcluded;
     // totalfuzzySearchTotalTime -= timeToBeExcluded;
     totalReadTime = 1;
     totalScoreDocsTime = 1;
     totalScoreWordsTime = 1;
     totalDoclistDecompressionTime = 1;
     totalPositionlistDecompressionTime = 1;
     totalWordlistDecompressionTime = 1;
     }
     */
  } // end: loop over queries
  fclose(statistics_file);

  if (yago_result_file)
  {
    fprintf(yago_result_file, "%s", yago_result_stream_buffer.str().c_str());
    fclose(yago_result_file);
  }

  //
  // SHOW SUMMARY
  //
  cout << endl;
  if (nofQueriesWithStatistics == 0)
  {
    cout << endl << "! no queries selected for detailed statistics" << endl
        << endl;
    exit(0);
  }
  cout << setiosflags(ios::fixed);
#define AF1 "   " << EMPH_OFF << setw(64) << left
#define AF2          EMPH_ON  << setw(8)  << right << setprecision(1)
#define AF3 "  "  << EMPH_ON  << setw(5)  << right << setprecision(1)
#define AF4 "  "  << EMPH_ON  << setw(4)  << right << setprecision(0)
  cout << EMPH_ON << "SUMMARY" << EMPH_OFF << flush;
  if (nofQueriesWithStatistics == nofQueries)
  {
    cout << " for all " << commaStr(nofQueriesWithStatistics) << " queries"
        << endl;
  }
  else
  {
    cout << " for " << commaStr(nofQueriesWithStatistics) << " of the "
        << commaStr(nofQueries) << " executed queries" << endl;
  }

  cout << endl << AF1 << "maximum query time" << AF2 << maxCompletionTime
      / 1000.0 << " millisecs" << endl << AF1 << "average query time" << AF2
      << totalCompletionTime / (1000.0 * nofQueries) << " millisecs" << endl
      << endl;
  //<< AF1 << "read+uncompress+copy" << AF2 << totalGetDataVolume/totalGetDataTime << " MiB/sec" << endl
  //<< AF1 << "read" << AF2 << totalReadVolume/totalReadTime << " MiB/sec" << endl
  //<< AF1 << "resize" << AF2 << totalDecompressionVolume/totalDecompressionTime << " MiB/sec" << endl
  //<< AF1 << "uncompress" << AF2 << totalDecompressionVolume/totalDecompressionTime << " MiB/sec" << endl
  //<< AF1 << "intersects" << AF2 << totalIntersectVolume/totalIntersectTime << " MiB/sec" << endl
  //<< AF1 << "sort+unique" << AF2 << totalSortVolume/totalSortTime << " MiB/sec" << endl
  //<< endl
  //<< AF1 << "other" << AF2
  //       << 100.0 - (100.0*(totalGetDataTime+totalIntersectTime+totalSortTime))/totalCompletionTime << "% of total time" << endl
  //<< EMPH_OFF << endl;

  cout << AF1 << "read" << AF2 << totalReadTime / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalReadTime) / totalCompletionTime
      << "% " << AF4 << totalReadVolume / totalReadTime << " MiB/sec"
      << EMPH_OFF << " wrt size on disk" << endl << AF1 << "resize" << AF2
      << totalResizeAndReserveTime / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalResizeAndReserveTime) / totalCompletionTime
      << "% " << AF4 << (totalDoclistDecompressionVolume
      + totalWordlistDecompressionVolume) / totalResizeAndReserveTime
      << " MiB/sec" << EMPH_OFF << " wrt size on disk" << endl << AF1
      << "uncompress docs" << AF2 << totalDoclistDecompressionTime / (1000.0
      * nofQueries) << " millisecs" << AF3 << (100.0
      * totalDoclistDecompressionTime) / totalCompletionTime << "% " << AF4
      << totalDoclistDecompressionVolume / totalDoclistDecompressionTime
      << " MiB/sec" << EMPH_OFF << " wrt size of uncompressed" << endl << AF1
      << "uncompress words" << AF2 << totalWordlistDecompressionTime / (1000.0
      * nofQueries) << " millisecs" << AF3 << (100.0
      * totalWordlistDecompressionTime) / totalCompletionTime << "% " << AF4
      << totalWordlistDecompressionVolume / totalWordlistDecompressionTime
      << " MiB/sec" << EMPH_OFF << " wrt size of uncompressed" << endl << AF1
      << "uncompress positions" << AF2 << totalPositionlistDecompressionTime
      / (1000.0 * nofQueries) << " millisecs" << AF3 << (100.0
      * totalPositionlistDecompressionTime) / totalCompletionTime << "% "
      << AF4 << totalPositionlistDecompressionVolume
      / totalPositionlistDecompressionTime << " MiB/sec" << EMPH_OFF
      << " wrt size of uncompressed" << endl << AF1 << "intersects" << AF2
      << totalIntersectTime / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalIntersectTime) / totalCompletionTime << "% " << AF4
      << totalIntersectVolume / totalIntersectTime << " MiB/sec" << EMPH_OFF
      << " wrt nof elements scanned" << endl << AF1 << "intersect wordlists"
      << AF2 << totalIntersectWordlistsTime / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalIntersectWordlistsTime)
      / totalCompletionTime << "% " << endl << AF1 << "intersect wordlists 1"
      << AF2 << totalIntersectWordlistsTime1 / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalIntersectWordlistsTime1)
      / totalCompletionTime << "% " << endl << AF1 << "intersect wordlists 2"
      << AF2 << totalIntersectWordlistsTime2 / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalIntersectWordlistsTime2)
      / totalCompletionTime << "% " << endl << AF1 << "intersect wordlists 3"
      << AF2 << totalIntersectWordlistsTime3 / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalIntersectWordlistsTime3)
      / totalCompletionTime << "% " << endl << AF1 << "intersect wordlists 4"
      << AF2 << totalIntersectWordlistsTime4 / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalIntersectWordlistsTime4)
      / totalCompletionTime << "% " << endl << AF1 << "intersect wordlists 5"
      << AF2 << totalIntersectWordlistsTime5 / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalIntersectWordlistsTime5)
      / totalCompletionTime << "% " << endl
  //<< AF1 << "sort+score"
      //<< AF2 << totalSortTime/(1000.0*nofQueries) << " millisecs"
      //<< AF3 << (100.0*totalSortTime)/totalCompletionTime << "% "
      //<< AF4 << totalSortVolume/totalSortTime 
      //       << " MiB/sec" << EMPH_OFF << " wrt input size" << endl
      << AF1 << "scoring docs" << AF2 << totalScoreDocsTime / (1000.0
      * nofQueries) << " millisecs" << AF3 << (100.0 * totalScoreDocsTime)
      / totalCompletionTime << "% " << AF4 << (totalSortVolume / 2)
      / totalScoreDocsTime //currently exactly half the volume is in docs
      << " MiB/sec" << EMPH_OFF << " wrt input size" << endl << AF1
      << "scoring docs 1 (preparation for partial sort, including copy)" << AF2
      << totalScoreDocsTime1 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalScoreDocsTime1) / totalCompletionTime << "% " << endl
      << AF1 << "scoring docs 2 (go through and aggregate scores)" << AF2
      << totalScoreDocsTime2 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalScoreDocsTime2) / totalCompletionTime << "% " << endl
      << AF1 << "scoring docs 3" << AF2 << totalScoreDocsTime3 / (1000.0
      * nofQueries) << " millisecs" << AF3 << (100.0 * totalScoreDocsTime3)
      / totalCompletionTime << "% " << endl << AF1
      << "scoring docs 4 (partial sort the docs)" << AF2 << totalScoreDocsTime4
      / (1000.0 * nofQueries) << " millisecs" << AF3 << (100.0
      * totalScoreDocsTime4) / totalCompletionTime << "% " << endl << AF1
      << "inv merge timer" << AF2 << totalInvMergeTime / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalInvMergeTime)
      / totalCompletionTime << "% " << endl << AF1 << "broad history timer"
      << AF2 << totalBroadHistoryTime / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalBroadHistoryTime) / totalCompletionTime << "% "
      << endl << AF1
      << "broad history timer 0 (direct match, but not copy+filter)" << AF2
      << totalBroadHistoryTime0 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalBroadHistoryTime0) / totalCompletionTime << "% " << endl
      << AF1 << "broad history timer 1 (direct match copy+filter)" << AF2
      << totalBroadHistoryTime1 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalBroadHistoryTime1) / totalCompletionTime << "% " << endl
      << AF1 << "broad history timer 2 (normal filtering + copyAndFilter)"
      << AF2 << totalBroadHistoryTime2 / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalBroadHistoryTime2) / totalCompletionTime << "% "
      << endl << AF1
      << "broad history timer 3 (filtering of last but one prefix)" << AF2
      << totalBroadHistoryTime3 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalBroadHistoryTime4) / totalCompletionTime << "% " << endl
      << AF1 << "broad history timer 4 (advanced filtering/shortcuts)" << AF2
      << totalBroadHistoryTime4 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalBroadHistoryTime4) / totalCompletionTime << "% " << endl
      << AF1 << "broad history timer 5 (default case, beginning)" << AF2
      << totalBroadHistoryTime5 / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalBroadHistoryTime5) / totalCompletionTime << "% " << endl
      << AF1 << "broad history timer 6 (end of default case, set completions)"
      << AF2 << totalBroadHistoryTime6 / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalBroadHistoryTime6) / totalCompletionTime << "% "
      << endl << AF1
      << "broad history timer 7 (end of default case, finalizing result)"
      << AF2 << totalBroadHistoryTime7 / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalBroadHistoryTime7) / totalCompletionTime << "% "
      << endl << AF1 << "set completions time" << AF2
      << totalSetCompletionsTime / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalSetCompletionsTime) / totalCompletionTime << "% "
      << endl << AF1 << "set completions time 1" << AF2
      << totalSetCompletionsTime1 / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalSetCompletionsTime1) / totalCompletionTime
      << "% " << endl << AF1 << "set completions time 2" << AF2
      << totalSetCompletionsTime2 / (1000.0 * nofQueries) << " millisecs"
      << AF3 << (100.0 * totalSetCompletionsTime2) / totalCompletionTime
      << "% " << endl << AF1 << "scoring words" << AF2 << totalScoreWordsTime
      / (1000.0 * nofQueries) << " millisecs" << AF3 << (100.0
      * totalScoreWordsTime) / totalCompletionTime << "% " << AF4
      << (totalSortVolume / 2) / totalScoreWordsTime //currently exactly half the volume is in words
      << " MiB/sec" << EMPH_OFF << " wrt input size" << endl << AF1
      << "appending (in intersect)" << AF2 << totalAppendTime / (1000.0
      * nofQueries) << " millisecs" << AF3 << (100.0 * totalAppendTime)
      / totalCompletionTime << "% " << endl
  //<< AF1 << " reserving (at beginning)"
      //<< AF2 << totalReserveTime/(1000.0*nofQueries) << " millisecs"
      //<< AF3 << (100.0*totalReserveTime)/totalCompletionTime << "% " << endl
      //<< AF1 << "write buffers"
      //<< AF2 << totalWriteBufferTime/(1000.0*nofQueries) << " millisecs"
      //<< AF3 << (100.0*totalWriteBufferTime)/totalCompletionTime << "%" << endl
      << AF1 << "history" << AF2 << totalHistoryTime / (1000.0 * nofQueries)
      << " millisecs" << AF3 << (100.0 * totalHistoryTime)
      / totalCompletionTime << "%" << endl;
  unsigned int totalOtherTime = totalCompletionTime - totalReadTime
      - totalResizeAndReserveTime - totalAppendTime
      - totalDoclistDecompressionTime - totalPositionlistDecompressionTime
      - totalWordlistDecompressionTime - totalIntersectTime
      - totalIntersectWordlistsTime - totalScoreDocsTime - totalScoreWordsTime
      - totalBroadHistoryTime;
  // - totalWriteBufferTime - totalReserveTime;
  cout << AF1 << "all the rest" << AF2 << totalOtherTime
      / (1000.0 * nofQueries) << " millisecs" << AF3
      << (100.0 * totalOtherTime) / totalCompletionTime << "%" << endl
      << EMPH_OFF << endl;

  cout << endl;
  if (showFuzzySearchStats)
  {
    cout << EMPH_ON << "SUMMARY" << EMPH_OFF << " for fuzzysearch" << endl
        << endl << flush;
    // if (!excludeTimesWWWPaper)
    // {
    cout << AF1 << "total fuzzysearch time" << AF2 << totalfuzzySearchTotalTime
        / (1000.0 * nofQueries) << " millisecs" << AF3 << (100.0
        * totalfuzzySearchTotalTime) / totalCompletionTime << "% " << endl
        << AF1 << "total fuzzysearch processQuery time" << AF2
        << totalFuzzySearchProcessQueryTime / (1000.0 * nofQueries)
        << " millisecs" << AF3 << (100.0 * totalFuzzySearchProcessQueryTime)
        / totalCompletionTime << "% " << endl << AF1
        << "total fuzzysearch distance computation time" << AF2
        << totalFuzzySearchDistanceComputationsTime / (1000.0 * nofQueries)
        << " millisecs" << AF3 << (100.0
        * totalFuzzySearchDistanceComputationsTime) / totalCompletionTime
        << "% " << endl;
    // }
    cout << AF1 << "avg. number of similar words" << AF2
        << totalNumOfSimilarWords / (1.0 * nofQueries) << endl << AF1
        << "avg. number of clusters" << AF2 << totalFuzzySearchClusters / (1.0
        * nofQueries) << endl << AF1 << "precision" << AF2
        << totalFuzzySearchPrecision / (1.0 * nofQueries) * 100 << " %" << endl
        << AF1 << "recall" << AF2 << totalFuzzySearchRecall
        / (1.0 * nofQueries) * 100 << " %" << endl;
    cout << endl;
  }

} /* end of method answerQueries */

//
// IS X IN GIVEN RANGE, where range is given by string like 4 5+ etc.
//
bool isInRange(int x, string range)
{
  if (range.length() == 0)
    return false;
  if (range[range.length() - 1] == '+')
    return (x >= atoi(range.c_str())); /* atoi parses longest prefix */
  if (range[range.length() - 1] == '-')
    return (x <= atoi(range.c_str()));
  return (x == atoi(range.c_str()));
}

//
// MAIN : parse command line argument and call answerQueries
//
int main(int argc, char *argv[])
{
  //#define VERSION "Version "__DATE__" "__TIME__
  cout << endl << EMPH_ON << "ANSWER QUERIES (" << VERSION << ")" << EMPH_OFF
      << endl << endl;

  //
  // PROCESS COMMAND LINE ARGUMENTS (gloabal variables declared after printUsage() above)
  //
  fuzzySearchEnabled = false;
  while (true)
  {
    char c = getopt(argc, argv, "d:n:v:CFHp:q:r:OLSs:y:fYeWh:");
    if (c == -1)
      break;
    switch (c)
    {
    case 'd':
      prefixLengthRange = optarg;
      break;
    case 'n':
      querySizeRange = optarg;
      break;
    case 'v':
      volumeRange = optarg;
      break;
    case 'C':
      showCompletions = true;
      break;
    case 'F':
      filterResults = false;
      break;
    case 'L': // "ell" not 1
      useLocations = false;
      break;
    case 'S':
      useScores = false;
      break;
    case 'p':
      min_prefix_length = atoi(optarg);
      break;
    case 'q':
      query_file = optarg;
      break;
    case 'r':
      nof_query_repeats = atoi(optarg) > 1 ? atoi(optarg) : 1;
      //clearHistory = (nof_query_repeats > 1);
      break;
    case 'H':
      clearHistory = true;
      break;
    case 'O':
      showStatisticsPerQuery = false;
      break;
    case 's':
      statisticsFileName = optarg;
      break;
    case 'y':
      yagoResultFileName = optarg;
      break;
    case 'f':
      showFuzzySearchStats = true;
      break;
    case 'Y':
      fuzzySearchEnabled = true;
      break;
    case 'e':
      excludeTimesWWWPaper = true;
      break;
    case 'E':
      localeString = optarg;
    case 'h':
      historyMaxSizeInBytes = atoi_ext(optarg);
      break;
    default:
      cout << endl << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << setbase(16) << int(c) << ")" << endl << endl;
      exit(1);
    }
  }
  localeString = "en_US.iso88591";

  // first remaining argument must be one of INV or HYB
  if (optind >= argc)
  {
    printUsage();
    exit(1);
  }
  method = argv[optind++];
  if (method != "INV" && method != "HYB")
  {
    printUsage();
    exit(1);
  }

  // next argument must be name of index file (was: db name)
  if (optind >= argc)
  {
    printUsage();
    exit(1);
  }
  indexFileName = argv[optind++];
  if (fuzzySearchEnabled)
  {
    baseName = indexFileName.substr(0, indexFileName.length() - 7);
    int encoding = WordClusteringBuilder<string>::ISO_8859_1;
    if (localeString.find("utf8") != string::npos)
      encoding = WordClusteringBuilder<string>::UTF_8;
    if (encoding == WordClusteringBuilder<string>::UTF_8)
      fuzzySearcher = new FuzzySearch::FuzzySearcherUtf8();
    else
      fuzzySearcher = new FuzzySearch::FuzzySearcherIso88591();
    // fuzzySearcher->setCutoff(fuzzySearchNeighbourhoodSize);
    // fuzzySearcher->setThreshold(fuzzySearchEditDistanceThreshold);
    fuzzySearcher->init(baseName);
  } else
  {
    fuzzySearcher = &nullFuzzySearcher;
  }
  // vocabulary file is basename of index file + suffix .vocabulary
  vocFileName = indexFileName;
  vocFileName = vocFileName.erase(vocFileName.rfind('.')) + ".vocabulary";

  // add queries from file if specified
  if (query_file != NULL)
  {
    cout << "* reading queries from file \"" << query_file << "\" ... "
        << flush;
    FILE* file = fopen(query_file, "r");
    if (file == NULL)
    {
      cout << endl << "! ERROR opening \"" << query_file << "\" (" << strerror(
          errno) << ")" << endl << endl;
      exit(1);
    }
    const int BUF_SIZE = 1000;
    char line[BUF_SIZE];
    while (fgets(line, BUF_SIZE, file))
    {
      int len = strlen(line);
      if (len == BUF_SIZE - 1)
      {
        cout << endl << "! ERROR reading \"" << query_file
            << "\" (line too long)" << endl << endl;
        exit(1);
      }
      while (len > 0 && iswspace(line[len - 1]))
        line[--len] = 0; // remove trailing space, tab, newline etc.
      queries.push_back(line);
    }
    cout << "done (" << queries.size() << " queries)" << endl << endl;
  }

  // if no query file specified, take remaining arguments as queries 
  if (queries.empty() && optind < argc)
  {
    cout << "* reading queries from command line ... " << flush;
    for (int i = optind; i < argc; ++i)
      queries.push_back(argv[i]);
    cout << "done (" << numberAndNoun(queries.size(), "query", "queries")
        << ")" << endl;
  }

  // with -p option add prefixes
  if (min_prefix_length >= 0)
  {
    if (min_prefix_length == 0)
      cout << "* adding whole-word queries ... " << flush;
    else
      cout << "* adding queries with minimal prefix length "
          << min_prefix_length << " ... " << flush;
    vector<string> queriesWithPrefixes;
    int posLastSpace;
    for (unsigned int i = 0; i < queries.size(); ++i)
    {
      posLastSpace = -1;
      for (unsigned int j = 0; j < queries[i].length(); ++j) /* check each prefix query[0..j] */
      {
        if (queries[i][j] == ' ')
          posLastSpace = j;
        if (((min_prefix_length == 0 || (int) (j) - posLastSpace
            < min_prefix_length)
        /* option -p0 or word shorter than min prefix length */
        && (j == queries[i].length() - 1 || queries[i][j + 1] == ' '))
            /* query[0..j] followed by space or end */
            || (min_prefix_length > 0 && j + 1
                >= (unsigned int) min_prefix_length && queries[i].substr(j + 1
                - min_prefix_length, min_prefix_length).find(' ')
                == string::npos))
        { /* last min_prefix_size chars of query[0..j] are non-space */
          if (queries[i][j] != '*')
          {
            queriesWithPrefixes.push_back(queries[i].substr(0, j + 1) + '*'); /* append a '*' if not already there */
          }
          else
          {
            queriesWithPrefixes.push_back(queries[i].substr(0, j + 1));
          }
        }
      }
    }
    queries = queriesWithPrefixes;
    cout << "done (now " << numberAndNoun(queries.size(), "query", "queries")
        << ")" << endl;
  }

  //
  // ANSWER QUERIES according to the specified method
  //
  //   NOTE: INV currently does not work anymore. Would need not separate into
  //   .cpp and .h first, as done for HYBCompleter. I'm not sure if that is all 
  //   that is needed (Holger 28Aug07)
  //
  unsigned char MODE = 0;
  MODE += ((useLocations) ? (2) : (0)) + ((useScores) ? (4) : (0));
  if (method == "HYB" && MODE == WITH_POS + WITH_SCORES)
    answerQueries<HybCompleter<WITH_DUPS + WITH_POS + WITH_SCORES> , HYBIndex> ();
#ifdef COMPILE_INV
  else if (method == "INV" && MODE == WITH_POS + WITH_SCORES)
  answerQueries< INVCompleter<WITH_DUPS + WITH_POS + WITH_SCORES>, INVIndex >();
#endif
#ifdef ALL_INDEX_VARIANTS
  else if (method == "INV" && MODE == 0)
  answerQueries< INVCompleter< 0>, INVIndex >();
  else if (method == "INV" && MODE == WITH_SCORES + WITH_POS)
  answerQueries< INVCompleter<WITH_SCORES+WITH_POS+WITH_DUPS>, INVIndex >();
  else if (method == "INV" && MODE == WITH_POS)
  answerQueries< INVCompleter<WITH_POS+WITH_DUPS>, INVIndex >();
  else if (method == "INV" && MODE == WITH_SCORES)
  answerQueries< INVCompleter<WITH_SCORES>, INVIndex >();
  else if (method == "HYB" && MODE == 0)
  answerQueries< HybCompleter<WITH_DUPS>, HYBIndex >();
  else if (method == "HYB" && MODE == WITH_POS + WITH_SCORES)
  answerQueries< HybCompleter<WITH_DUPS + WITH_POS + WITH_SCORES>, HYBIndex >();
  else if (method == "HYB" && MODE == WITH_POS)
  answerQueries< HybCompleter<WITH_DUPS + WITH_POS>, HYBIndex >();
  else if (method == "HYB" && MODE == WITH_SCORES)
  answerQueries< HybCompleter<WITH_DUPS + WITH_SCORES>, HYBIndex >();
#endif
  else
  {
    cout << endl << "! ERROR in main : invalid value for MODE" << " ("
        << (int) MODE << ")" << endl << endl;
    exit(1);
  }

}
// _____________________________________________________________________________
// CONVERT TO INTEGER
// accepting things like 10K (=10*1024) or 10M (=10*1024*1024)
int atoi_ext(string s)
{
  if (s.length() == 0)
    return 0;
  switch (s[s.length() - 1])
  {
  case 'k':
  case 'K':
    return atoi(s.substr(0, s.length() - 1).c_str()) * 1024;
  case 'm':
  case 'M':
    return atoi(s.substr(0, s.length() - 1).c_str()) * 1024 * 1024;
  case 'g':
  case 'G':
    return atoi(s.substr(0, s.length() - 1).c_str()) * 1024 * 1024 * 1024;
  default:
    return atoi(s.c_str());
  }
}
