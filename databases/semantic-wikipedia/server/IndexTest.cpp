// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./EntityList.h"
#include "./Identifiers.h"
#include "./Index.h"
#include "./IndexMetaData.h"
#include "./Aggregators.h"

using std::string;
using std::vector;

using ad_utility::File;
namespace ad_semsearch
{
class IndexTest: public ::testing::Test
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
      _plainBaseName = "test.plain";
      _plainIndexFile = _plainBaseName + ".index";
      _plainOntologyBaseName = _plainBaseName + "-ontology";
      _relationOneId = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 4;
    }

    virtual void TearDown()
    {
    }

  public:
    string _plainBaseName;
    string _plainIndexFile;
    string _plainOntologyBaseName;
    Id _relationOneId;
};
// _____________________________________________________________________________
TEST_F(IndexTest, registerFulltextIndexTest)
{
  Index objUnderTest;
  objUnderTest.registerFulltextIndex(_plainBaseName, false);
  ASSERT_EQ(size_t(1), objUnderTest._fullTextIndexes.size());
  ASSERT_EQ(size_t(1), objUnderTest._fulltextMetaData.size());
  ASSERT_EQ(size_t(1), objUnderTest._fulltextVocabularies.size());

  // Assumptions on the Meta Data
  Id word0 = getFirstId(IdType::WORD_ID);

  // Block 0:
  const BlockMetaData& block0Meta =
      objUnderTest._fulltextMetaData[0].getBlockInfoByWordRange(word0, word0);
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
  const BlockMetaData& block1Meta =
      objUnderTest._fulltextMetaData[0].getBlockInfoByWordRange(word0 + 2,
          word0 + 2);
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

  // Assumptions on the vocabulary
  ASSERT_EQ(size_t(5), objUnderTest._fulltextVocabularies[0].size());
}
// _____________________________________________________________________________
TEST_F(IndexTest, readBlockTest)
{
  Index index;
  File plain(_plainIndexFile.c_str(), "r");
  PostingList block0;
  PostingList block1;

  // Read meta data:
  off_t currentPos = plain.getTrailingOffT();
  // Block 0:
  BlockMetaData block0Meta;
  plain.read(&block0Meta._maxWordId, sizeof(Id), currentPos);
  currentPos += sizeof(Id);
  plain.read(&block0Meta._nofPostings, sizeof(size_t), currentPos);
  currentPos += sizeof(size_t);
  plain.read(&block0Meta._startOfWordList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block0Meta._startOfContextList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block0Meta._startOfScoreList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block0Meta._startOfPositionList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block0Meta._posOfLastPosition, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  // Block 1:
  BlockMetaData block1Meta;
  plain.read(&block1Meta._maxWordId, sizeof(Id), currentPos);
  currentPos += sizeof(Id);
  plain.read(&block1Meta._nofPostings, sizeof(size_t), currentPos);
  currentPos += sizeof(size_t);
  plain.read(&block1Meta._startOfWordList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block1Meta._startOfContextList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block1Meta._startOfScoreList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block1Meta._startOfPositionList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  plain.read(&block1Meta._posOfLastPosition, sizeof(off_t), currentPos);

  off_t posOfLastPos = plain.getTrailingOffT() - sizeof(Position);
  ASSERT_EQ(posOfLastPos, block1Meta._posOfLastPosition);
  ASSERT_EQ(off_t(10 * sizeof(Id) + 5 * sizeof(Score)),
      block0Meta._startOfPositionList);

  // Read the blocks
  index.readBlock(plain, block0Meta, &block0);
  index.readBlock(plain, block1Meta, &block1);

  // Assertions:
  ASSERT_EQ(size_t(5), block0.size());
  ASSERT_EQ(size_t(4), block1.size());

  ASSERT_EQ("[(WordId: 0, ContextId: 0, Score: 0, Pos: 0), "
      "(WordId: 1, ContextId: 0, Score: 0, Pos: 1), "
      "(EntityId: 0, ContextId: 0, Score: 0, Pos: 2), ...]", block0.asString());

  ASSERT_EQ("[(WordId: 4, ContextId: 0, Score: 0, Pos: 0), "
        "(EntityId: 0, ContextId: 0, Score: 0, Pos: 1), "
        "(WordId: 2, ContextId: 1, Score: 1, Pos: 0), ...]", block1.asString());
}

// _____________________________________________________________________________
TEST_F(IndexTest, registerOntologyIndexTest)
{
  Index index;
  index.registerOntologyIndex(_plainOntologyBaseName);
  ASSERT_EQ(size_t(5), index._ontologyMetaData._relationData.size());
  ASSERT_EQ(size_t(13), index._ontologyVocabulary.size());
}

// _____________________________________________________________________________
TEST_F(IndexTest, readRelationBlockTest)
{
  Index index;
  index.registerOntologyIndex(_plainOntologyBaseName);
  Relation rel;
  ASSERT_GT(index._ontologyMetaData._relationData.size(), size_t(0));
  ASSERT_GT(index._ontologyMetaData.getRelationMetaData
        (_relationOneId)._blockInfo.size(), size_t(0));
  index.readRelationBlock(
      index._ontologyMetaData.getRelationMetaData
      (_relationOneId)._blockInfo[0], &rel);
  ASSERT_EQ("[(0 - 1), (1 - 2)]", rel.asString());
}

// _____________________________________________________________________________
TEST_F(IndexTest, readFullRelationTest)
{
  Index index;
  index.registerOntologyIndex(_plainOntologyBaseName);
  Relation rel;
  ASSERT_GT(index._ontologyMetaData._relationData.size(), size_t(0));
  ASSERT_GT(index._ontologyMetaData.getRelationMetaData
      (_relationOneId)._blockInfo.size(), size_t(0));
  index.readRelationBlock(
      index._ontologyMetaData.getRelationMetaData
      (_relationOneId + 1)._blockInfo[0], &rel);
  ASSERT_EQ("[(1 - 0), (2 - 1)]", rel.asString());
}

// _____________________________________________________________________________
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
