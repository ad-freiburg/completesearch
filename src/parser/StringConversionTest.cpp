// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010


#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include "./StringConversion.h"
using std::cout;
using std::endl;

// Testing function StringConversion::utf8_tolower
TEST(utf8ToLower, StringConversion)
{
  StringConversion sc;
  sc.setEncoding(StringConversion::UTF_8);
  string word = "ÖsterreichHausAffeKammBaumÄnderungÜbersicht";
  sc.utf8ToLower(&word);
  string expected_word = "österreichhausaffekammbaumänderungübersicht";
  ASSERT_EQ(expected_word, word);
}

TEST(iso88591ToLower, StringConversion)
{
  StringConversion sc;
  string expected_word = "oesterreichhausaffekammbaumaenderunguebersicht";
  sc.setEncoding(StringConversion::ISO_8859_1);
  string word = "OesterreichHausAffeKammBaumAenderungUebersicht";
  sc.iso88591ToLower(&word);
  ASSERT_EQ(expected_word, word) << "Neues Wort:  " << "word" << endl;
}

