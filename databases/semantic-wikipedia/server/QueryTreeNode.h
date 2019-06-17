// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_QUERYTREENODE_H_
#define SEMANTIC_WIKIPEDIA_SERVER_QUERYTREENODE_H_

#include <set>
#include <algorithm>
#include <string>
#include "./ExecutableResultProvider.h"
#include "./Disjunct.h"
#include "./IntermediateQueryResult.h"

using std::set;
using std::string;

namespace ad_semsearch
{
//! QueryTreeNode. Relates to a variable rooting a query subtree.
//! Each node is always composed of 1-n disjuncts, each of them
//! being a conjunction of QueryTriples.
class QueryTreeNode: public ExecutableResultProvider
{
  public:
    QueryTreeNode() :
        ExecutableResultProvider()
    {
    }

    explicit QueryTreeNode(QueryExecutionContext* executionContext) :
        ExecutableResultProvider(executionContext)
    {
    }

    //! Adds another disjunct.
    void addDisjunct(const Disjunct& disjunct)
    {
      _disjuncts.insert(disjunct);
    }

    string asString() const;

    //! Collects hits for one disjunct who's result contains
    //! the entity.
    void getHitsForEntity(Id entityId, HitList* result) const;

  private:
    void computeResult(IntermediateQueryResult* result) const;
    set<Disjunct, ad_utility::AsStringComparatorLt> _disjuncts;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_QUERYTREENODE_H_
