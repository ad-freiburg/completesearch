// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./Identifiers.h"
#include "./IndexMetaData.h"
#include "./Index.h"

using std::string;
using std::vector;

using ad_utility::File;
namespace ad_semsearch
{
class FulltextIndexMetaDataTest: public ::testing::Test
{
    virtual void SetUp()
    {
      // Note that this file has to be checked-in to the svn.
      // Alternatives result in lots of code for writing an index file
      // and cause this tests to test that code instead of the actual unit
      // under test.
      // This file is an index as produced (and usually deleted) by the
      // ../index-builder/PlainSerializerTest
      // That test can be used to generate such a
      // file if there should be changes
      // to this file's layout.
      //
      // This file looks like this:
      // <block0><block1>...<blockMetaData><startOfMetaData>
      // whereas each block looks like this:
      // <wordIds><contextIds><Scores><Positions>
      // and BlockMetaData looks like this:
      // <MaxWordId><NofPostings><wFrom><cFrom><sFrom><posFrom><End>
      // With blocks:
      //
      // Block 0:
      // 0   0 0 0
      // 1   0 0 1
      // e0  0 0 2
      // 0   5 0 0
      // e10 5 0 1
      //
      // Block 1:
      // 4  0 0 0
      // e0 0 0 1
      // 2  1 1 0
      // e9 1 2 1
      _plainIndexFile = "test.plain.index";
    }

    virtual void TearDown()
    {
    }

  public:
    string _plainIndexFile;
};

TEST_F(FulltextIndexMetaDataTest, initFromFileTest)
{
  FulltextMetaData objUnderTest;
  File plain(_plainIndexFile.c_str(), "r");
  objUnderTest.initFromFile(&plain);
  ASSERT_EQ(size_t(2), objUnderTest._blockInfo.size());

  Id word0 = getFirstId(IdType::WORD_ID);

  // Block 0:
  BlockMetaData block0Meta = objUnderTest._blockInfo[0];
  ASSERT_EQ(word0 + 1, block0Meta._maxWordId);
  ASSERT_EQ(size_t(5), block0Meta._nofPostings);
  ASSERT_EQ(off_t(0), block0Meta._startOfWordList);
  ASSERT_EQ(off_t(5 * sizeof(Id)), block0Meta._startOfContextList);
  ASSERT_EQ(off_t(10 * sizeof(Id)), block0Meta._startOfScoreList);
  ASSERT_EQ(off_t(10 * sizeof(Id) + 5 * sizeof(Score)),
      block0Meta._startOfPositionList);
  ASSERT_EQ(off_t(10 * sizeof(Id) + 5 * sizeof(Score) + 4 * sizeof(Position)),
      block0Meta._posOfLastPosition);

  // Block 1:
  off_t startOfB1 = 10 * sizeof(Id) + 5 * sizeof(Score) + 5 * sizeof(Position);
  BlockMetaData block1Meta = objUnderTest._blockInfo[1];
  ASSERT_EQ(word0 +4, block1Meta._maxWordId);
  ASSERT_EQ(size_t(4), block1Meta._nofPostings);
  ASSERT_EQ(startOfB1, block1Meta._startOfWordList);
  ASSERT_EQ(off_t(startOfB1 + 4 * sizeof(Id)), block1Meta._startOfContextList);
  ASSERT_EQ(off_t(startOfB1 + 8 * sizeof(Id)), block1Meta._startOfScoreList);
  ASSERT_EQ(off_t(startOfB1 + 8 * sizeof(Id) + 4 * sizeof(Score)),
      block1Meta._startOfPositionList);
  ASSERT_EQ(off_t(startOfB1 + 8 * sizeof(Id) + 4 * sizeof(Score)
            + 3 * sizeof(Position)),
            block1Meta._posOfLastPosition);
}

TEST_F(FulltextIndexMetaDataTest, getBlockInfoByWordRangeTest)
{
  FulltextMetaData objUnderTest;
  File plain(_plainIndexFile.c_str(), "r");
  objUnderTest.initFromFile(&plain);

  BlockMetaData block0Meta = objUnderTest._blockInfo[0];
  BlockMetaData block1Meta = objUnderTest._blockInfo[1];

  Id word0 = getFirstId(IdType::WORD_ID);

  ASSERT_EQ(block0Meta,
        objUnderTest.getBlockInfoByWordRange(word0, word0));
  ASSERT_EQ(block0Meta,
        objUnderTest.getBlockInfoByWordRange(word0, word0 + 1));
  ASSERT_EQ(block0Meta,
        objUnderTest.getBlockInfoByWordRange(word0 + 1, word0 + 1));

  ASSERT_EQ(block1Meta,
        objUnderTest.getBlockInfoByWordRange(word0 + 2, word0 + 2));
  ASSERT_EQ(block1Meta,
        objUnderTest.getBlockInfoByWordRange(word0 + 3, word0 + 4));
  ASSERT_EQ(block1Meta,
        objUnderTest.getBlockInfoByWordRange(word0 + 4, word0 + 4));
}

class OntologyMetaDataTest : public ::testing::Test
  {
      virtual void SetUp()
      {
        // This file is checked-in. It looks like this:
        // :r:1                 :t:1 :t:1 :e:1 :e:2
        // :r:1                 :t:1 :t:1 :e:2 :e:3
        // :r:1_rev             :t:1 :t:1 :e:2 :e:1
        // :r:1_rev             :t:1 :t:1 :e:3 :e:2
        // :r:2                 :t:2 :t:1 :e:4 :e:1
        // :r:2_rev             :t:1 :t:2 :e:1 :e:4
        // :r:has-relations     :t:e :t:r :e:1 :r:1
        // :r:has-relations     :t:e :t:r :e:1 :r:2_rev
        // :r:has-relations     :t:e :t:r :e:2 :r:1
        // :r:has-relations     :t:e :t:r :e:2 :r:1_rev
        // :r:has-relations     :t:e :t:r :e:3 :r:1_rev
        // :r:has-relations     :t:e :t:r :e:4 :r:2
        // <MetaData>
        // ...
        // </MetaData>
        _plainIndexFile = "test.plain-ontology.index";
      }

      virtual void TearDown()
      {
      }

    public:
      string _plainIndexFile;
};

TEST_F(OntologyMetaDataTest, initFromFileTest)
{
  OntologyMetaData oMeta;
  File indexFile(_plainIndexFile.c_str(), "r");
  oMeta.initFromFile(&indexFile);
  ASSERT_EQ(size_t(5), oMeta._relationData.size());
}

TEST_F(OntologyMetaDataTest, getRelationMetaDataTest)
{
  OntologyMetaData oMeta;
  File indexFile(_plainIndexFile.c_str(), "r");
  oMeta.initFromFile(&indexFile);
  Id r1Id = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 4;
  Id t1Id = r1Id + 5;
  ASSERT_EQ(r1Id, oMeta.getRelationMetaData(r1Id)._relationId);
  ASSERT_EQ(Id(9), getPureValue(oMeta.getRelationMetaData(r1Id)._lhsType));
  ASSERT_EQ(Id(9), getPureValue(oMeta.getRelationMetaData(r1Id)._rhsType));
  ASSERT_EQ(r1Id + 1, oMeta.getRelationMetaData(r1Id + 1)._relationId);
  ASSERT_EQ(t1Id, oMeta.getRelationMetaData(r1Id + 1)._lhsType);
  ASSERT_EQ(t1Id, oMeta.getRelationMetaData(r1Id + 1)._rhsType);
  ASSERT_EQ(r1Id + 4, oMeta.getRelationMetaData(r1Id + 4)._relationId);
  ASSERT_EQ(t1Id + 2, oMeta.getRelationMetaData(r1Id + 4)._lhsType);
  ASSERT_EQ(t1Id + 3, oMeta.getRelationMetaData(r1Id + 4)._rhsType);
  ASSERT_EQ(size_t(1), oMeta.getRelationMetaData(r1Id + 2)._blockInfo.size());
  off_t startOfHasRelationsLhs =
      oMeta.getRelationMetaData(r1Id + 4)._blockInfo[0]._startOfLhsData;
  off_t startOfHasRelationsRhs =
        oMeta.getRelationMetaData(r1Id + 4)._blockInfo[0]._startOfRhsData;
  size_t nofElementsInHasRelations = (startOfHasRelationsRhs
      - startOfHasRelationsLhs) / sizeof(Id);
  ASSERT_EQ(size_t(6), nofElementsInHasRelations);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
