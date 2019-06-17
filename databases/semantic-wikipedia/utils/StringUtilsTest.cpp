// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include "./StringUtils.h"

using std::string;

namespace ad_utility
{
TEST(StringUtilsTest, startWith)
{
  string patternAsString = "ab";
  const char* patternAsCharP = patternAsString.c_str();
  size_t patternSize = 2;

  string matchingText1 = "abc";
  string matchingText2 = "ab";
  string mismatchText1 = "";
  string mismatchText2 = "bcd";
  string mismatchText3 = "aa";

  ASSERT_TRUE(startsWith(matchingText1, patternAsString));
  ASSERT_TRUE(startsWith(matchingText1, patternAsCharP, patternSize));
  ASSERT_TRUE(startsWith(matchingText1, patternAsCharP));

  ASSERT_TRUE(startsWith(matchingText2, patternAsString));
  ASSERT_TRUE(startsWith(matchingText2, patternAsCharP, patternSize));
  ASSERT_TRUE(startsWith(matchingText2, patternAsCharP));

  ASSERT_FALSE(startsWith(mismatchText1, patternAsString));
  ASSERT_FALSE(startsWith(mismatchText1, patternAsCharP, patternSize));
  ASSERT_FALSE(startsWith(mismatchText1, patternAsCharP));

  ASSERT_FALSE(startsWith(mismatchText2, patternAsString));
  ASSERT_FALSE(startsWith(mismatchText2, patternAsCharP, patternSize));
  ASSERT_FALSE(startsWith(mismatchText2, patternAsCharP));

  ASSERT_FALSE(startsWith(mismatchText3, patternAsString));
  ASSERT_FALSE(startsWith(mismatchText3, patternAsCharP, patternSize));
  ASSERT_FALSE(startsWith(mismatchText3, patternAsCharP));
}

TEST(StringUtilsTest, nospecialChars)
{
  ASSERT_EQ("bornin", noSpecialChars("born-in"));
  ASSERT_EQ("fooBar", noSpecialChars("#+-_foo__Bar++"));
}

TEST(StringUtilsTest, getNormalizedLowercase)
{
  ASSERT_EQ("schindlerslist", getNormalizedLowercase("Schindler's List"));
  ASSERT_EQ("foobar", getNormalizedLowercase("#+-_foo__Bar++"));
}
TEST(StringUtilsTest, getLowercase)
{
  ASSERT_EQ("schindler's list", getLowercase("Schindler's List"));
  ASSERT_EQ("#+-_foo__bar++", getLowercase("#+-_foo__Bar++"));
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}  // namespace
