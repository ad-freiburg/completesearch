// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "base/SemanticWikipediaReader.h"
#include "decomposer-rule-deep/DeepContextMarker.h"
#include "decomposer-rule-deep/DeepContextRecombiner.h"

namespace ad_decompose
{
typedef DeepContextMarker::DeepParserTerminalNode TNode;

void getTestSentenceA(Sentence<DeepToken> * sentence)
{
  string index = ":title:Anarchism[9]\t10\t2\t0\t*\t*\t*\t*\n"
      ":url:http://en.wikipedia.org/wiki/Anarchism\t10\t2\t0\t*\t*\t*\t*\n"
      ":wikiDoc:Anarchism\t10\t0\t0\t*\t*\t*\t*\n"
      "Anarchists\t10\t2\t1\tNNS\t*\t(NP\t*\n"
      "schools\t10\t2\t2\tNNS\t*\t)\t*\n"
      "may\t10\t2\t3\tMD\t*\t*\t*\n"
      "differ\t10\t2\t4\tVB\t*\t*\t*\n"
      "from\t10\t2\t5\tIN\t*\t(PP\t*\n"
      "each\t10\t2\t6\tDT\t*\t(NP\t*\n"
      "other\t10\t2\t7\tJJ\t*\t),)\t*\n"
      "fundamentally\t10\t2\t8\tRB\t*\t(ADVP,)\t*\n"
      ",\t10\t2\t9\t,\t*\t*\t*\n"
      "supporting\t10\t2\t10\tVBG\t*\t(S,(VP\t*\n"
      "anything\t10\t2\t11\tNN\t*\t(NP\t*\n"
      "from\t10\t2\t12\tIN\t*\t(PP\t*\n"
      "extreme\t10\t2\t13\tJJ\t*\t(NP\t*\n"
      ":e:entity:abstraction:psychologicalfeature:cognition:content:belief:"
      "doctrine:individualism:Individualism\t10\t2\t14\t*\t*\t*\t*\n"
      ":ee:entity:individualism:Individualism\t10\t2\t14\t*\t*\t*\t*\n"
      "individualism\t10\t2\t14\tNN\t*\t),)\t*\n"
      "__eofentity\t10\t0\t14\t*\t*\t*\t*\n"
      "to\t10\t2\t15\tTO\t*\t(S,(VP\t*\n"
      "complete\t10\t2\t16\tVB\t*\t(VP\t*\n"
      ":e:entity:abstraction:psychologicalfeature:cognition:attitude:orient"
      "ation:politicalorientation:collectivism:Collectivism\t10\t2\t17"
      "\t*\t*\t*\t*\n"
      ":ee:entity:collectivism:Collectivism\t10\t2\t17\t*\t*\t*\t*\n"
      "collectivism\t10\t2\t17\tNN\t*\t(NP,),),),),),)\t*\n"
      "__eofentity\t10\t0\t17\t*\t*\t*\t*\n"
      ".\t10\t2\t18\t.\t*\t)\t*";

  std::istringstream is(index);
  SemanticWikipediaReader<DeepToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
}

void getTestSentenceB(Sentence<DeepToken> * sentence)
{
  // This sentence closes more brackets, than it opens.
  string index = ":title:Anarchism[9]\t10\t2\t0\t*\t*\t*\t*\n"
      ":url:http://en.wikipedia.org/wiki/Anarchism\t10\t2\t0\t*\t*\t*\t*\n"
      "each\t10\t2\t6\tDT\t*\t(NP\t*\n"
      "other\t10\t2\t7\tJJ\t*\t),)\t*\n"
      "fundamentally\t10\t2\t8\tRB\t*\t(ADVP,)\t*\n"
      ",\t10\t2\t9\t,\t*\t*\t*\n"
      "supporting\t10\t2\t10\tVBG\t*\t(S,(VP\t*\n"
      "anything\t10\t2\t11\tNN\t*\t(NP,),)\t*\n"
      "from\t10\t2\t12\tIN\t*\t(PP\t*\n"
      "extreme\t10\t2\t13\tJJ\t*\t(NP\t*\n"
      ":e:entity:abstraction:psychologicalfeature:cognition:content:belief"
      ":doctrine:individualism:Individualism\t10\t2\t14\t*\t*\t*\t*\n"
      ":ee:entity:individualism:Individualism\t10\t2\t14\t*\t*\t*\t*\n"
      "individualism\t10\t2\t14\tNN\t*\t),)\t*\n"
      "__eofentity\t10\t0\t14\t*\t*\t*\t*\n"
      "to\t10\t2\t15\tTO\t*\t(S,(VP\t*\n"
      "complete\t10\t2\t16\tVB\t*\t(VP\t*\n"
      ":e:entity:abstraction:psychologicalfeature:cognition:attitude:orie"
      "ntation:politicalorientation:collectivism:Collectivism\t10\t2\t"
      "17\t*\t*\t*\t*\n"
      ":ee:entity:collectivism:Collectivism\t10\t2\t17\t*\t*\t*\t*\n"
      "collectivism\t10\t2\t17\tNN\t*\t(NP,),),),),),)\t*\n"
      "__eofentity\t10\t0\t17\t*\t*\t*\t*\n"
      ".\t10\t2\t18\t.\t*\t*\t*";

  std::istringstream is(index);
  SemanticWikipediaReader<DeepToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
}

// Test the addChild method.
TEST(DeepParserSentenceParser, testParseSentence)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  getTestSentenceA(&sentence);
  bool parseResult = parser.parseSentence(sentence, &rootList);
  ASSERT_TRUE(parseResult);
  ASSERT_EQ(size_t(6), rootList[0].getChildren().size());
  DeepContextMarker::DeepParserNodeList const & childList =
      rootList[0].getChildren().back().getChildren();
  ASSERT_EQ(size_t(2), childList.size());
  ASSERT_EQ(DeepParserNodeTypes::VP, childList[0].type());
  ASSERT_EQ(DeepParserNodeTypes::TERMINAL, childList[1].type());
  ASSERT_EQ(size_t(2), childList[0].getChildren().size());
  DeepContextMarker::DeepParserNodeList const & childChildList =
      childList[0].getChildren()[1].getChildren();
  ASSERT_EQ(size_t(3), childChildList.size());
  ASSERT_EQ(DeepParserNodeTypes::TERMINAL, childChildList[0].type());
  ASSERT_EQ(DeepParserNodeTypes::PP, childChildList[1].type());
  ASSERT_EQ(DeepParserNodeTypes::S, childChildList[2].type());
  ASSERT_EQ(size_t(1), childChildList[2].getChildren().size());
  ASSERT_EQ(size_t(2),
      childChildList[2].getChildren()[0].getChildren().size());
  ASSERT_EQ(DeepParserNodeTypes::TERMINAL,
      childChildList[2].getChildren()[0].getChildren()[0].type());
  TNode const & node = static_cast<TNode const &>
    (childChildList[2].getChildren()[0].getChildren()[0]);
  ASSERT_EQ(size_t(1), node.getTerminals().size());
  ASSERT_EQ("to", node.getTerminals()[0]->tokenString);

  DeepContextMarker::DeepParserNodeList rootListB;
  Sentence<DeepToken> sentenceB;
  getTestSentenceB(&sentenceB);
  parseResult = parser.parseSentence(sentenceB, &rootListB);
  ASSERT_FALSE(parseResult);

  DeepContextMarker::DeepParserNodeList rootListC;
  Sentence<DeepToken> sentenceC;
  string sentenceCStr = "(S (NP inflation) (VP could (VP increase ) "
      "(SBAR if (S (NP the plan (VP was (VP implemented )))))))";
  parseResult = parser.parseSentenceFromString<TNode>(sentenceCStr,
      &sentenceC, &rootListC, DeepParserNodeTypes::stringToTypeMap);
  // Correct string representation has an additional root node.
  string treeStr = "(ROOT (S (NP inflation )(VP could (VP increase )"
      "(SBAR if (S (NP the plan (VP was (VP implemented ))))))))";
  // Verify tree structure here.
  ASSERT_TRUE(parseResult);
  ASSERT_EQ(treeStr, rootListC[0].treeAsFlatString());
  DeepContextMarker::DeepParserNodeList const & childListC =
      rootListC[0].getChildren().back().getChildren();
  ASSERT_EQ(size_t(2), childListC.size());
  ASSERT_EQ(DeepParserNodeTypes::NP, childListC[0].type());
  ASSERT_EQ(DeepParserNodeTypes::VP, childListC[1].type());
  DeepContextMarker::DeepParserNodeList const & childChildListC =
      childListC[1].getChildren();
  ASSERT_EQ(size_t(3), childChildListC.size());
  ASSERT_EQ(DeepParserNodeTypes::TERMINAL, childChildListC[0].type());
  ASSERT_EQ(DeepParserNodeTypes::VP, childChildListC[1].type());
  ASSERT_EQ(DeepParserNodeTypes::SBAR, childChildListC[2].type());
}

// Test the testFindEnumerations method.
TEST(DeepContextMarker, testFindEnumerations)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  vector<vector<size_t> > enumerations;
  string sentenceStr = "(S A (NP B C D) (NP E F G H) (NP I J K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  marker.findEnumerations(rootList[0].getChildren().back(),
      &enumerations);
  ASSERT_EQ(size_t(1), enumerations.size());
  ASSERT_EQ(size_t(3), enumerations[0].size());
  ASSERT_EQ(size_t(1), enumerations[0][0]);
  ASSERT_EQ(size_t(2), enumerations[0][1]);
  ASSERT_EQ(size_t(3), enumerations[0][2]);

  DeepContextMarker::DeepParserNodeList rootListB;
  Sentence<DeepToken> sentenceB;
  vector<vector<size_t> > enumerationsB;
  string sentenceBStr = "(S A (NP B C ) D (NP E F G) H (NP I J K))";
  parseResult = parser.parseSentenceFromString<TNode>(sentenceBStr,
       &sentenceB, &rootListB, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  marker.findEnumerations(rootListB[0].getChildren().back(),
      &enumerationsB);
  ASSERT_EQ(size_t(0), enumerationsB.size());

  Sentence<DeepToken> sentenceC;
  DeepContextMarker::DeepParserNodeList rootListC;
  vector<vector<size_t> > enumerationsC;
  string sentenceCStr = "(S A (NP B C ) and (NP E F G) or (NP I J X K))";
  parseResult = parser.parseSentenceFromString<TNode>(sentenceCStr,
  &sentenceC, &rootListC, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  marker.findEnumerations(rootListC[0].getChildren().back(),
      &enumerationsC);
  ASSERT_EQ(size_t(1), enumerationsC.size());
  ASSERT_EQ(size_t(3), enumerationsC[0].size());
  ASSERT_EQ(size_t(1), enumerationsC[0][0]);
  ASSERT_EQ(size_t(3), enumerationsC[0][1]);
  ASSERT_EQ(size_t(5), enumerationsC[0][2]);
}

// Test the markLeftMostLeaf method.
TEST(DeepContextMarker, markLeftMostLeaf)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (NP B C D) (NP E F G H) (NP I J K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  marker.markLeftMostLeaf(rootList[0], DeepTokenMarks::C_STAR);
  ASSERT_EQ(DeepTokenMarks::C_STAR, words.front()->marks[0]);
  ASSERT_EQ(size_t(1), words.front()->marks.size());
  marker.markLeftMostLeaf(rootList[0].getChildren().front().getChildren()[2],
      DeepTokenMarks::C);
  ASSERT_EQ(DeepTokenMarks::C, words[4]->marks[0]);
  ASSERT_EQ(size_t(1), words[4]->marks.size());
}

// Test the markRightMostLeaf method.
TEST(DeepContextMarker, markRightMostLeaf)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (NP B C D) (NP E F G H) (NP I J K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  marker.markRightMostLeaf(rootList[0], DeepTokenMarks::C_STAR);
  ASSERT_EQ(DeepTokenMarks::C_STAR, words.back()->marks[0]);
  ASSERT_EQ(size_t(1), words.back()->marks.size());
  marker.markRightMostLeaf(rootList[0].getChildren().front().getChildren()[2],
      DeepTokenMarks::C);
  ASSERT_EQ(DeepTokenMarks::C, words[7]->marks[0]);
  ASSERT_EQ(size_t(1), words[7]->marks.size());
}

// Test the isEnumerationContinuation method.
TEST(DeepContextMarker, testIsEnumerationContinuation)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  string sentenceStr = "(S A (NP B C ) and (NP E F G) or (NP I J X K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
  &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  DeepContextMarker::DeepParserNode const & node =
      rootList[0].getChildren().front();
  DeepContextMarker::DeepParserNode const & cNode = node.getChildren()[2];
  DeepContextMarker::DeepParserNode const & cNodeB = node.getChildren()[4];
  DeepContextMarker::DeepParserNode const & notCNode = node.getChildren()[1];
  ASSERT_TRUE(marker.isEnumerationContinuation(cNode));
  ASSERT_TRUE(marker.isEnumerationContinuation(cNodeB));
  ASSERT_FALSE(marker.isEnumerationContinuation(notCNode));
}

// Test the markEnumerations method.
TEST(DeepContextMarker, testMarkSubtree)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (NP B C D) (NP E F G H) (NP I J K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  // markEnumerations does not encapsulate the sentence
  // in an ENUM but only discovers the inner enumerations.
  marker.markSubtree(rootList[0]);
  ASSERT_EQ(size_t(4), words[1]->marks.size());
  ASSERT_EQ(DeepTokenMarks::ENUM, words[1]->marks[0]);
  ASSERT_EQ(DeepTokenMarks::C, words[1]->marks[1]);
  ASSERT_EQ(DeepTokenMarks::C_H, words[1]->marks[2]);
  ASSERT_EQ(size_t(3), words[3]->marks.size());
  ASSERT_EQ(DeepTokenMarks::CLOSE, words[3]->marks[0]);
  ASSERT_EQ(size_t(3), words[4]->marks.size());
  ASSERT_EQ(DeepTokenMarks::C, words[4]->marks[0]);
  ASSERT_EQ(size_t(3), words[7]->marks.size());
  ASSERT_EQ(DeepTokenMarks::CLOSE, words[7]->marks[0]);
  ASSERT_EQ(size_t(3), words[8]->marks.size());
  ASSERT_EQ(DeepTokenMarks::C, words[8]->marks[0]);
  ASSERT_EQ(size_t(5), words[10]->marks.size());
  ASSERT_EQ(DeepTokenMarks::CLOSE, words[10]->marks[0]);
  ASSERT_EQ(DeepTokenMarks::CLOSE, words[10]->marks[1]);

  DeepContextMarker::DeepParserNodeList rootListB;
  Sentence<DeepToken> sentenceB;
  vector<DeepToken *> const & wordsB = sentenceB.getWords();
  string sentenceBStr = "(S A (NP B C ) and (NP E F G) or (NP I J X K))";
  parseResult = parser.parseSentenceFromString<TNode>(sentenceBStr,
  &sentenceB, &rootListB, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  // markEnumerations does not encapsulate the sentence
  // in an ENUM but only discovers the inner enumerations.
  marker.markSubtree(rootListB[0]);
  ASSERT_EQ(size_t(3), wordsB[8]->marks.size());
  ASSERT_EQ(size_t(5), wordsB[11]->marks.size());
  ASSERT_EQ(DeepTokenMarks::C, wordsB[8]->marks[0]);
  ASSERT_EQ(DeepTokenMarks::C_H, wordsB[8]->marks[1]);
  ASSERT_EQ(DeepTokenMarks::C, wordsB[8]->marks[2]);
  ASSERT_EQ(DeepTokenMarks::CLOSE, wordsB[11]->marks[0]);
  ASSERT_EQ(DeepTokenMarks::CLOSE, wordsB[11]->marks[1]);
  ASSERT_EQ(DeepTokenMarks::CLOSE, wordsB[11]->marks[2]);
  ASSERT_EQ(DeepTokenMarks::CLOSE, wordsB[11]->marks[3]);
  ASSERT_EQ(DeepTokenMarks::CLOSE, wordsB[11]->marks[4]);
}

// Test the isRelativeClause method.
TEST(DeepContextMarker, testIsRelativeClause)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  // vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (NP B C ) "
      "(SBAR (WHNP which) ) and (NP E F G) or (NP I J X K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
  &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  DeepContextMarker::DeepParserNode & node =
      *(rootList[0].getChild(0)->getChild(1));
  DeepContextMarker::DeepParserNode & nodeB =
      *(rootList[0].getChild(0)->getChild(2));
  ASSERT_FALSE(marker.isRelativeClause(node));
  ASSERT_TRUE(marker.isRelativeClause(nodeB));
}

// Test the attachRelativeClause method.
TEST(DeepContextMarker, testAttachRelativeClause)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  // vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (NP B C ) (PP of )"
      " (SBAR (WHNP which) ) and (NP E F G) or (NP I J X K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
  &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  DeepContextMarker::DeepParserNode & node =
      *(rootList[0].getChild(0));
  size_t head;
  ASSERT_TRUE(marker.attachRelativeClause(node, size_t(3), &head));
  ASSERT_EQ(size_t(1), head);
}

// Test the nestRelativeClauses method.
TEST(DeepContextMarker, testNestRelativeClauses)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  // vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (NP B C ) (PP of )"
      " (SBAR (WHNP which) ) and (NP E F G) or (NP I J X K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
  &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  DeepContextMarker::DeepParserNode & node =
      *(rootList[0].getChild(0));
  marker.nestRelativeClauses(node);
  ASSERT_EQ("(S A (NP (NP B C )(PP of )(SBAR (WHNP which )))and "
      "(NP E F G )or (NP I J X K ))", node.treeAsFlatString());

  sentenceStr = "(S A (NP B C ) (PP of )"
      " (SBAR (WHNP which) ) and (NP E F G) (PP of) (SBAR (WHNP which ) "
      "(NP noun) (SBAR (WHNP which))) or (NP I J X K))";
  DeepContextMarker::DeepParserNodeList rootListB;
  Sentence<DeepToken> sentenceB;
  parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
      &sentenceB, &rootListB, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker::DeepParserNode & nodeB =
      *(rootListB[0].getChild(0));
  marker.nestRelativeClauses(nodeB);
  ASSERT_EQ("(S A (NP (NP B C )(PP of )(SBAR (WHNP which )))and "
      "(NP (NP E F G )(PP of )(SBAR (WHNP which )(NP (NP noun )"
      "(SBAR (WHNP which )))))or (NP I J X K ))", nodeB.treeAsFlatString());
}

// Test the isC_HAT method.
TEST(DeepContextMarker, testIsC_HAT)
{
  DeepParserSentenceParser parser;
  DeepContextMarker::DeepParserNodeList rootList;
  Sentence<DeepToken> sentence;
  // vector<DeepToken *> const & words = sentence.getWords();
  string sentenceStr = "(S A (PP According to AE ) (PP of )"
      " (SBAR (WHNP which) ) and (NP E F G) or (NP I J X K))";
  bool parseResult = parser.parseSentenceFromString<TNode>(sentenceStr,
  &sentence, &rootList, DeepParserNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepContextMarker marker;
  DeepContextMarker::DeepParserNode & node =
      *(rootList[0].getChild(0)->getChild(1));
  ASSERT_TRUE(marker.isC_HAT(node));
  ASSERT_FALSE(marker.isC_HAT(*rootList[0].getChild(0)->getChild(2)));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
