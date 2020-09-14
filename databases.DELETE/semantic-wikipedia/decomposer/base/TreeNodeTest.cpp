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

// Test the addChild method.
TEST(TreeNode, testAddChild)
{
  MarkerTreeNode root;
  MarkerTerminalNode & terminal1 = root.addChild<MarkerTerminalNode>();
  ASSERT_EQ(&terminal1, &root.getChildren().back());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, root.getChildren().back().type());
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::TERMINAL));
  ASSERT_EQ(&root, terminal1.getParent());
  ASSERT_EQ(MarkerNodeTypes::ROOT, terminal1.getParent()->type());
  ASSERT_EQ(size_t(1), terminal1.getParent()->getChildren().size());
}

// Test creating a bigger tree structure.
TEST(TreeNode, testAddChildren)
{
  MarkerTreeNode root;
  MarkerTerminalNode & terminal1 = root.addChild<MarkerTerminalNode>();
  MarkerTreeNode& rel = root.addChild<MarkerTreeNode>(MarkerNodeTypes::REL);
  MarkerTerminalNode & terminal2 = rel.addChild<MarkerTerminalNode>();
  MarkerTreeNode& enumN = rel.addChild<MarkerTreeNode>(MarkerNodeTypes::ENUM);
  MarkerTerminalNode & terminal3 = enumN.addChild<MarkerTerminalNode>();
  MarkerTreeNode& lit1 = enumN.addChild<MarkerTreeNode>(MarkerNodeTypes::LIT);
  MarkerTreeNode& lit2 = enumN.addChild<MarkerTreeNode>(MarkerNodeTypes::LIT);
  MarkerTerminalNode & terminal4 = lit1.addChild<MarkerTerminalNode>();
  MarkerTerminalNode & terminal5 = lit2.addChild<MarkerTerminalNode>();
  MarkerTerminalNode & terminal6 = root.addChild<MarkerTerminalNode>();

  ASSERT_EQ(&terminal1, &root.getChildren().front());
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::REL));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::LIT));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::RELA));
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::TERMINAL));
  ASSERT_EQ(&terminal6, &root.getChildren().back());
  ASSERT_EQ(size_t(3), root.getChildren().size());
  ASSERT_EQ(&root, terminal1.getParent());
  ASSERT_EQ(&root, rel.getParent());
  ASSERT_EQ(&rel, enumN.getParent());
  ASSERT_EQ(&enumN, lit1.getParent());
  ASSERT_TRUE(rel.hasChildType(MarkerNodeTypes::ENUM));
  ASSERT_TRUE(enumN.hasChildType(MarkerNodeTypes::LIT));
  ASSERT_EQ(MarkerNodeTypes::LIT, enumN.getChildren().back().type());
  ASSERT_EQ(&terminal2, &rel.getChildren().front());
  ASSERT_EQ(&terminal3, &enumN.getChildren().front());
  ASSERT_EQ(&terminal4, &lit1.getChildren().front());
  ASSERT_EQ(&terminal5, &lit2.getChildren().front());
  ASSERT_EQ(MarkerNodeTypes::TERMINAL, root.getChildren().back().type());
}

TEST(TreeNode, testGetChild)
{
  MarkerTreeNode root;
  MarkerTerminalNode & terminal1 = root.addChild<MarkerTerminalNode>();
  ASSERT_EQ(&terminal1, root.getChild(0));
}

TEST(TreeNode, testSetType)
{
  MarkerTreeNode root;
  MarkerTerminalNode & terminal1 = root.addChild<MarkerTerminalNode>();
  MarkerTreeNode & child = root.addChild<MarkerTreeNode>(MarkerNodeTypes::LIT);
  ASSERT_EQ(&terminal1, root.getChild(0));
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::TERMINAL));
  root.setType(MarkerNodeTypes::RELA);
  ASSERT_EQ(MarkerNodeTypes::RELA, terminal1.getParent()->type());
  ASSERT_EQ(MarkerNodeTypes::RELA, root.type());
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::ENUM));
  ASSERT_TRUE(child.type() == MarkerNodeTypes::LIT);
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::LIT));
  child.setType(MarkerNodeTypes::ENUM);
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::ENUM));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::LIT));
}


TEST(TreeNode, testRemoveChildPtr)
{
  MarkerTreeNode root;
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTerminalNode & terminal2 = root.addChild<MarkerTerminalNode>();
  MarkerTreeNode& terminal3 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::TERMINAL));
  root.removeChild(&terminal2);
  ASSERT_EQ(size_t(2), root.getChildren().size());
  ASSERT_EQ(&terminal1, root.getChild(0));
  ASSERT_EQ(&terminal3, root.getChild(1));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::TERMINAL));
}

TEST(TreeNode, testRemoveChildIdx)
{
  MarkerTreeNode root;
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  root.addChild<MarkerTerminalNode>();
  MarkerTreeNode& terminal3 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::TERMINAL));
  root.removeChild(1);
  ASSERT_EQ(size_t(2), root.getChildren().size());
  ASSERT_EQ(&terminal1, root.getChild(0));
  ASSERT_EQ(&terminal3, root.getChild(1));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::TERMINAL));
}

TEST(TreeNode, testMoveNode)
{
  MarkerTreeNode root;
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  root.addChild<MarkerTerminalNode>();
  MarkerTreeNode& terminal11 = terminal1.addChild<MarkerTreeNode>(
      MarkerNodeTypes::REL);
  MarkerTerminalNode & terminal111 = terminal11.addChild<MarkerTerminalNode>();
  ASSERT_TRUE(terminal11.hasChildType(MarkerNodeTypes::TERMINAL));
  ASSERT_EQ(size_t(2), root.getChildren().size());
  terminal111.moveNode(&root);
  ASSERT_EQ(size_t(3), root.getChildren().size());
  ASSERT_EQ(&terminal111, root.getChild(2));
  ASSERT_FALSE(terminal11.hasChildType(MarkerNodeTypes::TERMINAL));
  ASSERT_EQ(size_t(0), terminal11.getChildren().size());
}

TEST(TreeNode, testGetParent)
{
  MarkerTreeNode root;
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTreeNode& terminal11 = terminal1.addChild<MarkerTreeNode>(
      MarkerNodeTypes::REL);
  ASSERT_EQ(&root, terminal1.getParent());
  ASSERT_EQ(&terminal1, terminal11.getParent());
}

TEST(TreeNode, testHasChildType)
{
  MarkerTreeNode root;
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  terminal1.addChild<MarkerTreeNode>(MarkerNodeTypes::RELA);
  ASSERT_TRUE(root.hasChildType(MarkerNodeTypes::REL));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::LIT));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::RELA));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::ENUM));
  ASSERT_FALSE(root.hasChildType(MarkerNodeTypes::TERMINAL));

  ASSERT_TRUE(terminal1.hasChildType(MarkerNodeTypes::RELA));
  ASSERT_FALSE(terminal1.hasChildType(MarkerNodeTypes::LIT));
  ASSERT_FALSE(terminal1.hasChildType(MarkerNodeTypes::REL));
  ASSERT_FALSE(terminal1.hasChildType(MarkerNodeTypes::ENUM));
  ASSERT_FALSE(terminal1.hasChildType(MarkerNodeTypes::TERMINAL));
}

TEST(TreeNode, testAddChildPriv)
{
  MarkerTreeNode root;
  ASSERT_EQ(size_t(0), root.getChildren().size());
  MarkerTreeNode* terminal1 = new MarkerTreeNode(MarkerNodeTypes::REL, &root);
  root.addChild(terminal1);
  ASSERT_EQ(size_t(1), root.getChildren().size());
  ASSERT_EQ(terminal1, &root.getChildren()[0]);
}

TEST(TreeNode, testAddChildIdxPriv)
{
  MarkerTreeNode root;
  ASSERT_EQ(size_t(0), root.getChildren().size());
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTreeNode& terminal2 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTreeNode& terminal3 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTreeNode* toAdd = new MarkerTreeNode(MarkerNodeTypes::REL, &root);
  root.addChild(toAdd, 1);

  ASSERT_EQ(size_t(4), root.getChildren().size());
  ASSERT_EQ(&terminal1, &root.getChildren()[0]);
  ASSERT_EQ(toAdd, root.getChild(1));
  ASSERT_EQ(&terminal2, &root.getChildren()[2]);
  ASSERT_EQ(&terminal3, &root.getChildren()[3]);
}

TEST(TreeNode, testReleaseChildPriv)
{
  MarkerTreeNode root;
  ASSERT_EQ(size_t(0), root.getChildren().size());
  MarkerTreeNode& terminal1 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTreeNode& terminal2 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  MarkerTreeNode& terminal3 = root.addChild < MarkerTreeNode
      > (MarkerNodeTypes::REL);
  ASSERT_EQ(size_t(3), root.getChildren().size());
  root.releaseChild(&terminal2);
  ASSERT_EQ(size_t(2), root.getChildren().size());
  ASSERT_EQ(&terminal1, &root.getChildren()[0]);
  ASSERT_EQ(&terminal3, &root.getChildren()[1]);
  delete &terminal2;
}

// Test creating a bigger tree structure.
TEST(TreeNode, testCombineNodes)
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
  vector<size_t> collate;
  collate.push_back(1);
  collate.push_back(2);
  rela.combineNodes<MarkerTreeNode>(collate, MarkerNodeTypes::ENUM);
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
}

// Test the mergeNodes operation.
TEST(TreeNode, testMergeNodes)
{
  string sentenceStr= "A (REL B) (REL C "
      "(LIT E (REL H)) (LIT F (REL I)) (LIT G (REL J))) (REL D)";
  Sentence<DefaultToken>sentence;
  DefaultPhraseParser<DefaultToken> parser;
  MarkerNodeList rootList;
  bool parseResult = parser.parseSentenceFromString
      <TerminalNode<MarkerNodeTypes, DefaultToken *> >(
      sentenceStr, &sentence, &rootList, MarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  vector<size_t> merge;
  merge.push_back(1);
  merge.push_back(2);
  merge.push_back(3);
  string correctMergeStr= "(ROOT A (REL B )(REL C "
      "(LIT E (REL H )F (REL I )G (REL J )))(REL D ))";
  rootList[0].getChild(2)->mergeNodes
      <TreeNode<MarkerNodeTypes > >(merge, MarkerNodeTypes::LIT);
  ASSERT_EQ(correctMergeStr, rootList[0].treeAsFlatString());
}

// Test the eraseChild operation.
TEST(TreeNode, testEraseChild)
{
  string sentenceStr= "A (REL B) (REL C (LIT E (REL H))"
      " (LIT F (REL I)) (LIT G (REL J))) (REL D)";
  Sentence<DefaultToken>sentence;
  DefaultPhraseParser<DefaultToken> parser;
  MarkerNodeList rootList;
  bool parseResult = parser.parseSentenceFromString
      <TerminalNode<MarkerNodeTypes, DefaultToken *> >(
      sentenceStr, &sentence, &rootList, MarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  vector<size_t> merge;
  merge.push_back(1);
  merge.push_back(2);
  merge.push_back(3);
  string correctMergeStr= "(ROOT A (REL B )C (LIT E (REL H ))"
      "(LIT F (REL I ))(LIT G (REL J ))(REL D ))";
  rootList[0].eraseChild(2);
  ASSERT_EQ(correctMergeStr, rootList[0].treeAsFlatString());
  ASSERT_EQ(&rootList[0], rootList[0].getChild(2)->getParent());
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

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
