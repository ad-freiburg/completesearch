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

using ad_semsearch::OntologyManager;
using ad_semsearch::OutputWriter;
using ad_semsearch::OntologyRelation;
using ad_semsearch::CORRECTIONS_DONE_FILENAME;

const char* ONTOLOGY_TEST_FOLDER = "../ontology-tests";
const char* TESTREL_NOT_TRANSITIVE = "facts.testNotTransitive.t1.t2.txt";
const char* TESTREL_TRANSITIVE = "facts.testTransitiveDelete.t1.t2.txt";

//! Helper method used by some tests below.
int numberOfFilesInDir(const string& myDir)
{
  // 2. Get available relations.
  int return_code;
  DIR* dir;
  struct dirent entry;
  struct dirent *result;
  if ((dir = opendir(myDir.c_str())) == NULL)
  {
    std::cout << "Error(" << errno << ") opening " << myDir << std::endl;
    exit(1);
  }
  int retVal = 0;
  for (return_code = readdir_r(dir, &entry, &result); result != NULL
      && return_code == 0; return_code = readdir_r(dir, &entry, &result))
  {
    retVal++;
  }
  closedir(dir);
  return retVal;
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, getRelations)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation> relations = om.getRelations();
  ASSERT_EQ(static_cast<size_t>(4), relations.size());
  int nofTransitiveRelations = 0;
  for (size_t i = 0; i < relations.size(); ++i)
  {
    if (relations[i].isTransitive())
    {
      nofTransitiveRelations++;
    }
  }
  ASSERT_EQ(2, nofTransitiveRelations);
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, revokeCorrections)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation> relations = om.getRelations();
  relations[0].addTuple("a", "b");
  int oldFilesNum = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  om.revokeCorrections();
  int newFilesNum = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  ASSERT_GT(oldFilesNum, newFilesNum);
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, applyCorrectionsNEW)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  // Check if corrected files are written
  int oldFilesNum = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  om.applyCorrections();
  int newFilesNum = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  ASSERT_LT(oldFilesNum, newFilesNum);
  // Check for the content
  OntologyRelation rel("facts.isA.instance.class.txt", false,
      ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation::SourceDestPair> content = rel.getContentSnapshot();
  bool foundWhatHasBeenInserted = false;
  for (size_t i = 0; i < content.size(); ++i)
  {
    foundWhatHasBeenInserted = foundWhatHasBeenInserted ||
    ((content[i].first == "insertS") && (content[i].second == "insertD"));
  }
  ASSERT_TRUE(foundWhatHasBeenInserted);
  // cleanup
  om.revokeCorrections();
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, applyCorrectionsDELETE)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);

  om.applyCorrections();
  // Check for the content
  OntologyRelation rel("facts.isA.instance.class.txt", false,
      ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation::SourceDestPair> content = rel.getContentSnapshot();
  bool foundWhatHasBeenDeleted = false;
  for (size_t i = 0; i < content.size(); ++i)
  {
    foundWhatHasBeenDeleted = foundWhatHasBeenDeleted || ((content[i].first
        == "I2") && (content[i].second == "C1"));
  }
  ASSERT_FALSE(foundWhatHasBeenDeleted);
  // cleanup
  om.revokeCorrections();
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, applyCorrectionsREPLACE)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);

  om.applyCorrections();
  // Check for the content
  OntologyRelation rel("facts.isA.instance.class.txt", false,
      ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation::SourceDestPair> content = rel.getContentSnapshot();
  bool foundOld = false;
  bool foundNew = false;
  for (size_t i = 0; i < content.size(); ++i)
  {
    foundOld = foundOld || ((content[i].first == "I4") && (content[i].second
        == "C3"));
    foundNew = foundNew || ((content[i].first == "I5") && (content[i].second
        == "C3"));
  }
  ASSERT_FALSE(foundOld);
  ASSERT_TRUE(foundNew);
  // cleanup
  om.revokeCorrections();
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, applyCorrectionsREPLACE_GLOBALLY)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);

  om.applyCorrections();
  // Check for the content
  OntologyRelation rel("facts.isA.instance.class.txt", false,
      ONTOLOGY_TEST_FOLDER);
  vector<OntologyRelation::SourceDestPair> content = rel.getContentSnapshot();
  // In this relation I. we want
  // I.a) x4  x2 --> x4  x3
  bool foundOld = false;
  bool foundNew = false;
  for (size_t i = 0; i < content.size(); ++i)
  {
    foundOld = foundOld || ((content[i].first == "x4") && (content[i].second
        == "x2"));
    foundNew = foundNew || ((content[i].first == "x4") && (content[i].second
        == "x3"));
  }
  ASSERT_FALSE(foundOld);
  ASSERT_TRUE(foundNew);
  // I.b) I5  x2 --> I5  x3
  foundOld = false;
  foundNew = false;
  for (size_t i = 0; i < content.size(); ++i)
  {
    foundOld = foundOld || ((content[i].first == "I5") && (content[i].second
            == "x2"));
    foundNew = foundNew || ((content[i].first == "I5") && (content[i].second
            == "x3"));
  }
  ASSERT_FALSE(foundOld);
  ASSERT_TRUE(foundNew);
  OntologyRelation rel2("facts.isA.class.class.txt", false,
      ONTOLOGY_TEST_FOLDER);
  content = rel2.getContentSnapshot();
  // In this relation II. we want
  // II. x2  x3 --> x3  x3
  foundOld = false;
  foundNew = false;
  for (size_t i = 0; i < content.size(); ++i)
  {
    foundOld = foundOld || ((content[i].first == "x2") && (content[i].second
            == "x3"));
    foundNew = foundNew || ((content[i].first == "x3") && (content[i].second
            == "x3"));
  }
  // cleanup
  om.revokeCorrections();
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, onlyProcessPendingCorrections)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  om.applyCorrections();
  int nofFiles = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  string rmstring = string(ONTOLOGY_TEST_FOLDER) + "/"
      + "__tmp.isA.instance.class.txt";
  std::remove(rmstring.c_str());
  int nofFiles2 = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  ASSERT_GT(nofFiles, nofFiles2);
  OntologyManager om2(ONTOLOGY_TEST_FOLDER);
  om2.applyCorrections();
  int nofFiles3 = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  ASSERT_EQ(nofFiles2, nofFiles3);
  om2.revokeCorrections();
}
// _____________________________________________________________________________
TEST(OntologyManagerTest, recoverFromInvalidCorrectionsDone)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  om.applyCorrections();
  int nofFiles = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  string rmstring = string(ONTOLOGY_TEST_FOLDER) + "/"
      + "__tmp.isA.instance.class.txt";
  std::remove(rmstring.c_str());
  int nofFiles2 = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  ASSERT_GT(nofFiles, nofFiles2);

  // Add another line to corrections done
  string correctionsDoneFilePath = string(ONTOLOGY_TEST_FOLDER) + "/"
  + CORRECTIONS_DONE_FILENAME;
  std::ofstream correctionsDone(correctionsDoneFilePath.c_str(), std::ios::app);
  correctionsDone << "extra line" << std::endl;
  correctionsDone.close();
  OntologyManager om2(ONTOLOGY_TEST_FOLDER);
  om2.applyCorrections();
  int nofFiles3 = numberOfFilesInDir(ONTOLOGY_TEST_FOLDER);
  ASSERT_EQ(nofFiles, nofFiles3);
  om2.revokeCorrections();
}
namespace ad_semsearch
{
// _____________________________________________________________________________
TEST(OntologyManagerTest, testRemoveFromTransitiveRelation)
{
  OntologyManager om(ONTOLOGY_TEST_FOLDER);
  OntologyRelation relation(TESTREL_TRANSITIVE, true, ONTOLOGY_TEST_FOLDER);
  om.removeFromTransitiveRelation(relation, "stupid_class");
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  ASSERT_EQ(static_cast<size_t>(4), content.size());
  om.revokeCorrections();
}
}
// _____________________________________________________________________________
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
