#include "Globals.h"
#include <gtest/gtest.h>
#include "HYBIndex.h"
#include "HYBCompleter.h"


// Test class with some useful functions for the test below.
class HYBIndexTest : public ::testing::Test
{
 protected:
  // Write posting to words file in ASCII.
  void writePostingToWordsFileAscii(FILE* file,
      const char* word, DocId docId, Score score, Position position)
  {
    fprintf(file, "%s\t%u\t%u\t%u\n", word, docId, score, position);
  }
            
  // Write posting to words file in BINARY.
  void writePostingToWordsFileBinary(FILE* file,
      WordId wordId, DocId docId, Score score, Position position)
  {
    ASSERT_EQ((unsigned) 4, sizeof(WordId));
    ASSERT_EQ((unsigned) 4, sizeof(DocId));
    ASSERT_EQ((unsigned) 4, sizeof(Score));
    ASSERT_EQ((unsigned) 4, sizeof(Position));
    fwrite(&wordId, 1, sizeof(WordId), file);
    fwrite(&docId, 1, sizeof(DocId), file);
    fwrite(&score, 1, sizeof(Score), file);
    fwrite(&position, 1, sizeof(Position), file);
  }
};


// Test whether HybIndex::buildIndex does what it should on a small test file
// for FORMAT_HTDIG.
TEST_F(HYBIndexTest, BuildIndexFormatHtdig)
{
  string wordsFileName = "HYBIndexTest.TMP.words";
  string vocabularyFileName = "HYBIndexTest.TMP.vocabulary";
  string indexFileName = "HYBIndexTest.TMP.hybrid";
  // Create a small words file.
  {
    FILE* words_file = fopen(wordsFileName.c_str(), "w");
    writePostingToWordsFileAscii(words_file, "aaa", 1, 2, 3);
    writePostingToWordsFileAscii(words_file, "abb", 4, 5, 6);
    writePostingToWordsFileAscii(words_file, "baa", 7, 8, 9);
    writePostingToWordsFileAscii(words_file, "bbb", 6, 5, 4);
    writePostingToWordsFileAscii(words_file, "bcc", 3, 2, 1);
    fclose(words_file);
  }
  // Build an index from this file, using blocks divided by prefix length 1,
  // that is we have one block [aaa - abb] and one block [baa - bcc].
  {
    HYB_BLOCK_VOLUME = 1;
    const int MODE = WITH_DUPS + WITH_POS + WITH_SCORES;
    HYBIndex index(indexFileName, vocabularyFileName, MODE);
    index.build(wordsFileName, "ASCII");
    ASSERT_EQ((unsigned) 2, index._metaInfo.getNofBlocks());
    TimedHistory history;
    FuzzySearch::FuzzySearcherUtf8 nullFuzzySearcher;
    HybCompleter<MODE> completer(&index, &history, &nullFuzzySearcher);
    {
      QueryResult block;
      completer.getDataForBlockId(0, block);
      ASSERT_EQ("{0,1}", block._wordIdsOriginal.debugString());
      ASSERT_EQ("{1,4}", block._docIds.debugString());
      ASSERT_EQ("{2,5}", block._scores.debugString());
      ASSERT_EQ("{3,6}", block._positions.debugString());
    }
    {
      QueryResult block;
      completer.getDataForBlockId(1, block);
      // Note that within a block, the postings are re-sorted by doc id.
      ASSERT_EQ("{4,3,2}", block._wordIdsOriginal.debugString());
      ASSERT_EQ("{3,6,7}", block._docIds.debugString());
      ASSERT_EQ("{2,5,8}", block._scores.debugString());
      ASSERT_EQ("{1,4,9}", block._positions.debugString());
    }
  }
}


// Test whether HybIndex::buildIndex does what it should on a small test file
// for FORMAT_BINARY.
TEST_F(HYBIndexTest, BuildIndexFormatBinary)
{
  string wordsFileName = "HYBIndexTest.TMP.words";
  string vocabularyFileName = "HYBIndexTest.TMP.vocabulary";
  string indexFileName = "HYBIndexTest.TMP.hybrid";
  // Create a small words file.
  {
    FILE* words_file = fopen(wordsFileName.c_str(), "w");
    ASSERT_EQ((unsigned) 4, sizeof(unsigned int));
    writePostingToWordsFileBinary(words_file, 0, 1, 2, 3);
    writePostingToWordsFileBinary(words_file, 1, 4, 5, 6);
    writePostingToWordsFileBinary(words_file, 2, 7, 8, 9);
    writePostingToWordsFileBinary(words_file, 3, 6, 5, 4);
    writePostingToWordsFileBinary(words_file, 4, 3, 2, 1);
    fclose(words_file);
    FILE* vocabulary_file = fopen(vocabularyFileName.c_str(), "w");
    fprintf(vocabulary_file, "%s\n", "Aaa"); 
    fprintf(vocabulary_file, "%s\n", "Abb"); 
    fprintf(vocabulary_file, "%s\n", "Baa"); 
    fprintf(vocabulary_file, "%s\n", "Bbb"); 
    fprintf(vocabulary_file, "%s\n", "Bcc"); 
    fclose(vocabulary_file);
  }
  // Build an index from this file, using blocks divided by prefix length 1,
  // that is we have one block [aaa - abb] and one block [baa - bcc].
  {
    HYB_BLOCK_VOLUME = 1;
    const int MODE = WITH_DUPS + WITH_POS + WITH_SCORES;
    HYBIndex index(indexFileName, vocabularyFileName, MODE);
    index.build(wordsFileName, "BINARY");
    ASSERT_EQ((unsigned) 2, index._metaInfo.getNofBlocks());
    TimedHistory history;
    FuzzySearch::FuzzySearcherUtf8 nullFuzzySearcher;
    HybCompleter<MODE> completer(&index, &history, &nullFuzzySearcher);
    {
      QueryResult block;
      completer.getDataForBlockId(0, block);
      ASSERT_EQ("{0,1}", block._wordIdsOriginal.debugString());
      ASSERT_EQ("{1,4}", block._docIds.debugString());
      ASSERT_EQ("{2,5}", block._scores.debugString());
      ASSERT_EQ("{3,6}", block._positions.debugString());
    }
    {
      QueryResult block;
      completer.getDataForBlockId(1, block);
      // Note that within a block, the postings are re-sorted by doc id.
      ASSERT_EQ("{4,3,2}", block._wordIdsOriginal.debugString());
      ASSERT_EQ("{3,6,7}", block._docIds.debugString());
      ASSERT_EQ("{2,5,8}", block._scores.debugString());
      ASSERT_EQ("{1,4,9}", block._positions.debugString());
    }
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
