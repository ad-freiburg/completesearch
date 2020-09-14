// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "util/GPOSTagger.h"
#include "sentence/Sentence.h"
#include "base/SemanticWikipediaDecomposer.h"

GPOSTagger gpostagger("/home/haussmae/tagger/gposttl/gposttl-0.9.3/data");

// Test the parseText method.
TEST(GPOSTaggerTest, testPOSTag)
{
  DefaultToken word1;
  word1.tokenString = "This";
  DefaultToken word2;
  word2.tokenString = "is";
  DefaultToken word3;
  word3.tokenString = "a";
  DefaultToken word4;
  word4.tokenString = "simple";
  DefaultToken word5;
  word5.tokenString = "test";
  std::vector<DefaultToken> context;
  context.push_back(word1);
  context.push_back(word2);
  context.push_back(word3);
  context.push_back(word4);
  context.push_back(word5);
  gpostagger.postag(context);

  ASSERT_EQ("DT", context[0].posTag);
  ASSERT_EQ("DT", context[2].posTag);
  ASSERT_EQ("NN", context[4].posTag);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
