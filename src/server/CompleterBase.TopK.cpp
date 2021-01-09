#include "CompleterBase.h"
#include <google/dense_hash_map>
#include <unordered_map>
#include <unordered_set>
#include "../fuzzysearch/StringDistances.h"

extern bool rankByGeneralizedEditDistance;
extern bool normalizeWords;
extern FuzzySearch::GeneralizedEditDistance* generalizedDistanceCalculator;

//! Compute top-k hits and completions for given result
/*!
 *   WARNING: docScoreAggSameCompletion is currently ignored and set to
 *   docScoreAggDifferentCompletions. If different values were passed, a warning
 *   is issued (but no exception thrown). See computeTopHits below for the
 *   reason behind this.
 *
 *   \param result  holds the posting lists (already computed), and the top-k
 *                  lists (to be computed by this method)
 *
 *   \param docScoreAggSameCompletion        how to aggregate doc scores of multiple occurrences of the same word in a document
 *   \param docScoreAggDifferentCompletions  how to aggregate doc scores of occurrences of different words/completions in a document
 *
 *   \param wordScoreAggSameDocument         how to aggregate word scores of multiple occurrences of the same word in the same document
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
template<unsigned char MODE>
void CompleterBase<MODE>::computeTopHitsAndCompletions(QueryResult& result)
{
  log << AT_BEGINNING_OF_METHOD << "; score aggregation is "
      << _queryParameters.getScoreAggregationChars() << endl;

  // Let the result remember with which parameters it was computed
  result._queryParameters = _queryParameters;

  // First two aggregations must be the same currently; see computeTopHits
  if (_queryParameters.docScoreAggSameCompletion
      != _queryParameters.docScoreAggDifferentCompletions)
  {
    log
        << "! WARNING: different values for docScoreAggSameCompletion and docScoreAggDifferentCompletions"
        << " (" << (int) (_queryParameters.docScoreAggSameCompletion)
        << " and " << (int) (_queryParameters.docScoreAggDifferentCompletions)
        << ")" << "; will only use docScoreAggDifferentCompletions" << endl;
  }

  if (_queryParameters.docScoreAggSameCompletion == SCORE_AGG_SUM)
  {
    SumAggregation scoreAggregationSum;
    computeTopHitsAndCompletions(result, scoreAggregationSum);
  }
  else if (_queryParameters.docScoreAggSameCompletion == SCORE_AGG_MAX)
  {
    MaxAggregation scoreAggregationMax;
    computeTopHitsAndCompletions(result, scoreAggregationMax);
  }
  else if (_queryParameters.docScoreAggSameCompletion
      == SCORE_AGG_SUM_WITH_BONUS)
  {
    SumProxAggregation scoreAggregationSumWithBonus;
    computeTopHitsAndCompletions(result, scoreAggregationSumWithBonus);
  }
  else
  {
    CS_THROW(Exception::INVALID_PARAMETER_VALUE,
        "docScoreAggSameCompletion = " << _queryParameters.docScoreAggSameCompletion);
  }

  log << AT_END_OF_METHOD << endl;
}

//! Compute top-k hits and completions; partially templated version
template<unsigned char MODE>
template<class T>
void CompleterBase<MODE>::computeTopHitsAndCompletions(QueryResult& result,
    const T& docScoreAggSameCompletion)
{
  if (_queryParameters.docScoreAggDifferentCompletions == SCORE_AGG_SUM)
  {
    SumAggregation scoreAggregationSum;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        scoreAggregationSum);
  }
  else if (_queryParameters.docScoreAggDifferentCompletions == SCORE_AGG_MAX)
  {
    MaxAggregation scoreAggregationMax;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        scoreAggregationMax);
  }
  else if (_queryParameters.docScoreAggDifferentCompletions
      == SCORE_AGG_SUM_WITH_BONUS)
  {
    SumProxAggregation scoreAggregationSumWithBonus;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        scoreAggregationSumWithBonus);
  }
  else
  {
    CS_THROW(Exception::INVALID_PARAMETER_VALUE,
        "docScoreAggDifferentCompletions = " << _queryParameters.docScoreAggDifferentCompletions);
  }
}

//! Compute top-k hits and completions; partially templated version
template<unsigned char MODE>
template<class T, class U>
void CompleterBase<MODE>::computeTopHitsAndCompletions(QueryResult& result,
    const T& docScoreAggSameCompletion,
    const U& docScoreAggDifferentCompletions)
{
  if (_queryParameters.wordScoreAggSameDocument == SCORE_AGG_SUM)
  {
    SumAggregation scoreAggregationSum;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        docScoreAggDifferentCompletions, scoreAggregationSum);
  }
  else if (_queryParameters.wordScoreAggSameDocument == SCORE_AGG_MAX)
  {
    MaxAggregation scoreAggregationMax;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        docScoreAggDifferentCompletions, scoreAggregationMax);
  }
  else if (_queryParameters.wordScoreAggSameDocument
      == SCORE_AGG_SUM_WITH_BONUS)
  {
    SumProxAggregation scoreAggregationSumWithBonus;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        docScoreAggDifferentCompletions, scoreAggregationSumWithBonus);
  }
  else
  {
    CS_THROW(Exception::INVALID_PARAMETER_VALUE,
        "wordScoreAggSameDocument = " << _queryParameters.wordScoreAggSameDocument);
  }
}

//! Compute top-k hits and completions; partially templated version
template<unsigned char MODE>
template<class T, class U, class V>
void CompleterBase<MODE>::computeTopHitsAndCompletions(QueryResult& result,
    const T& docScoreAggSameCompletion,
    const U& docScoreAggDifferentCompletions,
    const V& wordScoreAggSameDocument)
{
  if (_queryParameters.wordScoreAggDifferentDocuments == SCORE_AGG_SUM)
  {
    SumAggregation scoreAggregationSum;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        docScoreAggDifferentCompletions, wordScoreAggSameDocument,
        scoreAggregationSum);
  }
  else if (_queryParameters.wordScoreAggDifferentDocuments == SCORE_AGG_MAX)
  {
    MaxAggregation scoreAggregationMax;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        docScoreAggDifferentCompletions, wordScoreAggSameDocument,
        scoreAggregationMax);
  }
  else if (_queryParameters.wordScoreAggDifferentDocuments
      == SCORE_AGG_SUM_WITH_BONUS)
  {
    SumProxAggregation scoreAggregationSumWithBonus;
    computeTopHitsAndCompletions(result, docScoreAggSameCompletion,
        docScoreAggDifferentCompletions, wordScoreAggSameDocument,
        scoreAggregationSumWithBonus);
  }
  else
  {
    CS_THROW(Exception::INVALID_PARAMETER_VALUE,
        "wordScoreAggDifferentDocuments = " << _queryParameters.wordScoreAggDifferentDocuments);
  }
}

//! Compute top-k hits and completions; fully templated version
template<unsigned char MODE>
template<class T, class U, class V, class W>
void CompleterBase<MODE>::computeTopHitsAndCompletions(QueryResult& result,
    const T& docScoreAggSameCompletion,
    const U& docScoreAggDifferentCompletions,
    const V& wordScoreAggSameDocument, const W& wordScoreAggDifferentDocuments)
{
  // NEW(bast, 7Dec10): Now calling computeTopCompletions before computeTopHits,
  // so that we can reuse the precomputed edit distances from
  // computeTopCompletions (in case rw=5) in computeTopHits (in case rd=5). I
  // don't think the order mattered otherwise, but was not 100% sure.
  // log << "* NEW: now calling computeTopCompletions before computeTopHits"
  //     << " --- hopefully not a problem, is it?" << endl;

  // NEW(bast, 7Dec10): If we have a non-trivial word id map (for mapping the id
  // of C:1234:algorithm to the id of algorithm, and things like that), map the
  // word ids.
  // NOTE(bast): My original idea was to do this mapping *after*
  // sortAndAggregateByWordId in computeTopCompletions, when the list is much
  // shorter (and then aggregate again, but a much shorter list). However, the
  // problem with that approach is that we then cannot compute the exact doc
  // counts anymore, since after sortAndAggregateByWordId we do not know from
  // which set of doc ids a doc count for a word id came, and we then want to
  // aggregate once more the doc counts for two different word ids (mapped into
  // one), there is no way to determine the correct doc count.
  // NOTE2(bast): Moved this from computeTopCompletions to here, which now
  // permanently changes the word ids. Is this a problem?
  if (_vocabulary->isWordMapTrivial() == false)
    remapWordIds(result._wordIdsOriginal, &result._wordIdsMapped);
  else result._wordIdsMapped = result._wordIdsOriginal;

  computeTopCompletions(result, wordScoreAggSameDocument,
      wordScoreAggDifferentDocuments);

  computeTopHits(result, docScoreAggSameCompletion,
      docScoreAggDifferentCompletions);

  CS_ASSERT_EQ(result._docIds.size(), result._wordIdsMapped.size());
  CS_ASSERT_EQ(result._docIds.size(), result._wordIdsOriginal.size());
  CS_ASSERT_EQ(result._docIds.size(), result._scores.size());
  CS_ASSERT_EQ(result._docIds.size(), result._positions.size());
  CS_ASSERT_EQ(result._topDocIds.size(), result._topDocScores.size());
  CS_ASSERT_EQ(result._topWordIds.size(), result._topWordScores.size());
  CS_ASSERT_EQ(result._topWordIds.size(), result._topWordDocCounts.size());
  CS_ASSERT_EQ(result._topWordIds.size(), result._topWordOccCounts.size());
}

//! Compute top-k hits
/*!
 *    Input: the posting lists from \c result (must all be of the same length;
 *    positions currently not used).
 *
 *    Output: writes top-k list to \c result (_topDocIds and _topDocScores; must
 *    be of same size)
 *
 *    WARNING: the current implementation ignores the first aggregation method,
 *    and takes the second for both cases. (In computeTopHitsAndCompletions
 *    above this is checked.) That way, this method has a very simple and
 *    efficient realization. Two different methods would require that the
 *    postings for each doc id would be sorted by word id. At the moment, I
 *    (Holger) don't see any real use cases for having two different methods, so
 *    I haven't implemented it.
 *
 *    TODO: support for different methods for docScoreAggSameCompletion and
 *    docScoreAggDifferentCompletions
 *
 *    TODO: split QueryResult into two classes, one for the posting lists, and
 *    one for the top-k lists. This also makes sense for the cache, where the
 *    top-k lists need to be recomputed if some of the parameter changed. (Also
 *    note that at the moment we can cache only the most recent top-k list.)
 */
template<unsigned char MODE>
template<class T, class U>
void CompleterBase<MODE>::computeTopHits(QueryResult& result,
    const T& docScoreAggSameCompletion,
    const U& docScoreAggDifferentCompletions)
{
  log << AT_BEGINNING_OF_METHOD << "; result has " << result.getNofPostings()
      << " postings" << "; how to rank docs = "
      << (int) (_queryParameters.howToRankDocs) << "; sort order = "
      << (int) (_queryParameters.sortOrderDocs) << "; k = "
      << _queryParameters.nofTopCompletionsToCompute << endl;
  scoreDocsTimer.cont();

  // NEW(bast 28Sep11): When aggreating with special posting, always use sum. HHH.
  SumAggregation aggregateWithSpecialPosting;

  // Result posting list and top-k lists.
  const DocList& docIds = result._docIds;
  const WordList& wordIds = result._wordIdsMapped;
  // const PositionList& positions = result._positions;
  // CHANGE(hagn, 16Jun11): Note that the scores are copied because
  // we will temporarily overwrite them
  // NEW(bast, 28Sep11): overwrite the scores again.
  ScoreList& scores = result._scores;
  // ScoreList scores = result._scores;
  DocList& topDocIds = result._topDocIds;
  ScoreList& topDocScores = result._topDocScores;
  WordList topDocWordIds;

  // NEW(bast, 7Dec10): If RANK_DOCS_BY_COMPLETION_SCORES, overwrite the scores
  // by the already computed completion scores (computeTopCompletions is now
  // called before computeTopHits). Do not overwrite the score of the special
  // posting with word id NO_WORD_ID, which represents the aggregated score from
  // the first part of the query.
  // TODO: If RANK_DOCS_BY_COMPLETION_SCORES and RANK_WORDS_BY_DOC_COUNT,
  // RANK_WORDS_BY_OCC_COUNT or RANK_WORDS_BY_WORD_ID then we must copy
  // the value to the score
  if (_queryParameters.howToRankDocs == QueryParameters::RANK_DOCS_BY_COMPLETION_SCORES)
  {
    log << "! NEW: rank docs by completion scores ---"
        << " this can be slow for large result lists, watch out!"
        << " Number of precomputed edit distances is "
        << result._completionScoresByWordId.size() << endl;
    //log << "! WARNING: scores are overwritten, this leads to"
    //    << " unexpected results wenn switching rd mode between queries"
    //    << endl;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      WordId wordId = wordIds[i];
      if (wordId != SPECIAL_WORD_ID) scores[i]
          = result._completionScoresByWordId.count(wordId) > 0 ? result._completionScoresByWordId[wordId]
              : 1;
    }
  }

  // NEW(hagn, 10Jun11): If RANK_DOCS_BY_FUZZY_SCORE, overwrite the scores
  if (_queryParameters.howToRankDocs == QueryParameters::RANK_DOCS_BY_FUZZY_SCORE)
  {
    log << "! NEW: rank docs by fuzzy score ---"
        << " this can be slow for large result lists, watch out!"
        << " Number of precomputed edit distances is "
        << result._completionFuzzyScoresByWordId.size() << endl;
    //log << "! WARNING: scores are overwritten, this leads to"
    //    << " unexpected results when switching rd mode between queries"
    //    << endl;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      WordId wordId = wordIds[i];
      if (wordId != SPECIAL_WORD_ID) scores[i]
          = result._completionFuzzyScoresByWordId.count(wordId) > 0 ? scores[i] * ((double)result._completionFuzzyScoresByWordId[wordId]/100)
              : 1;
    }
  }

  //
  // 1. compute list of unique doc ids, together with the aggregated scores
  //
  topDocIds .resize(0);
  topDocScores .resize(0);
  topDocWordIds.resize(0);
  DocId currentDocId;
  Score currentScore;
  WordId currentWordId;
  unsigned int i = 0;
  CS_ASSERT_EQ(docIds.size(), scores.size());

  // NEW 29Mar11 (Björn): We need a deterministic outcome. Hence
  // it would be good to be able to sort by the old, true document
  // scores as secondary sorting value. However, we may not mix
  // up different groups (corresponding wordIds. In conclusion, we
  // try to add that to the word scores.
  typedef std::unordered_map<WordId, Score> WordScoreMap;
  WordScoreMap topWordScores;
  ScoreList topDocWordScores;
  std::unordered_map<WordId, unsigned int> completionVisited;
  if (_queryParameters.howToRankDocs == QueryParameters::GROUP_DOCS_BY_WORD_ID)
  {
    size_t nofWords = result._topWordIds.size();
    groupDocsByWordIdTopKTimer.cont();
    for (size_t j = 0; j < result._topWordIds.size(); ++j)
    {
      // NEW 14Nov11 (Hannah): dort word ids by scores, not by occ count.
      // TODO(bast): this should be made dependent on the value for 
      // SemanticQueryParameters::setDefaultValues, which can be
      // RANK_WORDS_BY_OCC_COUNT or RANK_WORDS_BY_SCORE (at the time of this
      // writing it is set to the latter, hence the change below).
      topWordScores[result._topWordIds[j]] = result._topWordScores[j]
          + (nofWords--);
      CS_ASSERT(topWordScores[result._topWordIds[j]]
          >= result._topWordScores[j]);
      // topWordScores[result._topWordIds[j]] = result._topWordOccCounts[j]
      //     + (nofWords--);
      // CS_ASSERT(topWordScores[result._topWordIds[j]]
      //     >= result._topWordOccCounts[j]);
    }
    groupDocsByWordIdTopKTimer.stop();
    // Do not aggregate scores at all when grouping by word ID!
    // Aggregation comprises hits for each documents leaving
    // only some matching word ID to represent the hit.
    // Since we still need to know WHICH words can be associated
    // with the hit, we may not do this here.
    // TODO: Think about handling scores appropriately,
    // even though we do not rank by the scores, they are used
    // as secondary attribute and decide which hits are assigned
    // to a group.
    // Hence, the aggregation is moved to the else branch and
    // here we simply ignore the special posting and remove
    // duplicates in a sense where word and doc are both equal.

    // NEW 24 May: Drop everything that is not from topWords.
    std::unordered_set<WordId> wordsDone;
    groupDocsByWordIdTopKTimer.cont();
   
    while (i < docIds.size())
    {
      currentDocId = docIds[i];
      wordsDone.clear();
      while (i < docIds.size() && docIds[i] == currentDocId)
      {
        currentWordId = wordIds[i];

        if (SPECIAL_WORD_ID != currentWordId
            && topWordScores.find(currentWordId) != topWordScores.end()
            && wordsDone.count(currentWordId) == 0)
        {
          currentScore = scores[i];
          wordsDone.insert(currentWordId);
          topDocIds .push_back(currentDocId);
          topDocScores .push_back(currentScore);
          topDocWordIds.push_back(currentWordId);
        }
        ++i;
      }
    }
    groupDocsByWordIdTopKTimer.stop();
  }
  else
  {
    while (i < docIds.size())
    {
      currentDocId = docIds[i];
      currentWordId = wordIds[i];
      currentScore = scores[i];
      CS_ASSERT(SPECIAL_WORD_ID != currentWordId);
      // currentScore  = rankDocsByCompletionScores
      //   ? (result._completionScoresByWordId.count(currentWordId) > 0
      //       ? result._completionScoresByWordId[currentWordId] : 1)
      //   : scores[i];
      ++i;
      while (i < docIds.size() && docIds[i] == currentDocId)
      {
        // NOTE(bast): here we aggregate the postings for the same doc (both
        // ordinary and the possible special posting at the end). HHH
        currentScore = wordIds[i] != SPECIAL_WORD_ID
	  // NOTE(bast): this aggregates two ordinary postings. HHH
          ? docScoreAggDifferentCompletions.aggregate(currentScore, scores[i])
	  // NOTE(bast): this aggregates with a special posting. HHH
          : aggregateWithSpecialPosting.aggregate(currentScore, scores[i]);
          // : docScoreAggDifferentCompletions.aggregate(currentScore, scores[i]);
        // Note: commented this out, because we must not take the last posting
        // from a doc id anymore, because it could be the special posting (with
        // word id SPECIAL_WORD_ID, which is meaningless in this context).
        // currentWordId = wordIds[i];
        ++i;
      }
      topDocIds .push_back(currentDocId);
      topDocScores .push_back(currentScore);
      topDocWordIds.push_back(currentWordId);
    }
  }
  // log << endl;
  CS_ASSERT_EQ(topDocIds.size(), topDocScores.size());
  CS_ASSERT_EQ(topDocIds.size(), topDocWordIds.size());
  result.nofTotalHits = topDocIds.size();

  // NEW(bjoern, 14Mar11): If GROUP_DOCS_BY_WORD_ID,
  // overwrite the scores in the following way.
  // Sort documents according to their score.
  // Overwrite the score with the already computed completion scores.
  // But do this only for k documents per completion. The rest gets 0.
  //
  // NEW 29Mar11 (Björn):
  // Do not overwrite the scores any longer, but append to the
  // topDocWordScores vector instead.
  if (_queryParameters.howToRankDocs == QueryParameters::GROUP_DOCS_BY_WORD_ID)
  {
    groupDocsByWordIdTopKTimer.cont();
    topDocScores.partialSortParallel(topDocWordIds, topDocIds, 0,
        SORT_ORDER_DESCENDING);
    groupDocsByWordIdTopKTimer.stop();

    typedef std::unordered_map<WordId, unsigned int> CountMap;
    CountMap completionVisited;
    size_t listSize = topDocScores.size();
    groupDocsByWordIdTopKTimer.cont();
    for (size_t j = 0; j < listSize; ++j)
    {
      WordId wordId = topDocWordIds[j];
      WordScoreMap::const_iterator entry = topWordScores.find(wordId);

      CountMap::const_iterator countEntry = completionVisited.find(wordId);

      unsigned int count = 0;
      if (countEntry == completionVisited.end())
      {
        completionVisited[wordId] = 0;
      }
      else
      {
        count = countEntry->second;
      }

      if (entry != topWordScores.end() && count
          < _queryParameters.nofHitsPerGroup)
      {
        ++(completionVisited[wordId]);
        topDocWordScores.push_back(topWordScores[wordId]);
      }
      else
      {
        topDocWordScores.push_back(1);
      }
    }
    groupDocsByWordIdTopKTimer.stop();
    CS_ASSERT(topDocWordScores.size() == topDocIds.size());
  }

  //
  // 2. Partial sort the matching doc ids to obtain the top-k documents (k = 0 means all)
  //
  //   Note: partialSortParallel resizes array to size k after partial sort
  //
  unsigned int k = _queryParameters.nofTopHitsToCompute;
  if (_queryParameters.howToRankDocs == QueryParameters::GROUP_DOCS_BY_WORD_ID
      && result._topWordIds.size() > 1 && result._topDocWordIds.size()
      * _queryParameters.nofHitsPerGroup < k)
  {
    k = result._topWordIds.size() * _queryParameters.nofHitsPerGroup;
 //   result.nofTotalHits = k;
  }
  if (k == 0 || k > topDocIds.size()) k = topDocIds.size();
  SortOrderEnum sortOrder = _queryParameters.sortOrderDocs;
  switch (_queryParameters.howToRankDocs)
  {
  case QueryParameters::RANK_DOCS_BY_SCORE:
  case QueryParameters::RANK_DOCS_BY_FUZZY_SCORE:
  case QueryParameters::RANK_DOCS_BY_COMPLETION_SCORES:
    topDocScores.partialSortParallel(topDocIds, k, sortOrder);
    break;

  case QueryParameters::GROUP_DOCS_BY_WORD_ID:
    // NEW 29Mar11 (Björn): We need a deterministic outcome. Hence
    // it would be good to be able to sort by the old, true document
    // scores as secondary sorting value. However, we may not mix
    // up different groups (corresponding wordIds. In conclusion, we
    // try to add that to the word scores.
    groupDocsByWordIdTopKTimer.cont();
    topDocWordScores.partialSortParallel(topDocScores, topDocIds,
        topDocWordIds, k, sortOrder);
    result._topDocWordIds.reserve(topDocWordIds.size());
    // Truncate the list after those that are sorted
    result._topDocIds.resize(k);
    result._topDocScores.resize(k);
    topDocWordIds.resize(k);
    result._topDocWordIds.clear();
    result._topDocWordIds.reserve(k);
    for (size_t j = 0; j < topDocWordIds.size(); ++j)
    {
      result._topDocWordIds.push_back(topDocWordIds[j]);
    }
    groupDocsByWordIdTopKTimer.stop();
    break;

  case QueryParameters::RANK_DOCS_BY_WORD_ID:
    topDocWordIds.partialSortParallel(topDocScores, topDocIds, k, sortOrder);
    break;

  case QueryParameters::RANK_DOCS_BY_DOC_ID:
    if (sortOrder == SORT_ORDER_ASCENDING)
    {
      // already sorted by ascending doc ids, so no need to sort in this case
      topDocIds .resize(k);
      topDocScores.resize(k);
    }
    else
    {
      topDocIds.partialSortParallel(topDocScores, k, sortOrder);
    }
    break;

  default:
    CS_THROW(Exception::INVALID_PARAMETER_VALUE, "howToRankDocs = " << _queryParameters.howToRankDocs)
    ;
    break;
  }
  CS_ASSERT_EQ(topDocIds.size(), topDocScores.size());

  scoreDocsTimer.stop();
  log << AT_END_OF_METHOD << "; computed top " << topDocIds.size()
      << " documents" << endl;
}

//! Compute top-k completions
/*!
 *    Input: the posting lists from \c result (must all be of the same length;
 *    positions currently not used).
 *
 *    Output: writes list of top-k completions to \c result (_topWordIds,
 *    _topWordScores, _topWordDocCounts, _topWordOccCounts; must all
 *    be of same size)
 *
 *    Implementation note: Uses a sort when the result list is relatively small
 *    to the range of word ids in the result, and an array of the size of the
 *    word range otherwise (storing the intermediate score for each word id, as
 *    well as the last doc id encountered).
 *
 *    TODO: so far does all the aggregation with wordScoreAggDifferentDocuments.
 *    Once this is tested, enhancing this to consider wordScoreAggSameDocument
 *    for postings from the same document will not be hard.
 */
template<unsigned char MODE>
template<class V, class W>
void CompleterBase<MODE>::computeTopCompletions(QueryResult& result,
    const V& wordScoreAggSameDocument, const W& wordScoreAggDifferentDocuments)
{
  log << AT_BEGINNING_OF_METHOD << "; result has " << result.getNofPostings()
      << " postings" << "; how to rank words = "
      << (int) (_queryParameters.howToRankWords) << "; sort order = "
      << (int) (_queryParameters.sortOrderWords) << "; k = "
      << _queryParameters.nofTopCompletionsToCompute << endl;

  // Clear the hash map of precomputed completion scores.
  result._completionScoresByWordId.clear();
  result._completionFuzzyScoresByWordId.clear();

  // If result is empty, do nothing; TODO: check whether this is the desired behaviour
  if (result.getNofPostings() == 0) return;

  // Result posting lists. Note that the word ids are copied because we
  // optionally re-map their ids.
  const DocList& docIds = result._docIds;
  WordList wordIds = result._wordIdsMapped;
  const ScoreList& scores = result._scores;

  // result top-k list
  DocList topWordDocIds;
  WordList& topWordIds = result._topWordIds;
  ScoreList& topWordScores = result._topWordScores;
  Vector<unsigned int>& topWordDocCounts = result._topWordDocCounts;
  Vector<unsigned int>& topWordOccCounts = result._topWordOccCounts;

  scoreWordsTimer.cont();

  // 1. Sort the raw posting list by word id, and aggregate the postings for
  // each word id.
  sortAndAggregateByWordId(docIds, wordIds, scores, &topWordDocIds,
      &topWordIds, &topWordScores, &topWordDocCounts, &topWordOccCounts,
      wordScoreAggSameDocument, wordScoreAggDifferentDocuments);
  result.nofTotalCompletions = topWordDocIds.size();

  //
  // 2. Partial sort the matching word ids to obtain the top-k completions (k = 0 means all)
  //
  //   Note: partialSortParallel resizes array to size k after partial sort
  //
  unsigned int k = _queryParameters.nofTopCompletionsToCompute;
  if (k == 0 || k > topWordIds.size()) k = topWordIds.size();
  SortOrderEnum sortOrder = _queryParameters.sortOrderWords;
  if (_queryParameters.howToRankWords == QueryParameters::RANK_WORDS_BY_EDIT_DISTANCE ||
      _queryParameters.howToRankDocs  == QueryParameters::RANK_DOCS_BY_FUZZY_SCORE)
  {
    log << IF_VERBOSITY_HIGH << "* NEW: howToRankWords = "
        << "RANK_WORDS_BY_EDIT_DISTANCE" << endl;
    // TODO(bast): This does one edit-distance calculation per distinct
    // completion. A single edit distance calculation takes about 1 usec, so
    // this is OK until about 10,000 distinct completions. When there are more
    // completions something should be done, for example consider just the
    // 10,000 most frequent completions. That is a heuristic, yes, but a very
    // reasonable one I think.

    {
      // Get last part of original query, and remove trailing ~ and trailing
      // *. Also remove everything until the last colon. For example getrai*~
      // -> getrai, :filter:autor:horst~ -> horst.
      string queryPrefix = result._prefixCompleted;
      if (queryPrefix == "") queryPrefix = _query.getQueryString();
      // NEW 11Oct13 (baumgari): GCC complains, that lastMatchingPosition is an
      // unused variable. Uncommented it.
      // bool isFuzzyQuery = false;
      if (queryPrefix.size() > 0 && queryPrefix[queryPrefix.size() - 1] == '~')
      {
        // isFuzzyQuery = true;
        queryPrefix.resize(queryPrefix.size() - 1);
      }
      bool isPrefixQuery = false;
      if (queryPrefix.size() > 0 && queryPrefix[queryPrefix.size() - 1] == '*')
      {
        isPrefixQuery = true;
        queryPrefix.resize(queryPrefix.size() - 1);
      }
      size_t pos = queryPrefix.rfind(wordPartSep);
      // TODO(celikik): what in case of two word queries? the full
      // query will be compared to the completion! This below is a
      // quick fix, one should do it properly.
      if (pos == string::npos) pos = queryPrefix.rfind(' ');
      if (pos == string::npos) pos = queryPrefix.rfind('.');
      if (pos == string::npos)
      {
        pos = queryPrefix.rfind("..");
        if (pos != string::npos) pos++;
      }
      if (pos != string::npos) queryPrefix = queryPrefix.substr(pos + 1);
	      log << IF_VERBOSITY_HIGH
          << "* FUZZY: ranking completions by their edit distance to \""
          << queryPrefix << "\"";
      if (isPrefixQuery)
        log << " (is prefix query => true completions are preferred)";
      log << endl;
      bool usingUtf8 = localeString.find("utf8") != string::npos;
      Timer editDistanceComputationTimer;
      editDistanceComputationTimer.start();
      string word;
      log << IF_VERBOSITY_HIGH << "* FUZZY: first completions are:";
      // Compute edit distance to each distinct completion. For each
      // completion, remove everything up to the last colon, for example
      // C:23432:hort -> hort, :filter:autor:C:8332:horst -> horst, and
      // hort -> hort.
      for (size_t i = 0; i < topWordIds.size(); ++i)
      {
        WordId wordId = topWordIds[i];
        word = (*_vocabulary)[wordId];
        if (i < 5) log << IF_VERBOSITY_HIGH << " " << word;
        size_t pos = word.rfind(wordPartSep);
        if (pos != string::npos) word = word.substr(pos + 1);
        Score score;
	// NEW (baumgari) 01Mai14: The uncommented part is used to make sure,
	// that prefixes are ranked higher than non prefixes. E.g. if the query
	// is müller, müllerschön should be ranked higher than maller.
	// This is not always the best way. With the introduction of
	// normalization it's not sure anymore, that the user is really looking
	// for the completions of the given query or for the completions of the
	// normalized query. I expect that the variations "müller", "muller"
	// and "mueller" are more interesting than "müllerschön" and therefore
	// this part is uncommented.

	// For an exact match, give score of 100, for exact completion
        // give score of 99.
/*        if (word.size() >= queryPrefix.size() && word.compare(0,
            queryPrefix.size(), queryPrefix) == 0)
        {
          // NEW(hagn, 14Jun11): Edit-Distance-Score
          // to be used in computeTopHits when howToRankDocs == RANK_DOCS_BY_FUZZY_SCORE
          result._completionFuzzyScoresByWordId[wordId] = word.size() == queryPrefix.size() ? 100 : 99;
          // CHANGE(hagn, 23Mai11): Now use the real average score
          //score = word.size() == queryPrefix.size() ? 100 : 99;
          score = word.size() == queryPrefix.size()
	    ? (((double) topWordScores[i] / topWordOccCounts[i]) * 100)
	    : (((double) topWordScores[i] / topWordOccCounts[i]) * 99);
        }
        // Otherwise, compute normalized edit distance to query prefix.
        else*/
        {
          double normalizedEditDistance = computeNormalizedEditDistance(
              queryPrefix,
              word,
              usingUtf8,
              rankByGeneralizedEditDistance
                  && !_fuzzySearcher->completionMatching());
          CS_ASSERT_LE(0.0, normalizedEditDistance);
          CS_ASSERT_GE(1.0, normalizedEditDistance);
          score = static_cast<Score> (100 * (1 - normalizedEditDistance));
          // NEW(hagn, 14Jun11): Edit-Distance-Score
          // to be used in computeTopHits when howToRankDocs == RANK_DOCS_BY_FUZZY_SCORE
          result._completionFuzzyScoresByWordId[wordId] = score * _queryParameters.fuzzyDamping;
          // NEW(hagn, 27Jul11): 0 not allowed
          result._completionFuzzyScoresByWordId[wordId] = (result._completionFuzzyScoresByWordId[wordId] == 0 ? 1 : result._completionFuzzyScoresByWordId[wordId]);
          // CHANGE(hagn, 23Mai11): Now use the real average score
          // and depreciate the Fuzzy score to max 50%
          // log << IF_VERBOSITY_HIGH
          //     << "DEBUG (score components): old score = " << topWordScores[i]
          //     << ", occ count = " << topWordOccCounts[i]
          //     << ", NED = " << (int)(100 * normalizedEditDistance) << "%"
          //     << ", damping = " << (int)(100 *_queryParameters.fuzzyDamping) << "%" << endl;
          // score = static_cast<Score> ((double)topWordScores[i]/topWordOccCounts[i] 
          //                             * (1 - normalizedEditDistance)) 
          //                             * _queryParameters.fuzzyDamping;
          // NEW(hagn, 27Jul11): 0 not allowed
          score = (score == 0 ? 1 : score);
          // log << endl
          //     << "! DEBUG computeTopCompletions : "
          //     << "i = " << setw(5) << i
          //     << ", W = " << setw(5) << wordId
          //     << ", S = " << setw(5) << score
          //     << ", NED = " << setw(5) << setprecision(2) << normalizedEditDistance
          //     << ", Prefix = " << setw(20) << queryPrefix
          //     << ", Word = " << word << endl;
        }
        // log << IF_VERBOSITY_HIGH
        //     << "DEBUG: overwriting topWordIds[" << i << "] = " << topWordScores[i] << " with " << score << endl;
        if (_queryParameters.howToRankWords == QueryParameters::RANK_WORDS_BY_EDIT_DISTANCE) topWordScores[i] = score;
        // Store these scores in a hash map, for use in computeTopHits in case
        // howToRankDocs == RANK_DOCS_BY_COMPLETION_SCORES.
        // CHANGE(hagn, 4Feb11):
        //result._completionScoresByWordId[wordId] = score;
      }
      log << IF_VERBOSITY_HIGH << (topWordIds.size() > 5 ? " ..." : "")
          << endl;
      editDistanceComputationTimer.stop();
      log << IF_VERBOSITY_HIGH
          << (editDistanceComputationTimer.msecs() > 100 ? EMPH_ON RED : "")
          << "* FUZZY: ranked " << numberAndNoun(topWordIds.size(),
          "completion", "completions") << " in "
          << editDistanceComputationTimer;
      if (topWordIds.size() > 0) log << IF_VERBOSITY_HIGH << " ("
          << setprecision(1) << (double) editDistanceComputationTimer.usecs()
          / topWordIds.size() << " usecs / completion)";
      log << IF_VERBOSITY_HIGH << EMPH_OFF BLACK << endl;
    }
  }
  switch (_queryParameters.howToRankWords)
  {
  case QueryParameters::RANK_WORDS_BY_SCORE:
    topWordScores.partialSortParallel(topWordDocCounts, topWordOccCounts,
        topWordIds, k, sortOrder);
    break;

  case QueryParameters::RANK_WORDS_BY_DOC_COUNT:
    topWordDocCounts.partialSortParallel(topWordScores, topWordOccCounts,
        topWordIds, k, sortOrder);
    break;

  case QueryParameters::RANK_WORDS_BY_OCC_COUNT:
    topWordOccCounts.partialSortParallel(topWordScores, topWordDocCounts,
        topWordIds, k, sortOrder);
    break;

  case QueryParameters::RANK_WORDS_BY_DOC_ID:
    topWordScores = topWordDocIds;
    topWordScores.partialSortParallel(topWordDocCounts, topWordOccCounts,
        topWordIds, k, sortOrder);
    break;

  case QueryParameters::RANK_WORDS_BY_WORD_ID:
    if (sortOrder == SORT_ORDER_ASCENDING)
    {
      // already sorted by ascending word ids, so no need to sort in this case
      topWordIds .resize(k);
      topWordScores .resize(k);
      topWordDocCounts.resize(k);
      topWordOccCounts.resize(k);
    }
    else
    {
      topWordIds.partialSortParallel(topWordDocCounts, topWordOccCounts,
          topWordScores, k, sortOrder);
    }
    break;

  case QueryParameters::RANK_WORDS_BY_EDIT_DISTANCE:
    topWordScores.partialSortParallel(topWordDocCounts, topWordOccCounts,
        topWordIds, k, sortOrder);
    break;

  default:
    CS_THROW(Exception::INVALID_PARAMETER_VALUE, "howToRankWords = " << _queryParameters.howToRankWords)
    ;
    break;
  }

  // NEW(hagn, 4Feb11):
  for (size_t i = 0; i < topWordIds.size(); ++i)
  {
    WordId wordId = topWordIds[i];
    result._completionScoresByWordId[wordId] = topWordScores[i];
  }
  log << IF_VERBOSITY_HIGH
      << "! NEW: stored " << result._completionScoresByWordId.size()
      << " scores in a hash map --- hopefully not too many, are they?"
      << endl;

  scoreWordsTimer.stop();
  CS_ASSERT_EQ(topWordIds.size(), topWordScores.size());
  CS_ASSERT_EQ(topWordIds.size(), topWordDocCounts.size());
  CS_ASSERT_EQ(topWordIds.size(), topWordOccCounts.size());
  log << AT_END_OF_METHOD << "; computed top " << topWordIds.size()
      << " completions" << endl;
}

template<unsigned char MODE>
template<class V, class W>
void CompleterBase<MODE>::sortAndAggregateByWordId(const DocList& docIds,
    const WordList& wordIds, const ScoreList& scores,
    DocList* docIdsAggregated, WordList* wordIdsAggregated,
    ScoreList* scoresAggregated, Vector<unsigned int>* docCountsAggregated,
    Vector<unsigned int>* occCountsAggregated,
    const V& wordScoreAggSameDocument, const W& wordScoreAggDifferentDocuments)
{
  assert(docIds.size() == wordIds.size());
  assert(docIds.size() == scores.size());
  assert(docIdsAggregated != NULL);
  assert(wordIdsAggregated != NULL);
  assert(scoresAggregated != NULL);
  assert(docCountsAggregated != NULL);
  assert(occCountsAggregated != NULL);
  assert(docIdsAggregated->size() == 0);
  assert(wordIdsAggregated->size() == 0);
  assert(scoresAggregated->size() == 0);
  assert(docCountsAggregated->size() == 0);
  assert(occCountsAggregated->size() == 0);
  size_t inputSize = docIds.size();

  // Get min and max word in in result
  WordId minWordId = MAX_WORD_ID;
  WordId maxWordId = MIN_WORD_ID;
  WordId currentWordId;
  for (unsigned int i = 0; i < wordIds.size(); ++i)
  {
    currentWordId = wordIds[i];
    if (currentWordId == SPECIAL_WORD_ID) continue;
    if (currentWordId < minWordId) minWordId = currentWordId;
    if (currentWordId > maxWordId) maxWordId = currentWordId;
  }
  CS_ASSERT_GT(wordIds.size(), 0);
  CS_ASSERT_GE(maxWordId, minWordId);
  unsigned int wordIdRangeSize = maxWordId - minWordId + 1;

  // CASE 1: for relatively small input size, use SORTING.
  if (inputSize < wordIdRangeSize / 10)
  {
    log << IF_VERBOSITY_HIGH << "! use sorting to rank completions"
        << "; input size is " << inputSize << "; word range size is "
        << wordIdRangeSize << endl;

    // Sort postings by word id (primary key) and doc id (secondary key).
    // Discard postings with word id SPECIAL_WORD_ID.

    {
      wordIdsAggregated->resize(wordIds.size());
      scoresAggregated->resize(scores.size());
      docIdsAggregated->resize(docIds.size());
      size_t i = 0;
      size_t j = 0;
      while (i < wordIds.size())
      {
        if (wordIds[i] != SPECIAL_WORD_ID)
        {
          (*wordIdsAggregated)[j] = wordIds[i];
          (*scoresAggregated)[j] = scores[i];
          (*docIdsAggregated)[j] = docIds[i];
          ++j;
        }
        ++i;
      }
      wordIdsAggregated->resize(j);
      scoresAggregated->resize(j);
      docIdsAggregated->resize(j);
      CS_ASSERT_EQ(wordIdsAggregated->size(), docIdsAggregated->size());
      CS_ASSERT_EQ(wordIdsAggregated->size(), scoresAggregated->size());
      wordIdsAggregated->sortParallel(*docIdsAggregated, *scoresAggregated);
      docCountsAggregated->clear();
      occCountsAggregated->clear();
      docIdsAggregated->clear();
    }

    // Aggregate scores.
    //
    //   TODO: using only wordScoreAggDifferentDocuments. See
    //   computeHitsAndCompletions above.
    //
    unsigned int i = 0;
    unsigned int j = 0;
    WordId currentWordId;
    Score currentScore1;
    Score currentScore2;
    DocId currentDocId;
    unsigned int currentDocCount;
    unsigned int currentOccCount;
    while (i < wordIdsAggregated->size())
    {
      currentWordId = (*wordIdsAggregated)[i];
      currentScore1 = 0;
      currentDocCount = 0;
      currentOccCount = 0;
      while (i < wordIdsAggregated->size() && (*wordIdsAggregated)[i]
          == currentWordId)
      {
        currentScore2 = (*scoresAggregated)[i];
        currentDocId = (*docIdsAggregated)[i];
        ++currentDocCount;
        ++currentOccCount;
        ++i;
        while (i < wordIdsAggregated->size() && (*wordIdsAggregated)[i]
            == currentWordId && (*docIdsAggregated)[i] == currentDocId)
        {
          currentScore2 = wordScoreAggSameDocument. aggregate(currentScore2,
              (*scoresAggregated)[i]);
          ++currentOccCount;
          ++i;
        }
        currentScore1 = wordScoreAggDifferentDocuments. aggregate(
            currentScore1, currentScore2);
      }
      CS_ASSERT_LT(j, i);
      (*wordIdsAggregated)[j] = currentWordId;
      (*scoresAggregated)[j] = currentScore1;
      (*docIdsAggregated)[j] = currentDocId;
      docCountsAggregated->push_back(currentDocCount);
      occCountsAggregated->push_back(currentOccCount);
      ++j;
    }
    wordIdsAggregated->resize(j);
    scoresAggregated->resize(j);
    docIdsAggregated->resize(j);
  }

  // CASE 2: for relatively large result, use array of size of word range

  else
  {
    log << IF_VERBOSITY_HIGH
        << "! use array of word range size to rank completions"
        << "; input size is " << inputSize << "; word range size is "
        << wordIdRangeSize << endl;

    // aggregate the scores
    vector<Score> currentScores1(wordIdRangeSize, 0);
    vector<Score> currentScores2(wordIdRangeSize, 0);
    vector<unsigned int> currentDocCounts(wordIdRangeSize, 0);
    vector<unsigned int> currentOccCounts(wordIdRangeSize, 0);
    vector<DocId> lastDocId(wordIdRangeSize, INFTY_DOCID);
    WordId currentWordId;
    DocId currentDocId;
    Score currentScore;
    unsigned int j;
    for (size_t i = 0; i < wordIds.size(); ++i)
    {
      currentWordId = wordIds[i];
      if (currentWordId == SPECIAL_WORD_ID) continue;
      currentDocId = docIds[i];
      currentScore = scores[i];
      j = currentWordId - minWordId;
      CS_ASSERT_LT(j, wordIdRangeSize);
      ++currentOccCounts[j];
      if (currentDocId != lastDocId[j])
      {
        currentScores1[j] = wordScoreAggDifferentDocuments.aggregate(
            currentScores1[j], currentScores2[j]);
        currentScores2[j] = currentScore;
        ++currentDocCounts[j];
      }
      else
      {
        currentScores2[j] = wordScoreAggSameDocument.aggregate(
            currentScores2[j], currentScore);
      }
      lastDocId[j] = currentDocId;
    }

    // copy the aggregated scores from the big array to the result
    wordIdsAggregated->clear();
    scoresAggregated->clear();
    docCountsAggregated->clear();
    occCountsAggregated->clear();
    docIdsAggregated->clear();
    for (unsigned int j = 0; j < wordIdRangeSize; ++j)
    {
      if (lastDocId[j] != INFTY_DOCID)
      {
        wordIdsAggregated->push_back(j + minWordId);
        scoresAggregated->push_back(
            wordScoreAggDifferentDocuments. aggregate(currentScores1[j],
                currentScores2[j]));
        docCountsAggregated->push_back(currentDocCounts[j]);
        occCountsAggregated->push_back(currentOccCounts[j]);
        docIdsAggregated->push_back(lastDocId[j]);
      }
    }
  }

  CS_ASSERT_EQ(wordIdsAggregated->size(), scoresAggregated->size());
  CS_ASSERT_EQ(wordIdsAggregated->size(), docCountsAggregated->size());
  CS_ASSERT_EQ(wordIdsAggregated->size(), occCountsAggregated->size());
  CS_ASSERT_EQ(wordIdsAggregated->size(), docIdsAggregated->size());
}

// _____________________________________________________________________________
template<unsigned char MODE>
void CompleterBase<MODE>::remapWordIds(const WordList& wordIdsIn,
                                             WordList* wordIdsOut)
{
  if (wordIdsIn.size() == 0) return;
  if (!wordIdsOut->empty()) wordIdsOut->clear();
  CS_ASSERT_EQ(wordIdsOut->size(), 0);
  wordIdsOut->reserve(wordIdsIn.size());
  mapWordIdsTimer.start();
  for (unsigned int i = 0; i < wordIdsIn.size(); ++i)
  {
    wordIdsOut->push_back(_vocabulary->mappedWordId(wordIdsIn[i]));
  }
  mapWordIdsTimer.stop();
  CS_ASSERT_EQ(wordIdsIn.size(), wordIdsOut->size());
}

// _____________________________________________________________________________
// Compute the normalized edit distance between the two given words. Optionally
// compute the generalized edit distance (Warning: SLOW).
// TODO(bast): this function should be provided by FuzzySearcher in this form.
// TODO(bast): there should be a (global) type for the encoding, and not just a
// bool which says whether it's utf-8 or not.
template<unsigned char MODE>
double CompleterBase<MODE>::computeNormalizedEditDistance(const string& word1,
    const string& word2, bool encodingIsUtf8,
    bool computeGeneralizedEditDistance)
{
  double normalizedEditDistance = -1;

  // bartsch 19.07.2011
  // We compare the index word with the matching search word.
  // If we use word normalization, we have to normalize the matching search
  // word as well because the index word is normalized.
  // Expl:
  // word1 = Grünlippmuschelpulver  ( the matching seach word)
  // word2 = gruenlippmuschelpulver ( the index word)
  // Instead of using word1 for the calculation of the edit distance,
  // we use the normalized form word1_cpy.
  //
  // This solves the setlocale/mbstowcs problem for the moment because
  // in the call of
  // generalizedDistanceCalculator->calculateUtf8(word1_cpy, word2, 1.0);
  // both words contain only ASCII character (which mbstowcs can handle in
  // in any locale.
  // (The 'setlocale/mbstowcs problem' means that setlocale fails for
  //  statically linked code which may lead to problems with mbstowcs)

  string word1_cpy = word1;
  if (normalizeWords == true) {
   word1_cpy = globalStringConverter.convert(word1,
                                           StringConverter::ENCODING_UTF8,
                                           StringConverter::CONVERSION_TO_LOWER);

   log << "NEW: CALL generalizedDistanceCalculator with "
       << word1_cpy << " and " << " word2 " << endl;
  }

  if (computeGeneralizedEditDistance)
  {
    CS_ASSERT(generalizedDistanceCalculator != NULL);
    if (encodingIsUtf8) normalizedEditDistance
        = generalizedDistanceCalculator->calculateUtf8(word1_cpy, word2, 1.0);
    else normalizedEditDistance = generalizedDistanceCalculator->calculate(
        word1_cpy, word2, 1.0);
  }
  else
  {
    normalizedEditDistance
        = _fuzzySearcher->usingNormalization() ? _fuzzySearcher->getDistance(
            _fuzzySearcher->normalizeWord(word1_cpy),
            _fuzzySearcher->normalizeWord(word2)) : _fuzzySearcher->getDistance(
            word1_cpy, word2);
    if (!_fuzzySearcher->completionMatching()) normalizedEditDistance
        /= MY_MAX(word1_cpy.length(), word2.length());
    else
    {
      normalizedEditDistance /= MY_MIN(word1_cpy.length(), word2.length()); // +
      //1 - 1.0 / MY_MAX(word1.length(), word2.length()));
    }
  }
  if (normalizedEditDistance >= 1.0) normalizedEditDistance = 1;
  return normalizedEditDistance;
}

// EXPLICIT INSTANTIATIONS (so that actual code gets generated)
template void
    CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>::computeTopHitsAndCompletions(
        QueryResult& result);

template void CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>::remapWordIds(
    const WordList& wordListIn, WordList* wordListOut);
