// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_QUERY_H_
#define SEMANTIC_WIKIPEDIA_SERVER_QUERY_H_

#include <string>
#include <vector>
#include "./QueryTreeNode.h"
#include "./QueryParameters.h"
#include "./QueryResult.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! Query as a whole.
//! Does not represent a subtree but instead a real query for
//! boxes and or hits. May or may not have a QueryTree
//! associated.
class Query
{
  public:
    //! Default ctor. An execution context has to be set later on!
    Query()
    : _originalRequest(), _parameters(), _executionContext(NULL),
      _prefix(), _queryTree(NULL)
    {
    }

    //! Typical ctor.
    Query(QueryExecutionContext* executionContext, const string& triples,
        const string& rootNode, const string& prefix,
        const QueryParameters& parameters)
    : _originalRequest(triples + ", query: " + rootNode),
      _parameters(parameters), _executionContext(executionContext),
      _prefix(_prefix), _queryTree(NULL)
    {
      constructQueryTreeFromTriplesString(triples, rootNode);
    }


    //! Copy ctor.
    Query(const Query& other)
    : _originalRequest(other._originalRequest),
      _parameters(other._parameters),
      _executionContext(other._executionContext),
      _prefix(other._prefix),
      _queryTree(new QueryTreeNode(*other._queryTree))
    {
    }

    //! Assignment operator.
    Query& operator=(const Query& rhs)
    {
      delete _queryTree;
      _originalRequest = rhs._originalRequest;
      _parameters = rhs._parameters;
      _executionContext = rhs._executionContext;
      _queryTree = new QueryTreeNode(*rhs._queryTree);
      return *this;
    }

    //! Destructor.
    virtual ~Query()
    {
      // No ownership over _executionContext, do not delete!
      delete _queryTree;
    }

    //! Set the executionContext
    void setExecutionContext(QueryExecutionContext* executionContext)
    {
      _executionContext = executionContext;
    }

    //! Set the QueryParameters.
    void setQueryParameters(const QueryParameters& parameters)
    {
      _parameters = parameters;
    }

    //! Get the QueryParameters.
    const QueryParameters& getQueryParameters() const
    {
      return _parameters;
    }

    //! Get the QueryParameters as non-const reference.
    QueryParameters& getQueryParameters()
    {
      return _parameters;
    }

    //! Constructs a query tree for the triples string matching
    //! the query language for our semantic search.
    void constructQueryTreeFromTriplesString(const string& triples,
        const string& rootNode);

    //! Get the internal, unique string representation that identifies queries.
    string asString() const;

    const string& getPrefix() const
    {
      return _prefix;
    }

    void setPrefix(const string& prefix)
    {
      _prefix = prefix;
    }

    const IntermediateQueryResult& getResultForQueryTree() const
    {
      return _queryTree->getResult();
    }

    const QueryExecutionContext* getExecutionContext() const
    {
      return _executionContext;
    }

    const Index& getIndex() const
    {
      return _executionContext->getIndex();
    }


    const Engine& getEngine() const
    {
      return _executionContext->getEngine();
    }

    string getOriginalRequest() const
    {
      return _originalRequest + ", prefix: " + _prefix;
    }

    //! Creates a query result. This uses pre-computed results
    //! of the query tree and various subtrees or triggers computation.
    //! On top of that it always triggers ranking and the retrieval and
    //! creation of a Hitlist.
    void createQueryResult(QueryResult* result) const;

    void createQueryResultForQueryWithoutTriples(QueryResult* result) const;

  private:
    string _originalRequest;
    QueryParameters _parameters;
    QueryExecutionContext* _executionContext;
    string _prefix;
    // Queries without query trees are possible
    // -> pure prefix / full-text search.
    // indicated by _queryTree == NULL
    QueryTreeNode* _queryTree;

    struct ParsedQueryTriple
    {
        string _source;
        string _relationName;
        vector<string> _destVars;
        vector<string> _destWords;
    };

    void fillClasses(QueryResult* result) const;

    void fillInstances(const EntityList& subtreeResult,
        QueryResult* result) const;
    void fillWords(const EntityList& subtreeResult, QueryResult* result) const;
    void fillRelations(const EntityList& subtreeResult,
        QueryResult* result) const;

    void fillHits(const EntityList& subtreeResult, QueryResult* result) const;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_QUERY_H_
