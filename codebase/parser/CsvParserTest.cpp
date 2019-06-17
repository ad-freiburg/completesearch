// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright 2011 Jens Hoffmann <hoffmaje@informatik.uni-freiburg.de>
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include <stdio.h>
#include <gtest/gtest.h>
#include <stdarg.h>
#include <errno.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include "./CsvParser.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;

// Test fixture.
class CsvParserTest : public testing::Test
{
 protected:
  CsvParserTest()
    : _name("testbase"), _extension(".csv"),
      _extensionOutputWords(".words_unsorted"),
      _extensionOutputDocs(".docs") {}

  // virtual ~CsvParserTest() {}

  virtual void SetUp()
  { }

  // Called after each test is run. If no cleanup work to do, you don't need
  // to provide it.
  virtual void TearDown()
  {
    remove("testbase.csv");
    remove("testbase.parse-log");
    remove("testbase.user-defined-words");
    remove("testbase.synonym-groups");
    remove("testbase.docs-unsorted");
    remove("testbase.words-unsorted.ascii");
  }

  // Write headline and contentline to the stream.
  void writeBaseCsv(const string& content)
  {
    std::cout << "Writing " << _name+_extension << std::endl;
    ofstream os;
    os.open((_name + _extension).c_str());
    os << content;
    os.close();
  }

  // Write content to file named filename.
  void write(const char* filename, const char* content)
  {
    FILE* file = fopen(filename, "w");
    fprintf(file, "%s", content);
    fclose(file);
  }

  // Return a files content.
  std::string fileToString(const char* filename, bool eraseGlobal = true)
  {
    std::string retval;
    int fileLength;
    ifstream is;
    char* buffer;

    is.open(filename, std::ios::binary);
    // Determine file size.
    is.seekg(0, std::ios::end);
    fileLength = is.tellg();
    is.seekg(0, std::ios::beg);
    // Read the files content.
    buffer = new char[fileLength + 1];
    is.read(buffer, fileLength);
    is.close();
    buffer[fileLength] = '\0';
    retval = std::string(buffer);
    delete[] buffer;
    if (eraseGlobal)
      retval = eraseGlobalInformation(retval);
    return retval;
  }

  // Erase global information.
  std::string eraseGlobalInformation(const string& words)
  {
    string infoPrefix = "!info!";
    string cleanedWords = "";
    string line;
    size_t pos = 0;
    size_t startPos = 0;
    while ((pos = words.find('\n', startPos)) != string::npos)
    {
      line = words.substr(startPos, pos - startPos + 1);
      if (line.compare(0, infoPrefix.size(), infoPrefix))
        cleanedWords += line;
      startPos = pos + 1;
    }
    return cleanedWords;
  }

  // Execute a command.
  void execute(const char* cmd)
  {
    FILE* fp = popen(cmd,  "w");
    pclose(fp);
  }

  // Variables used by the test.
  const std::string _name;
  const std::string _extension;
  const std::string _extensionOutputWords;
  const std::string _extensionOutputDocs;
};


// _____________________________________________________________________________
TEST_F(CsvParserTest, optionFullText)
{
  const char* csventry = "field\n"
                         "amicus5 certus in re incerta cernitur\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "amicus5\t1\t1\t1\n"
                          "certus\t1\t1\t2\n"
                          "in\t1\t1\t3\n"
                          "re\t1\t1\t4\n"
                          "incerta\t1\t1\t5\n"
                          "cernitur\t1\t1\t6\n";

  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --full-text=field"
          " --write-words-file-ascii --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionShow)
{
  string csventry = "field1\tfield2\n"
                         "show1\tshow2\n";
  write("testbase.csv", csventry.c_str());
  // Without info-delimiter and just one show title.
  {
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:<field1>show1</field1>"
                            "<field2>show2</field2>\tH:\n";
    execute("./CsvParserMain --base-name=testbase --show=field1,field2"
            " --write-words-file-ascii --write-docs-file > /dev/null");

    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }
  // Check that the info delimiter is optional.
  {
    remove("testbase.words-unsorted.ascii");
    remove("testbase.words-unsorted");
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:<field1>show1</field1>"
                            "<field2>show2</field2>"
                            "<field1>show1</field1>\tH:\n";
    execute("./CsvParserMain --base-name=testbase --show=field1,field2\\;field1"
            " --write-words-file-ascii --write-docs-file > /dev/null");
    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }
  // With info-delimiter and just one show title.
  {
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:<field1>show1</field1>"
                            "<field2>show2</field2>\tH:\n";
    execute("./CsvParserMain --base-name=testbase --show=field1,field2"
            " --info-delimiter=\\; --write-words-file-ascii --write-docs-file"
            " > /dev/null");
    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }
  // With info-delimiter and multiple title.
  {
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:<field1>show1</field1>"
                            "<field2>show2</field2>;"
                            "<field1>show1</field1>;"
                            "<field2>show2</field2>\tH:\n";
    execute("./CsvParserMain --show=field1,field2:xml\\;field1\\;field2:xml"
            " --info-delimiter=\\; --write-words-file-ascii --write-docs-file"
            " --base-name=testbase > /dev/null");
    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }

  // Tests concerning different input and output formats
  csventry = "xmlfield\ttextfield\tjsonfield\n"
             "show1 &amp; show2\t"
             "show1>\"test & \'test2\t"
             "[\"show1\":\"text\"]\n";
  write("testbase.csv", csventry.c_str());
  // Output: xml
  {
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:<xmlfield>show1 &amp; show2</xmlfield>"
                            "<textfield>show1&gt;&quot;test &amp; &apos;test2</textfield>"
                            "<jsonfield>[&quot;show1&quot;:&quot;text&quot;]</jsonfield>"
                            "\tH:\n";
    execute("./CsvParserMain --show=xmlfield,textfield,jsonfield:xml"
            " --write-words-file-ascii --write-docs-file"
            " --field-format=xmlfield:xml,textfield:text,jsonfield:json"
            " --base-name=testbase > /dev/null");
    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }
  // Output: json
  {
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:{\"xmlfield\":\"show1 &amp; show2\","
                            "\"textfield\":\"show1>\\\"test & \'test2\","
                            "\"jsonfield\":[\"show1\":\"text\"]}"
                            "\tH:\n";
    execute("./CsvParserMain --show=xmlfield,textfield,jsonfield:json"
            " --write-words-file-ascii --write-docs-file"
            " --field-format=xmlfield:xml,textfield:text,jsonfield:json"
            " --base-name=testbase > /dev/null");
    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }

  // Tests concerning fields, which are allowed to have multiple elements.
  csventry = "multipleField1\tmultipleField2\tnonMultipleField\n"
             "item1#item2#*item3#item4\t\titem1#withSep\n"
             "item5#*item6\titem7\titem8\n"
             "*item9\t*item10\t*item11\n";
  write("testbase.csv", csventry.c_str());
  {
    const char* expectedWordsOutput = "";
    const char* expectedDocsOutput = "1\tu:URL#1\t"
                          "t:{\"multipleField1\":[\"item1\",\"item2\",\"item4\"],"
                             "\"multipleField2\":[],"
                             "\"nonMultipleField\":\"item1#withSep\"}\tH:\n"
                        "2\tu:URL#2\t"
                        "t:{\"multipleField1\":[\"item5\"],"
                           "\"multipleField2\":[\"item7\"],"
                           "\"nonMultipleField\":\"item8\"}\tH:\n"
                        "3\tu:URL#3\t"
                        "t:{\"multipleField1\":[],"
                           "\"multipleField2\":[],"
                           "\"nonMultipleField\":\"\"}\tH:\n";

    execute("./CsvParserMain"
            " --show=multipleField1,multipleField2,nonMultipleField:json"
            " --write-words-file-ascii --write-docs-file"
            " --within-field-separator='#'"
            " --no-show-prefix='*'"
            " --allow-multiple-items=multipleField1,multipleField2"
            " --base-name=testbase > /dev/null");
    EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
    EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
  }
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionExcerpts)
{
  const char* csventry = "field1\tfield2\n"
    "es war als haett der himmel\tdie erde still gekuesst\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "";
  const char* expectedDocsOutput =
    "1\tu:URL#1\tt:\tH:es war als haett der himmel. "
    "die erde still gekuesst. \n";
  execute("./CsvParserMain --base-name=testbase --excerpts=field1,field2"
          " --write-words-file-ascii --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// ____________________________________________________________________________
TEST_F(CsvParserTest, optionFieldSeparator)
{
  const char* csventry = "field1,field2\n"
    "abc,def\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "abc\t1\t1\t1\ndef\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --full-text=field1,field2"
          " --field-separator=',' --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

//  ____________________________________________________________________________
TEST_F(CsvParserTest, optionCsvSeparator2)
{
  const char* csventry = "author\njens|simon\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "jens\t1\t1\t1\nsimon\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --full-text=author"
          " --write-words-file-ascii --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

//  ____________________________________________________________________________
TEST_F(CsvParserTest, optionScore)
{
  const char* csventry = "field1\tfield2\n"
    "festina lente\tanimam debet\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "festina\t1\t10\t1\n"
                       "lente\t1\t10\t2\n"
                       "animam\t1\t5\t3\n"
                       "debet\t1\t5\t4\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --full-text=field1,field2"
          " --score=field1:10,field2:5 --write-words-file-ascii"
          " --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionPhraseCompletion)
{
  const char* csventry = "field\n"
    "omnia vincit amor\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "omnia!omnia_vincit_amor\t1\t1\t1\n"
    "vincit!omnia_vincit_amor\t1\t1\t2\n"
    "amor!omnia_vincit_amor\t1\t1\t3\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --word-part-separator=!"
          " --phrase=field --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionFacets)
{
  const char* csventry = "field\nx box\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!facet!field!x_box\t1\t1\t1\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --word-part-separator=!"
          " --facets=field --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionFacetIds)
{
  const char* csventry = "field\nx box\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!facetid!field!x_box\t1\t1\t1\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --word-part-separator=!"
          " --facetids=field --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionFilter)
{
  const char* csventry = "field\ncras legam\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!filter!field!cras\t1\t1\t1\n"
    "!filter!field!legam\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --word-part-separator=!"
          " --filter=field --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionFilterPlus)
{
  const char* csventry = "field\ncras legam\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!filter!field!cras!cras_legam\t1\t1\t1\n"
    "!filter!field!legam!cras_legam\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --word-part-separator=!"
          " --filter-plus=field --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionOrderingPrecission)
{
  const char* csventry = "field1\tfield2\n"
    "1.12345678\t12345678.9\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!ordering!field1!0112345678\t1\t1\t1\n"
                         "!ordering!field2!1234567890\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase"
          " --word-part-separator-backend=!"
          " --ordering=field1:2.8,field2:8.2"
          " --write-words-file-ascii"
          " --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionOrderingDate)
{
  const char* csventry = "field1\tfield2\n"
    "22-01-2010\t31-10-1983\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!ordering!field1!20100122\t1\t1\t1\n"
    "!ordering!field2!19831031\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase"
          " --word-part-separator-backend=!"
          " --ordering=field1:date,field2:date"
          " --write-words-file-ascii"
          " --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, optionOrderingLiteral)
{
  const char* csventry = "field1\tfield2\n"
    "A305\tB404\n";
  write("testbase.csv", csventry);
  const char* expectedWordsOutput = "!ordering!field1!A305\t1\t1\t1\n"
                       "!ordering!field2!B404\t1\t1\t2\n";
  const char* expectedDocsOutput = "1\tu:URL#1\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase"
          " --word-part-separator-backend=!"
          " --ordering=field1:literal,field2:literal"
          " --write-words-file-ascii"
          " --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, userDefinedWords)
{
  const char* csventry = "field\n"
    "c+ c++\n"
    "yahoo? +yahoo!.\n"
    "c med.\n";
  write("testbase.csv", csventry);
  FILE* userDefinedWordsFile = NULL;
  userDefinedWordsFile = fopen("testbase.user-defined-words", "w");
  fprintf(userDefinedWordsFile, "Med.\nc++\nyahoo!\n__blubl!;;\n");
  fclose(userDefinedWordsFile);
  const char* expectedWordsOutput =
    "c\t1\t1\t1\n"
    "c++\t1\t1\t2\n"
    "yahoo\t2\t1\t1\n"
    "yahoo!\t2\t1\t2\n"
    "c\t3\t1\t1\n"
    "med.\t3\t1\t2\n";
  const char* expectedDocsOutput =
    "1\tu:URL#1\tt:\tH:\n"
    "2\tu:URL#2\tt:\tH:\n"
    "3\tu:URL#3\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase --full-text=field"
          " --read-user-defined-words --write-words-file-ascii"
          " --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, globalInformation)
{
  time_t now = time(NULL);
  struct tm time;
  localtime_r(&now, &time);
  char date[100];
  strftime(date, 100, "%d%b%y", &time);

  const char* csventry = "field\nx box\n";
  write("testbase.csv", csventry);
  std::string expectedWordsOutput = "!info!date!";
  expectedWordsOutput += date;
  expectedWordsOutput += "\t0\t0\t0\n"
    "!info!encoding!iso-8859-1\t0\t0\t0\n"
    "!info!name!testbase\t0\t0\t0\n"
    "!info!field-formats!\t0\t0\t0\n"
    "!info!facet!field\t0\t0\t0\n"
    "!facet!field!x_box\t1\t1\t1\n";
  execute("./CsvParserMain --base-name=testbase --word-part-separator=!"
          " --facets=field --write-words-file-ascii --write-docs-file"
          " > /dev/null");
  EXPECT_EQ(expectedWordsOutput.c_str(),
      fileToString("testbase.words-unsorted.ascii", false));
}

// _____________________________________________________________________________
TEST_F(CsvParserTest, USE_SYNONYM_IDS)
{
  const char* csventry = "A\n"
    "gtest\n"
    "volkswagen\n"
    "passat\n";
  write("testbase.csv", csventry);
  const char* synonymGroups =
    "test gtest unittest\r\n"
    "volkswagen vw *beatle *passat\n";
  write("testbase.synonym-groups", synonymGroups);
  const char* expectedWordsOutput =
    "gtest\t1\t1\t1\n"
    "S!2!gtest\t1\t1\t1\n"
    "volkswagen\t2\t1\t1\n"
    "S!4!volkswagen\t2\t1\t1\n"
    "passat\t3\t1\t1\n"
    "S!4!passat\t3\t1\t1\n";
  const char* expectedDocsOutput =
    "1\tu:URL#1\tt:\tH:\n"
    "2\tu:URL#2\tt:\tH:\n"
    "3\tu:URL#3\tt:\tH:\n";
  execute("./CsvParserMain --base-name=testbase"
          " --word-part-separator-backend=!"
          " --full-text=A"
          " --read-synonym-groups"
          " --write-words-file-ascii"
          " --write-docs-file > /dev/null");
  EXPECT_EQ(expectedWordsOutput, fileToString("testbase.words-unsorted.ascii"));
  EXPECT_EQ(expectedDocsOutput, fileToString("testbase.docs-unsorted"));
}

