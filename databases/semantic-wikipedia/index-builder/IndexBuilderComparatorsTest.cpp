// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../server/Identifiers.h"
#include "./Postings.h"
#include "./IndexBuilderComparators.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
TEST(PostingComparatorEntitiesMixedInText, testOperator)
{
  vector<BasicPosting> postings;
  // 2nd when last, 3rd otherwise
  postings.push_back(BasicPosting(1, getFirstId(IdType::WORD_ID), 2));
  // 3rd when last, 1st otherwise
  postings.push_back(
      BasicPosting(1, getFirstId(IdType::ONTOLOGY_ELEMENT_ID), 1));
  // first when last, 2nd otherwise
  postings.push_back(BasicPosting(1, getFirstId(IdType::WORD_ID) + 2, 1));
  // 4th
  postings.push_back(BasicPosting(2, getFirstId(IdType::WORD_ID), 1));
  // 5th when last 6th otherwise
  postings.push_back(BasicPosting(2, getFirstId(IdType::WORD_ID) + 1, 5));
  // 6th when last 5th otherwise
  postings.push_back(
      BasicPosting(2, getFirstId(IdType::ONTOLOGY_ELEMENT_ID), 5));

  PostingComparatorEntitiesMixedIn<BasicPosting> compUnderTest;
  std::sort(postings.begin(), postings.end(), compUnderTest);
  ASSERT_TRUE(isIdOfType(postings[0]._wordId, IdType::ONTOLOGY_ELEMENT_ID));
  ASSERT_EQ(size_t(1), postings[1]._contextId);
  ASSERT_EQ(size_t(2), postings[2]._contextId);
  ASSERT_EQ(size_t(1), postings[3]._contextId);
  ASSERT_TRUE(isIdOfType(postings[4]._wordId, IdType::ONTOLOGY_ELEMENT_ID));
  ASSERT_EQ(size_t(5), postings[5]._contextId);
}
TEST(PostingComparatorEntitiesLast, testOperator)
{
  vector<BasicPosting> postings;
  // 2nd when last, 3rd otherwise
  postings.push_back(BasicPosting(1, getFirstId(IdType::WORD_ID), 2));
  // 3rd when last, 1st otherwise
  postings.push_back(
      BasicPosting(1, getFirstId(IdType::ONTOLOGY_ELEMENT_ID), 1));
  // first when last, 2nd otherwise
  postings.push_back(BasicPosting(1, getFirstId(IdType::WORD_ID) + 2, 1));
  // 4th
  postings.push_back(BasicPosting(2, getFirstId(IdType::WORD_ID), 1));
  // 5th when last 6th otherwise
  postings.push_back(BasicPosting(2, getFirstId(IdType::WORD_ID) + 1, 5));
  // 6th when last 5th otherwise
  postings.push_back(
      BasicPosting(2, getFirstId(IdType::ONTOLOGY_ELEMENT_ID), 5));

  PostingComparatorEntitiesLast<BasicPosting> compUnderTest;
  std::sort(postings.begin(), postings.end(), compUnderTest);
  ASSERT_EQ(size_t(1), postings[0]._contextId);
  ASSERT_EQ(size_t(2), postings[1]._contextId);
  ASSERT_TRUE(isIdOfType(postings[2]._wordId, IdType::ONTOLOGY_ELEMENT_ID));
  ASSERT_EQ(size_t(1), postings[3]._contextId);
  ASSERT_EQ(size_t(5), postings[4]._contextId);
  ASSERT_TRUE(isIdOfType(postings[5]._wordId, IdType::ONTOLOGY_ELEMENT_ID));
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
