#include <gtest/gtest.h>
#include "Globals.h"
#include "CompleterBase.h"
#include "HYBCompleter.h"
#include <stdio.h>


// CompleterBases template argument MODE that is used within all the tests.
const int MODE = WITH_DUPS + WITH_POS + WITH_SCORES;
extern StringConverter globalStringConverter;


// Helper: class to hold a words file entry.
class WordsFileEntry
{
 public:
  WordsFileEntry() {}
  WordsFileEntry(const std::string &word, DocId docId, Score score, Position position)
  {
    _word = word;
    _docId = docId;
    _score = score;
    _position = position;
  }
  std::string _word;
  DocId _docId;
  Score _score;
  Position _position;
};


// Helper: vector of WordsFileEntry objects.
class WordsFileEntries : public vector<WordsFileEntry>
{
 public:
  void push_back(const std::string &word, DocId docId, Score score, Position position)
  {
    vector<WordsFileEntry>::push_back(WordsFileEntry(word, docId, score, position));
  }
};


// Helper: WordsFileEntries Iterator.
typedef std::vector<WordsFileEntry>::const_iterator WordsFileEntriesIterator;


// Helper: Compare two WordsFileEntry objects.
struct WordsFileEntryComparer
{
  bool operator() (const WordsFileEntry &w1, const WordsFileEntry &w2)
  {
    if (w1._word < w2._word) return true;
    if (w1._word == w2._word) return w1._docId < w2._docId;
    return false;
  }
} wordsFileEntryCompare;


// Helper: class to hold the environment for CompleterBase.
class CompleterBaseEnvironment
{
 public:
  CompleterBaseEnvironment()
  {
     FILE* fd;
     WordsFileEntries entries;

     entries.push_back("aachen",      1, 1, 1);
     entries.push_back("aal",         1, 1, 2);
     entries.push_back("aalglatt",    1, 1, 3);
     entries.push_back("aargau",      1, 1, 4);
     entries.push_back("aachen",      2, 1, 1);
     entries.push_back("aalglatt",    2, 1, 2);
     entries.push_back("aal",         2, 1, 3);
     entries.push_back("babbeln",     3, 1, 1);
     entries.push_back("baby",        3, 1, 2);
     entries.push_back("babyboom",    3, 1, 3);
     entries.push_back("babylonisch", 3, 1, 4);
     entries.push_back("babbeln",     4, 1, 1);
     entries.push_back("baby",        4, 1, 2);
     entries.push_back("babyboom",    4, 1, 3);


    sortAndSetWords(&entries);

    // Redirect stdout for a while.
    fd = freopen("/dev/null", "a", stdout);
    _index = new HYBIndex(INDEX_FILE_NAME, VOC_FILE_NAME, MODE);
    _index->build(WORDS_FILE_NAME, "ASCII");
    fd = freopen("/dev/tty", "a", stdout);
    _fuzzy = new FuzzySearch::FuzzySearcherUtf8();
    _history = new TimedHistory();
    _completer = new HybCompleter<MODE>(_index,
                                        _history,
                                        _fuzzy);
  };

  ~CompleterBaseEnvironment()
  {
    delete _history;
    delete _index;
    delete _fuzzy;
    remove(WORDS_FILE_NAME.c_str());
    remove(INDEX_FILE_NAME.c_str());
    remove(VOC_FILE_NAME.c_str());
  }

 private:
  void sortAndSetWords(WordsFileEntries *entries)
  {
    WordsFileEntriesIterator it;
    FILE* words_file;

    // Sort entries and write them to WORDS_FILE_NAME.
    std::sort(entries->begin(), entries->end(), wordsFileEntryCompare);
    words_file = fopen(WORDS_FILE_NAME.c_str(), "w");
    for (it = entries->begin(); it != entries->end(); ++it)
    {
      fprintf(words_file, "%s\t%u\t%u\t%u\n", it->_word.c_str(), it->_docId, it->_score, it->_position);
    }
    fclose(words_file);
  }

 public:
  TimedHistory *getHistory(void) const { return _history; }
  HYBIndex *getIndex(void) const { return _index; }
  FuzzySearch::FuzzySearcherUtf8 *getFuzzy(void) const { return _fuzzy; }
  CompleterBase<MODE> *getCompleter(void) const { return _completer; }

 private:
  TimedHistory                           *_history;
  HYBIndex                          *_index;
  FuzzySearch::FuzzySearcherUtf8    *_fuzzy;
  CompleterBase<MODE>               *_completer;

  static std::string                 WORDS_FILE_NAME;
  static std::string                 VOC_FILE_NAME;
  static std::string                 INDEX_FILE_NAME;
};

std::string CompleterBaseEnvironment::WORDS_FILE_NAME = "WORDS_FILE_TMP";
std::string CompleterBaseEnvironment::VOC_FILE_NAME   = "VOC_FILE_TMP";
std::string CompleterBaseEnvironment::INDEX_FILE_NAME = "INDEX_FILE_TMP";


typedef std::pair<std::vector<const char*>, const char*> QueriesResultPair;
typedef std::pair<const char *, const char*> QueryResultPair;


// Fixture: Basis.
class CompleterBaseTest : public ::testing::Test
{

 protected:

  CompleterBaseEnvironment _completerEnv;

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


// Fixture: Queries.
class ProcessQueryTest :  public CompleterBaseTest
{ };

// Fixture: Filtering History.
class FilterHistoryTest : public ProcessQueryTest,
                          public ::testing::WithParamInterface<const char*>
{ };

// Fixture: Filtering Index.
class FilterIndexTest : public ProcessQueryTest,
                        public ::testing::WithParamInterface<QueryResultPair>
{ };


// Each pair represents a test case.
// first  :  Query to evaluate.
// second :  Excpected result (see QueryResult::asString).
QueryResultPair queryResultPairs_SIMPLE[] = {
  // Empty query.
  std::make_pair(
      "",
      ", 4, '', 0, '', 0, 0, [], [], [], []"),
  // Simple query.
  std::make_pair(
      "aachen",
      "aachen, 12, '', 0, '', 2, 1, [1 2], [0 0], [1 1], [1 1]"),
  // Prefix query simple.
  std::make_pair(
      "aal*",
      "aal*, 12, '', 0, '', 2, 2, [1 1 2 2], [1 2 2 1], [1 1 1 1], [2 3 2 3]"),
  // AND query simple.
  std::make_pair(
      "aachen aal",
      "aal, 12, '', 0, '', 2, 1, [1 1 2 2], [1 -1 1 -1], [1 1 1 1], [2 99999 3 99999]"),
  // AND query with * left hand side.
  std::make_pair(
      "aachen* aal",
      "aal, 12, '', 0, '', 2, 1, [1 1 2 2], [1 -1 1 -1], [1 1 1 1], [2 99999 3 99999]"),
  // AND query with * right hand side.
  std::make_pair(
      "aachen aal*",
      "aal*, 12, '', 0, '', 2, 2, [1 1 1 2 2 2], [1 2 -1 2 1 -1], [1 1 1 1 1 1], [2 3 99999 2 3 99999]"),
  // . query simple.
  std::make_pair(
      "aachen.aal",
      "aal, 12, '', 0, '', 1, 1, [1], [1], [2], [2]"),
  // . query with * left hand side.
  std::make_pair(
      "aachen*.aal",
      "aal, 12, '', 0, '', 1, 1, [1], [1], [2], [2]"),
  // . query with * right hand side.
  std::make_pair(
      "aachen.aal*",
      "aal*, 12, '', 0, '', 2, 2, [1 2], [1 2], [2 2], [2 2]"),
  // .. query simple
  std::make_pair(
      "aachen..aal",
      "aal, 12, '', 0, '', 2, 1, [1 2], [1 1], [2 2], [2 3]"),
  // .. query with * left hand side.
  std::make_pair(
      "aachen*..aal",
      "aal, 12, '', 0, '', 2, 1, [1 2], [1 1], [2 2], [2 3]"),
  // .. query with * right hand side.
  std::make_pair(
      "aachen..aal*",
      "aal*, 12, '', 0, '', 2, 2, [1 1 2 2], [1 2 2 1], [2 2 2 2], [2 3 2 3]"),
  // OR query simple.
  std::make_pair(
      "aachen|aal",
      "aal, 12, '', 0, '', 2, 2, [1 1 2 2], [0 1 0 1], [1 1 1 1], [1 2 1 3]"),
  // OR query with * left hand side.
  std::make_pair(
      "aachen*|aal",
      "aal, 12, '', 0, '', 2, 2, [1 1 2 2], [0 1 0 1], [1 1 1 1], [1 2 1 3]"),
  // OR query with * right hand side.
  std::make_pair(
      "aachen|aal*",
      "aal*, 12, '', 0, '', 2, 3, [1 1 1 2 2 2], [0 1 2 0 2 1], [1 1 1 1 1 1], [1 2 3 1 2 3]"),
  // Join query simple.
  std::make_pair(
      "[aachen#aal]",
      ", 12, '', 0, '', 0, 0, [], [], [], []"),
};

// Generate Tests for SIMPLE Queries.
INSTANTIATE_TEST_CASE_P(SIMPLE_QUERIES,
                        FilterIndexTest,
                        ::testing::ValuesIn(queryResultPairs_SIMPLE));


// _____________________________________________________________________________
TEST_P(FilterIndexTest, processQuery)
{
  QueryResultPair pair = GetParam();
  QueryResult *result = NULL;
  _completerEnv.getCompleter()->processQuery(Query(pair.first), result);
  EXPECT_STREQ(pair.second, result->asStringFlat().c_str())
    << "Query was: '" << pair.first <<  "'";
};

// _____________________________________________________________________________
TEST_P(FilterIndexTest, processQuery_NEW)
{
  QueryResultPair pair = GetParam();
  QueryResult *result = NULL;
  _completerEnv.getCompleter()->processQuery_NEW(Query(pair.first), result);
  EXPECT_STREQ(pair.second, result->asStringFlat().c_str())
    << "Query was: '" << pair.first <<  "'";
};


// _____________________________________________________________________________
TEST_F(CompleterBaseTest, processComplexQuery_filters_history)
{
  QueryResult *result1, *result2;
  result1 = result2 = NULL;
  _completerEnv.getCompleter()->processQuery(Query("aachen"), result1);
  ASSERT_EQ(static_cast<unsigned>(1),
            _completerEnv.getHistory()->getNofQueries());
  _completerEnv.getCompleter()->processQuery(Query("aachen"), result2);
  EXPECT_STREQ("aachen, 12, '', 1, '', 2, 1, [1 2], [0 0], [1 1], [1 1]",
               result2->asStringFlat().c_str())    
    << "Query was: '" << "aachen" <<  "'";
}


// _____________________________________________________________________________
TEST_F(CompleterBaseTest, intersectTwoPostingLists)
{
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;
  QueryResult input1;
  QueryResult input2;
  QueryResult result;
  Separator separator = sameDocSeparator;
  ScoreAggregation scoreAggregation = SCORE_AGG_SUM;
  WordRange wordIdRange = infiniteWordIdRange;

  input1._docIds.parseFromString("11 13 15");
  input1._wordIdsOriginal.parseFromString("21 23 25");
  input1._scores.parseFromString("31 33 35");
  input1._positions.parseFromString("41 43 45");
  input2._docIds.parseFromString("14 15 16");
  input2._wordIdsOriginal.parseFromString("24 25 26");
  input2._scores.parseFromString("34 35 36");
  input2._positions.parseFromString("44 45 46");
  result.clear();
  completer.intersectTwoPostingLists
        (input1, input2, result,
         separator, scoreAggregation, wordIdRange);
  ASSERT_EQ("[15 15]", result._docIds.asString());
  ASSERT_EQ("[25 -1]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[35 35]", result._scores.asString());
  ASSERT_EQ("[45 99999]", result._positions.asString());

  // Test case inspired by scoring commutativity bug reported by Weitkaemper
  // 14Dec10 (samsung syncmaster vs. syncmaster samsung).
  input1._docIds.parseFromString("1 1");
  input1._wordIdsOriginal.parseFromString("1 1");
  input1._scores.parseFromString("1 1");
  input1._positions.parseFromString("1 1");
  input2._docIds.parseFromString("1");
  input2._wordIdsOriginal.parseFromString("2");
  input2._scores.parseFromString("1");
  input2._positions.parseFromString("1");
  result.clear();
  completer.intersectTwoPostingLists
        (input1, input2, result,
         separator, scoreAggregation, wordIdRange);
  ASSERT_EQ("[1 1]", result._docIds.asString());
  ASSERT_EQ("[2 -1]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[1 2]", result._scores.asString());
  ASSERT_EQ("[1 99999]", result._positions.asString());
  result.clear();
  completer.intersectTwoPostingLists
        (input2, input1, result,
         separator, scoreAggregation, wordIdRange);
  ASSERT_EQ("[1 1 1]", result._docIds.asString());
  ASSERT_EQ("[1 1 -1]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[1 1 1]", result._scores.asString());
  ASSERT_EQ("[1 1 99999]", result._positions.asString());
}

// _____________________________________________________________________________
TEST_F(CompleterBaseTest, computeTopHitsAndCompletions)
{
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;
  QueryParameters queryParameters;
  queryParameters.howToRankDocs = QueryParameters::RANK_DOCS_BY_DOC_ID;
  queryParameters.howToRankWords = QueryParameters::RANK_WORDS_BY_WORD_ID;
  queryParameters.sortOrderDocs = SORT_ORDER_ASCENDING;
  queryParameters.sortOrderWords = SORT_ORDER_ASCENDING;
  completer.setQueryParameters(queryParameters);

  QueryResult result;
  result._docIds.parseFromString("1 1 1 2 3 3 3 3");
  result._wordIdsOriginal.parseFromString("2 8 1 6 5 3 7 4");
  result._scores.parseFromString("1 1 1 1 1 1 1 1");
  result._positions.parseFromString("1 2 3 4 5 6 7 8");
  completer.computeTopHitsAndCompletions(result);
  ASSERT_EQ("[1 2 3]", result._topDocIds.asString());
  ASSERT_EQ("[1 2 3 4 5 6 7 8]", result._topWordIds.asString());
  ASSERT_EQ("[3 1 4]", result._topDocScores.asString());
}

// _____________________________________________________________________________
TEST_F(CompleterBaseTest, joinTwoPostingListsHashJoin)
{
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;
  QueryResult input1;
  QueryResult input2;
  QueryResult result;
  input1._docIds.parseFromString("11 13 15");
  input1._wordIdsOriginal.parseFromString("21 23 25");
  input1._scores.parseFromString("31 33 35");
  input1._positions.parseFromString("41 43 45");
  input2._docIds.parseFromString("14 15 16");
  input2._wordIdsOriginal.parseFromString("24 25 26");
  input2._scores.parseFromString("34 35 36");
  input2._positions.parseFromString("44 45 46");
  result.clear();
  completer.joinTwoPostingLists(input1, input2, result, QueryParameters::HASH_JOIN);
  ASSERT_EQ("[15 15]", result._docIds.asString());
  ASSERT_EQ("[25 25]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[35 35]", result._scores.asString());
  ASSERT_EQ("[45 45]", result._positions.asString());
}

// _____________________________________________________________________________
TEST_F(CompleterBaseTest, joinTwoPostingListsMergeJoin)
{
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;
  QueryResult input1;
  QueryResult input2;
  QueryResult result;
  input1._docIds.parseFromString("11 13 15");
  input1._wordIdsOriginal.parseFromString("21 23 25");
  input1._scores.parseFromString("31 33 35");
  input1._positions.parseFromString("41 43 45");
  input2._docIds.parseFromString("14 15 16");
  input2._wordIdsOriginal.parseFromString("24 25 26");
  input2._scores.parseFromString("34 35 36");
  input2._positions.parseFromString("44 45 46");
  result.clear();
  completer.joinTwoPostingLists(input1, input2, result, QueryParameters::MERGE_JOIN);
  ASSERT_EQ("[15 15]", result._docIds.asString());
  ASSERT_EQ("[25 25]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[35 35]", result._scores.asString());
  ASSERT_EQ("[45 45]", result._positions.asString());
}
// _____________________________________________________________________________
TEST_F(CompleterBaseTest, joinTwoPostingListsHashJoinWithSpecialWord)
{
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;
  QueryResult input1;
  QueryResult input2;
  QueryResult result;
  input1._docIds.parseFromString("11 13 15 15 20");
  input1._wordIdsOriginal.parseFromString("-1 23 -1 25 20");
  input1._scores.parseFromString("31 33 35 35 20");
  input1._positions.parseFromString("99999 43 99999 45 20");
  input2._docIds.parseFromString("14 15 16 21");
  input2._wordIdsOriginal.parseFromString("24 25 26 20");
  input2._scores.parseFromString("34 35 36 20");
  input2._positions.parseFromString("44 45 46 20");
  result.clear();
  completer.joinTwoPostingLists(input1, input2, result, QueryParameters::HASH_JOIN);
  ASSERT_EQ("[15 15 15 20 21]", result._docIds.asString());
  ASSERT_EQ("[25 25 -1 20 20]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[35 35 35 20 20]", result._scores.asString());
  ASSERT_EQ("[45 45 99999 20 20]", result._positions.asString());
}

// _____________________________________________________________________________
TEST_F(CompleterBaseTest, joinTwoPostingListsMergeJoinWithSpecialWord)
{
  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HybCompleter<MODE> completer;
  QueryResult input1;
  QueryResult input2;
  QueryResult result;
  input1._docIds.parseFromString("11 13 15 15 20");
  input1._wordIdsOriginal.parseFromString("-1 23 -1 25 20");
  input1._scores.parseFromString("31 33 35 35 20");
  input1._positions.parseFromString("99999 43 99999 45 20");
  input2._docIds.parseFromString("14 15 16 21");
  input2._wordIdsOriginal.parseFromString("24 25 26 20");
  input2._scores.parseFromString("34 35 36 20");
  input2._positions.parseFromString("44 45 46 20");
  result.clear();
  completer.joinTwoPostingLists(input1, input2, result, QueryParameters::MERGE_JOIN);
  ASSERT_EQ("[15 15 15 20 21]", result._docIds.asString());
  ASSERT_EQ("[25 25 -1 20 20]", result._wordIdsOriginal.asString());
  ASSERT_EQ("[35 35 35 20 20]", result._scores.asString());
  ASSERT_EQ("[45 45 99999 20 20]", result._positions.asString());
}

// _____________________________________________________________________________
int main(int argc, char **argv)
{
  globalStringConverter.init();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
