// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "./EntityAwareWriter.h"

class EntityAwareWriterTest: public ::testing::Test
{
  public:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

// Test the writeWord method.
TEST_F(EntityAwareWriterTest, writeWord)
{
  ad_semsearch::EntityAwareWriter writer(1, 10);
  writer.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");

  // Set up
  writer.setWriteWikiDocIdPosting(false);
  writer.setDocument("Test Title");

  // Test writing words
  writer.writeWord("myWord");
  writer.writeWord("a");
  writer.writeWord("myWord2");
  std::string output;
  std::string dummy;
  writer.finish(&output, &dummy);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      "myword\t2\t2\t0\n"
      "a\t2\t2\t1\n"
      "myword2\t2\t2\t2\n", output);
}

// Test the writeWord and contextId method.
TEST_F(EntityAwareWriterTest, writeWordAndContextId)
{
  ad_semsearch::EntityAwareWriter writer(1, 10);
  writer.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");

  // Set up
  writer.setWriteWikiDocIdPosting(true);
  writer.setTitle("Title");
  writer.setDocument("Test Title");


  // Test writing words
  writer.writeWord("myWord");
  writer.writeWord("a");
  writer.writeWord("myWord2");
  std::string output;
  std::string dummy;
  writer.finish(&output, &dummy);
  ASSERT_EQ(
      ":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      "myword\t2\t2\t0\n"
      "a\t2\t2\t1\n"
      "myword2\t2\t2\t2\n", output);
}

// Test the write entity method
TEST_F(EntityAwareWriterTest, writeEntity)
{
  ad_semsearch::EntityAwareWriter writer(1, 10);
  writer.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  writer.setWriteWikiDocIdPosting(false);

  // Set up
  writer.setDocument("Test Title");

  std::string entity = "Albert_Einstein";
  // Test without write words
  writer.writeEntity(entity, false, true);
  std::string output;
  std::string dummy;
  writer.finish(&output, &dummy);

  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      ":ee:entity:alberteinstein:Albert_Einstein\t2\t2\t0\n",
      output);
}

// Test the writeCategory mehtod
TEST_F(EntityAwareWriterTest, writeCategory)
{
  ad_semsearch::EntityAwareWriter writer(1, 10);
  writer.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  writer.setWriteWikiDocIdPosting(false);

  // Set up
  writer.setDocument("Test Title");

  // Test category writing
//  std::string category = "Bad Musicians";
//  writer.writeCategory(category);
//  std::string output;
//  std::string dummy;
//  writer.finish(&output, &dummy);
//  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
//      "test\t2\t1\t0\n"
//      "title\t2\t1\t0\n"
//      "ct:category:badmusicians:Bad_Musicians\t2\t2\t0\n", output);
}

// Test the writeSectionHead method
TEST_F(EntityAwareWriterTest, writeSectionHead)
{
  ad_semsearch::EntityAwareWriter writer(1, 10);
  writer.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  writer.setWriteWikiDocIdPosting(false);

  // Set up
  writer.setDocument("Test Title");
  std::vector<std::string> entities;
  entities.push_back("Abcd");
  entities.push_back("Efgh");

  // Test writing words
  writer.writeSectionHead("myWord a myWord2", 1, entities);
  std::string output;
  std::string dummy;
  writer.finish(&output, &dummy);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t3\t1\t0\n"
      "test\t3\t1\t0\n"
      "title\t3\t1\t0\n"
      ":ee:entity:abcd:Abcd\t3\t1\t1\n"
      "myword\t3\t1\t1\n"
      "a\t3\t1\t2\n"
      "myword2\t3\t1\t3\n", output);

  // Test minWordLength in this context
  // Set up
  ad_semsearch::EntityAwareWriter writerMWL2(2, 20);
  writerMWL2.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  writerMWL2.setWriteWikiDocIdPosting(false);
  writerMWL2.setDocument("Test Title");


  writerMWL2.writeSectionHead("myWord a myWord2", 1, entities);
  writerMWL2.finish(&output, &dummy);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t3\t1\t0\n"
      "test\t3\t1\t0\n"
      "title\t3\t1\t0\n"
      ":ee:entity:abcd:Abcd\t3\t1\t1\n"
      "myword\t3\t1\t1\n"
      "myword2\t3\t1\t2\n", output);
}

TEST_F(EntityAwareWriterTest, writeEntityWithHiding)
{
  ad_semsearch::EntityAwareWriter writer(1, 10);
  writer.readRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  writer.setWriteWikiDocIdPosting(false);

  // Set up
  writer.setDocument("Test Title");

  std::string entity = "(Albert) Einstein";
  // Test without write words
  writer.writeEntityWithHiding(entity);
  std::string output;
  std::string dummy;
  writer.finish(&output, &dummy);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      ":ee:entity:alberteinstein:Albert_Einstein\t2\t2\t0\n"
      "einstein\t2\t2\t0\n",
      output);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
