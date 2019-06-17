// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <vector>
#include <sstream>

#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "../codebase/semantic-wikipedia-utils/Conversions.h"
#include "./Server.h"

namespace ad_semsearch
{
// _____________________________________________________________________________
void Server::initialize(const string& ontologyBaseName,
    const vector<string>& fullTextBaseNames)
{
  LOG(INFO) << "Initializing server..." << std::endl;

  // Init the index.
  _index.registerOntologyIndex(ontologyBaseName);
  for (size_t i = 0; i < fullTextBaseNames.size(); ++i)
  {
    _index.registerFulltextIndex(fullTextBaseNames[i], true);
  }
  _index.initInMemoryRelations();

  // Init the server socket.
  bool ret = _serverSocket.create() && _serverSocket.bind(_port)
      && _serverSocket.listen();
  if (!ret)
  {
    LOG(ERROR)
    << "Failed to create socket on port " << _port << "." << std::endl;
    exit(1);
  }

  // Set flag.
  _initialized = true;
  LOG(INFO) << "Done initializing server." << std::endl;
}

// _____________________________________________________________________________
void Server::run()
{
  if (!_initialized)
  {
    LOG(ERROR) << "Cannot start an uninitialized server!" << std::endl;
    exit(1);
  }
  // For now, only use one QueryExecutionContext at all time.
  // This may be changed for some implementations of multi threading.
  // Cache(s) are associated with this execution context.
  QueryExecutionContext qec(_index, _engine);

  // Loop and wait for queries. Run forever, for now.
  while (true)
  {
    // Wait for new query
    LOG(INFO)
        << "---------- WAITING FOR QUERY AT PORT \"" << _port << "\" ... "
        << std::endl;

    ad_utility::Socket client;
    bool success = _serverSocket.acceptClient(&client);
    if (!success)
    {
      LOG(ERROR) << "Socket error while trying to accept client" << std::endl;
      continue;
    }
    // TODO(buchholb): Spawn a new thread here / use a ThreadPool / etc.
    LOG(INFO) << "Incoming connection, processing..." << std::endl;
    process(&client, &qec);
  }
}
// _____________________________________________________________________________
void Server::process(Socket* client, QueryExecutionContext* qec) const
{
  string request;
  string responseXml;
  client->recieve(&request);
  Query query;
  query.setExecutionContext(qec);
  try
  {
    parseHttpRequest(request, &query);
    QueryResult queryResult;
    query.createQueryResult(&queryResult);
    composeResponseXml(query, queryResult, &responseXml);
  }
  catch(const Exception& e)
  {
    composeResponseXml(query, e, &responseXml);
  }
  string httpResponse;
  createHttpResponse(responseXml, &httpResponse);
  client->send(httpResponse);
}
// _____________________________________________________________________________
void Server::parseHttpRequest(const string& httpRequest, Query* query) const
{
  LOG(DEBUG) << "Parsing HTTP Request." << endl;
  _requestProcessingTimer.start();
  // Parse the HTTP Request.
  string request;
  size_t indexOfGET = httpRequest.find("GET");
  size_t indexOfHTTP = httpRequest.find("HTTP");

  if (indexOfGET == httpRequest.npos || indexOfHTTP == httpRequest.npos)
  {
    AD_THROW(Exception::BAD_REQUEST, "Invalid request. "
    "Only supporting proper HTTP GET requests!");
  }

  decodeUrl(
      httpRequest.substr(indexOfGET + 3, indexOfHTTP - (indexOfGET + 3)),
      &request);

  ad_utility::HashMap<string, string> paramValueMap;
  size_t index = request.find("?");
  if (index == request.npos)
  {
    AD_THROW(Exception::BAD_REQUEST, "Invalid request. "
    "At least two parameters are required for meaningful queries!");
  }
  size_t next = request.find('&', index + 1);
  if (next == request.npos)
  {
    AD_THROW(Exception::BAD_REQUEST, "Invalid request. "
    "At least two parameters are required for meaningful queries!");
  }
  while (next != request.npos)
  {
    size_t posOfEq = request.find('=', index + 1);
    if (posOfEq == request.npos)
    {
      AD_THROW(Exception::BAD_REQUEST,
          "Parameter without \"=\" in HTTP Request.");
    }
    string param = ad_utility::getLowercase(
        request.substr(index + 1, posOfEq - (index + 1)));
    string value = request.substr(posOfEq + 1, next - (posOfEq + 1));
    if (paramValueMap.count(param) > 0)
    {
      AD_THROW(Exception::BAD_REQUEST, "Duplicate HTTP parameter: " + param);
    }
    paramValueMap[param] = value;
    index = next;
    next = request.find('&', index + 1);
  }
  size_t posOfEq = request.find('=', index + 1);
  if (posOfEq == request.npos)
  {
    AD_THROW(Exception::BAD_REQUEST,
        "Parameter without \"=\" in HTTP Request.");
  }
  string param = ad_utility::getLowercase(
      request.substr(index + 1, posOfEq - (index + 1)));
  string value = request.substr(posOfEq + 1,
      request.size() - 1 - (posOfEq + 1));
  if (paramValueMap.count(param) > 0)
  {
    AD_THROW(Exception::BAD_REQUEST, "Duplicate HTTP parameter.");
  }
  paramValueMap[param] = value;

  // Construct a Query object from the parsed request.
  typedef ad_utility::HashMap<string, string>::const_iterator MapIter;
  MapIter it = paramValueMap.find("prefix");
  bool hasPrefix = false;
  if (it != paramValueMap.end())
  {
    query->setPrefix(it->second);
    hasPrefix = (it->second.size() > 0);
  }

  it = paramValueMap.find("s");
  MapIter itt = paramValueMap.find("query");

  if (it == paramValueMap.end() || it->second == "")
  {
    if (!hasPrefix)
    {
      AD_THROW(Exception::BAD_REQUEST,
          "Expected at least one non-empty attribute \"s\" or \"prefix\"");
    }
  } else
  {
    if (itt == paramValueMap.end() && it->second.size() > 0)
    {
      AD_THROW(Exception::BAD_REQUEST,
          "Expected attribute \"query\" for non-empty triples string");
    }
    else
    {
      query->constructQueryTreeFromTriplesString(it->second, itt->second);
    }
  }

  query->getQueryParameters().constructFromParamMap(paramValueMap);
  LOG(DEBUG) << "Done parsing HTTP Request." << endl;
}
// _____________________________________________________________________________
void Server::createHttpResponse(
    const string& content, string* response) const
{
  std::ostringstream os;
  os << "HTTP/1.0 200 OK\r\n" << "Content-Length: " << content.size() << "\r\n"
      << "Connection: close\r\n" << "Content-Type: text/xml; charset="
      << "UTF-8" << "\r\n" << "\r\n" << content;
  *response = os.str();
}
// _____________________________________________________________________________
void Server::composeResponseXml(const Query& query, QueryResult& result,
    string* xml) const
{
  std::ostringstream resultOs;
  string resultString;
  result.asXml(&resultString);
  resultOs << resultString << "\r\n</result>\r\n";

  _requestProcessingTimer.stop();
  std::ostringstream os;
  os << "<?xml version=\"1.0\" encoding=\"" << "UTF-8" << "\"?>\r\n"
      << "<result>\r\n" << "<time>" << _requestProcessingTimer.msecs()
      << "msecs</time>\r\n" << "<query>" << "<![CDATA["
      << query.getOriginalRequest() << "]]>" << "</query>\r\n";
  os << "<status code=\"" << "0" << "\">" << "OK" << "</status>\r\n";


  *xml = os.str() + resultOs.str();
}
// _____________________________________________________________________________
void Server::composeResponseXml(const Query& query, const Exception& exception,
    string* xml) const
{
  std::ostringstream os;
  _requestProcessingTimer.stop();

  os << "<?xml version=\"1.0\" encoding=\"" << "UTF-8" << "\"?>\r\n"
      << "<result>\r\n" << "<time>" << _requestProcessingTimer.msecs()
      << "msecs</time>\r\n" << "<query>" << "<![CDATA["
      << query.getOriginalRequest() << "]]>" << "</query>\r\n";
  os << "<status code=\"" << "1" << "\">" << "ERROR" << "</status>\r\n";

  os << "<error>" << "<![CDATA[" << "Exception-Error-Message: ";
  string msg = exception.getFullErrorMessage();
  for (unsigned int i = 0; i < msg.length(); ++i)
  {
    if (msg[i] == '\r' || msg[i] == '\n')
    {
      os << ' ';
    }
    else
    {
      os << msg[i];
    }
  };
  os << "]]></error>\r\n" << "</result>\r\n";
  *xml = os.str();
}
}
