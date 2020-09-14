// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>

#include "./Identifiers.h"

using std::string;

namespace ad_semsearch
{
TEST(IdentifiersTest, testGetFirstId)
{
  size_t zero = 0;
  size_t one = 1;
  size_t msbSet = 2;
  size_t highestBitIndex = (sizeof(Id) * 8) - 1;
  for (size_t i = 2; i <= highestBitIndex; ++i) msbSet *= 2;

  Id word = getFirstId(IdType::WORD_ID);
  ASSERT_EQ(zero, word);
  ASSERT_EQ(one, ++word);

  Id entity = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  ASSERT_EQ(msbSet, entity);
  ASSERT_EQ(msbSet + one , ++entity);

  Id context = getFirstId(IdType::CONTEXT_ID);
  ASSERT_EQ(zero, context);
  ASSERT_EQ(one, ++context);
}

TEST(IdentifiersTest, testIsIdOfType)
{
  size_t zero = 0;
  size_t billion = 1000000000;
  size_t msbSet = 2;
  size_t highestBitIndex = (sizeof(Id) * 8) - 1;

  for (size_t i = 2; i <= highestBitIndex; ++i) msbSet *= 2;

  ASSERT_TRUE(isIdOfType(zero, IdType::WORD_ID));
  ASSERT_TRUE(isIdOfType(billion, IdType::WORD_ID));

  ASSERT_TRUE(isIdOfType(zero, IdType::CONTEXT_ID));
  ASSERT_TRUE(isIdOfType(billion, IdType::CONTEXT_ID));

  ASSERT_FALSE(isIdOfType(zero, IdType::ONTOLOGY_ELEMENT_ID));
  ASSERT_FALSE(isIdOfType(billion, IdType::ONTOLOGY_ELEMENT_ID));

  ASSERT_TRUE(isIdOfType(msbSet, IdType::ONTOLOGY_ELEMENT_ID));
  ASSERT_TRUE(isIdOfType(msbSet + billion, IdType::ONTOLOGY_ELEMENT_ID));

  ASSERT_FALSE(isIdOfType(msbSet, IdType::WORD_ID));
  ASSERT_FALSE(isIdOfType(msbSet + billion, IdType::WORD_ID));

  ASSERT_FALSE(isIdOfType(msbSet, IdType::CONTEXT_ID));
  ASSERT_FALSE(isIdOfType(msbSet + billion, IdType::CONTEXT_ID));
}

TEST(IdentifiersTest, getPureValueTest)
{
  // Setup.
  Id entityId = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id wordId = getFirstId(IdType::WORD_ID);
  ASSERT_GT(entityId, wordId);
  entityId += 10;
  wordId += 5;
  ASSERT_GT(entityId, Id(10));

  // Actual test of the method under test.
  ASSERT_NE(Id(10), entityId);
  ASSERT_EQ(Id(10), getPureValue(entityId));
  ASSERT_EQ(Id(5), wordId);
  ASSERT_EQ(Id(5), getPureValue(wordId));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
