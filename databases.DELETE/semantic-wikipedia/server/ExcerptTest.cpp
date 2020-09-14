// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "./Excerpt.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
TEST(ExcerptTest, delimiterStartsAtPosTest)
{
  string text1 = string(DOCSFILE_POS_DELIMITER) + " " + DOCSFILE_POS_DELIMITER
      + " hi@ tree" + DOCSFILE_POS_DELIMITER;
  string text2 = DOCSFILE_POS_DELIMITER;
  string text3 = "";

  ASSERT_TRUE(Excerpt::delimiterStartsAtPos(text1, 0));
  ASSERT_TRUE(Excerpt::delimiterStartsAtPos(text1, 3));
  ASSERT_TRUE(Excerpt::delimiterStartsAtPos(text1, 14));

  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 1));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 2));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 4));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 7));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 8));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 9));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 15));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 16));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text1, 20));

  ASSERT_TRUE(Excerpt::delimiterStartsAtPos(text2, 0));
  ASSERT_FALSE(Excerpt::delimiterStartsAtPos(text3, 0));
}
TEST(ExcerptTest, getExcerptWithHighlightingTest)
{
  string beginning = "1\tu:X\tt:X\t";
  string text1 = beginning + "";
  string text2 = beginning + DOCSFILE_POS_DELIMITER + " Hello"
      + DOCSFILE_POS_DELIMITER + " Albert Einstein" + DOCSFILE_POS_DELIMITER
      + ".";
  string text3 = beginning + "X" + DOCSFILE_POS_DELIMITER;
  string text4 = beginning + "X";

  Excerpt excerpt(text1);
  Excerpt::Highlights highlights;
  highlights.push_back(2);
  Excerpt::Highlights highlights2;
  Excerpt::Highlights highlights3;
  highlights3.push_back(0);

  excerpt.setHighlights(highlights);
  ASSERT_EQ("", excerpt.getExcerptWithHighlighting());

  Excerpt excerpt2(text2, highlights2);
  ASSERT_EQ(" Hello Albert Einstein.", excerpt2.getExcerptWithHighlighting());

  excerpt2.setHighlights(highlights);
  ASSERT_EQ(" Hello<hl> Albert Einstein</hl>.",
      excerpt2.getExcerptWithHighlighting());

  excerpt2.parseRawExcerpt(text3);
  ASSERT_EQ("X", excerpt2.getExcerptWithHighlighting());
  excerpt2.setHighlights(highlights3);
  ASSERT_EQ("<hl>X</hl>", excerpt2.getExcerptWithHighlighting());
  excerpt2.parseRawExcerpt(text4);
  ASSERT_EQ("<hl>X</hl>", excerpt2.getExcerptWithHighlighting());
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
