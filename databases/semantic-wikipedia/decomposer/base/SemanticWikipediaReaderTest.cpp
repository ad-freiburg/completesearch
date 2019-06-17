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

namespace ad_decompose
{
// Test the parseText method.
TEST(SemanticWikipediaReaderTest, testParseNextSentence)
{
  string index = ":title:Test\t0\t0\t1\t*\t*\t*\t*\n"
    ":url:http://Test\t0\t0\t1\t*\t*\t*\t*\n"
    ":e:entity:testtitle1:\t0\t0\t1\t*\t*\t*\t*\n"
    "This\t0\t0\t0\tDT\tB-NP\tPP(\tREL(,LIT(\n"
    "is\t0\t0\t0\tVBZ\tB-VP\t*\t*\n"
    "a\t0\t0\t0\tDT\tB-NP\t*\t*\n"
    "__eofentity\t0\t0\t0\tNN\tI-NP\t*\t*\n"
    ":e:entity:physicalentity:object:whole:livingthing:"
    "organism:person:intellectual:scholar:philosopher:aristotle:"
    "Aristotle\t0\t0\t2\t*\t*\t*\t*\n"
    "simple\t0\t0\t0\tJJ\tI-NP\t*\t*\n"
    "__eofentity\t0\t0\t0\tNN\tI-NP\t*\t*\n"
    "test\t0\t0\t0\tNN\tI-NP\t*\t*\n"
    "but\t0\t0\t0\tCC\tO\t*\t*\n"
    "it\t0\t0\t0\tPRP\tB-NP\t*\t*\n"
    "should\t0\t0\t0\tMD\tB-VP\t*\t*\n"
    "work\t0\t0\t0\tVB\tI-VP\t*\t*\n"
    "nontheless\t0\t0\t0\tJJ\tB-ADJP\t*\t*\n"
    ".\t0\t0\t0\tJJ\tO\t*\t*\n"
    ":title:Test\t1\t0\t1\t*\t*\t*\t*\n"
    ":url:http://Test\t1\t0\t1\t*\t*\t*\t*\n"
    ":e:entity:testtitle1:\t1\t0\t1\t*\t*\t*\t*\n"
    "This\t1\t0\t0\tDT\tB-NP\t*\t*\n"
    "is\t1\t0\t0\tVBZ\tB-VP\tS(,S(\tLIT)\n"
    "another\t1\t0\t0\tDT\tB-NP\t*\t*\n"
    "__eofentity\t1\t0\t0\tNN\tI-NP\t*\t*\n"
    ":e:entity:physicalentity:object:whole:livingthing:"
    "organism:person:intellectual:scholar:philosopher:aristotle:"
    "Aristotle\t1\t0\t2\t*\t*\t*\t*\n"
    "simple\t1\t0\t0\tJJ\tI-NP\t*\t*\n"
    "__eofentity\t1\t0\t0\tNN\tI-NP\t*\t*\n"
    "test\t1\t0\t0\tNN\tI-NP\t*\t*\n"
    "but\t1\t0\t0\tCC\tO\t*\t*\n"
    "it\t1\t0\t0\tPRP\tB-NP\t*\t*\n"
    "should\t1\t0\t0\tMD\tB-VP\t*\t*\n"
    "work\t1\t0\t0\tVB\tI-VP\t*\t*\n"
    "even\t1\t0\t0\tJJ\tB-ADJP\t*\t*\n"
    "better\t1\t0\t0\tJJ\tB-ADJP\t*\t*\n"
    ".\t1\t0\t0\tJJ\tO\t*\t*\n";

  std::istringstream is(index);
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputStream(&is);
  Sentence<DefaultToken> s;
  ASSERT_TRUE(swr.parseNextSentence(&s));
  ASSERT_EQ(size_t(11), s.getWords().size());
  ASSERT_EQ("This", s.getWords()[0]->tokenString);
  ASSERT_EQ("nontheless", s.getWords()[9]->tokenString);
  ASSERT_EQ(".", s.getWords()[10]->tokenString);
  ASSERT_EQ("DT", s.getWords()[0]->posTag);
  ASSERT_EQ("B-NP", s.getWords()[0]->npTag);
  ASSERT_EQ("PP(", s.getWords()[0]->cTag);
  ASSERT_EQ("REL(,LIT(", s.getWords()[0]->brTag);
  Sentence<DefaultToken> s2;
  ASSERT_TRUE(swr.parseNextSentence(&s2));
  ASSERT_EQ("This", s2.getWords()[0]->tokenString);
  ASSERT_EQ("better", s2.getWords()[10]->tokenString);
  ASSERT_EQ("VBZ", s2.getWords()[1]->posTag);
  ASSERT_EQ("B-VP", s2.getWords()[1]->npTag);
  ASSERT_EQ("S(,S(", s2.getWords()[1]->cTag);
  ASSERT_EQ("LIT)", s2.getWords()[1]->brTag);
  ASSERT_EQ(".", s2.getWords()[11]->tokenString);
  ASSERT_EQ(size_t(12), s2.getWords().size());
  ASSERT_FALSE(swr.parseNextSentence(&s2));
}




int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
