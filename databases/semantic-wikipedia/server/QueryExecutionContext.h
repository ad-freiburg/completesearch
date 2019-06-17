// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_QUERYEXECUTIONCONTEXT_H_
#define SEMANTIC_WIKIPEDIA_SERVER_QUERYEXECUTIONCONTEXT_H_

#include <string>
#include "../codebase/semantic-wikipedia-utils/LRUCache.h"
#include "./IntermediateQueryResult.h"
#include "./Engine.h"
#include "./Index.h"

using std::string;

namespace ad_semsearch
{
typedef ad_utility::LRUCache<string, IntermediateQueryResult> Cache;

//! Execution context for queries. Holds references to
//! index and engine, may implement caching.
class QueryExecutionContext
{
  public:

    QueryExecutionContext(const Index& index, const Engine& engine)
    : _index(index), _engine(engine), _cache(NOF_SUBTREES_TO_CACHE)
    {
    }

    IntermediateQueryResult* getCachedResultForQueryTree(
        const string& queryAsString)
    {
//      Cache::iterator itt = _cache.find(queryAsString);
//      IntermediateQueryResult* res;
//      if (itt == _cache.end())
//      {
//        res = new IntermediateQueryResult();
//        _cache[queryAsString] = res;
//
//        return res;
//      }
//      else
//      {
//        res = itt->second;
//      }
//      return res;
      return &_cache[queryAsString];
    }

    const Engine& getEngine() const
    {
      return _engine;
    }

    const Index& getIndex() const
    {
      return _index;
    }

  private:

    const Index& _index;
    const Engine& _engine;
    Cache _cache;
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_QUERYEXECUTIONCONTEXT_H_
