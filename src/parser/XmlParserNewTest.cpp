// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>, Ina Baumgarten <baumgari>.

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "parser/XmlParserNew.h"


using std::string;

class XmlParserNewTest : public ::testing::TestWithParam<vector<string> > {};

// Example parser subclassed from XmlParserNew.
class ExampleParser : public XmlParserNew
{
  void outputDocAndWords();
};

// Say what the example parser should do for each record (= top-level item) of
// the given XML file.
void ExampleParser::outputDocAndWords()
{
  // Get the contents of a particular tag like this.
  string url = getItem("url");
  string title = getItem("title");
  string text = getItem("text");
  string author = getItem("author");

  // Tokenise a string an output the individual words to the words file like
  // this.
  XmlParserNew::outputWords(text);
  XmlParserNew::outputWords(author, "author", "ct:", "cn:");

  // Output a line to the docs file like this. The doc id is maintained
  // automatically, starting from 1 for the first record and incremented by one
  // for each record.
  XmlParserNew::outputDocument(url, title, text);
}

const vector<string> baseOptions { "program-name-does-not-matter",
                             "--base-name=XmlParserNewTest.TMP",
                             "--encoding=utf8",
                             "--write-docs-file",
                             "--write-words-file-ascii",
                             "--word-part-separator-backend=:"
};

const char basexml[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<example-data>\n"
  "  <record>\n"
  "    <url>URL of record #1</url>\n"
  "    <title>Title of record #1</title>\n"
  "    <text>Text of record #1</text>\n"
  "  </record>\n"
  "  <record>\n"
  "    <url>URL of record #2</url>\n"
  "    <title>Title of record #2</title>\n"
  "    <text>Check umlaute: mähen</text>\n"
  "    <author>Jack Spärrow</author>"
  "  </record>\n"
  "</example-data>\n";

// Base test
const vector<string> v1 {basexml,
                  "1\tu:URL of record #1\tt:Title of record #1\tH:Text of record #1\n"
                  "2\tu:URL of record #2\tt:Title of record #2\tH:Check umlaute: mähen\n",
          "text\t1\t2\t1\n"
            "of\t1\t2\t2\n"
        "record\t1\t2\t3\n"
             "1\t1\t2\t4\n"
         "check\t2\t2\t1\n"
       "umlaute\t2\t2\t2\n"
         "mähen\t2\t2\t3\n"
        "cn:jack::_spärrow:author:Jack_Spärrow\t2\t2\t4\n"
         "cn:spärrow:jack::author:Jack_Spärrow\t2\t2\t4\n"
          "ct:author:jack_spärrow:Jack_Spärrow\t2\t2\t4\n"
};

// Test normalization
const vector<string> v2 {basexml,
                  "1\tu:URL of record #1\tt:Title of record #1\tH:Text of record #1\n"
                  "2\tu:URL of record #2\tt:Title of record #2\tH:Check umlaute: mähen\n",
          "text\t1\t2\t1\n"
            "of\t1\t2\t2\n"
        "record\t1\t2\t3\n"
             "1\t1\t2\t4\n"
         "check\t2\t2\t1\n"
       "umlaute\t2\t2\t2\n"
   "mahen:mähen\t2\t2\t3\n"
         "mähen\t2\t2\t3\n"
        "cn:jack::_spärrow:author:Jack_Spärrow\t2\t2\t4\n"
         "cn:spärrow:jack::author:Jack_Spärrow\t2\t2\t4\n"
          "ct:author:jack_spärrow:Jack_Spärrow\t2\t2\t4\n",
                  "--normalize",
};


// <input xml>, <expected docOutput>, <expected wordsOutput>, <additional
// optionis>
vector<string> testValues[] = {v1, v2};

INSTANTIATE_TEST_CASE_P(BASE_SET,
                        XmlParserNewTest,
                        ::testing::ValuesIn(testValues));

TEST_P(XmlParserNewTest, exampleParser)
{
  vector<string> params = GetParam();
  string xml = params[0];
  string expectedDocOutput = params[1];
  string expectedWordsOutput = params[2];

  // Write a simple XML file.
  string baseName = "XmlParserNewTest.TMP";
  FILE* xml_file = fopen((baseName + ".xml").c_str(), "w");
  EXPECT_TRUE(xml_file != NULL);
  fprintf(xml_file, "%s", xml.c_str());
  fclose(xml_file);

  // Call parser.
  int xp_argc = baseOptions.size();
  if (params.size() > 3) xp_argc += params.size() - 3;
  const char* xp_argv[xp_argc];
  for (unsigned int i = 0; i < baseOptions.size(); i++)
    xp_argv[i] = baseOptions[i].c_str();
  for (unsigned int i = 3; i < params.size(); i++)
    xp_argv[baseOptions.size() + i - 3] = params[i].c_str();

  ExampleParser xp;
  xp.parseCommandLineOptions(xp_argc, const_cast<char**>(xp_argv));
  xp.parse();

  // Variables for checking file contents.
  const int BUFFER_SIZE = 1023;
  char buffer[BUFFER_SIZE + 1];
  int numBytesRead;

  // Check the docs file.
  FILE* docs_file = fopen((baseName + ".docs-unsorted").c_str(), "r");
  EXPECT_TRUE(docs_file != NULL);
  numBytesRead = fread(buffer, 1, BUFFER_SIZE, docs_file);
  EXPECT_LE(0, numBytesRead);
  EXPECT_GE(BUFFER_SIZE, numBytesRead);
  buffer[numBytesRead] = 0;
  EXPECT_EQ(expectedDocOutput, buffer);
  EXPECT_EQ(expectedDocOutput.size(), (unsigned) numBytesRead);
  EXPECT_TRUE(feof(docs_file));
  fclose(docs_file);

  // Check the words file.
  FILE* words_file = fopen((baseName + ".words-unsorted.ascii").c_str(), "r");
  EXPECT_TRUE(words_file != NULL);
  numBytesRead = fread(buffer, 1, BUFFER_SIZE, words_file);
  EXPECT_LE(0, numBytesRead);
  EXPECT_GE(BUFFER_SIZE, numBytesRead);
  buffer[numBytesRead] = 0;
  // Erase global information (right now that's just ":info:date"), since it's
  // hard to test the time changes (but at least check that it's present).
  // This is done by finding the first occurence of ":info:". Global
  // information should start with ":info:".
  string content = buffer;
  EXPECT_NE(content.find(":info:date:"), content.npos);
  size_t infostart = content.find(":info:");
  content = content.substr(0, infostart);
  EXPECT_EQ(expectedWordsOutput, content);
  EXPECT_EQ(expectedWordsOutput.size(), content.size());
  EXPECT_TRUE(feof(words_file));
  fclose(words_file);

  // Clean up.
  remove((baseName + ".xml").c_str());
  remove((baseName + ".docs-unsorted").c_str());
  remove((baseName + ".words-unsorted.ascii").c_str());
}

// TODO(bast): Add unit test for at least the central function from
// XmlParserNew.

// Run all tests. TODO(bast): Remove and link all tests against -lgtest_main.
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
