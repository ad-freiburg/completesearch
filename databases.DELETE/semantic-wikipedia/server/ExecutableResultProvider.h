// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_EXECUTABLERESULTPROVIDER_H_
#define SEMANTIC_WIKIPEDIA_SERVER_EXECUTABLERESULTPROVIDER_H_

#include <string>
#include "./QueryExecutionContext.h"
#include "./IntermediateQueryResult.h"

#include "../codebase/semantic-wikipedia-utils/Exception.h"

using std::string;

namespace ad_semsearch
{
//! Abstract superclass for all query elements  that can haven
//! an intermediate query result.
class ExecutableResultProvider
{
  public:

    //! Default Constructor.
    ExecutableResultProvider()
    : _executionContext(NULL)
    {
    }

    //! Typical Constructor.
    explicit ExecutableResultProvider(QueryExecutionContext* executionContext)
    : _executionContext(executionContext)
    {
    }

    //! Destructor.
    virtual ~ExecutableResultProvider()
    {
      // Do NOT delete _executionContext, since
      // there is no ownership.
    }

    //! Get the result for the subtree rooted at this element.
    //! Use existing results if they are already available, otherwise
    //! trigger computation.
    const IntermediateQueryResult& getResult() const
    {
      IntermediateQueryResult* result =
          _executionContext->getCachedResultForQueryTree(asString());
      if (result->_status != IntermediateQueryResult::FINISHED)
      {
        computeResult(result);
      }
      AD_CHECK_EQ(IntermediateQueryResult::FINISHED, result->_status);
      return *result;
    }

    //! Get hits for a certain entity.
    //! Gets one hit for that particular entity (if possible)
    //! from each element in the subtree. This may include changing the
    //! entity on the go. Example:
    //! AE is a scientist that occurs-with relativty
    //! who born in a city that occurs-with "Münster".
    //! At the top the entity will be Albert Einstein, deeper in the tree
    //! it'll be Ulm during only one run of recursively building the
    //! hits group.
    virtual void getHitsForEntity(Id entityId, HitList* result) const = 0;

    //! Set the QueryExecutionContext for this particular element.
    void setQueryExecutionContext(QueryExecutionContext* executionContext)
    {
      _executionContext = executionContext;
    }

    const Index& getIndex() const
    {
      return _executionContext->getIndex();
    }

    const Engine& getEngine() const
    {
      return _executionContext->getEngine();
    }

    //! Get a unique, not ambiguous string representation for a subtree.
    //! This should possible act like an ID for each subtree.
    virtual string asString() const = 0;

  protected:

    QueryExecutionContext* getExecutionContext() const
    {
      return _executionContext;
    }

    //! The QueryExecutionContext for this particular element.
    //! No ownership.
    QueryExecutionContext* _executionContext;

  private:
    //! Compute the result of the query-subtree rooted at this element..
    //! Computes both, an EntityList and a HitList.
    virtual void computeResult(IntermediateQueryResult* result) const = 0;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_EXECUTABLERESULTPROVIDER_H_
