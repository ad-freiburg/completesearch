#include <vector>
#include <string>
#include <utility>
#include "../utility/TimerStatistics.h"
#include "./CompleterBase.h"
#include "./Exception.h"

//  Needed for CompleterBase default constructor below.
Vocabulary emptyVocabulary;
//History    emptyHistory;
// NEW 25 May (Björn): Now using TimedHistory in this place
// it should now be the only point that is different.
TimedHistory emptyHistory;
MetaInfo     emptyMetaInfo;
FuzzySearch::FuzzySearcherUtf8 nullFuzzySearcher;

// Defined in server/HYBIndex.h and by default set to false there. Can be set to
// true via command line option in StartCompletionServer.cpp.
extern bool fuzzySearchEnabled;
extern bool synonymSearchEnabled;
extern bool showQueryResult;

// NEW 07Aug13 (baumgari): Defined here, since this is the class where the
// queries are handled. The timeout can be adjusted as an option in
// startCompletionServer.
off_t queryTimeout = 5000;


// _____________________________________________________________________________
//! Constructor (from given history, vocabulary, index, and metainfo)
template <unsigned char MODE>
CompleterBase<MODE>::CompleterBase(      TimedHistory*    history,
                                         Vocabulary* vocabulary,
                                   const string& indexFileName,
                                         MetaInfo*   metaInfo,
                                         FuzzySearch::FuzzySearcherBase* fuzzySearcher
                                  )
:
  _indexStructureFile(indexFileName.c_str(), "rb")
{
  CS_ASSERT(history);
  CS_ASSERT(vocabulary);
  CS_ASSERT(metaInfo);
  CS_ASSERT(fuzzySearcher);
  CompleterBase<MODE>();
  _vocabulary = vocabulary;
  _metaInfo = metaInfo;
  _fuzzySearcher = fuzzySearcher;
  this->history = history;
  statusCode = -1;
}


// _____________________________________________________________________________
//! COPY CONSTRUCTOR (TODO: needed where?)
template <unsigned char MODE>
CompleterBase<MODE>::CompleterBase(const CompleterBase& orig)
:
  _indexStructureFile(orig._indexStructureFile),
  _vocabulary(orig._vocabulary),
  _metaInfo(orig._metaInfo),
  _fuzzySearcher(orig._fuzzySearcher)
{
  CompleterBase<MODE>();
  #ifndef NDEBUG
  log << " In copy constructor of CompleterBase ..." << endl;
  log << " size of copied vocabulary : " << _vocabulary.size() << endl;
  log << " size of original vocabulary : " << orig._vocabulary.size() << endl;
  #endif

  // non-trivial
  // TODO: why necessary, given the intializer above???
  _vocabulary = orig._vocabulary;
  _metaInfo = orig._metaInfo;

  // HYB-specific
  initialIntersectionBufferSize = orig.initialIntersectionBufferSize;
  initialUnionBufferSize = orig.initialUnionBufferSize;
  initialCompressionBufferSize = orig.initialCompressionBufferSize;

  #ifndef NDEBUG
  log << " Initial buffer sizes: " << initialIntersectionBufferSize << ", "
    << initialUnionBufferSize << ", " << initialCompressionBufferSize << endl;
  log << " Finished copy constructor of CompleterBase." << endl;
  #endif

}  // end: copy constructor


// _____________________________________________________________________________
//! DEFAULT DESTRUCTOR
//
//   needed, otherwise semantics of sth like the following would be undefined:
//   { CompleterBase* c = new InvCompleter; delete c; }
//
template <unsigned char MODE>
CompleterBase<MODE>::~CompleterBase()
{
  #ifndef NDEBUG
  log << "! CompleterBase destructor called." << endl;
  #endif
}


//! Get mode (TODO: is this needed anywhere?)
template <unsigned char MODE>
unsigned char CompleterBase<MODE>::mode()
{
  return MODE;
}

template <unsigned char MODE>
std::string CompleterBase<MODE>::asString(void) const
{
  std::stringstream ss;
  ss << "~~~~~~~~~~~" << std::endl
     << "Vocabulary" << std::endl
     << "~~~~~~~~~~~" << std::endl
     << _vocabulary->asString() << std::endl
     << "~~~~~~~~~~" << std::endl
     << "Meta info" << std::endl
     << "~~~~~~~~~~" << std::endl
     << _metaInfo->asString() << std::endl
     << "~~~~~~~~~~~~~~~" << std::endl
     << "Concurrent log" << std::endl
     << "~~~~~~~~~~~~~~~" << std::endl
     << log.asString() << std::endl
     << "~~~~~~~~" << std::endl
     << "History" << std::endl
     << "~~~~~~~~" << std::endl
     << history->asString() << std::endl
     << "~~~~~~~~~~~~~~~" << std::endl
     << "Fuzzy Searcher" << std::endl
     << "~~~~~~~~~~~~~~~" << std::endl
     << "TODO: write asString() here" << std::endl // _fuzzySearcher.asString() << std::endl
     << "~~~~~~~~~~~~~~~~~" << std::endl
     << "Query Parameters" << std::endl
     << "~~~~~~~~~~~~~~~~~" << std::endl
     // << _queryParameters.asString() << std::endl
     << "~~~~~~~~~~~~~" << std::endl
     << "Other Fields" << std::endl
     << "~~~~~~~~~~~~~" << std::endl
     << "Query                     : " << _query.asString() << std::endl
     << "Query (rewr.)             : " << _queryRewrittenForHighlighting.asString() << std::endl
     << "Last best matched word id : " << _lastBestMatchWordId << std::endl
     << "Mode                      : " << mode() << std::endl
     // << "Compression Buffer        : " << _compressionBuffer << std::endl
     << "~~~~~~~~~~~~~" << std::endl
     << "Buffer sizes" << std::endl
     << "~~~~~~~~~~~~~" << std::endl
     << "Initial intersection buffer size : " << initialIntersectionBufferSize << std::endl
     << "Initial union buffer size        : " << initialUnionBufferSize << std::endl
     << "Initial compression buffer size  : " << initialCompressionBufferSize << std::endl
     << "~~~~~~~~~~~~~~~~~~~~~" << std::endl
     << "Intersection Buffers" << std::endl
     << "~~~~~~~~~~~~~~~~~~~~~" << std::endl
     << "Docs      : ";
     if (_doclistIntersectionBuffer) ss << _doclistIntersectionBuffer->asString();
     else ss << (string) "NULL";
     ss << std::endl
     << "Words     : ";
     if (_wordlistIntersectionBuffer) ss << _wordlistIntersectionBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Positions : ";
     if (_positionlistIntersectionBuffer) ss << _positionlistIntersectionBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Scores    : ";
     if (_scorelistIntersectionBuffer) ss << _scorelistIntersectionBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "~~~~~~~~~~~~~~~" << std::endl
     << "Result Buffers" << std::endl
     << "~~~~~~~~~~~~~~~" << std::endl
     << "Docs union         : ";
     if (_doclistUnionBuffer) ss << _doclistUnionBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Words union        : ";
     if (_wordlistUnionBuffer) ss << _wordlistUnionBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Top doc score      : ";
     if (_topDocScoresBuffer) ss << _topDocScoresBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Last block score   : ";
     if (_lastBlockScoresBuffer) ss << _lastBlockScoresBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Top word score     : ";
     if (_topWordScoresBuffer) ss << _topWordScoresBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Top word doc count : ";
     if (_topWordDocCountsBuffer) ss << _topWordDocCountsBuffer->asString();
     else ss << "NULL";
     ss << std::endl
     << "Top word occ count : ";
     if (_topWordOccCountsBuffer) ss << _topWordOccCountsBuffer->asString();
     else ss << "NULL";

  return ss.str();
}


// _____________________________________________________________________________
//! PROCESS QUERY (method to be called from outside)
/*
 *    1. Rewrite join blocks of query
 *    2. Calls the internal recursive query processing method
 *    3. Sets result to NULL if some error occured (TODO: why?)
 */
template <unsigned char MODE>
void CompleterBase<MODE>::processQuery
      (const Query&        query,
             QueryResult*& result)
{
  log << AT_BEGINNING_OF_METHOD << "; query is \"" << query << "\"" << endl;
  // Note: use cont() and not start() for timer here because this method can
  // be called recursively (for special queries, TODO: for example?)
  processQueryTimer.cont();
  CS_ASSERT(_vocabulary->size() > 0);
  CS_ASSERT(result == NULL || result->nofTotalHits > result->_topDocIds.size());

  //
  // CASE 1: empty query -> return emptyQueryResult (defined in QueryResult.h, used to throw an exception)
  //
  if (query.empty())
  {
    result = &emptyQueryResult;
    result->_status = QueryResult::FINISHED;
  }

  //
  // CASE 2: non-empty query -> rewrite join blocks, and call the recursive processComplexQuery
  //
  else
  {
    Query queryRewritten = query;

    // 1. Rewrite join blocks: [...#...#...] -> ...#...#... with separators masked
    rewriteJoinBlocks(queryRewritten);
    log << IF_VERBOSITY_HIGH
        << "! query with join blocks rewritten: \"" << queryRewritten << "\"" << endl << flush;

    // 2. Call the internal recursive query processing method
    try
    {
      processComplexQuery(queryRewritten, result);
        // k1_docs, k2_words,
        // filterResults, useLinearWordlistIntersection);
      #ifndef NDEBUG
      cout << "! returned from recursive call (1)" << endl;
      #endif
      assert(result); // there will always be a (possibly empty) result
      assert(result->_status & QueryResult::FINISHED);
      assert(isInHistoryConst(queryRewritten));
      setStatusOfHistoryEntry(queryRewritten, QueryResult::FINISHED);
      setStatusOfHistoryEntry(queryRewritten, QueryResult::FINISHED | QueryResult::IN_USE);
    }

    // 3. If something went wrong, set result to NULL
    catch (Exception& e)
    {
      log << IF_VERBOSITY_HIGH
          << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
          << "\"" << queryRewritten << "\" from history" << endl;
      result = NULL;
      CS_RETHROW(e);
        // log << "! " << e.getFullErrorMessage() << endl;
        // CS_THROW(Exception::ERROR_PASSED_ON, e.getErrorMessage());
        // throw Exception(Exception::ERROR_PASSED_ON, e.getFullErrorMessage());
    }
  }

  if (result->check() == false) CS_THROW(Exception::BAD_QUERY_RESULT, "");
  assert(result == NULL || (result->_status & QueryResult::FINISHED));
  processQueryTimer.stop();
  log << AT_END_OF_METHOD << "; query was \"" << query << "\"" << endl;

} // end: processQuery


// _____________________________________________________________________________
//! Get the top continuation for a query, e.g. utf8 for !encoding:*
//
//  returns first completion WITHOUT the sent prefix, or
//  empty string if no completions or no * at the end
//
template <unsigned char MODE>
string CompleterBase<MODE>::getTopContinuationOfQuery(string queryString)
{
  if (queryString.length() == 0) return "";
  if (queryString[queryString.length() - 1] != '*') return "";

  Query query(queryString);
  QueryResult* result = NULL;
  _queryParameters.nofHitsToSend        = 0;
  _queryParameters.nofCompletionsToSend = 1;
  processQuery(query, result);
  if (result == NULL || result->_topCompletions.size() == 0) return "";
  string tmp = getWordFromVocabulary(result->_topWordIds[0]);
  tmp.erase(0, queryString.length() - 1);
  return tmp;
}

// _____________________________________________________________________________
//! Get all continuations for a query, e.g. !hitinfo-multiple:*
//
//  returns first completion WITHOUT the sent prefix, or
//  empty string if no completions or no * at the end
//
template <unsigned char MODE>
vector<string> CompleterBase<MODE>::getAllContinuationsOfQuery(string queryString)
{
  vector<string> allContinuations;
  if (queryString.length() == 0) return allContinuations;
  if (queryString[queryString.length() - 1] != '*') return allContinuations;

  Query query(queryString);
  QueryResult* result = NULL;
  _queryParameters.nofHitsToSend = 0;
  processQuery(query, result);
  if (result == NULL || result->_topCompletions.size() == 0) return allContinuations;
  for (size_t i = 0; i < result->_topCompletions.size(); i++)
  {
    string tmp = getWordFromVocabulary(result->_topWordIds[i]);
    tmp.erase(0, queryString.length() - 1);
    allContinuations.push_back(tmp);
  }
  return allContinuations;
}

// _____________________________________________________________________________
//! Merge two posting lists (currently only used for "or")
template <unsigned char MODE>
void CompleterBase<MODE>::mergeTwoPostingLists
      (const QueryResult& input1,
       const QueryResult& input2,
             QueryResult& result)
{
  // actual merge done via this call
  QueryResult::mergeResultLists(input1, input2, result);

  result._prefixCompleted = input1._prefixCompleted + " OR " +
                            input2._prefixCompleted;
  result.wasInHistory = false;
  result.resultWasFilteredFrom = string("");

  /*
  topMatchesFromAllMatches
   (result,
    WITHIN_DOC_AGGREGATION,
    BETWEEN_DOCS_AGGREGATION,
    WITHIN_COMPLETION_AGGREGATION,
    BETWEEN_COMPLETIONS_AGGREGATION,
    k1_docs,
    k2_words);
  */
}


// _____________________________________________________________________________
//! Process OR query, with last part of the form q1|q2|...|qm
/*!
 *    Implementation note: Uses an indirect recursion, for example if the
 *    query is xyz q1|q2|q3, call processQuery for xyz q1|q2, then call
 *    processQuery for xyz q3, and merge the two results. Note the
 *    processQuery will call this function here again, which will then call
 *    xyz q1, and merge the result for that with the result for xyz q2.
 *
 *    TODO: avoid the indirect recursion, it doesn't look necessary and makes
 *    the code harder to maintain.
 */
template <unsigned char MODE>
void CompleterBase<MODE>::processOrQuery
      (const QueryResult& resultFirstPart,
       const Query&       firstPartOfQuery,
       const Query&       lastPartOfQuery,
       const Separator&   separator,
             QueryResult& result)

{
  log << AT_BEGINNING_OF_METHOD
      << "; first part = \"" << firstPartOfQuery << "\""
      << "; last part = \"" << lastPartOfQuery  << "\""
      << "; separator = \"" << separator.getSeparatorString() << "\"" << endl;

  signed char separatorMode = separator._separatorIndex;
  assert(separatorMode != JOIN);
  assert(lastPartOfQuery.getQueryString().find(OR_QUERY_SEP) != string::npos);
  vector<string> partsOfOrQuery;

  // 1. Split OR block into compontens q1|q2|q3 -> vector of q1, q2, q3
  splitPartOfOrQueryBlock(lastPartOfQuery, partsOfOrQuery);
  if (partsOfOrQuery.size() < 2)
  {
    log << "! ERROR: ONLY GOT " << partsOfOrQuery.size()
        << " PARTS WHEN SPLITTING OR PART" << endl << flush;
    throw Exception(Exception::OR_PART_TOO_SHORT, "in method CompleterBase::allMatchesForOrPartAndCandidates");
  }

  // 2. Process first part of or query, e.g. for xyz q1|q2|q3 process xyz q1|q2
  //
  //   The first part calls processQuery again, which will call this procedure
  //   again if there is still an | left, so this is effectively a recursion;
  //   TODO: indirect recursion should be avoided if not really necessary like
  //   here.
  //
  string firstPartWithFirstOrPart = firstPartOfQuery.getQueryString();
  assert((separatorMode < (signed char) fixed_separators._separators.size()) || (separatorMode == FULL));

  if (separatorMode != FULL)
  {
    firstPartWithFirstOrPart += separator._separatorString;
  }
  else
  {
    assert(firstPartWithFirstOrPart == string(""));
  }

  if (partsOfOrQuery.size() == 2)
  {
    firstPartWithFirstOrPart += partsOfOrQuery[0];
  }
  else
  {
    firstPartWithFirstOrPart += partsOfOrQuery[0];
    for (unsigned int i=1; i < (partsOfOrQuery.size() -1); i++)
    {
      firstPartWithFirstOrPart += OR_QUERY_SEP;
      firstPartWithFirstOrPart += partsOfOrQuery[i];
    }
  }

  #ifndef NDEBUG
  log << "! constructed this query \"" << firstPartWithFirstOrPart
      << "\", for this first part \"" << firstPartOfQuery
      << "\" and this last part \"" << lastPartOfQuery << "\"" << endl << flush;
  #endif

  // DocId k1_docs = DEFAULT_NOF_HITS_TO_COMPUTE;
  // WordId k2_words = DEFAULT_NOF_COMPLETIONS_TO_COMPUTE;
  QueryResult* result1 = NULL;
  QueryResult* result2 = NULL;

  try
  {
    processQuery(Query(firstPartWithFirstOrPart), result1);
    #ifndef NDEBUG
    cout << "! returned from topCompletionsAndHitsForQuery (1)" << endl;
    #endif
    assert(isInHistoryConst(firstPartWithFirstOrPart));
    setStatusOfHistoryEntry(firstPartWithFirstOrPart, QueryResult::FINISHED);
    setStatusOfHistoryEntry(firstPartWithFirstOrPart, QueryResult::FINISHED | QueryResult::IN_USE);
  }
  catch (Exception& e)
  {
    assert(!isInHistoryConst(firstPartWithFirstOrPart));
    assert(result1 == NULL);
    CS_RETHROW(e);
      // log << "! " << e.getFullErrorMessage() << endl;
      // CS_THROW(Exception::ERROR_PASSED_ON, e.getErrorMessage());
      // throw Exception(Exception::ERROR_PASSED_ON, os.str());
  }
  processQueryTimer.cont();
  assert(result1);
  #ifndef NDEBUG
  log << "! (in or processing) the first part had "
      << result1->_docIds.size() << " matches " << endl;
  #endif

  // 3. Process last part of OR query, e.g. for xyz q1|q2|q3 process xyz q3
  string firstPartWithLastOrPart = firstPartOfQuery.getQueryString();
  if (separatorMode != FULL)
  {
    firstPartWithLastOrPart += separator._separatorString;
  }
  else
  {
    assert(firstPartWithLastOrPart == string(""));
  }
  firstPartWithLastOrPart += partsOfOrQuery.back();

  try
  {
    processQuery(Query(firstPartWithLastOrPart), result2);
    // empty query is not in history (this whole history thing is completely sick)
    if (firstPartWithLastOrPart.empty() == false)
    {
      setStatusOfHistoryEntry(firstPartWithLastOrPart, QueryResult::FINISHED);
      setStatusOfHistoryEntry(firstPartWithLastOrPart, QueryResult::FINISHED | QueryResult::IN_USE);
    }
  }
  catch (Exception& e)
  {
    assert(!isInHistoryConst(firstPartWithLastOrPart));
    assert(result2 == NULL);
    CS_RETHROW(e);
  }
  processQueryTimer.cont();
  assert(result2);
  #ifndef NDEBUG
  log << "! (in or processing) the second part had "
      << result2->_docIds.size() << " matches " << endl;
  #endif


  // 4. Merge the two results from 2. and 3., e.g. for xyz q1|q2|q3 merge the
  // results for xzy q1|q2 and xyz q3 (merge = union, not intersect)
  log << IF_VERBOSITY_HIGHER
      << "! in processOrQuery: before merging"
      << "; result1 has " << result1->getNofPostings() << " postings"
      << "; result2 has " << result2->getNofPostings() << " postings"
      << endl;
  mergeResultsTimer.cont();
  if (result1->isLockedForWriting) CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "");
  if (result2->isLockedForWriting) CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "");
  if (result.isLockedForWriting) CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "");
  result.isLockedForWriting = true;
  result1->isLockedForReading = true; // only unlockd in clean up
  result2->isLockedForReading = true; // only unlockd in clean up
  // // DEBUG(hannah): OR of equal lists gave trailing zero postings in result list.
  // log << "! result1, result2, and result before merging:" << endl;
  // log << "! result1 address = " << (int)(result1);
  // log << "! result2 address = " << (int)(result2);
  // result1->show();
  // result2->show();
  // result.show();
  mergeTwoPostingLists(*result1, *result2, result); // , k1_docs, k2_words);
  // // DEBUG(hannah): OR of equal lists gave trailing zero postings in result list.
  // log << "! result1, result2, and result after merging:" << endl;
  // log << "! result1 address = " << (int)(result1);
  // log << "! result2 address = " << (int)(result2);
  // result1->show();
  // result2->show();
  // result.show();
  if (!result.isLockedForWriting) CS_THROW(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "");
  result.isLockedForWriting = false;
  assert(isInHistoryConst(firstPartWithFirstOrPart));
  assert(isInHistoryConst(firstPartWithLastOrPart));
  result._query = firstPartOfQuery.getQueryString() + 
                  separator.getSeparatorString() +
                  lastPartOfQuery.getQueryString();
  result._prefixCompleted = string(partsOfOrQuery.back()); // set to last prefix
  mergeResultsTimer.stop();
  assert(result._docIds.isSorted());
  result._docIds.markAsSorted(true);
  log << IF_VERBOSITY_HIGHER
      << "! in processOrQuery: after merging"
      << "; result has " << result.getNofPostings() << " postings"
      << endl;

  log << AT_END_OF_METHOD << endl;
} // end: processOrQuery


// _____________________________________________________________________________
//! Process join query, with last part of the form [q1#q2#...#qm]
template <unsigned char MODE>
void CompleterBase<MODE>::processJoinQuery
      (const QueryResult& resultFirstPart,
       const Query&       firstPartOfQuery,
       const Query&       lastPartOfQuery,
       const Separator&   separator,
             QueryResult& result)
{
  log << AT_BEGINNING_OF_METHOD
      << "; first part = \"" << firstPartOfQuery << "\""
      << "; last part = \"" << lastPartOfQuery  << "\""
      << "; separator = \"" << separator.getSeparatorString() << "\"" << endl;

  if (!(result._status & QueryResult::UNDER_CONSTRUCTION))
    CS_THROW(Exception::BAD_HISTORY_ENTRY, "(empty) result was not marked as QueryResult::UNDER_CONSTRUCTION");
  signed char separatorMode = separator._separatorIndex;
  assert(separatorMode != JOIN);
  assert(lastPartOfQuery.getQueryString().find(ENHANCED_QUERY_SEP) != string::npos);
  vector<string> partsOfSpecialQuery;

  // SPLIT SPECIAL BLOCK INTO 'NORMAL' QUERIES
  splitPartOfEnhancedQueryBlock(lastPartOfQuery, partsOfSpecialQuery);
  if (partsOfSpecialQuery.size() < 2)
  {
    log << "! ERROR: ONLY GOT " << partsOfSpecialQuery.size() << " PARTS WHEN SPLITTING ENHANCED PART" << endl << flush;
    throw Exception(Exception::ENHANCED_PART_TOO_SHORT,
              "in method CompleterBase::allMatchesForEnhancedPartAndCandidates(");
  }


  // get first part for query
  //!   xyz [q1#q2#q3]  ->  xyz [q1#q2]
  //! or:   xyz [q1#q2]  ->  xyz q1


  string firstPartWithFirstEnhancedPart = firstPartOfQuery.getQueryString();
  assert((separatorMode < (signed char) fixed_separators._separators.size()) || (separatorMode == FULL));

  if (separatorMode != FULL)
  {
    //                firstPartWithFirstEnhancedPart += separators._separators[separatorMode].first;
    firstPartWithFirstEnhancedPart += separator._separatorString;
  }
  else
  {
    assert(firstPartWithFirstEnhancedPart == string(""));
  }

  if (partsOfSpecialQuery.size() == 2)
  {
    firstPartWithFirstEnhancedPart += partsOfSpecialQuery[0];
  }
  else
  {
    firstPartWithFirstEnhancedPart += ENHANCED_QUERY_BEG;
    firstPartWithFirstEnhancedPart += partsOfSpecialQuery[0];
    for (unsigned int i=1; i < (partsOfSpecialQuery.size() -1); i++)
    {
      firstPartWithFirstEnhancedPart += ENHANCED_QUERY_SEP;
      firstPartWithFirstEnhancedPart += partsOfSpecialQuery[i];
    }

    firstPartWithFirstEnhancedPart += ENHANCED_QUERY_END;
  }
  #ifndef NDEBUG
  log << "! constructed this query \"" << firstPartWithFirstEnhancedPart <<"\", for this first part \"" << firstPartOfQuery << "\" and this last part \"" << lastPartOfQuery << "\"" << endl << flush;
  #endif


  // DocId k1_docs = DEFAULT_NOF_HITS_TO_COMPUTE;
  // WordId k2_words = DEFAULT_NOF_COMPLETIONS_TO_COMPUTE;
  QueryResult* result1 = NULL;
  QueryResult* result2 = NULL;
  //            Query queryPart = Query(partsOfSpecialQuery[0]);
  //            topCompletionsAndHitsForQuery(queryPart, newResult, k1_docs, k2_words);

  // GET RESULT FOR FIRST PART IN SPECIAL BLOCK
  processQuery(Query(firstPartWithFirstEnhancedPart), result1); // , k1_docs, k2_words);
  #ifndef NDEBUG
  cout << "! returned from topCompletionsAndHitsForQuery (3)" << endl;
  #endif
  setStatusOfHistoryEntry(Query(firstPartWithFirstEnhancedPart), QueryResult::FINISHED);
  setStatusOfHistoryEntry(Query(firstPartWithFirstEnhancedPart), QueryResult::FINISHED | QueryResult::IN_USE);
  processQueryTimer.cont();
  //            if (errorStatus) {processQueryTimer.stop(); return;}
  assert(result1);

  processQuery(Query(partsOfSpecialQuery.back()), result2); // , k1_docs, k2_words);
  #ifndef NDEBUG
  cout << "! returned from topCompletionsAndHitsForQuery (4)" << endl;
  #endif
  setStatusOfHistoryEntry(Query(partsOfSpecialQuery.back()), QueryResult::FINISHED);
  setStatusOfHistoryEntry(Query(partsOfSpecialQuery.back()), QueryResult::FINISHED | QueryResult::IN_USE);
  processQueryTimer.cont();
  assert(result2);

  intersectWordlistsTimer.cont();
//  QueryResult::intersectWordlists(*result1, *result2, result); // , useLinearWordlistIntersection);
  joinTwoPostingLists(*result1, *result2, result);
  intersectWordlistsTimer.stop();

  assert(result._docIds.isSorted());
  result._docIds.markAsSorted(true);

  log << AT_END_OF_METHOD << endl;

} // end: processJoinQuery


// _____________________________________________________________________________
//! Process basic query for given posting lists and prefix
/*
 *   Wrapper method that deals with all kinds of special cases: join queries, or
 *   queries, empty D, empty W, etc. In the typical case (CASE 3.3 in the source
 *   code), just translates prefix to word range and then calls
 *   processBasicQuery from HybCompleter or InvCompleter
 */
template <unsigned char MODE>
void CompleterBase<MODE>::processBasicQuery
                           (const QueryResult& inputList,
                            const Query&       firstPartOfQuery,
                            const Query&       lastPartOfQuery,
                            const Separator&   separator,
                                  QueryResult& result)
{
  #ifndef NDEBUG
  log << "! separatorMode in allMatchesForLastPartAndCandidates " << (int) separator._separatorIndex << endl << flush;
  #endif
  assert(inputList._status & QueryResult::FINISHED);
  assert( inputList._docIds.isFullList() || inputList._docIds.size() > 0 );
  assert( !(MODE & WITH_SCORES) || inputList._scores.size() == inputList._docIds.size() );
  assert( !(MODE & WITH_POS) || inputList._positions.size() == inputList._docIds.size() );

  assert((firstPartOfQuery.length() > 0) || (separator._separatorIndex == FULL));
  assert((separator._separatorIndex != FULL) || (firstPartOfQuery.getQueryString() == ""));

  // #ifndef NDEBUG
  // Query dummy1;
  // Separator dummy2;
  // log << "! XXX YYY ZZZ" << endl;
  // assert(!lastPartOfQuery.splitAtLastSeparator(&dummy1, &dummy1, &dummy2));
  // assert(separator._separatorIndex != JOIN); // 27Jul2006: JOIN was used temporarily as a sep mode
  // #endif

  // If the input list is empty (that is, already firstPartOfQuery gave no
  // hits), then the result is also empty and we can stop here.
  if (inputList.isEmpty())
  {
    assert(result.isEmpty());
    #ifndef NDEBUG
    log << "! empty candidates in allMatchesForLastPartAndCandidates " << endl << flush;
    #endif
    return;
  }

  // Check that the lengths of the doc id, word id, score, and position lists of
  // inputList make sense.
  assert( inputList._docIds.isFullList() || inputList._docIds.size() > 0 );
  assert( !(MODE & WITH_SCORES) || inputList._scores.size() == inputList._docIds.size() );
  assert( !(MODE & WITH_POS) || inputList._positions.size() == inputList._docIds.size() );

  // NEW(bast, 21Jan10): Maintain _lastBestMatchWordId. In CASE 3 (fuzzy
  // search) this will be set to the id of the word closest to the query word,
  // if any exist. In CASE 4 (simple prefix), this will be set to the id of the
  // exact query word, if that exists. In all other cases it will remain
  // NO_WORD_ID, and scoring is unaffected. If not NO_WORD_ID, the
  // BEST_MATCH_BONUS is added in HybCompleter::processBasicQuery (single query
  // word) and in CompleterBase::intersectTwoPostingListsNewTemplated (multiple
  // query words) for each posting that matches _lastBestMatchWordId.
  WordId _lastBestMatchWordId = -1;

  // CASE 1:  Last part is a join query (of the form [...#...#...])
  if (lastPartOfQuery.getQueryString().find(ENHANCED_QUERY_SEP) != string::npos)
  {
    #ifndef NDEBUG
    log << "! last part (" << lastPartOfQuery << ") is a special query in allMatchesForLastPartAndCandidates" << endl;
    #endif
    processJoinQuery(inputList,
                     firstPartOfQuery,
                     lastPartOfQuery,
                     separator,
                     result);
  }

  // CASE 2:  Last part is an or query (of the form ...|...|...)
  else if (lastPartOfQuery.getQueryString().find(OR_QUERY_SEP) != string::npos)
  {
    #ifndef NDEBUG
    log << "! last part (" << lastPartOfQuery << ") is a 'or' query in allMatchesForLastPartAndCandidates" << endl;
    #endif
    processOrQuery(inputList,
                   firstPartOfQuery,
                   lastPartOfQuery,
                   separator,
                   result);
  }

  else if (fuzzySearchEnabled && lastPartOfQuery.getLastCharacter() == '~')
  {
      processFuzzySearchQuery(inputList,
                              firstPartOfQuery,
                              lastPartOfQuery,
                              separator,
                              result);
  }
 
  // CASE 4:  Last part is a synonym search query (ending with a ^). Note: the
  // code for processSynonymSearchQuery is in CompleterBase.SynonymSearch.cpp.
  // CHANGE(hagn, 10Aug11): If synonymSearchEnabled = false then skip synonym-search
  else if (lastPartOfQuery.getLastCharacter() == '^')
  {
    if (synonymSearchEnabled)
    {
      processSynonymSearchQuery(inputList,
                                firstPartOfQuery,
                                lastPartOfQuery,
                                separator,
                                result);

    }
    else
    {
      log << "! NEW SYN: synonym search is not enabled and will be skipped!" << endl;
      Query newLastPartOfQuery = lastPartOfQuery;
      newLastPartOfQuery.removeLastCharacter();
      log << "! NEW SYN: replacing \"" << lastPartOfQuery.getQueryString() << "\""
          << " by \"" << newLastPartOfQuery.getQueryString() << "\"" << endl;
      processBasicQuery(inputList,
                        firstPartOfQuery,
                        newLastPartOfQuery,
                        separator,
                        result);
    }
  }

  // CASE 5a:  Last part is a simple prefix. Note that all the previous cases
  // eventually reduce to this one!
  //
  // NEW(hagn, 10Aug11): Implementation of server parameter --use-suffix-for-exact-query.
  // This part is important for a exact-query when the index is built like [...]<Suchwort>:<Anzeigewort>
  // Because a query "haus" does not match "haus:Haus" in the index.
  // So we must append ":*" for exact-query.

  // NEW (baumgari) 18Apr13: This is misplaced here, since it just makes sense,
  // in case there a just [...]<Suchwort>:<Anzeigewort> words. In case of an
  // hybrid index, which uses such words, but also normal words, normal words
  // cannot be found anymore. This should should be called in prefixToRange in
  // case elsewhise nothing is found! (Moved it there and modified it).
  /*
  else if (useSuffixForExactQuery && lastPartOfQuery.getLastCharacter() != '*')
  {
    Query newLastPartOfQuery = lastPartOfQuery;
    newLastPartOfQuery.append(wordPartSep + string("*"));
    log << "! NEW EXAKT: replacing \"" << lastPartOfQuery.getQueryString() << "\""
        << " by \"" << newLastPartOfQuery.getQueryString() << "\"" << endl;
    processBasicQuery(inputList,
                      firstPartOfQuery,
                      newLastPartOfQuery,
                      separator,
                      result);
  }
  */

  // CASE 5b:  Last part is a simple prefix. Note that all the previous cases
  // eventually reduce to this one!
  else
  {
    #ifndef NDEBUG
    log << "! last part (" << lastPartOfQuery << ") is a normal query in allMatchesForLastPartAndCandidates" << endl;
    #endif

    assert( inputList._docIds.isFullList() || inputList._docIds.size() > 0 );
    assert( !(MODE & WITH_SCORES) || inputList._scores.size() == inputList._docIds.size() );
    assert( !(MODE & WITH_POS) || inputList._positions.size() == inputList._docIds.size() );

    // Translate the prefix to the corresponding word range.
    result._query = firstPartOfQuery.getQueryString() + 
                    separator.getSeparatorString() +
                    lastPartOfQuery.getQueryString();
    result._prefixCompleted = lastPartOfQuery.getQueryString();
    bool notIntersectionMode;
    const WordRange wordRange = prefixToRange(lastPartOfQuery.getQueryString(), notIntersectionMode);

    // CASE 4.1:  Empty word range and hence no completions for last part and
    // it's not a NOT query. In that case the result is empty and we can stop
    // here.
    if (((lastPartOfQuery.length() == 0) || (wordRange.isEmptyRange()))&&(!notIntersectionMode))
    {
      #ifndef NDEBUG
      log << "! trivial case (empty wordrange/zero length of last part) in allMatchesForLastPartAndCandidates" << endl;
      #endif
      assert(result.isEmpty());
      return;
    }

    // CASE 4.2:  Empty word range and hence no completions for last part and it
    // is a NOT query.  In that case the result is just the same as for the
    // first part of the query, that is, we can just copy that.
    if ((wordRange.isEmptyRange()) && (notIntersectionMode))
    {
      #ifndef NDEBUG
      log << "! not intersection where nothing is removed (as last prefix was empty)" << endl;
      #endif
      assert(result.isEmpty()); // so far, but the result will now be copied
      result = inputList;
      return;
    }

    // CASE 4.3:  Non-empty word range. This is the typical case!
    externalTimer.cont();
    assert(!wordRange.isEmptyRange());
    assert(inputList._status & QueryResult::FINISHED);
    assert( inputList._docIds.isFullList() || inputList._docIds.size() > 0 );
    assert( !(MODE & WITH_SCORES) || inputList._scores.size() == inputList._docIds.size() );
    assert( !(MODE & WITH_POS) || inputList._positions.size() == inputList._docIds.size() );
    // Set _lastBestMatchWordId in case there is an exact match for the query
    // prefix.
    WordId firstWordIdInRange = wordRange.firstElement();
    Query lastPartOfQueryTmp = lastPartOfQuery;
    if (lastPartOfQueryTmp.getLastCharacter() == '*')
      lastPartOfQueryTmp.removeLastCharacter();
    if (lastPartOfQueryTmp.getQueryString() == (*_vocabulary)[firstWordIdInRange])
      _lastBestMatchWordId = firstWordIdInRange;
    processBasicQuery(inputList, wordRange, result, separator);
                      // listsForPrefixFromHistory);
    externalTimer.stop();
  }
}  // end: processBasicQuery


// _____________________________________________________________________________
//! Process query, by recursing on part preceding last separator
/*!
 *    Note: this is a complete rewrite of Ingmar's code. The old version is
 *    still there (with a slightly different signature)
 *
 *    The following steps are performed:
 *
 *    1. If query is in history                => fetch result
 *    2. If first and last part are in history => fetch results and intersect them
 *    3. If prefix of query is in history      => fetch result and filter
 *    4.
 */
/*
template <unsigned char MODE>
void CompleterBase<MODE>::processQueryRecursively
      (const Query&           query,
       const QueryParameters& queryParameters,
             QueryResult*&    result)
{
  CS_THROW(Exception::NOT_YET_IMPLEMENTED, "");
}
*/



//! Process complex query from left to right, via a basic query for each separator
/*!
 *    For example, for the query a* b* c*, the following three basic queries are
 *    processed in the given order
 *
 *    1. input list = all postings, word range = a*
 *    2. input list = result of 1., word range = b*
 *    3. input list = result of 2., word range = c*
 *
 *    The method splits the query on the last separator (the one between b* and
 *    c* in the example), and calls itself recursively on the first part.
 *    Recursion is also used for the various kinds of filtering. Filtering
 *    attempts to compute the query from results in the history (i.e., already
 *    computed earlier) in cases where this is faster then computing the query
 *    from scratch.
 *
 *    TODO: describe the three kinds of filtering currently used.
 *
 *    TODO: when is this allowed to be called with non-NULL result?
 */
/*
template <unsigned char MODE>
void CompleterBase<MODE>::processComplexQuery
      (Query&           query,
       QueryParameters& queryParameters,
       QueryResult*&    result);
{
  // Table of contents
  //
  // 1. Query is in history but not yet finished      -> wait a little
  // 2. Query is in history and finished              -> fetch the result
  // 3  Query can be computed from results in history -> do that (filtering)
  // 4. Query neither in history nor can be filtered  -> compute from scratch
  // 5. Result checking

  log << IF_VERBOSITY_HIGH
      << "! At the beginning of processComplexQuery, query is \"" << query << "\"" << endl;

  //! TODO: what is this asserting?
  assert((finalResult == NULL)||(finalResult->nofTotalHits > finalResult->_topDocIds.size() ));

  // get status of query in history
  int

  // 1. Query is in history but not yet finished -> wait a little
  //
  //    TODO: this is a preliminary hack to see how it works. Make it more
  //    principled and parameterized.
  //
  unsigned int usecsSlept = 0;
  unsigned int usecsDelta = 10*1000;
  unsigned int usecsWaitForQueryInHistory = 2*1000*1000;
  while (getStatusOfHistoryEntry(query.getQueryString()) & QueryResult::UNDER_CONSTRUCTION)
  {
    usleep(usecsDelta);
    usecsSlept += usecsDelta;
    if (usecsSlept >= usecsWaitForQueryInHistory) break;
  }
  if (usecsSlept > 0)
  {
    log << EMPH_ON << "! NEW: Waited " << usecsSlept / 1000
        << " milliseconds for history entry from other thread"
        << (getStatusOfHistoryEntry(query.getQueryString()) & QueryResult::FINISHED
             ? " -> IS FINISHED NOW!" : " -> STILL UNDER CONSTRUCTION")
        << EMPH_OFF << endl;
  }


  // 2. Query is in history and finished -> fetch the result
  //
  //   TODO: explain a bit
  //
  if ((getStatusOfHistoryEntry(query.getQueryString()) & QueryResult::FINISHED)
        && (isInHistory(query.getQueryString(), finalResult) )) { }
*/


// _____________________________________________________________________________
//! Process complex query from left to right, via a basic query for each separator
/*!
 *    For example, for the query a* b* c*, the following three basic queries are
 *    processed in the given order
 *
 *    1. input list = all postings, word range = a*
 *    2. input list = result of 1., word range = b*
 *    3. input list = result of 2., word range = c*
 *
 *    The method splits the query on the last separator (the one between b* and
 *    c* in the example), and calls itself recursively on the first part.
 *    Recursion is also used for the various kinds of filtering. Filtering
 *    attempts to compute the query from results in the history (i.e., already
 *    computed earlier) in cases where this is faster then computing the query
 *    from scratch.
 *
 *    TODO: describe the three kinds of filtering currently used.
 *
 *    TODO: when is this allowed to be called with non-NULL result?
 */
template <unsigned char MODE>
void CompleterBase<MODE>::processComplexQuery
      (const Query&        query,
             QueryResult*& result)
{
  log << AT_BEGINNING_OF_METHOD << "; query is \"" << query << "\"" << endl;

  // TABLE OF CONTENTS
  //
  // 0.  Timers + a few checks
  // 1.  CASE: Query is in history but not yet finished    -> wait a little
  // 2a. CASE: Query is in history and finished            -> fetch the result
  // 2b. CASE: Query's first and last part are in history  -> intersect results
  // 3.  CASE: Query is not in history                     -> compute it now
  // 4.  Timers + result checking
  //

  // Copied and pasted from CompletionServer.cpp (and commented out there)
  // WAS A BUG: inconsistent with extractFromRequestStringHttp, where
  // nofTopHitsToCompute is set, which is then used in computeTopHits. Same for
  // nofTopCompletionsToCompute.
    // DocId k1_docs = _queryParameters.firstHitToSend + _queryParameters.nofHitsToSend > DEFAULT_NOF_HITS_TO_COMPUTE
    //                   ? MAX_DOC_ID : DEFAULT_NOF_HITS_TO_COMPUTE;
    // WordId k2_words = _queryParameters.nofCompletionsToSend > (unsigned int) DEFAULT_NOF_COMPLETIONS_TO_COMPUTE
    //                    ? MAX_WORD_ID : DEFAULT_NOF_COMPLETIONS_TO_COMPUTE;

  #ifndef NDEBUG
  log << "! query at beginning of processQueryRecursively : " << query << endl;
  #endif
  assert((result == NULL)||(result->nofTotalHits > result->_topDocIds.size() ));

  //! CHECK IF QUERY IS TOO SHORT; does not allow in*, but and$ is ok.
  if (query.length() < MIN_QUERY_LENGTH) CS_THROW(Exception::QUERY_TOO_SHORT, "");

  // TODO: what do these timers measure?
  broadHistoryTimer.cont();
  broadHistoryTimer0.cont();

  // CASE 0: QUERY IS IN HISTORY BUT NOT YET FINISHED -> wait a little!
  //
  //   NEW 14Oct07: preliminary hack, in order to see whether that
  //   works in principle; TODO: do it properly asap
  //
  unsigned int usecsSlept = 0;
  unsigned int usecsDelta = 10*1000;
  unsigned int usecsWaitForQueryInHistory = 1*1000*1000; // TODO: should become a parameter
  while (getStatusOfHistoryEntry(query) & QueryResult::UNDER_CONSTRUCTION)
  {
    // NEW(bast, 17Mar17): in single-threaded mode, if a query is in the history
    // and under construction, this can only be due a previous error in the
    // previous processing of that query. The right action is then to remove
    // that query from the history and proceed as if it weren't there.
    if (runMultithreaded == false) {
      log << "! NEW(17Mar17): Removing query with flag UNDER_CONSTRUCTION from"
          << " history (should not happen in single-threaded mode); query is:"
          << " \"" << query.getQueryString() << "\"" << endl << flush;
      removeFromHistory(query);
      break;
    }
    usleep(usecsDelta);
    usecsSlept += usecsDelta;
    if (usecsSlept >= usecsWaitForQueryInHistory) break;
  }
  if (usecsSlept > 0)
  {
    log << EMPH_ON << "! NEW: Waited " << usecsSlept / 1000
        << " milliseconds for history entry from other thread"
        << (getStatusOfHistoryEntry(query) & QueryResult::FINISHED
             ? " -> IS FINISHED NOW!" : " -> STILL UNDER CONSTRUCTION")
        << EMPH_OFF << endl;
  }

  //
  // CASE 1: QUERY IS IN HISTORY AND FINISHED
  //
  //   Just reuse it. If necessary, redo the top-k computation.
  //
  //   TODO: given that nothing much happens here, the code should be much shorter.
  //
  if ( (getStatusOfHistoryEntry(query) & QueryResult::FINISHED) && (isInHistory(query, result)) )
  {
    bool wasUsedAlready = false;
    if (getStatusOfHistoryEntry(query) & QueryResult::IN_USE) { wasUsedAlready = true;}
    assert(isInHistoryConst(query));
    setStatusOfHistoryEntry(query, QueryResult::FINISHED | QueryResult::IN_USE);
    assert(isInHistoryConst(query));
    #ifndef NDEBUG
    log << "! query found in history " << query.getQueryString() << endl << flush;
    assert(isInHistory(query));
    #endif
    assert(result != NULL);
    assert(getStatusOfHistoryEntry(query) & QueryResult::FINISHED);
    ++nofQueriesFromHistory;
    result->wasInHistory = true;
    result->setHowResultWasComputed(QueryResult::FROM_HISTORY);

    //
    // CASE 1.1: need to recompute top-k hits and/or completions
    //
    //   This can happen for one or more changes in the query parameters
    //   compared to those used when the result was computed and stored in the
    //   history
    //
    //   - larger value for nofTopHitsToCompute
    //   - larger value for nofTopCompletionsToCompute
    //   - different value of howToRankDocs
    //   - different value of howToRankWords
    //   - different value of sortOrderDocs
    //   - different value of sortOrderWords
    //   - different value of fuzzyDamping
    //
    CS_ASSERT(getStatusOfHistoryEntry(query) & QueryResult::FINISHED);
    DocId  k1_docs  = _queryParameters.nofTopHitsToCompute;
    WordId k2_words = _queryParameters.nofTopCompletionsToCompute;
    bool needToRecomputeHits        =    result->nofTotalHits > (DocId)(result->_topDocIds.size())
                                      && k1_docs              > (DocId)(result->_topDocIds.size());
    bool needToRecomputeCompletions =    result->nofTotalCompletions > (WordId)(result->_topWordIds.size())
                                      && k2_words                    > (WordId)(result->_topWordIds.size());
    bool needToRerankHits           =    _queryParameters.howToRankDocs                   != result->_queryParameters.howToRankDocs
                                      || _queryParameters.fuzzyDamping                    != result->_queryParameters.fuzzyDamping
                                      || _queryParameters.docScoreAggSameCompletion       != result->_queryParameters.docScoreAggSameCompletion
                                      || _queryParameters.docScoreAggDifferentCompletions != result->_queryParameters.docScoreAggDifferentCompletions;
    bool needToRerankCompletions    =    _queryParameters.howToRankWords                  != result->_queryParameters.howToRankWords
                                      || _queryParameters.fuzzyDamping                    != result->_queryParameters.fuzzyDamping
                                      || _queryParameters.wordScoreAggSameDocument        != result->_queryParameters.wordScoreAggSameDocument
                                      || _queryParameters.wordScoreAggDifferentDocuments  != result->_queryParameters.wordScoreAggDifferentDocuments;
    bool needToResortHits           =    _queryParameters.sortOrderDocs                   != result->_queryParameters.sortOrderDocs;
    bool needToResortCompletions    =    _queryParameters.sortOrderWords                  != result->_queryParameters.sortOrderWords;
    if (    needToRecomputeHits
         || needToRecomputeCompletions
         || needToRerankHits
         || needToRerankCompletions
         || needToResortHits
         || needToResortCompletions)
    {
      log << IF_VERBOSITY_HIGH
          << "! needToRecomputeHits = " << needToRecomputeHits
          << ", needToRecomputeCompletions = " << needToRecomputeCompletions
          << ", needToRerankHits = " << needToRerankHits
          << ", needToRerankCompletions = " << needToRerankCompletions
          << ", needToResortHits = " << needToResortHits
          << ", needToResortCompletions = " << needToResortCompletions
          << endl;
      if (needToRecomputeHits)
        log << IF_VERBOSITY_HIGH
            << "! needToRecomputeHits = 1"
            << ": result->nofTotalHits = " << result->nofTotalHits
            << ", result->_topDocIds.size() = " << result->_topDocIds.size()
            << ", k1_docs = " << k1_docs
            << endl;
      if (wasUsedAlready)
        CS_THROW(Exception::HISTORY_ENTRY_CONFLICT, "cannot recompute top-k at the moment as the result is currently in use");
      assert(isInHistoryConst(query));
      if(getStatusOfHistoryEntry(query)== QueryResult::UNDER_CONSTRUCTION)
      {
        log << endl << endl << endl << " ****** WAS ALREAY QueryResult::UNDER_CONSTRUCTION (1) *************" << endl << endl << endl;
      }
      setStatusOfHistoryEntry(query, QueryResult::UNDER_CONSTRUCTION);
      log << IF_VERBOSITY_HIGH << "! recomputing top hits and completions for result in history" << endl;
      #ifndef NDEBUG
      log << "! recomputing hits with k1_docs = " << k1_docs << " and k2_words = " << k2_words << endl << flush;
      log << "! nof matching docs = " << result->_topDocIds.size() << " and nof matching words = " << result->_topWordIds.size() << endl << flush;
      log << "! nof total matching docs = " << result->nofTotalHits << " and nof total matching words = " << result->nofTotalCompletions  << endl << flush;
      #endif
      assert((k1_docs <= (DocId) result->_topDocIds.size()) || (k1_docs == MAX_DOC_ID) || (k2_words == MAX_WORD_ID));
      assert((k2_words <= (WordId) result->_topWordIds.size()) || (k2_words == MAX_WORD_ID) || (k1_docs == MAX_DOC_ID));
      broadHistoryTimer0.stop();
      broadHistoryTimer.stop();
      assert(!(result->_status & QueryResult::FINISHED));
      if (result->isLockedForReading)
      CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "in method topMatchesFromAllMatches");
      if (result->isLockedForWriting)
      CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "in method topMatchesFromAllMatches");
      result->isLockedForWriting = true;
      computeTopHitsAndCompletions(*result);
      /*
       topMatchesFromAllMatches(*result,
       WITHIN_DOC_AGGREGATION, BETWEEN_DOCS_AGGREGATION,
       WITHIN_COMPLETION_AGGREGATION, BETWEEN_COMPLETIONS_AGGREGATION,
       k1_docs, k2_words);
       */
      if (!result->isLockedForWriting) throw Exception(
          Exception::RESULT_NOT_LOCKED_FOR_WRITING,
          "in method topMatchesFromAllMatches");
      result->isLockedForWriting = false;
      broadHistoryTimer.cont();
      broadHistoryTimer0.cont();
      assert( ( result->_docIds.size() == 0) || ( result->_topWordScores.size() > 0  ));

      #ifndef NDEBUG
      log << "! computed " << result->_topDocIds.size() << " hits out of " << result->nofTotalHits << " total hits" << endl << flush;
      log << "! computed " << result->_topWordIds.size() << " completions out of " << result->nofTotalCompletions << " total copmletions" << endl << flush;
      for (unsigned long i =0 ; i<result->_topWordDocCounts.size() ; i++)
      {
        assert (result->_topWordDocCounts.operator[](i) <=  result->_topWordOccCounts.operator[](i));
      }
      #endif
      assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
      assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
      // create the strings to display
      if (result->isLockedForReading)
        throw Exception(Exception::RESULT_LOCKED_FOR_READING, "before setting completions");
      if (result->isLockedForWriting)
        throw Exception(Exception::RESULT_LOCKED_FOR_WRITING, "before setting completions");
      result->isLockedForWriting = true;
      result->setCompletions(result->_topWordScores,
                             result->_topWordDocCounts,
                             result->_topWordOccCounts,
                             *_vocabulary);
      if (!result->isLockedForWriting)
        throw Exception(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after setting completions");
      result->isLockedForWriting = false;
      assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
      assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
      assert(result->_docIds.isSorted());
      result->_docIds.markAsSorted(true); // TODO: obsolete, but leave here for now for some checks

      assert(result == isInHistory(query));
      if (result->isLockedForReading)
       throw Exception(Exception::RESULT_LOCKED_FOR_READING, "before freeExtraSpace");
      if (result->isLockedForWriting)
       throw Exception(Exception::RESULT_LOCKED_FOR_WRITING, "before freeExtraSpace");
      result->isLockedForWriting = true;
      result->freeExtraSpace();
      if (!result->isLockedForWriting)
        throw Exception(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after freeExtraSpace");
      result->isLockedForWriting = false;
      // CHANGE(hagn, 28Jan11):
      //history.finalizeSize(query.getQueryString());
      finalizeSizeOfHistory(query);
      result->setHowResultWasComputed(QueryResult::FROM_HISTORY_TOPK_AGAIN);
      assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
      assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
    }
    //
    // CASE 1.1: have to recompute more matches
    //
    //  *** END
    //

    // Done
    assert(isInHistoryConst(query));
    setStatusOfHistoryEntry(query, QueryResult::FINISHED);
    setStatusOfHistoryEntry(query, QueryResult::FINISHED | QueryResult::IN_USE);
    if (MODE & WITH_POS) assert(result->_docIds.size() == 0 || result->_docIds.isMarkedSorted());
    assert(getStatusOfHistoryEntry(query) & QueryResult::FINISHED);
    broadHistoryTimer0.stop();
    broadHistoryTimer.stop();
    return;
  }
  //
  // CASE 1: QUERY IS IN HISTORY AND FINISHED
  //
  //  *** END
  //


  //
  // CASE 2: QUERY NOT IN HISTORY (not even being computed)
  //
  if (getStatusOfHistoryEntry(query.getQueryString()) == QueryResult::DOES_NOT_EXIST)
  {
    #ifndef NDEBUG
    log << "! Did not find query in history " << query.getQueryString() << endl;
    assert(result == NULL);
    assert(!isInHistoryConst(query));
    assert(result == NULL);
    assert(!isInHistoryConst(query));
    log << "! ... and it really isn't " << endl;
    #endif

    broadHistoryTimer.cont();

    // IN ALL OTHER CASES THE RESULT WAS NOT ALREADY IN THE HISTORY
    // ADD EMPTY QUERY RESULT TO HISTORY
    #ifndef NDEBUG
    log << "! adding query : " << query.getQueryString()  << " to history " << endl;
    #endif

    addToHistory(query, QueryResult()); // add an empty result to history: ONLY DONE HERE!
    assert(getStatusOfHistoryEntry(query)==QueryResult::UNDER_CONSTRUCTION);
    assert(result == NULL);

    isInHistory(query, result); // sets result pointers to this empty result
    assert(isInHistory(query));
    assert( result != NULL );

    result->wasInHistory = false;

    // INITIALIZE BUFFER (i.e., reserve a 'standard' amount of space for intersections)
    broadHistoryTimer.stop();
    assert(result->_docIds.size() == 0);
    // TODO: What kind of error is this? Throw appropriate exception
    if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
    {
      log << " ******************** ERROR ********** " << endl;
      exit(1);
    }
    reserveBuffersAndResetPointers(*result);
    broadHistoryTimer.cont();

    // SPLIT QUERY INTO TWO PARTS
    Query firstPart, lastPart;
    //      unsigned char sepIndex = 0;
    Separator splitSeparator;
    const bool hasManyParts = query.splitAtLastSeparator(&firstPart, &lastPart, &splitSeparator);
    signed char sepIndex = splitSeparator._separatorIndex;
    if (sepIndex == NEAR) {
      splitSeparator.setIntersectionWindow(_queryParameters.neighbourhoodStart,
                                           _queryParameters.neighbourhoodEnd);
    }


    log << IF_VERBOSITY_HIGHER
        << "! split query in case: not in history; separator is " << splitSeparator.infoString() << endl;

    // RETURN WITH ERROR IF STRANGE THINGS HAPPENED DURING SPLIT OF QUERY
    if (sepIndex == (signed char) -1) CS_THROW(Exception::CONTAINS_INVALID_SEPARATOR,
                                             string("\"") + splitSeparator._separatorString + string("\"") );
    if (lastPart.getQueryString() == "*") CS_THROW(Exception::SINGLE_STAR_NOT_ALLOWED, "");

    #ifndef NDEBUG
    if (hasManyParts) { log << "! query has many parts " << endl; }
    else  { log << "! query has only one part" << endl; }
    log << "! first part of query (" << query << ") in processComplexQuery: \"" << firstPart << "\"" << endl;
    log << "! last part of query in processComplexQuery: \"" << lastPart << "\"" << endl;
    log << "! sepIndex was " << (signed int) sepIndex << endl;
    log << " separator was: "; splitSeparator.show();
    log << " or directly : xx" << splitSeparator._separatorString << "xx" << endl;
    #endif

    assert( (!hasManyParts && (lastPart.getQueryString() == query.getQueryString()) )
               || (hasManyParts && (query.length() > lastPart.length())
               && (query.length() > firstPart.length() )));


    //
    // CASE 2.1: "NORMAL" FILTERING (check if any prefix of last part is in history)
    //
    //   Query        : scheduling algorithm*
    //   filter from  : scheduling algo*
    //
    broadHistoryTimer1.cont();
    bool resultFoundByFiltering = false;
    unsigned int lengthLastPart = lastPart.length();

    // Compute the word range pertaining to the last prefix.
    bool notIntersectionMode;
    const WordRange wordRange = prefixToRange(lastPart.getQueryString(), notIntersectionMode);

    if (_queryParameters.useFiltering == true
         && lastPart.getQueryString().find(NOT_QUERY_SEP)      == string::npos
         && lastPart.getQueryString().find(ENHANCED_QUERY_SEP) == string::npos
         && lastPart.getQueryString().find(OR_QUERY_SEP)       == string::npos
         && splitSeparator.getOutputMode()                     == Separator::OUTPUT_MATCHES
         && wordRange.isEmptyRange()                           == false)
    {
      QueryResult* resultForFiltering = NULL;
      for (signed short i = lengthLastPart - 2; i > 0 ; --i)
      {
        string queryPrefix = query.getQueryString().substr(0, query.length() - lengthLastPart + i);
        assert(resultForFiltering == NULL);
        string queryToCheckInHistory = queryPrefix + "*";
        #ifndef NDEBUG
        log << "! search for this query in history (for filtering): " << queryToCheckInHistory << endl;
        #endif

        // CASE: prefix of last part *found* in history
        if ( (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)
                && isInHistory(queryToCheckInHistory, resultForFiltering)
                && (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED))
        {
          assert(isInHistoryConst(queryToCheckInHistory));
          setStatusOfHistoryEntry(queryToCheckInHistory, QueryResult::FINISHED | QueryResult::IN_USE);
          ++nofQueriesByFiltering;
          assert(resultForFiltering != NULL);
          result->resultWasFilteredFrom = queryToCheckInHistory;
          assert(resultForFiltering->_docIds.size() == resultForFiltering->_wordIdsOriginal.size());
          assert(resultForFiltering->_wordIdsOriginal.size() == resultForFiltering->_wordIdsMapped.size());
          assert( resultForFiltering->_docIds.isSorted() );
          #ifndef NDEBUG
          log << "! using this Result for filtering:" << queryToCheckInHistory << endl;
          #endif
          assert((  resultForFiltering->_topCompletions.size() > 0 ) || ( resultForFiltering->_docIds.size() == 0));
          assert((  resultForFiltering->_topCompletions.size() == 0 ) || ( resultForFiltering->_docIds.size() > 0));
          assert((resultForFiltering->_docIds.size() == 0) || ( resultForFiltering->_topDocScores.size() > 0 ));
          assert((!(MODE & WITH_POS)) || resultForFiltering->_docIds.isMarkedSorted()
              || resultForFiltering->_docIds.size() == 0);
          broadHistoryTimer1.stop();
          broadHistoryTimer2.cont();
          if (resultForFiltering->isLockedForWriting)
            CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before copy and filter");
          resultForFiltering->isLockedForReading = true; // only unlockd in clean up
          if (result->isLockedForReading)
            CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "before copy and filter");
          if (result->isLockedForWriting)
            CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before copy and filter");
          result->isLockedForWriting = true;
          copyAndFilter(result->_wordIdsOriginal,
                        resultForFiltering->_docIds,
                        resultForFiltering->_wordIdsOriginal,
                        resultForFiltering->_positions,
                        resultForFiltering->_scores,
                        wordRange.firstElement(), wordRange.lastElement() ,
                        result->_docIds,
                        result->_positions,
                        result->_scores);
          if (!result->isLockedForWriting)
            CS_THROW(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after copy and filter");
          result->isLockedForWriting = false;
          broadHistoryTimer2.stop();
          broadHistoryTimer1.cont();
          assert(result->_docIds.isSorted());
          result->_docIds.markAsSorted(true); // TODO: obsolete: leave here for now for some checks
          assert(!notIntersectionMode || checkWordlistBuffer(wordRange));
          assert( &(result->_topWordIds) == _wordlistUnionBuffer);

          resultFoundByFiltering = true;
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_FILTERED_1);
          assert(isInHistoryConst(queryToCheckInHistory));
          ///////////////                  setStatusOfHistoryEntry(queryToCheckInHistory,QueryResult::FINISHED);
          break;
        }
        // END CASE: prefix of last part *found* in history
        assert(resultForFiltering == NULL);
      }
      // END LOOP over all prefixes of last part of query (candidates to filter from)
      #ifndef NDEBUG
      log << "! end of filtering " << endl;
      #endif
      broadHistoryTimer1.stop();
    }
    //
    // CASE 2.2: "NORMAL" FILTERING (check if any prefix of last part is in history)
    //
    //   *** END
    //


    broadHistoryTimer3.cont();
    if (hasManyParts
         && _queryParameters.useFiltering == true
         && lastPart .getQueryString().find(NOT_QUERY_SEP)      == string::npos
         && firstPart.getQueryString().find(ENHANCED_QUERY_SEP) == string::npos
         && lastPart .getQueryString().find(OR_QUERY_SEP)       == string::npos
         && splitSeparator.getOutputMode()                      == Separator::OUTPUT_MATCHES
         && wordRange.isEmptyRange()                            == false
         && resultFoundByFiltering                              == false
         && fuzzySearchEnabled                                  == false)
    {
      //
      // CASE 2.3: "ADVANCED" FILTERING (e.g. filter schedul* venue:* from sched* venue:*)
      //
      //   Query        : scheduling..algorithm*
      //   filter from  : schedul*..algorithm*
      //   by intersecting result("scheduling") with result("schedul* algorithm*") with separator ..
      //
      //   This can save a lot of time if the last part has many matches. For
      //   example, computing the query "schedul* venue:*" from scratch will take
      //   some time because "venue:*" has many matches. But if we have already
      //   "sched* venue:*", which has a much shorter result than "venue:*", we
      //   can filter from that.
      //
      //   Old explanations by Ingmar:
      //
      //   b: if query has many parts (approxim.algorith) and separator mode
      //   check if last part of first part is in history (approxi.algorith, approx.algorith, appro.algorith, appr.algorith, app.algorith ...)
      //   if we find appr.algori (= R1), intersect result for appr.algorith with result for approxim (= R2)  =>  (approxim).(appr.algorith)
      //   (R2).(R1)
      //
      //   ALSO WORKS WITH XML SEPARATOR !!!!!
      //   b: if query has many parts (title=appr) and separator mode is NOT xml mode
      //   check if last part of first part is in history (titl=appr, tit=appr, ti=appr, t=appr)
      //   if we find tit=appr, intersect result for tit=appr with result for title (separator is space)
      //   (title)=(tit=appr)
      //
      QueryResult* result1ForAdvancedFiltering = NULL;
      QueryResult* result2ForAdvancedFiltering = NULL;
      Query firstPartOfFirstPart;
      Query lastPartOfFirstPart;
      Separator splitSeparator2;
      firstPart.splitAtLastSeparator(&firstPartOfFirstPart, &lastPartOfFirstPart, &splitSeparator2);
      log << IF_VERBOSITY_HIGHER
          << "! split query in case: advanced filtering I; separator is " << splitSeparator2.infoString() << endl;
      signed char sepIndexFirstPart = splitSeparator2._separatorIndex;
      if (sepIndexFirstPart == NEAR) {
        splitSeparator2.setIntersectionWindow(_queryParameters.neighbourhoodStart,
                                              _queryParameters.neighbourhoodEnd);
      }

      #ifndef NDEBUG
      log << "firstPartOfFirstPart: " << firstPartOfFirstPart << ", lastPartofFirstPart : " << lastPartOfFirstPart << endl;
      log << "splitSeparator Index: \"" << splitSeparator2._separatorIndex << "\"" << endl;
      log << "splitSeparator separator String: \"" << splitSeparator2._separatorString << "\"" << endl;
      #endif
      if (sepIndexFirstPart == (signed char) -1) CS_THROW(Exception::CONTAINS_INVALID_SEPARATOR, "");
      if (lastPartOfFirstPart.getQueryString() == "*") CS_THROW(Exception::SINGLE_STAR_NOT_ALLOWED, "");

      // LOOP: OVER LAST BUT ONE PREFIX (cutting off letter by letter and checking in history)
      for (signed short i = lastPartOfFirstPart.length() - 2; i > 0; --i)
      {
        assert(!resultFoundByFiltering);
        string queryPrefix = firstPart.getQueryString().substr(0, firstPart.length() - lastPartOfFirstPart.length() + i);

        // If OR separator encountered, must not proceed further with advanced
        // filtering (was a bug, fixed 11Feb07)
        //
        //   For example, the following series of queries used to fail on DBLP:
        //
        //   ESA author:*
        //   ESA|ICALP author:*
        //
        //   because the result for the second query was wrongly
        //   advanced-filtered from the result for the first query
        //   (with empty history the second query worked alright)
        //
        if (queryPrefix.length() > 0 && queryPrefix[queryPrefix.length() - 1] == OR_QUERY_SEP)
        {
          log << IF_VERBOSITY_HIGH
              << "! encountered OR separator, must not try advanced filtering beyond here (was a bug)" << endl;
          break;
        }

        assert(result1ForAdvancedFiltering == NULL);
        string queryToCheckInHistory = queryPrefix + "*" + splitSeparator._separatorString + lastPart.getQueryString();
        log << IF_VERBOSITY_HIGHER
            << "! advanced filtering, check this query in history: \"" << queryToCheckInHistory << "\"" << endl;
        // log << "! queryPrefix: " << queryPrefix
        //    << ", splitSeparator._separatorString :" << splitSeparator._separatorString
        //    << ", last Part: " << lastPart << endl;

        // CASE: found query in history (for advanced filtering)
        if ((getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)
          && isInHistory(queryToCheckInHistory, result1ForAdvancedFiltering))
        {
          if (!result1ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
          assert(isInHistoryConst(queryToCheckInHistory));
          setStatusOfHistoryEntry(queryToCheckInHistory, QueryResult::FINISHED | QueryResult::IN_USE);
          assert(result1ForAdvancedFiltering);
          log << IF_VERBOSITY_HIGHER
              << "! advanced filtering, found this query in history: \"" << queryToCheckInHistory << "\"" << endl;
          broadHistoryTimer.stop();
          broadHistoryTimer3.stop();
          try {
            processComplexQuery(firstPart, result2ForAdvancedFiltering);
                                // k1_docs, k2_words, filterResults, useLinearWordlistIntersection);
            #ifndef NDEBUG
            cout << "! returned from recursive call (2)" << endl;
            #endif
            assert(isInHistoryConst(firstPart));
            setStatusOfHistoryEntry(firstPart, QueryResult::FINISHED);
            setStatusOfHistoryEntry(firstPart, QueryResult::FINISHED | QueryResult::IN_USE);
          }
          catch (Exception& e)
          {
            log << IF_VERBOSITY_HIGH
                << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
                << "\"" << query << "\" from history" << endl;
            if ((e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT) && (getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION))
            {
              assert(getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION);
            }
            result2ForAdvancedFiltering = NULL;
            CS_RETHROW(e);
              // log << "! " << e.getFullErrorMessage() << endl;
              // CS_THROW(Exception::ERROR_PASSED_ON, e.getErrorMessage());
          }
          if (!result2ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "in result from advanced filtering");
          broadHistoryTimer.cont();
          broadHistoryTimer3.cont();
          assert(result2ForAdvancedFiltering);

          WordRange wordRange(-1,1); // infinite range

          // TODO: ONCE INTERSECT AND UNITE ARE CHANGED, THIS CAN BE REMOVED
          broadHistoryTimer3.stop();
          broadHistoryTimer.stop();
          assert(result->_docIds.size() == 0);

          // TODO: What kind of error is this? Throw appropriate exception
          if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
          {
            CS_THROW(Exception::OTHER, "something bad happened with result during advanced filtering");
            log << " ******************** ERROR ********** " << endl;
            exit(1);
          }
          reserveBuffersAndResetPointers(*result);
          broadHistoryTimer.cont();
          broadHistoryTimer3.cont();
          //                  clearBuffer(); // uncommented 05Jun07
          if ((!result2ForAdvancedFiltering->isEmpty()) && (!result1ForAdvancedFiltering->isEmpty() ))
          {
            broadHistoryTimer.stop();
            broadHistoryTimer3.stop();
            assert((!(MODE & WITH_SCORES)) || (result2ForAdvancedFiltering->_scores.size() == result2ForAdvancedFiltering->_docIds.size()));
            log << IF_VERBOSITY_HIGH << "! new intersect called from case \"advanced filtering I\"" << endl;
            intersectTwoPostingLists
             (*result2ForAdvancedFiltering,
              *result1ForAdvancedFiltering,
              *result,
              splitSeparator,
              _queryParameters.docScoreAggDifferentQueryParts,
              wordRange);
            broadHistoryTimer.cont();
            broadHistoryTimer3.cont();
          }
          assert(result->_docIds.isSorted());
          result->_docIds.markAsSorted(true);

          resultFoundByFiltering = true;
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_FILTERED_2);
          // NEW 19Sep06 (Holger): important for one-line summary (was left empty before, probably an oversight, was it?)
          result->resultWasFilteredFrom = queryToCheckInHistory;
          assert(isInHistoryConst(queryToCheckInHistory));
          assert(isInHistoryConst(firstPart));
          break;
        }
        // END Case: found query in history (for advanced filtering)

        assert(!(isInHistoryConst(queryToCheckInHistory) && (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)));
        assert(result1ForAdvancedFiltering == NULL);
      }
      // end LOOP: OVER LAST BUT ONE PREFIX (cutting off letter by letter and checking in history)
      broadHistoryTimer3.stop();
      broadHistoryTimer4.cont();
      broadHistoryTimer.cont();
      //
      // CASE 2.3: "ADVANCED" FILTERING (e.g. filter schedul* venue:* from sched* venue:*)
      //
      //   *** END
      //


      //
      // CASE 2.4: "ADVANCED" FILTERING II (e.g., filter approx*..algo* venue:* from approx venue:*)
      //
      //   Query       : approx*..algo* venue:*
      //   filter from : approx* venue:*
      //   by intersecting result(approx*..algo*) with result(approx venue:*)
      //
      //   TODO: explain what exactly is the difference to "ADVANCED" FILTERING I
      //
      if (resultFoundByFiltering == false
          && firstPartOfFirstPart.length() > 0
          && lastPart.getQueryString().find(NOT_QUERY_SEP) == string::npos)
      {
        assert(!resultFoundByFiltering);
        assert(result1ForAdvancedFiltering == NULL);
        assert(fixed_separators._separators[SAME_DOC]._separatorString == " ");
        // Note: always have to use SAME_DOC separator here; TODO: explain why
        string queryToCheckInHistory = firstPartOfFirstPart.getQueryString()
          + fixed_separators._separators[SAME_DOC]._separatorString
          + lastPart.getQueryString();
        #ifndef NDEBUG
        log << "! search for this query in history (for advanced shortcut): " << queryToCheckInHistory << endl;
        #endif
        assert(!result1ForAdvancedFiltering);

        // Case: found query in history (for advanced shortcut)
        if ( (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)
                && isInHistory(queryToCheckInHistory, result1ForAdvancedFiltering)
                && (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED) )
        {
          if (!result1ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
          assert(isInHistoryConst(queryToCheckInHistory));
          setStatusOfHistoryEntry(queryToCheckInHistory,QueryResult::FINISHED|QueryResult::IN_USE);
          #ifndef NDEBUG
          log << "! FOUND this query in history (for advanced shortcut): " << queryToCheckInHistory << endl;
          log << "! combining with result for \"" << firstPart << "\" (for advanced shortcut): " << queryToCheckInHistory << endl;
          #endif
          assert(result1ForAdvancedFiltering);
          assert(!result2ForAdvancedFiltering);
          broadHistoryTimer.stop();
          broadHistoryTimer4.stop();
          try
          {
            processComplexQuery(firstPart, result2ForAdvancedFiltering);
                                // DEFAULT_NOF_HITS_TO_COMPUTE,DEFAULT_NOF_COMPLETIONS_TO_COMPUTE, true,useLinearWordlistIntersection);
            #ifndef NDEBUG
            cout << "! returned from recursive call (3)" << endl;
            #endif
            assert(isInHistoryConst(firstPart));
            setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED);
            setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED|QueryResult::IN_USE);
          }
          catch (Exception& e)
          {
            log << IF_VERBOSITY_HIGH
                << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
                << "\"" << query << "\" from history" << endl;
            if ((e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT) && (getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION))
            {
              assert(getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION);
            }
            result2ForAdvancedFiltering = NULL;
            CS_RETHROW(e);
              // log << "! " << e.getFullErrorMessage() << endl;
              // CS_THROW(Exception::ERROR_PASSED_ON, e.errorMessage());
          }
          if (!result2ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
          broadHistoryTimer4.cont();
          broadHistoryTimer.cont();
          assert(result2ForAdvancedFiltering);

          WordRange wordRange(-1,1); // infinite range

          // TODO: ONCE INTERSECT AND UNITE ARE CHANGED, THIS CAN BE REMOVED
          broadHistoryTimer4.stop();
          broadHistoryTimer.stop();
          assert(result->_docIds.size() == 0);

          // TODO: What kind of error is this? Throw appropriate exception
          if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
          {
            CS_THROW(Exception::OTHER, "something bad happened with result during advanced filtering II");
            log << " ******************** ERROR ********** " << endl;
            exit(1);
          }
          reserveBuffersAndResetPointers(*result);
          broadHistoryTimer.cont();
          broadHistoryTimer4.cont();
          if ((!result2ForAdvancedFiltering->isEmpty()) && (!result1ForAdvancedFiltering->isEmpty() ))
          {
            broadHistoryTimer4.stop();
            broadHistoryTimer.stop();
            assert((!(MODE & WITH_SCORES)) || (result2ForAdvancedFiltering->_scores.size() == result2ForAdvancedFiltering->_docIds.size()));
            log << "! NEW INTERSECT: called from case \"advanced filtering II\"" << endl;
            intersectTwoPostingLists
             (*result2ForAdvancedFiltering,
              *result1ForAdvancedFiltering,
              *result,
              splitSeparator,
              _queryParameters.docScoreAggDifferentQueryParts,
              wordRange);
            broadHistoryTimer.cont();
            broadHistoryTimer4.cont();
          }
          assert(isInHistoryConst(firstPart));
          assert(isInHistoryConst(queryToCheckInHistory));
          assert(result->_docIds.isSorted());
          result->_docIds.markAsSorted(true);

          resultFoundByFiltering = true;
          result->resultWasFilteredFrom = "[SHORTCUT]";
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_FILTERED_3);
        }
        // end Case: found query in history (for advanced shortcut)
      }
      //
      // CASE 2.4: "ADVANCED" FILTERING II (e.g., filter approx*..algo* venue:* from approx venue:*)
      //
      //    *** END
      //
      broadHistoryTimer4.stop();
    }
    // TODO: this is the closing } of CASE 2.3 and 2.4 together (advanced filtering I and II)


    //
    // CASE 2.4: NO FILTERING (none of "NORMAL", "ADVANCED", "ADVANCED II" above worked)
    //
    //   Recurse on first part, then intersect with last part
    //
    broadHistoryTimer5.cont();
    broadHistoryTimer.cont();
    log << IF_VERBOSITY_HIGHER << "! no filtering/shortcut successful " << endl;
    if (hasManyParts && !resultFoundByFiltering)
    {
      //
      // CASE 2.4.1: Processing first part of query recursively
      //
      QueryResult *resultFirstPart = NULL;
      broadHistoryTimer.stop();
      broadHistoryTimer5.stop();
      try
      {
        processComplexQuery(firstPart, resultFirstPart);
                            // k1_docs, k2_words, filterResults,useLinearWordlistIntersection);
        #ifndef NDEBUG
        cout << "! returned from recursive call (4)" << endl;
        #endif
        assert(resultFirstPart->_status & QueryResult::FINISHED);
        assert(isInHistoryConst(firstPart));
        setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED);
        setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED|QueryResult::IN_USE);
        assert( !(MODE & WITH_SCORES) || (resultFirstPart->_scores.size() == resultFirstPart->_docIds.size() ));
      }
      catch (Exception& e)
      {
        log << IF_VERBOSITY_HIGH
            << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
            << "\"" << query << "\" from history" << endl;
        if ((e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT) && (getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION))
        {
          assert(getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION);
        }
        resultFirstPart = NULL;
        CS_RETHROW(e);
          // log << "! " << e.getFullErrorMessage() << endl;
          // CS_THROW(Exception::ERROR_PASSED_ON, e.errorMessage());
      }  // end: catch Exception
      if (!resultFirstPart->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
      assert(resultFirstPart);
      assert(resultFirstPart->_status & QueryResult::FINISHED);
      assert( !(MODE & WITH_SCORES) || (resultFirstPart->_scores.size() == resultFirstPart->_docIds.size() ));


      //
      // CASE 2.4.2: Intersect result of first part with result for last part
      //

      // TODO: once intersect and unite are changed, this can be removed
      assert(result->_docIds.size() == 0);
      // TODO: What kind of error is this? Throw appropriate exception
      if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
      {
        CS_THROW(Exception::OTHER, "something bad happened with result during normal processing");
        log << " ******************** ERROR ********** " << endl; exit(1);
      }
      reserveBuffersAndResetPointers(*result);
      broadHistoryTimer.cont();
      broadHistoryTimer5.cont();
      if (resultFirstPart->isEmpty() == false)
      {
        if (MODE & WITH_POS) assert(resultFirstPart->_docIds.isSorted());
        assert((*resultFirstPart).isEmpty() || !(*resultFirstPart).isEmpty());
        #ifndef NDEBUG
        log << "! Computing result for last Part (" << lastPart <<") with candidates from first part" << endl << flush;
        #endif
        broadHistoryTimer.stop();
        broadHistoryTimer5.stop();
        assert(resultFirstPart->_status & QueryResult::FINISHED);
        assert( resultFirstPart->_docIds.isFullList() || resultFirstPart->_docIds.size() > 0 );
        if (MODE & WITH_SCORES) assert(resultFirstPart->_scores.size() == resultFirstPart->_docIds.size());
        if (MODE & WITH_POS)    assert(resultFirstPart->_positions.size() == resultFirstPart->_docIds.size());
        //
        // TODO: deal with case here that last part of query is in history. This
        // was previously dealt with in HybCompleter::processBasicQuery, which
        // is the inappropriate place to do so.
        //

        //
        // CASE 2.4.2.1: Last part is in history
        //
        QueryResult* resultLastPart = NULL;
        if ((getStatusOfHistoryEntry(lastPart) & QueryResult::FINISHED) && isInHistory(lastPart, resultLastPart))
        {
          log << IF_VERBOSITY_HIGH << "! last part of query is in history" << endl;
          setStatusOfHistoryEntry(lastPart, QueryResult::FINISHED | QueryResult::IN_USE);
          CS_ASSERT(resultLastPart != NULL);
          intersectTwoPostingLists
           (*resultFirstPart,
            *resultLastPart,
            *result,
            splitSeparator,
            _queryParameters.docScoreAggDifferentQueryParts);
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_INTERSECT);
          result->_query = query;
          result->_prefixCompleted = lastPart.getQueryString();
        }

        //
        // CASE 2.4.2.2: Last part is not in history, output mode is OUTPUT_MATCHES
        //
        else if (splitSeparator.getOutputMode() == Separator::OUTPUT_MATCHES)
        {
          log << IF_VERBOSITY_HIGH << "! last part of query is not in history; normal output mode" << endl;
          processBasicQuery(*resultFirstPart, firstPart, lastPart, splitSeparator, *result);
        }

        //
        // CASE 2.4.2.3: Last part is not in history, output mode is OUTPUT_NON_MATCHES or OUTPUT_ALL
        //
        else
        {
          log << IF_VERBOSITY_HIGH << "! last part of query is not history; special output mode" << endl;
          processQuery(lastPart, resultLastPart);
          CS_ASSERT(resultLastPart != NULL);
          intersectTwoPostingLists
           (*resultFirstPart,
            *resultLastPart,
            *result,
            splitSeparator,
            _queryParameters.docScoreAggDifferentQueryParts);
        }

        broadHistoryTimer.cont();
        broadHistoryTimer5.cont();
      }
      assert(isInHistoryConst(firstPart));
      result->setHowResultWasComputed(QueryResult::NORMAL);
    }
    //
    // CASE 2.4: NO FILTERING (none of "NORMAL", "ADVANCED", "ADVANCED II" above worked)
    //
    //   *** END
    //

    //
    // CASE 2.5: ONE-WORD QUERY and not in history
    //
    else if (resultFoundByFiltering == false)
    {
      // pass full list to completionsForWordRange
      QueryResult fullResult(true); // / THIS SEEMS TO FAIL ONCE IN A WHILE ...
      Separator fullSeparator = Separator("",pair<signed int, signed int>(-1,-1),FULL);
      broadHistoryTimer5.stop();
      broadHistoryTimer.stop();
      assert( fullResult._docIds.isFullList() || fullResult._docIds.size() > 0 );
      if (MODE & WITH_SCORES) assert(fullResult._scores.size() == fullResult._docIds.size());
      if (MODE & WITH_POS) assert(fullResult._positions.size() == fullResult._docIds.size());
      assert(fullResult._status & QueryResult::FINISHED);
      processBasicQuery(fullResult, firstPart, lastPart, fullSeparator, *result); // , NULL, useLinearWordlistIntersection); // NULL: 'Last part' cannot be read from history
      assert(result->_docIds.size() == result->_wordIdsOriginal.size());
      broadHistoryTimer.cont();
      broadHistoryTimer5.cont();
      result->setHowResultWasComputed(QueryResult::NORMAL);
    }
    //
    // CASE 2.5: ONE-WORD QUERY and not in history
    //
    //   *** END
    //

    //
    // 2.6 SOME CHECKS ON RESULT
    //
    assert(result == isInHistory(query));
    assert(result->_docIds.isSorted());
    result->_docIds.markAsSorted(true); // TODO: OBSOLETE: LEAVE HERE FOR NOW FOR SOME CHECKS
    broadHistoryTimer.stop();
    broadHistoryTimer5.stop();

    //
    // 2.7 TOP-K HITS AND COMPLETIONS
    //
    assert((result->_status == QueryResult::UNDER_CONSTRUCTION) || (notIntersectionMode));
    assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
    assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
    if (result->_status == QueryResult::UNDER_CONSTRUCTION)
    {
      if (result->isLockedForReading)
        CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "before topMatchesFromAllMatches");
      if (result->isLockedForWriting)
        CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before topMatchesFromAllMatches");
      result->isLockedForWriting = true;
      computeTopHitsAndCompletions(*result);
      /*
      topMatchesFromAllMatches(*result,
          WITHIN_DOC_AGGREGATION, BETWEEN_DOCS_AGGREGATION,
          WITHIN_COMPLETION_AGGREGATION, BETWEEN_COMPLETIONS_AGGREGATION,
          k1_docs, k2_words);
      */
      if (!result->isLockedForWriting)
        CS_THROW(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after topMatchesFromAllMatches");
      result->isLockedForWriting = false;
    }
    broadHistoryTimer.cont();
    broadHistoryTimer5.cont();
    assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
    assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
    assert( ( result->_docIds.size() == 0) || ( result->_topWordScores.size() > 0  ));
    assert(result->_docIds.isSorted());
    assert((!(MODE & WITH_POS)) || result->_docIds.isMarkedSorted()
        || result->_docIds.size() == 0);

    #ifndef NDEBUG
    assert(result->_topWordDocCounts.size() == result->_topWordOccCounts.size());
    for (unsigned long i =0 ; i<result->_topWordDocCounts.size() ; i++)
    {
      assert (result->_topWordDocCounts.operator[](i) <=  result->_topWordOccCounts.operator[](i));
    }
    #endif
    broadHistoryTimer5.stop();
    broadHistoryTimer6.cont();

    //
    // 2.8 CREATE THE STRINGS TO DISPLAY; TODO: what does that mean?
    //
    //   Does this have size default_nof_completions_to_compute ????
    //
    if (result->_status == QueryResult::UNDER_CONSTRUCTION)
    {
      assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
      assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
      result->setCompletions(result->_topWordScores, result->_topWordDocCounts,
          result->_topWordOccCounts, *_vocabulary);
    }
    broadHistoryTimer6.stop();

    //
    // 2.9 FINALIZE ENTRY IN HISTORY
    //
    broadHistoryTimer7.cont();
    assert(result == isInHistory(query));
    if (result->_status == QueryResult::UNDER_CONSTRUCTION)
    {
      if (MODE & WITH_POS )    assert(result->_docIds.size() == result->_positions.size());
      if (MODE & WITH_SCORES ) assert(result->_docIds.size() == result->_scores.size());
      if (result->isLockedForReading)
        CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "before freeExtraSpace");
      if (result->isLockedForWriting)
        CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before freeExtraSpace");
      result->isLockedForWriting = true;
      result->freeExtraSpace();
      if (!result->isLockedForWriting) CS_THROW
        (Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after freeExtraSpace");
      result->isLockedForWriting = false;
      // CHANGE(hagn, 28Jan11):
      //history.finalizeSize(query.getQueryString());
      finalizeSizeOfHistory(query);
      if (MODE & WITH_POS)    assert(result->_docIds.size() == result->_positions.size());
      if (MODE & WITH_SCORES) assert(result->_docIds.size() == result->_scores.size());
      assert(isInHistoryConst(query));
      setStatusOfHistoryEntry(query, QueryResult::FINISHED);
      setStatusOfHistoryEntry(query, QueryResult::FINISHED | QueryResult::IN_USE);
      //log << "! INFORMATION: Now history looks like this:" << "\n" << history->asString() << endl;
    }
    assert(result); // there will always be a (possibly empty) result
    assert(history.check(DO_LOCK, false)); // true means: also check status (= QueryResult::FINISHED) of entries
    if (MODE & WITH_POS) assert(result->_docIds.size() == 0 || result->_docIds.isMarkedSorted());
    broadHistoryTimer.stop();
    broadHistoryTimer7.stop();
  }
  //
  // CASE 2: QUERY NOT IN HISTORY (not even being computed)
  //
  //   *** END
  //


  //
  // CASE 3: QUERY WAS IN HISTORY BUT NOT YET FINISHED
  //
  //   Simply throws an exception.
  //
  //   Note: in CASE 0 at the beginning of this (long) function, there was some
  //   (preliminary) code for waiting a little if a query result is just being
  //   computed.
  //
  else
  {
    CS_ASSERT(getStatusOfHistoryEntry(query) != QueryResult::DOES_NOT_EXIST);
    ostringstream os;
    os << "history status: " << int(getStatusOfHistoryEntry(query));
    CS_THROW(Exception::HISTORY_ENTRY_CONFLICT, os.str());
  }


  //
  // CHECK THE RESULT (check equal list lenghts, QueryResult::FINISHED, etc.)
  //
  if (result->check() == false) CS_THROW(Exception::BAD_QUERY_RESULT, "");
  if (result != NULL) assert(result->_status & QueryResult::FINISHED);
  if (MODE & WITH_SCORES)  assert(result->_scores.size() == result->_docIds.size());

  log << AT_END_OF_METHOD << "; query was \"" << query << "\"; result has " << result->getSize() << " postings" << endl;
  if (showQueryResult) result->show();

} // end: processComplexQuery


// _____________________________________________________________________________
// CHECKS IF ALL WORDS IN THE BUFFER ARE IN THE SPECIFIED RANGE (as they must be)
template <unsigned char MODE>
bool CompleterBase<MODE>::checkWordlistBuffer(const WordRange& wordRange) const
{
  for (unsigned long i = 0; i < _wordlistIntersectionBuffer->size(); i++)
    if (!wordRange.isInRange((*_wordlistIntersectionBuffer)[i])) return false;
  return true;
}


// _____________________________________________________________________________
// SETS THE INITIAL BUFFER SIZE AND INITIALIZES THE COMPRESSION AND COMPLETIONS BUFFERS
// ALSO RESETS COUNTERS
// sizes for buffers are set by setInitialBufferSizes(...)
  template <unsigned char MODE>
void CompleterBase<MODE>::reserveCompressionBuffersAndResetPointersAndCounters() // was initialize()
{
#ifndef NDEBUG
  log << "! in initialize. initIntBuffSize: " << initialIntersectionBufferSize  << ", initUniBuffSize: " << initialUnionBufferSize << endl;
#endif
  //      vectorError = 0;
  //      errorStatus = 0;
  reserveCompressionBuffersAndResetPointers(); // sets pointers to NULL, reserves compression Buffer
  resetCounters();
}


// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::reserveCompressionBuffersAndResetPointers() // was: initializeBuffer()
{
  _doclistIntersectionBuffer = NULL;
  _wordlistIntersectionBuffer = NULL;
  _scorelistIntersectionBuffer = NULL;
  _positionlistIntersectionBuffer = NULL;
  _doclistUnionBuffer = NULL;
  _wordlistUnionBuffer = NULL;
  _topWordDocCountsBuffer = NULL;
  _topWordOccCountsBuffer = NULL;
  _topWordScoresBuffer = NULL;
  _topDocScoresBuffer = NULL;
  _lastBlockScoresBuffer = NULL;

  assert(initialCompressionBufferSize > 0);
  _compressionBuffer = new char[initialCompressionBufferSize];
  if (!_compressionBuffer) throw Exception(Exception::NEW_FAILED, "new in CompleterBase::reserveCompressionBuffersAndResetPointers() failed");
  //////      memset(_compressionBuffer,0,(size_t) COMPRESSION_BUFFER_INIT);
  assert(_compressionBuffer);
  _compressionBufferSize = initialCompressionBufferSize;
  /////      _bufferPosition = 0;

}


// _____________________________________________________________________________
// does NOT reserve the compression buffer
// only mildly useful, as the buffers are resized when needed anyways
template <unsigned char MODE>
void CompleterBase<MODE>::reserveBuffersAndResetPointers(QueryResult& resultToWriteTo)
{
  #ifndef NDEBUG
  log << "! reserving "
      << commaStr((off_t) initialIntersectionBufferSize)
      << " elements for each buffer ... " << flush;
  #endif

  if (resultToWriteTo.isLockedForWriting)
    throw Exception(Exception::RESULT_LOCKED_FOR_WRITING, "in reserveBuffersAndResetPointers");

  resultToWriteTo.isLockedForWriting = true;

  //! START TIMER
  resizeAndReserveTimer.cont();

  //! RESERVE THE FOUR LONG BUFFERS
  //      assert((initialIntersectionBufferSize > 0) && (initialUnionBufferSize > 0));
  assert(resultToWriteTo._docIds.size() == 0);
  resultToWriteTo._docIds.reserve(initialIntersectionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  assert(  resultToWriteTo._docIds.capacity() >= initialIntersectionBufferSize);
  assert(resultToWriteTo._wordIdsOriginal.size() == 0);
  resultToWriteTo._wordIdsOriginal.reserve(initialIntersectionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  assert(resultToWriteTo._wordIdsOriginal.capacity() >= initialIntersectionBufferSize);
  if(MODE & WITH_POS)
  {
    assert(resultToWriteTo._positions.size() == 0);
    resultToWriteTo._positions.reserve(initialIntersectionBufferSize);
    //          if (vectorError) {errorStatus = vectorError; return;}
    assert(resultToWriteTo._positions.capacity() >= initialIntersectionBufferSize);
  }
  if(MODE & WITH_SCORES)
  {
    assert(resultToWriteTo._scores.size() == 0);
    resultToWriteTo._scores.reserve(initialIntersectionBufferSize);
    //          if (vectorError) {errorStatus = vectorError; return;}
    assert(resultToWriteTo._scores.capacity() >= initialIntersectionBufferSize);
  }

  //! RESERVE THE SHORT BUFFERS
  //      assert(initialUnionBufferSize > 0);
  assert(resultToWriteTo._topWordIds.size() == 0);
  resultToWriteTo._topWordIds.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  assert(resultToWriteTo._topWordIds.capacity() >= initialUnionBufferSize);
  assert(resultToWriteTo._topDocIds.size() == 0);
  resultToWriteTo._topDocIds.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  assert(resultToWriteTo._topDocIds.capacity() >= initialUnionBufferSize);
  resultToWriteTo._topWordDocCounts.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  resultToWriteTo._topWordOccCounts.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  resultToWriteTo._topWordScores.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  resultToWriteTo._topDocScores.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  assert(resultToWriteTo._topDocScores.capacity() >= initialUnionBufferSize);
  resultToWriteTo._lastBlockScores.reserve(initialUnionBufferSize);
  //      if (vectorError) {errorStatus = vectorError; return;}
  assert(resultToWriteTo._lastBlockScores.capacity() >= initialUnionBufferSize);

  //! STOP TIMER
  resizeAndReserveTimer.stop();

  assert(&(resultToWriteTo._docIds) != NULL);
  _doclistIntersectionBuffer = &(resultToWriteTo._docIds);
  _wordlistIntersectionBuffer = &(resultToWriteTo._wordIdsOriginal);
  if(MODE & WITH_POS) {_positionlistIntersectionBuffer = &(resultToWriteTo._positions);}
  if(MODE & WITH_SCORES) {_scorelistIntersectionBuffer = &(resultToWriteTo._scores);}

  _doclistUnionBuffer =  &(resultToWriteTo._topDocIds);
  _wordlistUnionBuffer =  &(resultToWriteTo._topWordIds);
  _topDocScoresBuffer =  &(resultToWriteTo._topDocScores);
  _lastBlockScoresBuffer =  &(resultToWriteTo._lastBlockScores);
  _topWordScoresBuffer = &(resultToWriteTo._topWordScores); // NEW NEW NEW
  _topWordDocCountsBuffer = &(resultToWriteTo._topWordDocCounts); // NEW NEW NEW
  _topWordOccCountsBuffer = &(resultToWriteTo._topWordOccCounts); // NEW NEW NEW


  if (!resultToWriteTo.isLockedForWriting)
    throw Exception(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "in method reserveBuffersAndResetPointers");
  resultToWriteTo.isLockedForWriting = false;

  #ifndef NDEBUG
  log << "done reserving " << endl;
  #endif

}


// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::setInitialBufferSizes
 (unsigned long int initIntBuffSize,
  unsigned long int initUniBuffSize,
  unsigned long int initCompressBuffSize)
 {
   #ifndef NDEBUG
   log << "! (in setInitialBufferSizes) initIntBuffSize: "
       << initIntBuffSize << ", initUniBuffSize: "
       << initUniBuffSize << endl;
   #endif
   initialIntersectionBufferSize = initIntBuffSize; // can be changed later
   initialUnionBufferSize = initUniBuffSize;
   initialCompressionBufferSize = initCompressBuffSize;
 }


// _____________________________________________________________________________
// used in destructor
template <unsigned char MODE>
void CompleterBase<MODE>::freeCompressionBuffer()
{
  if (_compressionBuffer)
  {
    #ifndef NDEBUG
    log << "! freeing compression buffer ( "
        << _compressionBufferSize/(1024*1024) << " MB)" << endl;
    #endif
    assert(_compressionBufferSize > 0);
    delete[] _compressionBuffer;
    _compressionBuffer = NULL;
  }
  _compressionBufferSize = 0;
}


// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::writeCompletionsAsStringsToQueryResult(QueryResult &result)
{
  assert( ( result._docIds.size() == 0) || ( result._topWordScores.size() > 0  ));
  // Check Buffers
  assert(_wordlistIntersectionBuffer);
  assert((!(MODE & WITH_SCORES)) || ((*_scorelistIntersectionBuffer).size() == (*_doclistIntersectionBuffer).size()));
  assert((!(MODE & WITH_POS)) || ((*_positionlistIntersectionBuffer).size() == (*_doclistIntersectionBuffer).size()));
  assert((!(MODE & WITH_POS)) || ( (*_doclistIntersectionBuffer).isSorted()));
  assert((*_doclistIntersectionBuffer).size() == (*_wordlistIntersectionBuffer).size());
  assert((*_doclistIntersectionBuffer).isSorted());
  assert( ((*_doclistIntersectionBuffer).size() == 0) || ( _topWordScoresBuffer->size() > 0  ));
  assert(_topWordScoresBuffer->size() == _topWordDocCountsBuffer->size());
  assert(_topWordScoresBuffer->size() == _topWordOccCountsBuffer->size());
  assert(  ((*_doclistIntersectionBuffer).size() == 0) || ( _topWordScoresBuffer->size() > 0 ));
  assert(_topWordScoresBuffer->size() == result._topWordIds.size());
  // sets completions to strings (rather than wordIds)
  // also attributes scores and frequencies to these strings

  // !!! WORK IS ONLY DONE HERE !!!! This creates the strings
  // 20Dec2006: Introduce occCounts here
  #ifndef NDEBUG
  for (unsigned long i =0 ; i<_topWordDocCountsBuffer->size() ; i++)
  {
    assert (_topWordDocCountsBuffer->operator[](i) <=  _topWordOccCountsBuffer->operator[](i));
  }
  #endif
  result.setCompletions((*_topWordScoresBuffer), (*_topWordDocCountsBuffer),
      (*_topWordOccCountsBuffer), *_vocabulary);

  assert((!(MODE & WITH_SCORES)) || (result._scores.size() == result._docIds.size()));
} // end: writeCompletionsAsStringsToQueryResult


// _____________________________________________________________________________
  template <unsigned char MODE> 
    void CompleterBase<MODE>::resetTimers()
    {
      threadTimer.reset();
      receiveQueryTimer.reset();
      buildResultStringTimer.reset();
      sendResultTimer.reset();
      closeConnectionTimer.reset();
      fileReadTimer.reset();
      externalTimer.reset();
      doclistDecompressionTimer.reset();
      wordlistDecompressionTimer.reset();
      positionlistDecompressionTimer.reset();
      intersectionTimer.reset();
      prefixToRangeTimer.reset();
      history->resetTimer();
      historyCleanUpTimer.reset(); 
      processQueryTimer.reset();
      getExcerptsTimer.reset(); // NEW 18Sep06 (Holger): should be part of query summary
      intersectWordlistsTimer.reset(); // 18Jul06 so far this times the whole method, no breakdown yet
      intersectWordlistsTimer1.reset();
      intersectWordlistsTimer2.reset();
      intersectWordlistsTimer3.reset();
      intersectWordlistsTimer4.reset();
      intersectWordlistsTimer5.reset();
      mergeResultsTimer.reset(); // 31Oct06 so far this times the whole method, no breakdown yet
      resizeAndReserveTimer.reset();
      stlSortTimer.reset(); // only measures sorting outside of scoring
      invMergeTimer.reset();
      scoreDocsTimer.reset(); //includes time for removing duplicates and also for sorting
      scoreDocsTimer1.reset();
      scoreDocsTimer2.reset();
      scoreDocsTimer3.reset();
      scoreDocsTimer4.reset();
      scoreWordsTimer.reset(); //includes time for the bucket sort (which also removes duplicates)
      appendTimer.reset();
      broadHistoryTimer.reset();//includes everything to do with filtering
      broadHistoryTimer0.reset();
      broadHistoryTimer1.reset();
      broadHistoryTimer2.reset();
      broadHistoryTimer3.reset();
      broadHistoryTimer4.reset();
      broadHistoryTimer5.reset();
      broadHistoryTimer6.reset();
      broadHistoryTimer7.reset();
      timerTimer.reset();
      setCompletionsTimer.reset();
      setCompletionsTimer1.reset();
      setCompletionsTimer2.reset();
      groupDocsByWordIdTopKTimer.reset();

      // reset fuzzysearch timers
      // reset fuzzysearch timers
      fuzzySearchDistanceComputationsTimer.reset();
      fuzzySearchProcessQueryTimer.reset();
      fuzzySearchTotalTimer.reset();
      fuzzySearchAuxTimer.reset();
      fuzzySearchMergeTimer.reset();
      fuzzySearchPreMergeTimer.reset();
      fuzzySearchFetchListsTimer.reset();
      fuzzySearchComputeQuerySuggestionsTimer.reset();

      // reset kwaymerge timer
      kwayMergeTimer.reset();
    }

// _____________________________________________________________________________
  template <unsigned char MODE>
    void CompleterBase<MODE>::resetCounters()
    {
      nofQueriesFromHistory = 0;
      nofQueriesByFiltering = 0;
      //      recursionLevel = 0;
      doclistUniteVolume             = 0;
      wordlistUniteVolume            = 0;
      scorelistUniteVolume           = 0;
      positionlistUniteVolume        = 0;
      intersectedVolume              = 0;
      intersectNofPostings           = 0;
      nofIntersections               = 0;
      nofUnions                      = 0;
      volumeReadFromFile             = 0;
      doclistVolumeDecompressed      = 0;
      positionlistVolumeDecompressed = 0;
      wordlistVolumeDecompressed     = 0;
      scorelistVolumeRead            = 0;
      nofQueries                     = 0;
      nofBlocksReadFromFile          = 0;

      fuzzySearchCoverIndex                         = 0;
      fuzzySearchNumRelevantWordsSelected           = 0;
      fuzzySearchNumRelevantWords                   = 0;
      fuzzySearchNumDistinctWordsInSelectedClusters = 0;
      fuzzySearchNumSimilarWords                    = 0;

    }  // end: resetCounters


  /*
  //! WRITE VOCABULARY TO FILE (one word per line)
  template <unsigned char MODE>
    void CompleterBase<MODE>::writeVocabularyToFile(const char* fileName) const
    {
      FILE* file = fopen(fileName, "w");
      assert(file != NULL);
      assert(_vocabulary.size() > 0);
      log << "* writing all words to file \"" << fileName << "\" ... " << flush;
      for(unsigned int i=0; i<_vocabulary.size(); i++)
      {
         // does not write the null termination
         fputs(_vocabulary.operator[](i).c_str(),file);
         fputs("\n",file); // append a newline char as a readable word separator
      }
      log << "done (" << commaStr(_vocabulary.size()) << " words)" << endl;
      fclose(file);
    } // end: writeVocabularyToFile(..)
   */


// _____________________________________________________________________________
//! Copy list of postings, filtering out those postings with word id in given
//! range
/*!
 *   Note: result list is written from beginning, nothing is appended (and so
 *   there is no point in passing a non-empty result list)
 *
 *   TODO: explain where and why this is needed? Note that in the current
 *   version (as of 25Mar08) this is also used for very long postings lists like
 *   that of ct:author:*, where nothing has to be filtered. The mere copying
 *   then costs over 50 msecs on dork, which is pure waste if the thing was
 *   already in the history.
 */
template <unsigned char MODE>
template <typename vW, typename vD, typename vP, typename vS>
void CompleterBase<MODE>::copyAndFilter(      vW& filteredWords,
                                        const vD& unfilteredDoclist,
                                        const vW& unfilteredWordlist,
                                        const vP& unfilteredPositionlist,
                                        const vS& unfilteredScorelist,
                                        const WordId& wordrangeLow,
                                        const WordId& wordrangeHigh,
                                              vD& filteredDocs,
                                              vP& filteredPositionlist,
                                              vS& filteredScorelist)
{
  if (MODE & WITH_POS)
    assert(unfilteredPositionlist.size() == unfilteredDoclist.size());
  if (MODE & WITH_POS)
    assert(unfilteredDoclist.isSorted());
  if (MODE & WITH_SCORES)
    assert(unfilteredScorelist.size() == unfilteredDoclist.size());
  assert(unfilteredDoclist.size() == unfilteredWordlist.size());
  assert(wordrangeLow <= wordrangeHigh);
  assert(wordrangeLow >= 0);
  const unsigned long nofElementsToScan = unfilteredWordlist.size();
  filteredWords.resize(nofElementsToScan);
  filteredDocs.resize(nofElementsToScan);
  if (MODE & WITH_POS)    filteredPositionlist.resize(nofElementsToScan);
  if (MODE & WITH_SCORES) filteredScorelist.resize(nofElementsToScan);
  unsigned int nofMatches = 0;
  unsigned int lastDocIdThatPassedThroughFilter = MAX_DOC_ID;
  CS_ASSERT_LT(_metaInfo->getMaxDocID(), MAX_DOC_ID);
  for(unsigned int i = 0; i < nofElementsToScan; i++)
  {
    // TODO(bast): This is a bug, the special posting should only be kept, if
    // at least one posting with a normal word id made it through.
    WordId wordId = unfilteredWordlist[i];
    DocId docId = unfilteredDoclist[i];
    if ((wordId >= wordrangeLow && wordId <= wordrangeHigh) ||
        (wordId == SPECIAL_WORD_ID &&
         docId == lastDocIdThatPassedThroughFilter))
    {
      filteredWords[nofMatches] = wordId;
      filteredDocs [nofMatches] = docId;
      lastDocIdThatPassedThroughFilter = docId;
      if (MODE & WITH_POS)
        filteredPositionlist[nofMatches] = unfilteredPositionlist[i];
      if (MODE & WITH_SCORES)
        filteredScorelist[nofMatches] = unfilteredScorelist[i];
        // NEW(Hannah 14Dec10): exact match bonus commented out for two reasons:
        // 1. EXACT_MATCH_BONUS (which used to be 4) was added every time
        // copyAndFilter was called, which led to an ever increasing score when
        // launching a sequence of queries like sync*, syncm*, syncma*, ...
        // 2. The expression below would also add the EXACT_MATCH_BONUS for the
        // first completion of a prefix; in paticular, when a word had only a
        // single completion, as for sync* with Weitkaemper's test database from
        // 14Dec10.
        // filteredScorelist[nofMatches]    =
        //   unfilteredScorelist[i] +
        //   // add bonus for exact word match (gold for gold*)
        //   HAS_EXACT_MATCH_BONUS(unfilteredWordlist[i] == wordrangeLow);
      ++nofMatches;
    }
  }
  filteredWords.resize(nofMatches);
  filteredDocs.resize(nofMatches);
  if (MODE & WITH_POS) filteredPositionlist.resize(nofMatches);
  if (MODE & WITH_SCORES) filteredScorelist.resize(nofMatches);
}


// _____________________________________________________________________________
template <unsigned char MODE>
WordRange CompleterBase<MODE>::prefixToRange(string prefix,
                                             bool& notIntersectionMode) const
{
  // Case 0:  Empty query -> return the empty word range.
  if (prefix.length() == 0) { return WordRange(-1,-1); }

  // Store whether this is a NOT query or not, and then strip of the leading -
  // in case there is one.
  prefixToRangeTimer.cont();
  assert(_vocabulary.size() > 0);
  notIntersectionMode = (prefix[0] == NOT_QUERY_SEP) ? (true) : (false);
  log << IF_VERBOSITY_HIGH << "! computing range for prefix (possibly with -): "
      << prefix << " (notIntersectionMode = " << notIntersectionMode << ")"
      << endl;
  if (notIntersectionMode)
  {
    prefix.erase(0,1);
    if (prefix.length() == 0) return WordRange(-1,-1);
  }
  log << IF_VERBOSITY_HIGH << "! computing range for prefix (without -): "
      << prefix << endl;

  // NEW(bast): normalize the prefix before computing the range pertaining to
  // it.
  string prefixOriginal = prefix;
  prefix = Query::normalizeQueryPart(prefix);

  // For each of the cases that follow, we have the first word in the range, the
  // last word in the range, and the word ids, pertaining to these two.
  string rangeLowerEnd;
  string rangeUpperEnd;
  unsigned int low;
  unsigned int upp;

  // Note: findWord(x) finds smallest id of a word not strictly < x or UINT_MAX
  // if all < x, where * is assumed to be > than any character

  // Case 1:  Range query (for example, year:1997--year:2004).

  // NEW (baumgari) 18Apr13:
  // There might be some encoded special characters in the original string, like
  // - oder *. Those characters should not be interpreted.
  // Query::normalizeQueryPart does decode them, so we can't differentiate
  // anymore. Therefore save the original prefix and look within the original
  // one, if there special characters are ones to be interpreted.
  size_t rangeQueryPosOriginal = prefixOriginal.find("--");
  size_t rangeQueryPos = rangeQueryPosOriginal;
  if (rangeQueryPos != string::npos) rangeQueryPos = prefix.find("--");
  
  if (rangeQueryPos != string::npos && rangeQueryPos + 2 < prefix.size())
  {
    rangeLowerEnd = rangeQueryPos > 0 &&
                      prefixOriginal[rangeQueryPosOriginal - 1] == '*'
                      ? prefix.substr(0, rangeQueryPos - 1)
                      : prefix.substr(0, rangeQueryPos);
    rangeUpperEnd = prefix.substr(rangeQueryPos + 2);
    low = _vocabulary->findWord(rangeLowerEnd);
    upp = _vocabulary->findWord(rangeUpperEnd);
    if (prefixOriginal[prefixOriginal.length() - 1] == '*') --upp;
  }

  // Case 2:  Prefix query (for example, schedu*).
  else if (prefixOriginal[prefixOriginal.length()-1] == '*')
  {
    rangeLowerEnd = prefix.substr(0, prefix.length() - 1);
    rangeUpperEnd = prefix;
    low = _vocabulary->findWord(rangeLowerEnd);
    upp = _vocabulary->findWord(rangeUpperEnd) - 1;
  }

  // Case 3:  Single word query (for example, scheduling).
  else
  {
    rangeLowerEnd = prefix;
    rangeUpperEnd = prefix;
    low = _vocabulary->findWord(prefix);
    if (low < _vocabulary->size() && prefix != (*_vocabulary)[low]) low = UINT_MAX;
    upp = low;
  }

  // TODO(bast): Investigate the case in which low / upp are equal to
  // _vocabulary.size(). This happens,q for example, when the query word is after
  // the last word in the vocabulary. In the old code (before the two lines
  // below were added), the returned word range was then (_vocabulary.size(),
  // _vocabulary.size()), but not (-1,-1). It didn't crash, but this is not very
  // elegant.
  if (low >= _vocabulary->size()) low = UINT_MAX;
  if (upp >= _vocabulary->size()) upp = UINT_MAX;

  log << IF_VERBOSITY_HIGH
      << "! range is : \"" << rangeLowerEnd << "\" - \"" << rangeUpperEnd
      << "\", low  = " << low << " ("
      << (low < _vocabulary->size() ? (*_vocabulary)[low] : "NOT FOUND") << ")"
      << ", high = " << upp << " ("
      << (upp < _vocabulary->size() ? (*_vocabulary)[upp] : "NOT_FOUND") << ")"
      << endl;

  // If nothing was found and the original query does not end with *,
  // try appending :* (if useSuffixForExactQuery is enabled).
  // Explanation: The index is filled with words like
  // qury:query
  // haus
  // :facet:author:some_name
  // In especially the first example leads to problems, since searching for
  // "query" does not lead to any result, since the normalized version qury is not
  // part of the index, just qury:<original>. Using should a asterix leads to
  // the problem that also qurying:<original>, etc. would be found. For exact
  // match, we need to add :*.
  /*if (useSuffixForExactQuery
      && prefixOriginal[prefixOriginal.length() - 1] != '*'
      && prefixOriginal[prefixOriginal.length() - 1] != '~'
      && upp == UINT_MAX && low == UINT_MAX)
  {
    string newPrefix = prefixOriginal + wordPartSep + "*";
    log << IF_VERBOSITY_HIGH
        << "! NEW EXACT QUERY: replacing \"" << prefixOriginal << "\""
        << " by \"" << newPrefix << "\" and recomputing wordRange" << endl;

    WordRange tmp = prefixToRange(newPrefix, notIntersectionMode);
    if (tmp.firstElement() != -1) low = tmp.firstElement();
    if (tmp.lastElement()  != -1) upp = tmp.lastElement();
  }*/

  // Finally return the result.
  prefixToRangeTimer.stop();
  if (low == UINT_MAX || upp == UINT_MAX || upp < low) return WordRange(-1, -1);
  return WordRange(low, upp);
}


// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::resizeCompressionBuffer(unsigned long size)
{
#ifndef NDEBUG
  log << "! resizing compression buffer to size " << commaStr(size) << "... "
      << flush;
#endif
  if (_compressionBuffer)
  {
    resizeAndReserveTimer.cont();
    delete[] _compressionBuffer;
    resizeAndReserveTimer.stop();
    _compressionBuffer = NULL;
  }
  _compressionBufferSize = size;
  resizeAndReserveTimer.cont();
  _compressionBuffer = new char[size];
  memset(_compressionBuffer,0,size); /* to avoid TLB misses */
  resizeAndReserveTimer.stop();
  assert(_compressionBuffer);
#ifndef NDEBUG
  log << "done, now size " << commaStr(size) << endl;
#endif
}

// _____________________________________________________________________________
template <unsigned char MODE>
off_t CompleterBase<MODE>::getTotalProcessingTimeInUsecs() const
{
  threadTimer.mark();
  return threadTimer.usecs() - receiveQueryTimer.usecs();
}

// _____________________________________________________________________________
//! PRINT STATISTICS (baumgari's version, Nov 2010)
template <unsigned char MODE>
void CompleterBase<MODE>::showStatistics(ConcurrentLog& os,
                                         off_t totalTime,
                                         string indent)
{
  TimerStatistics statistics;
#ifndef NDEBUG
  statistics.addSubTimer(externalTimer,
                         statistics.AUTO,
                         "time in HYB/INV",
                         false);
#endif
  
  // receiveQueryTimer
  statistics.addSubTimer(receiveQueryTimer,
                         statistics.AUTO,
                         "receive query",
                         false);
  // fileReadTimer
  if (fileReadTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(fileReadTimer,
                                     statistics.AUTO,
                                     "read index list(s)",
                                     volumeReadFromFile / fileReadTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt size on disk",
                                     false);
  }
  else
  {
    statistics.addSubTimer(fileReadTimer,
                           statistics.AUTO,
                           "read index list(s)",
                           false);
  }
  // doclistDecompressionTimer
  if (doclistDecompressionTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(doclistDecompressionTimer,
                                     statistics.AUTO,
                                     "uncompress docs",
                                     doclistVolumeDecompressed /
                                       doclistDecompressionTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt size of uncompressed",
                                     false);
  }
  else
  {
    statistics.addSubTimer(doclistDecompressionTimer,
                           statistics.AUTO,
                           "uncompress docs",
                           false);
  }
  // wordlistDecompressionTimer
  if (wordlistDecompressionTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(wordlistDecompressionTimer,
                                     statistics.AUTO,
                                     "uncompress words",
                                     wordlistVolumeDecompressed /
                                       wordlistDecompressionTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt size of uncompressed",
                                     false);
  }
  else
  {
    statistics.addSubTimer(wordlistDecompressionTimer,
                           statistics.AUTO,
                           "uncompress words",
                           false);
  }
  // stlSortTimer
  if (stlSortTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(stlSortTimer,
                                     statistics.AUTO,
                                     "sort doclist",
                                     doclistUniteVolume   +
                                     wordlistUniteVolume  +
                                     scorelistUniteVolume +
                                     positionlistUniteVolume /
                                     stlSortTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt input size",
                                     false);
  }
  else
  {
    statistics.addSubTimer(stlSortTimer,
                           statistics.AUTO,
                           "sort doclist",
                           false);
  }
  // positionlistDecompressionTimer
  if (positionlistDecompressionTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(positionlistDecompressionTimer,
                                     statistics.AUTO,
                                     "uncompress positions",
                                     positionlistVolumeDecompressed /
                                       positionlistDecompressionTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt size of uncompressed",
                                     false);
  }
  else
  {
    statistics.addSubTimer(positionlistDecompressionTimer,
                           statistics.AUTO,
                           "uncompress positions",
                           false);
  }
  // intersectionTimer
  if (intersectionTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(intersectionTimer,
                                     statistics.AUTO,
                                     "intersect doclists",
                                     intersectNofPostings /
                                       intersectionTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt nof postings scanned",
                                     false);
  }
  else
  {
    statistics.addSubTimer(intersectionTimer,
                           statistics.AUTO,
                           "intersect doclists",
                           false);
  }
  // intersectWordlistTimer with its Subtimers.
  TimerStatistics intersectWordlistSubTimer;
  intersectWordlistSubTimer.addSubTimerWithComment(intersectWordlistsTimer1,
                                                   statistics.AUTO,
                                                   "init",
                                                   "intersect wordlists 1 (prep sorting or find ranges)",
                                                   false);
  intersectWordlistSubTimer.addSubTimerWithComment(intersectWordlistsTimer2,
                                                   statistics.AUTO,
                                                   "pre-scan",
                                                   "intersect wordlists 2 "
                                                   "(pushing sentinel or going through list 1 [1st time])",
                                                   false);
  intersectWordlistSubTimer.addSubTimerWithComment(intersectWordlistsTimer3,
                                                   statistics.AUTO,
                                                   "main-scan",
                                                   "intersect wordlists 3 "
                                                   "(linear intersection or going through list 2)",
                                                   false);
  intersectWordlistSubTimer.addSubTimerWithComment(intersectWordlistsTimer4,
                                                   statistics.AUTO,
                                                   "post-scan",
                                                   "intersect wordlists 4 "
                                                   "(- or going through list 1 [2nd time])",
                                                   false);
  intersectWordlistSubTimer.addSubTimerWithComment(intersectWordlistsTimer5,
                                                   statistics.AUTO,
                                                   "sort",
                                                   "intersect wordlists 5 (sorting results by doc id)",
                                                   false);
  statistics.addSubTimer(intersectWordlistsTimer,
                         statistics.AUTO,
                         "intersect wordlists",
                         false,
                         intersectWordlistSubTimer);
  // scoreWordsTimer with its Subtimer.
  TimerStatistics scoreWordsSubTimer; 
  scoreWordsSubTimer.addSubTimer(mapWordIdsTimer,
                                 statistics.AUTO,
                                 "map word ids",
                                 false);
  statistics.addSubTimer(scoreWordsTimer,
                         statistics.AUTO,
                         "score + dups (words)",
                         false,
                         scoreWordsSubTimer);
  // scoreDocsTimer
  if (scoreDocsTimer.usecs() != 0)
  {
    statistics.addSubTimerWithVolume(scoreDocsTimer,
                                     statistics.AUTO,
                                     "score + dups (docs)",
                                     doclistUniteVolume   +
                                     wordlistUniteVolume  +
                                     scorelistUniteVolume +
                                     positionlistUniteVolume /
                                     scoreDocsTimer.usecs(),
                                     "MB/sec",
                                     "$RATE wrt input size",
                                     false);
  }
  else
  {
    statistics.addSubTimer(scoreDocsTimer,
                           statistics.AUTO,
                           "score + dups (docs)",
                           false);
  }
  // appendTimer
  statistics.addSubTimer(appendTimer,
                         statistics.AUTO,
                         "append (triv inters)",
                         false);
  // broadHistoryTimer with its Subtimers.
  TimerStatistics broadHistorySubTimer;
  broadHistorySubTimer.addSubTimer(broadHistoryTimer0,
                                   statistics.AUTO,
                                   "0.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer1,
                                   statistics.AUTO,
                                   "1.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer2,
                                   statistics.AUTO,
                                   "2.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer3,
                                   statistics.AUTO,
                                   "3.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer4,
                                   statistics.AUTO,
                                   "4.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer5,
                                   statistics.AUTO,
                                   "5.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer6,
                                   statistics.AUTO,
                                   "6.",
                                   false);
  broadHistorySubTimer.addSubTimer(broadHistoryTimer7,
                                   statistics.AUTO,
                                   "7.",
                                   false);
  statistics.addSubTimer(broadHistoryTimer,
                         statistics.AUTO,
                         "filtering",
                         false,
                         broadHistorySubTimer);
  // historyTimer
  statistics.addSubTimer(history->getTimer(),
                         statistics.AUTO,
                         "history (fct calls)",
                         false);
  // resizeAndReserveTimer
  statistics.addSubTimer(resizeAndReserveTimer,
                         statistics.AUTO,
                         "resize, reserve, delete",
                         false);
  // invMergeTimer
  if (invMergeTimer.usecs() != 0)
  {
    statistics.addSubTimer(invMergeTimer,
                           statistics.AUTO,
                           "inv merge",
                           false);
  }
  // getExcerptsTimer
  statistics.addSubTimer(getExcerptsTimer,
                         statistics.AUTO,
                         "excerpts",
                         false);
  // buildResultStringTimer + sendResultTimer
  Timer buildAndSendTimer;
  buildAndSendTimer.setUsecs(buildResultStringTimer.usecs() +
                             sendResultTimer.usecs());  
  statistics.addSubTimer(buildAndSendTimer,
                         statistics.AUTO,
                         "send result + close",
                         false);
  // historyCleanUpTimer
  statistics.addSubTimer(historyCleanUpTimer,
                         statistics.AUTO,
                         "history cleanup",
                         false);

  // stats for for fuzzy search
  TimerStatistics fuzzySearchStats;

  // fuzzySearchProcessQueryTimer
  fuzzySearchStats.addSubTimer(fuzzySearchProcessQueryTimer,
                         statistics.AUTO,
                         "list processing",
                         false);

  // fuzzySearchFetchListsTimer
  fuzzySearchStats.addSubTimer(fuzzySearchFetchListsTimer,
                         statistics.AUTO,
                         "read blocks",
                         false);

  // fuzzySearchDistanceComputationsTimer
  fuzzySearchStats.addSubTimer(fuzzySearchDistanceComputationsTimer,
                         statistics.AUTO,
                         "distance + clusters computation",
                         false);

  // fuzzySearchComputeQuerySuggestionsTimer
  fuzzySearchStats.addSubTimer(fuzzySearchComputeQuerySuggestionsTimer,
                         statistics.AUTO,
                         "query suggestion",
                         false);

  statistics.addSubTimer(fuzzySearchTotalTimer,
                         statistics.AUTO,
                         "fuzzy search",
                         false,
                         fuzzySearchStats);

  // totalTimer
  Timer totalTimer;
  totalTimer.setUsecs(totalTime);
  statistics.addTotalTimer(totalTimer,
                           statistics.AUTO,
                           "total time");

  // Add indent and print statistics into log file.
  // Show every item that takes >= 10% of the total time.
  statistics.setThresholds(1, 90, -1);
  statistics.setPrintOptions(false, false, false);
  std::vector<std::string> statisticsVec = statistics.getStatistics();
  for (size_t i = 0; i < statisticsVec.size(); i++)
     os << indent <<  statisticsVec[i] << endl;
} // end: showStatistics

template <unsigned char MODE>
void CompleterBase<MODE>::showOneLineSummary(ConcurrentLog& os,
                                             Query& query,
                                             QueryResult* result,
                                             off_t total_usecs)
{
  os << setiosflags(ios::fixed) << setprecision(1)
     << EMPH_ON << setw(60) << left
     << (query.length() > 0 ? query : string("[empty query]"))
     << setw(6) << right << total_usecs/1000.0 << " millisecs  "
     << commaStr(result->_topDocIds.size()) << "/"
     << commaStr(result->nofTotalHits) << " hits, "
     << commaStr(result->_topCompletions.size()) << "/"
     << commaStr(result->nofTotalCompletions) << " completions, " << flush;
  if (nofBlocksReadFromFile > 0)
  os << "scanned " << numberAndNoun(nofBlocksReadFromFile, "block", "blocks")
     << " (" << commaStr(doclistVolumeDecompressed/sizeof(DocId))
     << " index items)" << flush;
  else
  os << result->_howResultWasComputed << flush;
    // else if (result->wasInHistory)
    // os << "result was in history" << flush;
    // else if (result->resultWasFilteredFrom != "")
    // os << "filtered from result in history" << flush;
    // NEW 19Sep06 (Holger): what happens when we come here?
    // else if (result->nofTotalCompletions == 0)
    // os << "no completions (trivial)" << flush;
    // else
    // os << "intersection of results in history (true?)" << flush;
  os << EMPH_OFF << endl;

  // os << EMPH_ON << setw(23) << left << (string("\"") + query + string("\""))
  //    << EMPH_OFF << " " << EMPH_ON << setw(4) << right
  //    << processQueryTimer.msecs() << " millisecs" << EMPH_OFF
  //    << "; " << setw(10) << commaStr(result->_topDocIds.size()) << "/"
  //    << commaStr(result->nofTotalHits) << " hits"
  //    << "; " << setw(4) << commaStr(result->_topCompletions.size()) << "/"
  //    << commaStr(result->nofTotalCompletions)<< " completions" << flush;
  // if (nofBlocksReadFromFile > 0)
  //   os << "; " << numberAndNoun(nofBlocksReadFromFile,"block","blocks")
  //      << " of volume " << commaStr(doclistVolumeDecompressed/sizeof(DocId))
  //      << " read" << endl;
  // else if (result->wasInHistory)
  //   os << "; [was in history]" << endl;
  // else if (result->resultWasFilteredFrom != "")
  //   os << "; [was filtered]" << endl;

} // end: showOneLineSummary




//! A LIST OF METHODS AS INTERFACE TO THE HISTORY
//
//   they are public (rather than protected), as answerQueries also accesses
//   them from the outside
//
// TODO: all of these just call the method with the same name in the History
// class, why on earth did Ingmar do that??? It is certainly not right.
//


// NEW(hagn, 28Jan11): get history flag (hf=...), so query string could become a unique id
// This was first inserted for RANK_DOCS_BY_COMPLETION_SCORES ("rd=5-Problematik")
template <unsigned char MODE>
string CompleterBase<MODE>::getFlagForHistory()
{
  ostringstream retVal;
  retVal << "&hf=";
  switch (_queryParameters.howToRankDocs)
  {
    case QueryParameters::RANK_DOCS_BY_COMPLETION_SCORES:
      retVal << (_queryParameters.howToRankWords + 10);
      break;

    case QueryParameters::RANK_DOCS_BY_FUZZY_SCORE:
      retVal << (_queryParameters.howToRankDocs + 20);
      break;

    default:
      retVal << History::HISTORY_FLAG_STD;
  }
  // NEW (baumgari) 12Nov14:
  // If ".." is used, also store start and end of the intersection window, since
  // the results differs for the same query.
  // TODO(baumgari): This is appended always, noneless if ".." was used or not,
  // since the overhead to find out if the ".." mode was used is quite high.
  // This is NOT a neat way of doing this and should be adjusted.

  // NEW(bast) 15Jun15: this effectively deactivated the keepInHistory
  // mechanism (see Globals.h) which is absolutely vital for the author, year,
  // and venue lists of DBLP.
  // retVal << "&n=" << _queryParameters.neighbourhoodStart
  //        << "," << _queryParameters.neighbourhoodEnd;
  return retVal.str();
}

// NEW(hagn, 28Jan11): Because of getFlagForHistory all history-functions should be encapsulated
template <unsigned char MODE>
void CompleterBase<MODE>::finalizeSizeOfHistory(const Query& query)
{
  // historyTimer.cont();
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  history->finalizeSize(strQuery.str());
  // historyTimer.stop();
}

template <unsigned char MODE>
void CompleterBase<MODE>::addToHistory(const Query& query,
                                       const QueryResult& result)
{
  // historyTimer.cont();
  // CHANGE(hagn, 28Jan11):
  // history.add(query.getQueryString(), result);
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  history->add(strQuery.str(), result);
  // historyTimer.stop();
}

template <unsigned char MODE>
void CompleterBase<MODE>::removeFromHistory(const Query& query)
{
  // historyTimer.cont();
  // CHANGE(hagn, 28Jan11):
  //history.remove(query.getQueryString());
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  history->remove(strQuery.str());
  // historyTimer.stop();
}

template <unsigned char MODE>
QueryResult* CompleterBase<MODE>::isInHistory(const Query& query)
{
  // historyTimer.cont();
  // CHANGE(hagn, 28Jan11):
  //QueryResult* retVal = history.isContained(query.getQueryString());
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  QueryResult* retVal = history->isContained(strQuery.str());
  // historyTimer.stop();
  return retVal;
}

template <unsigned char MODE>
const QueryResult* CompleterBase<MODE>::isInHistoryConst(const Query& query)
{
  // historyTimer.cont();
  // CHANGE(hagn, 28Jan11):
  //const QueryResult* retVal = history.isContained(query.getQueryString());
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  const QueryResult* retVal = history->isContained(strQuery.str());
  // historyTimer.stop();
  return retVal;
}

template <unsigned char MODE>
bool CompleterBase<MODE>::isInHistory(const Query& query, QueryResult*& result)
{
  // historyTimer.cont();
  assert((result == NULL) ||
         (result->nofTotalHits >= result->_topDocIds.size()));
  // CHANGE(hagn, 28Jan11):
  //bool retVal = history.isContained(query.getQueryString(), result);
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  bool retVal = history->isContained(strQuery.str(), result);
  // historyTimer.stop();
  assert((!retVal && !isInHistory(query)) || (retVal && isInHistory(query)));
  return retVal;
}

template <unsigned char MODE>
//unsigned char CompleterBase<MODE>::getStatusOfHistoryEntry(const Query& query) const
unsigned char CompleterBase<MODE>::getStatusOfHistoryEntry(const Query& query)
{
  // historyTimer.cont();
  // CHANGE(hagn, 28Jan11):
  //unsigned char retVal = history.getStatusOfEntry(query.getQueryString());
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  unsigned char retVal = history->getStatusOfEntry(strQuery.str());
  // historyTimer.stop();
  return retVal;
}

template <unsigned char MODE>
void CompleterBase<MODE>::setStatusOfHistoryEntry(const Query& query,
                                                  unsigned char status)
{
  const QueryResult* result = isInHistoryConst(query);
  // check that query is indeed in the history (was a bug)
  if (result == NULL)
    CS_THROW(Exception::HISTORY_ENTRY_NOT_FOUND,
             "for query \"" << query << "\"");
  result->checkSameNumberOfDocsWordsPositionsScores(MODE);

  // historyTimer.cont();
  // CHANGE(hagn, 28Jan11):
  //history.setStatusOfEntry(query.getQueryString(), status);
  ostringstream strQuery;
  strQuery << query.getQueryString() << getFlagForHistory();
  history->setStatusOfEntry(strQuery.str(), status);
  // historyTimer.stop();
}

// END METHODS FOR ACCESSING HISTORY

//! Rewrite join blocks
//
//   1.  rewrite all [...#...#...] -> ...#...#...
//   2.  map separators within these from SEP_CHARS -> MASKED_SEP_CHARS; see
//       Globals.h
//
void rewriteJoinBlocks(Query& query)
{
  string queryString = query.getQueryString();
  assert(SEP_CHARS.size() == MASKED_SEP_CHARS.size());
  size_t pos_beg, pos_end;
  while (true)
  {
    // find opening bracket and delete (if no more bracket -> done)
    if ((pos_beg = queryString.find(ENHANCED_QUERY_BEG)) != string::npos)
      queryString.erase(pos_beg, 1);
    else
      break;
    // find closing bracket and delete (if no closing racket -> warning, and
    // assume one at the end)
    if ((pos_end = queryString.find(ENHANCED_QUERY_END)) != string::npos)
      queryString.erase(pos_end, 1);
    else
    {
      pos_end = queryString.length();
      cerr << " [WARNING: missing '" << ENHANCED_QUERY_END
           << "' in enhanced queryString] " << endl;
    }
    // convert from -> to in covered substring
    for (unsigned int i = pos_beg; i < pos_end; ++i)
    {
      char j = SEP_CHARS.index(queryString[i]);
      if (j != -1)
      {
        assert(j < MASKED_SEP_CHARS.size());
        queryString[i] = MASKED_SEP_CHARS[j];
      }
    }
  }
  query.setQueryString(queryString);
}  // end of method rewriteJoinBlocks


//! SPLIT UNIT OF ENHANCED QUERY
//
//   1.  <part1>#<part2>#<part3> -> vector of parts
//   2.  map separators in parts from MASKED_SEP_CHARS ->
//       (unmasking, see Globals.h)
//   13Jul2006: I don't want to modify the query, hence no reference passed
//
void splitPartOfEnhancedQueryBlock(const Query& query, vector<string>& parts)
{
  string queryString = query.getQueryString();
  // unmask all separators
  assert(SEP_CHARS.size() == MASKED_SEP_CHARS.size());
  for (unsigned int i = 0; i < queryString.length(); ++i)
  {
    char j = MASKED_SEP_CHARS.index(queryString[i]);
    if (j != -1)
    {
      assert(j < SEP_CHARS.size());
      queryString[i] = SEP_CHARS[j];
    }
  }
  // split at #
  size_t pos_beg = 0, pos_end;
  while (true)
  {
    pos_end = queryString.find(ENHANCED_QUERY_SEP, pos_beg);
    if (pos_end == string::npos) pos_end = queryString.length();
    parts.push_back(queryString.substr(pos_beg, pos_end - pos_beg));
    if (pos_end == queryString.length()) break;
    pos_beg = pos_end + 1;
  }
}  // end: splitPartOfEnhancedQueryBlock


//! SPLIT UNIT OF OR QUERY
//
//   1.  <part1>|<part2>|<part3> -> vector of parts
//   2.  map separators in parts from MASKED_SEP_CHARS ->
//       (unmasking, see Globals.h)
//   Keeping masking so that we could actually do "(a* b*) or (c* d*)"
//
void splitPartOfOrQueryBlock(const Query& query, vector<string>& parts)
{
  string queryString = query.getQueryString();
  // unmask all separators
  assert(SEP_CHARS.size() == MASKED_SEP_CHARS.size());
  for (unsigned int i = 0; i < queryString.length(); ++i)
  {
    char j = MASKED_SEP_CHARS.index(queryString[i]);
    // HACK(Hagn): Skip the Fuzzy- [~] and the Synonym-Sign [^],
    // because at this point these signs are not masked.
    // Now a OR-search with *~ or *^~ will work.
    if (j != -1 && MASKED_SEP_CHARS[j] != '~' && MASKED_SEP_CHARS[j] != '^')
    {
      assert(j < SEP_CHARS.size());
      queryString[i] = SEP_CHARS[j];
    }
  }
  // split at |
  size_t pos_beg = 0, pos_end;
  while (true)
  {
    pos_end = queryString.find(OR_QUERY_SEP, pos_beg);
    if (pos_end == string::npos) pos_end = queryString.length();
    parts.push_back(queryString.substr(pos_beg, pos_end - pos_beg));
    if (pos_end == queryString.length()) break;
    pos_beg = pos_end + 1;
  }
}  // end: splitPartOfEnhancedQueryBlock



// _____________________________________________________________________________
//! PROCESS QUERY (method to be called from outside)
/*
 *    1. Rewrite join blocks of query
 *    2. Calls the internal recursive query processing method
 *    3. Sets result to NULL if some error occured (TODO: why?)
 */
template <unsigned char MODE>
unsigned int
CompleterBase<MODE>::processQuery_NEW
      (const Query& query,
       QueryResult*& result)
{
  log << AT_BEGINNING_OF_METHOD << "; query is \"" << query << "\"" << endl;
  processQueryTimer.cont();
  //
  // CASE 1: empty query -> return emptyQueryResult (defined in QueryResult.h, used to throw an exception)
  //
  if (query.empty())
  {
    result = &emptyQueryResult;
    result->_status = QueryResult::FINISHED;
  }
  //
  // CASE 2: non-empty query -> rewrite join blocks, and call the recursive processComplexQuery
  //
  else
  {
    // 1. Rewrite join blocks: [...#...#...] -> ...#...#... with separators masked
    Query queryRewritten = query;
    rewriteJoinBlocks(queryRewritten);
    log << IF_VERBOSITY_HIGH
        << "! query with join blocks rewritten: \"" << queryRewritten << "\"" << endl << flush;
    // 2. Call the internal recursive query processing method
    unsigned int retval = processComplexQuery_NEW(queryRewritten, result);
    switch (retval)
    {
      // case 0:
      // case 1:
      // case 2:
      // // ...
      // default:
    }
    setStatusOfHistoryEntry(queryRewritten, QueryResult::FINISHED);
    setStatusOfHistoryEntry(queryRewritten, QueryResult::FINISHED | QueryResult::IN_USE);
  }
  processQueryTimer.stop();
  log << AT_END_OF_METHOD << "; query was \"" << query << "\"" << endl;

  return 0;
}  // end: processQuery


// _____________________________________________________________________________
template <unsigned char MODE>
unsigned int CompleterBase<MODE>::processComplexQuery_NEW
      (const Query&        query,
             QueryResult*& result)
{
  log << AT_BEGINNING_OF_METHOD << "; query is \"" << query << "\"" << endl;

  // TABLE OF CONTENTS
  //
  // 0.  Timers + a few checks
  // 1.  CASE: Query is in history but not yet finished    -> wait a little
  // 2a. CASE: Query is in history and finished            -> fetch the result
  // 2b. CASE: Query's first and last part are in history  -> intersect results
  // 3.  CASE: Query is not in history                     -> compute it now
  // 4.  Timers + result checking
  //

  broadHistoryTimer.cont();
  broadHistoryTimer0.cont();

  //
  // CASE 0: QUERY IS IN HISTORY BUT NOT YET FINISHED -> wait a little!
  //
  if (getStatusOfHistoryEntry(query) & QueryResult::UNDER_CONSTRUCTION)
    processQuery_waitForQueryResult(query, 2*1000*1000);

  //
  // CASE 1: QUERY IS IN HISTORY AND FINISHED
  //
  //   Just reuse it. If necessary, redo the top-k computation.
  //
  if ((getStatusOfHistoryEntry(query) & QueryResult::FINISHED)
   && (isInHistory(query, result)))
  {
    bool wasUsedAlready = false;
    if (getStatusOfHistoryEntry(query) & QueryResult::IN_USE) { wasUsedAlready = true;}
    // OLD: setStatusOfHistoryEntry(query, QueryResult::FINISHED | QueryResult::IN_USE);
    setStatusOfHistoryEntry(query, QueryResult::IN_USE);
    result->setHowResultWasComputed(QueryResult::FROM_HISTORY);
    ++nofQueriesFromHistory;
    result->wasInHistory = true;

    //
    // CASE 1.1: need to recompute top-k hits and/or completions
    //
    unsigned int recomputeReasons = processQuery_NeedToRecompute(*result);
    if (recomputeReasons)
    {
      // Prepare recomputation.
      log << IF_VERBOSITY_HIGH
          << "! recomputing top hits and completions for result in history"
          << endl;
      processQuery_LogRecomputeReasons(recomputeReasons);
      if (wasUsedAlready) return Exception::HISTORY_ENTRY_CONFLICT;
      setStatusOfHistoryEntry(query, QueryResult::UNDER_CONSTRUCTION);
      broadHistoryTimer0.stop();
      broadHistoryTimer.stop();
      if (result->isLockedForReading) return Exception::RESULT_LOCKED_FOR_READING;
      if (result->isLockedForWriting) return Exception::RESULT_LOCKED_FOR_WRITING;
      
      // Do the recomputation.
      result->lock("w");
      computeTopHitsAndCompletions(*result);

      // Create the strings to display.
      broadHistoryTimer.cont();
      broadHistoryTimer0.cont();
      result->setCompletions(result->_topWordScores,
                             result->_topWordDocCounts,
                             result->_topWordOccCounts,
                             *_vocabulary);

      // Clean up.
      result->freeExtraSpace();
      result->unlock("w");
      finalizeSizeOfHistory(query);
      result->setHowResultWasComputed(QueryResult::FROM_HISTORY_TOPK_AGAIN);
    }
    //
    // CASE 1.1: have to recompute more matches
    //
    //  *** END
    //

    // Done
    assert(isInHistoryConst(query));
    setStatusOfHistoryEntry(query, QueryResult::FINISHED);
    setStatusOfHistoryEntry(query, QueryResult::FINISHED | QueryResult::IN_USE);
    if (MODE & WITH_POS) assert(result->_docIds.size() == 0 || result->_docIds.isMarkedSorted());
    assert(getStatusOfHistoryEntry(query) & QueryResult::FINISHED);
    broadHistoryTimer0.stop();
    broadHistoryTimer.stop();
    return 0;
  }
  //
  // CASE 1: QUERY IS IN HISTORY AND FINISHED
  //
  //  *** END
  //


  //
  // CASE 2: QUERY NOT IN HISTORY (not even being computed)
  //
  if (getStatusOfHistoryEntry(query.getQueryString()) == QueryResult::DOES_NOT_EXIST)
  {
    #ifndef NDEBUG
    log << "! Did not find query in history " << query.getQueryString() << endl;
    assert(result == NULL);
    assert(!isInHistoryConst(query));
    assert(result == NULL);
    assert(!isInHistoryConst(query));
    log << "! ... and it really isn't " << endl;
    #endif

    broadHistoryTimer.cont();

    // IN ALL OTHER CASES THE RESULT WAS NOT ALREADY IN THE HISTORY
    // ADD EMPTY QUERY RESULT TO HISTORY
    #ifndef NDEBUG
    log << "! adding query : " << query.getQueryString()  << " to history " << endl;
    #endif

    addToHistory(query, QueryResult()); // add an empty result to history: ONLY DONE HERE!
    assert(getStatusOfHistoryEntry(query)==QueryResult::UNDER_CONSTRUCTION);
    assert(result == NULL);

    isInHistory(query, result); // sets result pointers to this empty result
    assert(isInHistory(query));
    assert( result != NULL );

    result->wasInHistory = false;

    // INITIALIZE BUFFER (i.e., reserve a 'standard' amount of space for intersections)
    broadHistoryTimer.stop();
    assert(result->_docIds.size() == 0);
    // TODO: What kind of error is this? Throw appropriate exception
    if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
    {
      log << " ******************** ERROR ********** " << endl;
      exit(1);
    }
    reserveBuffersAndResetPointers(*result);
    broadHistoryTimer.cont();

    // SPLIT QUERY INTO TWO PARTS
    Query firstPart, lastPart;
    //      unsigned char sepIndex = 0;
    Separator splitSeparator;
    const bool hasManyParts = query.splitAtLastSeparator(&firstPart, &lastPart, &splitSeparator);
    signed char sepIndex = splitSeparator._separatorIndex;
    if (sepIndex == NEAR) {
      splitSeparator.setIntersectionWindow(_queryParameters.neighbourhoodStart,
                                           _queryParameters.neighbourhoodEnd);
    }

    log << IF_VERBOSITY_HIGHER
        << "! split query in case: not in history; separator is " << splitSeparator.infoString() << endl;

    // RETURN WITH ERROR IF STRANGE THINGS HAPPENED DURING SPLIT OF QUERY
    if (sepIndex == (signed char) -1) CS_THROW(Exception::CONTAINS_INVALID_SEPARATOR,
                                             string("\"") + splitSeparator._separatorString + string("\"") );
    if (lastPart.getQueryString() == "*") CS_THROW(Exception::SINGLE_STAR_NOT_ALLOWED, "");

    #ifndef NDEBUG
    if (hasManyParts) { log << "! query has many parts " << endl; }
    else  { log << "! query has only one part" << endl; }
    log << "! first part of query (" << query << ")in processComplexQuery: \"" << firstPart << "\"" << endl;
    log << "! last part of query in processComplexQuery: \"" << lastPart << "\"" << endl;
    log << "! sepIndex was " << (signed int) sepIndex << endl;
    log << " separator was: "; splitSeparator.show();
    log << " or directly : xx" << splitSeparator._separatorString << "xx" << endl;
    #endif

    assert( (!hasManyParts && (lastPart.getQueryString() == query.getQueryString()) )
               || (hasManyParts && (query.length() > lastPart.length())
               && (query.length() > firstPart.length() )));


    //
    // CASE 2.1: "NORMAL" FILTERING (check if any prefix of last part is in history)
    //
    //   Query        : scheduling algorithm*
    //   filter from  : scheduling algo*
    //
    broadHistoryTimer1.cont();
    bool resultFoundByFiltering = false;
    unsigned int lengthLastPart = lastPart.length();

    // Compute the word range pertaining to the last prefix.
    bool notIntersectionMode;
    const WordRange wordRange = prefixToRange(lastPart.getQueryString(), notIntersectionMode);

    if (_queryParameters.useFiltering == true
         && lastPart.getQueryString().find(NOT_QUERY_SEP)      == string::npos
         && lastPart.getQueryString().find(ENHANCED_QUERY_SEP) == string::npos
         && lastPart.getQueryString().find(OR_QUERY_SEP)       == string::npos
         && splitSeparator.getOutputMode()                     == Separator::OUTPUT_MATCHES
         && wordRange.isEmptyRange()                           == false)
    {
      QueryResult* resultForFiltering = NULL;
      for (signed short i = lengthLastPart - 2; i > 0 ; --i)
      {
        string queryPrefix = query.getQueryString().substr(0, query.length() - lengthLastPart + i);
        assert(resultForFiltering == NULL);
        string queryToCheckInHistory = queryPrefix + "*";
        #ifndef NDEBUG
        log << "! search for this query in history (for filtering): " << queryToCheckInHistory << endl;
        #endif

        // CASE: prefix of last part *found* in history
        if ( (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)
                && isInHistory(queryToCheckInHistory, resultForFiltering)
                && (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED))
        {
          assert(isInHistoryConst(queryToCheckInHistory));
          setStatusOfHistoryEntry(queryToCheckInHistory, QueryResult::FINISHED | QueryResult::IN_USE);
          ++nofQueriesByFiltering;
          assert(resultForFiltering != NULL);
          result->resultWasFilteredFrom = queryToCheckInHistory;
          assert(resultForFiltering->_docIds.size() == resultForFiltering->_wordIdsOriginal.size());
          assert( resultForFiltering->_docIds.isSorted() );
          #ifndef NDEBUG
          log << "! using this Result for filtering:" << queryToCheckInHistory << endl;
          #endif
          assert((  resultForFiltering->_topCompletions.size() > 0 ) || ( resultForFiltering->_docIds.size() == 0));
          assert((  resultForFiltering->_topCompletions.size() == 0 ) || ( resultForFiltering->_docIds.size() > 0));
          assert((resultForFiltering->_docIds.size() == 0) || ( resultForFiltering->_topDocScores.size() > 0 ));
          assert((!(MODE & WITH_POS)) || resultForFiltering->_docIds.isMarkedSorted()
              || resultForFiltering->_docIds.size() == 0);
          broadHistoryTimer1.stop();
          broadHistoryTimer2.cont();
          if (resultForFiltering->isLockedForWriting)
            CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before copy and filter");
          resultForFiltering->isLockedForReading = true; // only unlockd in clean up
          if (result->isLockedForReading)
            CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "before copy and filter");
          if (result->isLockedForWriting)
            CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before copy and filter");
          result->isLockedForWriting = true;
          copyAndFilter(result->_wordIdsOriginal,
                        resultForFiltering->_docIds,
                        resultForFiltering->_wordIdsOriginal,
                        resultForFiltering->_positions,
                        resultForFiltering->_scores,
                        wordRange.firstElement(), wordRange.lastElement() ,
                        result->_docIds,
                        result->_positions,
                        result->_scores);
          if (!result->isLockedForWriting)
            CS_THROW(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after copy and filter");
          result->isLockedForWriting = false;
          broadHistoryTimer2.stop();
          broadHistoryTimer1.cont();
          assert(result->_docIds.isSorted());
          result->_docIds.markAsSorted(true); // TODO: obsolete: leave here for now for some checks
          assert(!notIntersectionMode || checkWordlistBuffer(wordRange));
          assert( &(result->_topWordIds) == _wordlistUnionBuffer);

          resultFoundByFiltering = true;
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_FILTERED_1);
          assert(isInHistoryConst(queryToCheckInHistory));
          ///////////////                  setStatusOfHistoryEntry(queryToCheckInHistory,QueryResult::FINISHED);
          break;
        }
        // END CASE: prefix of last part *found* in history
        assert(resultForFiltering == NULL);
      }
      // END LOOP over all prefixes of last part of query (candidates to filter from)
      #ifndef NDEBUG
      log << "! end of filtering " << endl;
      #endif
      broadHistoryTimer1.stop();
    }
    //
    // CASE 2.2: "NORMAL" FILTERING (check if any prefix of last part is in history)
    //
    //   *** END
    //


    broadHistoryTimer3.cont();
    if (hasManyParts
         && _queryParameters.useFiltering == true
         && lastPart .getQueryString().find(NOT_QUERY_SEP)      == string::npos
         && firstPart.getQueryString().find(ENHANCED_QUERY_SEP) == string::npos
         && lastPart .getQueryString().find(OR_QUERY_SEP)       == string::npos
         && splitSeparator.getOutputMode()                      == Separator::OUTPUT_MATCHES
         && wordRange.isEmptyRange()                            == false
         && resultFoundByFiltering                              == false
   && fuzzySearchEnabled                                  == false)
    {
      //
      // CASE 2.3: "ADVANCED" FILTERING (e.g. filter schedul* venue:* from sched* venue:*)
      //
      //   Query        : scheduling..algorithm*
      //   filter from  : schedul*..algorithm*
      //   by intersecting result("scheduling") with result("schedul* algorithm*") with separator ..
      //
      //   This can save a lot of time if the last part has many matches. For
      //   example, computing the query "schedul* venue:*" from scratch will take
      //   some time because "venue:*" has many matches. But if we have already
      //   "sched* venue:*", which has a much shorter result than "venue:*", we
      //   can filter from that.
      //
      //   Old explanations by Ingmar:
      //
      //   b: if query has many parts (approxim.algorith) and separator mode
      //   check if last part of first part is in history (approxi.algorith, approx.algorith, appro.algorith, appr.algorith, app.algorith ...)
      //   if we find appr.algori (= R1), intersect result for appr.algorith with result for approxim (= R2)  =>  (approxim).(appr.algorith)
      //   (R2).(R1)
      //
      //   ALSO WORKS WITH XML SEPARATOR !!!!!
      //   b: if query has many parts (title=appr) and separator mode is NOT xml mode
      //   check if last part of first part is in history (titl=appr, tit=appr, ti=appr, t=appr)
      //   if we find tit=appr, intersect result for tit=appr with result for title (separator is space)
      //   (title)=(tit=appr)
      //
      QueryResult* result1ForAdvancedFiltering = NULL;
      QueryResult* result2ForAdvancedFiltering = NULL;
      Query firstPartOfFirstPart;
      Query lastPartOfFirstPart;
      Separator splitSeparator2;
      firstPart.splitAtLastSeparator(&firstPartOfFirstPart, &lastPartOfFirstPart, &splitSeparator2);
      log << IF_VERBOSITY_HIGHER
          << "! split query in case: advanced filtering I; separator is " << splitSeparator2.infoString() << endl;
      signed char sepIndexFirstPart = splitSeparator2._separatorIndex;
      #ifndef NDEBUG
      log << "firstPartOfFirstPart: " << firstPartOfFirstPart << ", lastPartofFirstPart : " << lastPartOfFirstPart << endl;
      log << "splitSeparator Index: \"" << splitSeparator2._separatorIndex << "\"" << endl;
      log << "splitSeparator separator String: \"" << splitSeparator2._separatorString << "\"" << endl;
      #endif
      if (sepIndexFirstPart == (signed char) -1) CS_THROW(Exception::CONTAINS_INVALID_SEPARATOR, "");
      if (lastPartOfFirstPart.getQueryString() == "*") CS_THROW(Exception::SINGLE_STAR_NOT_ALLOWED, "");

      // LOOP: OVER LAST BUT ONE PREFIX (cutting off letter by letter and checking in history)
      for (signed short i = lastPartOfFirstPart.length() - 2; i > 0; --i)
      {
        assert(!resultFoundByFiltering);
        string queryPrefix = firstPart.getQueryString().substr(0, firstPart.length() - lastPartOfFirstPart.length() + i);

        // If OR separator encountered, must not proceed further with advanced
        // filtering (was a bug, fixed 11Feb07)
        //
        //   For example, the following series of queries used to fail on DBLP:
        //
        //   ESA author:*
        //   ESA|ICALP author:*
        //
        //   because the result for the second query was wrongly
        //   advanced-filtered from the result for the first query
        //   (with empty history the second query worked alright)
        //
        if (queryPrefix.length() > 0 && queryPrefix[queryPrefix.length() - 1] == OR_QUERY_SEP)
        {
          log << IF_VERBOSITY_HIGH
              << "! encountered OR separator, must not try advanced filtering beyond here (was a bug)" << endl;
          break;
        }

        assert(result1ForAdvancedFiltering == NULL);
        string queryToCheckInHistory = queryPrefix + "*" + splitSeparator._separatorString + lastPart.getQueryString();
        log << IF_VERBOSITY_HIGHER
            << "! advanced filtering, check this query in history: \"" << queryToCheckInHistory << "\"" << endl;
        // log << "! queryPrefix: " << queryPrefix
        //    << ", splitSeparator._separatorString :" << splitSeparator._separatorString
        //    << ", last Part: " << lastPart << endl;

        // CASE: found query in history (for advanced filtering)
        if ((getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)
          && isInHistory(queryToCheckInHistory, result1ForAdvancedFiltering))
        {
          if (!result1ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
          assert(isInHistoryConst(queryToCheckInHistory));
          setStatusOfHistoryEntry(queryToCheckInHistory, QueryResult::FINISHED | QueryResult::IN_USE);
          assert(result1ForAdvancedFiltering);
          log << IF_VERBOSITY_HIGHER
              << "! advanced filtering, found this query in history: \"" << queryToCheckInHistory << "\"" << endl;
          broadHistoryTimer.stop();
          broadHistoryTimer3.stop();
          try {
            processComplexQuery(firstPart, result2ForAdvancedFiltering);
                                // k1_docs, k2_words, filterResults, useLinearWordlistIntersection);
            #ifndef NDEBUG
            cout << "! returned from recursive call (2)" << endl;
            #endif
            assert(isInHistoryConst(firstPart));
            setStatusOfHistoryEntry(firstPart, QueryResult::FINISHED);
            setStatusOfHistoryEntry(firstPart, QueryResult::FINISHED | QueryResult::IN_USE);
          }
          catch (Exception& e)
          {
            log << IF_VERBOSITY_HIGH
                << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
                << "\"" << query << "\" from history" << endl;
            if ((e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT) && (getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION))
            {
              assert(getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION);
            }
            result2ForAdvancedFiltering = NULL;
            CS_RETHROW(e);
              // log << "! " << e.getFullErrorMessage() << endl;
              // CS_THROW(Exception::ERROR_PASSED_ON, e.getErrorMessage());
          }
          if (!result2ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "in result from advanced filtering");
          broadHistoryTimer.cont();
          broadHistoryTimer3.cont();
          assert(result2ForAdvancedFiltering);

          WordRange wordRange(-1,1); // infinite range

          // TODO: ONCE INTERSECT AND UNITE ARE CHANGED, THIS CAN BE REMOVED
          broadHistoryTimer3.stop();
          broadHistoryTimer.stop();
          assert(result->_docIds.size() == 0);

          // TODO: What kind of error is this? Throw appropriate exception
          if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
          {
            CS_THROW(Exception::OTHER, "something bad happened with result during advanced filtering");
            log << " ******************** ERROR ********** " << endl;
            exit(1);
          }
          reserveBuffersAndResetPointers(*result);
          broadHistoryTimer.cont();
          broadHistoryTimer3.cont();
          //                  clearBuffer(); // uncommented 05Jun07
          if ((!result2ForAdvancedFiltering->isEmpty()) && (!result1ForAdvancedFiltering->isEmpty() ))
          {
            broadHistoryTimer.stop();
            broadHistoryTimer3.stop();
            assert((!(MODE & WITH_SCORES)) || (result2ForAdvancedFiltering->_scores.size() == result2ForAdvancedFiltering->_docIds.size()));
            log << IF_VERBOSITY_HIGH << "! new intersect called from case \"advanced filtering I\"" << endl;
            intersectTwoPostingLists
             (*result2ForAdvancedFiltering,
              *result1ForAdvancedFiltering,
              *result,
              splitSeparator,
              _queryParameters.docScoreAggDifferentQueryParts,
              wordRange);
            broadHistoryTimer.cont();
            broadHistoryTimer3.cont();
          }
          assert(result->_docIds.isSorted());
          result->_docIds.markAsSorted(true);

          resultFoundByFiltering = true;
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_FILTERED_2);
          // NEW 19Sep06 (Holger): important for one-line summary (was left empty before, probably an oversight, was it?)
          result->resultWasFilteredFrom = queryToCheckInHistory;
          assert(isInHistoryConst(queryToCheckInHistory));
          assert(isInHistoryConst(firstPart));
          break;
        }
        // END Case: found query in history (for advanced filtering)

        assert(!(isInHistoryConst(queryToCheckInHistory) && (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)));
        assert(result1ForAdvancedFiltering == NULL);
      }
      // end LOOP: OVER LAST BUT ONE PREFIX (cutting off letter by letter and checking in history)
      broadHistoryTimer3.stop();
      broadHistoryTimer4.cont();
      broadHistoryTimer.cont();
      //
      // CASE 2.3: "ADVANCED" FILTERING (e.g. filter schedul* venue:* from sched* venue:*)
      //
      //   *** END
      //


      //
      // CASE 2.4: "ADVANCED" FILTERING II (e.g., filter approx*..algo* venue:* from approx venue:*)
      //
      //   Query       : approx*..algo* venue:*
      //   filter from : approx* venue:*
      //   by intersecting result(approx*..algo*) with result(approx venue:*)
      //
      //   TODO: explain what exactly is the difference to "ADVANCED" FILTERING I
      //
      if (resultFoundByFiltering == false
          && firstPartOfFirstPart.length() > 0
          && lastPart.getQueryString().find(NOT_QUERY_SEP) == string::npos)
      {
        assert(!resultFoundByFiltering);
        assert(result1ForAdvancedFiltering == NULL);
        assert(fixed_separators._separators[SAME_DOC]._separatorString == " ");
        // Note: always have to use SAME_DOC separator here; TODO: explain why
        string queryToCheckInHistory = firstPartOfFirstPart.getQueryString()
          + fixed_separators._separators[SAME_DOC]._separatorString
          + lastPart.getQueryString();
        #ifndef NDEBUG
        log << "! search for this query in history (for advanced shortcut): " << queryToCheckInHistory << endl;
        #endif
        assert(!result1ForAdvancedFiltering);

        // Case: found query in history (for advanced shortcut)
        if ( (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED)
                && isInHistory(queryToCheckInHistory, result1ForAdvancedFiltering)
                && (getStatusOfHistoryEntry(queryToCheckInHistory) & QueryResult::FINISHED) )
        {
          if (!result1ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
          assert(isInHistoryConst(queryToCheckInHistory));
          setStatusOfHistoryEntry(queryToCheckInHistory,QueryResult::FINISHED|QueryResult::IN_USE);
          #ifndef NDEBUG
          log << "! FOUND this query in history (for advanced shortcut): " << queryToCheckInHistory << endl;
          log << "! combining with result for \"" << firstPart << "\" (for advanced shortcut): " << queryToCheckInHistory << endl;
          #endif
          assert(result1ForAdvancedFiltering);
          assert(!result2ForAdvancedFiltering);
          broadHistoryTimer.stop();
          broadHistoryTimer4.stop();
          try
          {
            processComplexQuery(firstPart, result2ForAdvancedFiltering);
                                // DEFAULT_NOF_HITS_TO_COMPUTE,DEFAULT_NOF_COMPLETIONS_TO_COMPUTE, true,useLinearWordlistIntersection);
            #ifndef NDEBUG
            cout << "! returned from recursive call (3)" << endl;
            #endif
            assert(isInHistoryConst(firstPart));
            setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED);
            setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED|QueryResult::IN_USE);
          }
          catch (Exception& e)
          {
            log << IF_VERBOSITY_HIGH
                << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
                << "\"" << query << "\" from history" << endl;
            if ((e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT) && (getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION))
            {
              assert(getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION);
            }
            result2ForAdvancedFiltering = NULL;
            CS_RETHROW(e);
              // log << "! " << e.getFullErrorMessage() << endl;
              // CS_THROW(Exception::ERROR_PASSED_ON, e.errorMessage());
          }
          if (!result2ForAdvancedFiltering->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
          broadHistoryTimer4.cont();
          broadHistoryTimer.cont();
          assert(result2ForAdvancedFiltering);

          WordRange wordRange(-1,1); // infinite range

          // TODO: ONCE INTERSECT AND UNITE ARE CHANGED, THIS CAN BE REMOVED
          broadHistoryTimer4.stop();
          broadHistoryTimer.stop();
          assert(result->_docIds.size() == 0);

          // TODO: What kind of error is this? Throw appropriate exception
          if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
          {
            CS_THROW(Exception::OTHER, "something bad happened with result during advanced filtering II");
            log << " ******************** ERROR ********** " << endl;
            exit(1);
          }
          reserveBuffersAndResetPointers(*result);
          broadHistoryTimer.cont();
          broadHistoryTimer4.cont();
          if ((!result2ForAdvancedFiltering->isEmpty()) && (!result1ForAdvancedFiltering->isEmpty() ))
          {
            broadHistoryTimer4.stop();
            broadHistoryTimer.stop();
            assert((!(MODE & WITH_SCORES)) || (result2ForAdvancedFiltering->_scores.size() == result2ForAdvancedFiltering->_docIds.size()));
            log << "! NEW INTERSECT: called from case \"advanced filtering II\"" << endl;
            intersectTwoPostingLists
             (*result2ForAdvancedFiltering,
              *result1ForAdvancedFiltering,
              *result,
              splitSeparator,
              _queryParameters.docScoreAggDifferentQueryParts,
              wordRange);
            broadHistoryTimer.cont();
            broadHistoryTimer4.cont();
          }
          assert(isInHistoryConst(firstPart));
          assert(isInHistoryConst(queryToCheckInHistory));
          assert(result->_docIds.isSorted());
          result->_docIds.markAsSorted(true);

          resultFoundByFiltering = true;
          result->resultWasFilteredFrom = "[SHORTCUT]";
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_FILTERED_3);
        }
        // end Case: found query in history (for advanced shortcut)
      }
      //
      // CASE 2.4: "ADVANCED" FILTERING II (e.g., filter approx*..algo* venue:* from approx venue:*)
      //
      //    *** END
      //
      broadHistoryTimer4.stop();
    }
    // TODO: this is the closing } of CASE 2.3 and 2.4 together (advanced filtering I and II)


    //
    // CASE 2.4: NO FILTERING (none of "NORMAL", "ADVANCED", "ADVANCED II" above worked)
    //
    //   Recurse on first part, then intersect with last part
    //
    broadHistoryTimer5.cont();
    broadHistoryTimer.cont();
    log << IF_VERBOSITY_HIGHER << "! no filtering/shortcut successful " << endl;
    if (hasManyParts && !resultFoundByFiltering)
    {
      //
      // CASE 2.4.1: Processing first part of query recursively
      //
      QueryResult *resultFirstPart = NULL;
      broadHistoryTimer.stop();
      broadHistoryTimer5.stop();
      try
      {
        processComplexQuery(firstPart, resultFirstPart);
                            // k1_docs, k2_words, filterResults,useLinearWordlistIntersection);
        #ifndef NDEBUG
        cout << "! returned from recursive call (4)" << endl;
        #endif
        assert(resultFirstPart->_status & QueryResult::FINISHED);
        assert(isInHistoryConst(firstPart));
        setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED);
        setStatusOfHistoryEntry(firstPart,QueryResult::FINISHED|QueryResult::IN_USE);
        assert( !(MODE & WITH_SCORES) || (resultFirstPart->_scores.size() == resultFirstPart->_docIds.size() ));
      }
      catch (Exception& e)
      {
        log << IF_VERBOSITY_HIGH
            << "! " << (e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT ? "Removing(?) " : "Not removing(?) ")
            << "\"" << query << "\" from history" << endl;
        if ((e.getErrorCode() != Exception::HISTORY_ENTRY_CONFLICT) && (getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION))
        {
          assert(getStatusOfHistoryEntry(firstPart) == QueryResult::UNDER_CONSTRUCTION);
        }
        resultFirstPart = NULL;
        CS_RETHROW(e);
          // log << "! " << e.getFullErrorMessage() << endl;
          // CS_THROW(Exception::ERROR_PASSED_ON, e.errorMessage());
      }  // end: catch Exception
      if (!resultFirstPart->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
      assert(resultFirstPart);
      assert(resultFirstPart->_status & QueryResult::FINISHED);
      assert( !(MODE & WITH_SCORES) || (resultFirstPart->_scores.size() == resultFirstPart->_docIds.size() ));


      //
      // CASE 2.4.2: Intersect result of first part with result for last part
      //

      // TODO: once intersect and unite are changed, this can be removed
      assert(result->_docIds.size() == 0);
      // TODO: What kind of error is this? Throw appropriate exception
      if ((result->_status != QueryResult::UNDER_CONSTRUCTION) || (result->_docIds.size() != 0 ))
      {
        CS_THROW(Exception::OTHER, "something bad happened with result during normal processing");
        log << " ******************** ERROR ********** " << endl; exit(1);
      }
      reserveBuffersAndResetPointers(*result);
      broadHistoryTimer.cont();
      broadHistoryTimer5.cont();
      if (resultFirstPart->isEmpty() == false)
      {
        if (MODE & WITH_POS) assert(resultFirstPart->_docIds.isSorted());
        assert((*resultFirstPart).isEmpty() || !(*resultFirstPart).isEmpty());
        #ifndef NDEBUG
        log << "! Computing result for last Part (" << lastPart <<") with candidates from first part" << endl << flush;
        #endif
        broadHistoryTimer.stop();
        broadHistoryTimer5.stop();
        assert(resultFirstPart->_status & QueryResult::FINISHED);
        assert( resultFirstPart->_docIds.isFullList() || resultFirstPart->_docIds.size() > 0 );
        if (MODE & WITH_SCORES) assert(resultFirstPart->_scores.size() == resultFirstPart->_docIds.size());
        if (MODE & WITH_POS)    assert(resultFirstPart->_positions.size() == resultFirstPart->_docIds.size());
        //
        // TODO: deal with case here that last part of query is in history. This
        // was previously dealt with in HybCompleter::processBasicQuery, which
        // is the inappropriate place to do so.
        //

        //
        // CASE 2.4.2.1: Last part is in history
        //
        QueryResult* resultLastPart = NULL;
        if ((getStatusOfHistoryEntry(lastPart) & QueryResult::FINISHED) && isInHistory(lastPart, resultLastPart))
        {
          log << IF_VERBOSITY_HIGH << "! last part of query is in history" << endl;
          setStatusOfHistoryEntry(lastPart, QueryResult::FINISHED | QueryResult::IN_USE);
          CS_ASSERT(resultLastPart != NULL);
          intersectTwoPostingLists
           (*resultFirstPart,
            *resultLastPart,
            *result,
            splitSeparator,
            _queryParameters.docScoreAggDifferentQueryParts);
          result->setHowResultWasComputed(QueryResult::FROM_HISTORY_INTERSECT);
          result->_query = query;
          result->_prefixCompleted = lastPart.getQueryString();
        }

        //
        // CASE 2.4.2.2: Last part is not in history, output mode is OUTPUT_MATCHES
        //
        else if (splitSeparator.getOutputMode() == Separator::OUTPUT_MATCHES)
        {
          log << IF_VERBOSITY_HIGH << "! last part of query is not in history; normal output mode" << endl;
          processBasicQuery(*resultFirstPart, firstPart, lastPart, splitSeparator, *result);
        }

        //
        // CASE 2.4.2.3: Last part is not in history, output mode is OUTPUT_NON_MATCHES or OUTPUT_ALL
        //
        else
        {
          log << IF_VERBOSITY_HIGH << "! last part of query is not history; special output mode" << endl;
          processQuery(lastPart, resultLastPart);
          CS_ASSERT(resultLastPart != NULL);
          intersectTwoPostingLists
           (*resultFirstPart,
            *resultLastPart,
            *result,
            splitSeparator,
            _queryParameters.docScoreAggDifferentQueryParts);
        }

        broadHistoryTimer.cont();
        broadHistoryTimer5.cont();
      }
      assert(isInHistoryConst(firstPart));
      result->setHowResultWasComputed(QueryResult::NORMAL);
    }
    //
    // CASE 2.4: NO FILTERING (none of "NORMAL", "ADVANCED", "ADVANCED II" above worked)
    //
    //   *** END
    //

    //
    // CASE 2.5: ONE-WORD QUERY and not in history
    //
    else if (resultFoundByFiltering == false)
    {
      // pass full list to completionsForWordRange
      QueryResult fullResult(true); // / THIS SEEMS TO FAIL ONCE IN A WHILE ...
      Separator fullSeparator = Separator("",pair<signed int, signed int>(-1,-1),FULL);
      broadHistoryTimer5.stop();
      broadHistoryTimer.stop();
      assert( fullResult._docIds.isFullList() || fullResult._docIds.size() > 0 );
      if (MODE & WITH_SCORES) assert(fullResult._scores.size() == fullResult._docIds.size());
      if (MODE & WITH_POS) assert(fullResult._positions.size() == fullResult._docIds.size());
      assert(fullResult._status & QueryResult::FINISHED);
      processBasicQuery(fullResult, firstPart, lastPart, fullSeparator, *result); // , NULL, useLinearWordlistIntersection); // NULL: 'Last part' cannot be read from history
      assert(result->_docIds.size() == result->_wordIdsOriginal.size());
      broadHistoryTimer.cont();
      broadHistoryTimer5.cont();
      result->setHowResultWasComputed(QueryResult::NORMAL);
    }
    //
    // CASE 2.5: ONE-WORD QUERY and not in history
    //
    //   *** END
    //

    //
    // 2.6 SOME CHECKS ON RESULT
    //
    assert(result == isInHistory(query));
    assert(result->_docIds.isSorted());
    result->_docIds.markAsSorted(true); // TODO: OBSOLETE: LEAVE HERE FOR NOW FOR SOME CHECKS
    broadHistoryTimer.stop();
    broadHistoryTimer5.stop();

    //
    // 2.7 TOP-K HITS AND COMPLETIONS
    //
    assert((result->_status == QueryResult::UNDER_CONSTRUCTION) || (notIntersectionMode));
    assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
    assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
    if (result->_status == QueryResult::UNDER_CONSTRUCTION)
    {
      if (result->isLockedForReading)
        CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "before topMatchesFromAllMatches");
      if (result->isLockedForWriting)
        CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before topMatchesFromAllMatches");
      result->isLockedForWriting = true;
      computeTopHitsAndCompletions(*result);
      /*
      topMatchesFromAllMatches(*result,
          WITHIN_DOC_AGGREGATION, BETWEEN_DOCS_AGGREGATION,
          WITHIN_COMPLETION_AGGREGATION, BETWEEN_COMPLETIONS_AGGREGATION,
          k1_docs, k2_words);
      */
      if (!result->isLockedForWriting)
        CS_THROW(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after topMatchesFromAllMatches");
      result->isLockedForWriting = false;
    }
    broadHistoryTimer.cont();
    broadHistoryTimer5.cont();
    assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
    assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
    assert( ( result->_docIds.size() == 0) || ( result->_topWordScores.size() > 0  ));
    assert(result->_docIds.isSorted());
    assert((!(MODE & WITH_POS)) || result->_docIds.isMarkedSorted()
        || result->_docIds.size() == 0);

    #ifndef NDEBUG
    assert(result->_topWordDocCounts.size() == result->_topWordOccCounts.size());
    for (unsigned long i =0 ; i<result->_topWordDocCounts.size() ; i++)
    {
      assert (result->_topWordDocCounts.operator[](i) <=  result->_topWordOccCounts.operator[](i));
    }
    #endif
    broadHistoryTimer5.stop();
    broadHistoryTimer6.cont();

    //
    // 2.8 CREATE THE STRINGS TO DISPLAY; TODO: what does that mean?
    //
    //   Does this have size default_nof_completions_to_compute ????
    //
    if (result->_status == QueryResult::UNDER_CONSTRUCTION)
    {
      assert((!(MODE & WITH_POS )) || (result->_docIds.size() == result->_positions.size()));
      assert((!(MODE & WITH_SCORES )) || (result->_docIds.size() == result->_scores.size()));
      result->setCompletions(result->_topWordScores, result->_topWordDocCounts,
          result->_topWordOccCounts, *_vocabulary);
    }
    broadHistoryTimer6.stop();

    //
    // 2.9 FINALIZE ENTRY IN HISTORY
    //
    broadHistoryTimer7.cont();
    assert(result == isInHistory(query));
    if (result->_status == QueryResult::UNDER_CONSTRUCTION)
    {
      if (MODE & WITH_POS )    assert(result->_docIds.size() == result->_positions.size());
      if (MODE & WITH_SCORES ) assert(result->_docIds.size() == result->_scores.size());
      if (result->isLockedForReading)
        CS_THROW(Exception::RESULT_LOCKED_FOR_READING, "before freeExtraSpace");
      if (result->isLockedForWriting)
        CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "before freeExtraSpace");
      result->isLockedForWriting = true;
      result->freeExtraSpace();
      if (!result->isLockedForWriting) CS_THROW
        (Exception::RESULT_NOT_LOCKED_FOR_WRITING, "after freeExtraSpace");
      result->isLockedForWriting = false;
      // CHANGE(hagn, 28Jan11):
      //history.finalizeSize(query.getQueryString());
      finalizeSizeOfHistory(query);
      if (MODE & WITH_POS)    assert(result->_docIds.size() == result->_positions.size());
      if (MODE & WITH_SCORES) assert(result->_docIds.size() == result->_scores.size());
      assert(isInHistoryConst(query));
      setStatusOfHistoryEntry(query, QueryResult::FINISHED);
      setStatusOfHistoryEntry(query, QueryResult::FINISHED | QueryResult::IN_USE);
    }
    assert(result); // there will always be a (possibly empty) result
    assert(history.check(DO_LOCK, false)); // true means: also check status (= QueryResult::FINISHED) of entries
    if (MODE & WITH_POS) assert(result->_docIds.size() == 0 || result->_docIds.isMarkedSorted());
    broadHistoryTimer.stop();
    broadHistoryTimer7.stop();
  }
  //
  // CASE 2: QUERY NOT IN HISTORY (not even being computed)
  //
  //   *** END
  //


  //
  // CASE 3: QUERY WAS IN HISTORY BUT NOT YET FINISHED
  //
  //   Simply throws an exception.
  //
  //   Note: in CASE 0 at the beginning of this (long) function, there was some
  //   (preliminary) code for waiting a little if a query result is just being
  //   computed.
  //
  else
  {
    CS_ASSERT(getStatusOfHistoryEntry(query) != QueryResult::DOES_NOT_EXIST);
    ostringstream os;
    os << "history status: " << int(getStatusOfHistoryEntry(query));
    CS_THROW(Exception::HISTORY_ENTRY_CONFLICT, os.str());
  }


  //
  // CHECK THE RESULT (check equal list lenghts, QueryResult::FINISHED, etc.)
  //
  if (result->check() == false) CS_THROW(Exception::BAD_QUERY_RESULT, "");
  if (result != NULL) assert(result->_status & QueryResult::FINISHED);
  if (MODE & WITH_SCORES)  assert(result->_scores.size() == result->_docIds.size());

  log << AT_END_OF_METHOD << "; query was \"" << query << "\"; result has " << result->getSize() << " postings" << endl;
  if (showQueryResult) result->show();

  return 0;
}  // end: processComplexQuery_NEW


// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::processQuery_waitForQueryResult(
    const Query& query,
    unsigned int usecs2Wait)
{
  unsigned int usecsSlept = 0;
  unsigned int usecsDelta = 10*1000;
  while (getStatusOfHistoryEntry(query) & QueryResult::UNDER_CONSTRUCTION)
  {
    usleep(usecsDelta);
    usecsSlept += usecsDelta;
    if (usecsSlept >= usecs2Wait) break;
  }
  if (usecsSlept > 0)
  {
    log << EMPH_ON << "! NEW: Waited " << usecsSlept / 1000
        << " milliseconds for history entry from other thread"
        << (getStatusOfHistoryEntry(query) & QueryResult::FINISHED
             ? " -> IS FINISHED NOW!" : " -> STILL UNDER CONSTRUCTION")
        << EMPH_OFF << endl;
  }
}


// _____________________________________________________________________________
template <unsigned char MODE>
unsigned int CompleterBase<MODE>::processQuery_NeedToRecompute(
    const QueryResult& result) const
{
  unsigned int retval = 0;
  DocId nofDocIdsFound = (DocId)(result._topDocIds.size());
  WordId nofWordIdsFound = (WordId)(result._topWordIds.size());
   
  // Recompute hits? - Set 1. bit.
  retval |=
    (result.nofTotalHits > nofDocIdsFound &&
     _queryParameters.nofTopHitsToCompute > nofDocIdsFound)
    << 0;
  // Rerank hits? - Set 2. bit.
  retval |= 
    (_queryParameters.howToRankDocs !=
     result._queryParameters.howToRankDocs
     ||
     _queryParameters.fuzzyDamping !=
     result._queryParameters.fuzzyDamping
     ||
     _queryParameters.docScoreAggSameCompletion !=
     result._queryParameters.docScoreAggSameCompletion
     ||
     _queryParameters.docScoreAggDifferentCompletions !=
     result._queryParameters.docScoreAggDifferentCompletions)
    << 1;
  // Resort hits? - Set 3. bit.
  retval |= 
    (_queryParameters.sortOrderDocs !=
     result._queryParameters.sortOrderDocs)
    << 2;
  // Recompute completions? - Set 4. bit.
  retval |=
    (result.nofTotalCompletions > nofWordIdsFound &&
     (signed)_queryParameters.nofTopCompletionsToCompute > nofWordIdsFound)
    << 3;
  // Rerank completions? - Set 5. bit.
  retval |=
    (_queryParameters.howToRankWords !=
     result._queryParameters.howToRankWords
     ||
     _queryParameters.fuzzyDamping !=
     result._queryParameters.fuzzyDamping
     ||
     _queryParameters.wordScoreAggSameDocument !=
     result._queryParameters.wordScoreAggSameDocument
     ||
     _queryParameters.wordScoreAggDifferentDocuments !=
     result._queryParameters.wordScoreAggDifferentDocuments)
    << 4;
  // Resort completions? - Set 6. bit.
  retval |=
    (_queryParameters.sortOrderWords !=
     result._queryParameters.sortOrderWords)
    << 5;
  return retval;
}


// _____________________________________________________________________________
template <unsigned char MODE>
void CompleterBase<MODE>::processQuery_LogRecomputeReasons(unsigned int reason)
{
  log << IF_VERBOSITY_HIGH
      << "! needToRecomputeHits = "
      << (reason & RECOMPUTE_HITS)
      << ", needToRecomputeCompletions = "
      << (reason & RECOMPUTE_COMPLETIONS)
      << ", needToRerankHits = "
      << (reason & RERANK_HITS)
      << ", needToRerankCompletions = "
      << (reason & RERANK_COMPLETIONS)
      << ", needToResortHits = "
      << (reason & RESORT_HITS)
      << ", needToResortCompletions = "
      << (reason & RESORT_COMPLETIONS)
      << endl;
}

//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template class CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>;

