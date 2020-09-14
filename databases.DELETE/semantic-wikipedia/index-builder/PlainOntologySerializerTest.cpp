// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../server/Identifiers.h"
#include "../server/IndexMetaData.h"
#include "./Facts.h"
#include "./PlainOntologySerializer.h"
#include "../codebase/semantic-wikipedia-utils/File.h"

using std::string;
using std::vector;

using ad_utility::File;

namespace ad_semsearch
{
static const char PLAINONTOLOGYSERIALIZER_TEST_FILENAME[] =
    "__tmp.test.plain-ontology-serializer-test.index";

class PlainOntologySerializerTest: public ::testing::Test
{
    virtual void SetUp()
    {
      ad_utility::File stxxlConfig(".stxxl", "w");
      std::ostringstream config;
      config << "disk=" << "./" << "stxxl.disk,"
          << ad_semsearch::STXXL_DISK_SIZE_INDEX_BUILDER_TESTS << ",syscall";
      stxxlConfig.writeLine(config.str());
    }

    virtual void TearDown()
    {
      remove(".stxxl");
    }
};
TEST_F(PlainOntologySerializerTest, testSerialize)
{
  // Setup
  stxxl::vector<RelationFact> facts;
  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);

  facts.push_back(RelationFact(entity0, entity0 + 1, entity0 + 2));
  facts.push_back(RelationFact(entity0, entity0 + 2, entity0 + 3));
  facts.push_back(RelationFact(entity0, entity0 + 2, entity0 + 4));
  facts.push_back(RelationFact(entity0 + 9, entity0 + 2, entity0 + 1));
  facts.push_back(RelationFact(entity0 + 9, entity0 + 3, entity0 + 2));
  facts.push_back(RelationFact(entity0 + 9, entity0 + 4, entity0 + 2));

  vector<RelationMetaData> meta;
  meta.push_back(RelationMetaData(entity0, entity0 + 5, entity0 + 6));
  meta.push_back(RelationMetaData(entity0 + 9, entity0 + 6, entity0 + 5));
  PlainOntologySerializer objUnderTest;
  ad_utility::HashSet<Id> emptySet;
  objUnderTest.serialize(facts, meta, emptySet,
      string(PLAINONTOLOGYSERIALIZER_TEST_FILENAME));
  File output(PLAINONTOLOGYSERIALIZER_TEST_FILENAME, "r");

  // ASSERTION ON THE FINAL OFFSET:
  off_t expectedStartOfMeta = 6 * 2 * sizeof(Id);
  off_t startOfMeta = output.getTrailingOffT();
  ASSERT_EQ(expectedStartOfMeta, startOfMeta);

  // ASSERTIONS ON THE META DATA:
  Id relId;
  Id lhsType;
  Id rhsType;
  off_t nextMeta;

  output.read(&nextMeta, sizeof(off_t), startOfMeta);
  output.read(&relId, sizeof(Id), startOfMeta + sizeof(off_t));
  output.read(&lhsType, sizeof(Id), startOfMeta + sizeof(off_t) + sizeof(Id));
  output.read(
      &rhsType, sizeof(Id), startOfMeta + sizeof(off_t) + 2 * sizeof(Id));

  ASSERT_EQ(off_t(startOfMeta + sizeof(off_t) + 3 * sizeof(Id)
            + 3 * sizeof(off_t) +sizeof(size_t) + sizeof(Id)), nextMeta);
  ASSERT_EQ(entity0, relId);
  ASSERT_EQ(entity0 + 5, lhsType);
  ASSERT_EQ(entity0 + 6, rhsType);

  output.read(&relId, sizeof(Id), nextMeta + sizeof(off_t));
  output.read(&lhsType, sizeof(Id), nextMeta + sizeof(off_t) + sizeof(Id));
  output.read(&rhsType, sizeof(Id), nextMeta + sizeof(off_t) + 2 * sizeof(Id));

  ASSERT_EQ(entity0 + 9, relId);
  ASSERT_EQ(entity0 + 6, lhsType);
  ASSERT_EQ(entity0 + 5, rhsType);


  // Cleanup
  remove(PLAINONTOLOGYSERIALIZER_TEST_FILENAME);
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
