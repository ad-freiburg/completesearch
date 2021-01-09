#ifndef __COMPLETER_BASE_H__
#define __COMPLETER_BASE_H__

#include "Globals.h"
#include "History.h"
#include "Query.h"
#include "QueryParameters.h"
#include "Separator.h"
#include "WordRange.h"
#include "File.h"
#include "Vocabulary.h"
#include "MetaInfo.h"
#include "CompressionAlgorithm.h"
#include "Timer.h"
#include "assert.h"
#include "DocList.h"
#include "WordList.h"
#include "WordsFile.h"
#include "ScoreAggregators.h"
#include "ConcurrentLog.h"
#include <unordered_set>
#include <gtest/gtest.h>

#include "../fuzzysearch/FuzzySearcher.h"

extern bool doSortHits;

//! Methods for enhanced query parsing (implemented in CompleterBase.cpp)
void rewriteJoinBlocks(Query& query);
void splitPartOfEnhancedQueryBlock(const Query& query, vector<string>& parts);
void splitPartOfOrQueryBlock(const Query& query, vector<string>& parts);

//! Needed for CompleterBase default constructor below
extern Vocabulary emptyVocabulary;
extern TimedHistory emptyHistory;
extern MetaInfo emptyMetaInfo;
extern FuzzySearch::FuzzySearcherUtf8 nullFuzzySearcher;

//! Completer Base Class; provides the basic functionality for query processing (compute hits and completions for the last query word)
/*
 *    The functionality of this abstract class is based on a method for
 *    processing context-sensitive prefix search & completion queries, or just
 *    prefix completion queries in short, according to Bast, Weber: The
 *    CompleteSearch Engine ... CIDR'06 and SIGIR'06. 
 *
 *    A concrete subclass, e.g. HYBCompleter, provides an implementation of this
 *    method, called allMatchesForLastPartAndCandidates (TODO: bad name).
 *
 *    Apart from this, most of the functionality is already implemented in this
 *    abstract class, in particular: translating queries to a chain of prefix
 *    completion queries, handling "join" queries, "or" queries, and "not"
 *    queries, score aggregation and ranking, handling of the result history
 *    (cache), filtering, etc.
 */
template<unsigned char MODE>
class CompleterBase
{

    //  TABLE OF CONTENTS
    //  *****************
    //
    //  0. MAIN VARIABLES
    //  1. CONSTRUCTORS AND DESTRUCTOR
    //  2. VARIOUS ACCESS METHODS
    //  3. QUERY PROCESSING
    //  4. BUFFERS
    //  5. TIMER, COUNTER, STATISTICS
    //  6. HISTORY HANDLING

    //  0. MAIN VARIABLES
    //  *****************

    //  NOTE: there are more later, e.g. buffer, timer, counter, ...

  public:

    //! The file holding the index (inverted lists for INV, blocks for HYB, etc.)
    File _indexStructureFile;

    //! The vocabulary (all words, in lexicographic order)
    Vocabulary* _vocabulary;
    //vector<string>& _vocabulary;

    //! Meta info: number of documents, number of words, etc.
    MetaInfo* _metaInfo;

    //! Log object, which can be used like cout but handles concurrent access properly
    mutable ConcurrentLog log;

    // History (cache) of query results.
    TimedHistory* history;

    // Object providing fuzzy search functionality.
    FuzzySearch::FuzzySearcherBase* _fuzzySearcher;

    //! TODO: what's this?
    pair<signed int, signed int> currentIntersectionWindow;

    // NEW (baumgari 09Mar14)
    //! Stores the status code of the request, which is especially important in
    //! in case of errors. The status code is used the return the correct
    //! header.
    int statusCode;

  protected:

    //! Query parameters;
    /*!
     *   Note: maybe more intuitive to pass them to each call of processQuery
     *   since it's really something that changes on a per query basis (sometimes
     *   sort like this, sometimes like that, sometimes score like this,
     *   sometimes like that), and not so much a thing that a pertains to a given
     *   completer.
     *
     *   Note: must be protected, because HYBCompleter needs to acces them. TODO:
     *   better write an access function.
     */
    QueryParameters _queryParameters;

    // NEW(bast, 29Jan10): The query processed by this completer object. Note that
    // each query is processed by its own completer object, also in the
    // multithreaded case. This makes sense, because also all the (per-query)
    // timers and statistics are part of the CompleterBase class.
    Query _query;
    Query _queryRewrittenForHighlighting;

    // NEW(bast, 21Jan10): See explanation at the beginning of
    // CompleterBase::processBasicQuery, where this is initialized to -1.
    WordId _lastBestMatchWordId;

    //! How to rank words  NEW 01Dec07
    //enum HowToRankWordsEnum { RANK_WORDS_BY_SCORE, RANK_WORDS_BY_ID } _howToRankWords;

    //! How to rank documents  NEW 01Dec07
    //enum HowToRankDocsEnum { RANK_DOCS_BY_SCORE, RANK_DOCS_BY_ID } _howToRankDocs;


    //  1. CONSTRUCTORS AND DESTRUCTOR
    //  ******************************

  public:

    //! Default constructor.
    CompleterBase()
    {
      _vocabulary = &emptyVocabulary;
      history = &emptyHistory;
      _metaInfo = &emptyMetaInfo;
      _fuzzySearcher = &nullFuzzySearcher;
      _doclistIntersectionBuffer = NULL;
      _wordlistIntersectionBuffer = NULL;
      _positionlistIntersectionBuffer = NULL;
      _scorelistIntersectionBuffer = NULL;
      _doclistUnionBuffer = NULL;
      _wordlistUnionBuffer = NULL;
      _topDocScoresBuffer = NULL;
      _lastBlockScoresBuffer = NULL;
      _topWordScoresBuffer = NULL;
      _topWordDocCountsBuffer = NULL;
      _topWordOccCountsBuffer = NULL;
      _compressionBuffer = NULL;
    }
    /*
     CompleterBase(History& passedHistory) : history(passedHistory) { _compressionBuffer = NULL; }// vectorError = 0;}
     CompleterBase() : errorStatus(0) { _compressionBuffer = NULL; }// vectorError = 0;}
     CompleterBase(vector<string> dummyVocabulary=vector<string>(0))
     : _vocabulary(dummyVocabulary), errorStatus(0) { _compressionBuffer = NULL; vectorError = 0;}
     */

    //! Constructor (from given history, vocabulary, index, and metainfo)
    CompleterBase(TimedHistory* history, Vocabulary* vocabulary,
        const string& indexFileName, MetaInfo* metaInfo,
        FuzzySearch::FuzzySearcherBase* fuzzySearcher);

    //! Copy constructor, TODO: needed where?
    CompleterBase(const CompleterBase& orig);

    //! Default destructor (does nothing but required for abstract classes)
    virtual ~CompleterBase();

    //! Build index from .words file, must be implemented by subclass; TODO: why not made pure?
    virtual void buildIndex(string wordsFileName, string indexFileName,
        string vocFileName, string format)
    { }

    //  1. VARIOUS ACCESS METHODS
    //  *************************

  public:

    // Dump object to string.
    std::string asString(void) const;

    //! Return the mode of this completer (with or without positions, with or without scores, etc.)
    static unsigned char mode();

    //! Get handle on index file (needed by Holger for test-compression)
    File& getIndexStructureFile()
    {
      return _indexStructureFile;
    }

    //! For debugging
    virtual void printAllListLengths(){ CS_THROW(Exception::NOT_YET_IMPLEMENTED, "abstract"); }
    // NEW 09Aug07 (Holger): I made the class non-abstract, so that I can compile
    // something in CompleterBase.cpp (see the dummy function at the top there)
    // = 0;

    //! Show meta info: number of documents, number of words, etc.
    void showMetaInfo()
    {
      _metaInfo->show();
    }

    // Set query parameters.
    void setQueryParameters(const QueryParameters& queryParameters)
    {
      _queryParameters = queryParameters;
    }

    // Set and get query.
    void setQuery(const Query& query)
    {
      _query = query;
      _queryRewrittenForHighlighting = query;
    }
    Query getQuery()
    {
      return _query;
    }
    Query getQueryRewrittenForHighlighting()
    {
      // Encode quoted parts. See CompleterBase.cpp:237 8Feb13.
      _queryRewrittenForHighlighting.setQueryString(
        encodeQuotedQueryParts(_queryRewrittenForHighlighting.getQueryString())
       );
      return _queryRewrittenForHighlighting;
    }

    //! Set how to rank words
    //void setHowToRankWords(int mode) { _howToRankWords = (HowToRankWordsEnum)(mode); }

    //! Set how to rank docs
    //void setHowToRankDocs(int mode) { _howToRankDocs = (HowToRankDocsEnum)(mode); }


    //  2. QUERY PROCESSING
    //  *******************

  public:

    //! Process query; method to be called from outside
    /*
     *   TODO: 13Jul2006: introduce possibility to ONLY find large Buffer
     *   (if k1_docs==k2_words==-1); TODO: what the hell does that mean?
     *
     *   NEW 14Oct07 (Holger): removed const& from Query, why?
     */
    void processQuery(const Query& query, QueryResult*& result);
    unsigned int processQuery_NEW(const Query&, QueryResult*&);

  private:

   // processQuery helper methods.

   void processQuery_waitForQueryResult(const Query& query,
                                        unsigned int usecs2Wait);
    
    // Determine if the given QueryResult needs recomputation.
    // Returns true if for one or more changes in the query parameters
    // compared to those used when the result was computed and stored in the
    // history
    //
    //   - larger value for nofTopHitsToCompute
    //   - larger value for nofTopCompletionsToCompute
    //   - different value of howToRankDocs
    //   - different value of howToRankWords
    //   - different value of sortOrderDocs
    //   - different value of sortOrderWords
    unsigned int processQuery_NeedToRecompute(const QueryResult&) const;
    
    void processQuery_LogRecomputeReasons(unsigned int reason);
    
    // Filter result list for fuzzy search
    void fuzzySearchFilterResultList(const QueryResult& oldResultList,
                                const vector<int>& words,
                                const string& query,
                                QueryResult* newResultList);

    // filter a fuzzy list. fuzzySearchFilterResultList calls this
    // function
    void fuzzySearchFilterList(
                             const QueryResult& oldResultList,
                             const std::unordered_set<int>& hashSet,
                             QueryResult* newResultList);


    // compute query top-k suggestion for a fuzzy search query,
    // assuming the query has been already processed and
    // all intermediate lists are cached
    void findFuzzySearchQuerySuggestions(
        Query& query,
        int k,
        bool suggestOnlyPhrases,
        vector<string>* suggestions,
        vector<double>* scores);

    // see above. Special case when two-word query takes place
    // (due to speed).
    void findFuzzySearchQuerySuggestions2(
        QueryResult& list1,
        QueryResult& list2,
        int k,
        vector<string>* suggestions,
        vector<double>* scores);

    // find frequent co-occurring query terms using filtering
    // from cached query
    void filterQuerySuggestions(
        Query& query,
        int k,
        vector<string>* suggestions,
        vector<double>* scores);

   // given a set of word ids of fuzzy search, compute the equivalent
   // (hash)set of word ids of the completer
   void computeWordIds(const vector<int>& closestWordIds,
                       const string& query,
                       std::unordered_set<int>* hashSet);

  public:

    enum RecomputeReason
    {
      RECOMPUTE_HITS        = 0x01,
      RERANK_HITS           = 0x02,
      RESORT_HITS           = 0x04,
      RECOMPUTE_COMPLETIONS = 0x08,
      RERANK_COMPLETIONS    = 0x10,
      RESORT_COMPLETIONS    = 0x20
    };
  
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
     */
    /*
     void processComplexQuery
     (Query&           query,
     QueryParameters& queryParameters,
     QueryResult*&    result);
     */

    //! Process complex query from left to right; new interface but OLD IMPLEMENTATION (Ingmar's)
    void processComplexQuery(const Query& query, QueryResult*& result);
    unsigned processComplexQuery_NEW(const Query&, QueryResult*&);
  
    //! Get top continuation of a query; e.g. utf8 for !encoding:*
    string getTopContinuationOfQuery(string queryString);

    //! Get all continuations of a query; e.g. for !hitinfo-multiple:*
    vector<string> getAllContinuationsOfQuery(string queryString);

    //! Process basic query for given posting lists and prefix
    /*
     *   Wrapper method that deals with all kinds of special cases: join queries, or
     *   queries, empty D, empty W, etc. In the typical case (CASE 3.3 in the source
     *   code), just translates prefix to word range and then calls
     *   processBasicQuery from HybCompleter or InvCompleter
     */
    void processBasicQuery(const QueryResult& inputList, //!< input list of postings
        const Query& firstPartOfQuery, //!< part of query before that separator
        const Query& lastPartOfQuery, //!< part of query after that separator, i.e., last prefix
        const Separator& separator, //!< separator before last prefix
        QueryResult& result //!< output list of postings
        );

    //! Process contex-sensitive prefix-search query for given set of doc ids and range of word ids;
    //! ABSTRACT METHOD to be implemented by each particular completer, e.g. HYB and INV.
    virtual void processBasicQuery(const QueryResult& inputList, //!< input list of postings
        const WordRange& wordRange, //!< the range W of word ids
        QueryResult& result, //!< output list of postings
        const Separator& separator //!< separator before last prefix
        //const QueryResult* listsForPrefixFromHistory =  NULL   //!< list of postings for word range, if available from history
        ) { CS_THROW(Exception::NOT_YET_IMPLEMENTED, "abstract"); }

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
    void processOrQuery(const QueryResult& resultFirstPart,
        const Query& firstPartOfQuery, const Query& lastPartOfQuery,
        const Separator& separator, QueryResult& result);

    //! Process join query, with last part of the form [q1#q2#...#qm]
    void processJoinQuery(const QueryResult& resultFirstPart,
        const Query& firstPartOfQuery, const Query& lastPartOfQuery,
        const Separator& separator, QueryResult& result);

    // Process fuzzy search query, with last part of the form xyz~ or xyz*~.
    void processFuzzySearchQuery(const QueryResult& resultFirstPart,
        const Query& firstPartOfQuery, const Query& lastPartOfQuery,
        const Separator& separator, QueryResult& result);

    // Process synonym search query, with last part of the form xyz+ or xyz*+.
    void processSynonymSearchQuery(const QueryResult& resultFirstPart,
        const Query& firstPartOfQuery, const Query& lastPartOfQuery,
        const Separator& separator, QueryResult& result);

    //! Compute top-k hits and completions for given result; finalize result; TODO: explain what finalize means
    /*!
     *   \param result  holds the posting lists (already computed), and the top-k
     *                  lists (to be computed by this method)
     *
     *   \param docScoreAggSameCompletion        how to aggregate doc scores of multiple occurrences of the same word in a document
     *   \param docScoreAggDifferentCompletions  how to aggregate doc scores of occurrences of different words/completions in a document
     *
     *   \param wordScoreAggSameDocument         how to aggregate word scores of multiple occurences of the same word in the same document
     *   \param wordScoreAggDifferentDocuments   how to aggregate word scores from different documents
     *
     *   Note: in QueryParameters, there is also docScoreAggDifferentQueryParts,
     *   which is for "how to aggregate doc scores from the different query
     *   parts". This has already been done at this point, during the computation of result
     *   (the ScoreAggregation parameter to intersect).
     *
     *   Note: the counter part for docScoreAggDifferentQueryParts does not exist
     *   for word scores, because completions are always for the last part of the
     *   query only.
     *
     *   Note: here we recall the necessary terminology to understand the param names
     *
     *   1. each query consists of several parts, e.g. the query "scheduling venue:*" has two parts
     *   2. each part can have several completions, e.g., venue:* has completions venue:esa, venue:icalp, etc.
     *   3. each completion can occur multiple times in a document
     */
    void computeTopHitsAndCompletions(QueryResult& result);

    //! Compute top-k hits and completions; partially templated version
    template<class T>
    void computeTopHitsAndCompletions(QueryResult& result,
        const T& docScoreAggSameCompletion);

    //! Compute top-k hits and completions; partially templated version
    template<class T, class U>
    void computeTopHitsAndCompletions(QueryResult& result,
        const T& docScoreAggSameCompletion,
        const U& docScoreAggDifferentCompletions);

    //! Compute top-k hits and completions; partially templated version
    template<class T, class U, class V>
    void computeTopHitsAndCompletions(QueryResult& result,
        const T& docScoreAggSameCompletion,
        const U& docScoreAggDifferentCompletions,
        const V& wordScoreAggSameDocument);

    //! Compute top-k hits and completions; fully templated version
    template<class T, class U, class V, class W>
    void computeTopHitsAndCompletions(QueryResult& result,
        const T& docScoreAggSameCompletion,
        const U& docScoreAggDifferentCompletions,
        const V& wordScoreAggSameDocument,
        const W& wordScoreAggDifferentDocuments);

    //! Compute top-k hits
    /*!
     *    NOTE: the current implementation ignores the first aggregation method, and
     *    takes the second for both cases. (In computeTopHitsAndCompletions above
     *    this is checked.) That way, this method has a very simple and efficient
     *    realization. Two different methods would require that the postings for
     *    each doc id would be sorted by word id.
     *
     *    TODO: also support two different methods.
     */
    template<class T, class U>
    void computeTopHits(QueryResult& result,
        const T& docScoreAggSameCompletion,
        const U& docScoreAggDifferentCompletions);

    //! Compute top-k completions
    template<class V, class W>
    void computeTopCompletions(QueryResult& result,
        const V& wordScoreAggSameDocument,
        const W& wordScoreAggDifferentDocuments);

    // Helper function used in computeTopCompletions. Maps the word ids according
    // to the (optional) word id map.
    void remapWordIds(const WordList& wordIdsIn, WordList* wordIdsOut);
    FRIEND_TEST(CompleterBaseTest, remapWordIds);

    // Helper function used in computeTopCompletions. Takes a raw posting list
    // as input (doc ids, word ids, scores), and aggregates these by word id.
    // The three input lists have the same length, and so do the five output
    // lists.
    template<class V, class W>
    void sortAndAggregateByWordId(const DocList& docIds,
        const WordList& wordIds, const ScoreList& scores,
        DocList* docIdsAggregated, WordList* wordIdsAggregated,
        ScoreList* scoresAggregated,
        Vector<unsigned int>* docCountsAggregated,
        Vector<unsigned int>* occCountsAggregated,
        const V& wordScoreAggSameDocument,
        const W& wordScoreAggDifferentDocuments);FRIEND_TEST(CompleterBaseTest, sortAndAggregateByWordId);

    // Helper function to compute the normalized edit distance between the two
    // given words. Optionally compute the generalized edit distance (Warning:
    // VERY SLOW).
    // TODO(bast): this function should be provided by FuzzySearcher in this form.
    // TODO(bast): there should be a (global) type for the encoding, and not just
    // a bool which says whether it's utf-8 or not.
    double computeNormalizedEditDistance(const string& word1,
        const string& word2, bool encodingIsUtf8,
        bool computeGeneralizedEditDistance);

    //! Merge two posting lists (currently only used for "or")
    void mergeTwoPostingLists(const QueryResult& input1,
        const QueryResult& input2, QueryResult& result);

    //! Filter a list of postings wrt to a given word range
    /*
     *   TODO: why are the word ids of the filtered list not output?
     */
    template<typename vW, typename vD, typename vP, typename vS>
    void copyAndFilter(vW& filteredWords, //!< TODO: what's this?
        const vD& unfilteredDoclist, //!< input posting list, doc ids
        const vW& unfilteredWordlist, //!< input posting list, word ids
        const vP& unfilteredPositionlist, //!< input posting list, positions
        const vS& unfilteredScorelist, //!< input posting list, scores
        const WordId& wordrangeLow, //!< word range, lowest word id
        const WordId& wordrangeHigh, //!< word range, highest word id
        vD& filteredDocs, //!< output posting list, doc ids
        vP& filteredPositionlist, //!< output posting list, positions
        vS& filteredScorelist); //!< output posting list, scores


    //  2 QUERY PROCESSING
    //  2.1 INTERSECT
    //  *********************************

    //! Intersect two posting lists; NEW IMPLEMENTATION BY HOLGER
    /*!
     *   Note: much simpler, more efficient, and more powerful than the original
     *   intersect. Well documented and with a proper signature.
     *
     *   Built-in capability to check it's own results. If ??? additional
     *   information is computed while computing the result, with the help of which
     *   the result can then be post-checked in linear time. (Note that given just
     *   the two input lists and the result, checking the correctness is essentially
     *   a matter of redoing the intersection which is not what we want.)
     *
     *    \param inputList1         the first input list
     *    \param inputList2         the secon input list
     *    \param resultList         the result list
     *    \param separator          the separator that defines what counts as an intersection
     *    \param wordIdRange        from input2 only consider word ids from this range
     *    \param scoreAggregation   aggregate scores according to this object
     *
     *   TODO: result checking only works for some of the modes so far.
     *
     *   TODO: make proximity bonus part of score aggregator. In particular, each
     *   score aggregator object should be able to tell whether positions need to be
     *   looked at or not (or should this decision be made solely on the
     *   CheckPosition object).
     *
     *   TODO: There should be a score aggregation which does NOTHING at all, i.e.,
     *   the scores of the output list are undefined, or zero, or the score list is
     *   empty. For example, Gabriel's query rotation doesn't need the scores, and
     *   then it will be more efficient if one doesn't do any aggregation. Should
     *   this be the default score aggregation?
     */
    void intersectTwoPostingLists(const QueryResult& input1,
        const QueryResult& input2, QueryResult& result,
        const Separator& separator = sameDocSeparator,
        const ScoreAggregation scoreAggregation = SCORE_AGG_SUM,
        const WordRange& wordIdRange = infiniteWordIdRange);

    //! Intersect two posting lists; partially templated version
    /*!
     *    Used in intermediate calls on the way from the outside call of
     *    intersectTwoPostingLists to the fully templated (hence more efficient but
     *    complicated to call directly) intersectTwoPostingListsTemplated.
     */
    template<class ScoreAggregator>
    void intersectTwoPostingListsPartiallyTemplated(
        const QueryResult& input1, const QueryResult& input2,
        QueryResult& result, const Separator& separator,
        const WordRange& wordIdRange, const ScoreAggregator& aggregateScores);

    //! Intersect two posting lists; partially templated version
    /*!
     *    Used in intermediate calls on the way from the outside call of
     *    intersectTwoPostingLists to the fully templated (hence more efficient but
     *    complicated to call directly) intersectTwoPostingListsTemplated.
     */
    template<class ScoreAggregator, class CheckWordId>
    void
    intersectTwoPostingListsPartiallyTemplated(const QueryResult& input1,
        const QueryResult& input2, QueryResult& result,
        const Separator& separator, const CheckWordId& checkWordId,
        const ScoreAggregator& aggregateScores);

    //! Intersect two posting lists; partially templated version
    /*!
     *    Used in intermediate calls on the way from the outside call of
     *    intersectTwoPostingLists to the fully templated (hence more efficient but
     *    complicated to call directly) intersectTwoPostingListsTemplated.
     */
    template<class ScoreAggregator, class CheckWordId, class CheckPosition>
    void intersectTwoPostingListsPartiallyTemplated(
        const QueryResult& input1, const QueryResult& input2,
        QueryResult& result, const CheckWordId& checkWordId,
        const CheckPosition& checkPosition,
        const ScoreAggregator& aggregateScores, const unsigned int outputMode);

    //! Intersect two posting lists, fully templated version
    /*!
     *    This method contains the actual code doing the intersection.
     *
     *    TODO: Make notes on that the templating is really important for efficiency
     */
    template<class CheckWordId, class CheckPosition, class ScoreAggregator,
        unsigned int outputMode>
    void intersectTwoPostingListsTemplated(const QueryResult& input1,
        const QueryResult& input2, QueryResult& result,
        const CheckWordId& checkWordId, const CheckPosition& checkPosition,
        const ScoreAggregator& aggregateScores);

    //! Intersect two posting lists NEW; fully templated version   NNN
    /*!
    *    This method is like intersectTwoPostingListsTemplated with some changes
    *    in Algorithm like replacing of pushback with []
    */
    template<class CheckWordId, class CheckPosition, class ScoreAggregator,
        unsigned int outputMode>
    void intersectTwoPostingListsTemplated2(const QueryResult& input1,
        const QueryResult& input2, QueryResult& result,
        const CheckWordId& checkWordId, const CheckPosition& checkPosition,
        const ScoreAggregator& aggregateScores);

    //! Check the result of an intersection (incomplete and deprecated)
    void checkCorrectnessOfIntersectionResult(const QueryResult& input1,
        const QueryResult& input2, const QueryResult& result);

    //! Check equality of two posting lists (used for testing only)
    void checkEqualityOfTwoPostingLists(const QueryResult& input1,
        const QueryResult& input2);

    //! Intersect two posting lists OLD; partially templated version
    /*
     *    Used in intermediate calls on the way from the outside call of
     *    intersectTwoPostingListsOld to the fully templated (hence more efficient but
     *    complicated to call directly) intersectTwoPostingListsOldTemplated.
     *
     *    NOTE: old, fucked-up signature. Changed for the NEW intersect, but left
     *    it as is for the old one (which is just there for checking anyway).
     */
    template<class scoreType, class T, class U, class V>
    void intersect(const Separator& splitSeparator, const T& checkWordIds,
        const V& notIntersectionMode, const U& locationScoreAggregationMethod,
        const scoreType scoretype, const QueryResult& candidateLists,
        const DocList& doclist, const Vector<Position>& positionlist,
        const Vector<scoreType>& scorelist, const WordList& wordlist,
        QueryResult& resultLists, const WordRange& wordRange);

    //! Intersect two posting lists OLD; fully templated version
    /*!
     *    This contains the actual old code. Fucked-up signature, see above.
     */
    template<class S, class T, class U, class V, class scoreType>
    void intersect(const S& intersectionMode, const T& checkWordIds,
        const V& notIntersectionMode, const U& locationScoreAggregationMethod,
        const scoreType scoretype, const QueryResult& candidateLists,
        const DocList& doclist, const Vector<Position>& positionlist,
        const Vector<scoreType>& scorelist, const WordList& wordlist,
        QueryResult& resultLists, const WordRange& wordRange);

    //! Intersect two posting lists, nicer interface; for use in experiments etc.
    /*!
     *    Note: the two methods above have a hard-to-understand signature. This
     *    method has the signature one would expect:
     *
     *    \param inputList1  the first input list
     *    \param inputList2  the secon input list
     *    \param resultList  the result list
     *    \param separator   the separator that defines what counts as an intersection
     *
     *    NOTE: Holger has written this for Gabriel, before writing the new
     *    intersect and adapting the signature of the old intersect as described
     *    above. The new intersectTwoPostingLists has exactly the signature
     *    anticipated here.
     */
    /*
     void intersect(const QueryResult& inputList1,
     const QueryResult& inputList2,
     QueryResult& resultList,
     const Separator    separator);
     */

    // Translate given prefix to range of word ids. A * at the end indicates a
    // proper prefix/a range, without a star the range is a singleton or empty.
    // Also deals with range queries like year:1997--year:2005. This method also
    // tells the caller whether the prefix started with a - and hence was a NOT
    // query.
    //
    // TODO(bast): I would consider it more elegant to incorporate the
    // notIntersectionMode into the WordRange. That is, add a bool _isInverse or
    // something like that to the WordRange class and a public getter.
    //
    // NOTE(bast): This is the place to do any form of query normalization, because
    // here is the point where we have broken down the query to an individual query
    // word.
    WordRange prefixToRange(string prefix, bool& notIntersectionMode) const;

    //! Translate word id to word (via the lexicographically sorted vocabulary)
    const string& getWordFromVocabulary(WordId wordId) const
    {
      assert( wordId < (WordId) _vocabulary->size());
      return _vocabulary->operator[](wordId);
    }

    //  2.2 JOIN
    //  *********************************

    //! Join two posting lists, Refactored and taken from QueryResult.
    //! TODO: Probably also refactor the
    //! actual implementation since the changes due to the
    //! introduction of the word with SPECIAL_WORD_ID feel quite invasive.
    /*!
     *    \param inputList1         the first input list
     *    \param inputList2         the second input list
     *    \param resultList         the result list that will be filled
     *    \param joinMethod         whether to use hash or merge join
     */
    void joinTwoPostingLists(const QueryResult& input1,
        const QueryResult& input2, QueryResult& result,
        const int joinMethod = QueryParameters::HASH_JOIN);

  private:

    //! Worker method for the merge join. Since this join needs to sort
    //! the input, calls are by value as they used to be for the
    //! whole old join method.
    /*!
     *    \param inputList1         the first input list
     *    \param inputList2         the second input list
     *    \param result             the result list that will be filled
     */
    void doMergeJoin(QueryResult input1, QueryResult input2,
        QueryResult& result);

    //! Worker method for the hash join. No sorting of the input required
    //! and hence using const references.
    /*!
     *    \param inputList1         the first input list
     *    \param inputList2         the second input list
     *    \param result             the result list that will be filled
     */
    void doHashJoin(const QueryResult& input1, const QueryResult& input2,
        QueryResult& result);

    //! Worker method for the hash join. No sorting of the input required
    //! and hence using const references.
    /*!
     *    \param inputList1         the first input list
     *    \param inputList2         the second input list
     *    \param result             the result list that will be filled,
     *                              excluding postings with special wordId.
     *    \param specialPostings    a second result produced, containing the
     *                              union of postings with special wordId
     *                              from both input lists.
     */
    void restoreSpecialPostings(const QueryResult& input,
        const QueryResult& specialPostings, QueryResult& result);

    // TODO: Has this been used just by the semantic query stuff? If yes, delete
    // it. QueryResultComparator, reWriteQueryWord, entityToLower
    class QueryResultComparator
    {
      public:
        bool operator()(QueryResult* const & x, QueryResult* const & y) const
        {
          return x->getSize() < y->getSize();
        }
    };

    // Removes the prefix from entities and replaces classes by their type path.
    string reWriteQueryWord(const string& word);
  public:
    static string entityToLower(const string& entity);

    //  3. BUFFERS
    //  **********

  protected:

    //! INITIAL BUFFER SIZES
    unsigned long int initialIntersectionBufferSize; // in number of elements
    unsigned long int initialUnionBufferSize; // dito
    unsigned long int initialCompressionBufferSize; // in bytes (of the compressed lists)

    //! INTERSECTION BUFFERS
    //   elements of intersection are appended one by one. These buffers will
    //   contain duplicates and are "in parallel". They will always have the
    //   same size. No repeated or fulllist thingies are allowed on these buffers
    DocList* _doclistIntersectionBuffer;
    WordList* _wordlistIntersectionBuffer;
    Vector<Position>* _positionlistIntersectionBuffer;
    Vector<Score>* _scorelistIntersectionBuffer;

    //! RESULT BUFFERS
    //   These buffers will hold the united/sorted/merged elements i.e., all
    //   duplicates will be removed
    DocList* _doclistUnionBuffer;
    WordList* _wordlistUnionBuffer;
    Vector<Score>* _topDocScoresBuffer; //scores for matching docs
    Vector<Score>* _lastBlockScoresBuffer; //scores for matching docs without the current block
    Vector<Score>* _topWordScoresBuffer; //scores for completions
    Vector<unsigned int>* _topWordDocCountsBuffer; //scores for frequencies of completions (# docs)
    Vector<unsigned int>* _topWordOccCountsBuffer; //scores for frequencies of completions (# docs)
    char *_compressionBuffer;

    // CHECKS IF ALL WORDS IN THE BUFFER ARE IN THE SPECIFIED RANGE (as they must be)
    bool checkWordlistBuffer(const WordRange& wordRange) const;

    //   currently (23Jun06) ONLY clears the _topWordScoresBuffer and the
    //   _lastBlockScoresBuffer does NOT have to clear compressionBuffer as
    //   this is overwritten from the beginning anyways

  public:

    //! TODO: what's this?
    unsigned long _compressionBufferSize;

    //! TODO: explain
    void freeCompressionBuffer();

    //! TODO: explain
    unsigned long compressionBufferSize() const
    {
      return _compressionBufferSize;
    }

    //! TODO: explain
    void resizeCompressionBuffer(unsigned long size);

    //! INITIALIZATION

    //   TODO: warum braucht man drei verschiedene Funktionen? Sehr
    //   verwirrendes Interface!

    //   sets the initial buffer size and initializes the compression and
    //   completions buffers also resets counters calls
    //   reserveCompressionBuffersAndResetPointers()
    void setInitialBufferSizes(
        unsigned long int initIntBuffSize,
        unsigned long int initUniBuffSize,
        unsigned long int initCompressBuffSize =
            COMPRESSION_BUFFER_INIT_DEFAULT);

    void reserveCompressionBuffersAndResetPointersAndCounters();

    // sets Buffers in QueryResult to empty vectors (with enough space)
    // also sets the global pointers (which should/could be removed altogether)
    void reserveBuffersAndResetPointers(QueryResult& resultToWriteTo);

    // called by reserveCompressionBuffersAndResetPointersAndCounters(...)
    // uses sizes set by setInitialBufferSizes
    void reserveCompressionBuffersAndResetPointers();

    //  4. TIMER, COUNTER, STATISTICS
    //  *****************************

  public:

    //! SHOW ONE-LINE SUMMARY (moved from CompletionServer.cpp)
    virtual void showOneLineSummary(ConcurrentLog& os, // was: ostream&
        Query& query, QueryResult* result, off_t total_usecs);

    //! PRINT STATISTICS (baumgari's version, Nov. 2010)
    virtual void showStatistics(ConcurrentLog& os, off_t totalTime, string indent = "");

    //  TIMERS (also moved "global Timers" here on 05Jun07)
    mutable Timer threadTimer; // NEW 13Sep13 (baumgari): was part of CompletionServer
    mutable Timer receiveQueryTimer; // NEW 25Dec07 (Holger): was part of CompletionServer
    mutable Timer sendResultTimer; // NEW 25Dec07 (Holger): was part of CompletionServer
    mutable Timer buildResultStringTimer;
    mutable Timer closeConnectionTimer;
    mutable Timer processQueryTimer;
    mutable Timer intersectionTimer;
    mutable Timer prefixToRangeTimer;
    mutable Timer historyCleanUpTimer;
    mutable Timer completionsForWordRangeTimer;
    // the following three include both decompression and copying of decompressed
    mutable Timer doclistDecompressionTimer;
    mutable Timer positionlistDecompressionTimer;
    mutable Timer wordlistDecompressionTimer;
    mutable Timer fileReadTimer;
    mutable Timer externalTimer; // Timer for total time in HYB/INV
    mutable Timer getExcerptsTimer; // NEW 18Sep06 (Holger): should be part of query summary
    mutable Timer intersectWordlistsTimer; // 18Jul06 so far this times the whole method, no breakdown yet
    mutable Timer intersectWordlistsTimer1;
    mutable Timer intersectWordlistsTimer2;
    mutable Timer intersectWordlistsTimer3;
    mutable Timer intersectWordlistsTimer4;
    mutable Timer intersectWordlistsTimer5;
    mutable Timer mergeResultsTimer; // 31Oct06 so far this times the whole method, no breakdown yet
    mutable Timer resizeAndReserveTimer;
    mutable Timer stlSortTimer; // only measures sorting outside of scoring
    mutable Timer invMergeTimer;
    mutable Timer scoreDocsTimer; //includes time for removing duplicates and also for sorting
    mutable Timer scoreDocsTimer1;
    mutable Timer scoreDocsTimer2;
    mutable Timer scoreDocsTimer3;
    mutable Timer scoreDocsTimer4;
    mutable Timer scoreWordsTimer; //includes time for the bucket sort (which also removes duplicates)
    mutable Timer appendTimer;
    mutable Timer broadHistoryTimer; //includes everything to do with filtering, TODO: always >= historyTimer ?
    mutable Timer broadHistoryTimer0;
    mutable Timer broadHistoryTimer1;
    mutable Timer broadHistoryTimer2;
    mutable Timer broadHistoryTimer3;
    mutable Timer broadHistoryTimer4;
    mutable Timer broadHistoryTimer5;
    mutable Timer broadHistoryTimer6;
    mutable Timer broadHistoryTimer7;
    mutable Timer timerTimer;
    mutable Timer setCompletionsTimer;
    mutable Timer setCompletionsTimer1;
    mutable Timer setCompletionsTimer2;
    mutable Timer mapWordIdsTimer;

    // timer for the k-way merge
    mutable Timer kwayMergeTimer;

    // Timers for fuzzysearch
    // includes the time for all performed distance computations
    // + clusters selections
    mutable Timer fuzzySearchDistanceComputationsTimer;
    // the time to process the fuzzy search query (not counting computation of
    // the clusters etc.)
    mutable Timer fuzzySearchProcessQueryTimer;
    // the total time for fuzzy search on the last key-word
    mutable Timer fuzzySearchTotalTimer;
    // auxilary timer
    mutable Timer fuzzySearchAuxTimer;
    // list merge timer
    mutable Timer fuzzySearchMergeTimer;
    // timer used for merging short list in fuzzy search
    mutable Timer fuzzySearchPreMergeTimer;
    // time to read (and decompress) lists from disk
    mutable Timer fuzzySearchFetchListsTimer;
    // time to compute the query suggestions
    mutable Timer fuzzySearchComputeQuerySuggestionsTimer;

    // NEW 17Oct13 (baumgari): Replaced several semsearchTopK*Timers by one
    // groupDocsByWordIdTopKTimer.
    // time needed in TopK if GROUP_DOCS_BY_WORD_ID is set
    mutable Timer groupDocsByWordIdTopKTimer;

    //! COUNTERS
    mutable unsigned long nofBlocksReadFromFile; // = doclists for INV
    mutable off_t doclistVolumeDecompressed; // in bytes, total volume decompressed
    mutable off_t positionlistVolumeDecompressed; // in bytes, total volume decompressed
    mutable off_t wordlistVolumeDecompressed; // in bytes, total volume decompressed
    mutable off_t scorelistVolumeRead; // in bytes, total volume decompressed
    mutable unsigned long nofUnions;
    mutable off_t doclistUniteVolume, wordlistUniteVolume,
        scorelistUniteVolume, positionlistUniteVolume; // in bytes
    mutable unsigned int nofQueriesFromHistory, nofQueriesByFiltering;
    mutable off_t intersectNofPostings;
    mutable off_t intersectedVolume; // in bytes, only docslist are counted here
    mutable unsigned long nofIntersections;
    mutable unsigned long nofQueries;
    mutable off_t volumeReadFromFile; // in bytes, does not include metaData or vocabulary or index

    // counters for fuzzysearch
    mutable size_t fuzzySearchCoverIndex; // number of used clusters
    mutable size_t fuzzySearchNumRelevantWordsSelected;
    mutable size_t fuzzySearchNumRelevantWords;
    mutable size_t fuzzySearchNumDistinctWordsInSelectedClusters;
    mutable size_t fuzzySearchNumSimilarWords;

    //! RESET TIMERS AND COUNTERS
    virtual void resetTimersAndCounters()
    {
      resetTimers();
      resetCounters();
    }

    //! RETURN THE TIME, WHICH ELASPED WHILE PROCESSING THE QUERY
    off_t getTotalProcessingTimeInUsecs() const;

    /*
     //! REMEMBER WHETHER LAST QUERY ANSWERED FROM HISTORY (used in showStatistics)
     bool resultWasInHistory;
     //! REGISTER WHETHER LAST QUERY WAS FILTERED AND FROM WHICH QUERY (used in showStatistics)
     string resultWasFilteredFrom;
     */

    //  5. VOCABULARY HANDLING
    //  **********************

    //! WRITE VOCABULARY TO FILE (one word per line)
    void writeVocabularyToFile(const char* fileName) const;

    //! ADD WORD TO DICTIONARY (must be done in lexicographical order!)
    void addWordToVocabulary(const string& word)
    {
      assert( (_vocabulary->size() == 0) || (word > _vocabulary->getLastWord()) );
      _vocabulary->push_back(word);
    }

    //! GET VOCABULARY SIZE
    unsigned long getVocabularySize()
    {
      return _vocabulary->size();
    }

    //! RESET TIMERS AND COUNTERS (virtual was missing!)
    virtual void resetCounters();
    virtual void resetTimers();

    //! COPIES ALL WORD-IN-DOC PAIRS, ONLY WORDS AND ONLY DOCS
    //  (AND LATER SCORES) TO THE QUERYRESULT
    //
    //   TODO: 09March06: also need to pass the "old" QueryResult, as in some ....
    //   Better: have another buffer to which we write the document scores of the last block
    //   We do not want to find the matching docs twice
    //   have to output them somewhere in the intersection routine
    //
    //   TODO: I don't really understand what this does!?
    //
    void writeCompletionsAsStringsToQueryResult(QueryResult &result);

    //  6. HISTORY HANDLING
    //  *******************

    //    NOTE: these methods are public (rather than protected), because
    //    answerQueries accesses them from the outside
    //
    //    TODO: all of these just call the method with the same name in the
    //    History class, why did Ingmar do that? It is certainly not the proper
    //    way to do this.
    //
    //    TODO: a couple of them do slightly more, say which ones and why
    //

  public:

    // NEW(hagn, 28Jan11):
    string getFlagForHistory();
    // NEW(hagn, 28Jan11):
    void finalizeSizeOfHistory(const Query& query);
    void addToHistory(const Query& key, const QueryResult& result);
    void removeFromHistory(const Query& query);
    // CHANGE(hagn, 28Jan11):
    QueryResult* isInHistory(const Query& query);
    const QueryResult* isInHistoryConst(const Query& query);
    bool isInHistory(const Query& key, QueryResult*& result);
    // CHANGE(hagn, 28Jan11):
    //unsigned char getStatusOfHistoryEntry(const Query& key) const;
    unsigned char getStatusOfHistoryEntry(const Query& key);
    void setStatusOfHistoryEntry(const Query& key, unsigned char status);
    size_t getSizeOfHistoryInBytes() const
    {
      return history->sizeInBytes();
    }
    unsigned int getNofQueriesInHistory() const
    {
      return history->getNofQueries();
    }
};

#endif
