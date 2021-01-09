// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#include <gtest/gtest.h>
#include "./Vocabulary.h"

// Fixture class, which sets up a small test vocabulary.
class VocabularyTest : public ::testing::Test
{
 protected:
  Vocabulary _vocabulary;
  Vocabulary _vocabulary2;
  virtual void SetUp()
  {
    _vocabulary._words.resize(3);
    _vocabulary._words[0] = "aa";
    _vocabulary._words[1] = "bb";
    _vocabulary._words[2] = "cc";

    _vocabulary2._words.resize(5);
    _vocabulary2._words[0] = "a";
    _vocabulary2._words[1] = "b";
    _vocabulary2._words[2] = string("C") + wordPartSep + string("1234")
                                         + wordPartSep + string("a");
    _vocabulary2._words[3] = string("C") + wordPartSep + string("2345")
                                         + wordPartSep + string("a");
    _vocabulary2._words[4] = string("C") + wordPartSep + string("1234")
                                         + wordPartSep + string("c");
  }
};

// Test Vocabulary::findWord.
TEST_F(VocabularyTest, findWord)
{
  ASSERT_EQ((unsigned) 0, _vocabulary.findWord("aa"));
  ASSERT_EQ((unsigned) 1, _vocabulary.findWord("bb"));
  ASSERT_EQ((unsigned) 2, _vocabulary.findWord("cc"));

  ASSERT_EQ((unsigned) 0, _vocabulary.findWord("a"));
  ASSERT_EQ((unsigned) 1, _vocabulary.findWord("b"));
  ASSERT_EQ((unsigned) 2, _vocabulary.findWord("c"));
  ASSERT_EQ((unsigned) 3, _vocabulary.findWord("d"));

  ASSERT_EQ((unsigned) 1, _vocabulary.findWord("aaa"));
  ASSERT_EQ((unsigned) 2, _vocabulary.findWord("bbb"));
  ASSERT_EQ((unsigned) 3, _vocabulary.findWord("ccc"));
}

// Test Vocabulary::computeTopCompletion.
TEST_F(VocabularyTest, precomputeWordIdMap)
{
  _vocabulary2.precomputeWordIdMap();
  ASSERT_FALSE(_vocabulary2.isWordMapTrivial());
  ASSERT_EQ(0, _vocabulary2._wordIdMap[0]);
  ASSERT_EQ(1, _vocabulary2._wordIdMap[1]);
  ASSERT_EQ(0, _vocabulary2._wordIdMap[2]);
  ASSERT_EQ(0, _vocabulary2._wordIdMap[3]);
  ASSERT_EQ(4, _vocabulary2._wordIdMap[4]);
}

// Test Vocabulary::computeTopCompletion.
TEST_F(VocabularyTest, mappedWordId)
{
  // Before call to precomputeWordIdMap mappedWordId is the identity function.
  ASSERT_TRUE(_vocabulary2.isWordMapTrivial());
  ASSERT_EQ(0, _vocabulary2.mappedWordId(0));
  ASSERT_EQ(1, _vocabulary2.mappedWordId(1));
  ASSERT_EQ(2, _vocabulary2.mappedWordId(2));
  ASSERT_EQ(3, _vocabulary2.mappedWordId(3));
  ASSERT_EQ(4, _vocabulary2.mappedWordId(4));
  ASSERT_EQ(-1, _vocabulary2.mappedWordId(-1));
  ASSERT_EQ(5, _vocabulary2.mappedWordId(5));

  // After call to precomputeWordIdMap mappedWordId should behave as in
  // VocabularyTest::precomputeWordIdMap above.
  _vocabulary2.precomputeWordIdMap();
  ASSERT_FALSE(_vocabulary2.isWordMapTrivial());
  ASSERT_EQ(0, _vocabulary2.mappedWordId(0));
  ASSERT_EQ(1, _vocabulary2.mappedWordId(1));
  ASSERT_EQ(0, _vocabulary2.mappedWordId(2));
  ASSERT_EQ(0, _vocabulary2.mappedWordId(3));
  ASSERT_EQ(4, _vocabulary2.mappedWordId(4));
  ASSERT_EQ(-1, _vocabulary2.mappedWordId(-1));
  ASSERT_EQ(5, _vocabulary2.mappedWordId(5));
}

// _____________________________________________________________________________
int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
