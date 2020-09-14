// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <expat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include <ostream>
#include <string>
#include <vector>
#include "./SemanticWikipediaQuery.h"

using std::string;
using std::vector;
using std::ostringstream;

// Buffer size.
// Should be sufficient for all kinds of words.
const int BUF_SIZE = 1024;
const size_t RECIEVE_BUFFER_SIZE = 1024 * 1024 * 10;

// ____________________________________________________________________________
void start(void *data, const char *el, const char **attr)
{
  SemanticWikipediaQuery* q =
      reinterpret_cast<SemanticWikipediaQuery*> (data);
  q->doHandleStart(el, attr);
}

// ____________________________________________________________________________
void end(void *data, const char *el)
{
  SemanticWikipediaQuery* q =
      reinterpret_cast<SemanticWikipediaQuery*> (data);
  q->doHandleEnd(el);
}

// ____________________________________________________________________________
void handleChar(void *data, const XML_Char *s, int len)
{
  SemanticWikipediaQuery* q =
      reinterpret_cast<SemanticWikipediaQuery*> (data);
  q->doHandleCharacter(s, len);
}
// ____________________________________________________________________________
size_t handle_response(void *ptr, size_t size, size_t nmemb, void *data)
{
  SemanticWikipediaQuery* q =
      reinterpret_cast<SemanticWikipediaQuery*> (data);
  size_t bytes = nmemb * size;
  char* res = reinterpret_cast<char*> (ptr);
  res[bytes] = 0;
  q->appendToHttpResponseOs(res);
  return bytes;
}

// _____________________________________________________________________________
SemanticWikipediaQuery::SemanticWikipediaQuery(const string& query,
    const string& hostIP, const int& port)
{
  _query = query;
  _host = hostIP;
  _port = port;
  _completionsNum = 100000;
  _hitsNum = 0;
  _excerptsNum = 0;
  _excerptsSize = 0;
  _current = new char[BUF_SIZE];
  _cTagOpen = false;
}
// _____________________________________________________________________________
SemanticWikipediaQuery::~SemanticWikipediaQuery()
{
  delete[] _current;
}
// _____________________________________________________________________________
string SemanticWikipediaQuery::getFullQueryString()
{
  ostringstream os;
  os << "http://" << _host << ":" << _port << "/?q=" << _query << "&h="
      << _hitsNum << "&c=" << _completionsNum << "&en=" << _excerptsNum
      << "&er=" << _excerptsSize;
  return os.str();
}
// _____________________________________________________________________________
void SemanticWikipediaQuery::queryForEntities(vector<string>* resultSet)
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (curl)
  {
    string response;
    curl_easy_setopt(curl, CURLOPT_URL, this->getFullQueryString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handle_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    res = curl_easy_perform(curl);
    if (res != 0)
    {
      std::cerr << "Failed to query " << this->getFullQueryString() << ":"
          << std::endl << curl_easy_strerror(res) << std::endl << std::flush;
      exit(1);
    }

    curl_easy_cleanup(curl);

    // Process the result.
    processResult(_httpResponseOs.str(), resultSet);
    _httpResponseOs.str("");
  }
}
// _____________________________________________________________________________
void SemanticWikipediaQuery::processResult(const string& resultXml, vector<
    string>* resultSet)
{
  // Create Parser and register handlers.
  XML_Parser xmlParser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xmlParser, start, end);
  XML_SetCharacterDataHandler(xmlParser, handleChar);
  XML_SetUserData(xmlParser, reinterpret_cast<void*> (this));

  int ret = XML_Parse(xmlParser, resultXml.c_str(), resultXml.length(), true);
  if (ret == XML_STATUS_ERROR)
  {
    std::cerr << "ERROR parsing XML! \n";
    if (resultXml.size() == 0)
    {
      std::cerr << "HTTP response was empty. Please ensure that host"
          << " and port are correct and that the server is in fact running!\n";
    }
    exit(1);
  }
  *resultSet = _parserResultSet;
}
// _____________________________________________________________________________
void SemanticWikipediaQuery::doHandleStart(const char* el, const char** attr)
{
  if (string(el) == "c")
  {
    _cTagOpen = true;
    _curPos = 0;
  }
  if (string(el) == "completions")
  {
    int total = 0;
    int sent = -1;

    for (int i = 0; attr[i]; i += 2)
    {
      if (string(attr[i]) == "total")
      {
        total = atoi(attr[i + 1]);
      }
      if (string(attr[i]) == "sent")
      {
        sent = atoi(attr[i + 1]);
      }
    }
    // Deactivated this check to support top-k retrieval
    // assert(total == sent);
  }
}
// _____________________________________________________________________________
void SemanticWikipediaQuery::doHandleEnd(const char* el)
{
  if (string(el) == "c")
  {
    _current[_curPos] = 0;
    _parserResultSet.push_back(string(_current));
    _cTagOpen = false;
  }
}
// _____________________________________________________________________________
void SemanticWikipediaQuery::doHandleCharacter(const char* s, int len)
{
  if (_cTagOpen)
  {
    for (int i = 0; i < len; ++i)
    {
      if (s[i] == ':')
      {
        _curPos = 0;
      } else
      {
        _current[_curPos] = s[i];
        _curPos++;
        assert(_curPos < BUF_SIZE);
      }
    }
  }
}
