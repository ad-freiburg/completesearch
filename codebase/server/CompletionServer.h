#ifndef __COMPLETION_SERVER_H__
#define __COMPLETION_SERVER_H__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <netinet/in.h> /* for sockets */
#include <arpa/inet.h>
#include <signal.h> /* for signal handling */
#include <unistd.h> /* for sleep */
#include <fcntl.h>
#include <pthread.h> /* don't forget to link with -lpthread; for a great tutorial, see
                        http://yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html */
#include "Globals.h"
#include "History.h"
#include "QueryParameters.h"
#include "ExcerptsGenerator.h"
#include "Timer.h"
#include "../fuzzysearch/FuzzySearcher.h"

// GLOBALS (implemented in CompletionServer.cpp, used in constructor below as well as in main)
extern pthread_mutex_t excerpts_generation;
string signalName(int signal);
extern string getPidFileName(int port);
extern void killServer(string pidFileName);

enum
{
  CS_PROTOCOL_UNDEFINED,
  CS_PROTOCOL_HTTP_GET,
  CS_PROTOCOL_HTTP_HEAD,
  CS_PROTOCOL_HTTP_POST
};

enum InfoFieldFormat
{
  XML,
  JSON
};

extern int completionServerProtocol;
extern bool sendErrorDetailsToClient;

//! Class that provides the main server loop: listen to query requests on a certain port and process them
/*!
 *   -# Read vocabulary file and open index file (in constructor CompletionServer())
 *   -# Create socket (in method createSocket())
 *   -# Start server loop (in method waitForRequestsAndProcess())
 *   -# Upon query request:
 *   -# Create own thread
 *   -# Read query
 *   -# Process query (via HYBCompleter / CompleterBase)
 *   -# Get excerpts for top-ranked documents
 *   -# Send result string
 *   -# Close connection and finish thread
 */
template<class Completer, class Index>
class CompletionServer
{
  public:

    //! Port at which the server listens for queries
    int port;

    /*
     //! Protocol types   NEW 13Nov07
     enum Protocol { OLD, HTTP };

     //! Protocol (OLD or HTTP)   NEW 13Nov07 (Holger)
     static Protocol protocol;
     */

#ifdef DEBUG_PTHREAD_CREATE_TIME
    //! Timer for thread creation overhead (only measured when multithreading is off)
    static Timer threadCreateTimer;
#endif

    //! Constructor; reads vocabulary, opens index file, and creates socket.
    /*!
     *    TODO: Wouldn't it make more sense to create the index before, and pass
     *    it by reference here?
     */
    CompletionServer(const string& indexFileName, //!< index file name
        const string& vocabularyFileName, //!< vocabulary file name
        unsigned char mode, //!< the mode of the completer (its template parameter); TODO: needed where?
        int port, //!< where to listen for reqeusts
        FILE* log_file, //!< TODO: still needed?
        bool restartServerIfRunning //!< if not true and a server is already running, exit
        );

    //! Destructor; doesn't do anything except printing a message.
    ~CompletionServer();

    //! Create server socket.
    void createSocket(int port, FILE* log_file, bool killServerIfRunning =
        true);

    //! Main server loop: Wait for requests and process each in its own thread.
    void waitForRequestsAndProcess();

    //! Create and run the thread that will process the request from the given client.
    void processRequestLaunchThread(boost::asio::ip::tcp::socket& client, int query_id);

    //! Thread function called by processRequestLaunchThread (required to be of type void*(*)(void*) by pthreads)
    /*!
     *    Note: method must be declared static, otherwise not of type
     *    \c void*(*)(void*) as required by \c pthread_create from the pthreads
     *    library
     */
    static void* processRequestThreadFunction(void* args);

    //! The function where the query is actually processed (called from processRequestThreadFunction)
    static void processRequest(boost::asio::ip::tcp::socket& client, boost::asio::io_service& io_service, Completer& completer);

  private:
    //! Maximal allowed request length of GET and HEAD requests.
    static const unsigned int MAX_QUERY_LENGTH = 2083;
    //! Maximal allowed request length of POST requests.
    static const unsigned int MAX_POST_QUERY_LENGTH = 2 * 1024 * 1024 - 1;

    // NEW (baumgari) Apr14: We used to use an array of pointers to pass all
    // necessary thread parameters. That was unclear. Changed that.
    //! Struct containing all parameters needed by a thread.
    struct pthread_struct {
      int query_id;
      Index* index;
      TimedHistory* history;
      FuzzySearch::FuzzySearcherBase* fuzzySearcher;
      boost::asio::ip::tcp::socket* client;
      boost::asio::io_service* io_service;
    };

    //! Provides core functionality for async network services.
    boost::asio::io_service io_service;
    //! Used for accepting new incoming socket connections.
    boost::asio::ip::tcp::acceptor acceptor;

    //! Query Id (will be 1,2,3,...)
    int query_id;

    //! The (one) Index used for processing queries
    /*!
     *  NEW 07Aug07 (Holger): I had to remove the const, otherwise I cannot
     *  instantiate a completer in the obvious way, as done now in the
     *  constructor of CompletionServer
     */
    Index index; //const Index index;

    //! Central history (caching mechanism) for all queries
    TimedHistory history;

    // The (one) object for processing fuzzy search queries.
    // MMM
    FuzzySearch::FuzzySearcherBase* _fuzzySearcher;

    //! Clean up query to avoid requests which can "paralyse" the server, since
    //! everything would be returned.
    static string cleanupQuery(const string& query, ConcurrentLog& log);

    //! Send result to client.
    static void sendResult(const unsigned int len, const string& resultString,
        boost::asio::ip::tcp::socket& client, const Completer& completer, ConcurrentLog& log);

    //! Build a response string using the function resultAsJsonObject and
    //! resultAsHttpResponse.
    static string buildResponseString(const Query& query,
        const QueryParameters& queryParameters, const QueryResult& result,
        const Completer& completer, const vector<HitData>& hits, ConcurrentLog& log);

    //! Format a query result + excerpts as a HTTP 1.0 response with xml-like body
    static string resultAsHttpResponse(const Query& query,
        const QueryParameters& queryParameters, const QueryResult& result,
        const Completer& completer, const vector<HitData>& hits, ConcurrentLog& log);

    //! Format a query result + excerpts as a json object
    static string resultAsJsonObject(const Query& query,
        const QueryParameters& queryParameters, const QueryResult& result,
        const Completer& completer, const vector<HitData>& hits, ConcurrentLog& log,
        bool convertXmlToJson = false);

    //! Callback for boost::asio::read_some(...)
    static void wait_callback(boost::asio::ip::tcp::socket& client,
                              const boost::system::error_code& error);
    
    //! Callback for boost::asio::wait(...)
    static void read_callback(const char* buffer, int& completionServerProtocotol,
                              Completer& completer, boost::asio::deadline_timer& timeout,
                              const boost::system::error_code& error, std::size_t bytes_transferred);


}; // end of class CompletionServer

#endif
