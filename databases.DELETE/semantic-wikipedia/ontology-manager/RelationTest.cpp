// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "./OntologyManager.h"

namespace ad_semsearch
{
const char* ONTOLOGY_TEST_FOLDER = "../ontology-tests";
const char* TESTREL_NOT_TRANSITIVE = "facts.testNotTransitive.t1.t2.txt";

// _____________________________________________________________________________
TEST(RelationTest, getTuples)
{
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
  ASSERT_EQ("r1c1", content[0].first);
  ASSERT_EQ("r2c2", content[1].second);
}
// _____________________________________________________________________________
TEST(RelationTest, addTuple)
{
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  relation.addTuple("add", "me");
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(4), content.size());
  ASSERT_EQ("r1c1", content[0].first);
  ASSERT_EQ("r2c2", content[1].second);
  ASSERT_EQ("add", content[3].first);
  ASSERT_EQ("me", content[3].second);
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  om.revokeCorrections();
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
}
// _____________________________________________________________________________
TEST(RelationTest, deleteTuple)
{
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  relation.deleteTuple("r1c1", "r1c2");
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(2), content.size());
  relation.deleteTuple("r2c1", "r2c2");
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(1), content.size());
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  om.revokeCorrections();
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
}
// _____________________________________________________________________________
TEST(RelationTest, deleteTuplesBySource)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  int nofD = relation.deleteTuplesBySource("r1c1");
  ASSERT_EQ(2, nofD);
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(1), content.size());
  om.revokeCorrections();
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
  nofD = relation.deleteTuplesBySource("r2c1");
  ASSERT_EQ(1, nofD);
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(2), content.size());
  om.revokeCorrections();
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
}
// _____________________________________________________________________________
TEST(RelationTest, deleteTuplesByDest)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  int nofD = relation.deleteTuplesByDest("r1c2");
  ASSERT_EQ(1, nofD);
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(2), content.size());
  om.revokeCorrections();
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
  nofD = relation.deleteTuplesByDest("r2c2");
  ASSERT_EQ(2, nofD);
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(1), content.size());
  om.revokeCorrections();
  content = relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(3), content.size());
}
// _____________________________________________________________________________
TEST(RelationTest, replace)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  int count = relation.replaceEntity("r1c1", "r1c1Repl");
  ASSERT_EQ(2, count);
  om.revokeCorrections();
}
// _____________________________________________________________________________
TEST(RelationTest, introduceAlias)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  OntologyRelation relation(TESTREL_NOT_TRANSITIVE, false,
      ONTOLOGY_TEST_FOLDER);
  int count = relation.introduceAlias("r2c2", "r2c2Alias");
  ASSERT_EQ(2, count);
  om.revokeCorrections();
}
//
// _____________________________________________________________________________
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
}
