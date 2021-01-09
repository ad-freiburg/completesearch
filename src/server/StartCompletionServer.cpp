// Copyright 2010 Hannah Bast

/*
 * Type 'CompleteServer' to see the usage of this server.
 *
 * If the server has been successfully started, a file $HOME/.CompleteServer:portnumber.pid is written.
 * This file is needed when the -k option is given.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <regex.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <clocale>
#include <string>
#include "./ExcerptsGenerator.h"
#include "./QueryParameters.h"
#include "./ConcurrentLog.h"
#include "./HYBCompleter.h"
#include "./INVCompleter.h"
#include "./CompletionServer.h"
#include "./Exception.h"
#include "./Timer.h"
#include "./Globals.h"

// using namespace std;
// using namespace aucmpl;

// PARAMETERS with defaults
void* indexForCompletionServer;
int port = 8888;
int portToKill = 0;
bool killServerIfRunning = true;
bool autoRestart = false;
static enum { DOUBLE_FORK, SINGLE_FORK, NO_FORK } forkMode = DOUBLE_FORK;
string indexFileName = "";       // NOLINT
string excerptsDBFileName = "";  // NOLINT
string logFileName = "";         // NOLINT
#define LOG_BEGIN pthread_mutex_lock(&log_file_mutex);
#define LOG_END pthread_mutex_unlock(&log_file_mutex);
string vocabularyFileName;       // NOLINT
string indexTypeName = "HYB";    /* "INV" or "HYB" */  // NOLINT
string mapsDirectory = "codebase/utility";
// FILE* log_file = NULL;
extern bool sendErrorDetailsToClient;
extern string pidFileNameFormatString;
extern string exe_command;
extern string documentRoot;
extern off_t queryTimeout;
extern bool fuzzySearchEnabled;
extern bool synonymSearchEnabled;
// extern int fuzzySearchNeighbourhoodSize;
// extern double fuzzySearchEditDistanceThreshold;
extern bool fuzzySearchNormalizeWords;
extern bool normalizeWords;
extern bool rankByGeneralizedEditDistance;
extern bool fuzzySearchUseClustering;
extern bool readCustomScores;
extern string baseName;
extern bool showQueryResult;
extern bool alreadyWellformedXml;
extern char infoDelim;
extern char wordPartSepFrontend;
extern bool corsEnabled;
// Default values for ranking and score aggregation
extern QueryParameters::HowToRankDocsEnum  howToRankDocsDefault;
extern QueryParameters::HowToRankWordsEnum howToRankWordsDefault;
extern SortOrderEnum sortOrderDocsDefault;
extern SortOrderEnum sortOrderWordsDefault;
extern ScoreAggregation docScoreAggDifferentQueryPartsDefault;
extern ScoreAggregation docScoreAggSameCompletionDefault;
extern ScoreAggregation docScoreAggDifferentCompletionsDefault;
extern ScoreAggregation wordScoreAggSameDocumentDefault;
extern ScoreAggregation wordScoreAggDifferentDocumentsDefault;


// _____________________________________________________________________________
// USAGE INFO
void printUsage()
{
  cout << endl
       << EMPH_ON << "COMPLETION SERVER (" << VERSION << ")" << EMPH_OFF << endl
                  << endl
       << "Usage: startCompletionServer [options] <db>.[inverted|hybrid]"
       << endl << endl
       << "Options:" << endl
       << endl
       << " -p port number       The port where the server is listening "
                                 "(default: 8888)"
       << endl
       << " -l log file name     File name for log messages "
                                 "(default: <db>.log)"
       << endl
       << " -k port number       Stop server running at specified port "
                                 "(ignores all other options/parameters)"
       << endl
       // << "           -s synonyms_db_file  The name of an synonyms.db file\n"
       << " -c cache size        Sets the cache size for the excerpts "
                                 "generator (default: 16 megabytes)"
       << endl
       << " -h history size      Set the history size "
                                 "(default: 32 megabytes)"
       << endl
       << " -q max nof queries   At most that many queries in history "
                                 "(default: 200; note: current impl. is "
                                 "quadratic)"
       << endl
       << " -e docs file         Name of file containing excerpts info "
                                 "(default: <db>.docs.DB)"
       << endl
       // << " -e exerpts file      Name of BerkeleyDB file containing with "
       //                          "document info (default: <db>.docs.db)"
       //                          << endl
       << " -t [INV|HYB]         Type of index "
                                 "(default: guess from index file name)"
       << endl
       << " -v verbosity level   Set the log verbosity (1 = normal, 2 = high, "
                                 "default is 1)"
       << endl
       << " -P pid file format   Specifiy name of file containing the process "
                                 "id, leading ~ will be replaced by home dir, "
                                 "first %s"
       << endl
       << "                      will be replaced by host name, second %s "
                                 "will be replaced by port. Default is "
                                 << pidFileNameFormatString
       << endl
       << " -i infofield sep     Delimiter to split a multiple field"
       << endl
       << " -L encoding          Set LC_ALL to this string, irrespective of "
                                 "special \"!encoding:...\" word in index"
       << endl
       << " -D document root     Specify document root. If set, a given "
                                 "(non-query/non-search) request is searched "
                                 "within document root"
       << endl
       << " -o timeout           Specify the time a query is allowed to be processed. "
       <<                        "After <timeout> msecs the processing will be aborted."
       << endl
       << " -f frontend sep	 Specify the frontend word separator, which should be "
                                 "used within the api"
       << endl
       << " -b backend sep       Specify the backend word separator, which is used "
                                 "within the index"
       << endl
       << " -s aggregation       Specify score aggregation by a 4-letter "
                                 "string over the alphabet {S,M,B}, see "
                                 "explanations below"
       << endl
       << " -M maps-directory    Specify the directory containing maps for "
                                 "utf8 and iso conversion (default = current directory)."
       << endl
       << " -A queries-file      Never remove the results for the queries in "
                                 "the given file from the history (NOT YET IM)"
       << endl
       << " -I queries-file      Upon server start, execute the queries from "
                                 "the given file so that they are in the history"
       << endl

       << " -d                   Specify how to rank documents (0 = by score, "
                                 "1 = by doc id, 2 = by word id, "
       << endl
       << "                      followed by a = ascending or d = descending, "
                                 "default is 0d)"
       << endl
       << " -w                   Specify how to rank words (0 = by score, "
                                 "1 = by doc count, 2 = by occ count, "
       << endl
       << "                      3 = by word id, followed by a = ascending "
                                 "or d = descending, deafult is 0d)"
       << endl
       << " -F                   No double fork, process will run forever or "
                                 "until server killed"
       << endl
       << " -Z                   No fork at all, everything running as one "
                                 "process, log output to terminal (for "
                                 "debugging)"
       << endl
       << " -K                   Do not kill a running server (default: kill "
                                 "server, if one is running on given port)"
       << endl
       << " -r                   Automatically restart the server if it "
                                 "crashes (requires double fork mode)"
       << endl
       << " -m                   Run in multithreaded mode (default: process "
                                 "one query after the other; still "
                                 "recommended)"
       << endl
       << " -E                   On error, send single hit with error message "
                                 "(will be seen in browser then)"
       << endl
       << " -G                   Use generalized edit distance to rank the"
       << "                      word-ids (slow!)."
       << endl
       << " -C                   Cleanup query before processing" 
       << endl
       << " -H                   Stop escaping text (using cdata). Should be "
                                 "used if the text is already proper escaped "
                                 "xml" 
       << endl
       << " -U                   Enables possibility to find [...:]wordNorm:wordOriginal "
       	                         "by wordOriginal instead of wordOriginal:*"
       << endl
       << " -O                   Enables cross-origin resource sharing (CORS) "
                                 "by sending Access-Control-Allow-Origin: *"
       << endl << endl
       << "Cache/history sizes must be greater than 0 and are given in one of "
          "the forms:"
       << endl << endl
       << "  n   meaning n bytes" << endl
       << "  nK  meaning n kilobytes" << endl
       << "  nM  meaning n megabytes" << endl
       << "  nG  meaning n gigabytes" << endl
       << endl
       << "Score aggregation: There are currently three types of score "
          "aggregation, S = sum, M = max, B = sum with bonus for"
       << endl
       << "proximity and exact word match. There are two aggregations for "
          "doc scores (same completion, different completion)"
       << endl
       << "and two aggregations for word scores (same doc, different doc)"
       << endl << endl;
}

// _____________________________________________________________________________
// FUNCTION DECLARATIONS (implementation below, after main)
size_t atoi_ext(string s);
void startServer(string indexTypeName);
template<class Completer, class Index> void startServer();

// _____________________________________________________________________________
// MAIN LOOP
int main(int argc, char** argv)
{
  // Parse options.
  try
  {
    while (true)
    {
      static struct option long_options[] =
      {
        {"auto-restart"                       , 0, NULL, 'r'},
        {"port"                               , 1, NULL, 'p'},
        {"kill"                               , 1, NULL, 'k'},
        {"log-file"                           , 1, NULL, 'l'},
        {"cache-size-excerpts"                , 1, NULL, 'c'},
        {"max-size-history"                   , 1, NULL, 'h'},
        {"max-queries-history"                , 1, NULL, 'q'},
        {"no-statistics"                      , 1, NULL, 'V'},
        {"index-type"                         , 1, NULL, 't'},
        {"info-delimiter"                     , 1, NULL, 'i'},
        {"kill-running-server-false"          , 0, NULL, 'K'},
        {"no-double-fork"                     , 0, NULL, 'F'},
        {"zero-fork"                          , 0, NULL, 'Z'},
        {"multi-threaded"                     , 0, NULL, 'm'},
        {"verbosity"                          , 1, NULL, 'v'},
        {"how-to-rank-docs"                   , 1, NULL, 'd'},
        {"how-to-rank-words"                  , 1, NULL, 'w'},
        {"score-aggregations"                 , 1, NULL, 's'},
        {"pid-file"                           , 1, NULL, 'P'},
        {"exe-command"                        , 1, NULL, 'X'},
        {"locale"                             , 1, NULL, 'L'},
        {"enable-fuzzy-search"                , 0, NULL, 'Y'},
        {"enable-synonym-search"              , 0, NULL, 'S'},
        {"normalize-words"                    , 0, NULL, 'N'},
        {"fuzzy-normalize-words"              , 0, NULL, 'W'},
        {"show-query-result"                  , 0, NULL, 'Q'},
        {"use-generalized-edit-distance-slow" , 0, NULL, 'G'},
        {"use-baseline-fuzzysearch"           , 0, NULL, 'B'},
        {"cleanup-query-before-processing"    , 0, NULL, 'C'},
        {"use-suffix-for-exact-query"         , 0, NULL, 'U'},
        {"disable-cdata-tags"                 , 0, NULL, 'H'},
        {"document-root"                      , 1, NULL, 'D'},
        {"maps-directory"                     , 1, NULL, 'M'},
        {"query-timeout"                      , 1, NULL, 'o'},
        {"word-part-separator-frontend"       , 1, NULL, 'f'},
        {"word-part-separator-backend"        , 1, NULL, 'b'},
        {"read-custom-scores"                 , 0, NULL, '0'}, 
        {"keep-in-history-queries"            , 1, NULL, 'A'}, 
        {"warm-history-queries"               , 1, NULL, 'I'}, 
        {"enable-cors"                        , 0, NULL, 'O'},
        {NULL                                 , 0, NULL,  0 }
      };
      int c = getopt_long(argc, argv,
          "A:Bb:Cc:D:d:Ee:Ff:GHh:I:i:Kk:L:l:MmN:o:P:p:Qq:rS:s:t:UVv:Ww:X:YZ0",
          long_options, NULL);

      if (c == -1) break;
      switch (c)
      {
        case 'r': autoRestart = true;
                  break;
        case 'p': port = atoi(optarg);
                  break;
        case 'k': portToKill = atoi(optarg);
                  break;
        case 'l': logFileName = optarg;
                  break;
        case 'e': excerptsDBFileName = optarg;
                  break;
        case 'c': excerptsDBCacheSize = atoi_ext(optarg);
                  break; /* permits suffix K or M */
        case 'h': historyMaxSizeInBytes = atoi_ext(optarg);
                  break; /* dito */
        case 'q': historyMaxNofQueries = atoi(optarg);
                  break;
        case 't': indexTypeName = optarg;
                  break;
        case 'K': killServerIfRunning = false;
                  break;
        case 'F': forkMode = SINGLE_FORK;
                  break;
        case 'Z': forkMode = NO_FORK;
                  break;
        case 'V': showStatistics = false;
                  break;
        case 'm': runMultithreaded = true;
                  break;
        case 'v': ::logVerbosity = atoi(optarg);
                  break;
        case 'E': sendErrorDetailsToClient = true;
                  break;
        case 'd': QueryParameters::setHowToRank(optarg, howToRankDocsDefault,
                                                sortOrderDocsDefault);
                  break;
        case 'w': QueryParameters::setHowToRank(optarg, howToRankWordsDefault,
                                                sortOrderWordsDefault);
                  break;
        case 's': QueryParameters::setAllScoreAggregationDefaults(optarg);
                  break;
        case 'P': pidFileNameFormatString = optarg;
                  break;
        case 'L': localeString = optarg;
                  break;
        case 'X': exe_command = optarg;
                  break;
        case 'Y': fuzzySearchEnabled = true;
                  break;
        case 'S': synonymSearchEnabled = true;
                  break;
        // case 'y': fuzzySearchEditDistanceThreshold = atof(optarg);
        //          break;
        // case 'z': fuzzySearchNeighbourhoodSize = atoi(optarg);
        //          break;
        case 'W': fuzzySearchNormalizeWords = true;
                  break;
        case 'N': normalizeWords = true;
                  break;
        case 'G': rankByGeneralizedEditDistance = true;
                  break;
        case 'B': fuzzySearchUseClustering = false;
                  break;
        case 'Q': showQueryResult = true;
                  break;
        case 'C': cleanupQueryBeforeProcessing  = true;
                  break;
        case 'U': useSuffixForExactQuery  = true;
                  break;
        case 'H': alreadyWellformedXml = true;
                  break;
        case 'D': documentRoot = optarg;
                  break;
        case 'o': queryTimeout = atoi(optarg);
                  break;
        case 'i': infoDelim = optarg[0];
		  break;
 	case 'f': wordPartSepFrontend = optarg[0];
		  break;
 	case 'b': wordPartSep = optarg[0];
		  break;
 	case 'M': mapsDirectory = optarg;
		  break;
        case '0': readCustomScores = true;
                  break;
        case 'A': keepInHistoryQueriesFileName = optarg;
                  break;
        case 'I': warmHistoryQueriesFileName = optarg;
                  break;
        case 'O': corsEnabled = true;
                  break;
        default : printUsage();
                  exit(1);
                  break;
      }
    }
  }
  catch(Exception& e)
  {
    cout << "! " << e.getFullErrorMessage() << endl << endl;
    exit(1);
  }

  // If called with option -k <port>, kill corresponding server and exit
  // (killServer and getPidFileName are defined in CompletionServer.cpp).
  if (portToKill > 0)
  {
    killServer(getPidFileName(portToKill));
    exit(0);
  }

  // If called without non-option argument, print usage info and exit.
  if (optind >= argc)
  {
    printUsage();
    exit(1);
  }

  // Initialize the string conversion maps (requires file <encoding>.map,
  // default directory is ./utility, which has iso8859-1.map utf8.map).
  // Exits if map directory not found.
  if (globalStringConverter.init(mapsDirectory) == false)
  {
    cout << "! " << globalStringConverter.getLastError() << endl << endl;
    exit(1);
  }

  // Check that pid file name is valid. Otherwise, exit here.
  string pidFileName = getPidFileName(port);

  // PARSE REMAINING PARAMETERS (index file name)
  indexFileName = argv[optind++];
  string::size_type pos = indexFileName.find(".");
  baseName = pos != string::npos ? indexFileName.substr(0, pos) : indexFileName;
  if (excerptsDBFileName == "") excerptsDBFileName = baseName + ".docs.DB";
  if (vocabularyFileName == "") vocabularyFileName = baseName + ".vocabulary";
  if (logFileName == "") logFileName = baseName + ".log";
  if (indexTypeName != "INV" && indexTypeName != "HYB")
  {
    if (indexFileName.find("inverted") != string::npos)
      indexTypeName = "INV";
    else if (indexFileName.find("hybrid") != string::npos)
      indexTypeName = "HYB";
    else
    {
      cerr << "! ERROR in main: could not guess index type" << endl << endl;
      printUsage();
      exit(1);
    }
  }

  // OPEN LOG FILE
  // NEW 18Sep06 (Holger): was "w" (=truncated)
  log_file = fopen(logFileName.c_str(), "a");
  if (log_file == NULL)
  {
    perror("! ERROR in main: fopen logfile");
    exit(1);
  }

  // START SERVER
  try
  {
    startServer(indexTypeName);
  }
  catch(Exception& e)
  {
    cerr << EMPH_ON << "COMPETION SERVER ABNORMALLY TERMINATED: "
         << e.getFullErrorMessage() << EMPH_OFF << endl << endl;
  }

  // EXIT (should never happen though)
  cerr << "! Exiting server (this is not supposed to happen)" << endl << endl;
  exit(1);
}  // end of MAIN


// _____________________________________________________________________________
// START SERVER (wrapper for the actual, templated startServer below)
void startServer(string indexTypeName)
{
  // NEW 26Sep06 (Holger): INVCompleter not yet updated for use in Cygwin
  // (see comment in CompletionServer.h)
  // if (indexTypeName == "INV") { cerr << "NOT IN CYGWIN" << endl; exit(1); }
  if (indexTypeName == "INV")
  {
    #ifdef COMPILE_INV
    startServer<INVCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> , INVIndex >();
    #else
    cerr << "*** NOT COMPILED FOR USE WITH INV ***" << endl << endl
         << "the respective line in StartCompletionServer.cpp "
            "(172 as of this writing)"
            " has been commented out to speed up compile-times."
            " Just de-comment and recompile if you really need INV."
         << endl << endl;
    exit(1);
    #endif
  }
  else if (indexTypeName == "HYB")
  {
    startServer<HybCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> , HYBIndex >();
  }
  else
  {
    cerr << "! ERROR in main::startServer: invalid index type name ("
      << indexTypeName << ")" << endl << endl;
    exit(1);
  }
}  // end of startServer wrapper



// START SERVER (double fork, actual server run by grandchild)
template<class Completer, class Index>
void startServer()
{
  pid_t pid;

  // NO FORK MODE: everything runs as one process, and log output to terminal
  if (forkMode == NO_FORK)
  {
    cout << "running in no fork mode (for debugging only)" << endl << endl;
  }

  // SINGLE FORK MODE: launch server and watch it
  if (forkMode == SINGLE_FORK)
  {
    cout << "SINGLE FORK MODE: process will run until server process "
            "terminates, all further output in \"" << logFileName << "\""
         << endl;
  }

  // DOUBLE FORK MODE: let child launch server and watch it, then exit
  if (forkMode == DOUBLE_FORK)
  {
    pid = fork();
    if (pid < 0)
    {
      perror("! ERROR in startServer: fork 1");
      exit(1);
    }
    if (pid > 0)
    {
      cout << "Forked server, all further output in \"" << logFileName << "\""
           << endl;
      exit(0);
    }
  }

  // SINGLE OR DOUBLE FORK MODE: run server as child and watch it
  if (forkMode == SINGLE_FORK || forkMode == DOUBLE_FORK)
  {
    dup2(fileno(log_file), STDOUT_FILENO);
    dup2(fileno(log_file), STDERR_FILENO);
    while (true)
    {
      pid = fork();
      // CASE: fork error (should not happen)
      if (pid < 0)
      {
        perror("! ERROR in startServer: fork 2");
        exit(1);
      }
      // CASE: child process is the actual server, code continues below ...
      if (pid == 0)
      {
        break;
      }
      // CASE: parent process watches child process (with optional restart)
      if (pid > 0)
      {
        int status;
        pid = wait(&status);
        cerr << "* " << "completion server with pid " << pid
             << " terminated ... " << flush;
        // Note: the W... macros are from clib
        if (WIFEXITED(status))
          cerr << "process exited with code " << WEXITSTATUS(status)
                                              << flush;
        else if (WIFSIGNALED(status))
          cerr << "process received signal " << WTERMSIG(status)
               << " (" << signalName(WTERMSIG(status)) << ")" << flush;
        else if (WIFSTOPPED(status))
          cerr << "process stopped by signal " << WSTOPSIG(status)
               << EMPH_ON << ", this should not happen" << EMPH_OFF << flush;
        else
          cerr << "process terminated for unknown reason"
               << EMPH_ON << ", this should not happen" << EMPH_OFF << flush;
        cerr << endl;
        // Restart server, unless signal was SIGUSR1 (= killed intentionally by
        // an explicit call to function killServer).
        if (WTERMSIG(status) != SIGUSR1 && autoRestart)
        {
          cerr << "* " << EMPH_ON << "automatic server restart" << EMPH_OFF
               << endl << endl;
          continue;
        }
        exit(status);
      }
    }
  }

  // The actual server code starts here. Depending on the fork mode, the
  // following code is executed by the original process, its child, or its 
  // grandchild.

  // Get server start time for log file
  time_t NOW = time(NULL);
  string currentTime = ctime(&NOW);
  size_t pos = currentTime.find_first_of("\r\n");
  if (pos != string::npos) currentTime.erase(pos);

  cout << endl
       << EMPH_ON << "COMPLETION SERVER (compiled " << VERSION << ")"
       << " started at " << currentTime
       << EMPH_OFF << endl
       << endl;
  if (runMultithreaded == false)
    cout << "* multithreaded mode turned off, processing one query "
            "after the other" << endl;
  Timer timer;
  timer.start();

  // INIT EXCERPTS DB
  if ( access(excerptsDBFileName.c_str(), F_OK) == 0 )
  {
    excerptsGenerator = new ExcerptsGenerator(excerptsDBFileName,
                                              excerptsDBCacheSize);
  }
  else
  {
    excerptsGenerator = NULL;
    cout << "! could not open \"" << excerptsDBFileName << "\"," 
         << EMPH_ON << "running without excerpts database"
         << EMPH_OFF << endl;
  }

  CompletionServer<Completer, Index> completionServer(indexFileName,
      vocabularyFileName,
      Completer::mode(),
      port,
      log_file,
      killServerIfRunning);  // must exist long enough !!!

  // cout << "! start waiting for request ..." << endl;
  nofRunningProcessorThreads = 0;
  completionServer.waitForRequestsAndProcess();

  cerr << "! reached code after infinite server loop, "
       << EMPH_ON << " this is not supposed to happen" 
       << EMPH_OFF << endl << endl;
}  // end of startServer


// _____________________________________________________________________________
// CONVERT TO INTEGER
// accepting things like 10K (=10*1024) or 10M (=10*1024*1024)
size_t atoi_ext(string s)
{
  if (s.length() == 0) return 0;
  switch (s[s.length()-1])
  {
    case 'k': case 'K':
      return (size_t)(atoi(s.substr(0, s.length()-1).c_str()))*1024;
    case 'm': case 'M':
      return (size_t)(atoi(s.substr(0, s.length()-1).c_str()))*1024*1024;
    case 'g': case 'G':
      return (size_t)(atoi(s.substr(0, s.length()-1).c_str()))*1024*1024*1024;
    default:
      return (size_t)(atoi(s.c_str()));
  }
}
