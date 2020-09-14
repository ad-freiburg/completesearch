// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_SERVER_H_
#define SEMANTIC_WIKIPEDIA_SERVER_SERVER_H_

#include <string>
#include <vector>

#include "../codebase/semantic-wikipedia-utils/Socket.h"
#include "../codebase/semantic-wikipedia-utils/Timer.h"

#include "./Query.h"
#include "./QueryExecutionContext.h"

using std::string;
using std::vector;

using ad_utility::Socket;

namespace ad_semsearch
{
//! The HTTP Sever used.
class Server
{
  public:
    explicit Server(const int port)
    : _serverSocket(), _port(port), _index(), _engine(), _initialized(false)
    {
    }

    // Initialize the server.
    void initialize(const string& ontologyBaseName,
        const vector<string>& fullTextBaseNames);

    //! Loop, wait for requests and trigger processing.
    void run();

  private:
    Socket _serverSocket;
    int _port;
    Index _index;
    Engine _engine;
    bool _initialized;

    void process(Socket* client, QueryExecutionContext* qec) const;

    void parseHttpRequest(const string& request, Query* result) const;

    void createHttpResponse(const string& content, string* response) const;

    void composeResponseXml(const Query& query, QueryResult& result,
        string* xml) const;

    void composeResponseXml(const Query& query, const Exception& exception,
        string* xml) const;

    mutable ad_utility::Timer _requestProcessingTimer;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_SERVER_H_
