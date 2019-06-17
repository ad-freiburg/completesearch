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
typedef DeepMarkerTerminalNode TNode;

void getErrorSentence(Sentence<DeepToken>* sentence)
{
  string index =
      ":title:Anarchism [294]\t295\t2\t0\t*\t*\t*\t*\n"
          ":url:http://en.wikipedia.org/wiki/Anarchism\t295\t2\t0\t*\t*\t*\t*\n"
          ":wikiDoc:Anarchism\t295\t0\t0\t*\t*\t*\t*\n"
          "Post-anarchism\t295\t2\t1\tNN\t*\t(TOP,(S,(NP,)\t*\n"
          "__eofentity\t295\t0\t1\t*\t*\t*\t*\n"
          "is\t295\t2\t2\tVBZ\t*\t(VP\t*\n"
          "a\t295\t2\t3\tDT\t*\t(NP,(NP\t*\n"
          "term\t295\t2\t4\tNN\t*\t)\t*\n"
          "coined\t295\t2\t5\tVBN\t*\t(VP\t*\n"
          "by\t295\t2\t6\tIN\t*\t(PP\t*\n"
          ":e:entity:physicalentity:object:whole:livingthing:organism:person:saulnewman:Saul_Newman\t295\t2\t7\t*\t*\t*\t*\n" //NOLINT
          ":ee:entity:saulnewman:Saul_Newman\t295\t2\t7\t*\t*\t*\t*\n"
          "Saul\t295\t2\t7\tNNP\t*\t(NP,(NP\t*\n"
          "Newman\t295\t2\t7\tNNP\t*\t)\t*\n"
          "__eofentity\t295\t0\t7\t*\t*\t*\t*\n"
          ",\t295\t2\t8\t,\t*\t*\t*\n"
          "which\t295\t2\t9\tWDT\t*\t(SBAR,(WHNP,)\t*\n"
          "first\t295\t2\t10\tRB\t*\t(S,(ADVP,)\t*\n"
          "received\t295\t2\t11\tVBD\t*\t(VP\t*\n"
          "popular\t295\t2\t12\tJJ\t*\t(NP\t*\n"
          "attention\t295\t2\t13\tNN\t*\t)\t*\n"
          "in\t295\t2\t14\tIN\t*\t(PP\t*\n"
          ":e:entity:physicalentity:object:whole:livingthing:organism:person:saulnewman:Saul_Newman\t295\t2\t15\t*\t*\t*\t*\n"//NOLINT
          ":ee:entity:saulnewman:Saul_Newman\t295\t2\t15\t*\t*\t*\t*\n"
          "his\t295\t2\t15\tPRP$\t*\t(NP,(NP\t*\n"
          "__eofentity\t295\t0\t15\t*\t*\t*\t*\n"
          "book\t295\t2\t16\tNN\t*\t)\t*\n"
          ":e:entity:physicalentity:object:whole:artifact:creation:product:work:publication:book:frombakunintolacan:From_Bakunin_to_Lacan\t295\t2\t17\t*\t*\t*\t*\n"//NOLINT
          ":ee:entity:frombakunintolacan:From_Bakunin_to_Lacan\t295\t2\t17\t*\t*\t*\t*\n" // NOLINT
          "From\t295\t2\t17\tIN\t*\t(PP\t*\n"
          "Bakunin\t295\t2\t17\tNNP\t*\t(NP,),),),)\t*\n"
          "to\t295\t2\t17\tTO\t*\t(S,(VP\t*\n"
          "Lacan\t295\t2\t17\tVB\t*\t(VP\t*\n"
          "__eofentity\t295\t0\t17\t*\t*\t*\t*\n"
          "to\t295\t2\t18\tTO\t*\t(S,(VP\t*\n"
          "refer\t295\t2\t19\tVB\t*\t(VP\t*\n"
          "to\t295\t2\t20\tTO\t*\t(PP\t*\n"
          "a\t295\t2\t21\tDT\t*\t(NP\t*\n"
          "theoretical\t295\t2\t22\tJJ\t*\t*\t*\n"
          "move\t295\t2\t23\tNN\t*\t),)\t*\n"
          "towards\t295\t2\t24\tIN\t*\t(PP\t*\n"
          "a\t295\t2\t25\tDT\t*\t(NP,(NP\t*\n"
          "synthesis\t295\t2\t26\tNN\t*\t)\t*\n"
          "of\t295\t2\t27\tIN\t*\t(PP\t*\n"
          ":e:entity:abstraction:psychologicalfeature:cognition:attitude:orientation:politicalorientation:classicalliberalism:Classical_liberalism\t295\t2\t28\t*\t*\t*\t*\n"//NOLINT
          ":ee:entity:classicalliberalism:Classical_liberalism\t295\t2\t28\t*\t*\t*\t*\n" // NOLINT
          "classical\t295\t2\t28\tJJ\t*\t(NP,(NP\t*\n"
          "__eofentity\t295\t0\t28\t*\t*\t*\t*\n"
          ":e:entity:abstraction:group:socialgroup:organization:institution:educationalinstitution:school:anarchistcommunism:Anarchist_communism\t295\t2\t29\t*\t*\t*\t*\n"//NOLINT
          ":e:entity:abstraction:psychologicalfeature:cognition:attitude:orientation:politicalorientation:anarchistcommunism:Anarchist_communism\t295\t2\t29\t*\t*\t*\t*\n"//NOLINT
          ":ee:entity:anarchistcommunism:Anarchist_communism\t295\t2\t29\t*\t*\t*\t*\n" // NOLINT
          "anarchist\t295\t2\t29\tNN\t*\t*\t*\n"
          "__eofentity\t295\t0\t29\t*\t*\t*\t*\n"
          ":e:entity:abstraction:group:socialgroup:organization:institution:educationalinstitution:school:mutualismeconomictheory:Mutualism_(economic_theory)\t295\t2\t30\t*\t*\t*\t*\n"//NOLINT
          ":ee:entity:mutualismeconomictheory:Mutualism_(economic_theory)\t295\t2\t30\t*\t*\t*\t*\n"//NOLINT
          "theory\t295\t2\t30\tNN\t*\t)\t*\n"
          "__eofentity\t295\t0\t30\t*\t*\t*\t*\n"
          "and\t295\t2\t31\tCC\t*\t*\t*\n"
          ":e:entity:abstraction:psychologicalfeature:event:act:action:change:motion:poststructuralism:Post-structuralism\t295\t2\t32\t*\t*\t*\t*\n"//NOLINT
          ":e:entity:abstraction:group:socialgroup:organization:institution:educationalinstitution:school:poststructuralism:Post-structuralism\t295\t2\t32\t*\t*\t*\t*\n"//NOLINT
          ":ee:entity:poststructuralism:Post-structuralism\t295\t2\t32\t*\t*\t*\t*\n" // NOLINT
          "poststructuralist\t295\t2\t32\tNN\t*\t(NP\t*\n"
          "__eofentity\t295\t0\t32\t*\t*\t*\t*\n"
          "thought\t295\t2\t33\tNN\t*\t),),),),),),),),),),),),),),),),),),)\t*\n" // NOLINT
          ".\t295\t2\t34\t.\t*\t),)\t*\n";
  std::istringstream is(index);
  SemanticWikipediaReader<DeepToken> swr;
  swr.setInputStream(&is);
  swr.parseNextSentence(sentence);
  // sentence->printSentence();
}

// Test the parseSentenceFromString method.
TEST(DeepMarkerSentenceParser, testParseSentenceFromString)
{
  DeepParserSentenceParser parser;
  Sentence<DeepToken> sentence;
  DeepMarkerSentenceParser markParser;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootList;
  string sentenceStr =
      "(ENUM (C A (ENUM (C B C D) (C E F G H) (C H I J K))))";
  bool parseResult = markParser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  string correctTreeStr =
      "(ROOT (ENUM (C A (ENUM (C B C D )(C E F G H )(C H I J K )))))";
  TreeNode<DeepMarkerNodeTypes> const & enumRoot =
      rootList[0].getChildren()[0];
  ASSERT_EQ(DeepMarkerNodeTypes::ENUM, enumRoot.type());
  ASSERT_EQ(DeepMarkerNodeTypes::C, enumRoot.getChildren()[0].type());
  ASSERT_EQ(correctTreeStr, rootList[0].treeAsFlatString());
}

// Test the parseSentence method.
TEST(DeepMarkerSentenceParser, testParseSentence)
{
  DeepParserSentenceParser parser;
  Sentence<DeepToken> sentence;
  getErrorSentence(&sentence);
  DeepContextMarker marker;
  marker.markSentence(&sentence);
  DeepMarkerSentenceParser markParser;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootList;
  bool parseResult = markParser.parseSentence(sentence, &rootList);
  ASSERT_TRUE(parseResult);

  // This fails because the ordering of marks is wrong.
  Sentence<DeepToken> sentenceB;
  DeepToken * a = new DeepToken();
  a->marks.push_back(DeepTokenMarks::C_HAT);
  a->marks.push_back(DeepTokenMarks::C_HAT);
  a->marks.push_back(DeepTokenMarks::CLOSE);
  a->marks.push_back(DeepTokenMarks::C_HAT);
  a->tokenString = "Test";
  DeepToken * b = new DeepToken();
  b->marks.push_back(DeepTokenMarks::CLOSE);
  b->marks.push_back(DeepTokenMarks::CLOSE);
  b->tokenString = "fails";
  sentenceB.storeToken(a);
  sentenceB.appendWord(a);
  sentenceB.storeToken(b);
  sentenceB.appendWord(b);
  parseResult = markParser.parseSentence(sentenceB, &rootList);
  ASSERT_FALSE(parseResult);
}

TEST(DeepRecombiner, testApply)
{
  // Test a simple sentence.
  Sentence<DeepToken> sentence;
  DeepMarkerSentenceParser markParser;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootList;
  string sentenceStr =
      "(ENUM (C A (ENUM (C B C D) (C E F G H) (C H I J K))))";
  bool parseResult = markParser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  DeepRecombiner<DeepToken> recombiner;
  Contexts<DeepToken> res = recombiner.apply(&rootList[0]);
  ASSERT_EQ(size_t(3), res.size());
  ASSERT_EQ("A B C D", res[0].asString());
  ASSERT_EQ("A E F G H", res[1].asString());
  ASSERT_EQ("A H I J K", res[2].asString());

  // Test two ENUMs in sequence.
  Sentence<DeepToken> sentenceB;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListB;
  string sentenceStrB =
      "(ENUM (C (C A) (ENUM (C B C D) (C E F G H) (C H I J K)) (C X )"
                  "(ENUM (C L M N) (C O P Q) (C R S T))))";
  parseResult = markParser.parseSentenceFromString<TNode>(sentenceStrB,
      &sentenceB, &rootListB, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  Contexts<DeepToken> resB = recombiner.apply(&rootListB[0]);
  ASSERT_EQ(size_t(9), resB.size());
  ASSERT_EQ("A B C D X L M N", resB[0].asString());
  ASSERT_EQ("A B C D X O P Q", resB[1].asString());
  ASSERT_EQ("A B C D X R S T", resB[2].asString());
  ASSERT_EQ("A E F G H X L M N", resB[3].asString());
  ASSERT_EQ("A E F G H X O P Q", resB[4].asString());
  ASSERT_EQ("A E F G H X R S T", resB[5].asString());
  ASSERT_EQ("A H I J K X L M N", resB[6].asString());
  ASSERT_EQ("A H I J K X O P Q", resB[7].asString());
  ASSERT_EQ("A H I J K X R S T", resB[8].asString());

  // Test nested ENUMs.
  Sentence<DeepToken> sentenceC;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListC;
  string sentenceStrC =
      "(ENUM (C (C A) (ENUM "
              "(C B C D) "
              "(C E (ENUM (C F) (C G) (C H))) "
              "(C I J K)) (C X )))";
  parseResult = markParser.parseSentenceFromString<TNode>(sentenceStrC,
      &sentenceC, &rootListC, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  Contexts<DeepToken> resC = recombiner.apply(&rootListC[0]);
  ASSERT_EQ(size_t(5), resC.size());
  ASSERT_EQ("A B C D X", resC[0].asString());
  ASSERT_EQ("A E F X", resC[1].asString());
  ASSERT_EQ("A E G X", resC[2].asString());
  ASSERT_EQ("A E H X", resC[3].asString());
  ASSERT_EQ("A I J K X", resC[4].asString());

  // Test a relative clause sentence.
  Sentence<DeepToken> sentenceD;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListD;
  string sentenceStrD =
  "(ENUM(C(C(C(C(C During )(CH(C the  flight )) , )(CH(C she ))"
      "(C(C reported (C to )(CH(C Houston  Mission  Control )))"
          "(C*(C(C that )(CH(C she ))(C(C had (C spotted )"
  "(CH(C an  unidentified  flying  object )))))) . )))))";
  parseResult = markParser.parseSentenceFromString<TNode>(sentenceStrD,
      &sentenceD, &rootListD, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  SimplifyDeepMarkerTree simplifier;
  simplifier.apply(&rootListD[0]);
  Contexts<DeepToken> resD = recombiner.apply(&rootListD[0]);
  ASSERT_EQ(size_t(2), resD.size());
  ASSERT_EQ("During the flight , she reported to Houston Mission Control .",
      resD[0].asString());
  ASSERT_EQ("Houston Mission Control that she had spotted an "
      "unidentified flying object", resD[1].asString());

  // Test a sentence with a C_HAT.
  Sentence<DeepToken> sentenceE;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListE;
  string sentenceStrE = "(ENUM (C (C^ According to someone)) , "
      "(C some stuff happened))";
  parseResult = markParser.parseSentenceFromString<TNode>(sentenceStrE,
      &sentenceE, &rootListE, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  simplifier.apply(&rootListE[0]);
  DeepRecombiner<DeepToken> recombinerE;
  Contexts<DeepToken> resE = recombinerE.apply(&rootListE[0]);
  ASSERT_EQ(size_t(2), resE.size());
  ASSERT_EQ("some stuff happened", resE[0].asString());
  ASSERT_EQ("According to someone", resE[1].asString());
}

TEST(SimplifyDeepMarkerTree, testApply)
{
  // Apply to a simple nested structure.
  Sentence<DeepToken> sentence;
  DeepMarkerSentenceParser markParser;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootList;
  string sentenceStr =
      "(ENUM (C (C (C (C (C A B C))))))";
  bool parseResult = markParser.parseSentenceFromString<TNode>(sentenceStr,
      &sentence, &rootList, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  SimplifyDeepMarkerTree simplifier;
  simplifier.apply(&rootList[0]);
  string correctTreeStr = "(ROOT (ENUM (C A B C )))";
  ASSERT_EQ(correctTreeStr, rootList[0].treeAsFlatString());

  // Apply to a simple parallel structure.
  Sentence<DeepToken> sentenceB;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListB;
  string sentenceStrB =
      "(ENUM (C (C (C A) (C B) (C C))))";
  parseResult = markParser.parseSentenceFromString<TNode>(sentenceStrB,
      &sentenceB, &rootListB, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  simplifier.apply(&rootListB[0]);
  string correctTreeStrB = "(ROOT (ENUM (C A B C )))";
  ASSERT_EQ(correctTreeStr, rootListB[0].treeAsFlatString());


  // Apply to a simple parallel structure, not below same node type.
  Sentence<DeepToken> sentenceC;
  TreeNode<DeepMarkerNodeTypes>::NodeList rootListC;
  string sentenceStrC =
      "(ENUM (C (C A) (C B) (C C) (C D) (C E) (C F) (C G) (C H)))";
  parseResult = markParser.parseSentenceFromString<TNode>(sentenceStrC,
      &sentenceC, &rootListC, DeepMarkerNodeTypes::stringToTypeMap);
  ASSERT_TRUE(parseResult);
  simplifier.apply(&rootListC[0]);
  string correctTreeStrC = "(ROOT (ENUM (C A B C D E "
      "F G H )))";
  ASSERT_EQ(correctTreeStrC, rootListC[0].treeAsFlatString());
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
