// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "sentence/Sentence.h"
#include "util/ContextDecomposerUtil.h"
#include "base/SemanticWikipediaReader.h"


// TODO(elmar): tear-down?
// ContextDecomposerBase * decomp = new RobustContextDecomposer(true);
// SemanticWikipediaDecomposer swc("Test", "Test", true, decomp);

void getTestSentence(Sentence<DefaultToken> * sentence)
{
  string index =
  ":title:Test\t0\t0\t1\t*\t*\t*\n"
  ":url:http://Test\t0\t0\t1\t*\t*\t*\n"
  ":e:entity:testtitle1:\t0\t0\t1\t*\t*\t*\n"
  "This\t0\t0\t0\tDT\tB-NP\t*\n"
  "is\t0\t0\t0\tVBZ\tB-VP\t*\n"
  "a\t0\t0\t0\tDT\tB-NP\t*\n"
  "__eofentity\t0\t0\t0\tNN\tI-NP\t*\n"
  ":e:entity:physicalentity:object:whole:livingthing:"
      "organism:person:intellectual:scholar:philosopher:aristotle:"
      "Aristotle\t0\t0\t2\t*\t*\t*\n"
  "simple\t0\t0\t0\tJJ\tI-NP\t*\n"
  "__eofentity\t0\t0\t0\tNN\tI-NP\t*\n"
  "test\t0\t0\t0\tNN\tI-NP\t*\n"
  "but\t0\t0\t0\tCC\tO\t*\n"
  "it\t0\t0\t0\tPRP\tB-NP\t*\n"
  "should\t0\t0\t0\tMD\tB-VP\t*\n"
  "work\t0\t0\t0\tVB\tI-VP\t*\n"
  "nontheless\t0\t0\t0\tJJ\tB-ADJP\t*\n"
  ".\t0\t0\t0\tJJ\tO\t*\n";
  std::istringstream is(index);
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
  // sentence->printSentence();
}

// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, testAppendWord)
{
  Sentence<DefaultToken> s;
  DefaultToken * t = s.getNewToken();
  // Phrase * p = s.getNewPhrase("B-NP");

  // s.appendPhrase(p);
  t->tokenString = "Test";
  t->posTag = "DT";
  t->npTag = "B-NP";
  s.appendWord(t);
  // p->addWord(t, 0);

  t = s.getNewToken();
  t->tokenString = "NextWord";
  t->posTag = "CC";
  t->npTag = "B-VP";
  s.appendWord(t);
  // p->addWord(t, 1);

  // s.printSentence();
  ASSERT_EQ(size_t(2), s.getWords().size());
  ASSERT_EQ("Test", s.getWords()[0]->tokenString);
  ASSERT_EQ("NextWord", s.getWords()[1]->tokenString);
  ASSERT_EQ("DT", s.getWords()[0]->posTag);
  ASSERT_EQ("CC", s.getWords()[1]->posTag);
  ASSERT_EQ("B-NP", s.getWords()[0]->npTag);
  ASSERT_EQ("B-VP", s.getWords()[1]->npTag);
  ASSERT_EQ(0, s.getWords()[0]->tokenWordPosition);
  ASSERT_EQ(1, s.getWords()[1]->tokenWordPosition);
  ASSERT_EQ(0, s.getWords()[0]->tokenCharPosition);
  ASSERT_EQ(5, s.getWords()[1]->tokenCharPosition);
  ASSERT_EQ("Test NextWord", s.asString());
  ASSERT_EQ("DT    CC    ", s.asPOSTagString());
}

// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, testAppendPhrase)
{
  Sentence<DefaultToken> sentence;
  getTestSentence(&sentence);
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  ASSERT_EQ(size_t(8), phrases.size());
  ASSERT_EQ(size_t(8), phrases[5]->getWordsEndIndex());
  ASSERT_EQ(size_t(7), phrases[5]->getWordsStartIndex());
  ASSERT_EQ(size_t(0), phrases[0]->getWordsStartIndex());
  ASSERT_EQ(size_t(0), phrases[0]->getWordsEndIndex());
  ASSERT_EQ(size_t(9), phrases[6]->getWordsStartIndex());
  ASSERT_EQ(size_t(9), phrases[6]->getWordsEndIndex());
  ASSERT_EQ(size_t(2), phrases[5]->getWords().size());
  ASSERT_EQ(size_t(1), phrases[0]->getWords().size());
  ASSERT_EQ(size_t(1), phrases[6]->getWords().size());
  ASSERT_EQ("ADJP", phrases[6]->getType());
  ASSERT_EQ("NP", phrases[0]->getType());
  ASSERT_EQ("VP", phrases[5]->getType());
  ASSERT_EQ("O", phrases[7]->getType());
  ASSERT_EQ("should", phrases[5]->getFirstWord().tokenString);
  ASSERT_EQ("work", phrases[5]->getLastWord().tokenString);
  ASSERT_EQ("This", phrases[0]->getLastWord().tokenString);
  ASSERT_EQ("This", phrases[0]->getFirstWord().tokenString);
  ASSERT_EQ("nontheless", phrases[6]->getLastWord().tokenString);
  ASSERT_EQ(".", phrases[7]->getLastWord().tokenString);
  ASSERT_EQ("should work", phrases[5]->asString());
  ASSERT_EQ("This", phrases[0]->asString());
  ASSERT_EQ("nontheless", phrases[6]->asString());
  ASSERT_EQ(".", phrases[7]->asString());
}

// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, testappendEntity)
{
  Sentence<DefaultToken> sentence;
  getTestSentence(&sentence);
  vector<DefaultToken *> const & words = sentence.getWords();
  ASSERT_TRUE(words[0]->isPartOfEntity);
  ASSERT_TRUE(words[2]->isPartOfEntity);
  ASSERT_TRUE(words[3]->isPartOfEntity);
  ASSERT_FALSE(words[4]->isPartOfEntity);
  ASSERT_EQ(":e:entity:physicalentity:object:whole:livingthing:org"
      "anism:person:intellectual:scholar:philosopher:aristotle:Aristotle",
      words[3]->_entity->getEntityId());
  ASSERT_EQ(":e:entity:testtitle1:", words[1]->_entity->getEntityId());
}


// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, sentenceUrlTitle)
{
  Sentence<DefaultToken> sentence;
  getTestSentence(&sentence);
  ASSERT_EQ("Test", sentence.getDocTitle());
  ASSERT_EQ("http://Test", sentence.getDocUrl());
}

// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, Phrase_matchWordsRegex)
{
  Sentence<DefaultToken> sentence;
  getTestSentence(&sentence);
  vector<Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  ASSERT_EQ("VP", phrases[5]->getType());
  ASSERT_TRUE(phrases[5]->matchWordsRegex("should work"));
  ASSERT_TRUE(phrases[5]->matchWordsRegex("sho.*rk"));
  ASSERT_TRUE(phrases[0]->matchWordsRegex("This"));
  ASSERT_TRUE(phrases[6]->matchWordsRegex("nontheless"));
  ASSERT_FALSE(phrases[5]->matchWordsRegex("should not work"));
  ASSERT_EQ(1, 1);
}

// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, Phrase_matchPOSRegex)
{
  Sentence<DefaultToken> sentence;
  getTestSentence(&sentence);
  vector <Phrase<DefaultToken> *> const & phrases = sentence.getPhrases();
  ASSERT_EQ("VP", phrases[5]->getType());
  ASSERT_TRUE(phrases[5]->matchPOSRegex(".*"));
  ASSERT_TRUE(phrases[5]->matchPOSRegex("MD.*VB.*"));
  ASSERT_TRUE(phrases[6]->matchPOSRegex("JJ.*"));
  ASSERT_TRUE(phrases[0]->matchPOSRegex("DT.*"));
  ASSERT_FALSE(phrases[2]->matchPOSRegex("VBG.*"));
  ASSERT_EQ(1, 1);
}






int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
