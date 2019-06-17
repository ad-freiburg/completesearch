// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include "./EntityRepository.h"

ad_semsearch::EntityRepository repos;

// Test the isEntity method.
TEST(EntityRepositoryTest, isEntity)
{
  std::string title = "abcd";
  repos.setNewPageEntity(title);
  ASSERT_EQ(title, repos.isEntity("abcd"));
  ASSERT_EQ("", repos.isEntity("foo"));

  title = "Some title";
  repos.setNewPageEntity(title);
  ASSERT_EQ(title, repos.isEntity("title"));
  ASSERT_EQ(title, repos.isEntity("some"));
  ASSERT_EQ("", repos.isEntity("foo"));

  title = "I_am_ABC_foobar";
  repos.setNewPageEntity(title);
  ASSERT_EQ("", repos.isEntity("i"));
  ASSERT_EQ("", repos.isEntity("am"));
  ASSERT_EQ(title, repos.isEntity("foobar"));
  ASSERT_EQ(title, repos.isEntity("abc"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
