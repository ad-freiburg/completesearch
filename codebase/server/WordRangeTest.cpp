// Albert-Ludwigs Universität Freiburg
// cc 2010 Jens Hoffmann
// <hoffmaje@informatik.uni-freiburg.de>

#include "./WordRange.h"
#include <gtest/gtest.h>

// _____________________________________________________________________________
TEST(WordRangeTest, constructor)
{
  // Test if  default constructor sets range (-1,-1).
	WordRange wr;
	// Empty range is (-1,-1)
	ASSERT_EQ(-1, wr._first);
	ASSERT_EQ(-1, wr._second);
  // Test if constructor sets range correctly with given range.
  WordRange wr2(0, -1);
  ASSERT_EQ(0, wr2._first);
  ASSERT_EQ(-1, wr2._second);
}

// _____________________________________________________________________________
TEST(WordRangeTest, size)
{
  // Empty range: Both WordIds = -1.
  {
    WordRange wr(-1, -1);
    ASSERT_EQ(0, wr.size());
  }
  // Infinite range, size = -1.
  {
    WordRange wr(2, 0);
    ASSERT_EQ(-1, wr.size());
  }
  // Infinite range, size = -1.
  {
    WordRange wr(0, -2);
    ASSERT_EQ(-1, wr.size());
  }
  // Range 1.
  {
    WordRange wr(123, 123);
    ASSERT_EQ(1, wr.size());
  }
  // Range 2.
  {
    WordRange wr(123, 124);
    ASSERT_EQ(2, wr.size());
  }
}

// _____________________________________________________________________________
TEST(WordRangeTest, isInRange)
{
	//! @todo(hoffmaje): The following commented cases fail until they all
	//! struggle with the ambiguous definition for 'infinite'. See WordRange.h for
	//! more details.
	
  // With range size = 1 only one WordId is in range.
  {
    WordRange wr(1, 1);
    ASSERT_TRUE(wr.isInRange(1));
    ASSERT_FALSE(wr.isInRange(0));
    ASSERT_FALSE(wr.isInRange(2));
  }
  // With range size = 2 two WordIds are in range.
  {
    WordRange wr(1, 2);
    ASSERT_EQ(2, wr.size());
    ASSERT_TRUE(wr.isInRange(1));
    ASSERT_TRUE(wr.isInRange(2));
    ASSERT_FALSE(wr.isInRange(0));
    ASSERT_FALSE(wr.isInRange(3));
  }
  // With range size = 0 no WordId is in range.
  {
    WordRange wr(-1, -1);
    ASSERT_EQ(0, wr.size());
		// WordIds are signed.
    // ASSERT_FALSE(wr.isInRange(-1));
  }
   // With range size = 0 no WordId is in range.
  {
    // WordRange wr(-10, -10);
    // ASSERT_EQ(0, wr.size());
    // ASSERT_FALSE(wr.isInRange(-10));
  }
  // With range size = inf (-1) any WordId is in range.
  {
    // WordRange wr(2, 0);
    // ASSERT_EQ(-1, wr.size());
    // ASSERT_TRUE(wr.isInRange(123456789));
  }
  // With range size = inf (-1) any WordId is in range.
  {
    // WordRange wr(0, -2);
    // ASSERT_TRUE(wr.size() == -1 && wr.isInRange(123456789));
  }
  // With range size = inf (-1) any WordId is in range.
  {
    // WordRange wr(-2, -4);
    // ASSERT_EQ(-1, wr.size());
    // ASSERT_TRUE(wr.isInRange(123456789));
  }
}

// _____________________________________________________________________________
TEST(WordRangeTest, isEmptyRange)
{
  // With range size = 1 range is not empty.
  {
    WordRange wr(0, 0);
    ASSERT_EQ(1, wr.size());
    ASSERT_FALSE(wr.isEmptyRange());
  }
  // With range size = inf (-1) range is not empty.
  {
    WordRange wr(2, 0);
    ASSERT_EQ(-1, wr.size());
    ASSERT_FALSE(wr.isEmptyRange());
  }
  // With range size = 0 range is empty.
  {
    WordRange wr(-1, -1);
    ASSERT_EQ(0, wr.size());
    ASSERT_TRUE(wr.isEmptyRange());
  }
}

// _____________________________________________________________________________
TEST(WordRangeTest, isInfiniteRange)
{
  // With range size = 1 range is not infinite.
  {
    WordRange wr(0, 0);
    ASSERT_EQ(1, wr.size());
    ASSERT_FALSE(wr.isInfiniteRange());
  }
  // With range size = inf (-1) range is infinite.
  {
    // WordRange wr(0, -2);
    // ASSERT_EQ(-1, wr.size());
    // ASSERT_TRUE(wr.isInfiniteRange());
  }
  // With range size = inf (-1) range is infinite.
  {
    // WordRange wr(2, 0);
    // ASSERT_EQ(-1, wr.size());
    // ASSERT_TRUE(wr.isInfiniteRange());
  }
  // With range size = inf (-1) range is infinite.
  {
    // WordRange wr(4, 2);
    // ASSERT_EQ(-1, wr.size());
    // ASSERT_TRUE(wr.isInfiniteRange());
  }
  // With range size = 0 range is not infinite.
  {
    WordRange wr(-1, -1);
    ASSERT_EQ(0, wr.size());
		ASSERT_FALSE(wr.isInfiniteRange());
  }
}

// _____________________________________________________________________________
int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
