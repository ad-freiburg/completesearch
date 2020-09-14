// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Hannah Bast <bast>, Björn Buchhold <buchholb>

#include <gtest/gtest.h>
#include <string>
#include "./SemanticWikipediaParser.h"

namespace ad_semsearch
{
class SemanticWikipediaParserTest : public ::testing::Test
{
  public:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

// Test the parseText method.
TEST_F(SemanticWikipediaParserTest, parseText)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  SemanticWikipediaParser<STANDARD> swp2(1, 1);
  SemanticWikipediaParser<STANDARD> swp3(1, 0);
  SemanticWikipediaParser<OUTPUT_SPECIAL_CHARS> swp4(1, 10);
  swp.setSectionHeadersToSkip("semantic-wikipedia.sections_to_skip");
  swp.setRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  swp2.setSectionHeadersToSkip("semantic-wikipedia.sections_to_skip");
  swp2.setRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  swp3.setSectionHeadersToSkip("semantic-wikipedia.sections_to_skip");
  swp3.setRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  swp4.setSectionHeadersToSkip("semantic-wikipedia.sections_to_skip");
  swp4.setRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  swp.setWriteWikiDocIdPosting(false);
  swp2.setWriteWikiDocIdPosting(false);
  swp3.setWriteWikiDocIdPosting(false);
  swp4.setWriteWikiDocIdPosting(false);

  string wordsFileOutput;
  string docsFileOutput;
  int docId = 1;
  string title = "Test Title";

  // Test a basic sequence of words with upper and lower case.
  string text = "word Word2 wOrd3 woRd4";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(
      ":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t3\t150\t0\n"
      "test\t3\t2\t0\n"
      "title\t3\t2\t0\n"
      "word\t3\t2\t1\n"
      "word2\t3\t2\t2\n"
      "word3\t3\t2\t3\n"
      "word4\t3\t2\t4\n", wordsFileOutput);

  // Empty text is not accepted, so border case to test should be
  // a text consisting of one character only
  text = "a";
  assert(docId == 1);
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t4\t1\t0\n"
      "test\t4\t1\t0\n"
      "title\t4\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t5\t150\t0\n"
      "test\t5\t2\t0\n"
      "title\t5\t2\t0\n"
      "a\t5\t2\t1\n", wordsFileOutput);

  // Test a text with multiple spaces
  text = "word  word2";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t6\t1\t0\n"
      "test\t6\t1\t0\n"
      "title\t6\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t7\t150\t0\n"
      "test\t7\t2\t0\n"
      "title\t7\t2\t0\n"
      "word\t7\t2\t1\n"
      "word2\t7\t2\t2\n", wordsFileOutput);

  // Test if comments get ignored properly
  text = "<!-- foo -->word <!-- <foobar> some comment --> word2<!-- bar -->";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t8\t1\t0\n"
      "test\t8\t1\t0\n"
      "title\t8\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t9\t150\t0\n"
      "test\t9\t2\t0\n"
      "title\t9\t2\t0\n"
      "word\t9\t2\t1\n"
      "word2\t9\t2\t2\n", wordsFileOutput);

  // Test a text with special characters
  text = "Ta-da, it's an Encyclopædia";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t10\t1\t0\n"
      "test\t10\t1\t0\n"
      "title\t10\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t11\t150\t0\n"
      "test\t11\t2\t0\n"
      "title\t11\t2\t0\n"
      "ta-da\t11\t2\t1\n"
      "it's\t11\t2\t2\n"
      "an\t11\t2\t3\n"
      "encyclopædia\t11\t2\t4\n", wordsFileOutput);

  // Test if math tags get handled properly.
  text = "<math>x</math>word <math>1+1=2</math> word2<math>x</math>";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t12\t1\t0\n"
      "test\t12\t1\t0\n"
      "title\t12\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t13\t150\t0\n"
      "test\t13\t2\t0\n"
      "title\t13\t2\t0\n"
      "word\t13\t2\t1\n"
      "word2\t13\t2\t2\n", wordsFileOutput);

  // Test if other tags get ignored porperly
  text = "word <b>word2</B>";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t14\t1\t0\n"
      "test\t14\t1\t0\n"
      "title\t14\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t15\t150\t0\n"
      "test\t15\t2\t0\n"
      "title\t15\t2\t0\n"
      "word\t15\t2\t1\n"
      "word2\t15\t2\t2\n", wordsFileOutput);

  // Test if full stops get handled properly
  text = "word one. <b>word2</B>";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t16\t1\t0\n"
      "test\t16\t1\t0\n"
      "title\t16\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t17\t150\t0\n"
      "test\t17\t2\t0\n"
      "title\t17\t2\t0\n"
      "word\t17\t2\t1\n"
      "one\t17\t2\t2\n"
      ":ee:entity:testtitle:Test_Title\t18\t100\t0\n"
      "test\t18\t2\t0\n"
      "title\t18\t2\t0\n"
      "word2\t18\t2\t1\n", wordsFileOutput);

  // Test section handling
  text = "foo ==Section one==\n some words\n == References ==\n"
  "more words\n==== skip this ====\n==Now again== foobar\n===x=== xyz";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t19\t1\t0\n"
      "test\t19\t1\t0\n"
      "title\t19\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t20\t150\t0\n"
      "test\t20\t2\t0\n"
      "title\t20\t2\t0\n"
      "foo\t20\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t21\t1\t0\n"
      "test\t21\t1\t0\n"
      "title\t21\t1\t0\n"
      "section\t21\t1\t1\n"
      "one\t21\t1\t2\n"
      ":ee:entity:testtitle:Test_Title\t22\t100\t0\n"
      "test\t22\t2\t0\n"
      "title\t22\t2\t0\n"
      "section\t22\t2\t0\n"
      "one\t22\t2\t0\n"
      "some\t22\t2\t1\n"
      "words\t22\t2\t2\n"
      ":ee:entity:testtitle:Test_Title\t23\t1\t0\n"
      "test\t23\t1\t0\n"
      "title\t23\t1\t0\n"
      "now\t23\t1\t1\n"
      "again\t23\t1\t2\n"
      ":ee:entity:testtitle:Test_Title\t24\t100\t0\n"
      "test\t24\t2\t0\n"
      "title\t24\t2\t0\n"
      "now\t24\t2\t0\n"
      "again\t24\t2\t0\n"
      "foobar\t24\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t25\t1\t0\n"
      "test\t25\t1\t0\n"
      "title\t25\t1\t0\n"
      "now\t25\t1\t0\n"
      "again\t25\t1\t0\n"
      "x\t25\t1\t1\n"
      ":ee:entity:testtitle:Test_Title\t26\t100\t0\n"
      "test\t26\t2\t0\n"
      "title\t26\t2\t0\n"
      "now\t26\t2\t0\n"
      "again\t26\t2\t0\n"
      "x\t26\t2\t0\n"
      "xyz\t26\t2\t1\n", wordsFileOutput);

  // Now swp2 only writes the title at each pos.
  text = "foo ==Section one==\n some words\n ==References==\n"
  "more words\n==== skip this ====\n==Now again== foobar\n===x=== xyz";
  swp2.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t3\t150\t0\n"
      "test\t3\t2\t0\n"
      "title\t3\t2\t0\n"
      "foo\t3\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t4\t1\t0\n"
      "test\t4\t1\t0\n"
      "title\t4\t1\t0\n"
      "section\t4\t1\t1\n"
      "one\t4\t1\t2\n"
      ":ee:entity:testtitle:Test_Title\t5\t100\t0\n"
      "test\t5\t2\t0\n"
      "title\t5\t2\t0\n"
      "some\t5\t2\t1\n"
      "words\t5\t2\t2\n"
      ":ee:entity:testtitle:Test_Title\t6\t1\t0\n"
      "test\t6\t1\t0\n"
      "title\t6\t1\t0\n"
      "now\t6\t1\t1\n"
      "again\t6\t1\t2\n"
      ":ee:entity:testtitle:Test_Title\t7\t100\t0\n"
      "test\t7\t2\t0\n"
      "title\t7\t2\t0\n"
      "foobar\t7\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t8\t1\t0\n"
      "test\t8\t1\t0\n"
      "title\t8\t1\t0\n"
      "x\t8\t1\t1\n"
      ":ee:entity:testtitle:Test_Title\t9\t100\t0\n"
      "test\t9\t2\t0\n"
      "title\t9\t2\t0\n"
      "xyz\t9\t2\t1\n", wordsFileOutput);

  // Now swp3 only writes the words that actually occur each pos.
  text = "foo ==Section one==\n some words\n ==References==\n"
  "more words\n==== skip this ====\n==Now again== foobar\n===x=== xyz";
  swp3.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t2\t1\t0\n"
      "test\t2\t1\t0\n"
      "title\t2\t1\t0\n"
      "foo\t3\t2\t1\n"
      "section\t4\t1\t1\n"
      "one\t4\t1\t2\n"
      "some\t5\t2\t1\n"
      "words\t5\t2\t2\n"
      "now\t6\t1\t1\n"
      "again\t6\t1\t2\n"
      "foobar\t7\t2\t1\n"
      "x\t8\t1\t1\n"
      "xyz\t9\t2\t1\n", wordsFileOutput);

  // Test if lists are handled properly
  text = "foo * same \n *: item\n**next";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t27\t1\t0\n"
      "test\t27\t1\t0\n"
      "title\t27\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t28\t150\t0\n"
      "test\t28\t2\t0\n"
      "title\t28\t2\t0\n"
      "foo\t28\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t29\t100\t0\n"
      "test\t29\t2\t0\n"
      "title\t29\t2\t0\n"
      "same\t29\t2\t1\n"
      "item\t29\t2\t2\n"
      ":ee:entity:testtitle:Test_Title\t30\t100\t0\n"
      "test\t30\t2\t0\n"
      "title\t30\t2\t0\n"
      "next\t30\t2\t1\n", wordsFileOutput);

  // Test if lists are handled properly. Identical test
  // for numbered lists.
  text = "foo # same \n #: item\n##next";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t31\t1\t0\n"
      "test\t31\t1\t0\n"
      "title\t31\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t32\t150\t0\n"
      "test\t32\t2\t0\n"
      "title\t32\t2\t0\n"
      "foo\t32\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t33\t100\t0\n"
      "test\t33\t2\t0\n"
      "title\t33\t2\t0\n"
      "same\t33\t2\t1\n"
      "item\t33\t2\t2\n"
      ":ee:entity:testtitle:Test_Title\t34\t100\t0\n"
      "test\t34\t2\t0\n"
      "title\t34\t2\t0\n"
      "next\t34\t2\t1\n", wordsFileOutput);

  // Test if tables are handled properly
  text =
  "{| class=\"wikitable\"\n"
  "|- \n"
  "! header\n"
  "|- \n"
  "| row1\n"
  "|- "
  "|row2\n"
  "|}";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ(":ee:entity:testtitle:Test_Title\t35\t1\t0\n"
      "test\t35\t1\t0\n"
      "title\t35\t1\t0\n"
      ":ee:entity:testtitle:Test_Title\t36\t150\t0\n"
      "test\t36\t2\t0\n"
      "title\t36\t2\t0\n"
      ":ee:entity:testtitle:Test_Title\t37\t100\t0\n"
      "test\t37\t2\t0\n"
      "title\t37\t2\t0\n"
      "header\t37\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t38\t100\t0\n"
      "test\t38\t2\t0\n"
      "title\t38\t2\t0\n"
      "row1\t38\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t39\t100\t0\n"
      "test\t39\t2\t0\n"
      "title\t39\t2\t0\n"
      "row2\t39\t2\t1\n"
      ":ee:entity:testtitle:Test_Title\t40\t100\t0\n"
      "test\t40\t2\t0\n"
      "title\t40\t2\t0\n", wordsFileOutput);
}

TEST_F(SemanticWikipediaParserTest, commentStarts)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);

  string text = "<!-- Beginning<> -->Some <text> "
    "<!-- pretty <>boring text --> with comments <!-- the end<>-->";

  // Test for false positives.
  ASSERT_FALSE(swp.commentStarts(text, 23));

  // Test fake comment start
  ASSERT_FALSE(swp.commentStarts(text, 22));

  // Test recognition in the beginning.
  ASSERT_TRUE(swp.commentStarts(text, 0));

  // Test recognition in the middle.
  ASSERT_TRUE(swp.commentStarts(text, 32));

  // Test recognition in the end.
  ASSERT_TRUE(swp.commentStarts(text, 76));

  // Test for false positives inside comments.
  ASSERT_FALSE(swp.commentStarts(text, 77));
}

TEST_F(SemanticWikipediaParserTest, commentSize)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);

  string text = "<!-- Beginning<> -->Some <text> "
    "<!-- pretty <>boring text --> with comments <!-- the end<>-->";

  // Test recognition in the beginning.
  ASSERT_EQ(20, swp.commentSize(text, 0));

  // Test recognition in the middle.
  ASSERT_EQ(29, swp.commentSize(text, 32));

  // Test recognition in the end.
  ASSERT_EQ(17, swp.commentSize(text, 76));
}

TEST_F(SemanticWikipediaParserTest, mathStarts)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  string text = "Fancy math! <math>1+1=2</math> ... kind of";

  // Test math recognition.
  ASSERT_TRUE(swp.mathStarts(text, 12));
}

TEST_F(SemanticWikipediaParserTest, mathSize)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  string text = "Fancy math! <math>1+1=2</math> ... kind of";
  // Test math size calculation.
  ASSERT_EQ(18, swp.mathSize(text, 12));
}

TEST_F(SemanticWikipediaParserTest, templateStarts)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  string text = "Some {{template | xyz}}";

  // Test template recognition.
  ASSERT_TRUE(swp.templateStarts(text, 5));
  ASSERT_FALSE(swp.templateStarts(text, 6));
}

TEST_F(SemanticWikipediaParserTest, templateSize)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  string text = "Some {{template | xyz}}";

  // Test template size determination.
  ASSERT_EQ(18, swp.templateSize(text, 5));
}

TEST_F(SemanticWikipediaParserTest, tagSize)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  string text = "Test <blockcomment> tags </blockcomment>";

  // Test block size calculation.
  ASSERT_EQ(14, swp.tagSize(text, 5));
  ASSERT_EQ(15, swp.tagSize(text, 25));
}

TEST_F(SemanticWikipediaParserTest, handleLink)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  // Test external link handling
  string text = "Some [http://www.google.com] links "
    "[http://www.google.com google link] here";

  // The first links gets ignored entirely
  ASSERT_EQ(27, swp.handleLink(text, 5));
  // The second link get's skipped until the description starts
  ASSERT_EQ(57, swp.handleLink(text, 35));
}

// Test handleSection method
TEST_F(SemanticWikipediaParserTest, handleSection)
{
  SemanticWikipediaParser<STANDARD> swp(1, 10);
  swp.setSectionHeadersToSkip("semantic-wikipedia.sections_to_skip");

  string text = "=Section one=\n some words\n ==References==\n"
    "more words\n ==  Now meaningful again  == foo-bar\n ==References== xyz";

  // The first section is no problem and ahndling ends up after the header
  ASSERT_EQ(12, swp.handleSection(text, 0));
  // The references section should get skipped
  ASSERT_EQ(53, swp.handleSection(text, 27));
  // Test Processing the mid section with higher level
  ASSERT_EQ(81, swp.handleSection(text, 54));
  // The last references section should be skipped and
  // return the end / position of last char of the text
  ASSERT_EQ(109, swp.handleSection(text, 92));
}

TEST_F(SemanticWikipediaParserTest, testPrechunkMode)
{
  // Create a parser with minWordLength 1 and maxHeaderLevel 0
  // which are kind of the default settings used always.
  SemanticWikipediaParser<OUTPUT_SPECIAL_CHARS> swp(1, 0);
  swp.setSectionHeadersToSkip("semantic-wikipedia.sections_to_skip");
  swp.setRedirectMap("semantic-wikipedia-test.title_entity_pairs");
  swp.setPreserveCaseInOutput(true);
  swp.setWriteWikiDocIdPosting(false);

  string wordsFileOutput;
  string docsFileOutput;

  string title = "MyTitle";
  string text = "I'm it's&nbsp; &nbsp me Ta-da, hey. Cool!";

  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ("", docsFileOutput);
  ASSERT_EQ(
      ":title:MyTitle [1]\t2\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t2\t2\t0\n"
      ":ee:entity:mytitle:MyTitle\t2\t1\t0\n"
      "MyTitle\t2\t1\t0\n"
      ":title:MyTitle [2]\t3\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t3\t2\t0\n"
      "I'm\t3\t2\t1\n"
      "it's\t3\t2\t2\n"
      "me\t3\t2\t3\n"
      "Ta-da\t3\t2\t4\n"
      ",\t3\t2\t5\n"
      "hey\t3\t2\t6\n"
      ".\t3\t2\t7\n"
      ":title:MyTitle [3]\t4\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t4\t2\t0\n"
      "Cool\t4\t2\t1\n"
      "!\t4\t2\t2\n"
      ":title:MyTitle [4]\t5\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t5\t2\t0\n"
      , wordsFileOutput);

  text = "Pre- and postconditions are: great (no they aren't)?";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ("", docsFileOutput);
  ASSERT_EQ(
      ":title:MyTitle [1]\t6\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t6\t2\t0\n"
      ":ee:entity:mytitle:MyTitle\t6\t1\t0\n"
      "MyTitle\t6\t1\t0\n"
      ":title:MyTitle [2]\t7\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t7\t2\t0\n"
      "Pre-\t7\t2\t1\n"
      "and\t7\t2\t2\n"
      "postconditions\t7\t2\t3\n"
      "are\t7\t2\t4\n"
      ":\t7\t2\t5\n"
      "great\t7\t2\t6\n"
      "(\t7\t2\t7\n"
      "no\t7\t2\t8\n"
      "they\t7\t2\t9\n"
      "aren't\t7\t2\t10\n"
      ")\t7\t2\t11\n"
      "?\t7\t2\t12\n"
      ":title:MyTitle [3]\t8\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t8\t2\t0\n", wordsFileOutput);

  text = "John's house is {{bullshit}} nice (100-200)";
  swp.parseText(text, title, &wordsFileOutput, &docsFileOutput);
  ASSERT_EQ("", docsFileOutput);
  ASSERT_EQ(
      ":title:MyTitle [1]\t9\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t9\t2\t0\n"
      ":ee:entity:mytitle:MyTitle\t9\t1\t0\n"
      "MyTitle\t9\t1\t0\n"
      ":title:MyTitle [2]\t10\t2\t0\n"
      ":url:http://en.wikipedia.org/wiki/MyTitle\t10\t2\t0\n"
      "John's\t10\t2\t1\n"
      "house\t10\t2\t2\n"
      "is\t10\t2\t3\n"
      "nice\t10\t2\t4\n"
      "(\t10\t2\t5\n"
      "100-200\t10\t2\t6\n"
      ")\t10\t2\t7\n"
      , wordsFileOutput);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
