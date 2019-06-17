// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "./EntityList.h"
#include "./Identifiers.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
class EntityListTest: public ::testing::Test
{
  virtual void SetUp()
  {
    Id entityIdZero = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);

    // L1 is wellformed
    _l1.push_back(EntityWithScore(entityIdZero, 0));
    _l1.push_back(EntityWithScore(entityIdZero + 1, 0));
    _l1.push_back(EntityWithScore(entityIdZero + 6, 3));
    _l1.push_back(EntityWithScore(entityIdZero + 8, 2));
    _l1.push_back(EntityWithScore(entityIdZero + 9, 5));

    // L2 isn't
    _l2.push_back(EntityWithScore(entityIdZero, 0));
    _l2.push_back(EntityWithScore(entityIdZero + 1, 0));
    _l2.push_back(EntityWithScore(entityIdZero + 1, 3));

    // Neither is l3
    _l3.push_back(EntityWithScore(entityIdZero, 0));
    _l3.push_back(EntityWithScore(entityIdZero + 2, 0));
    _l3.push_back(EntityWithScore(entityIdZero + 1, 3));
  }

  virtual void TearDown()
  {
  }

  public:
    EntityList _l1;
    EntityList _l2;
    EntityList _l3;
};

TEST_F(EntityListTest, isWellFormedTest)
{
  ASSERT_TRUE(_l1.isWellFormed());
  ASSERT_FALSE(_l2.isWellFormed());
  ASSERT_FALSE(_l3.isWellFormed());
}

TEST_F(EntityListTest, asStringTest)
{
  ASSERT_EQ(string(
      "[(EntityId: 0, Score: 0), ") +
      "(EntityId: 1, Score: 0), " +
      "(EntityId: 6, Score: 3), ...]",
      _l1.asString());
  ASSERT_EQ(string(
      "[(EntityId: 0, Score: 0), ") +
      "(EntityId: 1, Score: 0), " +
      "(EntityId: 1, Score: 3)]",
      _l2.asString());
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
