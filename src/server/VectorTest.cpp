#include <gtest/gtest.h>
#include "Vector.h"

// _____________________________________________________________________________
TEST(Vector, asString)
{
  Vector<int> v;
  ASSERT_EQ("[]", v.asString());
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  ASSERT_EQ("[1 2 3]", v.asString());
}

// _____________________________________________________________________________
TEST(Vector, parseFromString)
{
  Vector<int> v;
  v.push_back(1);

  v.parseFromString("");
  ASSERT_EQ((unsigned) 0, v.size());

  v.parseFromString("1 2 3");
  ASSERT_EQ((unsigned) 3, v.size());
  ASSERT_EQ(1, v[0]);
  ASSERT_EQ(2, v[1]);
  ASSERT_EQ(3, v[2]);
}

// _____________________________________________________________________________
int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
