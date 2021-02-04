#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <fstream>
#include <cfloat>
#include <limits>
#include "CompletionServer.h"
#include "HYBCompleter.h"
#include "INVCompleter.h"
#include "./Globals.h"
#include "./Vector.h"
#include "../utility/XmlToJson.h"
#include "../fuzzysearch/StringDistances.h"
#include "server/CustomScorer.h"

// MMM TODO: declare which ever parameters you need for the fuzzy search. They
// are set in StartCompletionServer.cpp, where these variables are declared as
// extern.
bool fuzzySearchEnabled = false;
bool fuzzySearchNormalizeWords = false;
bool rankByGeneralizedEditDistance = false;
bool fuzzySearchUseClustering = true;
bool readCustomScores = false;
bool alreadyWellformedXml = false;
char infoDelim = '\0';
FuzzySearch::GeneralizedEditDistance* generalizedDistanceCalculator = NULL;
string baseName;

bool showQueryResult = false;

//! Name of the PID (process id) file. Leading ~ s will be replaced by home
//! directory, first %s will be replaced by hostname, second %d will be replaced 
//! by port. The ~ and the two %s are optional.
string pidFileNameFormatString = "~/.completesearch_%s_%s.pid";

//! Show error message as hit (activate with -E option)
bool sendErrorDetailsToClient = false;

//! Command to be executed for ?exe=... query.
string exe_command = "";

//! If set, non-query/non-search requests are searched in document root.
string documentRoot = "";

//! String that specifies the format of the different info fields by using J for
//! json and X for xml.
string infoFieldsFormats = "";

//! Vector holding attributes, which should always be handled as list within json.
vector<string> multipleAttributes = vector<string>();

//! Sorting factor c in c * log n * n.
//! Sorting is done in n * log n and we want to estimate, how long a sort does
//! take. Attention: The factor is computed by using a sufficient vector of
//! integers. In CompletionServer we might use Quadruples and other types - so
//! we might multiplicate with another factor, which specifies how many
//! additional operations are executed on each sort.
float parallelSortTimePerMillionIntegers = FLT_MAX;

#ifdef DEBUG_PTHREAD_CREATE_TIME
//! Timer for thread creation overhead (only measured when multithreading is off)
template<class Completer, class Index>
Timer CompletionServer<Completer, Index>::threadCreateTimer;
#endif

//! MUTEX FOR EXCERPT GENERATION (not yet thread-safe, TODO: why?)
pthread_mutex_t excerpts_generation;

//! Enables or disables cross-origin resource sharing.  If enabled, the HTTP
//! header "Access-Control-Allow-Origin: *" is sent with each response to
//! allow access from other domains and ports.
bool corsEnabled = false;

//! GET SIGNAL NAME (copied from man page for signal, cf. man 7 signal)
string signalName(int signal)
{
  switch (signal)
  {
  case SIGHUP:
    return "SIGHUP";
  case SIGINT:
    return "SIGINT";
  case SIGQUIT:
    return "SIGQUIT";
  case SIGILL:
    return "SIGILL";
  case SIGTRAP:
    return "SIGTRAP";
  case SIGABRT:
    return "SIGABRT";
  case SIGFPE:
    return "SIGFPE";
  case SIGKILL:
    return "SIGKILL";
  case SIGUSR1:
    return "SIGUSR1";
  case SIGSEGV:
    return "SIGSEGV";
  case SIGUSR2:
    return "SIGUSR2";
  case SIGPIPE:
    return "SIGPIPE";
  case SIGALRM:
    return "SIGALRM";
  case SIGTERM:
    return "SIGTERM";
  default:
    return "cf. man 7 signal";
  }
}

//! FILE NAME OF SERVER PID (full path)
string getPidFileName(int port)
{
  int MAX_LEN = 1000;
  char buf[MAX_LEN];
  // check that pidFileNameFormatString contains indeed %s, %s, %d (otherwise snprintf will produce a segmantation fault)
  try
  {
    size_t pos = 0;
    unsigned int count = 0;
    while ((pos = pidFileNameFormatString.find('%', pos)) != string::npos)
    {
      count++;
      pos++;
      if (pos >= pidFileNameFormatString.length())
      {
        CS_THROW(Exception::INVALID_PARAMETER_VALUE, "pid file name ends in %");
      }
      CS_ASSERT(pos < pidFileNameFormatString.length());
      if (count == 1 && pidFileNameFormatString[pos] != 's')
      {
        CS_THROW(Exception::INVALID_PARAMETER_VALUE, "first %. in pid file name must be \%s");
      }
      else if (count == 2 && pidFileNameFormatString[pos] != 's')
      {
        CS_THROW(Exception::INVALID_PARAMETER_VALUE, "second %. in pid file name must be \%s");
      }
      else if (count > 2)
      {
        CS_THROW(Exception::INVALID_PARAMETER_VALUE, "at most two \% allowed in pid file name");
      }
    }
  }
  catch (Exception& e)
  {
    cerr << "! " << e.getFullErrorMessage() << endl;
    exit(1);
  }
  ostringstream os;
  os << port;
  int ret = snprintf(buf, MAX_LEN, pidFileNameFormatString.c_str(),
      gethostname(MAX_LEN, "localhost").c_str(), os.str().c_str());
  if (ret >= MAX_LEN) LOG
      << "! WARNING: name of pid (process id) file truncated, max length is "
      << MAX_LEN;
  //cout << "! pid file name is \"" << buf << "\"" << endl;
  if (*buf != '~') return buf;
  else return getenv("HOME", ".") + (buf + 1);
  //ostringstream os;
  //os << getenv("HOME", ".") << "/.completesearch_" << getenv("HOSTNAME", "localhost") << "_" << port << ".pid";
  //return os.str();
}

string getHTTPStatusMessage(int statusCode) {
  switch (statusCode) {
    case 200: return "OK";
    case 204: return "No Content";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 408: return "Request Timeout";
    case 411: return "Length required";
    case 413: return "Request Entity Too Large";;
    case 414: return "Request-URI Too Long";
    case 417: return "Expectation Failed";
    case 500: return "Internal Error";
    default: return "Unknown Error"; break;
  }
}

//! KILL SERVER 
void killServer(string pidFileName)
{
  // WARNING: log file not opened here, when called from main => cannot use LOCK/UNLOCK
  cout << "! reading pid from file \"" << pidFileName << "\" ... " << flush;
  ifstream pidfile(pidFileName.c_str());
  if (!pidfile.is_open())
  {
    cerr << "could not open file, exiting" << endl;
    exit(1);
  }
  pid_t pid;
  pidfile >> pid;
  int rc;
  rc = kill(pid, SIGUSR1);
  if (rc != 0) cerr << "send SIGUSR1 to process " << pid << ": " << strerror(
      errno) << flush;
  else cout << "killed server with pid " << pid << flush;
  // NEW 29Oct06 (Holger): use unlink (more portable)
  rc = unlink(pidFileName.c_str());
  //string cmd = "rm " + pidFileName;
  //rc = system(cmd.c_str());
  if (rc != 0)
  {
    cerr << " ... tried but could not remove pid file (" << strerror(errno)
        << ")" << endl;
  }
  cout << " ... removed pid file" << endl;
} // end of function killServer


//! CONSTRUCTOR 
template<class Completer, class Index>
CompletionServer<Completer, Index>::CompletionServer(
    const string& passedIndexStructureFile,
    const string& passedVocabularyFile, unsigned char mode, int port,
    FILE* log_file, bool killServerIfRunning) :
  io_service(),
  acceptor(io_service),
  index(passedIndexStructureFile, passedVocabularyFile, mode)
{
  index.read();

  // Determine encoding, date, name, etc. 
  //
  //   Note: we create a completer object just for this purpose (like we will
  //   create a completer object for every query in the main server loop). Each
  //   completer gets a handle to (no copy of) the *one* index and the *one*
  //   history.
  //
  if (!fuzzySearchEnabled) _fuzzySearcher = &nullFuzzySearcher;
  Completer completer(&index, &history, &nullFuzzySearcher);
  string infoQuery = string(":info:encoding") + wordPartSep + string("*");
  completer.resetTimersAndCounters();
  string tmp1 = completer.getTopContinuationOfQuery(infoQuery);
  // NOTE(bast): the encoding set here determines the encoding in the first line
  // of the result XML.
  cout << "* extracting encoding for result XML from index : " << flush;
  if (tmp1 == "iso-8859-1")
  {
    encoding = Encoding::ISO88591;
    cout << "iso-8859-1" << endl;
  }
  else if (tmp1 == "utf-8")
  {
    encoding = Encoding::UTF8;
    cout << "utf-8" << endl;
  }
  else if (tmp1 == "")
  {
    encoding = Encoding::UTF8;
    cout << "not specified, taking utf-8" << endl;
  }
  else
  {
    encoding = Encoding::UTF8;
    cout << "not recognized (" << tmp1 << "), taking utf-8" << endl;
  }
  if (localeString != "")
  {
    cout << "* setting LC_ALL to user-specified \"" << localeString << "\""
        << endl;
    setlocale(LC_ALL, localeString.c_str());
    if (fuzzySearchEnabled)
    {
      if (rankByGeneralizedEditDistance) generalizedDistanceCalculator
          = new FuzzySearch::GeneralizedEditDistance();
      if (localeString.find("utf8") != string::npos)
      {
        cout << "* creating FuzzySearcher object using utf8" << endl;
        _fuzzySearcher = new FuzzySearch::FuzzySearcherUtf8();
      }
      else
      {
        cout << "* creating FuzzySearcher object using iso88591" << endl;
        _fuzzySearcher = new FuzzySearch::FuzzySearcherIso88591();
      }
      cout << "* fuzzysearch normalization : ";
      fuzzySearchNormalizeWords ? cout << "yes" << endl : cout << "no" << endl;
      _fuzzySearcher->useNormalization(fuzzySearchNormalizeWords);
      fuzzySearchEnabled = _fuzzySearcher->init(baseName);
    }
  }
  cout << "* locale (LC_ALL) is: " << setlocale(LC_ALL, NULL) << endl;

  // If locale specifies encoding, override the latter.
  if (localeString != "")
  {
    size_t pos = localeString.find(".");
    string encodingString = pos != string::npos ? localeString.substr(pos + 1)
        : "UTF8";
    for (size_t i = 0; i < encodingString.size(); ++i)
      encodingString[i] = toupper(encodingString[i]);
    cout << "* overriding encoding from user-specified locale : " << flush;
    if (encodingString == "UTF8")
    {
      encoding = Encoding::UTF8;
      cout << "utf-8" << endl;
    }
    else if (encodingString == "ISO88591")
    {
      encoding = Encoding::ISO88591;
      cout << "iso-8859-1" << endl;
    }
  }
  // Show date of index building and name of index (for convenience only).
  infoQuery = string(":info:date") + wordPartSep + string("*");
  completer.resetTimersAndCounters();
  string tmp2 = completer.getTopContinuationOfQuery(infoQuery);
  for (size_t i = 0; i < tmp2.size(); ++i)
    if (tmp2[i] == '_') tmp2[i] = ' ';
  cout << "* date of index construction is " << tmp2 << endl;
  infoQuery = string(":info:name") + wordPartSep + string("*");
  completer.resetTimersAndCounters();
  string tmp3 = completer.getTopContinuationOfQuery(infoQuery);
  for (size_t i = 0; i < tmp3.size(); ++i)
    if (tmp3[i] == '_') tmp3[i] = ' ';
  cout << "* name of collection is \"" << tmp3 << "\"" << endl;

  // NEW(bast, 21-03-2017): if option --keep-in-history-queries specified, read
  // queries from file and add with History::addKeepInHistoryQuery.
  if (keepInHistoryQueriesFileName != "") {
    Vocabulary keepInHistoryQueries;
    readWordsFromFile(keepInHistoryQueriesFileName, keepInHistoryQueries,
                       "queries to always keep in history");
    for (size_t i = 0; i < keepInHistoryQueries.size(); i++) {
      string query = keepInHistoryQueries[i];
      history.addKeepInHistoryQuery(query);
    }
  }

  // NEW(bast, 19-03-2017): if option --warm-history-queries specified, read
  // queries from file and execute them.
  if (warmHistoryQueriesFileName != "") {
    Vocabulary warmHistoryQueries;
    readWordsFromFile(warmHistoryQueriesFileName, warmHistoryQueries,
                       "queries for history warming");
    Timer timer;
    off_t queryTimeoutCopy = queryTimeout;
    queryTimeout = std::numeric_limits<off_t>::max();
    for (size_t i = 0; i < warmHistoryQueries.size(); i++) {
      string query = warmHistoryQueries[i];
      cout << "* warming history with query \"" << query << "\" ... " << flush;
      timer.start();
      string tmp = completer.getTopContinuationOfQuery(query);
      timer.stop();
      cout << "done in " << timer << endl;
      // cout << "done, top result = \"" << tmp << "\"" << endl;
    }
    queryTimeout = queryTimeoutCopy;
  }

  /*
   * NEW 15Nov12 (baumgari): Vector which holds all attributes, which should
   * be printed as a list within json, even if there is just one element.
   * E.g. trees = [ "oak" ] instead of trees = "oak". In the index, there is
   * a special word ":info:multiple <field>" for each such field.
   */
  infoQuery = string(":info:multiple") + wordPartSep + string("*");
  completer.resetTimersAndCounters();
  multipleAttributes
    = completer.getAllContinuationsOfQuery(infoQuery);
  if (!multipleAttributes.empty())
  {
    cout << "* attributes which should always be handled as list within json: ";
    for (size_t i = 0; i < multipleAttributes.size(); i++)
      cout << multipleAttributes[i] + " ";
    cout << endl;
  }

  /* NEW 20Feb14 (baumgari): String which specifies the formats of the info
   * fields. */
  infoQuery = string(":info:field-formats") + wordPartSep + string("*");
  completer.resetTimersAndCounters();
  infoFieldsFormats = completer.getTopContinuationOfQuery(infoQuery);
  if (infoFieldsFormats.empty())
    cout << "* No info field output format is specified. Default is xml." << endl;
  else
    cout << "* The format of the info fields are (J = json, X = xml): "
         << infoFieldsFormats << endl;

  /* NEW 05Mar14 (baumgari): String which specifies the info delimiter, if
   * defined in the index. */
  infoQuery = string(":info:field-delimiter") + wordPartSep + string("*");
  completer.resetTimersAndCounters();
  infoQuery = completer.getTopContinuationOfQuery(infoQuery);
  if (infoQuery.empty())
    cout << "* No info field delimiter specified." << endl;
  else
  {
    infoDelim = infoQuery[0];
    cout << "* The specified info field delimiter is " 
         << infoDelim << endl;
  }

  // establish socket
  this->port = port;
  createSocket(port, log_file, killServerIfRunning);

  cout << "* maximum size of history: " << commaStr(historyMaxSizeInBytes)
      << " bytes, " << commaStr(historyMaxNofQueries) << " queries" << endl;

  // NEW 10Nov11 (Hannah): Optionally read custom scores.
  if (readCustomScores == true)
  {
    // TODO(bast): This pointer will never be freed again. Not a big deal,
    // because it's only 4 or 8 bytes. But not clean. On the other hand, the
    // server currently runs forever until it is killed, anyway.
    globalCustomScorer = new CustomScorer(index._vocabulary);
    string customScoresFileName = baseName + ".custom-scores";
    globalCustomScorer->readCustomScores(customScoresFileName);
  }

  // NEW 18Oct13 (baumgari): Compute c in c * n * log n. This is done to
  // estimate the sorting time, which can take very long. Sorting is in O(n log
  // n).
  // To get this factor, we need to create several lists, since it changes
  // Create a list of size N, which shall be sorted.OA

  cout << "* Measuring average time for sorting one million unsigned integers" << flush;
  srand (time(NULL));

  size_t n = 250000;
  Vector<unsigned int> A[4];
  for (size_t i = 0; i < 4; ++i) A[i].resize(n);
  
  Timer initialSortTimer;
  parallelSortTimePerMillionIntegers = 0;
  unsigned int numOfRuns = 10;
  for (size_t x = 0; x < numOfRuns; x++)
  {
    for (size_t i = 0; i < 4; ++i)
      for (size_t j = 0; j < n; j++)
        // worst case
        A[i][j] = (n - j) * (i + 1);
        // random case
        // A[i][j] = rand() % (4 * n);
    initialSortTimer.cont();
    A[0].sortParallel(A[1], A[2], A[3]);
    initialSortTimer.stop();
  }
  parallelSortTimePerMillionIntegers = initialSortTimer.usecs() * 1.0 / numOfRuns;
  cout << " ... done: " << parallelSortTimePerMillionIntegers << " usecs." << endl;
}

//! ESTABLISH SOCKET CONNECTION
template<class Completer, class Index>
void CompletionServer<Completer, Index>::createSocket(int port,
    FILE* log_file, bool killServerIfRunning)
{
  // check for server running on given port
  string pidFileName = getPidFileName(port);
  if (access(pidFileName.c_str(), F_OK) == 0)
  {
    cout << LOCK << "! found pid file \"" << pidFileName << "\" ... " << flush;
    if (!killServerIfRunning)
    {
      cout << "exiting" << endl << endl << UNLOCK;
      exit(1);
    }
    else
    {
      cout << "trying to kill process" << endl << UNLOCK;
      killServer(pidFileName);
    }
  }

  // Create socket and set options.
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
  acceptor.open(endpoint.protocol());
  acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.set_option(boost::asio::ip::tcp::acceptor::keep_alive(true));
 
  boost::system::error_code createSocketError;

  const unsigned int NOF_TRIALS_CREATE_SOCKET = 19;
  unsigned int usecsToSleepBetweenTrials[NOF_TRIALS_CREATE_SOCKET - 1] = {100
      * 1000, 500 * 1000, 500 * 1000, 1000 * 1000, 1000 * 1000, 1000 * 1000, 5
      * 1000 * 1000, 5 * 1000 * 1000, 5 * 1000 * 1000, 5 * 1000 * 1000, 5
      * 1000 * 1000, 5 * 1000 * 1000, 5 * 1000 * 1000, 5 * 1000 * 1000, 5
      * 1000 * 1000, 5 * 1000 * 1000, 5 * 1000 * 1000, 5 * 1000 * 1000};
  // Try to bind socket.
  for (unsigned int i = 0; i < NOF_TRIALS_CREATE_SOCKET; ++i)
  {
    acceptor.bind(endpoint, createSocketError);
    if (createSocketError == boost::system::errc::success) break;
    cout << "! bind socket to port \"" << port << "\" failed (" << 
        createSocketError.message() << ")" << flush;
    if (i >= NOF_TRIALS_CREATE_SOCKET - 1) 
    {
      ostringstream os;
      os << "tried " << NOF_TRIALS_CREATE_SOCKET << " times";
      CS_THROW(Exception::COULD_NOT_CREATE_SOCKET, os.str());
    }
    unsigned int usecs = usecsToSleepBetweenTrials[i];
    cerr << ", sleeping for " << setiosflags(ios::fixed) << setprecision(1)
        << (usecs / 1000000.0) << " seconds and then trying again" << " ("
        << (i + 1) << "/" << NOF_TRIALS_CREATE_SOCKET << ")" << endl;
    usleep(usecs);
  }
  
  acceptor.listen(boost::asio::socket_base::max_connections, createSocketError);
  if (createSocketError != boost::system::errc::success) {
    ostringstream os;
    os << "! cannot listen to socket (" << createSocketError.message() << ")";
    CS_THROW(Exception::COULD_NOT_CREATE_SOCKET, os.str());
  }

  // write pid file
  ofstream pidFile(pidFileName.c_str());
  if (!pidFile.is_open())
  {
    perror("! open pid file");
    exit(1);
  }
  pidFile << getpid();
  pidFile.close();
  cout << "* process id (" << getpid() << ") written to file \""
      << pidFileName << "\"" << endl;
}

//! DESTRUCTOR (TODO: not really needed, is it?)
template<class Completer, class Index>
CompletionServer<Completer, Index>::~CompletionServer()
{
  cout << endl << endl
      << " ********** COMPLETIONSERVER DESTRUCTED ***********" << endl << endl;
}

// WAIT FOR REQUEST AND PROCESS IN OWN THREAD (forever until killed)
template<class Completer, class Index>
void CompletionServer<Completer, Index>::waitForRequestsAndProcess()
{
  query_id = 0;
  ConcurrentLog log;
  cout << endl;
  while (true)
  {
    // Wait for new query
    query_id++;
    log.setId(query_id);
    log << endl;
    log << "---------- WAITING FOR QUERY AT PORT \"" << port << "\" ... " << endl;
    boost::asio::ip::tcp::socket client(io_service);
    acceptor.accept(client);
    time_t NOW = time(NULL);
    string currentTime = ctime(&NOW);
    size_t pos = currentTime.find_first_of("\r\n");
    if (pos != string::npos) currentTime.erase(pos);
    log << "new query from " << client.remote_endpoint().address().to_string()
        << " at " << currentTime << endl;

    // Launch thread that will process query (connection must be closed in that function).
    processRequestLaunchThread(client, query_id);
  } // end of while(true)

} // end of waitForRequestsAndProcess()


//! Create and run the thread that will process the request from the given client.
template<class Completer, class Index>
void CompletionServer<Completer, Index>::processRequestLaunchThread(
    boost::asio::ip::tcp::socket& client, int query_id)
{
  pthread_struct* args = new pthread_struct();
  args->query_id = query_id;
  args->index = &index;
  args->history = &history;
  args->fuzzySearcher = _fuzzySearcher;
  args->client = &client;
  args->io_service = &io_service;

  ConcurrentLog log;
  log.setId(query_id);
  log << IF_VERBOSITY_HIGH << "* creating new thread " << endl;

#ifdef DEBUG_PTHREAD_CREATE_TIME
  if (runMultithreaded == false) CompletionServer<Completer, Index>::threadCreateTimer.start();
#endif

  pthread_t pthread_id;
  int ret = pthread_create(&pthread_id, NULL,
      CompletionServer<Completer, Index>::processRequestThreadFunction,
      (void*) args);
  if (ret != 0) throw Exception(Exception::COULD_NOT_CREATE_THREAD,
      strerror(errno));

  // NEW 11Sep07 (Holger): optionally wait for thread to finish
  if (runMultithreaded == false) pthread_join(pthread_id, NULL); // dared to remove this 08Jun07
} // end of processRequestInOwnThread


//! Thread function called by processRequestLaunchThread (required to be of type void*(*)(void*) by pthreads)
template<class Completer, class Index>
void* CompletionServer<Completer, Index>::processRequestThreadFunction(
    void* arguments)
{
  // Keep track of number of threads
  pthread_mutex_lock(&process_query_thread_mutex);
  ++nofRunningProcessorThreads;
  pthread_mutex_unlock(&process_query_thread_mutex);
#ifndef NDEBUG
  cout << "In processRequest: currently " << nofRunningProcessorThreads << " threads running" << endl;
#endif

#ifndef NDEBUG
  // Measure time for thread creation (as of 26Dec07 on dork: around 50 microseconds)
  if (runMultithreaded == false) CompletionServer<Completer, Index>::threadCreateTimer.stop();
#endif

  // Get arguments and free the memory for the struct pointer
  assert(arguments != NULL);
  pthread_struct args = *(struct pthread_struct*) arguments;

  // Each thread gets its own completer, with own buffers, timers, etc.
  // (but they all share the same Index and History)
  Completer completer(args.index, args.history, args.fuzzySearcher);
  //Completer completer(*(Index*)(((void**)(args))[2]), *(History*)(((void**)(args))[3]));
  completer.log.setId(args.query_id);

  // Process request in separate function with proper exception handling (can't
  // do exception handling with thread function directly, since it is only
  // called implicitly via pthread_create)   NEW 26Dec07 (Holger)
  try
  {
    CompletionServer<Completer, Index>::processRequest(*args.client, *args.io_service, completer);
  }
  // Report any communication errors that occurred during processing (errors in
  // the actual computation are dealt with inside processRequest already)
  catch (Exception& e)
  {
    completer.log << "! " << e.getFullErrorMessage() << endl;
    // currently assuming that all \c throws come before the connection to
    // the client has been closed, so make sure to close it here
    args.client->close();
  }
  catch (exception& e)
  {
    completer.log << "! STD EXCEPTION: " << e.what() << endl;
  }
  catch (...)
  {
    completer.log << "! UNKNOWN EXCEPTION (should never happen)" << endl;
  }

  // End thread
  assert(nofRunningProcessorThreads > 0);
  pthread_mutex_lock(&process_query_thread_mutex);
  --nofRunningProcessorThreads;
  pthread_mutex_unlock(&process_query_thread_mutex);
  return NULL;

} // end of processRequestThreadFunction


//! The function where the query is actually processed (called from processRequestThreadFunction)
/*
 *   1. Get request string from client
 *   2. Parse request string (extract parameters and actual query)
 *   3. Process query
 *   4. Get excerpts and build result string for client
 *   5. Send result to client
 *   6. Show some statistics
 *   7. History maintenance (cut down if it has become too large)
 */
template<class Completer, class Index>
void CompletionServer<Completer, Index>::processRequest(boost::asio::ip::tcp::socket& client,
    boost::asio::io_service& io_service,
    Completer& completer)
{
  ConcurrentLog& log = completer.log;
  completer.resetTimersAndCounters(); // TODO: not really necessary for newly created completer, is it?
  completer.threadTimer.start();
  QueryParameters queryParameters;
  Query query;
  // Note: result is a pointer, so that processQuery can redirect to an already existing result in the history
  QueryResult* result = NULL;
  QueryResult resultOnError;
  bool errorOccurred = false;
  string errorMessage;
  string requestString = "";
  string resultString = "";
  string postRequestContent;
  // Protocol (GET, HEAD, POST)   NEW 17Oct13 (baumgari)
  int completionServerProtocol = CS_PROTOCOL_UNDEFINED;

#ifdef DEBUG_PTHREAD_CREATE_TIME
  if (runMultithreaded == false) log << IF_VERBOSITY_HIGH
      << "! time passed between pthread_create and call of thread function: "
      << CompletionServer<Completer, Index>::threadCreateTimer << endl;
#endif
  try
  {
    //
    // 1. Get request string from client
    //
    log << "reading from client ... " << flush;
    completer.receiveQueryTimer.start();
    char* buf = new char[MAX_POST_QUERY_LENGTH + 1];
    unsigned int postRequestContentLength = 0;
    size_t endOfHeader = string::npos;
    boost::asio::deadline_timer timeout(io_service);
    boost::system::error_code read_error;
    boost::system::error_code wait_error;

    // Receive data.
    client.async_read_some(boost::asio::buffer(buf, MAX_POST_QUERY_LENGTH + 1),
      boost::bind(&read_callback, boost::ref(buf), boost::ref(completionServerProtocol),
                  boost::ref(completer), boost::ref(timeout), read_error,
                  boost::asio::placeholders::bytes_transferred));
    timeout.expires_from_now(boost::posix_time::milliseconds(1000));
    timeout.async_wait(boost::bind(&wait_callback, boost::ref(client), wait_error));

    // Will block until async callbacks are finished
    io_service.run();
    io_service.reset();
  
    // Interpret received data.
    requestString = buf;
    delete[] buf;

    if (requestString.empty()) {
      ostringstream os;
      os << "Empty request string";
      completer.statusCode = 0;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }

    if (completionServerProtocol == CS_PROTOCOL_UNDEFINED)
    {
      if (requestString.compare(0, 3, "GET") == 0)
        completionServerProtocol = CS_PROTOCOL_HTTP_GET;
      else if (requestString.compare(0, 4, "POST") == 0)
        completionServerProtocol = CS_PROTOCOL_HTTP_POST;
      else if (requestString.compare(0, 4, "HEAD") == 0)
       completionServerProtocol = CS_PROTOCOL_HTTP_HEAD;
    } 
    if (endOfHeader == string::npos) endOfHeader = requestString.find("\r\n\r\n");

    // POST request procedure.
    // Read out content length, if not already done.
    if (completionServerProtocol == CS_PROTOCOL_HTTP_POST
        && endOfHeader != string::npos && postRequestContentLength == 0)
    {
      // NEW 04Oct13 (baumgari): Big data is often requested by using the
      // header "Expect: 100-continue". This means, that the client needs a
      // confirmation before sending the data itself and sends afterwards
      // the data without response body using the same connection.
      // This can be initiated by answering with a response header with
      // status code 100 Continue. I tried that by using curl. After
      // answering it did nothing else. Don't know how to initiate it or if
      // the server just "oversights" the following request.
      // TODO(baumgari): Take a deeper look into that. Until that we
      // respond with the correct error status code.
      string expectHeader = "Expect: 100-continue";
      if (requestString.find(expectHeader) != string::npos)
      {
        completer.receiveQueryTimer.stop();
        completer.statusCode = 417;
        ostringstream os;
        os << "Header \"Expect: 100-continue\": data is sent in the next request." << endl;
        CS_THROW(Exception::BAD_REQUEST, os.str());
      }

      // Get content length header.
      string contentLengthHeader = "Content-Length: ";
      size_t pos = requestString.find(contentLengthHeader);
      if (pos != string::npos) pos += contentLengthHeader.size();
      else
      {
        ostringstream os;
        os << "Content length header in post request is missing: "
           << requestString << ".";
        completer.statusCode = 411;
        CS_THROW(Exception::BAD_REQUEST, os.str());
      }

      // Get content length.
      string strContentLength = requestString.substr(pos, pos + requestString.find('\r'));
      postRequestContentLength = atoi(strContentLength.c_str());

      // Check content length.
      if (postRequestContentLength != 0) {
	postRequestContent = "?" + requestString.substr(endOfHeader + 4, postRequestContentLength);
      }
      else
      {
        ostringstream os;
        os << "Content length is zero or could not be converted to an integer: \""
           << strContentLength << "\"";
        completer.statusCode = 400;
        CS_THROW(Exception::BAD_REQUEST, os.str());
      }
    }
  
    completer.receiveQueryTimer.stop();

    // Read buffer -> request string
    if (completionServerProtocol == CS_PROTOCOL_HTTP_POST && postRequestContent.empty())
    {
      ostringstream os;
      os << "Could not extract content of POST request:\n"
         << requestString << endl;
      completer.statusCode = 400;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }
      
    unsigned int requestStringLength = requestString.length();
    size_t pos = requestString.find('\r');
    if (pos != string::npos) requestString.erase(pos) + "...";
    log << "received \"" << requestString
        << (postRequestContent.empty() ? ""  : 
	   ("\" with query parameters: \"" + postRequestContent))
        << "\" (" << requestStringLength << " characters) in " 
        << completer.receiveQueryTimer 
	<< endl;

    //
    // 2. Parse request string (extract parameters and actual prefix completion query) 
    //

    log << IF_VERBOSITY_HIGH << "! completion server protocol: HTTP" << endl;
    pos = requestString.find(" HTTP/1.");
    if (pos == string::npos)
    {
      ostringstream os;
      os << "missing \"HTTP/1.x\" in HTTP request \"" << requestString
          << "\"";
      completer.statusCode = 400;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }
    requestString.erase(pos);

    // Erase request method string ("GET ", "POST ", "HEAD ").
    switch (completionServerProtocol) 
    {
      case CS_PROTOCOL_HTTP_GET:
        requestString.erase(0, 4);
        break;
      case CS_PROTOCOL_HTTP_POST:
        requestString.erase(0, 5);
        break;
      case CS_PROTOCOL_HTTP_HEAD:
        requestString.erase(0, 5);
        break;
      case CS_PROTOCOL_UNDEFINED:
      {
        ostringstream os;
        os << "missing \"GET\" or \"HEAD\" or \"POST\" in HTTP request \"" << requestString << "\"";
        completer.statusCode = 400;
        CS_THROW(Exception::BAD_REQUEST, os.str());
        break;
      }
      default:
      {
        ostringstream os;
        os << "invalid protocol: " << completionServerProtocol;
        completer.statusCode = 400;
        CS_THROW(Exception::OTHER, os.str());
        break;
      }
    }


    // This was a post request.
    if (!postRequestContent.empty()) requestString = postRequestContent;
    // NEW 01Nov12 (baumgari) I: Move query type checking to QueryParameters.
    // Erase "/" but not the question mark - relevant for parsing.
    else if (requestString.compare(0, 2, "/?") == 0) requestString.erase(0, 1);
    // NEW 25Mar12 (baumgari): In case /? is missing, we want to be able to
    // return the requested file, e.g. <host>:<port>/index.html, if a
    // document root is specified.
    else
    {
      // NEW 13Mar09 (Hannah): url decode the request string (+ -> space, %20 -> space, etc.)
      requestString = decodeHexNumbers(requestString);
 
      ostringstream os;
      if (!documentRoot.empty())
      {
        if (requestString == "/") requestString = "/index.html";
        string encodingAsString = encoding == Encoding::UTF8 ? "UTF-8"
                                              : "ISO-8859-1";

        // The request should not contain "..", since this could allow moving
        // within the filesystem and doing bad stuff.
        if (requestString.find("..") != string::npos)
        {
          os << "Request string \"" << requestString << "\" contained \"..\". Blocked for safety reasons.";
          completer.statusCode = 400;
          CS_THROW(Exception::BAD_REQUEST, os.str());
        }

        string path = documentRoot + requestString;
        ifstream f;
        f.open(path.c_str());
        // File does not exist or rights are missing.
        if (!f.is_open())
        {
          os << "Can't open \"" << path << "\".";
          completer.statusCode = 404;
          CS_THROW(Exception::BAD_REQUEST, os.str());
        }

        // Copy the content of the file and return it.
        while (!f.eof())
        {
          string buffer;
          getline(f, buffer);
          resultString += buffer + "\n";
        }
        f.close();

        // Add header
        string contentType = "text/plain";
        string extension = requestString.substr(requestString.rfind('.') + 1);
        if      (extension == "css")   contentType = "text/css";
        else if (extension == "js")    contentType = "text/javascript";
        else if (extension == "pdf")   contentType = "application/pdf";
        else if (extension == "html"   
              || extension == "htm"
              || extension == "shtml") contentType = "text/html";

        os << "HTTP/1.1 200 OK\r\n"
           << "Content-Length: " << resultString.size() << "\r\n"
           << "Connection: close\r\n" 
           << "Content-Type: " << contentType
           << "; charset=" << encodingAsString << "\r\n";
        if (corsEnabled)
          os << "Access-Control-Allow-Origin: *\r\n";
        os << "\r\n" << resultString;
        resultString = os.str();

        log << "* NEW: Returning specified file: \"" << path << "\""
            << " ... extension was: \"" << extension << "\"" << endl;
        sendResult(resultString.length(), resultString, client, completer, log);
        return;
      }
      else
      {
        os << "missing parameter start \"/?\" in HTTP request \""
           << requestString << "\""
           << " and document root for file returning is not specified.";
        completer.statusCode = 400;
        CS_THROW(Exception::BAD_REQUEST, os.str());
      }
    }

    // NEW 01Nov12 (baumgari) II: Get queryParameters.
    if (queryParameters.extractFromRequestStringHttp(requestString) == false) {
      ostringstream os;
      os << "! ERROR in QueryParameters.cpp.";
      completer.statusCode = 400;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }

    requestString = queryParameters.query;
    // NEW 13Mar09 (Hannah): url decode the request string (+ -> space, %20 -> space, etc.)
    requestString = decodeHexNumbers(requestString);
 
    if (queryParameters.queryType == QueryParameters::NORMAL)
    {
      //queryParameters.extractFromRequestStringHttp(requestString);
      // NEW 26Aug11 (Hannah+Ina): Make sure that number of hits and number of
      // completions are 
      // TODO(bast): add command-line options for these thresholds.
      // TODO(bast): better have a threshold on the size of the result object
      // (for example, also a result with relatively few hits could be very
      // large, e.g. if the excerpts are large).
      unsigned int maxNofTopHitsToCompute = 10000;
      unsigned int maxNofHitsToSend = 1000;
      unsigned int maxNofTopCompletionsToCompute = 1000;
      unsigned int maxNofCompletionsToSend = 1000;
      // NOTE: maxNofTopCompletionsToCompute must not be too big, because the
      // completion string is computed for each of them, not only for the top
      // nofCompletionsToSend, see Completions::set.
      struct
      {
        unsigned int& nof;
        unsigned int max;
        const char* description;
      }
      maxNofs[4] =
      { 
        { queryParameters.nofTopHitsToCompute,
          maxNofTopHitsToCompute, "nofTopHitsToCompute" },
        { queryParameters.nofHitsToSend,
          maxNofHitsToSend, "nofHitsToSend" },
        { queryParameters.nofTopCompletionsToCompute,
          maxNofTopCompletionsToCompute, "nofTopCompletionsToCompute" },
        { queryParameters.nofCompletionsToSend,
          maxNofCompletionsToSend, "nofCompletionsToSend" }
      };
      for (int i = 0; i < 4; i++)
      {
        if (maxNofs[i].nof > maxNofs[i].max)
        {
          log << RED << "WARNING: " << maxNofs[i].description << " too high ("
              << maxNofs[i].nof << "), setting to " << maxNofs[i].max
              << RESET << endl;
          maxNofs[i].nof = maxNofs[i].max;
        }
      }
    }

    // NEW (baumgari) 05Apr13:
    // Instead of :, there is a variable now, containing the word separator.
    // Nevertheless many user use : and some scripts might depend on that. So
    // we allow the colon and convert it to our new word separator. Moreover
    // we need to check the completions, since we might need to replace the
    // word separator by the :.
    requestString = replaceWordPartSeparatorFrontendToBackend(requestString);
    // NEW(bartsch, 17Mai11):
    if (cleanupQueryBeforeProcessing == true)
    {
      // Cleaning request string to avoid heavy server load by receiving
      // requests like: 
      // bi*^~ *^~ *^~ gehäuse*^~ 
      // 10..25.40cm.*^~ Digitus*^~
      // NEW (baumgari, 04Apr13): extracted a method. Method name is not
      // chosen properly, since it's like the attribute holding the option.
      // Open for new suggestions.
      requestString = cleanupQuery(requestString, log);
    }

    if (queryParameters.queryType == QueryParameters::NORMAL)
    {
      log << IF_VERBOSITY_HIGH << "* parameters are: "
          << queryParameters << endl;
      log << IF_VERBOSITY_HIGH << "* remaining part of query is \""
          << requestString << "\"" << endl;
      query.setQueryString(requestString);
      query.normalize();
      Query queryRewritten = query;
      queryRewritten.rewriteQuery();

      //
      // 3. Process prefix search and completion query
      // 
      //   NOTE: the query result is a *pointer* which will either point to an already
      //   existing object in the history, or to a newly allocated object 
      //
      //   TODO(bast): central processQuery method computes completions as
      //   strings and hits ads doc ids; would be more natural to compute word
      //   ids here
      //
      //   TODO(bast): it's a bit illogical that query is passed as an argument
      //   to processQuery, and queryParameters is not. Note that the only
      //   reason we have the setQuery here (NEW 29Jan10) is that we need to
      //   know the query string in computeTopCompletions when howToRankWords ==
      //   RANK_WORDS_BY_EDIT_DISTANCE.
      //

      completer.setQueryParameters(queryParameters);
      completer.setQuery(query);
      completer.processQuery(queryRewritten, result);
      CS_ASSERT(result != NULL);
    }
  }

  //
  // 4a. An exception occurred in one of 1., 2., or 3.
  //     -> send result with error message
  //

  catch (Exception& e)
  {
    errorOccurred = true;
    errorMessage = e.getFullErrorMessage();
    // If the message was empty, do not send something, since nothing was
    // requested.
    if (completer.statusCode == 0) {
      CS_RETHROW(e);
    }
  }
  catch (exception& e)
  {
    errorOccurred = true;
    errorMessage = string("STD EXCEPTION: ") + e.what();
  }
  catch (...)
  {
    errorOccurred = true;
    errorMessage = string("UNKNOWN EXCEPTION");
  }
 
  if (errorOccurred == true)
  {
    log << "! " << errorMessage << endl;
    result = &resultOnError;
    result->_status = QueryResult::ERROR;
    result->setErrorMessage(errorMessage);

    // NEW 07Aug13 (baumgari): Added default response header in case an
    // exception occured.
    vector<HitData> hits;
    resultString = buildResponseString(query, queryParameters, *result,
                                       completer, hits, log);
  }
  else
  {
    if (::logVerbosity >= LogVerbosity::HIGHEST) result->show();

    //
    // 4b. For empty query or query *, set the total number of completions / hits
    // to the number of words / documents in the collection
    //
    if (queryParameters.queryType == QueryParameters::NORMAL)
    {
      if ((query.empty() || query.getQueryString() == "*"))
      {
        result->nofTotalCompletions = completer._metaInfo->getNofWords();
        result->nofTotalHits = completer._metaInfo->getNofDocs();
      }
    }

    //
    // 4c. For /?exe query, execute user-defined command.
    //
    else if (queryParameters.queryType == QueryParameters::EXE)
    {
      log << "* exe query, argument is \"" << requestString << "\"" << endl;
      // int ret = system(NULL);
      // int ret = system("cmd.exe /C start http://www.spiegel.de");
      // int ret = system("cmd.exe /C start mailto:hannah@mpi-inf.mpg.de");
      if (exe_command == "")
      {
        errorMessage = "no system call, because exe_command is empty, specify via -X flag";
        completer.statusCode = 400;
        log << "! " << errorMessage << endl;
        result = &resultOnError;
        result->_status = QueryResult::ERROR;
        result->setErrorMessage(errorMessage);
      }
      else
      {
        ostringstream cmd;
        cmd << exe_command << requestString;
        log << "* system call is \"" << cmd.str() << "\"" << endl;
        int ret = system(cmd.str().c_str());
        log << "* system call executed, return value is " << ret << endl;
        completer.statusCode = 204;
      }
      vector<HitData> hits;
      resultString = buildResponseString(query, queryParameters, *result,
                                         completer, hits, log);
    }

    //
    // 5. Get excerpts and build result string for client
    //
    //   TODO: these are actually two separate steps and the code should reflect
    //   this
    //
    //   WARNING: the excerpt generation part is not yet thread-safe!!!
    //
    if (excerptsGenerator != NULL && result != NULL
        && queryParameters.queryType == QueryParameters::NORMAL)
    {
      vector<HitData> hits;
      completer.getExcerptsTimer.start();
      log << IF_VERBOSITY_HIGH << "* NEW: query rewritten for highlighting: \""
          << completer.getQueryRewrittenForHighlighting().getQueryString()
          << "\"" << endl;
      excerptsGenerator->getExcerpts(
          completer.getQueryRewrittenForHighlighting(), queryParameters,
          *result, hits);
      completer.getExcerptsTimer.stop();
      if (result->_status != QueryResult::ERROR) completer.statusCode = 200;
      resultString = buildResponseString(query, queryParameters, *result,
                                         completer, hits, log);
    }
  }

  //
  // 6. Send result to client
  //
  //   TODO: ignore SIGPIPE!!! (process gets SIGPIPE when client aborts during
  //   write, and probably also during read above)
  //
  sendResult(resultString.length(), resultString, client, completer, log);

  if (errorOccurred)
  {
    if (completer.isInHistoryConst(query))
    {
      completer.removeFromHistory(query);
    }
    return;
  }
  //
  // 7. History maintenance, NEW(Hannah, 18Aug11): now before show statistics.
  //

  completer.historyCleanUpTimer.start();
#ifndef NDEBUG
  log << "! Before clean-up and cut-down, history looks like this:" << endl;
  completer.history.show();
#endif

  // Check if result that was just computed is still in history; TODO: why?
  if (result != NULL)
  {
    if (!completer.isInHistoryConst(query)) log
        << " In CompletionServer.h: query " << query
        << " was already removed. Cannot set its status. " << endl;
    else
    {
    } 
  }

  // Clean up the history; TODO: what is that?
  pthread_mutex_lock(&process_query_thread_mutex);
  if (nofRunningProcessorThreads == 1)
  {
#ifndef NDEBUG
    log << "! Cleaning up history" << endl;
#endif
    completer.history->cleanUp();
    if (nofRunningProcessorThreads != 1)
    {
      log << "! WARNING: CLEANED HISTORY WHILE THREAD WAS CREATED" << endl;
    }
  }
  else
  {
    log << "! NOT cleaning up history, because other threads are running"
        << " (" << nofRunningProcessorThreads - 1 << ")" << endl;
  }
  pthread_mutex_unlock(&process_query_thread_mutex);

  // Check if history has become too large and if so, remove some old results
  log << "checking history size ... " << flush;
  pthread_mutex_lock(&process_query_thread_mutex);
  bool wasHistoryCutDown = false;
  if (nofRunningProcessorThreads == 1) wasHistoryCutDown
      = completer.history->cutToSizeAndNumber(historyMaxSizeInBytes,
          historyMaxNofQueries);
  completer.historyCleanUpTimer.stop();
  log << "done in " << completer.historyCleanUpTimer << flush;
  if (wasHistoryCutDown == true)
  {
    log << ", cut down to <= " << historyMaxSizeInBytes / (1024 * 1024)
        << " MB" << " / " << historyMaxNofQueries << " queries" << endl;
  }
  else
  {
    log << ", left history unchanged" << endl;
  }
  log << IF_VERBOSITY_HIGH << "! current history size: " << commaStr(
      completer.getSizeOfHistoryInBytes()) << " bytes"
      << "; number of queries: " << completer.getNofQueriesInHistory() << endl;
  pthread_mutex_unlock(&process_query_thread_mutex);

  #ifndef NDEBUG
  log << "! After clean-up & cut-down, history looks like this:" << endl;
  completer.history.show();
  #endif

  //
  // 8. Show some statistics, NEW(Hannah, 18Aug11): now after history cleanup.
  //

  completer.threadTimer.stop();
        
  if (queryParameters.queryType == QueryParameters::NORMAL)
  {
    if (true)
    {
      log << endl;
      completer.showOneLineSummary(log, query, result, completer.threadTimer.usecs());
      // log << endl;
    }
    if (showStatistics)
    {
      // NEW(Hannah, 18Aug11): show statistics only if time is more than 5
      // milliseconds.
      if (completer.threadTimer.msecs() > 5)
      {
        log << endl;
        completer.showStatistics(log, completer.threadTimer.usecs());
      }
    }
    // Commented out, because it's now shown in processComplexQuery.
    // if (showQueryResult && result != NULL) result->show();
  }
} // end of processRequest

// ____________________________________________________________________________
template<class Completer, class Index>
string CompletionServer<Completer, Index>
  ::cleanupQuery(const string& query, ConcurrentLog& log)
{
  string cleanedQuery;
  cleanedQuery.resize(query.size());
  size_t posCleanedQuery = 0;
  size_t posQuery = 0;

  // NEW (baumgari) 29Oct14: Removed ".", since there is a separator "..".
  // CHANGE(hagn, 03Aug11): add the '#' sign.
  // We ignore after characters in [ .|#] all occurrences of characters in
  // [*.^ |~#] until a character occurs that is not in [*.^ |~#]
  while (posQuery < query.size())
  {
    if (strchr(" |#", query[posQuery]) == NULL)
    {
      cleanedQuery[posCleanedQuery++] = query[posQuery++];
    }
    else
    {
      // Ignore after a character in  [ .|#] the characters in
      // [*.^ |~#] until a character occurs, that is not in [*.^ |~#]
      cleanedQuery[posCleanedQuery++] = query[posQuery++];
      while (posQuery < query.size())
      {
        if (strchr("*^ |~#", query[posQuery]) != NULL)
        {
          posQuery++;
        }
        else
        {
          cleanedQuery[posCleanedQuery++] = query[posQuery++];
          break;
        }
      }
    }
  }
  cleanedQuery = cleanedQuery.substr(0, posCleanedQuery);
  // NEW(hagn, 20Jul11):
  // When the order of *^~ is not correct, then correct it
  // When the order of *^, *~ or ^~ is not correct, then correct it
  string charsToCombine = "*^~";
  size_t pos;
  posCleanedQuery = 0;
  while (posCleanedQuery < cleanedQuery.size())
  {
    if ((pos = charsToCombine.find(cleanedQuery[posCleanedQuery++])) == string::npos)
    {
      charsToCombine = "*^~";
    }
    else
    {
      charsToCombine.replace(pos, 1, "");
      if (charsToCombine.length() == 0)
      {
        strncpy(&cleanedQuery[posCleanedQuery - 3], "*^~", 3);
        charsToCombine = "*^~";
      }
      else if (charsToCombine.length() == 1)
      {
        if (charsToCombine[0] == '*')
        {
          strncpy(&cleanedQuery[posCleanedQuery - 2], "^~", 2);
        }
        else if (charsToCombine[0] == '^')
        {
          strncpy(&cleanedQuery[posCleanedQuery - 2], "*~", 2);
        }
        else if (charsToCombine[0] == '~')
        {
          strncpy(&cleanedQuery[posCleanedQuery - 2], "*^", 2);
        }
      }
    }
  }
  cleanedQuery = cleanedQuery.substr(0, posCleanedQuery);

  if (cleanedQuery != query)
    log << "NEW: query \"" << query << "\" after cleaning: \"" << cleanedQuery << "\""
        << endl;
  return cleanedQuery;

}

//! Send given result to client.
template<class Completer, class Index>
void CompletionServer<Completer, Index>::sendResult(
    const unsigned int len, const string& resultString, boost::asio::ip::tcp::socket& client,
    const Completer& completer, ConcurrentLog& log)
{
  log << "sending result ... " << flush;
  completer.sendResultTimer.start();
  boost::system::error_code write_error;
  boost::asio::write(client, boost::asio::buffer(resultString),
                     boost::asio::transfer_all(), write_error);
  if (write_error) {
    ostringstream os;
    os << "sending result failed (" << write_error.message() << ")";
    CS_THROW(Exception::OTHER, os.str());
  }
  boost::system::error_code send_error;
  client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, send_error);
  if (send_error) {
    ostringstream os;
    os << "Shutting down socket failed (" << send_error.message() << ")";
    CS_THROW(Exception::OTHER, os.str());
  }
  client.close(send_error);
  if (send_error) {
    ostringstream os;
    os << "Closing socketfailed (" << send_error.message() << ")";
    CS_THROW(Exception::OTHER, os.str());
  }

  completer.sendResultTimer.stop();
  log << "done, sent " << commaStr(int(len)) << " bytes in "
      << completer.sendResultTimer << " ("
      << (completer.sendResultTimer.usecs() != 0 ? commaStr(
          int(len) / completer.sendResultTimer.usecs()) : "???")
      << " MB/sec)" << endl;
}

template<class Completer, class Index>
string CompletionServer<Completer, Index>::buildResponseString(
    const Query& query, const QueryParameters& queryParameters,
    const QueryResult& result, const Completer& completer,
    const vector<HitData>& hits, ConcurrentLog& log)
{
  // NEW (baumgari) 20Feb14: The default format of the response is now
  // set to the format of the info field within the docs file. This
  // information is stored in the variable infoFieldsFormats and interpreted
  // here.
  InfoFieldFormat infoFieldFormat = XML;
  if (queryParameters.titleIndex < infoFieldsFormats.size()) {
    char format = infoFieldsFormats[queryParameters.titleIndex];
    if (format == 'J') infoFieldFormat = JSON;
       else if (format == 'X') infoFieldFormat = XML;
  }

  // NEW (baumgari) 04Feb14: New option --info-field-format, which allows to
  // specify which format (right now xml or json) is used in the docs file.
  // Right now, we provide a valid json AND a valid xml output only, if the
  // info field is formatted as xml and not as json. Because of this we
  // deactivate the xml output, if json is specified as info field format.
  // TODO: Add a possibility to return both xml AND json if json is
  // specified as info field fomat.
  string resultString;
  if (infoFieldFormat == XML)
  {
    if (queryParameters.format == QueryParameters::XML)
      resultString =
        resultAsHttpResponse(query, queryParameters, result, completer,
            hits, log);
    // If the responseFormat is not xml, it has to be json or jsonp.
    else
    {   
      bool convertXmlToJson = true;
      resultString = resultAsJsonObject(query, queryParameters, result,
                                    completer, hits, log, convertXmlToJson);
    }
  }
  else if (infoFieldFormat == JSON)
  {
    resultString = resultAsJsonObject(query, queryParameters, result,
                                      completer, hits, log);
  }
  return resultString;
}

//! Format a query result + excerpts as a HTTP 1.0 response with xml-like body
template<class Completer, class Index>
string CompletionServer<Completer, Index>::resultAsHttpResponse(
    const Query& query, const QueryParameters& queryParameters,
    const QueryResult& result, const Completer& completer,
    const vector<HitData>& hits, ConcurrentLog& log)
{
  completer.buildResultStringTimer.start();
  string encodingAsString = encoding == Encoding::UTF8 ? "UTF-8"
      : "ISO-8859-1";
  ostringstream os;
  ostringstream os1;
  ostringstream os2;
  ostringstream os3;
  // In case api and server are using different word separators, replace them.
  string queryStrRewritten
    = replaceWordPartSeparatorBackendToFrontend(query.getQueryString());
  queryStrRewritten = addCdataTagIfNecessary(queryStrRewritten, false);
  // NEW 06Mar13 (baumgari): Query parts can be quoted now to ensure that the
  // quoted parts should not be parsed for separators. This is done to find
  // query containing special characters like Philip_S._Yu, which would be
  // splitted at the point elsewhise -> Philip_S"."_Yu is correct now.
  // This can lead to problems within the xml and json output, so the quotes
  // should be escaped here.
  for (size_t i = 0; i < queryStrRewritten.size(); i++)
  {
    if (queryStrRewritten[i] == '\"')
    {
      queryStrRewritten.replace(i, 1, "\\\"");
      i++;
    }
  }

  os1 << "<?xml version=\"1.0\" encoding=\"" << encodingAsString << "\"?>\r\n"
      << "<result>\r\n" << "<query id=\"" << log._id << "\">" << queryStrRewritten
      << "</query>\r\n";
  os1 << "<status code=\"" << completer.statusCode << "\">" 
      << getHTTPStatusMessage(completer.statusCode) << "</status>\r\n";
  unsigned int nc = queryParameters.nofCompletionsToSend;
  if (nc > result._topWordIds.size()) nc = result._topWordIds.size();
  // BUG(Hannah 25Jul11): Deliberately added + 1 to fool end2end test.
  // os3 << "<completions total=\"" << result.nofTotalCompletions + 1 << "\""
  os3 << "<completions total=\"" << result.nofTotalCompletions << "\""
      << " computed=\"" << result._topWordIds.size() << "\"" << " sent=\""
      << nc << "\">\r\n";

  CS_ASSERT_GE(result._topWordIds .size(), nc);
  CS_ASSERT_GE(result._topWordScores .size(), nc);
  CS_ASSERT_GE(result._topWordDocCounts.size(), nc);
  CS_ASSERT_GE(result._topWordOccCounts.size(), nc);
  CS_ASSERT_GE(result._topWordIds .size(), nc);
  for (unsigned int i = 0; i < nc; i++)
  {
    WordId wordId = result._topWordIds[i];
    string wordRewritten;
    if (wordId < (WordId) (completer._vocabulary->size()))
    {
      wordId = (*completer._vocabulary).mappedWordId(wordId);
      wordRewritten = replaceWordPartSeparatorBackendToFrontend((*completer._vocabulary)[wordId]);
      wordRewritten = addCdataTagIfNecessary(wordRewritten, false);
    }
    else
      wordRewritten = "[invalid word id]";
    os3 << "<c sc=\"" << result._topWordScores[i] << "\" " << "dc=\""
        << result._topWordDocCounts[i] << "\" " << "oc=\""
        << result._topWordOccCounts[i] << "\" " << "id=\""
        << result._topWordIds[i] << "\">" << wordRewritten << "</c>\r\n";
  }
  os3 << "</completions>\r\n";
  if (fuzzySearchEnabled)
  {
    size_t nofSuggestions = result._topFuzzySearchQuerySuggestions.size();
    size_t nofSuggSent = MIN(nc, nofSuggestions);
    os3 << "<suggestions computed=\"" << nofSuggestions << "\""
        << " sent=\"" << nofSuggSent
        << "\">\r\n";
    for (size_t i = 0; i < nofSuggSent; i++)
    {
      const string& suggestion = result._topFuzzySearchQuerySuggestions[i];
      double score = result._topFuzzySearchQuerySuggestionScores[i];
      os3 << "<s ss=\"" << score << "\">"
          << addCdataTagIfNecessary(suggestion, false)
          << "</s>\r\n";
    }
    os3 << "</suggestions>\r\n";
  }
  unsigned int f = queryParameters.firstHitToSend;
  os3 << "<hits total=\"" << result.nofTotalHits << "\"" << " computed=\""
      << result._topDocIds.size() << "\"" << " sent=\"" << hits.size() << "\""
      << " first=\"" << f << "\">\r\n";
  CS_ASSERT_GE(result._topDocIds.size(), hits.size());
  for (unsigned int i = 0; i < hits.size(); ++i)
  {
    // NEW 26Sep12 (baumgari): Changed <title>....</title> to <info>...</info>,
    // since title contains actually all information about the hit.
    os3 << "<hit score=\"" << hits[i].score << "\" " << "id=\""
        << hits[i].docId << "\">\r\n" << "<info>" << addCdataTagIfNecessary(
        hits[i].title, alreadyWellformedXml) << "</info>\r\n" << "<url>" << addCdataTagIfNecessary(
        hits[i].url, alreadyWellformedXml) << "</url>\r\n";
    for (unsigned int j = 0; j < hits[i].excerpts.size(); ++j)
      os3 << "<excerpt>" << addCdataTagIfNecessary(hits[i].excerpts[j], alreadyWellformedXml)
          << "</excerpt>\r\n";
    os3 << "</hit>\r\n";
  }
  os3 << "</hits>\r\n";
  if (result._status == QueryResult::ERROR && sendErrorDetailsToClient == true)
  {
    string msg = result.errorMessage;
    for (unsigned int i = 0; i < msg.length(); ++i)
      if (msg[i] == '\r' || msg[i] == '\n') msg[i] = ' ';
    os3 << "<completesearch-error-message>" << addCdataTagIfNecessary(msg, false) << "</completesearch-error-message>\r\n";
  }
  os3 << "</result>\r\n";

  completer.buildResultStringTimer.stop();
  double totalUsecs = completer.getTotalProcessingTimeInUsecs();
  // NEW (05Dec13) baumgari: The given time was totally wrong. We should use the
  // threadTimer instead.
  /* completer.processQueryTimer.usecs()
      + completer.getExcerptsTimer.usecs()
      + completer.buildResultStringTimer.usecs();*/
  os2 << "<time unit=\"msecs\">" << setiosflags(ios::fixed) << setprecision(2)
      << totalUsecs / 1000 << "</time>\r\n";
  string content = os1.str() + os2.str() + os3.str();

  int statusCode = (completer.statusCode == -1 ? 500 : completer.statusCode);
  os << "HTTP/1.1 " << statusCode << " " << getHTTPStatusMessage(statusCode) << "\r\n"
     << "Content-Length: " << content.size() << "\r\n"
     << "Connection: close\r\n"
     << "Content-Type: text/xml; charset=" << encodingAsString << "\r\n";
  if (corsEnabled)
    os << "Access-Control-Allow-Origin: *\r\n";
  os << "\r\n" << content;
  return os.str();
}

//! Format a query result + excerpts as a json obect 
template<class Completer, class Index>
string CompletionServer<Completer, Index>::resultAsJsonObject(
    const Query& query, const QueryParameters& queryParameters,
    const QueryResult& result, const Completer& completer,
    const vector<HitData>& hits, ConcurrentLog& log, bool convertXmlToJson)
{
  completer.buildResultStringTimer.start();

  string encodingAsString = encoding == Encoding::UTF8 ? "UTF-8"
   : "ISO-8859-1";
  ostringstream os;
  ostringstream os1;
  ostringstream os2;
  ostringstream os3;
  string openbrace = "{\r\n";
  string closebrace = "}\r\n";
  string closebracecomma = "},\r\n";

  XmlToJson x2j(multipleAttributes);

  // In case the api and server word part separators differ, we need to replace
  // them.
  string queryStrRewritten
    = replaceWordPartSeparatorBackendToFrontend(query.getQueryString());
  // NEW 06Mar13 (baumgari): Query parts can be quoted now to ensure that the
  // quoted parts should not be parsed for separators. This is done to find
  // query containing special characters like Philip_S._Yu, which would be
  // splitted at the point elsewhise -> Philip_S"."_Yu is correct now.
  // This can lead to problems within the xml and json output, so the quotes
  // should be escaped here.
  queryStrRewritten = x2j.escapeInvalidChars(queryStrRewritten); 

  /*os1 << "<?xml version=\"1.0\" encoding=\"" << encodingAsString << "\"?>\r\n"i*/
  if (queryParameters.format == QueryParameters::JSONP)
  {
    os1 << queryParameters.callback << "(\r\n";
  }
  os1 << openbrace << "\"result\":" << openbrace << "\"query\":\""
      << queryStrRewritten << "\",\r\n"
      << "\"status\":" << openbrace << "\"@code\":\"" << completer.statusCode
      << "\",\r\n" << "\"text\":\""
      << getHTTPStatusMessage(completer.statusCode) << "\"\r\n"
      << closebracecomma;

  unsigned int nc = queryParameters.nofCompletionsToSend;
  if (nc > result._topWordIds.size()) nc = result._topWordIds.size();

  os3 << "\"completions\":" << openbrace << "\"@total\":\""
      << result.nofTotalCompletions << "\",\r\n" << "\"@computed\":\""
      << result._topWordIds.size() << "\",\r\n" << "\"@sent\":\"" << nc
      << (nc > 0 ? "\",\r\n" : "\"\r\n");

  CS_ASSERT_GE(result._topWordIds .size(), nc);
  CS_ASSERT_GE(result._topWordScores .size(), nc);
  CS_ASSERT_GE(result._topWordDocCounts.size(), nc);
  CS_ASSERT_GE(result._topWordOccCounts.size(), nc);
  CS_ASSERT_GE(result._topWordIds .size(), nc);

  if (nc > 0) os3 << "\"c\":";
  if (nc > 1) os3 << "[\r\n";
  for (unsigned int i = 0; i < nc; i++)
  {
    WordId wordId = result._topWordIds[i];
    string wordRewritten;
    if (wordId < (WordId) (completer._vocabulary->size()))
    {
      // In case the api and server word part separators differ, we need to replace
      // them.
      wordId = (*completer._vocabulary).mappedWordId(wordId);
      wordRewritten = replaceWordPartSeparatorBackendToFrontend((*completer._vocabulary)[wordId]);
      wordRewritten = x2j.escapeInvalidChars(wordRewritten); 
    }
    else
      wordRewritten = "[invalid word id]";
    os3 << openbrace << "\"@sc\":\"" << result._topWordScores[i] << "\",\r\n"
        << "\"@dc\":\"" << result._topWordDocCounts[i] << "\",\r\n" << "\"@oc\":\""
        << result._topWordOccCounts[i] << "\",\r\n" << "\"@id\":\""
        << result._topWordIds[i] << "\",\r\n" << "\"text\":\"" << wordRewritten
        << "\"\r\n" << (i < (nc - 1) ? closebracecomma : closebrace);
  }
  if (nc > 1) os3 << "]\r\n";
  os3 << closebracecomma;
  unsigned int f = queryParameters.firstHitToSend;
  os3 << "\"hits\":" << openbrace << "\"@total\":\"" << result.nofTotalHits
      << "\",\r\n" << "\"@computed\":\"" << result._topDocIds.size() << "\",\r\n"
      << "\"@sent\":\"" << hits.size() << "\",\r\n" << "\"@first\":\"" << f << "\"";

  CS_ASSERT_GE(result._topDocIds.size(), hits.size());

  if (hits.size() > 0) os3 << ",\r\n\"hit\":" << "[";
  for (unsigned int i = 0; i < hits.size(); ++i)
  {
    // NEW 26Sep12 (baumgari): Changed title to info,
    // since title contains actually all information about the hit.
    os3 << openbrace << "\"@score\":\"" << hits[i].score << "\",\r\n"
        << "\"@id\":\"" << hits[i].docId << "\",\r\n" << "\"info\":"
        << (convertXmlToJson ? x2j.xmlToJson(hits[i].title) : hits[i].title) << ",\r\n"
        << "\"url\":\"" << hits[i].url
        << (hits[i].excerpts.size() > 0 ? "\",\r\n\"excerpt\":" : "\"\r\n");
    // NEW 02Sep18 (bast): Fixed formatting for excerpts (had several mistakes before).
	if (hits[i].excerpts.size() == 1)
	    os3 << "\"" << hits[i].excerpts[0] << "\"\r\n";
	else
	  for (unsigned int j = 0; j < hits[i].excerpts.size(); ++j)
        os3 << (j == 0 ? "[\r\n\"" : "\"") << hits[i].excerpts[j]
		       << (j + 1 < hits[i].excerpts.size() ? "\"," : "\"\r\n]")
			   << "\r\n";
    os3 << (i + 1 < hits.size() ? closebracecomma : closebrace);
  }
  if (hits.size() > 0) os3 << "]" << "\r\n";
  // Close brace for hits.
  os3 << closebrace;

  completer.buildResultStringTimer.stop();
  double totalUsecs = completer.getTotalProcessingTimeInUsecs();
  // NEW (05Dec13) baumgari: The given time was totally wrong. We should use the
  // threadTimer instead.
  /* completer.processQueryTimer.usecs()
      + completer.getExcerptsTimer.usecs()
      + completer.buildResultStringTimer.usecs();*/
  os2 << "\"time\":" << openbrace << "\"@unit\":\"msecs\",\r\n" << "\"text\":\""
      << setiosflags(ios::fixed) << setprecision(2) << totalUsecs / 1000
      << "\"\r\n" << closebracecomma;

  if (result._status == QueryResult::ERROR && sendErrorDetailsToClient == true)
  {
    string msg;
    for (unsigned int i = 0; i < result.errorMessage.length(); ++i) {
      if (result.errorMessage[i] == '\r' || result.errorMessage[i] == '\n') msg += " ";
      else if (msg[i] == '\"') msg += "\\\"";
      else msg += result.errorMessage[i];
    }
    os3 << ",\"completesearch-error-message\": \"" << msg << "\"\r\n";
  }
  os3 << closebrace << closebrace;
  if (queryParameters.format == QueryParameters::JSONP) os3 << ")\r\n";
  string content = os1.str() + os2.str() + os3.str();

  int statusCode = (completer.statusCode == -1 ? 500 : completer.statusCode);
  os << "HTTP/1.1 " << statusCode << " " << getHTTPStatusMessage(statusCode) << "\r\n"
     << "Content-Length: " << content.size() << "\r\n"
     << "Connection: close\r\n";

  if (queryParameters.format == QueryParameters::JSONP)
    os << "Content-Type: application/javascript; charset=" << encodingAsString << "\r\n";
  else
    os << "Content-Type: application/json; charset=" << encodingAsString << "\r\n";
  
  if (corsEnabled)
    os << "Access-Control-Allow-Origin: *\r\n";

  os << "\r\n" << content;
  return os.str();
}

//! Called by boost::asio::async_read_some(...)
template<class Completer, class Index>
void CompletionServer<Completer, Index>::read_callback(const char* buffer, int& completionServerProtocol,
                                                       Completer& completer,
                                                       boost::asio::deadline_timer& timeout,
                                                       const boost::system::error_code& error,
                                                       std::size_t bytesTransferred)
{
  if (error || !bytesTransferred)
  {
    if (error) 
    {
      ostringstream os;
      os << "An error occured while retrieving the request: \""
         << error.message() << "\"";
      completer.statusCode = 400;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }
    // No data was read!
    return;
  }
  
  // Read request type.
  if (completionServerProtocol == CS_PROTOCOL_UNDEFINED)
  {
    if (strncmp(buffer, "GET", 3) == 0)
      completionServerProtocol = CS_PROTOCOL_HTTP_GET;
    else if (strncmp(buffer, "POST", 4) == 0)
      completionServerProtocol = CS_PROTOCOL_HTTP_POST;
    else if (strncmp(buffer, "HEAD", 4) == 0)
     completionServerProtocol = CS_PROTOCOL_HTTP_HEAD;
  }
  // Check for maximal request length.
  if (completionServerProtocol == CS_PROTOCOL_HTTP_POST)
  {
    if (bytesTransferred >= MAX_POST_QUERY_LENGTH)
    {
      ostringstream os;
      os << "Content length exceeds the limit \""
         << MAX_POST_QUERY_LENGTH << "\"";
      completer.statusCode = 414;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }
  }
  else
  {
    if (bytesTransferred >= MAX_QUERY_LENGTH)
    {
      completer.receiveQueryTimer.stop();
      completer.statusCode = 414;
      ostringstream os;
      os << "string too long: " << bytesTransferred << " bytes, max is "
         << MAX_QUERY_LENGTH;
      CS_THROW(Exception::BAD_REQUEST, os.str());
    }
  }
  // will cause wait_callback to fire with an error
  timeout.cancel();
}

//! Called by boost::asio::async_wait(...)
template<class Completer, class Index>
void CompletionServer<Completer, Index>::wait_callback(boost::asio::ip::tcp::socket& client,
                                                       const boost::system::error_code& error)
{
  // Data was read and this timeout was canceled
  if (error) return;
  // will cause read_callback to fire with an error
  client.cancel();
}

//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template class CompletionServer<HybCompleter<WITH_SCORES + WITH_POS
    + WITH_DUPS> , HYBIndex> ;
#ifdef COMPILE_INV
template class CompletionServer<INVCompleter<WITH_SCORES + WITH_POS + WITH_DUPS>, INVIndex>;
#endif

