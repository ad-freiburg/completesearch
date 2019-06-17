// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include "./Comparators.h"
#include "./Globals.h"

using std::string;

namespace ad_semsearch
{
TEST(EntitiesLastLessThanStringComparatorTest, testOperator)
{
  EntitiesLastLessThanStringComparator objectUnderTest;
  string e1 = string(ENTITY_PREFIX) + "abc";
  string e2 = string(ENTITY_PREFIX) + "bc";
  string e3 = string(ENTITY_PREFIX) + "bcd";
  string w1 = "abc";
  string w2 = "bc";
  string w3 = "bcd";

  ASSERT_TRUE(objectUnderTest(w1, e1));
  ASSERT_TRUE(objectUnderTest(w1, e2));
  ASSERT_TRUE(objectUnderTest(w1, e3));
  ASSERT_TRUE(objectUnderTest(w2, e1));
  ASSERT_TRUE(objectUnderTest(w2, e2));
  ASSERT_TRUE(objectUnderTest(w2, e3));
  ASSERT_TRUE(objectUnderTest(w3, e1));
  ASSERT_TRUE(objectUnderTest(w3, e2));
  ASSERT_TRUE(objectUnderTest(w3, e3));

  ASSERT_FALSE(objectUnderTest(e1, w1));
  ASSERT_FALSE(objectUnderTest(e1, w2));
  ASSERT_FALSE(objectUnderTest(e1, w3));
  ASSERT_FALSE(objectUnderTest(e2, w1));
  ASSERT_FALSE(objectUnderTest(e2, w2));
  ASSERT_FALSE(objectUnderTest(e2, w3));
  ASSERT_FALSE(objectUnderTest(e3, w1));
  ASSERT_FALSE(objectUnderTest(e3, w2));
  ASSERT_FALSE(objectUnderTest(e3, w3));

  ASSERT_TRUE(objectUnderTest(e1, e2));
  ASSERT_TRUE(objectUnderTest(e2, e3));
  ASSERT_TRUE(objectUnderTest(e1, e3));

  ASSERT_TRUE(objectUnderTest(w1, w2));
  ASSERT_TRUE(objectUnderTest(w2, w3));
  ASSERT_TRUE(objectUnderTest(w1, w3));

  ASSERT_FALSE(objectUnderTest(e1, e1));
  ASSERT_FALSE(objectUnderTest(e2, e2));
  ASSERT_FALSE(objectUnderTest(e3, e3));
  ASSERT_FALSE(objectUnderTest(w1, w1));
  ASSERT_FALSE(objectUnderTest(w2, w2));
  ASSERT_FALSE(objectUnderTest(w3, w3));
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}  // namespace
