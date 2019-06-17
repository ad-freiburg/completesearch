// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../server/Identifiers.h"
#include "../server/IndexMetaData.h"
#include "./Postings.h"
#include "./PlainSerializer.h"
#include "../codebase/semantic-wikipedia-utils/File.h"

using std::string;
using std::vector;

using ad_utility::File;

namespace ad_semsearch
{
static const char PLAINSERIALIZER_TEST_FILENAME[] =
    "__tmp.test.plain-serializer-test.index";

class PlainSerializerTest: public ::testing::Test
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
TEST_F(PlainSerializerTest, testSerialize)
{
  // Setup
  stxxl::vector<Posting> postings;
  Id word0 = getFirstId(IdType::WORD_ID);
  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id context0 = getFirstId(IdType::CONTEXT_ID);
  Id block0 = Id(0);
  Score score0 = Score(0);
  Position pos0 = Position(0);

  postings.clear();

  // Block 0:
  // 0   0 0 0
  // 1   0 0 1
  // e0  0 0 2
  // 0   5 0 0
  // e10 5 0 1
  postings.push_back(Posting(block0, word0, context0, score0, pos0));
  postings.push_back(Posting(block0, word0 + 1, context0, score0, pos0 + 1));
  postings.push_back(Posting(block0, entity0, context0, score0, pos0 + 2));
  postings.push_back(Posting(block0, word0, context0 + 5, score0, pos0));
  postings.push_back(
      Posting(block0, entity0 + 10, context0 + 5, score0, pos0 + 1));

  // Block 1:
  // 4  0 0 0
  // e0 0 0 1
  // 2  1 1 0
  // e9 1 2 1
  postings.push_back(Posting(block0 + 1, word0 + 4, context0, score0, pos0));
  postings.push_back(Posting(block0 + 1, entity0, context0, score0, pos0 + 1));
  postings.push_back(
      Posting(block0 + 1, word0 + 2, context0 + 1, score0 + 1, pos0));
  postings.push_back(
      Posting(block0 + 1, entity0 + 9, context0 + 1, score0 + 2, pos0 + 1));

  PlainSerializer objUnderTest;
  objUnderTest.serialize(postings, string(PLAINSERIALIZER_TEST_FILENAME));
  File output(PLAINSERIALIZER_TEST_FILENAME, "r");

  Id idFromOutput;
  Score scoreFromOutput;
  Position posFromOutput;


  // ASSERTION ON THE FINAL OFFSET:
  off_t startOfB1 = 10 * sizeof(Id) + 5 * sizeof(Score) + 5 * sizeof(Position);
  off_t expectedStartOfMeta = 18 * sizeof(Id)
        + 9 * sizeof(Score) + 9 * sizeof(Position);
  off_t startOfMeta = output.getTrailingOffT();
  ASSERT_EQ(expectedStartOfMeta, startOfMeta);


  // ASSERTIONS ON META DATA:
  // Block 0:
  off_t currentPos = startOfMeta;
  BlockMetaData block0Meta;
  output.read(&block0Meta._maxWordId, sizeof(Id), currentPos);
  currentPos += sizeof(Id);
  output.read(&block0Meta._nofPostings, sizeof(size_t), currentPos);
  currentPos += sizeof(size_t);
  output.read(&block0Meta._startOfWordList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block0Meta._startOfContextList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block0Meta._startOfScoreList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block0Meta._startOfPositionList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block0Meta._posOfLastPosition, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
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
  BlockMetaData block1Meta;
  output.read(&block1Meta._maxWordId, sizeof(Id), currentPos);
  currentPos += sizeof(Id);
  output.read(&block1Meta._nofPostings, sizeof(size_t), currentPos);
  currentPos += sizeof(size_t);
  output.read(&block1Meta._startOfWordList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block1Meta._startOfContextList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block1Meta._startOfScoreList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block1Meta._startOfPositionList, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
  output.read(&block1Meta._posOfLastPosition, sizeof(off_t), currentPos);
  currentPos += sizeof(off_t);
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


  // ASSERTIONS ON ACTUAL DATA:
  // Block 0:
  output.read(&idFromOutput, sizeof(Id), block0Meta._startOfWordList);
  ASSERT_EQ(word0, idFromOutput);

  output.read(&idFromOutput, sizeof(Id),
      block0Meta._startOfWordList + 4 * sizeof(Id));
  ASSERT_EQ(entity0 + 10, idFromOutput);

  output.read(&idFromOutput, sizeof(Id),
      block0Meta._startOfContextList + 3 * sizeof(Id));
  ASSERT_EQ(context0 + 5, idFromOutput);

  output.read(&scoreFromOutput, sizeof(Score),
      block0Meta._startOfScoreList + 2 * sizeof(Score));
  ASSERT_EQ(scoreFromOutput, score0);

  output.read(&scoreFromOutput, sizeof(Score),
        block0Meta._startOfScoreList + 4 * sizeof(Score));
    ASSERT_EQ(scoreFromOutput, score0);

  output.read(&posFromOutput, sizeof(Position),
        block0Meta._startOfPositionList);
  ASSERT_EQ(pos0, posFromOutput);

  output.read(&posFromOutput, sizeof(Position),
      block0Meta._startOfPositionList + 2 * sizeof(Position));
  ASSERT_EQ(pos0 + 2, posFromOutput);

  // Block 1:
  output.read(&idFromOutput, sizeof(Id), block1Meta._startOfWordList);
  ASSERT_EQ(word0 + 4, idFromOutput);

  output.read(&scoreFromOutput, sizeof(Score),
          block1Meta._startOfScoreList + 3 * sizeof(Score));
      ASSERT_EQ(scoreFromOutput, score0 + 2);

  output.read(&posFromOutput, sizeof(Position), block1Meta._posOfLastPosition);
  ASSERT_EQ(pos0 + 1, posFromOutput);

  // Cleanup
  remove(PLAINSERIALIZER_TEST_FILENAME);
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
