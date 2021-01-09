// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include "./UserDefinedIndexWords.h"
#include "./SimpleTextParser.h"

using std::cout;
using std::endl;

// Test fixture.
class UserDefinedIndexWordsTest : public testing::Test
{
 protected:
  UserDefinedIndexWordsTest() : properNamesFileName("properNames.lst") {}

  virtual void SetUp()
  {}

  virtual void TearDown()
  {
    deleteFile(properNamesFileName);
    // properNames.clear();
  }

  // Create file with given name and given content.
  void createFile(const string& fileName, const string& content)
  {
    FILE *file = NULL;
    file = fopen(fileName.c_str(), "w");
    assert(file);
    fputs(content.c_str(), file);
    fclose(file);
  }

  // Delete file with given name.
  int deleteFile(const string& fileName)
  {
    string commandLine = "rm -f " + fileName;
    return system(commandLine.c_str());
  }

  // This function is to call for any test case.
  void init(const string& input)
  {
    createFile(properNamesFileName, input.c_str());
    properNames.init(properNamesFileName);
  }

  // Variables the test uses.
  UserDefinedIndexWords properNames;
  const string properNamesFileName;
};


TEST_F(UserDefinedIndexWordsTest, isProperBase)
{
  vector<UserDefinedIndexWords::PosVec> positions;
  UserDefinedIndexWords::PosVec position;

  init("c++\n.NeT!\n!@blub///\n...blub...\n");

  ASSERT_TRUE(properNames.isProperBase("c", positions));
  ASSERT_EQ((unsigned) 2, positions.size());
  position = positions[0];
  ASSERT_EQ(0, position.first);
  ASSERT_EQ(2, position.second);

  positions.resize(0);
  ASSERT_TRUE(properNames.isProperBase("net", positions));
  ASSERT_EQ((unsigned) 1, positions.size());
  position = positions[0];
  ASSERT_EQ(position.first, -1);
  ASSERT_EQ(position.second, 1);

  positions.resize(0);
  ASSERT_TRUE(properNames.isProperBase("blub", positions));
  ASSERT_EQ((unsigned) 4, positions.size());
  position = positions[0];
  ASSERT_EQ(position.first, -2);
  ASSERT_EQ(position.second, 3);
  position = positions[2];
  ASSERT_EQ(position.first, -3);
  ASSERT_EQ(position.second, 3);
}

TEST_F(UserDefinedIndexWordsTest, isProperName)
{
  init("c++\n.NeT!\n!@blub///\n...blub...\n");
  ASSERT_TRUE(properNames.isProperName("c++"));
  ASSERT_TRUE(properNames.isProperName(".NeT!"));
  ASSERT_TRUE(properNames.isProperName("!@blub///"));
  ASSERT_TRUE(properNames.isProperName("...blub..."));
}

TEST_F(UserDefinedIndexWordsTest, extendToUserDefinedWord)
{
  SimpleTextParser simpleTextParser;

  size_t word_start = 0;
  size_t word_end = 0;
  string text = "Like c++ no #c#!";
  bool value = false;
  int cnt = 0;
  string word = "";

  // Write down here some set of proper words.
  init("c#\nc++\n");

  while (word_end < text.size())
  {
    simpleTextParser.parseText(text, &word_start, &word_end);
    word = text.substr(word_start, word_end - word_start);
    if (word_start == word_end) break;
    value = properNames.extendToUserDefinedWord(text, &word_start, &word_end);
    if (value)
    {
      // cout << "Word extended!" << endl;
      word = text.substr(word_start, word_end - word_start);
    }
    // cout << "Test:120" << endl;
    switch (cnt)
    {
      case 0:  ASSERT_EQ(word, "Like"); break;
      case 1:  ASSERT_EQ(word, "c++"); break;
      case 2:  ASSERT_EQ(word, "no"); break;
      case 3:  ASSERT_EQ(word, "c#"); break;
      default: break;
    }
    ++cnt;
  }
}

