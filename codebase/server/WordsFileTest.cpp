#include "Globals.h"
#include <gtest/gtest.h>
#include "WordsFile.h"

// Test whether getNextLine does what it should for FORMAT_HTDIG.
TEST(WordsFileTest, GetNextLineFormatHtdig)
{
  string wordsFileName = "WordsFileTest.TMP";
  // Create a small test file.
  {
    FILE* words_file = fopen(wordsFileName.c_str(), "w");
    fprintf(words_file, "%s\t%u\t%u\t%u\n", "word", 1, 2, 3);
    fclose(words_file);
  }
  // Read from it with WordsFile::getNextLine.
  {
    WordsFile wordsFile(wordsFileName);
    wordsFile.setFormat(WordsFile::FORMAT_HTDIG);
    string word;
    WordId wordId;
    DocId docId;
    DiskScore score;
    Position position;
    bool ret;
    ret = wordsFile.getNextLine(word, wordId, docId, score, position);
    ASSERT_TRUE(ret);
    ASSERT_EQ("word", word);
    ASSERT_EQ(-1, wordId);
    ASSERT_EQ((unsigned) 1, docId);
    ASSERT_EQ(2, score);
    ASSERT_EQ((unsigned) 3, position);
    ret = wordsFile.getNextLine(word, wordId, docId, score, position);
    ASSERT_FALSE(ret);
    ASSERT_TRUE(wordsFile.isEof());
  }
}

// Test whether getNextLine does what it should for FORMAT_BINARY.
TEST(WordsFileTest, GetNextLineFormatBinary)
{
  string wordsFileName = "WordsFileTest.TMP";
  // Create a small test file.
  {
    FILE* words_file = fopen(wordsFileName.c_str(), "w");
    WordId wordId = 0;
    DocId docId = 1;
    Score score = 2;
    Position position = 3;
    ASSERT_EQ((unsigned) 4, sizeof(WordId));
    ASSERT_EQ((unsigned) 4, sizeof(DocId));
    ASSERT_EQ((unsigned) 4, sizeof(Score));
    ASSERT_EQ((unsigned) 4, sizeof(Position));
    fwrite(&wordId, sizeof(WordId), 1, words_file);
    fwrite(&docId, sizeof(DocId), 1, words_file);
    fwrite(&score, sizeof(Score), 1, words_file);
    fwrite(&position, sizeof(Position), 1, words_file);
    fclose(words_file);
  }
  // Read from it with WordsFile::getNextLine.
  {
    WordsFile wordsFile(wordsFileName);
    wordsFile.setFormat(WordsFile::FORMAT_BINARY);
    string word;
    WordId wordId;
    DocId docId;
    DiskScore score;
    Position position;
    bool ret;
    ret = wordsFile.getNextLine(word, wordId, docId, score, position);
    ASSERT_TRUE(ret);
    ASSERT_EQ("", word);
    ASSERT_EQ(0, wordId);
    ASSERT_EQ((unsigned) 1, docId);
    ASSERT_EQ(2, score);
    ASSERT_EQ((unsigned) 3, position);
    ret = wordsFile.getNextLine(word, wordId, docId, score, position);
    ASSERT_FALSE(ret);
    ASSERT_TRUE(wordsFile.isEof());
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
