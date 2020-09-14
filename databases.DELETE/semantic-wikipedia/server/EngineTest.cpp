// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./EntityList.h"
#include "./Identifiers.h"
#include "./Engine.h"
#include "./IndexMetaData.h"
#include "./Aggregators.h"

using std::string;
using std::vector;

using ad_utility::File;
namespace ad_semsearch
{
// _____________________________________________________________________________
TEST(EngineTest, filterPostingListBySingleWordIdTest)
{
  // Setup.
  PostingList postings;
  PostingList postings2;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  postings.push_back(TextPosting(entityId0 + 3, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 2, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  // Do the test.
  Engine engine;
  PostingList result;
  PostingList result2;
  PostingList result3;
  PostingList result4;
  engine.filterPostingListBySingleWordId(postings, wordId0, &result);
  engine.filterPostingListBySingleWordId(postings, wordId0 + 1, &result2);
  engine.filterPostingListBySingleWordId(postings, wordId0 + 5, &result3);
  engine.filterPostingListBySingleWordId(postings2, wordId0, &result4);

  // Make assertions.
  ASSERT_EQ(size_t(7), result.size());
  ASSERT_EQ(
      "[(WordId: 0, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 3, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 0, ContextId: 1, Score: 1, Pos: 1), ...]",
      result.asString());

  ASSERT_EQ(size_t(4), result2.size());
  ASSERT_EQ(
      "[(WordId: 1, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 3, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 1, Score: 1, Pos: 1), ...]",
      result2.asString());

  ASSERT_EQ(size_t(0), result3.size());
  ASSERT_EQ(size_t(0), result4.size());
}

// _____________________________________________________________________________
TEST(EngineTest, filterPostingListByWordRangeTest)
{
  // Setup.
  PostingList postings;
  PostingList postings2;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  postings.push_back(TextPosting(entityId0 + 3, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 2, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  // Do the test.
  Engine engine;
  PostingList result;
  PostingList result2;
  PostingList result3;
  PostingList result4;
  engine.filterPostingListByWordRange(postings, wordId0, wordId0, &result);
  engine.filterPostingListByWordRange(postings, wordId0 + 1, wordId0 + 2,
      &result2);
  engine.filterPostingListByWordRange(postings, wordId0 + 4, wordId0 + 5,
      &result3);
  engine.filterPostingListByWordRange(postings2, wordId0, wordId0, &result4);

  // Make assertions.
  ASSERT_EQ(size_t(7), result.size());
  ASSERT_EQ(
      "[(WordId: 0, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 3, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 0, ContextId: 1, Score: 1, Pos: 1), ...]",
      result.asString());

  ASSERT_EQ(size_t(7), result2.size());
  ASSERT_EQ(
      "[(WordId: 1, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 3, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 1, Score: 1, Pos: 1), ...]",
      result2.asString());

  ASSERT_EQ(size_t(0), result3.size());
  ASSERT_EQ(size_t(0), result4.size());
}

// _____________________________________________________________________________
TEST(EngineTest, aggregatePostingListTest)
{
  // Setup.
  PostingList postings;
  PostingList postings2;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  postings.push_back(TextPosting(entityId0 + 3, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  // Do the test.
  Engine engine;
  EntityList result;
  EntityList result2;
  EntityList result3;
  EntityList result4;
  engine.aggregatePostingList<SumAggregator<AggregatedScore> > (postings,
      &result);
  engine.aggregatePostingList<SumAggregator<AggregatedScore> > (postings2,
      &result2);
  engine.aggregatePostingList<PlusOneAggregator<AggregatedScore> > (postings,
      &result3);
  engine.aggregatePostingList<PlusOneAggregator<AggregatedScore> > (postings2,
       &result4);

  // Make assertions.
  ASSERT_EQ(size_t(3), result.size());
  ASSERT_EQ(entityId0, result[0]._id);
  ASSERT_EQ(score * 2, result[0]._score);
  ASSERT_EQ(entityId0 + 1, result[1]._id);
  ASSERT_EQ(score * 1, result[1]._score);
  ASSERT_EQ(entityId0 + 3, result[2]._id);
  ASSERT_EQ(score * 1, result[2]._score);

  ASSERT_EQ(size_t(0), result2.size());

  ASSERT_EQ(size_t(3), result3.size());
  ASSERT_EQ(entityId0, result3[0]._id);
  ASSERT_EQ(score + 1, result3[0]._score);
  ASSERT_EQ(entityId0 + 1, result3[1]._id);
  ASSERT_EQ(score, result3[1]._score);
  ASSERT_EQ(entityId0 + 3, result3[2]._id);
  ASSERT_EQ(score, result3[2]._score);

  ASSERT_EQ(size_t(0), result4.size());
}

// _____________________________________________________________________________
TEST(EngineTest, joinPostingListsOnContextIdTest)
{
  // Setup.
  PostingList l1;
  PostingList l2;
  PostingList l3;
  PostingList l4;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  l1.push_back(TextPosting(wordId0, contextId0, score, pos));
  l1.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  l1.push_back(TextPosting(entityId0, contextId0, score, pos));
  l1.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  l1.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));

  l2.push_back(TextPosting(entityId0, contextId0 + 1, score, pos));
  l2.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  l2.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  l2.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  l3.push_back(TextPosting(entityId0, contextId0, score, pos));
  l3.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  l3.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  l3.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  // Do the action.
  Engine engine;
  PostingList result1;
  PostingList result2;
  PostingList result3;
  PostingList result4;

  engine.joinPostingListsOnContextId(l1, l2, &result1);
  engine.joinPostingListsOnContextId(l1, l3, &result2);
  engine.joinPostingListsOnContextId(l2, l3, &result3);
  engine.joinPostingListsOnContextId(l1, l4, &result4);

  // Make assertions:
  ASSERT_EQ(size_t(3), result1.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 1, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 1, Score: 1, Pos: 1), "
      "(EntityId: 0, ContextId: 1, Score: 1, Pos: 1)]",
      result1.asString());

  ASSERT_EQ(size_t(4), result2.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 0, ContextId: 0, Score: 1, Pos: 1), ...]",
      result2.asString());

  ASSERT_EQ(size_t(6), result3.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 2, Score: 1, Pos: 1), "
      "(WordId: 0, ContextId: 2, Score: 1, Pos: 1), "
      "(EntityId: 0, ContextId: 2, Score: 1, Pos: 1), ...]",
      result3.asString());

  ASSERT_EQ("[]", result4.asString());
  ASSERT_EQ(size_t(0), result4.size());
}

// _____________________________________________________________________________
TEST(EngineTest, joinAndAggregateEntityListsTest)
{
  // Setup.
  EntityList l1;
  EntityList l2;
  EntityList l3;
  EntityList l4;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Score score = 1;
  l1.push_back(EntityWithScore(entityId0, score));
  l1.push_back(EntityWithScore(entityId0 + 1, 2 * score));
  l1.push_back(EntityWithScore(entityId0 + 2, score));

  l2.push_back(EntityWithScore(entityId0, score));
  l2.push_back(EntityWithScore(entityId0 + 1, score));
  l2.push_back(EntityWithScore(entityId0 + 3, score));

  l3.push_back(EntityWithScore(entityId0 + 3, score));

  // Do the action.
  Engine engine;
  EntityList result;
  EntityList result2;
  EntityList result3;
  EntityList result4;
  EntityList result5;
  engine.intersectEntityLists(l1, l2, &result);
  engine.intersectEntityLists(l2, l1, &result2);
  engine.intersectEntityLists(l2, l2, &result3);
  engine.intersectEntityLists(l3, l2, &result4);
  engine.intersectEntityLists(l1, l4, &result5);

  // Make assertions:
  ASSERT_EQ(size_t(2), result.size());
  ASSERT_EQ("[(EntityId: 0, Score: 2), (EntityId: 1, Score: 3)]",
      result.asString());

  ASSERT_EQ(size_t(2), result2.size());
  ASSERT_EQ("[(EntityId: 0, Score: 2), (EntityId: 1, Score: 3)]",
      result2.asString());

  ASSERT_EQ(size_t(3), result3.size());
  ASSERT_EQ("[(EntityId: 0, Score: 2), (EntityId: 1, Score: 2), "
      "(EntityId: 3, Score: 2)]", result3.asString());

  ASSERT_EQ(size_t(1), result4.size());
  ASSERT_EQ("[(EntityId: 3, Score: 2)]", result4.asString());

  ASSERT_EQ(size_t(0), result5.size());
  ASSERT_EQ("[]", result5.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, filterPostingListByEntityListTest)
{
  // Setup.
  PostingList postings;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0 + 2, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  PostingList postings2;

  EntityList entities;
  entities.push_back(EntityWithScore(entityId0, score));
  entities.push_back(EntityWithScore(entityId0 + 2, score));

  EntityList entities2;
  entities2.push_back(EntityWithScore(entityId0 + 2, score));

  EntityList entities3;

  // Do the action.
  Engine engine;
  PostingList result;
  PostingList result2;
  PostingList result3;
  PostingList result4;
  engine.filterPostingListByEntityList(postings, entities, &result);
  engine.filterPostingListByEntityList(postings, entities2, &result2);
  engine.filterPostingListByEntityList(postings, entities3, &result3);
  engine.filterPostingListByEntityList(postings2, entities, &result4);

  // Make assertions.
  ASSERT_EQ(size_t(3), result.size());
  ASSERT_EQ("[(EntityId: 0, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 2, ContextId: 1, Score: 1, Pos: 1), "
      "(EntityId: 0, ContextId: 2, Score: 1, Pos: 1)]", result.asString());

  ASSERT_EQ(size_t(1), result2.size());
  ASSERT_EQ("[(EntityId: 2, ContextId: 1, Score: 1, Pos: 1)]",
      result2.asString());

  ASSERT_EQ(size_t(0), result3.size());
  ASSERT_EQ("[]", result3.asString());

  ASSERT_EQ(size_t(0), result4.size());
  ASSERT_EQ("[]", result4.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, filterPostingListByEntityListKeepWordPostingsTest)
{
  // Setup.
  PostingList postings;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0 + 2, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  PostingList postings2;
  EntityList entities;
  entities.push_back(EntityWithScore(entityId0, score));
  entities.push_back(EntityWithScore(entityId0 + 2, score));

  EntityList entities2;
  entities2.push_back(EntityWithScore(entityId0 + 2, score));

  EntityList entities3;

  // Do the action.
  Engine engine;
  PostingList result;
  PostingList result2;
  PostingList result3;
  PostingList result4;
  engine.filterPostingListByEntityListKeepWordPostings(
      postings, entities, &result);
  engine.filterPostingListByEntityListKeepWordPostings(
      postings, entities2, &result2);
  engine.filterPostingListByEntityListKeepWordPostings(
      postings, entities3, &result3);
  engine.filterPostingListByEntityListKeepWordPostings(
      postings2, entities, &result4);

  // Make assertions.
  ASSERT_EQ(size_t(9), result.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 0, ContextId: 0, Score: 1, Pos: 1), ...]", result.asString());

  ASSERT_EQ(size_t(3), result2.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 1, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 1, Score: 1, Pos: 1), "
      "(EntityId: 2, ContextId: 1, Score: 1, Pos: 1)]", result2.asString());

  ASSERT_EQ(size_t(0), result3.size());
  ASSERT_EQ("[]", result3.asString());

  ASSERT_EQ(size_t(0), result4.size());
  ASSERT_EQ("[]", result4.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, filterPostingsByEntityIdTest)
{
  // Setup.
  PostingList postings;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0 + 2, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(entityId0 + 1, contextId0 + 2, score, pos));

  PostingList postings2;

  // Do the action.
  Engine engine;
  PostingList result;
  PostingList result2;
  PostingList result3;
  PostingList result4;
  engine.filterPostingsByEntityId(postings, entityId0, &result);
  engine.filterPostingsByEntityId(postings, entityId0 + 1, &result2);
  engine.filterPostingsByEntityId(postings, entityId0 + 5, &result3);

  engine.filterPostingsByEntityId(postings2, entityId0, &result4);

  // Make assertions.
  ASSERT_EQ(size_t(6), result.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 0, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 0, Score: 1, Pos: 1), "
      "(EntityId: 0, ContextId: 0, Score: 1, Pos: 1), ...]", result.asString());

  ASSERT_EQ(size_t(3), result2.size());
  ASSERT_EQ("[(WordId: 0, ContextId: 2, Score: 1, Pos: 1), "
      "(WordId: 1, ContextId: 2, Score: 1, Pos: 1), "
      "(EntityId: 1, ContextId: 2, Score: 1, Pos: 1)]", result2.asString());

  ASSERT_EQ(size_t(0), result3.size());
  ASSERT_EQ("[]", result3.asString());

  ASSERT_EQ(size_t(0), result4.size());
  ASSERT_EQ("[]", result4.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, getRelationRhsBySingleLhsTest)
{
  // Setup.
  Relation relation;
  Relation relation2;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);

  relation.push_back(RelationEntry(entityId0, entityId0 + 1));
  relation.push_back(RelationEntry(entityId0 + 1, entityId0));
  relation.push_back(RelationEntry(entityId0 + 1, entityId0 + 3));
  relation.push_back(RelationEntry(entityId0 + 2, entityId0 + 2));

  // Do the action.
  Engine engine;
  EntityList result;
  EntityList result2;
  EntityList result3;
  engine.getRelationRhsBySingleLhs(relation, entityId0 + 1, &result);
  engine.getRelationRhsBySingleLhs(relation, entityId0 + 4, &result2);
  engine.getRelationRhsBySingleLhs(relation2, entityId0 + 1, &result3);

  // Make assertions.
  ASSERT_EQ(size_t(2), result.size());
  ASSERT_EQ(""
      "[(EntityId: 0, Score: 1), "
      "(EntityId: 3, Score: 1)]",
      result.asString());

  ASSERT_EQ(size_t(0), result2.size());
  ASSERT_EQ("[]", result2.asString());

  ASSERT_EQ(size_t(0), result3.size());
  ASSERT_EQ("[]", result3.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, getTopKContextsWithHighlightsTest)
{
  // Setup.
  PostingList postings;
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId0 = getFirstId(IdType::WORD_ID);
  Id contextId0 = getFirstId(IdType::CONTEXT_ID);
  Position pos = 1;
  Score score = 1;
  postings.push_back(TextPosting(wordId0, contextId0, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0, score, pos + 1));
  postings.push_back(TextPosting(entityId0, contextId0, score, pos + 2));
  postings.push_back(TextPosting(wordId0, contextId0 + 1, score, pos + 2));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 1, score, pos));
  postings.push_back(TextPosting(entityId0, contextId0 + 1, score, pos + 1));
  postings.push_back(TextPosting(wordId0, contextId0 + 2, score, pos));
  postings.push_back(TextPosting(wordId0 + 1, contextId0 + 2, score, pos + 1));
  postings.push_back(TextPosting(entityId0, contextId0 + 2, 2 * score, pos));

  PostingList postingsEmpty;

  // Do the action.
  Engine engine;
  HitList result;
  HitList result2;
  HitList result3;
  HitList result4;
  HitList result5;

  engine.getTopKContextsWithHighlights(postings, 3, &result);
  engine.getTopKContextsWithHighlights(postings, 4, &result2);
  engine.getTopKContextsWithHighlights(postings, 1, &result3);
  engine.getTopKContextsWithHighlights(postings, 0, &result4);
  engine.getTopKContextsWithHighlights(postingsEmpty, 2, &result5);

  // Make assertions.
  ASSERT_EQ(size_t(3), result.size());
  ASSERT_EQ(
      "[(ContextId: 2, Score: 4), "
      "(ContextId: 0, Score: 3), "
      "(ContextId: 1, Score: 3)]", result.asString());

  ASSERT_EQ(size_t(3), result2.size());
  ASSERT_EQ(result.asString(), result2.asString());

  ASSERT_EQ(size_t(1), result3.size());
  ASSERT_EQ("[(ContextId: 2, Score: 4)]", result3.asString());

  ASSERT_EQ(size_t(0), result4.size());
  ASSERT_EQ("[]", result4.asString());

  ASSERT_EQ(size_t(0), result5.size());
  ASSERT_EQ("[]", result5.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, getTopKEntitiesTest)
{
  // Setup.
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  AggregatedScore score = 1;

  EntityList entities;
  EntityList entitiesEmpty;

  entities.push_back(EntityWithScore(entityId0, score));
  entities.push_back(EntityWithScore(entityId0 + 1, score + 2));
  entities.push_back(EntityWithScore(entityId0 + 3, score + 3));
  entities.push_back(EntityWithScore(entityId0 + 4, score - 1));


  // Do the action.
  Engine engine;
  EntityList result;
  EntityList result2;
  EntityList result3;
  EntityList result4;
  EntityList result5;

  engine.getTopKEntities(entities, 3, &result);
  engine.getTopKEntities(entities, 20, &result2);
  engine.getTopKEntities(entities, 1, &result3);
  engine.getTopKEntities(entities, 0, &result4);
  engine.getTopKEntities(entitiesEmpty, 2, &result5);

  // Make assertions.
  ASSERT_EQ(size_t(3), result.size());
  ASSERT_EQ(
      "[(EntityId: 3, Score: 4), "
      "(EntityId: 1, Score: 3), "
      "(EntityId: 0, Score: 1)]", result.asString());

  ASSERT_EQ(size_t(4), result2.size());
  ASSERT_EQ(
      "[(EntityId: 3, Score: 4), "
      "(EntityId: 1, Score: 3), "
      "(EntityId: 0, Score: 1), ...]", result2.asString());

  ASSERT_EQ(size_t(1), result3.size());
  ASSERT_EQ("[(EntityId: 3, Score: 4)]", result3.asString());

  ASSERT_EQ(size_t(0), result4.size());
  ASSERT_EQ("[]", result4.asString());

  ASSERT_EQ(size_t(0), result5.size());
  ASSERT_EQ("[]", result5.asString());
}

// _____________________________________________________________________________
TEST(EngineTest, filterEntityListByIdRangeTest)
{
  // Setup.
  Id entityId0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  AggregatedScore score = 1;

  EntityList entities;
  EntityList entitiesEmpty;

  entities.push_back(EntityWithScore(entityId0, score));
  entities.push_back(EntityWithScore(entityId0 + 1, score));
  entities.push_back(EntityWithScore(entityId0 + 3, score));
  entities.push_back(EntityWithScore(entityId0 + 4, score));
  entities.push_back(EntityWithScore(entityId0 + 5, score));
  entities.push_back(EntityWithScore(entityId0 + 8, score));
  entities.push_back(EntityWithScore(entityId0 + 10, score));


  IdRange fullRange = IdRange(entityId0, entityId0 +10);
  IdRange subRangeMiddle = IdRange(entityId0 + 2, entityId0 + 5);
  IdRange subRangeBottom = IdRange(entityId0, entityId0 + 5);
  IdRange subRangeTop = IdRange(entityId0 + 6, entityId0 + 12);
  IdRange smaller = IdRange(entityId0 - 5, entityId0 - 3);
  IdRange larger = IdRange(entityId0 + 11, entityId0 + 12);
  IdRange single = IdRange(entityId0 + 3, entityId0 + 3);

  // Do the action and make assertions.
  Engine engine;
  EntityList result;

  engine.filterEntityListByIdRange(entitiesEmpty, fullRange, &result);
  ASSERT_EQ(size_t(0), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, fullRange, &result);
  ASSERT_EQ(entities.size(), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, subRangeMiddle, &result);
  ASSERT_EQ(size_t(3), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, subRangeBottom, &result);
  ASSERT_EQ(size_t(5), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, subRangeTop, &result);
  ASSERT_EQ(size_t(2), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, single, &result);
  ASSERT_EQ(size_t(1), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, larger, &result);
  ASSERT_EQ(size_t(0), result.size());

  result.clear();
  engine.filterEntityListByIdRange(entities, smaller, &result);
  ASSERT_EQ(size_t(0), result.size());
}

// _____________________________________________________________________________
TEST(EngineTest, aggregateWordsInPostingListTest)
{
  PostingList emptyInput;
  PostingList typicalInput;
  Id word0 = getFirstId(IdType::WORD_ID);
  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id context0 = getFirstId(IdType::CONTEXT_ID);

  typicalInput.push_back(TextPosting(word0, context0, 1, 0));
  typicalInput.push_back(TextPosting(entity0, context0, 1, 1));
  typicalInput.push_back(TextPosting(word0 + 3, context0, 1, 1));
  typicalInput.push_back(TextPosting(word0 + 2, context0, 1, 2));
  typicalInput.push_back(TextPosting(word0 + 2, context0 + 1, 1, 0));
  typicalInput.push_back(TextPosting(entity0 + 1, context0 + 1, 1, 1));
  typicalInput.push_back(TextPosting(word0 + 10, context0 + 1, 1, 1));
  typicalInput.push_back(TextPosting(word0, context0 + 1, 1, 2));
  typicalInput.push_back(TextPosting(word0, context0 + 1, 1, 3));

  Engine engine;
  EntityList result;
  engine.aggregateWordsInPostingList(emptyInput, &result);

  ASSERT_EQ(size_t(0), result.size());

  engine.aggregateWordsInPostingList(typicalInput, &result);
  ASSERT_EQ(size_t(4), result.size());
  ASSERT_EQ(word0, result[0]._id);
  ASSERT_EQ(size_t(3), result[0]._score);
  ASSERT_EQ(word0 + 2, result[1]._id);
  ASSERT_EQ(size_t(2), result[1]._score);
  ASSERT_EQ(word0 + 3, result[2]._id);
  ASSERT_EQ(size_t(1), result[2]._score);
  ASSERT_EQ(word0 + 10, result[3]._id);
  ASSERT_EQ(size_t(1), result[3]._score);
}

// _____________________________________________________________________________
TEST(EngineTest, intersectEntityListsTest)
{
  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);

  EntityList emptyInput;
  EntityList emptyInput2;

  EntityList input1;
  input1.push_back(EntityWithScore(entity0, 1));
  input1.push_back(EntityWithScore(entity0 + 1, 1));
  input1.push_back(EntityWithScore(entity0 + 2, 1));
  input1.push_back(EntityWithScore(entity0 + 4, 1));

  EntityList input2;
  input2.push_back(EntityWithScore(entity0 + 2, 1));
  input2.push_back(EntityWithScore(entity0 + 3, 1));
  input2.push_back(EntityWithScore(entity0 + 4, 5));

  EntityList input3;
  input3.push_back(EntityWithScore(entity0 + 5, 1));
  input3.push_back(EntityWithScore(entity0 + 6, 1));

  EntityList result;

  Engine engine;

  // Both empty
  engine.intersectEntityLists(emptyInput, emptyInput2, &result);
  ASSERT_EQ(size_t(0), result.size());
  result.clear();
  // One empty
  engine.intersectEntityLists(emptyInput, input1, &result);
  ASSERT_EQ(size_t(0), result.size());
  result.clear();
  engine.intersectEntityLists(input2, emptyInput, &result);
  ASSERT_EQ(size_t(0), result.size());
  result.clear();

  // No match, empty result
  engine.intersectEntityLists(input3, input1, &result);
  ASSERT_EQ(size_t(0), result.size());
  result.clear();
  engine.intersectEntityLists(input2, input3, &result);
  ASSERT_EQ(size_t(0), result.size());
  result.clear();

  // Matches
  engine.intersectEntityLists(input1, input2, &result);
  ASSERT_EQ(size_t(2), result.size());
  ASSERT_EQ(entity0 + 2, result[0]._id);
  ASSERT_EQ(size_t(2), result[0]._score);
  ASSERT_EQ(entity0 + 4, result[1]._id);
  ASSERT_EQ(size_t(6), result[1]._score);
  result.clear();
  engine.intersectEntityLists(input2, input1, &result);
  ASSERT_EQ(size_t(2), result.size());
  ASSERT_EQ(entity0 + 2, result[0]._id);
  ASSERT_EQ(size_t(2), result[0]._score);
  ASSERT_EQ(entity0 + 4, result[1]._id);
  ASSERT_EQ(size_t(6), result[1]._score);
  result.clear();
}

// _____________________________________________________________________________
TEST(EngineTest, getRelationRhsByEntityListLhsTest)
{
  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);

  Relation emptyRelation;
  Relation relation;
  relation.push_back(RelationEntry(entity0 + 10, entity0 + 1));
  relation.push_back(RelationEntry(entity0 + 10, entity0 + 2));
  relation.push_back(RelationEntry(entity0 + 10, entity0 + 6));
  relation.push_back(RelationEntry(entity0 + 11, entity0 + 3));
  relation.push_back(RelationEntry(entity0 + 11, entity0 + 4));
  relation.push_back(RelationEntry(entity0 + 11, entity0 + 9));
  relation.push_back(RelationEntry(entity0 + 12, entity0 + 9));

  EntityList emptyEL;
  EntityList lowerEL;
  lowerEL.push_back(EntityWithScore(entity0, 1));
  EntityList upperEL;
  upperEL.push_back(EntityWithScore(entity0 + 50, 1));
  EntityList insideEL;
  insideEL.push_back(EntityWithScore(entity0 + 11, 1));
  insideEL.push_back(EntityWithScore(entity0 + 12, 1));
  EntityList lowEL;
  lowEL.push_back(EntityWithScore(entity0 + 9, 1));
  lowEL.push_back(EntityWithScore(entity0 + 10, 1));
  lowEL.push_back(EntityWithScore(entity0 + 11, 1));
  EntityList upEL;
  upEL.push_back(EntityWithScore(entity0 + 11, 1));
  upEL.push_back(EntityWithScore(entity0 + 12, 1));
  upEL.push_back(EntityWithScore(entity0 + 13, 1));

  Engine engine;

  EntityList result;
  Relation matching;

  // Empty relation
  engine.getRelationRhsByEntityListLhs(emptyRelation, insideEL, &matching,
      &result);
  ASSERT_EQ(size_t(0), result.size());
  ASSERT_EQ(size_t(0), matching.size());
  result.clear();
  matching.clear();

  // Both empty
  engine.getRelationRhsByEntityListLhs(emptyRelation, emptyEL, &matching,
      &result);
  ASSERT_EQ(size_t(0), result.size());
  ASSERT_EQ(size_t(0), matching.size());
  result.clear();
  matching.clear();

  // Empty EL
  engine.getRelationRhsByEntityListLhs(relation, emptyEL, &matching,
      &result);
  ASSERT_EQ(size_t(0), result.size());
  ASSERT_EQ(size_t(0), matching.size());
  result.clear();
  matching.clear();

  // No match EL too low
  engine.getRelationRhsByEntityListLhs(relation, lowerEL, &matching,
      &result);
  ASSERT_EQ(size_t(0), result.size());
  ASSERT_EQ(size_t(0), matching.size());
  result.clear();
  matching.clear();

  // No match EL too high
  engine.getRelationRhsByEntityListLhs(relation, upperEL, &matching,
      &result);
  ASSERT_EQ(size_t(0), result.size());
  ASSERT_EQ(size_t(0), matching.size());
  result.clear();
  matching.clear();

  // Match: entities lower and matching
  engine.getRelationRhsByEntityListLhs(relation, lowEL, &matching, &result);
  ASSERT_EQ(size_t(6), result.size());
  ASSERT_EQ(size_t(6), matching.size());
  result.clear();
  matching.clear();

  // Match: entities higher and matching
  engine.getRelationRhsByEntityListLhs(relation, upEL, &matching, &result);
  ASSERT_EQ(size_t(3), result.size());
  ASSERT_EQ(size_t(4), matching.size());
  result.clear();
  matching.clear();

  // Match: entities inside and matching
  engine.getRelationRhsByEntityListLhs(relation, insideEL, &matching, &result);
  ASSERT_EQ(size_t(3), result.size());
  ASSERT_EQ(size_t(4), matching.size());
  result.clear();
  matching.clear();
}

// _____________________________________________________________________________
TEST(EngineTest, getTopKContextsWithHighlightsAndEntitiesTest)
{
  Engine engine;
}

// _____________________________________________________________________________
TEST(EngineTest, getAnyMatchingEntitiyTest)
{
  Engine engine;
}

// _____________________________________________________________________________
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
