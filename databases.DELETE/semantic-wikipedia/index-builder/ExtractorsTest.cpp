// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include "../server/Identifiers.h"
#include "./Postings.h"
#include "./Extractors.h"
#include "../codebase/semantic-wikipedia-utils/File.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
class OntologyTripleExtractorTest : public ::testing::Test
{
  protected:

    virtual void SetUp()
    {
      ad_utility::File stxxlConfig(".stxxl", "w");
      std::ostringstream config;
      config << "disk=" << "./" << "stxxl.disk,"
          << ad_semsearch::STXXL_DISK_SIZE_INDEX_BUILDER_TESTS << ",syscall";
      stxxlConfig.writeLine(config.str());
    }
};

TEST(WordsFileExtractorBaseTest, setBlockBoundariesTest)
{
  // Setup.
  DefaultExtractor exUndertest;

  HashMap<string, Id> ontoVoc;
  ontoVoc[":ee:entity:ae:AE"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 0;
  ontoVoc[":ee:entity:bc:BC"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 1;
  ontoVoc[":ee:entity:de:DE"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 2;
  ontoVoc[":ee:entity:hey:Hey"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 3;
  ontoVoc[":ee:entity:oo:OO"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 4;
  ontoVoc[":ee:entity:schindler'slist:Schindler's_List"] =
      getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 5;
  HashMap<string, Id> textVoc;
  textVoc["bar"] = 0;
  textVoc["boo"] = 1;
  textVoc["boolll"] = 2;
  textVoc["foo"] = 3;
  textVoc["lar"] = 4;

  exUndertest.setOntologyVocabulary(&ontoVoc);
  exUndertest.setTextVocabulary(&textVoc);

  ASSERT_FALSE(exUndertest.isProperlyInitialized());

  // Do the tests
  vector<string> blockBoundaries;
  blockBoundaries.push_back("boo");
  blockBoundaries.push_back("foo");
  exUndertest.setTextBlockBoundaries(blockBoundaries);
  vector<string> blockBoundaries2;
  exUndertest.setOntologyBlockBoundaries(blockBoundaries2);
  ASSERT_TRUE(exUndertest.isProperlyInitialized());

  ASSERT_EQ(Id(0), exUndertest._blockIdMapForText[0]);
  ASSERT_EQ(Id(1), exUndertest._blockIdMapForText[1]);
  ASSERT_EQ(Id(1), exUndertest._blockIdMapForText[2]);
  ASSERT_EQ(Id(2), exUndertest._blockIdMapForText[3]);
  ASSERT_EQ(Id(2), exUndertest._blockIdMapForText[4]);

  ASSERT_EQ(Id(0), exUndertest._blockIdMapForOntology[0]);
  ASSERT_EQ(Id(0), exUndertest._blockIdMapForOntology[1]);
  ASSERT_EQ(Id(0), exUndertest._blockIdMapForOntology[2]);
  ASSERT_EQ(Id(0), exUndertest._blockIdMapForOntology[3]);
  ASSERT_EQ(Id(0), exUndertest._blockIdMapForOntology[4]);
  ASSERT_EQ(Id(0), exUndertest._blockIdMapForOntology[5]);

  vector<string> blockBoundaries3;
  blockBoundaries3.push_back(":ee:entity:a");
  blockBoundaries3.push_back(":ee:entity:notinthere");
  exUndertest.setOntologyBlockBoundaries(blockBoundaries3);

  ASSERT_EQ(Id(1), exUndertest._blockIdMapForOntology[0]);
  ASSERT_EQ(Id(1), exUndertest._blockIdMapForOntology[1]);
  ASSERT_EQ(Id(1), exUndertest._blockIdMapForOntology[2]);
  ASSERT_EQ(Id(1), exUndertest._blockIdMapForOntology[3]);
  ASSERT_EQ(Id(2), exUndertest._blockIdMapForOntology[4]);
  ASSERT_EQ(Id(2), exUndertest._blockIdMapForOntology[5]);
}
TEST(BasicExtractorTest, testExtract)
{
  // Setup.
  BasicExtractor exUndertest;
  ASSERT_FALSE(exUndertest.isProperlyInitialized());

  HashMap<string, Id> ontoVoc;
  ontoVoc[":ee:entity:ae:AE"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 0;
  ontoVoc[":ee:entity:bc:BC"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 1;
  ontoVoc[":ee:entity:de:DE"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 2;
  ontoVoc[":ee:entity:hey:Hey"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 3;
  ontoVoc[":ee:entity:oo:OO"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 4;
  ontoVoc[":ee:entity:schindler'slist:Schindler's_List"]
          = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 5;
  HashMap<string, Id> textVoc;
  textVoc["bar"] = 0;
  textVoc["boo"] = 1;
  textVoc["dar"] = 2;
  textVoc["foo"] = 3;
  textVoc["lar"] = 4;

  exUndertest.setOntologyVocabulary(&ontoVoc);
  exUndertest.setTextVocabulary(&textVoc);
  vector<string> blockBoundaries;
  blockBoundaries.push_back("boo");
  exUndertest.setTextBlockBoundaries(blockBoundaries);
  vector<string> blockBoundaries2;
  exUndertest.setOntologyBlockBoundaries(blockBoundaries2);
  ASSERT_TRUE(exUndertest.isProperlyInitialized());

  // Do the tests.
  BasicPosting extraction;
  exUndertest.extract("foo\t1\t1\t1", &extraction);
  ASSERT_EQ(
      "Basic Posting - Block Id: 1, Word Id: 3, Context Id: 1",
      extraction.asString());

  exUndertest.extract("bar\t2", &extraction);
  ASSERT_EQ(
      "Basic Posting - Block Id: 0, Word Id: 0, Context Id: 2",
      extraction.asString());

  exUndertest.extract(
      ":ee:entity:schindler'slist:Schindler's_List\t1",
      &extraction);
  ASSERT_EQ(
      "Basic Posting - Entity-Block Id: 0, Entity-Word Id: 5, Context Id: 1",
      extraction.asString());
  exUndertest.extract(":ee:entity:bc:BC\t500", &extraction);
  ASSERT_EQ(
      "Basic Posting - Entity-Block Id: 0, Entity-Word Id: 1, Context Id: 500",
      extraction.asString());
  exUndertest.extract("lar\t9", &extraction);
  ASSERT_EQ(
      "Basic Posting - Block Id: 1, Word Id: 4, Context Id: 9",
      extraction.asString());
}
TEST(DefaultExtractorTest, extractTest)
{
  // Setup.
  DefaultExtractor exUndertest;
  ASSERT_FALSE(exUndertest.isProperlyInitialized());

  HashMap<string, Id> ontoVoc;
  ontoVoc[":ee:entity:ae:AE"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 0;
  ontoVoc[":ee:entity:bc:BC"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 1;
  ontoVoc[":ee:entity:de:DE"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 2;
  ontoVoc[":ee:entity:hey:Hey"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 3;
  ontoVoc[":ee:entity:oo:OO"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 4;
  ontoVoc[":ee:entity:schindler'slist:Schindler's_List"]
          = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 5;
  HashMap<string, Id> textVoc;
  textVoc["bar"] = 0;
  textVoc["boo"] = 1;
  textVoc["dar"] = 2;
  textVoc["foo"] = 3;
  textVoc["lar"] = 4;

  exUndertest.setOntologyVocabulary(&ontoVoc);
  exUndertest.setTextVocabulary(&textVoc);
  vector<string> blockBoundaries;
  blockBoundaries.push_back("boo");
  exUndertest.setTextBlockBoundaries(blockBoundaries);
  vector<string> blockBoundaries2;
  exUndertest.setOntologyBlockBoundaries(blockBoundaries2);
  ASSERT_TRUE(exUndertest.isProperlyInitialized());

  // Do the tests.
  Posting extraction;
  exUndertest.extract("foo\t1\t1\t1", &extraction);
  ASSERT_EQ(
      "Posting - Block Id: 1, Word Id: 3, Context Id: 1, Score: 1, Pos: 1",
      extraction.asString());

  exUndertest.extract("bar\t2\t1\t1", &extraction);
  ASSERT_EQ(
      "Posting - Block Id: 0, Word Id: 0, Context Id: 2, Score: 1, Pos: 1",
      extraction.asString());

  exUndertest.extract(
      ":ee:entity:schindler'slist:Schindler's_List\t1\t2\t3",
      &extraction);
  ASSERT_EQ(
      "Posting - Entity-Block Id: 0, Entity-Word Id: 5, "
      "Context Id: 1, Score: 2, Pos: 3",
      extraction.asString());
  exUndertest.extract(":ee:entity:bc:BC\t500\t1\t1", &extraction);
  ASSERT_EQ(
      "Posting - Entity-Block Id: 0, Entity-Word Id: 1, "
      "Context Id: 500, Score: 1, Pos: 1",
      extraction.asString());
  exUndertest.extract("lar\t9\t1\t1", &extraction);
  ASSERT_EQ(
      "Posting - Block Id: 1, Word Id: 4, Context Id: 9, Score: 1, Pos: 1",
      extraction.asString());
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
