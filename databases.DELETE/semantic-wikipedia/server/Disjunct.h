// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_DISJUNCT_H_
#define SEMANTIC_WIKIPEDIA_SERVER_DISJUNCT_H_

#include <string>
#include <set>
#include <algorithm>
#include "./ExecutableResultProvider.h"
#include "./QueryTriple.h"
#include "./IntermediateQueryResult.h"

using std::string;
using std::set;

namespace ad_semsearch
{
//! Each disjunct is a conjunction of query triples.
//! Disjuncts are direct successors of nodes aka variables
//! in the query tree.
class Disjunct: public ExecutableResultProvider
{
  public:
    Disjunct() :
        ExecutableResultProvider()
    {
    }

    explicit Disjunct(QueryExecutionContext* executionContext) :
        ExecutableResultProvider(executionContext)
    {
    }

    //! Adds another triple to the conjunction.
    void addTriple(const OccursWithTriple& triple)
    {
      _owConjuncts.insert(triple);
    }
    //! Adds another triple to the conjunction.
    void addTriple(const IsATriple& triple)
    {
      _isaConjuncts.insert(triple);
    }
    //! Adds another triple to the conjunction.
    void addTriple(const RelationTriple& triple)
    {
      _relConjuncts.insert(triple);
    }

    //! Adds another triple to the conjunction.
    void addTriple(const EqualsTriple& triple)
    {
      _eqConjuncts.insert(triple);
    }

    //! Collects hits for all conjuncts.
    void getHitsForEntity(Id entityId, HitList* result) const;

    string asString() const;

  private:
    set<OccursWithTriple, ad_utility::AsStringComparatorLt> _owConjuncts;
    set<RelationTriple, ad_utility::AsStringComparatorLt> _relConjuncts;
    set<IsATriple, ad_utility::AsStringComparatorLt> _isaConjuncts;
    set<EqualsTriple, ad_utility::AsStringComparatorLt> _eqConjuncts;
    void computeResult(IntermediateQueryResult* result) const;
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_DISJUNCT_H_
