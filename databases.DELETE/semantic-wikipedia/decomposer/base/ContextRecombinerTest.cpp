// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <string>
#include <vector>
#include "sentence/Sentence.h"
#include "./ContextRecombiner.h"
#include "./SemanticWikipediaReader.h"
#include "./TreeNode.h"
#include "./RecombinerBase.h"

namespace ad_decompose
{
typedef MarkerTreeNode::NodeList MarkerNodeList;

void getTestSentenceA(Sentence<DefaultToken>* sentence)
{
  string index = "A\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "B\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "C\t0\t0\t0\tDT\tB-NP\t*\tREL(\n"
      "D\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "E\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "F\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "G\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "H\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "I\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "J\t0\t0\t0\tDT\tB-NP\t*\tREL)\n"
      "K\t0\t0\t0\tDT\tB-NP\t*\t*\n";
  std::istringstream is(index);
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
  // sentence->printSentence();
}

void getTestSentenceB(Sentence<DefaultToken>* sentence)
{
  string index = "A\t0\t0\t0\tDT\tB-NP\t*\tREL(,REL)\n"
      "B\t0\t0\t0\tDT\tB-NP\t*\tLIT(,REL(\n"
      "C\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "D\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "E\t0\t0\t0\tDT\tB-NP\t*\tREL),LIT)\n"
      "F\t0\t0\t0\tDT\tB-NP\t*\tRELA(,REL(,REL),RELA)\n"
      "G\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "H\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "I\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "J\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "K\t0\t0\t0\tDT\tB-NP\t*\tSEP\n"
      "L\t0\t0\t0\tDT\tB-NP\t*\tSEP\n"
      "M\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "N\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "O\t0\t0\t0\tDT\tB-NP\t*\tSEP\n";
  std::istringstream is(index);
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
  // sentence->printSentence();
}

void getTestSentenceC(Sentence<DefaultToken>* sentence)
{
  string index = "A\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "B\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "C\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "D\t0\t0\t0\tDT\tB-NP\t*\tLIT(\n"
      "E\t0\t0\t0\tDT\tB-NP\t*\tLIT)\n"
      "F\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "G\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "H\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "I\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "J\t0\t0\t0\tDT\tB-NP\t*\tREL(\n"
      "K\t0\t0\t0\tDT\tB-NP\t*\tREL)\n";
  std::istringstream is(index);
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
  // sentence->printSentence();
}


void getTestSentenceD(Sentence<DefaultToken>* sentence)
{
  string index = "A\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "B\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT(,LIT)\n"
      "C\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "X\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "D\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "E\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT),LIT)\n"
      "G\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT(,LIT)\n"
      "H\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "Y\t0\t0\t0\tDT\tB-NP\t*\t*\n"
      "I\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT)\n"
      "J\t0\t0\t0\tDT\tB-NP\t*\tLIT(,LIT),LIT)\n"
      "K\t0\t0\t0\tDT\tB-NP\t*\t*\n";
  std::istringstream is(index);
  SemanticWikipediaReader<DefaultToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
  // sentence->printSentence();
}

TEST(CollateEnumerationTransformer, apply)
{
  MarkerTreeNode root;
  MarkerTerminalNode & terminal1 = root.addChild<MarkerTerminalNode>();
  MarkerTreeNode& rel = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTerminalNode & terminal2 = rel.addChild<MarkerTerminalNode>();
  MarkerTreeNode& rela = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::RELA);
  MarkerTerminalNode & terminal3 = rela.addChild<MarkerTerminalNode>();
  MarkerTreeNode& lit1 = rela.addChild<MarkerTreeNode>(MarkerNodeTypes::LIT);
  MarkerTreeNode& lit2 = rela.addChild<MarkerTreeNode>(MarkerNodeTypes::LIT);
  MarkerTerminalNode & terminal4 = lit1.addChild<MarkerTerminalNode>();
  MarkerTerminalNode & terminal5 = lit2.addChild<MarkerTerminalNode>();
  MarkerTerminalNode & terminal6 = root.addChild<MarkerTerminalNode>();
  CollateEnumerationTransformer er;
  root.applyTransformation(&er);
  // std::cout << root.treeAsString();

  ASSERT_EQ(size_t(2), rela.getChildren().size());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, rela.getChildren()[0].type());
  ASSERT_EQ(MarkerNodeTypes::ENUM, rela.getChildren()[1].type());
  ASSERT_EQ(size_t(2), rela.getChildren()[1].getChildren().size());
  ASSERT_EQ(&terminal3, &rela.getChildren()[0]);
  ASSERT_EQ(&lit1, &rela.getChildren()[1].getChildren()[0]);
  ASSERT_EQ(&lit2, &rela.getChildren()[1].getChildren()[1]);
  ASSERT_EQ(size_t(4), root.getChildren().size());
  ASSERT_EQ(&terminal1, &root.getChildren()[0]);
  ASSERT_EQ(&terminal2, &rel.getChildren()[0]);
  ASSERT_EQ(&terminal4, &lit1.getChildren()[0]);
  ASSERT_EQ(&terminal5, &lit2.getChildren()[0]);
  ASSERT_EQ(&terminal6, &root.getChildren()[3]);
  // std::cout << test.value.size();
}

TEST(DefaultParser, testParseSentenceA)
{
  Sentence<DefaultToken>sentence;
  DefaultPhraseParser<DefaultToken> parser;
  getTestSentenceA(&sentence);
  MarkerNodeList rootList;
  parser.parseSentence(sentence, &rootList);
  ASSERT_EQ(size_t(1), rootList.size());
  MarkerTreeNode const & root = rootList[0];
  // std::cout << root.treeAsString();
  MarkerNodeList const & children = root.getChildren();
  ASSERT_EQ(size_t(3), children.size());
  ASSERT_EQ(MarkerNodeTypes::REL, children[1].type());
  MarkerNodeList const & children2 = children[1].getChildren();
  ASSERT_EQ(size_t(4), children2.size());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children2[0].type());
  ASSERT_EQ(MarkerNodeTypes::LIT, children2[1].type());
  ASSERT_EQ(MarkerNodeTypes::LIT, children2[2].type());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children2[3].type());
  ASSERT_EQ(size_t(1), children2[1].getChildren().size());
  ASSERT_EQ(size_t(1), children2[2].getChildren().size());
  ASSERT_EQ(size_t(0), children2[0].getChildren().size());
  ASSERT_EQ(size_t(0), children2[3].getChildren().size());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children2[1].getChildren()[0].type());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children2[2].getChildren()[0].type());
}

TEST(DefaultParser, testParseSentenceB)
{
  Sentence<DefaultToken>sentence;
  DefaultPhraseParser<DefaultToken> parser;
  getTestSentenceB(&sentence);
  MarkerNodeList rootList;
  parser.parseSentence(sentence, &rootList);
  ASSERT_EQ(size_t(2), rootList.size());
  MarkerTreeNode& root1 = rootList[0];
  MarkerTreeNode& root2 = rootList[1];

  // std::cout << root1.treeAsString("");
  // std::cout << root2.treeAsString("");

  ASSERT_EQ(size_t(5), root1.getChildren().size());
  ASSERT_EQ(size_t(1), root2.getChildren().size());

  MarkerNodeList const & children1 = root1.getChildren();
  MarkerNodeList const & children2 = root2.getChildren();

  ASSERT_EQ(MarkerNodeTypes::REL, children1[0].type());
  ASSERT_EQ(MarkerNodeTypes::LIT, children1[1].type());
  ASSERT_EQ(MarkerNodeTypes::RELA, children1[2].type());
  ASSERT_EQ(MarkerNodeTypes::LIT, children1[3].type());
  ASSERT_EQ(MarkerNodeTypes::LIT, children1[4].type());
  ASSERT_EQ(MarkerNodeTypes::LIT, children2[0].type());

  ASSERT_EQ(size_t(1), children1[0].getChildren().size());
  ASSERT_EQ(size_t(1), children1[1].getChildren().size());
  ASSERT_EQ(size_t(1), children1[2].getChildren().size());
  ASSERT_EQ(size_t(1), children1[3].getChildren().size());
  ASSERT_EQ(size_t(1), children1[4].getChildren().size());
  ASSERT_EQ(size_t(1), children2[0].getChildren().size());

  MarkerNodeList const & children11 = children1[0].getChildren();
  MarkerNodeList const & children12 = children1[1].getChildren();
  MarkerNodeList const & children13 = children1[2].getChildren();
  MarkerNodeList const & children14 = children1[3].getChildren();
  MarkerNodeList const & children15 = children1[4].getChildren();
  MarkerNodeList const & children21 = children2[0].getChildren();
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children11[0].type());
  ASSERT_EQ(size_t(1),
      (static_cast<const MarkerTerminalNode &>(children11[0])).
      getTerminals().size());
  ASSERT_EQ(MarkerNodeTypes::REL, children12[0].type());
  ASSERT_EQ(MarkerNodeTypes::REL, children13[0].type());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children14[0].type());
  ASSERT_EQ(size_t(2),
      (static_cast<const MarkerTerminalNode &>(children14[0])).
      getTerminals().size());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children15[0].type());
  ASSERT_EQ(size_t(2),
      (static_cast<const MarkerTerminalNode &>(children15[0])).
      getTerminals().size());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children21[0].type());
  ASSERT_EQ(size_t(2),
      (static_cast<const MarkerTerminalNode &>(children21[0])).
      getTerminals().size());
  ASSERT_EQ(size_t(0), children11[0].getChildren().size());
  ASSERT_EQ(size_t(1), children12[0].getChildren().size());
  ASSERT_EQ(size_t(1), children13[0].getChildren().size());
  ASSERT_EQ(size_t(0), children14[0].getChildren().size());
  ASSERT_EQ(size_t(0), children15[0].getChildren().size());
  ASSERT_EQ(size_t(0), children21[0].getChildren().size());
  MarkerNodeList const & children121 = children12[0].getChildren();
  MarkerNodeList const & children131 = children13[0].getChildren();
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children121[0].type());
  ASSERT_EQ(
      size_t(4),
      (static_cast<const MarkerTerminalNode &>(children121[0]))
      .getTerminals().size());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, children131[0].type());
  ASSERT_EQ(
      size_t(1),
      (static_cast<const MarkerTerminalNode &>(children131[0]))
      .getTerminals().size());
}

TEST(DefaultParser, testRecombineSentenceC)
{
  Sentence<DefaultToken>sentence;
  DefaultPhraseParser<DefaultToken> parser;
  getTestSentenceC(&sentence);
  MarkerNodeList rootList;
  parser.parseSentence(sentence, &rootList);
  ASSERT_EQ(size_t(1), rootList.size());
  MarkerTreeNode& root = rootList[0];
  CollateEnumerationTransformer er;
  root.applyTransformation(&er);
  // std::cout << root.treeAsString();
  DefaultRecombiner<DefaultToken> r;
  Contexts<DefaultToken> res = root.applyRecombiner(&r);
  // std::cout << res.asString();
  ASSERT_EQ(size_t(6), res.size());
  // TODO(elmar): add result verification
}


TEST(DefaultParser, testRecombineSentenceD)
{
  Sentence<DefaultToken>sentence;
  DefaultPhraseParser<DefaultToken> parser;
  getTestSentenceD(&sentence);
  MarkerNodeList rootList;
  parser.parseSentence(sentence, &rootList);
  ASSERT_EQ(size_t(1), rootList.size());
  MarkerTreeNode& root = rootList[0];
  CollateEnumerationTransformer er;
  root.applyTransformation(&er);
  // std::cout << root.treeAsString();
  DefaultRecombiner<DefaultToken> r;
  Contexts<DefaultToken> res = root.applyRecombiner(&r);
  // std::cout << res.asString();
  ASSERT_EQ(size_t(8), res.size());
  // TODO(elmar): add result verification
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
