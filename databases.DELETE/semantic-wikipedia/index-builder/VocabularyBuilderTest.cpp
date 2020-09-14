// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "./VocabularyBuilder.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
VocabularyBuilder vocabularyBuilder;

TEST(VocabularyBuilderTest, constructVocabularyWordsfile)
{
  vocabularyBuilder.constructVocabularyFromASCIIWordsFile(
      "./test.words-unsorted.ascii");
  ASSERT_EQ(static_cast<size_t>(5), vocabularyBuilder._vocabulary.size());
  ASSERT_EQ("test5", vocabularyBuilder._vocabulary[4]);
}

TEST(VocabularyBuilderTest, getWordId)
{
  vocabularyBuilder.constructVocabularyFromASCIIWordsFile(
      "./test.words-unsorted.ascii");
  ASSERT_EQ(static_cast<size_t>(5), vocabularyBuilder._vocabulary.size());
  ASSERT_EQ(4 , vocabularyBuilder.getWordId("test5"));
  ASSERT_EQ(-1 , vocabularyBuilder.getWordId(":ee:entity:5"));
  ASSERT_EQ(-1 , vocabularyBuilder.getWordId("notInThere"));
}

TEST(VocabularyBuilderTest, constructVocabularyOntology)
{
  vocabularyBuilder.constructVocabularyFromOntology(
      "./test.ontology.ascii");
  ASSERT_EQ(static_cast<size_t>(16), vocabularyBuilder._vocabulary.size());
  ASSERT_EQ(":c:bar", vocabularyBuilder._vocabulary[0]);
  ASSERT_EQ(":r:has-instances", vocabularyBuilder._vocabulary[6]);
  ASSERT_EQ(":r:has-relations", vocabularyBuilder._vocabulary[7]);
  ASSERT_EQ(":r:r2", vocabularyBuilder._vocabulary[10]);
  ASSERT_EQ(":r:r2_(reversed)", vocabularyBuilder._vocabulary[11]);
  ASSERT_EQ(":t:class", vocabularyBuilder._vocabulary[12]);
  ASSERT_EQ(":t:count", vocabularyBuilder._vocabulary[13]);
  ASSERT_EQ(":t:entity", vocabularyBuilder._vocabulary[14]);
  ASSERT_EQ(":t:relation", vocabularyBuilder._vocabulary[15]);
}
}
