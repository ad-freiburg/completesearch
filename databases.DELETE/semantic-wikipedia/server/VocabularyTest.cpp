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
#include "./Vocabulary.h"

using std::string;
using std::vector;

using ad_utility::File;
namespace ad_semsearch
{
class VocabularyTest: public ::testing::Test
{
    virtual void SetUp()
    {
      _testPlainFile = "test.plain.vocabulary";
    }

    virtual void TearDown()
    {
    }

  public:
    string _testPlainFile;
};

TEST_F(VocabularyTest, asStringTest)
{
  Vocabulary emptyVocab;
  ASSERT_EQ("Vocabulary with size: 0", emptyVocab.asString());

  Vocabulary oneElem;
  oneElem.push_back("testelem");
  ASSERT_EQ("Vocabulary with size: 1; Word: testelem", oneElem.asString());

  Vocabulary twoElem;
  twoElem.push_back("testelem1");
  twoElem.push_back("testelem2");
  ASSERT_EQ("Vocabulary with size: 2; Words: testelem1, testelem2",
        twoElem.asString());

  Vocabulary fromFile;
  fromFile.readFromFile(_testPlainFile);
  ASSERT_EQ("Vocabulary with size: 5; Words: wordA0, ..., wordB4",
      fromFile.asString());
}

TEST_F(VocabularyTest, readFromFileTest)
{
  Vocabulary fromFile;
  fromFile.readFromFile(_testPlainFile);
  ASSERT_EQ(size_t(5), fromFile.size());
  ASSERT_EQ("Vocabulary with size: 5; Words: wordA0, ..., wordB4",
      fromFile.asString());
}

TEST_F(VocabularyTest, getIdForFullTextWordTest)
{
  Vocabulary fromFile;
  fromFile.readFromFile(_testPlainFile);

  Id word0 = getFirstId(IdType::WORD_ID);

  Id res;
  ASSERT_TRUE(fromFile.getIdForFullTextWord("wordA0", &res));
  ASSERT_EQ(word0, res);
  ASSERT_TRUE(fromFile.getIdForFullTextWord("wordB2", &res));
  ASSERT_EQ(word0 + 2, res);
  ASSERT_TRUE(fromFile.getIdForFullTextWord("wordB4", &res));
  ASSERT_EQ(word0 + 4, res);
  ASSERT_FALSE(fromFile.getIdForFullTextWord("foobar", &res));
}


TEST_F(VocabularyTest, getIdForOntologyWord)
{
  Vocabulary ontoVocab;
  ontoVocab.push_back(":e:entity0");
  ontoVocab.push_back(":e:entity1");

  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);
  Id res;
  ASSERT_TRUE(ontoVocab.getIdForOntologyWord(":e:entity0", &res));
  ASSERT_EQ(entity0, res);
  ASSERT_TRUE(ontoVocab.getIdForOntologyWord(":e:entity1", &res));
  ASSERT_EQ(entity0 + 1, res);
  ASSERT_FALSE(ontoVocab.getIdForOntologyWord(":e:foo", &res));
}

TEST_F(VocabularyTest, getIdRangeForFullTextPrefixTest)
{
  Vocabulary fromFile;
  fromFile.readFromFile(_testPlainFile);

  Id word0 = getFirstId(IdType::WORD_ID);
  IdRange retVal;
  // Match exactly one
  ASSERT_TRUE(fromFile.getIdRangeForFullTextPrefix("wordA1*", &retVal));
  ASSERT_EQ(word0 + 1, retVal._first);
  ASSERT_EQ(word0 + 1, retVal._last);

  // Match all
  ASSERT_TRUE(fromFile.getIdRangeForFullTextPrefix("word*", &retVal));
  ASSERT_EQ(word0, retVal._first);
  ASSERT_EQ(word0 + 4, retVal._last);

  // Match first two
  ASSERT_TRUE(fromFile.getIdRangeForFullTextPrefix("wordA*", &retVal));
  ASSERT_EQ(word0, retVal._first);
  ASSERT_EQ(word0 + 1, retVal._last);

  // Match last three
  ASSERT_TRUE(fromFile.getIdRangeForFullTextPrefix("wordB*", &retVal));
  ASSERT_EQ(word0 + 2, retVal._first);
  ASSERT_EQ(word0 + 4, retVal._last);

  ASSERT_FALSE(fromFile.getIdRangeForFullTextPrefix("foo*", &retVal));
}


TEST_F(VocabularyTest, bracketsOperaTest)
{
  Vocabulary fromFile;
  fromFile.readFromFile(_testPlainFile);
  Id word0 = getFirstId(IdType::WORD_ID);

  ASSERT_EQ("wordA0", fromFile[word0]);
  ASSERT_EQ("wordA1", fromFile[word0 + 1]);
  ASSERT_EQ("wordB2", fromFile[word0 + 2]);
  ASSERT_EQ("wordB3", fromFile[word0 + 3]);
  ASSERT_EQ("wordB4", fromFile[word0 + 4]);

  Vocabulary ontoVocab;
  ontoVocab.push_back(":e:entity0");
  ontoVocab.push_back(":e:entity1");
  Id entity0 = getFirstId(IdType::ONTOLOGY_ELEMENT_ID);

  ASSERT_EQ(":e:entity0", ontoVocab[entity0]);
  ASSERT_EQ(":e:entity1", ontoVocab[entity0 + 1]);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
