// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <limits>
#include <vector>
#include <algorithm>

#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"

#include "./Engine.h"
#include "./Identifiers.h"
#include "./Aggregators.h"

using std::endl;
using std::vector;

namespace ad_semsearch
{
// _____________________________________________________________________________
void Engine::filterPostingListBySingleWordId(const PostingList& postingList,
    const Id wordId, PostingList* result) const
{
  LOG(DEBUG)
  << "Filtering posting list with " << postingList.size()
      << " elements by a single wordId." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(size_t(0), result->size());

  // Make sure it is still true that all entity Ids are larger than word Ids.
  AD_CHECK_GT(getFirstId(IdType::ONTOLOGY_ELEMENT_ID),
      getFirstId(IdType::WORD_ID));

  if (postingList.size() == 0) return;

  Id currentContext = postingList[0]._contextId;
  bool keepThisContextsEntities = false;
  for (size_t i = 0; i < postingList.size(); ++i)
  {
    if (postingList[i]._contextId != currentContext)
    {
      currentContext = postingList[i]._contextId;
      keepThisContextsEntities = false;
    }
    if (keepThisContextsEntities
        && isIdOfType(postingList[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
    {
      result->push_back(postingList[i]);
    }
    if (postingList[i]._id == wordId)
    {
      keepThisContextsEntities = true;
      result->push_back(postingList[i]);
    }
  }

  LOG(DEBUG)
  << "Done filtering posting list. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::filterPostingListByWordRange(const PostingList& postingList,
    const Id lower, const Id upper, PostingList* result) const
{
  // Check if the range is correct
  AD_CHECK_LE(lower, upper);
  // Check if we have a trivial range.
  if (lower == upper)
  {
    filterPostingListBySingleWordId(postingList, lower, result);
    return;
  }
  LOG(DEBUG)
  << "Filtering posting list with " << postingList.size()
      << " elements by word range." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(size_t(0), result->size());

  // Make sure it still holds that all entity Ids are larger than word Ids
  AD_CHECK_GT(getFirstId(IdType::ONTOLOGY_ELEMENT_ID),
      getFirstId(IdType::WORD_ID));

  if (postingList.size() == 0) return;

  Id currentContext = postingList[0]._contextId;
  bool keepThisContextsEntities = false;
  for (size_t i = 0; i < postingList.size(); ++i)
  {
    if (postingList[i]._contextId != currentContext)
    {
      currentContext = postingList[i]._contextId;
      keepThisContextsEntities = false;
    }
    if (keepThisContextsEntities
        && isIdOfType(postingList[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
    {
      result->push_back(postingList[i]);
    }
    if (postingList[i]._id >= lower && postingList[i]._id <= upper)
    {
      keepThisContextsEntities = true;
      result->push_back(postingList[i]);
    }
  }

  LOG(DEBUG)
  << "Done filtering posting list. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
template<class ScoreAggregationFunctor>
void Engine::aggregatePostingList(const PostingList& postings,
    EntityList* result) const
{
  LOG(DEBUG)
  << "Aggregating posting list with " << postings.size() << " elements."
      << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(size_t(0), result->size());

  // Collect all distinct entity Ids and aggregate scores for them.
  ad_utility::HashMap<Id, AggregatedScore> entityScoreMap;

  // IMPORTANT NOTE:
  // Use always use agg(current, newValue) when aggregating. This is important
  // if the plus 1 aggregator is used, because it only increments the
  // first
  ScoreAggregationFunctor agg;

  // Iterator over the postings
  for (size_t i = 0; i < postings.size(); ++i)
  {
    // Only treat entity postings.
    if (isIdOfType(postings[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
    {
      ad_utility::HashMap<Id, AggregatedScore>::iterator it =
          entityScoreMap.find(postings[i]._id);

      if (it != entityScoreMap.end())
      {
        // If this entiy has already been seen, just update the score.
        it->second = agg(static_cast<AggregatedScore>(it->second),
            static_cast<AggregatedScore>(postings[i]._score));
      }
      else
      {
        // If this entity has not been seen yet,
        // create an entry for it in the map.
        entityScoreMap[postings[i]._id] = agg(0,
            static_cast<AggregatedScore>(postings[i]._score));
      }
    }
  }

  // Transform the map to a list again
  for (ad_utility::HashMap<Id, AggregatedScore>::const_iterator it =
      entityScoreMap.begin(); it != entityScoreMap.end(); ++it)
  {
    result->push_back(EntityWithScore(it->first, it->second));
  }

  // Sort the result
  std::sort(result->begin(), result->end());

  LOG(DEBUG)
  << "Done aggregating posting list. Resulting EntityList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
template<class ScoreAggregationFunctor>
void Engine::aggregateWordsInPostingList(const PostingList& postings,
    EntityList* result) const
{
  LOG(DEBUG)
  << "Aggregating posting list with " << postings.size() << " elements."
      << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(size_t(0), result->size());

  // Collect all distinct entity Ids and aggregate scores for them.
  ad_utility::HashMap<Id, AggregatedScore> wordScoreMap;

  // IMPORTANT NOTE:
  // Use always use agg(current, newValue) when aggregating. This is important
  // if the plus 1 aggregator is used, because it only increments the
  // first
  ScoreAggregationFunctor agg;

  // Iterator over the postings
  for (size_t i = 0; i < postings.size(); ++i)
  {
    // Only treat entity postings.
    if (isIdOfType(postings[i]._id, IdType::WORD_ID))
    {
      ad_utility::HashMap<Id, AggregatedScore>::iterator it =
          wordScoreMap.find(postings[i]._id);

      if (it != wordScoreMap.end())
      {
        // If this entiy has already been seen, just update the score.
        it->second = agg(static_cast<AggregatedScore>(it->second),
            static_cast<AggregatedScore>(postings[i]._score));
      }
      else
      {
        // If this entity has not been seen yet,
        // create an entry for it in the map.
        wordScoreMap[postings[i]._id] = agg(0,
            static_cast<AggregatedScore>(postings[i]._score));
      }
    }
  }

  // Transform the map to a list again
  for (ad_utility::HashMap<Id, AggregatedScore>::const_iterator it =
      wordScoreMap.begin(); it != wordScoreMap.end(); ++it)
  {
    result->push_back(EntityWithScore(it->first, it->second));
  }

  // Sort the result
  std::sort(result->begin(), result->end());

  LOG(DEBUG)
  << "Done aggregating posting list. Resulting EntityList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::joinPostingListsOnContextId(const PostingList& list1,
    const PostingList& list2, PostingList* result) const
{
  LOG(DEBUG)
  << "Joining two PostingLists with " << list1.size() << " and "
      << list2.size() << " elements on context Id." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(size_t(0), result->size());

  TextPosting sentinel1(0, std::numeric_limits<Id>::max() - 1, 0, 0);
  TextPosting sentinel2(0, std::numeric_limits<Id>::max(), 0, 0);
  // Cast away constness so we can add sentinels that will be removed
  // in the end.
  PostingList& l1 = const_cast<PostingList&>(list1);
  PostingList& l2 = const_cast<PostingList&>(list2);
  l1.push_back(sentinel1);
  l2.push_back(sentinel2);

  // Intersection loop
  size_t i = 0;
  size_t j = 0;
  while (l1[i]._contextId < sentinel1._contextId)
  {
    if (l1[i]._contextId == l2[j]._contextId)
    {
      // Match found. Now make sure to
      // collect all postings with this context id
      // from both list and ensure to keep them ordered
      // by word / entity Id.
      if (l1[i]._id < l2[j]._id)
      {
        result->push_back(l1[i]);
        ++i;
        if (l1[i]._contextId > l2[j]._contextId)
        {
          // If this was the last one with that ID form l1,
          // add all remaining from l2. Otherwise just continue.
          while (l2[j]._contextId == l1[i - 1]._contextId)
          {
            result->push_back(l2[j]);
            ++j;
          }
        }
      }
      else
      {
        result->push_back(l2[j]);
        ++j;
        if (l2[j]._contextId > l1[i]._contextId)
        {
          // If this was the last one with that ID form l2,
          // add all remaining from l1. Otherwise just continue.
          while (l1[i]._contextId == l2[j - 1]._contextId)
          {
            result->push_back(l1[i]);
            ++i;
          }
        }
      }
    }
    // TODO(buchholb): Fix this mix of bounds checking and sentinel usage
    // and make it a proper intersection.
    if (j >= l2.size() - 1) break;
    // Increase counters. Will stop at sentinels because first
    // l1[i] should reach the sentinel, secondly l2 will
    // still reach the sentinels because it is larger.
    // Afterwards the loop will be left because the sent in l1 is reached.

    while (l1[i]._contextId < l2[j]._contextId)
      ++i;
    while (l2[j]._contextId < l1[i]._contextId)
      ++j;
  }

  // Remove sentinels
  l1.resize(l1.size() - 1);
  l2.resize(l2.size() - 1);

  LOG(DEBUG)
  << "Done joining PostingLists. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
template<class ScoreAggregationFunctor>
void Engine::intersectEntityLists(const EntityList& list1,
    const EntityList& list2, EntityList* result) const
{
  LOG(DEBUG)
      << "Intersecting two EntityLists with " << list1.size()
      << " and " << list2.size()
      << " elements." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(size_t(0), result->size());

  if (list1.size() > 0 && list2.size() > 0)
  {
    // Cast away constness so we can add sentinels that will be removed
    // in the end.
//    bool list1HasLowerId = list1[0]._id < list2[0]._id;
    Id sent = std::numeric_limits<Id>::max();
    EntityList& l1 = const_cast<EntityList&>(list1);
    EntityList& l2 = const_cast<EntityList&>(list2);
    l1.push_back(EntityWithScore(sent, Score(1)));
    l2.push_back(EntityWithScore(sent, Score(1)));

    // Find a good position to start.
//    EntityList::const_iterator it = std::lower_bound(l1.begin(), l1.end(),
//        EntityWithScore(l2[0]._id, Score(0)),
//        ad_utility::CompareIdComparator());
//
//    size_t i = it - l1.begin();
    size_t i = 0;
    size_t j = 0;

    // Use the special property that entity IDs should be unique
    // among all those lists. Hence this is much easier than the method
    // for two PostingLists.
    ScoreAggregationFunctor agg;
    while (l1[i]._id < sent)
    {
      if (l1[i]._id == l2[j]._id)
      {
        result->push_back(
            EntityWithScore(l1[i]._id, agg(l1[i]._score, l2[j]._score)));
        ++i;
        ++j;
      }
      while (l1[i]._id < l2[j]._id)
        ++i;
      while (l2[j]._id < l1[i]._id)
        ++j;
    }

    // Remove sentinels
    l1.resize(l1.size() - 1);
    l2.resize(l2.size() - 1);
  }
  LOG(DEBUG)
  << "Done intersecting EntityLists. Resulting EntityList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::filterPostingListByEntityList(const PostingList& postingList,
    const EntityList& entityList, PostingList* result) const
{
  LOG(DEBUG)
      << "Filtering posting list with " << postingList.size()
      << " elements by an entity list with " << entityList.size()
      << " elements." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());

  // TODO(buchholb): As Hannah suggested, a bloom filter might speed this up.
  // For now, just use a HashSet of entity Ids.
  ad_utility::HashSet<Id> entitiesToKeep;
  for (size_t i = 0; i < entityList.size(); ++i)
  {
    entitiesToKeep.insert(entityList[i]._id);
  }

  // Now filter the posting list
  for (size_t i = 0; i < postingList.size(); ++i)
  {
    if (entitiesToKeep.count(postingList[i]._id) > 0)
    {
      result->push_back(postingList[i]);
    }
  }

  LOG(DEBUG)
      << "Done filtering posting list. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::filterPostingListByEntityListKeepWordPostings(
    const PostingList& postingList, const EntityList& entityList,
    PostingList* result) const
{
  LOG(DEBUG)
      << "Filtering posting list with " << postingList.size()
      << " elements by an entity list with " << entityList.size()
      << " elements." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());

  // TODO(buchholb): As Hannah suggested, a bloom filter might speed this up.
  // For now, just use a HashSet of entity Ids.
  ad_utility::HashSet<Id> entitiesToKeep;
  for (size_t i = 0; i < entityList.size(); ++i)
  {
    entitiesToKeep.insert(entityList[i]._id);
  }

  // Now filter the posting list
  // Process the posting list backwards so we will first encounter entities
  // and only afterwards word postings for each context.
  Id keepContext = std::numeric_limits<Id>::max();
  for (int i = static_cast<int>(postingList.size() - 1); i >= 0; --i)
  {
    // Is an entity posting. Decide if we keep this posting
    // and words postings form the same context.
    if (isIdOfType(postingList[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
    {
      if (entitiesToKeep.count(postingList[i]._id) > 0)
      {
        keepContext = postingList[i]._contextId;
        result->push_back(postingList[i]);
      }
    }
    else
    {
      // Is a word posting.
      if (postingList[i]._contextId == keepContext)
      {
        result->push_back(postingList[i]);
      }
    }
  }
  // Reverse the result because we had to process the postings backwards.
  std::reverse(result->begin(), result->end());

  LOG(DEBUG)
  << "Done filtering posting list. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::filterPostingsByEntityId(const PostingList& postingList,
    Id entityId, PostingList* result) const
{
  LOG(DEBUG)
      << "Filtering posting list with " << postingList.size()
      << " elements by a single entity ID." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());

  // Now filter the posting list
  // Process the posting list backwards so we will first encounter entities
  // and only afterwards word postings for each context.
  // Now filter the posting list
  // Process the posting list backwards so we will first encounter entities
  // and only afterwards word postings for each context.
  Id keepContext = std::numeric_limits<Id>::max();
  for (int i = static_cast<int>(postingList.size() - 1); i >= 0; --i)
  {
    if (isIdOfType(postingList[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
    {
      if (postingList[i]._id == entityId)
      {
        keepContext = postingList[i]._contextId;
        result->push_back(postingList[i]);
      }
    }
    else
    {
      // Is a word posting.
      if (postingList[i]._contextId == keepContext)
      {
        result->push_back(postingList[i]);
      }
    }
  }
  // Reverse the result because we had to process the postings backwards.
  std::reverse(result->begin(), result->end());

  LOG(DEBUG)
  << "Done filtering posting list. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::filterPostingsByEntityId(const PostingList& postingList,
    Id entityId, const ad_utility::HashSet<Id>& otherEntitiesToKeep,
    PostingList* result) const
{
  LOG(DEBUG)
      << "Filtering posting list with " << postingList.size()
      << " elements by a single entity ID with an additional list."
      << " of entities to keep." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());


  // Now filter the posting list
  // Since the additional entities to keep be both,
  // before and after, the entity to filter with, we have three
  // states to assign to each item: keep, discard and unknown.
  // Unknown items are kept and depending on what happens next either
  // discarded or kept.
  // However, we still process the list backwards so that
  // non-entity postings can always be kept or discarded immediately.
  PostingList keepMaybe;
  Id keepContext = std::numeric_limits<Id>::max();
  for (int i = static_cast<int>(postingList.size() - 1); i >= 0; --i)
  {
    if (isIdOfType(postingList[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
    {
      if (postingList[i]._id == entityId)
      {
        keepContext = postingList[i]._contextId;
        for (size_t j = 0; j < keepMaybe.size(); ++j)
        {
          if (keepMaybe[j]._contextId == keepContext)
          {
            result->push_back(keepMaybe[j]);
          }
        }
        keepMaybe.clear();
        result->push_back(postingList[i]);
      }
      else
      {
        if (postingList[i]._contextId == keepContext)
        {
          result->push_back(postingList[i]);
        }
        else if (otherEntitiesToKeep.count(postingList[i]._id) > 0)
        {
          keepMaybe.push_back(postingList[i]);
        }
      }
    }
    else
    {
      // Is a word posting.
      if (postingList[i]._contextId == keepContext)
      {
        result->push_back(postingList[i]);
      }
    }
  }
  // Reverse the result because we had to process the postings backwards.
  std::reverse(result->begin(), result->end());

  LOG(DEBUG)
  << "Done filtering posting list. Resulting PostingList now has "
      << result->size() << " elements." << endl;
}
// _____________________________________________________________________________
void Engine::getRelationRhsBySingleLhs(const Relation& relation, Id key,
    EntityList* result) const
{
  LOG(DEBUG)
  << "Accessing relation with " << relation.size() << " elements by an Id key."
      << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());

  Relation::const_iterator it = std::lower_bound(relation.begin(),
      relation.end(), RelationEntry(key, 0),
      ad_utility::CompareLhsComparator());

  while (it != relation.end() && it->_lhs == key)
  {
    result->push_back(EntityWithScore(it->_rhs, ENTITY_FROM_RELATION_SCORE));
    ++it;
  }

  LOG(DEBUG)
  << "Done accessing relation. Matching right-hand-side EntityList now has "
      << result->size() << " elements." << endl;
}

// _____________________________________________________________________________
template<class ScoreAggregationFunctor>
void Engine::getRelationRhsByEntityListLhs(const Relation& relation,
    EntityList lhs, Relation* matchingRelEntries, EntityList* result) const
{
  LOG(DEBUG)
      << "Accessing relation with " << relation.size()
      << " elements by an EntityList with " << lhs.size() << " elements."
      << endl;
  AD_CHECK(result);

  AD_CHECK_EQ(0, result->size());

  EntityList resultWithDuplicates;

  if (lhs.size() > 0)
  {
    // Cast away constness so we can add sentinels that will be removed
    // in the end.
    Id sent = std::numeric_limits<Id>::max();
    Relation& rel = const_cast<Relation&>(relation);
    EntityList& l = const_cast<EntityList&>(lhs);
    rel.push_back(RelationEntry(sent, sent));
    l.push_back(EntityWithScore(sent, Score(1)));

    // Find a good position to start.
    Relation::const_iterator it = std::lower_bound(relation.begin(),
        relation.end(), RelationEntry(lhs[0]._id, 0),
        ad_utility::CompareLhsComparator());

    size_t i = it - relation.begin();
    size_t j = 0;

    // Use the special property that entity IDs should be unique
    // in the entity list. However, they are not in the realtions.
    // Hence this is less complicated as intersecting posting lists
    // but more complicated as intersecting two entity lists.
    while (rel[i]._lhs < sent)
    {
      if (rel[i]._lhs == l[j]._id)
      {
        while (rel[i]._lhs == l[j]._id)
        {
          resultWithDuplicates.push_back(
              EntityWithScore(rel[i]._rhs, l[j]._score));
          matchingRelEntries->push_back(rel[i]);
          ++i;
        }
        ++j;
      }
      while (rel[i]._lhs < l[j]._id)
        ++i;
      while (l[j]._id < rel[i]._lhs)
        ++j;
    }

    // Remove sentinels
    rel.resize(rel.size() - 1);
    l.resize(l.size() - 1);
  }

  LOG(DEBUG)
  << "Done accessing relation. Now sorting and aggregating result." << endl;

  // Sort
  std::sort(resultWithDuplicates.begin(), resultWithDuplicates.end());

  // IMPORTANT NOTE:
  // Use always use agg(current, newValue) when aggregating. This is important
  // if the plus 1 aggregator is used, because it only increments the
  // first
  ScoreAggregationFunctor agg;

  // Aggregate scores and add unique entities to the result.
  if (resultWithDuplicates.size() > 0)
  {
    Id currentEntity = resultWithDuplicates[0]._id;
    AggregatedScore currentScore = resultWithDuplicates[0]._score;
    for (size_t i = 0; i < resultWithDuplicates.size(); ++i)
    {
      if (resultWithDuplicates[i]._id == currentEntity)
      {
        currentScore = agg(currentScore, resultWithDuplicates[i]._score);
      }
      else
      {
        result->push_back(EntityWithScore(currentEntity, currentScore));
        // Setup everything for the next entity.
        currentEntity = resultWithDuplicates[i]._id;
        currentScore = resultWithDuplicates[i]._score;
      }
    }
    // Process the entity.
    result->push_back(EntityWithScore(currentEntity, currentScore));
  }


  LOG(DEBUG)
  << "Done. Matching right-hand-side EntityList now has " << result->size()
      << " elements." << endl;
}

// _____________________________________________________________________________
template<class Aggregator>
void Engine::getTopKContextsWithHighlights(const PostingList& postings,
    size_t k, HitList* result) const
{
  LOG(DEBUG)
  << "Getting the top " << k << " contexts with highlights"
      << " from a PostingList with " << postings.size() << " elements."
      << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());
  Aggregator agg;

  // Iterate over the postings list
  if (postings.size() > 0)
  {
    Id currentContextId = postings[0]._contextId;
    Excerpt::Highlights highlights;
    AggregatedScore currentScore = 0;
    for (size_t i = 0; i < postings.size(); ++i)
    {
      // For each context, aggregate scores
      // and collect positions / highlights.
      if (postings[i]._contextId != currentContextId)
      {
        // Create a hit object for the accumulated data.
        // Sort highlights and make them unique.
        std::sort(highlights.begin(), highlights.end());
        highlights.resize(
            std::unique(highlights.begin(), highlights.end())
                - highlights.begin());
        result->push_back(
            Hit(currentContextId, "", highlights, currentScore));
        // Reset buffers.
        currentContextId = postings[i]._contextId;
        highlights.clear();
        currentScore = 0;
      }
      // Accumulate data.
      highlights.push_back(postings[i]._position);
      currentScore = agg(currentScore,
          static_cast<AggregatedScore>(postings[i]._score));
    }
    // Create a hit object for the last set of accumulated data.
    // Sort highlights and make them unique.
    std::sort(highlights.begin(), highlights.end());
    highlights.resize(
        std::unique(highlights.begin(), highlights.end())
            - highlights.begin());
    result->push_back(
        Hit(currentContextId, "", highlights, currentScore));
  }

  // Determine the top k
  // TODO(buchholb): Make use of the k, partial sort!
  LOG(DEBUG)
  << "Determining top-k in a trivial way. Replace this in time." << endl;

  std::sort(result->begin(), result->end(),
      ad_utility::CompareScoreComparatorGt());
  if (result->size() > k) result->resize(k);

  LOG(DEBUG)
  << "Got top-hits. There were " << result->size() << " items to get."
      << endl;
}

// _____________________________________________________________________________
template<class Aggregator>
void Engine::getTopKContextsWithHighlightsAndEntities(
    const PostingList& postings, size_t k,
    HitList* result) const
{
  LOG(DEBUG)
      << "Getting the top " << k << " contexts with highlights and entities"
      << " from a PostingList with " << postings.size() << " elements."
      << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());
  Aggregator agg;

  // Iterate over the postings list
  if (postings.size() > 0)
  {
    Id currentContextId = postings[0]._contextId;
    Excerpt::Highlights highlights;
    vector<Id> entities;
    AggregatedScore currentScore = 0;
    for (size_t i = 0; i < postings.size(); ++i)
    {
      // For each context, aggregate scores
      // and collect positions / highlights.
      if (postings[i]._contextId != currentContextId)
      {
        // Create a hit object for the accumulated data.
        // Sort highlights and make them unique.
        std::sort(highlights.begin(), highlights.end());
        highlights.resize(
            std::unique(highlights.begin(), highlights.end())
                - highlights.begin());
        result->push_back(
            Hit(currentContextId, "", highlights, currentScore,
                entities));
        // Reset buffers.
        currentContextId = postings[i]._contextId;
        highlights.clear();
        entities.clear();
        currentScore = 0;
      }
      // Accumulate data.
      highlights.push_back(postings[i]._position);
      currentScore = agg(currentScore,
          static_cast<AggregatedScore>(postings[i]._score));
      if (isIdOfType(postings[i]._id, IdType::ONTOLOGY_ELEMENT_ID))
      {
        entities.push_back(postings[i]._id);
      }
    }
    // Create a hit object for the last set of accumulated data.
    // Sort highlights and make them unique.
    std::sort(highlights.begin(), highlights.end());
    highlights.resize(
        std::unique(highlights.begin(), highlights.end())
            - highlights.begin());
    result->push_back(
        Hit(currentContextId, "", highlights, currentScore, entities));
  }

  // Determine the top k
  // TODO(buchholb): Make use of the k, partial sort!
  LOG(DEBUG)
  << "Determining top-k in a trivial way. Replace this in time." << endl;

  std::sort(result->begin(), result->end(),
      ad_utility::CompareScoreComparatorGt());
  if (result->size() > k) result->resize(k);

  LOG(DEBUG)
  << "Got top-hits. There were " << result->size() << " items to get."
      << endl;
}

// _____________________________________________________________________________
void Engine::getTopKEntities(const EntityList& entities, size_t k,
    EntityList* result) const
{
  LOG(DEBUG)
  << "Getting the top " << k << " entities" << " from an EntityList with "
      << entities.size() << " elements." << endl;
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());

  // Determine the top k
  // TODO(buchholb): Make use of the k, partial sort!
  LOG(DEBUG)
  << "Determining top-k in a trivial way. Replace this in time." << endl;

  // Make a copy.
  *result = entities;
  // Sort it.
  std::sort(result->begin(), result->end(),
      ad_utility::CompareScoreComparatorGt());

  // Truncate
  if (result->size() > k) result->resize(k);

  LOG(DEBUG)
  << "Got top-entities. There were " << result->size() << " items to get."
      << endl;
}

// _____________________________________________________________________________
Id Engine::getAnyMatchingEntitiy(const EntityList& entities,
    const vector<Id>& ids) const
{
  ad_utility::HashSet<Id> filter;
  for (size_t i = 0; i < ids.size(); ++i)
  {
    filter.insert(ids[i]);
  }
  for (size_t i = 0; i < entities.size(); ++i)
  {
    if (filter.count(entities[i]._id) > 0)
    {
      return entities[i]._id;
    }
  }
  AD_THROW(
      Exception::CHECK_FAILED,
      "Couldn't find a matching entity in two lists "
      "that should always have matching entities.");
}

// _____________________________________________________________________________
void Engine::filterEntityListByIdRange(const EntityList& entities,
    const IdRange& idRrange, EntityList* result) const
{
  AD_CHECK(result);
  AD_CHECK_EQ(0, result->size());

  LOG(DEBUG) << "Filtering EntityList with " << entities.size() << " elements"
      << " by Id-range " << idRrange << '.' << endl;


  EntityList::const_iterator first = std::lower_bound(entities.begin(),
      entities.end(), EntityWithScore(idRrange._first, 0),
      ad_utility::CompareIdComparator());

  EntityList::const_iterator end = std::upper_bound(entities.begin(),
      entities.end(), EntityWithScore(idRrange._last, 0),
      ad_utility::CompareIdComparator());


  if (first != entities.end() && first < end)
  {
    result->insert(result->begin(), first, end);
  }

  LOG(DEBUG)
      << "Filtering EntityList by Id-range. Result has size: " << result->size()
      << '.' << endl;
}
}

// Template instantiations:
template void ad_semsearch::Engine::aggregatePostingList<
    ad_semsearch::PlusOneAggregator<ad_semsearch::AggregatedScore> >(
    ad_semsearch::List<ad_semsearch::TextPosting> const&,
    ad_semsearch::EntityList*) const;

template void ad_semsearch::Engine::aggregatePostingList<
    ad_semsearch::SumAggregator<ad_semsearch::AggregatedScore> >(
    ad_semsearch::List<ad_semsearch::TextPosting> const&,
    ad_semsearch::EntityList*) const;

template void ad_semsearch::Engine::aggregateWordsInPostingList<
    ad_semsearch::PlusOneAggregator<ad_semsearch::AggregatedScore> >(
    ad_semsearch::List<ad_semsearch::TextPosting> const&,
    ad_semsearch::EntityList*) const;

template void ad_semsearch::Engine::intersectEntityLists<
    ad_semsearch::SumAggregator<ad_semsearch::AggregatedScore> >(
    ad_semsearch::EntityList const&, ad_semsearch::EntityList const&,
    ad_semsearch::EntityList*) const;

template void ad_semsearch::Engine::getTopKContextsWithHighlights<
    ad_semsearch::SumAggregator<ad_semsearch::AggregatedScore> >(
    ad_semsearch::List<ad_semsearch::TextPosting> const&, size_t,
    ad_semsearch::HitList*) const;

template void
ad_semsearch::Engine::getTopKContextsWithHighlightsAndEntities<
    ad_semsearch::SumAggregator<ad_semsearch::AggregatedScore> >(
    ad_semsearch::List<ad_semsearch::TextPosting> const&, size_t,
    ad_semsearch::HitList*) const;

template void
ad_semsearch::Engine::getRelationRhsByEntityListLhs<
    ad_semsearch::SumAggregator<ad_semsearch::AggregatedScore> >(
    const Relation& relation, EntityList lhs, Relation* matchingRelEntries,
    EntityList* result) const;

template void
ad_semsearch::Engine::getRelationRhsByEntityListLhs<
    ad_semsearch::PlusOneAggregator<ad_semsearch::AggregatedScore> >(
    const Relation& relation, EntityList lhs, Relation* matchingRelEntries,
    EntityList* result) const;
