#include "ExcerptsGenerator.h"
#include <gtest/gtest.h>

#define INSERT_INTO_EXCERPT ExcerptsGenerator::insertIntoExcerpt
#define GET_PART_OF_STRING ExcerptsGenerator::getPartOfString

extern char infoDelim;

// _____________________________________________________________________________
TEST(ExcerptsGeneratorTest, isEqual)
{
  ASSERT_TRUE(ExcerptsGenerator::isEqual("proseminar", "proseminar"));
  ASSERT_FALSE(ExcerptsGenerator::isEqual("seminar", "proseminar"));
  ASSERT_FALSE(ExcerptsGenerator::isEqual("pro", "proseminar"));

  ASSERT_TRUE(ExcerptsGenerator::isEqual("proseminar", "^pro^seminar"));
  ASSERT_TRUE(ExcerptsGenerator::isEqual("seminar", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isEqual("pro", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isEqual("prosemina", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isEqual("semina", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isEqual("pr", "^pro^seminar"));

  ASSERT_FALSE(ExcerptsGenerator::isEqual("proseminar", "^pro^^seminar"));
}

// _____________________________________________________________________________
TEST(ExcerptsGeneratorTest, isPrefix)
{
  ASSERT_TRUE(ExcerptsGenerator::isPrefix("pro", "proseminar"));
  ASSERT_TRUE(ExcerptsGenerator::isPrefix("prose", "proseminar"));
  ASSERT_FALSE(ExcerptsGenerator::isPrefix("semir", "proseminar"));

  ASSERT_TRUE(ExcerptsGenerator::isPrefix("pro", "^pro^seminar"));
  ASSERT_TRUE(ExcerptsGenerator::isPrefix("semin", "^pro^seminar"));
  ASSERT_TRUE(ExcerptsGenerator::isPrefix("prose", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isPrefix("ro", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isPrefix("emir", "^pro^seminar"));
  ASSERT_FALSE(ExcerptsGenerator::isPrefix("prosex", "^pro^seminar"));

  ASSERT_FALSE(ExcerptsGenerator::isPrefix("prose", "^pro^^seminar"));
}

// _____________________________________________________________________________
TEST(ExcerptsGeneratorTest, insertIntoExcerpt)
{
  string s;
  s = "ac"; INSERT_INTO_EXCERPT(1, "b", &s); ASSERT_EQ("abc", s);
  s = "\xc3\xa4""c"; INSERT_INTO_EXCERPT(0, "b", &s); ASSERT_EQ("b\xc3\xa4""c", s);
  s = "\xc3\xa4""c"; INSERT_INTO_EXCERPT(1, "b", &s); ASSERT_EQ("\xc3\xa4""bc", s);
  s = "\xc3\xa4""c"; INSERT_INTO_EXCERPT(2, "b", &s); ASSERT_EQ("\xc3\xa4""bc", s);
  s = "\xc3\xa4""c"; INSERT_INTO_EXCERPT(3, "b", &s); ASSERT_EQ("\xc3\xa4""cb", s);
}

// _____________________________________________________________________________
TEST(ExcerptsGeneratorTest, getPartOfString)
{
  ASSERT_EQ("abc", GET_PART_OF_STRING("abc", 0, 3));
  ASSERT_EQ("b", GET_PART_OF_STRING("abc", 1, 1));
  ASSERT_EQ("\xc3\xa4", GET_PART_OF_STRING("a""\xc3\xa4""c", 1, 2));
  ASSERT_EQ("c", GET_PART_OF_STRING("a""\xc3\xa4""c", 2, 2));
  ASSERT_EQ("b""\xc3\xa4", GET_PART_OF_STRING("a""\xc3\xa4""b""\xc3\xa4""c", 2, 3));
  ASSERT_EQ("", GET_PART_OF_STRING("a""\xc3\xa4", 2, 3));
}

// _____________________________________________________________________________
TEST(ExcerptsGeneratorTest, getPartOfMultipleField)
{
  string s = "abc&dfg&hij";
  // Wrong delimitter -> string should not be modified.
  infoDelim = ';';
  ASSERT_EQ("abc&dfg&hij", ExcerptsGenerator::getPartOfMultipleField(1, s));

  // Common cases.
  infoDelim = '&';
  ASSERT_EQ("abc", ExcerptsGenerator::getPartOfMultipleField(0, s));
  ASSERT_EQ("dfg", ExcerptsGenerator::getPartOfMultipleField(1, s));
  ASSERT_EQ("hij", ExcerptsGenerator::getPartOfMultipleField(2, s));

  // Out of bounds.
  ASSERT_EQ("abc", ExcerptsGenerator::getPartOfMultipleField(-1, s));
  ASSERT_EQ("abc", ExcerptsGenerator::getPartOfMultipleField(3, s));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
