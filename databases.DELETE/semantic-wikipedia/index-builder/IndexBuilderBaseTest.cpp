// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <stxxl/vector>
#include <string>
#include <vector>
#include <algorithm>
#include "../server/Identifiers.h"
#include "./Postings.h"
#include "./Extractors.h"
#include "./IndexBuilderComparators.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "./IndexBuilderBase.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! Mock object for testing the methods of the abstract class.
//! Empty method definitions.
class IndexBuilderBaseMock: public IndexBuilderBase
{
    virtual void buildFulltextIndex(const string& asciiFilename,
        const string& vocabularyFilename,
        const string& fulltextBlockBoundariesFilename,
        const string& ontologyBlockBoundariesFilename,
        const string& outputFilename) const
    {
    }

    virtual void buildOntologyIndex(const string& asciiFilename,
        const vector<string>& relationsToBeSplit,
        const string& dummy,
        const string& outputFilename) const
    {
    }
};

class IndexBuilderBaseTest : public ::testing::Test
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

    virtual void TearDown()
    {
      remove(".stxxl");
    }
};

TEST_F(IndexBuilderBaseTest, multiplyTest)
{
  // Setup.
  IndexBuilderBaseMock objectUnderTest;

  vector<BasicPosting> wordPostingsFirstContext;
  wordPostingsFirstContext.push_back(BasicPosting(0, 0, 0));
  wordPostingsFirstContext.push_back(BasicPosting(0, 1, 0));
  wordPostingsFirstContext.push_back(BasicPosting(1, 2, 0));
  wordPostingsFirstContext.push_back(BasicPosting(1, 3, 0));
  vector<BasicPosting> wordPostingsSecondContext;
  wordPostingsSecondContext.push_back(BasicPosting(1, 3, 1));

  Id firstEntityId = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  vector<BasicPosting> entityPostingsFirstContext;
  entityPostingsFirstContext.push_back(
      BasicPosting(firstEntityId, firstEntityId, 0));
  entityPostingsFirstContext.push_back(
      BasicPosting(firstEntityId, firstEntityId + 1, 0));
  vector<BasicPosting> entityPostingsSecondContext;
  entityPostingsSecondContext.push_back(
      BasicPosting(firstEntityId + 1, firstEntityId + 1, 1));

  stxxl::vector<BasicPosting> postingsToBeSorted;
  // Do the test.
  objectUnderTest.multiply(wordPostingsFirstContext,
      entityPostingsFirstContext, &postingsToBeSorted,
      PostingComparatorEntitiesMixedIn<BasicPosting> ());
  objectUnderTest.multiply(wordPostingsSecondContext,
      entityPostingsSecondContext, &postingsToBeSorted,
      PostingComparatorEntitiesMixedIn<BasicPosting> ());

  size_t entityListSize = entityPostingsFirstContext.size()
      + entityPostingsSecondContext.size();
  size_t wordListSize = wordPostingsFirstContext.size()
      + wordPostingsSecondContext.size();
  size_t entitiesInWordsBlock0 = 2;
  size_t entitiesInWordsBlock1 = 3;
  size_t expectedSize = entityListSize + wordListSize + entitiesInWordsBlock0
      + entitiesInWordsBlock1;

  size_t actualEntityListSize = 0;
  size_t actualWordListSize = 0;
  size_t actualEntitiesInWordsBlock0 = 0;
  size_t actualEntitiesInWordsBlock1 = 0;

  // Count the different kinds of postings in the list.
  for (size_t i = 0; i < postingsToBeSorted.size(); ++i)
  {
    if (isIdOfType(postingsToBeSorted[i]._blockId, IdType::ONTOLOGY_ELEMENT_ID))
    {
      ++actualEntityListSize;
    }
    else
    {
      if (isIdOfType(postingsToBeSorted[i]._wordId, IdType::WORD_ID))
      {
        ++actualWordListSize;
      }
      else
      {
        if (getPureValue(postingsToBeSorted[i]._blockId) == 0)
        {
          ++actualEntitiesInWordsBlock0;
        }
        else
        {
          assert(getPureValue(postingsToBeSorted[i]._blockId) == 1);
          ++actualEntitiesInWordsBlock1;
        }
      }
    }
  }
  ASSERT_EQ(entityListSize, actualEntityListSize);
  ASSERT_EQ(wordListSize, actualWordListSize);
  ASSERT_EQ(entitiesInWordsBlock0, actualEntitiesInWordsBlock0);
  ASSERT_EQ(entitiesInWordsBlock1, actualEntitiesInWordsBlock1);
  ASSERT_EQ(expectedSize, postingsToBeSorted.size());
}
TEST_F(IndexBuilderBaseTest, extractPostingsFromAsciiFulltextFileTest)
{
  // Setup.
  IndexBuilderBaseMock objectUnderTest;
  stxxl::vector<BasicPosting> postingsToBeSorted;

  // Do the test.
  string file = "./test.ascii-file";
  BasicExtractor ex;

  HashMap<string, Id> ontologyVocabulary;
  ontologyVocabulary[":e:a"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 0;
  ontologyVocabulary[":e:b"] = getFirstId(IdType::ONTOLOGY_ELEMENT_ID) + 1;
  ex.setOntologyVocabulary(&ontologyVocabulary);
  vector<string> noBoundaries;
  ex.setOntologyBlockBoundaries(noBoundaries);

  HashMap<string, Id> textVocabulary;
  textVocabulary["a"] = 0;
  textVocabulary["b"] = 1;
  textVocabulary["c"] = 2;
  textVocabulary["d"] = 3;
  ex.setTextVocabulary(&textVocabulary);
  vector<string> textBlockBoundaries;
  textBlockBoundaries.push_back("b");
  ex.setTextBlockBoundaries(textBlockBoundaries);

  objectUnderTest.extractPostingsFromAsciiFulltextFile(file, ex,
      &postingsToBeSorted, PostingComparatorEntitiesMixedIn<BasicPosting> ());

  size_t entityListSize = 3;
  size_t wordListSize = 5;
  size_t entitiesInWordsBlock0 = 2;
  size_t entitiesInWordsBlock1 = 3;
  size_t expectedSize = entityListSize + wordListSize + entitiesInWordsBlock0
      + entitiesInWordsBlock1;

  size_t actualEntityListSize = 0;
  size_t actualWordListSize = 0;
  size_t actualEntitiesInWordsBlock0 = 0;
  size_t actualEntitiesInWordsBlock1 = 0;

  // Count the different kinds of postings in the list.
  for (size_t i = 0; i < postingsToBeSorted.size(); ++i)
  {
    if (isIdOfType(postingsToBeSorted[i]._blockId, IdType::ONTOLOGY_ELEMENT_ID))
    {
      ++actualEntityListSize;
    }
    else
    {
      if (isIdOfType(postingsToBeSorted[i]._wordId, IdType::WORD_ID))
      {
        ++actualWordListSize;
      }
      else
      {
        if (getPureValue(postingsToBeSorted[i]._blockId) == 0)
        {
          ++actualEntitiesInWordsBlock0;
        }
        else
        {
          assert(getPureValue(postingsToBeSorted[i]._blockId) == 1);
          ++actualEntitiesInWordsBlock1;
        }
      }
    }
  }
  ASSERT_EQ(entityListSize, actualEntityListSize);
  ASSERT_EQ(wordListSize, actualWordListSize);
  ASSERT_EQ(entitiesInWordsBlock0, actualEntitiesInWordsBlock0);
  ASSERT_EQ(entitiesInWordsBlock1, actualEntitiesInWordsBlock1);
  ASSERT_EQ(expectedSize, postingsToBeSorted.size());
}

TEST_F(IndexBuilderBaseTest, createHasRelationsTest)
{
  // Setup.
  IndexBuilderBaseMock objectUnderTest;
  objectUnderTest._ontologyVocabulary[string(RELATION_PREFIX)
      + HAS_RELATIONS_RELATION] = Id(1);
  stxxl::vector<RelationFact> facts;
  facts.push_back(RelationFact(4, 0, 1));
  facts.push_back(RelationFact(4, 1, 2));
  facts.push_back(RelationFact(5, 1, 0));
  facts.push_back(RelationFact(5, 2, 1));
  facts.push_back(RelationFact(6, 3, 0));
  facts.push_back(RelationFact(7, 0, 3));
  objectUnderTest.createHasRelations(&facts);

  ASSERT_EQ(size_t(12), facts.size());
  ASSERT_EQ(Id(0), facts[6]._lhs);
  ASSERT_EQ(Id(4), facts[6]._rhs);
  ASSERT_EQ(Id(0), facts[7]._lhs);
  ASSERT_EQ(Id(7), facts[7]._rhs);
  ASSERT_EQ(Id(1), facts[8]._lhs);
  ASSERT_EQ(Id(4), facts[8]._rhs);
  ASSERT_EQ(Id(1), facts[9]._lhs);
  ASSERT_EQ(Id(5), facts[9]._rhs);
  ASSERT_EQ(Id(2), facts[10]._lhs);
  ASSERT_EQ(Id(5), facts[10]._rhs);
  ASSERT_EQ(Id(3), facts[11]._lhs);
  ASSERT_EQ(Id(6), facts[11]._rhs);
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
