// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TOOLS_EVALUATION_SEMANTICWIKIPEDIAQUERY_H_
#define SEMANTIC_WIKIPEDIA_TOOLS_EVALUATION_SEMANTICWIKIPEDIAQUERY_H_

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <sstream>
#include "./SemanticWikipediaQuery.h"

using std::string;
using std::vector;
using std::ostringstream;

// Class used in the evaluation part.
// Takes care of communication with the semantic wikipedia search,
// creation of query strings according to desired options and
// especially parsing the XML result into result sets that
// can be used for further evaluation.
//
// Those options that are supported by the completesearch engine
// can also be set for this query:
// h  : number of hits
// c  : number of completions (of last query word, if you put a * behind it)
// en : number of excerpts per hit
// er : size of excerpt
//
// The follwoing options are currently NOT supported:
// f  : send hits starting from this one
// rd : how to rank the documents (0 = by score, 1 = by doc id, 2 = by word id,
//      append a or d for ascending or descending)
// rw : how to rank the words (0 = by score, 1 = by doc count,
//      2 = by occurrence count, 3 = by word id,
//      4 = by doc id, append a or d as above)
// s  : how to aggregate scores (expert option, ignore for the moment)
class SemanticWikipediaQuery
{
  public:
  // Creates a query from the given string. The string should correspond to the
  // "q"-part of actual queries.
  // Other parameters can be specified using the setter methods if the
  // default settings should not be desired in some case
  SemanticWikipediaQuery(const string& query, const string& hostIP,
      const int& port);

  // Destructor
  virtual ~SemanticWikipediaQuery();

  // Gets the string representation of this query as it is performed.
  string getFullQueryString();

  // Executes this query and returns a corresponding list of entities.
  // Entities are assumed to be returned as the last part of completions.
  void queryForEntities(vector<string>* resultSet);

  // Sets the number of completions that should be returned for the query.
  // Usually, all completions should always be relevant for evaluation queries.
  void setNumberOfCompletionsParam(int paramValue)
  {
    _completionsNum = paramValue;
  }

  // Sets the number of hits that should be returned for the query.
  // DEFAULT: 0
  void setNumberOfHitsParam(int paramValue)
  {
    _hitsNum = paramValue;
  }

  // Sets the number of excerpts that should be returned for the query.
  // DEFAULT: 0
  void setNumberOfExcerptsParam(int paramValue)
  {
    _excerptsNum = paramValue;
  }

  // Sets the size of excerpts that should be returned for the query.
  // DEFAULT: 0
  void setSizeOfExcerptsParam(int paramValue)
  {
    _excerptsSize = paramValue;
  }

  // Methods used when parsing the XML
  void doHandleCharacter(const char* s, int len);
  void doHandleStart(const char *el, const char **attr);
  void doHandleEnd(const char *el);

  void appendToHttpResponseOs(char* res)
  {
    _httpResponseOs << string(res);
  }

  private:
  int _port;
  string _host;
  string _query;
  int _completionsNum;
  int _hitsNum;
  int _excerptsNum;
  int _excerptsSize;
  char* _current;
  bool _cTagOpen;
  int _curPos;
  vector<string> _parserResultSet;
  ostringstream _httpResponseOs;

  FRIEND_TEST(SemanticWikipediaQueryTest, processResult);
  void processResult(const string& resultXml, vector<string>* resultSet);
};

#endif  // SEMANTIC_WIKIPEDIA_TOOLS_EVALUATION_SEMANTICWIKIPEDIAQUERY_H_
