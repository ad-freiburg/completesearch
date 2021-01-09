#ifndef __QUERYRESULT_H__
#define __QUERYRESULT_H__

#include <google/dense_hash_map>
#include "Globals.h"
#include "DocList.h"
#include "Completions.h"
#include "DocList.h"
#include "WordList.h"
#include "Vector.h"
#include "QueryParameters.h"

//#include <vector>
//#include <string>
//#include <sstream>
//#include "assert.h"


using namespace std;

//! Class for storing the result of processing a query; TODO: rename to Result.
/*!
 *  A result has the following components:
 *   - a list of postings (4-tuples of word id, doc id, position, score),
 *     stored in four parallel vectors - a list of the top ranked doc ids
 *   - a list of the top ranked word ids
 *
 *  The class provides support for concurrent access (from multiple threads) to
 *  the same result. TODO: Explain more.
 */
class QueryResult
{
 public: /* HOLGER 14Jan06: more convenient to have these public */


  //
  // 1. PARAMETERS
  //

  // NEW(bast, 15Dec10): The query for which this is the result.
  Query _query;

  //! The prefix (last query word) that was completed
  string _prefixCompleted;

  //! Status a result might have (important when running in multithreaded mode)
  enum StatusEnum {
    ERROR              = 0,
    DOES_NOT_EXIST     = 1, // what is this used for?
    UNDER_CONSTRUCTION = 2,
    FINISHED           = 4,
    IN_USE             = 8, // to avoid removal from history
  };

  //! Status of result; one or combination of: DOES_NOT_EXIST(?), UNDER_CONSTRUCTION, FINISHED, IN_USE
  StatusEnum _status;

  //! Error message, in case something went wrong (otherwise empty)
  string errorMessage;

  //! Will be \c false on creation but will be changed to \c true for later requests; TODO: bad name.
  bool wasInHistory;

  //! If result was obtained by filtering the result from another query, this provides the query string.
  string resultWasFilteredFrom;

  //! Number of top-ranked doc ids to compute and store
  DocId nofTotalHits; 

  //! Number of top-ranked word ids to compute and store
  WordId nofTotalCompletions; 

  //! Possible kinds of results (describing the way it was computed)
  static string UNDEFINED;
  static string NORMAL;
  static string FROM_HISTORY;
  static string FROM_HISTORY_TOPK_AGAIN;
  static string FROM_HISTORY_FILTERED_1;
  static string FROM_HISTORY_FILTERED_2;
  static string FROM_HISTORY_FILTERED_3;
  static string FROM_HISTORY_INTERSECT;
  static string ALL_POSTINGS;
  static string OTHER;

  //! The kind of this result (describing the way it was computed)
  string _howResultWasComputed;

  //  
  //  2. RESULT LISTS
  //

  // Matching postings: These four vectors have the same length (of
  // the position list). If built without positions, then it has the length of
  // the docs
  
  //! Doc ids of the matching postings;
  DocList _docIds;
  //! Original word ids of the matching postings;
  // NEW 17May13 (baumgari): We need to differ between the original word ids and
  // the mapped word ids. This has to be done, since elsewhise we would need to
  // map word id ranges, which cannot be done in a valid way in all cases: i.e.
  // original: muller:müller to mullerburg:muellerburg maps to müller to
  // muellerburg, which might be empty, if int (ü) > int (u). Right now this is
  // just relevant for CompleterBase::copyAndFilter.
  // Usually we should use the original word ids to avoid errors. In especially
  // _wordIdsMapped is empty before mapping in CompleterBase.TopK::remapWordIds.
  // This might be changed for consistency reasons, but right now this would
  // just mean an overload, which is not necessary.
  WordList _wordIdsOriginal;
  //! Mapped word ids of the matching postings;
  WordList _wordIdsMapped;
  //! Positions of the matching postings;
  Vector<Position> _positions;
  //! Scores (aggregated) of the matching postings;
  //  Note: these are NOT the raw scores of the last position
  Vector<Score> _scores;

  // Top-ranked documents and words

  //! Matching doc ids; TODO: sorted or not?;
  DocList _topDocIds;
  // These two (?) vectors have the same length
  //! Scores of the matching doc ids;
  Vector<Score> _topDocScores; 
  // TODO: 30Jun06: Discontinue use of this field (as it won't be fully written in a proper top-k setting)
  // These are the scores for the case where the block possibly continues
  // while we're still in the first block, this is just a vector of zeros
  // TODO: Which scores are these?
  Vector<Score> _lastBlockScores;
  // also store document scores for the last (!) block before
  // if we are in the first block, then this should all be zeros.
  
  //! Matching word ids;
  WordList _topWordIds;
  //! Matching words (completions); TODO: rename as _topCompletions
  //  TODO: does not really belong to this class, does it? 
  Completions _topCompletions;

  //  NEW 30Jun06: So far this was part of the strings in Completions
  //! Total number of documents containing each matching word;
  Vector<unsigned int> _topWordDocCounts;
  //! Total number of occurrences of each matching word; TODO: rename to _topWordOccCounts
  Vector<unsigned int> _topWordOccCounts;
  //! Score of each matching word;
  Vector<Score> _topWordScores;

  // NEW(bast, 7Dec10): Scores for the top completions, to be used in
  // computeTopHits when howToRankDocs == RANK_DOCS_BY_COMPLETION_SCORE.
  // TODO(bast): replace by google::dense_hash_map. When I tried it, it produced
  // a segmentation fault, however, and I don't know why.
  std::unordered_map<WordId, Score> _completionScoresByWordId;
  // google::dense_hash_map<WordId, float> _normalizedEditDistances;

  // NEW(hagn, 14Jun11): Edit-Distance-Scores (Fuzzy-Score) for the top completions,
  // to be used in computeTopHits when howToRankDocs == RANK_DOCS_BY_FUZZY_SCORE.
  std::unordered_map<WordId, Score> _completionFuzzyScoresByWordId;

  // NEW(buchholb, 16Mar11): WordIds for the TopDocs. Required to properly
  // group them and assign the group a title
  WordList _topDocWordIds;

  //! Query parameters used to compute this result
  QueryParameters _queryParameters;

  // NEW(celikik, 26Aug11): Top fuzzy search query suggestions
  vector<string> _topFuzzySearchQuerySuggestions;

  // NEW(celikik, 26Aug11): Scores of the top fuzzy search query suggestions
  vector<double> _topFuzzySearchQuerySuggestionScores;

  //
  //  3. WORKING VARIABLES
  //

  //! TODO: Which position in which buffer?
  /*!
   *   I think this is the position of the current posting when constructing a
   *   result as, e.g., when doing an intersect; see that method
   */
  unsigned long _bufferPosition; 

  //! TODO: what exactly is this used for?
  mutable bool isLockedForWriting;
  
  //! TODO: what exactly is this used for?
  mutable bool isLockedForReading;
 
  // Mark result as locked for reading "r"
  // or writing "w".
  void lock(const char* mode)
  {
    if (strcmp(mode, "w") == 0) isLockedForWriting = true;
    else if (strcmp(mode, "r") == 0) isLockedForReading = true;
  }
  
  // Mark result as unlocked for reading "r"
  // or writing "w".
  void unlock(const char* mode)
  {
    if (strcmp(mode, "w") == 0) isLockedForWriting = false;
    else if (strcmp(mode, "r") == 0) isLockedForReading = false;
  }

 public:

  //
  //  1. CONSTRUCTORS
  //

  //! Create empty result, with status UNDER_CONSTRUCTION
  QueryResult();

  //! Create query result containing (conceptually) all postings; used for processing one-word queries; TODO: what if argument if \c false?
  /*!
   *    Note: made "explicit", since otherwise anything that can be implicitly
   *    cast to bool, can be passed as a QueryResult parameter, without compiler
   *    error
   */
  explicit QueryResult(bool fullList);

  //! Copy constructor; used for join (intersection of word list) and for not operator (intersection with complement)
  QueryResult(const QueryResult& orig);


  //
  //  2. SET
  //
   
  //! Set to empty result
  void clear();

  //! Set error message (if empty, writes default message)
  void setErrorMessage(string msg = "");

  //! Set docs ids of matching postings
  void setMatchingDocsWithDups(const DocList& matchingDocsWithDups)
  {
    _docIds.copy(matchingDocsWithDups);
  }

  //! Set word ids of matching postings
  void setMatchingWordsWithDups(const WordList& matchingWordsWithDups)
  {
    _wordIdsOriginal.copy(matchingWordsWithDups);
  }

  //! Set positions of matching postings
  void setPositionlist(const Vector<Position>& positions)
  {
    assert(positions.size() == _wordIdsOriginal.size());
    assert(_docIds.size() == _wordIdsOriginal.size());
    _positions.copy(positions);
  }

  //! Set scores of matching postings
  /*!   
   *    If there are positions (and scores) then there will be a score for every position
   *    Otherwise, there will a score for each document only.
   */
  void setScorelist(const Vector<Score>& scores)
  {
    assert(scores.isPositive());
    assert(scores.size() == _wordIdsOriginal.size());
    assert(_docIds.size() == _wordIdsOriginal.size());
    _scores.copy(scores);
  }

  //! TODO: what are last block scores?
  void setLastBlockScores(const Vector<Score>& lastBlockScores)
  {
    assert((lastBlockScores.size() == _topDocIds.size()) || ( lastBlockScores.size() == 0));
    _lastBlockScores.copy(lastBlockScores);
  }

  //! Set matching doc ids; must be sorted!
  void setMatchingDocs(const DocList& matchingDocs)
  {
    assert(matchingDocs.isSorted(true));
    _topDocIds.copy(matchingDocs);
    assert(_topDocIds.isSorted(true));
  }

  //! Set matching word ids; must be sorted (really?)!
  void setMatchingWords(const WordList& matchingWords)
  {
    _topWordIds.copy(matchingWords);
    assert(_topWordIds.isSorted(true));
  }

  //! Set scores of matching doc ids
  void setDocumentScores(const Vector<Score>& docScores)
  {
    assert(_topDocIds.size() == docScores.size());
    _topDocScores.copy(docScores);
  }
 
  //! Set matching words (completions) and their scores; TODO: same method in class Completions; TODO: why templated?; TODO: not timed
  template <typename vS, typename vC, typename vO>  
  void setCompletions(const vS& scores, const vC& docCounts, const vO& occCounts, const Vocabulary& vocabulary)
  {
    assert(&scores == &_topWordScores);
    assert(&docCounts == &_topWordDocCounts);
    assert(&occCounts == &_topWordOccCounts);
    // TODO: PASS THIS TIMER AS ARGUMENT
    //      GlobalTimers::setCompletionsTimer.cont();
    assert(_topWordIds.size() == scores.size());
    //      assert((unsigned long long) _topWordIds.size() <= (unsigned long long) DEFAULT_NOF_COMPLETIONS_TO_COMPUTE);
    _topCompletions.set(vocabulary,_topWordIds,scores,docCounts,occCounts);
    //      GlobalTimers::setCompletionsTimer.stop();
  }

  //! Set description how result was computed
  void setHowResultWasComputed(const string& description) { _howResultWasComputed = description; }

  //! Free all extra (reserved) space, once result is finished; TODO: not timed
  void freeExtraSpace();


  //
  //  3. GET
  //

  //! Get number of postings
  unsigned int getNofPostings() const { return _docIds.size(); }

  //! Get number of postings; deprecated (stupid name)
  unsigned int getSize() const { return _docIds.size(); }

  //! Get status 
  unsigned int getStatus() const { return (unsigned int)(_status); }

  //! Check if result (conceptually) contains all postings
  bool isFullResult() const { return _topDocIds.isFullList(); }
  
  //! Number of bytes consumed by this result (essentially the total size of the postings lists)
  size_t sizeInBytes() const;

  //! Get matching doc ids 
  /*!
   *   Note: can't make const "due to push_back(INFTY_DOCID)" (quote from Ingmar)
   */
  DocList& getMatchingDocs() 
  {
    assert(_topDocIds.isSorted(true));
    return _topDocIds;
  }

  //! Get matching words (completions)
  const Completions& getCompletions() const { return _topCompletions; }

  //! TODO: what does this do and where is it used?
  void unlock() const
  {
    isLockedForWriting = false;
    isLockedForReading = false;
  }


  //
  //  4. CHECKS
  //

  //! Check that the parallel vectors are of the same length; throw exception otherwise 
  /*!
   *    \param MODE  the mode of the completer; e.g. for a completer without
   *    scores the length of the score vector is always zero.
   */
  void checkSameNumberOfDocsWordsPositionsScores(unsigned char MODE) const;

  //! Simplistic sanity check; i.p. checks that status is IS_FINISHED
  /*!
   *   WARNING: this check will be used in various ASSERTs and must not take any
   *   significant time!!!
   */
  bool check() const;

  //! Check if the max word and max document score are equal (must only be true when all aggrgations are max)
  bool sameMax() const;

  //! True iff result is empty (zero matching postings)
  bool isEmpty() const;


  //
  //  5. SORTING
  //

  //! Sort the postings by doc id (sort the four vectors 'in parallel')
  void sortLists();
  
  //! Merge two results into one
  static void mergeResultLists(const QueryResult& input1, const QueryResult& input2, QueryResult& output);

  // Merge k results into one via k-way merge
  static void mergeResultLists(const vector<QueryResult>& inputLists, QueryResult* result);

  // Merge k results into one via k-way merge
  // Use pointers instead of objects
  // Needed for fuzzysearch
  static void mergeResultLists(const vector<const QueryResult*>& inputLists,
                               QueryResult* result);

  // Merge k *results into one via k-way merge
  // As input takes vector of pointers to QueryResult
  // Needed for fuzzysearch's groupMerging
  static void mergeResultLists(const vector<const QueryResult*>& inputLists,
                                     int from,
                                     int to,
                                     QueryResult* result);

//  //! Join two results = intersect lists of word ids; for queries of the kind \c auth[sigir sigmod]
//  /*!
//   *    Note 1: inputs are passed by value, since they will be (re)sorted; TODO: unavoidable?
//   *    Note 2: sorts using stl sort; TODO: why is that a problem or noteworthy?
//   *    Note 3: must be static, called without an object from CompleterBase::processJoinQuery
//   *
//   *    Historical note: last argument used to be called useLinear meaning the
//   *    merge join. This was a bad misnomer, since the has join has linear
//   *    running time, while the merge join takes superlinear time.
//   */
//  static void intersectWordlists(      QueryResult  input1,
//                                       QueryResult  input2,
//                                       QueryResult& result,
//                                 const int          howToJoin = QueryParameters::HASH_JOIN);


  //
  //  6. OUTPUT (for debugging)
  //
  
  // Dump object to string.
  std::string asString(void) const;

  std::string asStringFlat(void) const;

  //! TODO: Needed where? For debugging only?
  void dumpResultToStream(ostringstream& outputStream, Vocabulary& vocabulary) const;

  //! TODO: Needed where? For debugging only?
  void writeRankedResultToStream(ostringstream& outputStream, bool writeWords = true) const;

  //! Show result in details; for debugging purposes only I guess
  void show(unsigned int maxNumItemsToShowPerList = 10) const;

}; // end of class QueryResult


//! Special query result returned for empty query
extern QueryResult emptyQueryResult;

// needed for mergeResultLists(const Vector<QueryResult>& inputLists,
// QueryResult* result)
class Triple
{
  public:
   DocId docId;
   Position position;
   WordId wordId;
   int list;
};

// needed for mergeResultLists(const Vector<QueryResult>& inputLists,
// QueryResult* result)
class CompareTriple
{
  public:
    bool operator()(const Triple& x, const Triple& y) const
    {
      if (x.docId == y.docId)
        return x.position > y.position;
      else
        return x.docId > y.docId;
    }
};

#endif
