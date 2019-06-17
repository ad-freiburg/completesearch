// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_ENGINE_H_
#define SEMANTIC_WIKIPEDIA_SERVER_ENGINE_H_

#include <vector>

#include "./PostingList.h"
#include "./Identifiers.h"
#include "./Relation.h"
#include "./EntityList.h"
#include "./HitList.h"
#include "./Aggregators.h"
#include "./Vocabulary.h"

#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::vector;

namespace ad_semsearch
{
//! Engine that performs operations on the index retrieved from
//! the index. Basically class Index allows getting lists from the
//! actual index, and Engine allows deriving new lists through operations.
class Engine
{
  public:
    Engine()
    {
    }

    virtual ~Engine()
    {
    }

    //! Filters a posting list by a word Id. Only postings with either
    //! that particular Id, or postings with ontology Id in the same
    //! context as other postings with the given Id are contained by the result.
    void filterPostingListBySingleWordId(const PostingList& postingList,
        const Id wordId, PostingList* result) const;

    //! Filters a posting list by a range of Ids. Only postings with either
    //! an Id form that range, or postings with ontology Id in the same
    //! context as postings with an Id from the given range
    //! are contained by the result.
    void filterPostingListByWordRange(const PostingList& postingList,
        const Id lower, const Id upper, PostingList* result) const;

    //! Aggregates a posting list into an entity list.
    //! Collects only entity postings from the posting lists,
    //! discards ContextId and Position and aggregates the
    //! scores for postings of the same entity.
    //! The resulting lists is ordered by entityId.
    template<class ScoreAggregationFunctor>
    void aggregatePostingList(const PostingList& postings,
        EntityList* result) const;

    //! Aggregates a posting list into an entity list.
    //! Collects only entity postings from the posting lists,
    //! discards ContextId and Position and aggregates the
    //! scores for postings of the same entity.
    //! The resulting lists is ordered by entityId.
    //! Calls the templated version, currently using a default aggregator,
    //! later depending on settings.
    void aggregatePostingList(const PostingList& postings,
        EntityList* result) const
    {
        aggregatePostingList<PlusOneAggregator<AggregatedScore> > (
            postings, result);
    }

    //! Aggregates a posting list into an entity list.
    //! Collects only word postings from the posting lists,
    //! discards ContextId and Position and aggregates the
    //! scores for postings of the same word.
    //! The resulting lists is ordered by wordId.
    template<class ScoreAggregationFunctor>
    void aggregateWordsInPostingList(const PostingList& postings,
        EntityList* result) const;

    //! Aggregates a posting list into an entity list.
    //! Collects only word postings from the posting lists,
    //! discards ContextId and Position and aggregates the
    //! scores for postings of the same word.
    //! The resulting lists is ordered by wordI
    //! Calls the templated version, currently using a default aggregator,
    //! later depending on settings.
    void aggregateWordsInPostingList(const PostingList& postings,
        EntityList* result) const
    {
      aggregateWordsInPostingList<PlusOneAggregator<AggregatedScore> > (
            postings, result);
    }


    //! Join two posting lists on contextId.
    //! This relates to what was called "intersectTwoPostingLists" in
    //! CompleteSearch.
    //! Input and result are all assumed to be ordered by contexId.
    void joinPostingListsOnContextId(const PostingList& list1,
        const PostingList& list2, PostingList* result) const;

    //! Join two entity lists on EntityId
    //! Input and result are assumed to be ordered by entity Id.
    //! The result contain all entities that occur in both
    //! lists with the scores aggregated somehow.
    template<class ScoreAggregationFunctor>
    void intersectEntityLists(const EntityList& list1,
        const EntityList& list2,
        EntityList* result) const;

    //! Join two entity lists on EntityId
    //! Input and result are assumed to be ordered by entity Id.
    //! The result contain all entities that occur in both
    //! lists with the scores aggregated somehow.
    //! Calls the templated version, currently using a default aggregator,
    //! later depending on settings.
    void intersectEntityLists(const EntityList& list1,
        const EntityList& list2, EntityList* result) const
    {
      intersectEntityLists<SumAggregator<AggregatedScore> > (list1, list2,
          result);
    }

    //! Filters a posting list by a (small) list of entities.
    //! Note that postings lists usually are sorted by contextId
    //! whereas entity lists are sorted by entity Id.
    //! Currently this filter does not make any assumptions on the
    //! ordering of this lists so do not used this method for
    //! intersection of equally sorted lists.
    void filterPostingListByEntityList(const PostingList& postingList,
        const EntityList& entityList, PostingList* result) const;

    //! Filters a posting list by a (small) list of entities.
    //! Also keeps all word postings from the same context as
    //! one of the entities from the filter.
    void filterPostingListByEntityListKeepWordPostings(
        const PostingList& postingList, const EntityList& entityList,
        PostingList* result) const;

    //! Filters a posting list by a single entity ID.
    //! Also keeps all word postings from the same context as
    //! the entity to filter with
    void filterPostingsByEntityId(const PostingList& postingList,
        Id entityId, PostingList* result) const;

    //! Filters a posting list by a single entity ID.
    //! Also keeps <all word postings + entityPostings
    // that match the argument list> from the same context as
    //! the entity to filter with.
    void filterPostingsByEntityId(const PostingList& postingList,
        Id entityId, const ad_utility::HashSet<Id>& otherEntitiesToKeep,
        PostingList* result) const;

    //! Takes a relation and an Id that should occur on the left-hand-side
    //! of the relation (if it doesn't the result is empty)
    //! and returns an EntityList with matching right-hand-sides.
    //! The scores are currently determined by a constant.
    void getRelationRhsBySingleLhs(const Relation& relation, Id key,
        EntityList* result) const;

    //! Takes a relation and an EntityList that should occur on the
    //! left-hand-side of the relation (if it doesn't the result is empty)
    //! and returns an EntityList with matching right-hand-sides.
    //! The scores are currently determined by a constant.
    //! Also remembers the relation triples that were hits. This can
    //! be used to construct excerpts later on.
    template <class ScoreAggregationFunctor>
    void getRelationRhsByEntityListLhs(const Relation& relation,
        EntityList lhs, Relation* matchingRelEntriesLhs,
        EntityList* result) const;

    //! Takes a relation and an EntityList that should occur on the
    //! left-hand-side of the relation (if it doesn't the result is empty)
    //! and returns an EntityList with matching right-hand-sides.
    //! The scores are currently determined by a constant.
    //! Also remembers the relation triples that were hits. This can
    //! be used to construct excerpts later on.
    //! Calls the templated version.
    void getRelationRhsByEntityListLhs(const Relation& relation,
        EntityList lhs, Relation* matchingRelEntriesLhs,
        EntityList* result) const
    {
      getRelationRhsByEntityListLhs<SumAggregator<AggregatedScore> >(relation,
          lhs, matchingRelEntriesLhs, result);
    }

    //! Computes the top-k contexts with highlights of a PostingList w.r.t.
    //! the template arguments.
    //! This includes collecting all positions
    template<class Aggregator>
    void getTopKContextsWithHighlights(const PostingList& postings, size_t k,
        HitList* result) const;

    //! Computes the top-k contexts with highlights of a PostingList w.r.t.
    //! the template arguments.
    //! This includes collecting all positions and entities that are matched.
    template<class Aggregator>
    void getTopKContextsWithHighlightsAndEntities(const PostingList& postings,
        size_t k, HitList* result) const;


    //! Computes the top-k contexts with highlights.
    //! This includes collecting all positions.
    //! Calls the templated version, currently using a default arguments,
    //! later depending on settings.
    void getTopKContextsWithHighlights(const PostingList& postings, size_t k,
        HitList* result) const
    {
      getTopKContextsWithHighlights<SumAggregator<AggregatedScore> > (
          postings, k, result);
    }

    //! Computes the top-k contexts with highlights.
    //! This includes collecting all positions and entities that were
    //! highlighted.
    //! Calls the templated version, currently using a default arguments,
    //! later depending on settings.
    void getTopKContextsWithHighlightsAndEntities(const PostingList& postings,
        size_t k, HitList* result) const
    {
      getTopKContextsWithHighlightsAndEntities<SumAggregator<AggregatedScore> >(
          postings, k, result);
    }

    //! Computes the top-k entities of an entity list
    void getTopKEntities(const EntityList& entities, size_t k,
        EntityList* result) const;

    //! Gets any match between the EntitiyList and the List of Ids passed.
    //! Throws an exception when there is no match. Assumes the second argument
    //! is a very small list.
    Id getAnyMatchingEntitiy(const EntityList& entities,
        const vector<Id>& ids) const;

    //! Filters an EntityList by a range of Ids.
    void filterEntityListByIdRange(const EntityList& entities,
        const IdRange& idRange, EntityList* result) const;


  private:
    // TODO(buchholb): introduce members for settings, like howToRankEntities
    // etc.
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_ENGINE_H_
