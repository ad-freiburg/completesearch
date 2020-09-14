// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <sstream>
#include "base/ContextDecomposerBase.h"
#include "base/SemanticWikipediaDecomposer.h"
#include "util/ContextDecomposerUtil.h"
#include "decomposer-ml/FeatureExtractor.h"

namespace ad_decompose
{
// Test the parseText method.
TEST(SemanticWikipediaSentenceTest, parseFeatureConfig)
{
  FeatureExtractorConfig configa =
      ContextDecomposerUtil::getDefaultFexConfig();
  // TODO(elmar): do not rely on a file lying in the filesystem.
  FeatureExtractorConfig configb =
      ContextDecomposerUtil::parseFeatureConfig(
          "config/default_feature_config.txt");
  ASSERT_EQ(configa.brTypes, configb.brTypes);
  ASSERT_EQ(configa.bwLeft, configb.bwLeft);
  ASSERT_EQ(configa.bwRight, configb.bwRight);
  ASSERT_EQ(configa.countCTypes, configb.countCTypes);
  ASSERT_EQ(configa.countPTypes, configb.countPTypes);
  ASSERT_EQ(configa.countWTypes, configb.countWTypes);
  ASSERT_EQ(configa.cwLeft, configb.cwLeft);
  ASSERT_EQ(configa.cwRight, configb.cwRight);
  ASSERT_EQ(configa.dynbrLeft, configb.dynbrLeft);
  ASSERT_EQ(configa.dynbrRight, configb.dynbrRight);
  ASSERT_EQ(configa.dynbrType, configb.dynbrType);
  ASSERT_EQ(configa.pre_bw, configb.pre_bw);
  ASSERT_EQ(configa.pre_count, configb.pre_count);
  ASSERT_EQ(configa.pre_cw, configb.pre_cw);
  ASSERT_EQ(configa.pre_pw, configb.pre_pw);
  ASSERT_EQ(configa.pre_ww, configb.pre_ww);
  ASSERT_EQ(configa.pwLeft, configb.pwLeft);
  ASSERT_EQ(configa.pwRight, configb.pwRight);
  ASSERT_EQ(configa.wwLeft, configb.wwLeft);
  ASSERT_EQ(configa.wwRight, configb.wwRight);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
