// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_QUERYTRIPLE_H_
#define SEMANTIC_WIKIPEDIA_SERVER_QUERYTRIPLE_H_

#include <gtest/gtest_prod.h>
#include <string>
#include <set>
#include "./ExecutableResultProvider.h"

using std::string;
using std::set;

namespace ad_semsearch
{
// Forward declare node class.
class QueryTreeNode;
//! An OccursWithTriple demands co-occurrence between
//! 0-n words and 0-m subtree (in particular QueryNode) results.
//! One of the three possible leaves of query trees.
//! (Occurs-with, equals, is-a).
class OccursWithTriple : public ExecutableResultProvider
{
  public:
    //! Default ctor.
    OccursWithTriple();

    //! Typical ctor.
    explicit OccursWithTriple(QueryExecutionContext* executionContext);

    //! Copy ctor.
    OccursWithTriple(const OccursWithTriple& other);

    //! Destructor.
    virtual ~OccursWithTriple();

    //! Assignment operator
    OccursWithTriple& operator=(const OccursWithTriple& rhs);

    //! Adds a subtree which whose result co-occurrence is requested.
    void addSubtree(const QueryTreeNode& subtree);

    //! Adds a word with which co-occurrence is requested.
    void addWord(const string& word);

    string asString() const;

    //! Adds excerpts as evidence for this co-occurrence
    //! and collects hits from possible subtrees.
    void getHitsForEntity(Id entityId, HitList* result) const;

  private:
    set<string> _words;
    set<QueryTreeNode*, ad_utility::AsStringPtrComparatorLt> _subtrees;
    void computeResult(IntermediateQueryResult* result) const;
};

//! An IsATriple requires access to the is-a relation. Due to special
//! usage of this relation (access always through exactly one class,
//! not through a subtree result), there also is a special triple
//! representation. This allows specialized computation.
//! One of the three possible leaves of query trees
//! (Occurs-with, equals, is-a).
class IsATriple : public ExecutableResultProvider
{
  public:
    //! Default ctor.
    IsATriple()
    : ExecutableResultProvider()
    {
    }

    //! Typical ctor.
    explicit IsATriple(QueryExecutionContext* executionContext)
    : ExecutableResultProvider(executionContext)
    {
    }

    //! Full ctor
    IsATriple(QueryExecutionContext* executionContext,
        const string& targetClass)
    : ExecutableResultProvider(executionContext), _targetClass(targetClass)
    {
    }

    //! Set the target class
    void setTargetClass(const string& targetClass)
    {
      _targetClass = targetClass;
    }

    //! Adds hits of the form "Albert Einstein is a physicist".
    void getHitsForEntity(Id entityId, HitList* result) const;

    string asString() const;

  private:
    string _targetClass;
    void computeResult(IntermediateQueryResult* result) const;
};

//! An EqualsTriple has a trivial solution.
//! One of the three possible leaves of query trees.
//! (Occurs-with, equals, is-a).
class EqualsTriple : public ExecutableResultProvider
{
  public:
    //! Default ctor.
    EqualsTriple()
    : ExecutableResultProvider()
    {
    }

    //! Typical ctor.
    explicit EqualsTriple(QueryExecutionContext* executionContext)
    : ExecutableResultProvider(executionContext)
    {
    }

    //! Full ctor
    EqualsTriple(QueryExecutionContext* executionContext,
        const string& entity)
    : ExecutableResultProvider(executionContext), _entity(entity)
    {
    }

    //! Set the target class
    void setTargetClass(const string& entity)
    {
      _entity = entity;
    }

    string asString() const;

    //! An equals triple does not need a hit evidence.
    void getHitsForEntity(Id entityId, HitList* result) const {}

  private:
    string _entity;
    void computeResult(IntermediateQueryResult* result) const;
};

//! A RelationTriple models an arbitrary relation in the query tree
//! that is NOT the is-a relation or the special relation co-occurs.
class RelationTriple : public ExecutableResultProvider
{
  public:
    //! Default ctor.
    RelationTriple();

    //! Typical ctor.
    explicit RelationTriple(QueryExecutionContext* executionContext);

    //! Full ctor.
    RelationTriple(QueryExecutionContext* executionContext,
        const string& relationName, const QueryTreeNode& target);

    //! Copy ctor.
    RelationTriple(const RelationTriple& other);

    //! Destructor.
    virtual ~RelationTriple();

    //! Assignment operator.
    RelationTriple& operator=(const RelationTriple& rhs);

    //! The the relation name and hence the actual relation this
    //! triple is concerned with.
    void setRelationName(const string& relationName);

    //! Set the target of the relation. A target is always
    //! another QueryTreeNode with and IntermediateQueryResult.
    void setTarget(const QueryTreeNode& target);

    string asString() const;

    //! Adds hits of the form "Albert Einstein born in Ulm"
    //! and collects hits from possible subtrees.
    void getHitsForEntity(Id entityId, HitList* result) const;

  private:
    string _relationName;
    QueryTreeNode* _target;
    void computeResult(IntermediateQueryResult* result) const;
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_QUERYTRIPLE_H_
