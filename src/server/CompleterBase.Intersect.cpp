#include "CompleterBase.h"

//! Provide operator which takes a word id and always returns true
/*!
 *    Note: used as a template parameter in the new intersect when all word ids
 *    in the second list are known to be within the limits given by the query
 */
class AnyWordIdOk
{ 
 public: 
  bool operator()(WordId w) const { return true; }
  WordRange getWordIdRange() const { return infiniteWordIdRange; }
};

AnyWordIdOk anyWordIdOk;

//! Provide operator which returns true iff the given word id is within set range
/*!
 *    Note: used as a template parameter in the new intersect when the second
 *    list is a superset of the relevants postings
 */
class CheckWordId
{
  WordId _min, _max;
 public:
  CheckWordId() { _min = 0; _max = INT_MAX; }
  CheckWordId(WordId min, WordId max) { set(min, max); }
  void set(WordId min, WordId max) { _min = min; _max = max; }
  WordId getMin() const { return _min; }
  WordId getMax() const { return _max; }
  WordRange getWordIdRange() const { return WordRange(_min, _max); }
  bool operator()(WordId wordId) const { return wordId >= _min && wordId <= _max; }
};

//! Provide operator which takes a position difference and always returns true
/*!
 *    Note: used as a template parameter in the new intersect when all positions
 *    from the second list are eligible for output (space operator)
 */
class AnyPositionOk
{
 public:
  bool operator()(unsigned int posDiff) const { return true; }
  int getLeft() const { return 0; }
  int getRight() const { return 0; }
  unsigned int getMaxDiff() const { return UINT_MAX; }
  bool needToCheckPositions() const { return false; }
};

AnyPositionOk anyPositionOk;

//! Provide operator which returns true iff given position *difference* is bounded
/*!
 *    Note: used as a template parameter in the new intersect when positions
 *    from the second list must be within a certain window [a,b] of some
 *    position in the first list. The maximal position difference allowed is
 *    then b - a. 
 *
 *    The usage in the main loop of the new intersect is (deliberately) such 
 *    that only non-negative position differences are evaluated, hence the
 *    argument to the operator is (deliberately) unsigned int. See there.
 */
class CheckPosition
{ 
  int _left;
  int _right;
  unsigned int _maxDiff;
 public:
  CheckPosition() { set(-INT_MAX, INT_MAX); }
  CheckPosition(int left, int right) { set(left, right); }
  void set(int left, int right) { _left = left; _right = right; _maxDiff = right - left; }
  int getLeft() const { return _left; }
  int getRight() const { return _right; }
  unsigned int getMaxDiff() const { return _maxDiff; }
  bool operator()(unsigned int posDiff) const { return posDiff <= _maxDiff; } 
  bool needToCheckPositions() const { return true; }
};


//! Intersect two posting lists, NEW METHOD TO BE ACTUALLY CALLED BY APPLICATION   III
/*!
 */

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
 *
 *    TODO: better have scoreAggregation before wordIdRange. Rationale: one
 *    always wants to specify the score aggregation, but filtering the word ids
 *    is more of an optional thing.
 */
template<unsigned char MODE>
void CompleterBase<MODE>::intersectTwoPostingLists
      (const QueryResult&     input1,
       const QueryResult&     input2,
             QueryResult&     result,
       const Separator&       separator,
       const ScoreAggregation scoreAggregation,
       const WordRange&       wordIdRange)
{
  if (scoreAggregation == SCORE_AGG_SUM)
  {
    SumAggregation scoreAggregationSum;
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, separator, wordIdRange, scoreAggregationSum);
  }
  else if (scoreAggregation == SCORE_AGG_MAX)
  {
    MaxAggregation scoreAggregationMax;
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, separator, wordIdRange, scoreAggregationMax);
  }
  else if (scoreAggregation == SCORE_AGG_SUM_WITH_BONUS)
  {
    SumProxAggregation scoreAggregationSumWithBonus;
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, separator, wordIdRange, scoreAggregationSumWithBonus);
  }
  else
  {
    ostringstream os;
    os << "score aggregation none of SUM, MAX, SUM_WITH_BONUS (" << scoreAggregation << ")";
    CS_THROW(Exception::NOT_YET_IMPLEMENTED, os.str());
  }
}



//! Intersect two posting lists NEW; partially templated version 
/*!
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingLists to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsTemplated.
 */
template<unsigned char MODE>
template<class ScoreAggregator>
void CompleterBase<MODE>::intersectTwoPostingListsPartiallyTemplated
      (const QueryResult&     input1,
       const QueryResult&     input2,
             QueryResult&     result,
       const Separator&       separator,
       const WordRange&       wordIdRange,
       const ScoreAggregator& aggregateScores)
{
  if (wordIdRange.isInfiniteRange() == true)
  {
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, separator, anyWordIdOk, aggregateScores);
  }
  else 
  {
    // Note: for OUTPUT_NON_MATCHES and OUTPUT_ALL the roles of input1 and input2
    // are switched, and it doesn't make sense to apply a word range pertaining
    // to input2 to input1. Therefore for these two modes, the word range is set
    // to infinity. This is not done here, but two methods further down.
    // 
    // TODO: This, however, has the effect that input1 is not filtered as it
    // should, which can leads to wrong result (too many postings removed for
    // OUTPUT_NON_MATCHES or too many score boosts for OUTPUT_ALL).
    //
    // if (separator.getOutputMode() == Separator::OUTPUT_NON_MATCHES)
    //   CS_THROW(Exception::OTHER, "may not restrict word range with output mode OUTPUT_NON_MATCHES");
    // if (separator.getOutputMode() == Separator::OUTPUT_ALL)
    //   CS_THROW(Exception::OTHER, "may not restrict word range with output mode OUTPUT_ALL");
    //
    CheckWordId checkWordId(wordIdRange.firstElement(), wordIdRange.lastElement());
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, separator, checkWordId, aggregateScores);
  }
}



//! Intersect two posting lists NEW; partially templated version 
/*!
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingLists to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsTemplated.
 */
template<unsigned char MODE>
template<class ScoreAggregator,
         class CheckWordId>
void CompleterBase<MODE>::intersectTwoPostingListsPartiallyTemplated
      (const QueryResult&     input1,
       const QueryResult&     input2,
             QueryResult&     result,
       const Separator&       separator,
       const CheckWordId&     checkWordId,
       const ScoreAggregator& aggregateScores)
{
  int outputMode = separator.getOutputMode();

  if (separator.hasInfiniteIntersectionWindow())
  {
    AnyPositionOk anyPositionOk;
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, checkWordId, anyPositionOk, aggregateScores, outputMode);
  }
  else 
  {
    int a = separator.getIntersectionWindowLeftBoundary();
    int b = separator.getIntersectionWindowRightBoundary();
    CheckPosition checkPosition(a, b);
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, checkWordId, checkPosition, aggregateScores, outputMode);
  }
}



//! Intersect two posting lists NEW; partially templated version 
/*!
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingLists to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsTemplated.
 */
template<unsigned char MODE>
template<class ScoreAggregator,
         class CheckWordId,
         class CheckPosition>
void CompleterBase<MODE>::intersectTwoPostingListsPartiallyTemplated
      (const QueryResult&     input1,
       const QueryResult&     input2,
             QueryResult&     result,
       const CheckWordId&     checkWordId,
       const CheckPosition&   checkPosition,
       const ScoreAggregator& aggregateScores,
       const unsigned int     outputMode)
{
  if (outputMode == Separator::OUTPUT_MATCHES)
  {
    intersectTwoPostingListsTemplated
      <CheckWordId, CheckPosition, ScoreAggregator, Separator::OUTPUT_MATCHES>
      (input1, input2, result, checkWordId, checkPosition, aggregateScores);
  }
  else if (outputMode == Separator::OUTPUT_NON_MATCHES)
  {
    log << "! WARNING: operator NOT not yet working properly, "
           "all postings from block(s) matching the NOT part are removed (need to filter first)" << endl;
    // Note 1: must switch order of input1 and input2
    // Note 2: must not check word ids (word range pertains to input2)
    intersectTwoPostingListsTemplated
      <AnyWordIdOk, CheckPosition, ScoreAggregator, Separator::OUTPUT_NON_MATCHES>
      (input2, input1, result, anyWordIdOk, checkPosition, aggregateScores);
  }
  else if (outputMode == Separator::OUTPUT_ALL)
  {
    log << "! WARNING: operator OPTIONAL not yet working properly, "
           "all postings from block(s) matching the OPTIONAL part boost the score (need to filter first)" << endl;
    // Note 1: must switch order of input1 and input2
    // Note 2: must not check word ids (word range pertains to input2)
    intersectTwoPostingListsTemplated
      <AnyWordIdOk, CheckPosition, ScoreAggregator, Separator::OUTPUT_ALL>
      (input2, input1, result, anyWordIdOk, checkPosition, aggregateScores);
  }
  else
  {
    ostringstream os;
    os << "invalid output mode (" << outputMode << ")";
    CS_THROW(Exception::OTHER, os.str());
  }
}



//! Intersect two posting lists NEW; fully templated version   NNN
/*!
 *    This method contains the actual code doing the intersection.
 *
 *    TODO: Make notes on that the templating is really important for efficiency
 */
template<unsigned char MODE>
template<class        CheckWordId, 
         class        CheckPosition, 
         class        ScoreAggregator,
         unsigned int outputMode>
void CompleterBase<MODE>::intersectTwoPostingListsTemplated
      (const QueryResult&     input1,
       const QueryResult&     input2,
             QueryResult&     result,
       const CheckWordId&     checkWordId,
       const CheckPosition&   checkPosition,
       const ScoreAggregator& aggregateScores)
{
  log << AT_BEGINNING_OF_METHOD 
      << "; input list 1 has " << input1.getSize() << " postings"
      << "; input list 2 has " << input2.getSize() << " postings"
      << "; outputMode is "    << Separator::getOutputModeAsChar(outputMode)
      << "; wordRange is "     << checkWordId.getWordIdRange()
      << "; needToCheckPositions is " << (checkPosition.needToCheckPositions() ? "true" : "false")
      << endl;

  // NEW(bast 28Sep11): When aggreating with special posting, always use sum. HHH.
  SumAggregation aggregateWithSpecialPosting;

  // The non-standard output mode only work for the case, where positions need not be checked
  if (outputMode != Separator::OUTPUT_MATCHES) CS_ASSERT(checkPosition.needToCheckPositions() == false);

  // Note: input1 = conceptual full list NOT YET IMPLEMENTED (and maybe no longer needed?)
  if (input1.isFullResult()) CS_THROW(Exception::NOT_YET_IMPLEMENTED, "input1 = full list");

  const DocList&      docIds1    = input1._docIds;
  const WordList&     wordIds1   = input1._wordIdsOriginal;
  const PositionList& positions1 = input1._positions;
  const ScoreList&    scores1    = input1._scores;
  const DocList&      docIds2    = input2._docIds;
  const WordList&     wordIds2   = input2._wordIdsOriginal;
  const PositionList& positions2 = input2._positions;
  const ScoreList&    scores2    = input2._scores;
        DocList&      docIds3    = result._docIds;
        WordList&     wordIds3   = result._wordIdsOriginal;
        PositionList& positions3 = result._positions;
        ScoreList&    scores3    = result._scores;
  unsigned int len1 = docIds1.size();
  unsigned int len2 = docIds2.size();
  resizeAndReserveTimer.cont();
  docIds3   .reserve(docIds3.size() + len2);
  wordIds3  .reserve(wordIds3.size() + len2);
  positions3.reserve(positions3.size() + len2);
  scores3   .reserve(scores3.size() + len2);
  resizeAndReserveTimer.stop();
  unsigned int i = 0;
  unsigned int j = 0;
  //WordId wordId;
  //int posDiff; 
  //Score score;
  //DocId lastDocId1;
  const bool needToCheckPositions = checkPosition.needToCheckPositions();
  const bool needToScanListsToEnd = outputMode == Separator::OUTPUT_MATCHES ? false : true;
  CS_ASSERT(checkPosition.getRight() >= checkPosition.getLeft());
  // at most one of the two is non-zero, examples:
  //   [-3,3] => add 3 to positions from input1
  //   [4,17] => add 4 to positions from input2
  //   [0,10] => leave both positions as they are
  const unsigned int offset1 = checkPosition.getLeft() > 0 ?  checkPosition.getLeft() : 0;
  const unsigned int offset2 = checkPosition.getLeft() < 0 ? -checkPosition.getLeft() : 0;
  #ifndef NDEBUG
  cout << "Offset 1: " << offset1 << endl;
  cout << "Offset 2: " << offset2 << endl;
  #endif
    //cout << "Output Mode : " << outputMode << endl;
    //cout << "needToCheckPositions : " << needToCheckPositions << endl;
    //cout << endl;

  // The following "trace" of the order in which the elements from the three
  // lists were encountered is built along with the computation:
  //
  // For example: 1, 1, 2, 2, 3, 2, 1, 1, 2, 3, ... would mean the first two
  // postings were from input1, then came two postings from input2, then one
  // posting was written to input3, etc.
  //
  // Note: The length of the trace will be at most the length of the three list
  // length. It will be shorter iff the largest doc id is not the same in both
  // input1 and input2, in which case the list with the larger doc id is not
  // scanned to the end
  #ifdef CHECK_INTERSECT
  vector<char> trace; 
  #endif

  // Main Loop: Iterate through the two lists in the order of ascending doc id.
  // Explain what happens by the following example, for which 2 iterations will
  // be done:
  //
  //   L1 : D10 D12         D27 D39         
  //   L2 :         D13 D24         D39 D39 D56
  //
  intersectionTimer.cont();
  while (true)
  {
    // At this point, we can have j == len2 for two reasons: (1) The second list
    // could have been empty so that we have len2 == 0 and hence j == len2 right
    // at the beginning of the loop; (2) it can happen that at the end of the
    // loop (see comment there) we have j == len2 (but still i < len1, so that
    // the condition of the next while below can potentially lead to a
    // segmentation fault).
    // NOTE: this was a bug, which led to segmentation faults in the past, when
    // this routine was called with an empty second list. In principle, it could
    // have also crashed for non-empty second lists but it seems that never
    // happened. Anyway, with the if (j == len2) break; below this issue should
    // be solved now.
    if (j == len2) break;

    // Main Loop, Step 1: Advance in first list
    //
    //   L1 : D10 D12         D27 D39         
    //   L2 :         D13 D24         D39 D39 D56
    //   #1   i=0 i=1         i=2
    //   #2                                       i=4=len1            
    while (i < len1 && docIds1[i] < docIds2[j]) 
    {
      #ifdef CHECK_INTERSECT
      trace.push_back(1);
      #endif
      ++i;
    }
    if (i == len1 && !needToScanListsToEnd) break;

    // Main Loop, Step 2: Advance in second list 
    //
    // Note: for OUTPUT_NON_MATCHES or OUTPUT_ALL, add to output
    //
    //   L1 : D10 D12         D27 D39         
    //   L2 :         D13 D24         D39 D39 D56
    //   #1           j=0 j=1         j=2
    //   #2                                   j=4 j=5=len2
    //
    while (j < len2 && (i == len1 || docIds2[j] < docIds1[i]))
    // while (j < len2 && (docIds2[j] < docIds1[i] 
    //                      || (i == len1 && needToScanListsToEnd))) 
    {
      #ifdef CHECK_INTERSECT
      trace.push_back(2);
      #endif
      // Note: if outputMode were not a template parameter but an ordinary
      // argument, the following if alone costs a factor of two (10 seconds
      // versus 5 seconds for the query "ct:venue:soda:* ct:author:*" on dblp
      if (outputMode == Separator::OUTPUT_NON_MATCHES || outputMode == Separator::OUTPUT_ALL)
      {
        if (checkWordId(wordIds2[j]))
        {
          docIds3.   push_back(docIds2[j]);
          wordIds3.  push_back(wordIds2[j]);
          positions3.push_back(positions2[j]);
          scores3.   push_back(scores2[j]);
          #ifdef CHECK_INTERSECT
          trace.push_back(3);
          #endif
        }
      }
      ++j;
    }
    if (j == len2) break;

    // At this point j < len2 (otherwise we would have exited the loop with the
    // break above) and i < len1 (otherwise the previous loop would have gone
    // until j == len2 and the break afterwards would have been executed).
    assert(j < len2);
    assert(i < len1);
   
    // Main Loop, Step 3: Same doc ids
    //
    if (docIds1[i] == docIds2[j])
    {
      // log << "* DEBUG INTERSECT: D1[" << i << "] = " << docIds1[i]
      //     << ", D2[" << j << "] = " << docIds2[j]
      //     << ", |D1| = " << len1 << ", |D2| = " << len2 << endl;
      DocId docId = docIds1[i];
      // Case 1: no need to look at positions.
      //
      // NEW(bast, 15Dec10): The case with more than one posting with the current
      // doc id in the first list was not dealt with properly so far. Only the
      // first posting from the first list was considered and its score was
      // aggregated with each posting from the second list. This had two problems:
      // (1) in the first list, all postings with the current doc id after the
      // first were simply discarded; (2) in computeTopHitsAndCompletions the
      // score from the first posting from the first list was effectively counted
      // k times, where k is the number of postings in the second list.
      // We now proceed as follows: all postings from the first list (with the
      // current doc id) are aggregated to a single special posting with word id
      // NO_WORD_ID and position NO_POSITION, which is written to the result list.
      // In computeTopHits score of this posting contributes to the doc score just
      // like any other posting. In computeTopCompletions this posting is ignored.
      if (needToCheckPositions == false)
      {
        // First write the postings from the second list (whenever the word id is
        // in the given range).
        bool atLeastOnePostingWritten = false;
        while (j < len2 && docIds2[j] == docId)
        {
          #ifdef CHECK_INTERSECT
          trace.push_back(2);
          #endif
          // For OUTPUT_NON_MATCHES, don't add to output.
          if (outputMode == Separator::OUTPUT_MATCHES ||
              outputMode == Separator::OUTPUT_ALL)
          {
            if (checkWordId(wordIds2[j]))
            {
              docIds3.push_back(docIds2[j]);
              wordIds3.push_back(wordIds2[j]);
              positions3.push_back(positions2[j]);
              scores3.push_back(scores2[j]);
              atLeastOnePostingWritten = true;
              // scores3.push_back(aggregateScores.aggregate(scores1[i], scores2[j]) +
              //   (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
              #ifdef CHECK_INTERSECT
              trace.push_back(3);
              #endif
            }
          }
          ++j;
        }
        // Now write the special posting with the aggregated scores from the
        // first list. If the second list ended with a special posting,
        // aggregate that score, too. A special posting from the first list is
        // automatically considered.
        if (atLeastOnePostingWritten)
        {
          Score score = scores1[i++];
          while (i < len1 && docIds1[i] == docId)
            score = wordIds1[i] != SPECIAL_WORD_ID
	      // NOTE(bast): this aggregates two ordinary postings. HHH
	      ? aggregateScores.aggregate(score, scores1[i++])
	      // NOTE(bast): this aggregates with a special posting. HHH
	      : aggregateWithSpecialPosting.aggregate(score, scores1[i++]);
	      // : aggregateScores.aggregate(score, scores1[i++]);
          if (wordIds3.back() == SPECIAL_WORD_ID)
          {
	    // NOTE(bast): this aggregates two special postings. HHH
            Score& lastScore = scores3.back();
            lastScore = aggregateWithSpecialPosting.aggregate(lastScore, score);
            // lastScore = aggregateScores.aggregate(lastScore, score);
          }
          else
          {
            docIds3.push_back(docId);
            wordIds3.push_back(SPECIAL_WORD_ID);
            positions3.push_back(SPECIAL_POSITION);
            scores3.push_back(score);
          }
        }

        if (i == len1 && !needToScanListsToEnd) break;
        if (j == len2) break;
      }

      // Case 2: have to look at positions. Merge them!
      //
      // TODO(bast): Check that the scoring works properly here, too. So far, no
      // special treatment of special postings (with word id SPECIAL_WORD_ID)
      // here.
      else
      {
        // Loop to merge the positions
        //
        // Here is an example. All positions are from postings with the same doc id, 
        // previous postings are from a smaller doc id (or none if the doc id is
        // the first in the list), following postings are from a larger doc id (or
        // none if the doc id is the last in the list)
        //
        //   L2:  ... P1            P10 P12 P13         P37 P39 ...
        //   L1:  ...     P3 P8 P10             P19 P35         ...
        //        i-1  j        i-1  j              i-1  j       
        //
        // When the second while loop starts, i-1 and j are the positions drawn
        //
        // Note: a fixed non-negative offset is added for each position list. At
        // most one of these offsets is non-zero. This effectively shifts 
        //
        while (true)
        {
          while (i < len1 && docIds1[i] == docId 
                   && positions1[i] + offset1 <= positions2[j] + offset2) 
          {
            #ifdef CHECK_INTERSECT
            trace.push_back(1);
            #endif
            ++i;
          }
          bool done1 = (i == len1 || docIds1[i] != docId);
          // at this point, either done1 or positions1[i] + offset1 >
          // positions2[j] + offset2, so that the while loop will do at least one
          // iteration, or the following if "(j == len2 || docIds2[j] != docId)
          // ..." will break
          while (j < len2 && docIds2[j] == docId 
                  && (done1 || positions2[j] + offset2 < positions1[i] + offset1)) 
          {
            #ifdef CHECK_INTERSECT
            trace.push_back(2);
            #endif
            if (i > 0 && docIds1[i-1] == docId)
            {
              // Beware to use ...[i-1] not ...[i] for input1 here!!! 
              bool wordIdOk = checkWordId(wordIds2[j]);
              bool posDiffOk = checkPosition(positions2[j] + offset2 - (positions1[i-1] + offset1));
              // NEW (baumgari) 14Nov14:
              // At this point we look within a specified window. It might
              // happen, that a word is entered more than once, like in the
              // query "information..information". In this case it's likely,
              // that the user wants to find documents in which "information"
              // occurs at least twice. Until now this didn't work. Now we do
              // only intersect the given matches, if they differ.
              // TODO (baumgari): I could not find any disadvantages because of
              // that. This should only be relevant if we have a fixed
              // neighbourhood size, which is only set for the operators "." and
              // "..".
              bool sameWord = (positions1[i-1] == positions2[j]);
              if (0)
              {
                cout << "Doc" << setw(6) << setfill('0') << docId << " :  "  
                     << "Pos" << setw(5) << setfill('0') << positions1[i-1] << " / " 
                     << "Pos" << setw(5) << setfill('0') << positions2[j] 
                     << " ; WordId : " << (wordIdOk ? "YES" : "NO")
                     << " ; PosDiff : " << (posDiffOk ? "YES" : "NO") 
                     << endl;
              }
              if (posDiffOk == true && !sameWord
                    && (outputMode == Separator::OUTPUT_MATCHES || outputMode == Separator::OUTPUT_ALL))
              {
                if (wordIdOk == true)
                {
                  docIds3.   push_back(docIds2[j]);
                  wordIds3.  push_back(wordIds2[j]);
                  positions3.push_back(positions2[j]);
                  scores3.   push_back(aggregateScores.aggregate(scores1[i-1], scores2[j]));
                  #ifdef CHECK_INTERSECT
                  trace.push_back(3);
                  #endif
                }
              }
              else if ((posDiffOk == false || sameWord)
                        && (outputMode == Separator::OUTPUT_NON_MATCHES || outputMode == Separator::OUTPUT_ALL))
              {
                if (wordIdOk == true)
                {
                  docIds3.   push_back(docIds2[j]);
                  wordIds3.  push_back(wordIds2[j]);
                  positions3.push_back(positions2[j]);
                  scores3.   push_back(scores2[j]);
                  #ifdef CHECK_INTERSECT
                  trace.push_back(3);
                  #endif
                }
              }
            }
            ++j;
          }
          if (j == len2 || docIds2[j] != docId) break;
        }
      }
    }

    // Note: At this point (at the end of the body of the main while loop), both
    // i == len1 and j = len2 can happen (and all combinations, i.e. only one of
    // them true or both true).
  }
  intersectionTimer.stop();
  intersectNofPostings += i + j;

  // Unreserve the additional space (Note: might involve a copy)
  resizeAndReserveTimer.cont();
  docIds3   .unreserve();
  wordIds3  .unreserve();
  positions3.unreserve();
  scores3   .unreserve();
  resizeAndReserveTimer.stop();

  log << AT_END_OF_METHOD << "; result list has " << result.getSize() << " postings" << endl;

  // Check the result, using the "trace" computed along with the above
  #ifdef CHECK_INTERSECT
  cout << "Checking intersect result ... " << flush;
  {
    CS_ASSERT(trace.size() <= docIds1.size() + docIds2.size() + docIds3.size());
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    int positionWindowLeft  = checkPosition.getLeft();
    int positionWindowRight = checkPosition.getRight();
    cout << needToCheckPositions << endl;
      //cout << positionWindowLeft << ".." << positionWindowRight << endl;
    DocId        lastDocId;
    Position     lastPosition;
    int          lastList;
    unsigned int lastIndex;
    ostringstream os;
    for (unsigned int l = 0; l < trace.size(); ++l)
    {
      if (trace[l] == 1)
      {
        CS_ASSERT(i < docIds1.size());
        // check sortedness
        bool orderOk = needToCheckPositions
                        ? (l == 0 || lastDocId < docIds1[i] 
                            || lastDocId == docIds1[i] && lastPosition <= positions1[i])
                        : (l == 0 || lastDocId <= docIds1[i]);
        if (orderOk == false)
        {
          os << "elements not processed in sorted order in intersect:"
             << " posting " << i << " of list 1 with doc id " << docIds1[i]
             << " and position " << positions1[i] << " processed after"
             << " posting " << lastIndex << " of list " << lastList
             << " with doc id " << lastDocId << " and position " << lastPosition;
          CS_THROW(Exception::CHECK_FAILED, os.str());
        }
        lastDocId    = docIds1[i];
        lastPosition = positions1[i];
        lastList     = 1;
        lastIndex    = i;
        ++i;
      }
      else if (trace[l] == 2)
      {
        CS_ASSERT(j < docIds2.size());
        // check sortedness
        bool orderOk = needToCheckPositions
                        ? (l == 0 || lastDocId < docIds2[j] 
                            || lastDocId == docIds2[j] && lastPosition <= positions2[j])
                        : (l == 0 || lastDocId <= docIds2[j]);
        if (orderOk == false)
        {
          os << "elements not processed in sorted order in intersect:"
             << " posting " << j << " of list 2 with doc id " << docIds2[j]
             << " and position " << positions2[j] << " processed after"
             << " posting " << lastIndex << " of list " << lastList
             << " with doc id " << lastDocId << " and position " << lastPosition;
          CS_THROW(Exception::CHECK_FAILED, os.str());
        }
        lastDocId    = docIds2[j];
        lastPosition = positions2[j];
        lastList     = 2;
        lastIndex    = j;
        // check possible match
        if (i > 0)
        {
          //
        }
        ++j;
      }
      else if (trace[l] == 3)
      {
        CS_ASSERT(k < docIds3.size());
        CS_ASSERT(i > 0);
        CS_ASSERT(j > 0);
        CS_ASSERT(docIds3[k] == docIds1[i-1]);
        CS_ASSERT(docIds3[k] == docIds2[j-1]);
        CS_ASSERT(checkWordId(wordIds3[k]));
        CS_ASSERT(positions3[k] == positions2[j-1]);
        bool positionOk1 = positionWindowLeft == -INT_MAX
                            || (int)(positions1[i-1]) + positionWindowLeft <= (int)(positions3[k]);
        bool positionOk2 = positionWindowRight == INT_MAX
                            || (int)(positions3[k]) <= (int)(positions1[i-1]) + positionWindowRight;
        if (positionOk1 == false || positionOk2 == false)
        {
          os << "position " << positions3[k] << " of posting " << k << " from intersect result" 
             << " does not lie in window [" << positionWindowLeft << ".."
             << positionWindowRight << "] around position " << positions1[i-1]
             << " of posting " << i-1 << " from first input list";
          CS_THROW(Exception::CHECK_FAILED, os.str());
        }
        ++k;
      }
      else
      {
        CS_THROW(Exception::OTHER, "wrong value in check trace");
      }
    }
    cout << "OK" << endl;
  }
  #endif
}

//! Intersect two posting lists NEW; fully templated version   NNN
/*!
 *    This method is like intersectTwoPostingListsTemplated with some changes
 *    in Algorithm like replacing of pushback with []
 */
template<unsigned char MODE>
template<class        CheckWordId,
         class        CheckPosition,
         class        ScoreAggregator,
         unsigned int outputMode>
void CompleterBase<MODE>::intersectTwoPostingListsTemplated2
      (const QueryResult&     input1,
       const QueryResult&     input2,
             QueryResult&     result,
       const CheckWordId&     checkWordId,
       const CheckPosition&   checkPosition,
       const ScoreAggregator& aggregateScores)
{
  log << AT_BEGINNING_OF_METHOD
      << "; input list 1 has " << input1.getSize() << " postings"
      << "; input list 2 has " << input2.getSize() << " postings"
      << "; outputMode is "    << Separator::getOutputModeAsChar(outputMode)
      << "; wordRange is "     << checkWordId.getWordIdRange()
      << "; needToCheckPositions is " << (checkPosition.needToCheckPositions() ? "true" : "false")
      << endl;

  // The non-standard output mode only work for the case, where positions need not be checked
  if (outputMode != Separator::OUTPUT_MATCHES) CS_ASSERT(checkPosition.needToCheckPositions() == false);

  // Note: input1 = conceptual full list NOT YET IMPLEMENTED (and maybe no longer needed?)
  if (input1.isFullResult()) CS_THROW(Exception::NOT_YET_IMPLEMENTED, "input1 = full list");

  const DocList&      docIds1    = input1._docIds;
  //const WordList&     wordIds1   = input1._wordIdsOriginal;
  const PositionList& positions1 = input1._positions;
  const ScoreList&    scores1    = input1._scores;
  const DocList&      docIds2    = input2._docIds;
  const WordList&     wordIds2   = input2._wordIdsOriginal;
  const PositionList& positions2 = input2._positions;
  const ScoreList&    scores2    = input2._scores;
        DocList&      docIds3    = result._docIds;
        WordList&     wordIds3   = result._wordIdsOriginal;
        PositionList& positions3 = result._positions;
        ScoreList&    scores3    = result._scores;
  unsigned int len1 = docIds1.size();
  unsigned int len2 = docIds2.size();
  resizeAndReserveTimer.cont();
  docIds3   .reserve(docIds3.size() + len2);
  wordIds3  .reserve(wordIds3.size() + len2);
  positions3.reserve(positions3.size() + len2);
  scores3   .reserve(scores3.size() + len2);

  docIds3   .resize(docIds3.size() + len2, 0);
  wordIds3  .resize(wordIds3.size() + len2, 0);
  positions3.resize(positions3.size() + len2, 0);
  scores3   .resize(scores3.size() + len2, 0);
  resizeAndReserveTimer.stop();
  unsigned int i = 0;
  unsigned int j = 0;
  //WordId wordId;
  //int posDiff;
  //Score score;
  //DocId lastDocId1;
  const bool needToCheckPositions = checkPosition.needToCheckPositions();
  const bool needToScanListsToEnd = outputMode == Separator::OUTPUT_MATCHES ? false : true;
  CS_ASSERT(checkPosition.getRight() >= checkPosition.getLeft());
  // at most one of the two is non-zero, examples:
  //   [-3,3] => add 3 to positions from input1
  //   [4,17] => add 4 to positions from input2
  //   [0,10] => leave both positions as they are
  const unsigned int offset1 = checkPosition.getLeft() > 0 ?  checkPosition.getLeft() : 0;
  const unsigned int offset2 = checkPosition.getLeft() < 0 ? -checkPosition.getLeft() : 0;
  #ifndef NDEBUG
  cout << "Offset 1: " << offset1 << endl;
  cout << "Offset 2: " << offset2 << endl;
  #endif
    //cout << "Output Mode : " << outputMode << endl;
    //cout << "needToCheckPositions : " << needToCheckPositions << endl;
    //cout << endl;

  // The following "trace" of the order in which the elements from the three
  // lists were encountered is built along with the computation:
  //
  // For example: 1, 1, 2, 2, 3, 2, 1, 1, 2, 3, ... would mean the first two
  // postings were from input1, then came two postings from input2, then one
  // posting was written to input3, etc.
  //
  // Note: The length of the trace will be at most the length of the three list
  // length. It will be shorter iff the largest doc id is not the same in both
  // input1 and input2, in which case the list with the larger doc id is not
  // scanned to the end
  #ifdef CHECK_INTERSECT
  vector<char> trace;
  #endif

  // Main Loop: Iterate through the two lists in the order of ascending doc id.
  // Explain what happens by the following example, for which 2 iterations will
  // be done:
  //
  //   L1 : D10 D12         D27 D39
  //   L2 :         D13 D24         D39 D39 D56
  //
  intersectionTimer.cont();
  //
  size_t resultListCounter = 0 ;
  while (true)
  {
    // At this point, we can have j == len2 for two reasons: (1) The second list
    // could have been empty so that we have len2 == 0 and hence j == len2 right
    // at the beginning of the loop; (2) it can happen that at the end of the
    // loop (see comment there) we have j == len2 (but still i < len1, so that
    // the condition of the next while below can potentially lead to a
    // segmentation fault).
    // NOTE: this was a bug, which led to segmentation faults in the past, when
    // this routine was called with an empty second list. In principle, it could
    // have also crashed for non-empty second lists but it seems that never
    // happened. Anyway, with the if (j == len2) break; below this issue should
    // be solved now.
    if (j == len2) break;

    // Main Loop, Step 1: Advance in first list
    //
    //   L1 : D10 D12         D27 D39
    //   L2 :         D13 D24         D39 D39 D56
    //   #1   i=0 i=1         i=2
    //   #2                                       i=4=len1
    while (i < len1 && docIds1[i] < docIds2[j])
    {
      #ifdef CHECK_INTERSECT
      trace.push_back(1);
      #endif
      ++i;
    }
    if (i == len1 && !needToScanListsToEnd) break;

    // Main Loop, Step 2: Advance in second list
    //
    // Note: for OUTPUT_NON_MATCHES or OUTPUT_ALL, add to output
    //
    //   L1 : D10 D12         D27 D39
    //   L2 :         D13 D24         D39 D39 D56
    //   #1           j=0 j=1         j=2
    //   #2                                   j=4 j=5=len2
    //
    while (j < len2 && (i == len1 || docIds2[j] < docIds1[i]))
    // while (j < len2 && (docIds2[j] < docIds1[i]
    //                      || (i == len1 && needToScanListsToEnd)))
    {
      #ifdef CHECK_INTERSECT
      trace.push_back(2);
      #endif
      // Note: if outputMode were not a template parameter but an ordinary
      // argument, the following if alone costs a factor of two (10 seconds
      // versus 5 seconds for the query "ct:venue:soda:* ct:author:*" on dblp
      if (outputMode == Separator::OUTPUT_NON_MATCHES || outputMode == Separator::OUTPUT_ALL)
      {
        if (checkWordId(wordIds2[j]))
        {
          docIds3[docIds2[j]];
          wordIds3[wordIds2[j]];
          positions3[positions2[j]];
          scores3[scores2[j]];
          resultListCounter++;
          #ifdef CHECK_INTERSECT
          trace.push_back(3);
          #endif
        }
      }
      ++j;
    }
    if (j == len2) break;

    // At this point j < len2 (otherwise we would have exited the loop with the
    // break above) and i < len1 (otherwise the previous loop would have gone
    // until j == len2 and the break afterwards would have been executed).
    assert(j < len2);
    assert(i < len1);

    // Main Loop, Step 3: Same doc ids
    //
    if (docIds1[i] == docIds2[j])
    {
      // log << "* DEBUG INTERSECT: D1[" << i << "] = " << docIds1[i]
      //     << ", D2[" << j << "] = " << docIds2[j]
      //     << ", |D1| = " << len1 << ", |D2| = " << len2 << endl;
      DocId docId = docIds1[i];
      // Case 1: no need to look at positions.
      //
      // NEW(bast, 15Dec10): The case with more than one posting with the current
      // doc id in the first list was not dealt with properly so far. Only the
      // first posting from the first list was considered and its score was
      // aggregated with each posting from the second list. This had two problems:
      // (1) in the first list, all postings with the current doc id after the
      // first were simply discarded; (2) in computeTopHitsAndCompletions the
      // score from the first posting from the first list was effectively counted
      // k times, where k is the number of postings in the second list.
      // We now proceed as follows: all postings from the first list (with the
      // current doc id) are aggregated to a single special posting with word id
      // NO_WORD_ID and position NO_POSITION, which is written to the result list.
      // In computeTopHits score of this posting contributes to the doc score just
      // like any other posting. In computeTopCompletions this posting is ignored.
      if (needToCheckPositions == false)
      {
        // First write the postings from the second list (whenever the word id is
        // in the given range).
        bool atLeastOnePostingWritten = false;
        while (j < len2 && docIds2[j] == docId)
        {
          #ifdef CHECK_INTERSECT
          trace.push_back(2);
          #endif
          // For OUTPUT_NON_MATCHES, don't add to output.
          if (outputMode == Separator::OUTPUT_MATCHES ||
              outputMode == Separator::OUTPUT_ALL)
          {
            if (checkWordId(wordIds2[j]))
            {
              docIds3[docIds2[j]];
              wordIds3[wordIds2[j]];
              positions3[positions2[j]];
              scores3[scores2[j]];
              resultListCounter++;
              atLeastOnePostingWritten = true;
              // scores3.push_back(aggregateScores.aggregate(scores1[i], scores2[j]) +
              //   (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
              #ifdef CHECK_INTERSECT
              trace.push_back(3);
              #endif
            }
          }
          ++j;
        }
        // Now write the special posting with the aggregated scores from the
        // first list. If the second list ended with a special posting,
        // aggregate that score, too. A special posting from the first list is
        // automatically considered.
        if (atLeastOnePostingWritten)
        {
          Score score = scores1[i++];
          while (i < len1 && docIds1[i] == docId)
            score = aggregateScores.aggregate(score, scores1[i++]);
          if (wordIds3.back() == SPECIAL_WORD_ID)
          {
            Score& lastScore = scores3.back();
            lastScore = aggregateScores.aggregate(lastScore, score);
          }
          else
          {
            docIds3[docId];
            wordIds3[SPECIAL_WORD_ID];
            positions3[SPECIAL_POSITION];
            scores3[score];
            resultListCounter++;
          }
        }

        if (i == len1 && !needToScanListsToEnd) break;
        if (j == len2) break;
      }

      // Case 2: have to look at positions. Merge them!
      //
      // TODO(bast): Check that the scoring works properly here, too. So far, no
      // special treatment of special postings (with word id SPECIAL_WORD_ID)
      // here.
      else
      {
        // Loop to merge the positions
        //
        // Here is an example. All positions are from postings with the same doc id,
        // previous postings are from a smaller doc id (or none if the doc id is
        // the first in the list), following postings are from a larger doc id (or
        // none if the doc id is the last in the list)
        //
        //   L2:  ... P1            P10 P12 P13         P37 P39 ...
        //   L1:  ...     P3 P8 P10             P19 P35         ...
        //        i-1  j        i-1  j              i-1  j
        //
        // When the second while loop starts, i-1 and j are the positions drawn
        //
        // Note: a fixed non-negative offset is added for each position list. At
        // most one of these offsets is non-zero. This effectively shifts
        //
        while (true)
        {
          while (i < len1 && docIds1[i] == docId
                   && positions1[i] + offset1 <= positions2[j] + offset2)
          {
            #ifdef CHECK_INTERSECT
            trace.push_back(1);
            #endif
            ++i;
          }
          bool done1 = (i == len1 || docIds1[i] != docId);
          // at this point, either done1 or positions1[i] + offset1 >
          // positions2[j] + offset2, so that the while loop will do at least one
          // iteration, or the following if "(j == len2 || docIds2[j] != docId)
          // ..." will break
          while (j < len2 && docIds2[j] == docId
                  && (done1 || positions2[j] + offset2 < positions1[i] + offset1))
          {
            #ifdef CHECK_INTERSECT
            trace.push_back(2);
            #endif
            if (i > 0 && docIds1[i-1] == docId)
            {
              // Beware to use ...[i-1] not ...[i] for input1 here!!!
              bool wordIdOk = checkWordId(wordIds2[j]);
              bool posDiffOk = checkPosition(positions2[j] + offset2 - (positions1[i-1] + offset1));
              if (0)
              {
                cout << "Doc" << setw(6) << setfill('0') << docId << " :  "
                     << "Pos" << setw(5) << setfill('0') << positions1[i-1] << " / "
                     << "Pos" << setw(5) << setfill('0') << positions2[j]
                     << " ; WordId : " << (wordIdOk ? "YES" : "NO")
                     << " ; PosDiff : " << (posDiffOk ? "YES" : "NO")
                     << endl;
              }
              if (posDiffOk == true
                    && (outputMode == Separator::OUTPUT_MATCHES || outputMode == Separator::OUTPUT_ALL))
              {
                if (wordIdOk == true)
                {
                  docIds3[docIds2[j]];
                  wordIds3[wordIds2[j]];
                  positions3[positions2[j]];
                  scores3[aggregateScores.aggregate(scores1[i-1], scores2[j])];
                  resultListCounter++;
                  #ifdef CHECK_INTERSECT
                  trace.push_back(3);
                  #endif
                }
              }
              else if (posDiffOk == false
                        && (outputMode == Separator::OUTPUT_NON_MATCHES || outputMode == Separator::OUTPUT_ALL))
              {
                if (wordIdOk == true)
                {
                  docIds3[docIds2[j]];
                  wordIds3[wordIds2[j]];
                  positions3[positions2[j]];
                  scores3[scores2[j]];
                  resultListCounter++;
                  #ifdef CHECK_INTERSECT
                  trace.push_back(3);
                  #endif
                }
              }
            }
            ++j;
          }
          if (j == len2 || docIds2[j] != docId) break;
        }
      }
    }

    // Note: At this point (at the end of the body of the main while loop), both
    // i == len1 and j = len2 can happen (and all combinations, i.e. only one of
    // them true or both true).
  }
  intersectionTimer.stop();
  intersectNofPostings += i + j;

  // Unreserve the additional space (Note: might involve a copy)
  resizeAndReserveTimer.cont();
  docIds3   .unreserve();
  wordIds3  .unreserve();
  positions3.unreserve();
  scores3   .unreserve();
  resizeAndReserveTimer.stop();

  log << AT_END_OF_METHOD << "; result list has " << result.getSize() << " postings" << endl;

  // Check the result, using the "trace" computed along with the above
  #ifdef CHECK_INTERSECT
  cout << "Checking intersect result ... " << flush;
  {
    CS_ASSERT(trace.size() <= docIds1.size() + docIds2.size() + docIds3.size());
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;
    int positionWindowLeft  = checkPosition.getLeft();
    int positionWindowRight = checkPosition.getRight();
    cout << needToCheckPositions << endl;
      //cout << positionWindowLeft << ".." << positionWindowRight << endl;
    DocId        lastDocId;
    Position     lastPosition;
    int          lastList;
    unsigned int lastIndex;
    ostringstream os;
    for (unsigned int l = 0; l < trace.size(); ++l)
    {
      if (trace[l] == 1)
      {
        CS_ASSERT(i < docIds1.size());
        // check sortedness
        bool orderOk = needToCheckPositions
                        ? (l == 0 || lastDocId < docIds1[i]
                            || lastDocId == docIds1[i] && lastPosition <= positions1[i])
                        : (l == 0 || lastDocId <= docIds1[i]);
        if (orderOk == false)
        {
          os << "elements not processed in sorted order in intersect:"
             << " posting " << i << " of list 1 with doc id " << docIds1[i]
             << " and position " << positions1[i] << " processed after"
             << " posting " << lastIndex << " of list " << lastList
             << " with doc id " << lastDocId << " and position " << lastPosition;
          CS_THROW(Exception::CHECK_FAILED, os.str());
        }
        lastDocId    = docIds1[i];
        lastPosition = positions1[i];
        lastList     = 1;
        lastIndex    = i;
        ++i;
      }
      else if (trace[l] == 2)
      {
        CS_ASSERT(j < docIds2.size());
        // check sortedness
        bool orderOk = needToCheckPositions
                        ? (l == 0 || lastDocId < docIds2[j]
                            || lastDocId == docIds2[j] && lastPosition <= positions2[j])
                        : (l == 0 || lastDocId <= docIds2[j]);
        if (orderOk == false)
        {
          os << "elements not processed in sorted order in intersect:"
             << " posting " << j << " of list 2 with doc id " << docIds2[j]
             << " and position " << positions2[j] << " processed after"
             << " posting " << lastIndex << " of list " << lastList
             << " with doc id " << lastDocId << " and position " << lastPosition;
          CS_THROW(Exception::CHECK_FAILED, os.str());
        }
        lastDocId    = docIds2[j];
        lastPosition = positions2[j];
        lastList     = 2;
        lastIndex    = j;
        // check possible match
        if (i > 0)
        {
          //
        }
        ++j;
      }
      else if (trace[l] == 3)
      {
        CS_ASSERT(k < docIds3.size());
        CS_ASSERT(i > 0);
        CS_ASSERT(j > 0);
        CS_ASSERT(docIds3[k] == docIds1[i-1]);
        CS_ASSERT(docIds3[k] == docIds2[j-1]);
        CS_ASSERT(checkWordId(wordIds3[k]));
        CS_ASSERT(positions3[k] == positions2[j-1]);
        bool positionOk1 = positionWindowLeft == -INT_MAX
                            || (int)(positions1[i-1]) + positionWindowLeft <= (int)(positions3[k]);
        bool positionOk2 = positionWindowRight == INT_MAX
                            || (int)(positions3[k]) <= (int)(positions1[i-1]) + positionWindowRight;
        if (positionOk1 == false || positionOk2 == false)
        {
          os << "position " << positions3[k] << " of posting " << k << " from intersect result"
             << " does not lie in window [" << positionWindowLeft << ".."
             << positionWindowRight << "] around position " << positions1[i-1]
             << " of posting " << i-1 << " from first input list";
          CS_THROW(Exception::CHECK_FAILED, os.str());
        }
        ++k;
      }
      else
      {
        CS_THROW(Exception::OTHER, "wrong value in check trace");
      }
    }
    cout << "OK" << endl;
  }
  #endif
}



//! Check the result of an intersection
/*!
 *    Old version, incomplete and to be overridded by what is now part of the
 *    intersect routine
 */
template<unsigned char MODE>
void CompleterBase<MODE>::checkCorrectnessOfIntersectionResult
      (const QueryResult&     input1,
       const QueryResult&     input2,
       const QueryResult&     result)
{
  cout << "Checking ... " << flush;
  // first write all doc ids to a vector, without duplicates, with a reference
  // from which of input1, input2, result they stem, and with the index of the
  // first doc id in that posting list. Sort this list.
  const DocList&      docIds1    = input1._docIds;
  //const WordList&     wordIds1   = input1._wordIdsOriginal;
  //const PositionList& positions1 = input1._positions;
  //const ScoreList&    scores1    = input1._scores;
  const DocList&      docIds2    = input2._docIds;
  //const WordList&     wordIds2   = input2._wordIdsOriginal;
  //const PositionList& positions2 = input2._positions;
  //const ScoreList&    scores2    = input2._scores;
  const DocList&      docIds3    = result._docIds;
  //const WordList&     wordIds3   = result._wordIdsOriginal;
  //const PositionList& positions3 = result._positions;
  //const ScoreList&    scores3    = result._scores;
  typedef pair<DocId, pair<int, unsigned int> > Triple;
  vector<Triple> all;
  Triple triple;
  DocId docId;
  DocId lastDocId = INFTY_DOCID;
  vector<const DocList*> lists(3);
  lists[0] = &docIds3;
  lists[1] = &docIds1;
  lists[2] = &docIds2;
  for (unsigned int l = 0; l < 3; ++l)
  {
    const DocList& docIds = *lists[l];
    for (unsigned int i = 0; i < docIds.size(); ++i)
    {
      docId = docIds[i];
      if (docId == lastDocId) continue;
      lastDocId = docId;
      triple.first         = docId;
      triple.second.first  = l;
      triple.second.second = i;
      all.push_back(triple);
    }
  }
  sort(all.begin(), all.end());
  //for (unsigned int i = 0; i < all.size(); ++i)
  //  cout << "(" << all[i].first << "," << all[i].second.first << ","
  //                                     << all[i].second.second << ") " << flush;

  // for each doc id from result check whether it's in both input1 and input2
  for (unsigned int i = 0; i < all.size(); ++i)
  {
    if (all[i].second.first == 0)
    {
      docId = all[i].first;
      if (i + 1 >= all.size() || all[i+1].first != docId)
      {
        ostringstream os;
        os << "doc id " << docId << " from result list not found in either of the two input lists";
        CS_THROW(Exception::OTHER, os.str());
      }
      else if (i + 2 >= all.size() || all[i+2].first != docId)
      {
        ostringstream os;
        os << "doc id " << docId << " from result list not found in input list " << 3 - all[i+1].second.first;
        CS_THROW(Exception::OTHER, os.str());
      }
      CS_ASSERT(i + 2 < all.size() && all[i+1].second.first == 1 && all[i+2].second.first == 2);
    }
  }

  // check all doc ids common to input1 and input2
  /*
  for (unsigned int i = 0; i + 1 < all.size(); ++i)
  {
    docId = all[i].first;
    if (all[i].second.first == 1 && all[i+1].first == docId)
    {
      CS_ASSERT(all[i+1].second.first == 2);
      unsigned int jj1 == all[i].second.second;
      unsigned int jj2 == all[i+1].second.second;
      CS_ASSERT(jj1 < docIds1.size() && docIds1[jj1] == docId);
      CS_ASSERT(jj2 < docIds2.size() && docIds2[jj2] == docId);
      unsigned int j0 = UINT_MAX;
      for (unsigned int j2 = jj2; j2 < docIds2.size() && docIds2[j2] == docId; ++j2)
      {
        if (checkWordId(wordIds2[jj2]) == false) continue;
        for (unsigned int j1 = jj1; j1 < docIds1.size() && docIds1[j1] == docId; ++j1)
        {
          int posDiff = positions2[j2] - positions1[j1];
          if (checkPosition(posDiff) == false) continue;
          if (j0 == UINT_MAX)
          {
            if (i == 0 || all[i-1].first != docId || all[i-1].second.first != 0)
            {
              ostringstream os;
              os << "
  */
  cout << "OK (only partial check so far)" << endl;
}



//! Check equality of two posting lists (used for testing only)
template<unsigned char MODE>
void CompleterBase<MODE>::checkEqualityOfTwoPostingLists
      (const QueryResult& input1,
       const QueryResult& input2)
{
  //CS_ASSERT(input1.check() == true);
  //CS_ASSERT(input2.check() == true);

  cout << "check equality: " << flush;
  const DocList&      docIds1    = input1._docIds;
  const WordList&     wordIds1   = input1._wordIdsOriginal;
  const PositionList& positions1 = input1._positions;
  const DocList&      docIds2    = input2._docIds;
  const WordList&     wordIds2   = input2._wordIdsOriginal;
  const PositionList& positions2 = input2._positions;

  CS_ASSERT(docIds1.size() == wordIds1.size());
  CS_ASSERT(wordIds1.size() == positions1.size());
  CS_ASSERT(docIds2.size() == wordIds2.size());
  CS_ASSERT(wordIds2.size() == positions2.size());

  // same size?
  if (docIds1.size() != docIds2.size())
  {
    cout << "size differs (" << docIds1.size() << " vs. " << docIds2.size() << ")"  << endl;
    return;
  }

  // same doc ids?
  for (unsigned int i = 0; i < docIds1.size(); ++i)
  {
    if (docIds1[i] != docIds2[i])
    {
      cout << "same size, but doc ids at " << i << " differ ("
           << docIds1[i] << " vs. " << docIds2[i] << ")" << endl;
      return;
    }
  }

  // same word ids?
  for (unsigned int i = 0; i < wordIds1.size(); ++i)
  {
    if (wordIds1[i] != wordIds2[i])
    {
      cout << "same size and same doc ids, but word ids at " << i << " differ ("
           << wordIds1[i] << " vs. " << wordIds2[i] << ")" << endl;
      return;
    }
  }

  // same positions?
  for (unsigned int i = 0; i < positions1.size(); ++i)
  {
    if (positions1[i] != positions2[i])
    {
      cout << "same size, same doc ids, and same word ids, but positions at " << i << " differ ("
           << positions1[i] << " vs. " << positions2[i] << ")" << endl;
      return;
    }
  }

  cout << "OK" << endl;
}

//! Intersect two posting lists OLD; partially templated version
/*
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingListsOld to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsOldTemplated.
 *
 *    NOTE: old, fucked-up signature. Changed for the NEW intersect, but left
 *    it as is for the old one (which is just there for checking anyway).
 *
 *    TODO: this is now superseded by the previous routine (which is in large
 *    parts copied & pasted from this one). Therefore to be deleted soon, as
 *    soon as all calls to this method have been replaced by calls using the new
 *    human-friendly signature
 */
template<unsigned char MODE>
template<class scoreType, class T, class U, class V>
void CompleterBase<MODE>::intersect
      (const Separator&         splitSeparator, 
       const T&                 checkWordIds, 
       const V&                 notIntersectionMode,
       const U&                 locationScoreAggregationMethod,
       const scoreType          scoretype, 
       const QueryResult&       candidateLists, 
       const DocList&           doclist,
       const Vector<Position>&  positionlist, 
       const Vector<scoreType>& scorelist,
       const WordList&          wordlist, 
             QueryResult&       resultLists, 
       const WordRange&         wordRange)
{
  if (!candidateLists.check()) CS_THROW(Exception::BAD_QUERY_RESULT, "bad candidate lists");
  signed char intersectionMode = splitSeparator._separatorIndex;
  assert(notIntersection());
  assert(!andIntersection());
  assert((sizeof(scoreType) == sizeof(DiskScore)) || (sizeof(scoreType) == sizeof(Score)));
  
  if (resultLists.isLockedForReading) 
    throw Exception(Exception::RESULT_LOCKED_FOR_READING, "in intersect");
  if (resultLists.isLockedForWriting) 
    throw Exception(Exception::RESULT_LOCKED_FOR_WRITING, "in intersect");
  resultLists.isLockedForWriting = true;
  if (candidateLists.isLockedForWriting) 
    throw Exception(Exception::RESULT_LOCKED_FOR_WRITING, "in intersect");
  candidateLists.isLockedForReading = true; // only unlockd in clean up      
  switch(intersectionMode)
  {
    case SAME_DOC:
      if (notIntersectionMode()) {
        #ifndef NDEBUG
        log << " intersection choice 1 " << endl;
        #endif
      CompleterBase<MODE>::template intersect<Zero>(SameDoc, checkWordIds, notIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange); }
      else {
        #ifndef NDEBUG
        log << " intersection choice 2 " << endl;
        #endif
                  intersect(SameDoc, checkWordIds, andIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange); }
      break;

    case ADJACENT:
      if (notIntersectionMode()) {            
        log << " intersection choice 3 " << endl;
        log << " NOT SUPPORTED YET " << endl;
        exit(1);
      }
      else {
        #ifndef NDEBUG
        log << " intersection choice 4 " << endl;
        #endif
      intersect(Adjacent, checkWordIds, andIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange);}
      break;
        
    case NEAR:
      if (notIntersectionMode()) {
        log << " intersection choice 5 " << endl;
        log << " NOT SUPPORTED YET " << endl;
        exit(1);}
      else {
        #ifndef NDEBUG
        log << " intersection choice 6 " << endl;
        #endif
      intersect(Near, checkWordIds, andIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange);}
      break;

    case FULL:
      assert (!notIntersectionMode());
        #ifndef NDEBUG
        log << " intersection choice 7 " << endl;
        #endif
      intersect(Full, checkWordIds, andIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange);
      break;

    case PAIRS:
      if (notIntersectionMode()) {
        log << " intersection choice 8 " << endl;
        log << " NOT SUPPORTED YET " << endl;
        exit(1);}
      else {
        #ifndef NDEBUG
        log << " intersection choice 9 " << endl;
        #endif
      intersect(Pairs, checkWordIds, andIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange);}
      break;

    case FLEXI:
      assert(!notIntersectionMode());
        #ifndef NDEBUG
        log << " intersection choice 10 " << endl;
        #endif
      currentIntersectionWindow = splitSeparator._intersectionWindow;
      intersect(Flexi, checkWordIds, andIntersection, locationScoreAggregationMethod, scoretype, candidateLists,
                doclist, positionlist, scorelist, wordlist, resultLists, wordRange);
      break;

    default:
      log << "Received illegal intersection Mode (" << (unsigned int) intersectionMode << ") in call of intersect()." << endl;
      exit(1);

    }// end of switch statement
  //      assert(!errorStatus);
  if (!resultLists.isLockedForWriting) 
    throw Exception(Exception::RESULT_NOT_LOCKED_FOR_WRITING, "in intersect");
  resultLists.isLockedForWriting = false;

  #ifndef NDEBUG
  log << "end of wrapper intersect()" << endl;
  #endif
}




//! Intersect two posting lists OLD; fully templated version   OOO
/*!
 *    This contains the actual old code. Fucked-up signature, see above. 
 */
template<unsigned char MODE>
template<class S, class T, class U, class V, class scoreType>
void CompleterBase<MODE>::intersect
      (const S&                 intersectionMode, 
       const T&                 checkWordIds,
       const V&                 notIntersectionMode,
       const U&                 locationScoreAggregationMethod,
       const scoreType          scoretype, 
       const QueryResult&       input1, 
       const DocList&           input2_docIds,
       const Vector<Position>&  input2_positions, 
       const Vector<scoreType>& input2_scores,
       const WordList&          input2_wordIdsOriginal, 
             QueryResult&       result, 
       const WordRange&         wordRange)
{

  // TABLE OF CONTENTS
  //
  //      TRIVIAL CASE: FIRST LIST IS EMPTY
  //   0. VARIOUS CHECKS
  //   1. MACRO DEFINITIONS
  //   2. CHECK BUFFER SIZES AND RESIZE IF NECESSARY
  //   3. CASE: FIRST LIST = LIST OF ALL DOC IDS
  //   4. CASE: NORMAL FIRST LIST (not the list of all doc ids)
  //   5. SOME POST CHECKS
  
  if (!(input1._status & QueryResult::FINISHED)) 
    CS_THROW(Exception::BAD_HISTORY_ENTRY, "input1 should be finished");
  if (!(result._status == QueryResult::UNDER_CONSTRUCTION)) 
    CS_THROW(Exception:: BAD_HISTORY_ENTRY, "result should be under construction");
  assert(input1._status & QueryResult::FINISHED);
  
  assert((!(MODE & WITH_SCORES)) || (input1._scores.size() == input1._docIds.size()));
  ++nofIntersections;

  // TRIVIAL CASE: FIRST DOCLIST IS EMPTY
  if ((input2_docIds.size() == 0) && notIntersectionMode())
  {
    #ifndef NDEBUG
    log << " ! in intersect: input2_docIds was empty " << endl << flush;
    #endif
    return;
  }

  #ifndef NDEBUG
  if (checkWordIds()) {log << "! have to check word ids in intersect" << endl;}
  #endif

  // 0. VARIOUS CHECKS

  // Check for errors when using tag/xml intersection
  if ((intersectionMode() == PAIRS)&& (!input1._positions.hasEvenSize()))
  {
    throw Exception(Exception::ODD_LIST_LENGTH_FOR_TAG);  
  }
  if ((intersectionMode() == PAIRS)&& (!input1._docIds.checkPairedness()))
  {
    throw Exception(Exception::DOCS_NOT_PAIRED_FOR_TAG);
  }

  // Some checks for invalid combinations of modes
  assert(!notIntersectionMode() || !(intersectionMode() == PAIRS));
  assert(!notIntersectionMode() || !(intersectionMode() == Adjacent()));
  assert(!notIntersectionMode() || !(intersectionMode() == Near()));
  assert(!notIntersectionMode() || !(intersectionMode() == Flexi()));

  // Some checks for input lists
  assert((!(MODE & WITH_SCORES)) || (input1._scores.size() == input1._docIds.size()));
  assert( !(intersectionMode() == PAIRS) || (input1._positions.hasEvenSize() ));
  assert( !(intersectionMode() == PAIRS) || (input1._docIds.checkPairedness()));
  assert( !(intersectionMode() == PAIRS) || (MODE & WITH_POS));
  assert(input2_docIds.size() == input2_wordIdsOriginal.getNofElements());
  assert( input1._docIds.isSorted() );
  assert(((input2_docIds.size() == 0) && notIntersectionMode()) || (input2_docIds.isSorted(!(MODE & WITH_DUPS))));
  assert(notIntersectionMode() || checkWordlistBuffer(wordRange));     
  assert(result._docIds.size() == result._bufferPosition);
  assert(result._docIds.size() == result._wordIdsOriginal.size());
  assert((!(MODE & WITH_SCORES)) || (result._docIds.size() == result._scores.size()));
  assert((!(MODE & WITH_POS )) || (result._docIds.size() == result._positions.size()));
  assert((!(MODE & WITH_POS )) || (input2_docIds.size() == input2_positions.size()));
  assert((!(MODE & WITH_SCORES )) || (input2_docIds.size() == input2_scores.size()));
  assert((MODE & WITH_POS ) || (0 == input2_positions.size()));
  assert((MODE & WITH_SCORES ) || (0 == input2_scores.size()));
  ////      assert((input1._docIds.size() == 0)||(input1._docIds.back() != INFTY_DOCID));
  ////      assert(input2_docIds.back() != INFTY_DOCID);
  assert(input2_docIds.size() == input2_wordIdsOriginal.getNofElements());
  assert((input1._docIds.size()>0)||(input1._docIds.isFullList()));
  assert(!input2_docIds.isFullList());
  assert(!input2_wordIdsOriginal.isFullList());
  assert(result._docIds.size() == result._docIds.size());
  assert(result._wordIdsOriginal.size() == result._wordIdsOriginal.getNofElements());
  assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
  register const WordId wordRangeLow = wordRange.firstElement();
  register const WordId wordRangeHigh = wordRange.lastElement();
  assert(!checkWordIds() || (wordRangeLow >= 0));
  ////      assert((input1._docIds.size() == input1._wordIdsOriginal.size())||( (input1._docIds.size() == input1._wordIdsOriginal.size() +1 ) && (input1._docIds.back() == INFTY_DOCID)));
  assert(result._docIds.size() == result._wordIdsOriginal.size());

  #ifndef NDEBUG
  if (intersectionMode() == Flexi())
  {
    log << "! intersection routine called with Flexi mode! " << endl;
    log << "! intersection window : [" << currentIntersectionWindow.first << "," << currentIntersectionWindow.second << "]" << endl;
  }
  #endif

  // Check that for both lists to be intersected the positions are sorted within a document
  #ifndef NDEBUG
  if(MODE & WITH_POS)
  {
    for(unsigned long j = 1; !input1._docIds.isFullList() 
                               && j < input1._docIds.size(); j++)
    {
      assert(input1._docIds[j] >= input1._docIds[j-1]);
      assert(input1._docIds[j] > input1._docIds[j-1]
              || input1._positions[j] >= input1._positions[j-1]);
    }          
    for(unsigned long j = 1; j < input2_docIds.size(); j++)
    {
      assert(input2_docIds[j] >= input2_docIds[j-1]);
      assert(input2_docIds[j] > input2_docIds[j-1] || input2_positions[j] >= input2_positions[j-1]);
    }
  }
  #endif


  // 1. MACRO DEFINITIONS
  //
  //   The following macros implement various conditions on when to write a
  //   posting to the result list. They are implemented as macros because they
  //   are called more than once. Alternatively, they could as well have been
  //   implemented as inline functions.

  // Check relative position of two postings
  //
  //   Evaluate to true iff the given difference x between two positions is
  //   within the range given by intersectionMode (which, in turn, was
  //   derived from the respective separator). For example, if the separator
  //   was a space, the intersectionMode is SameDoc, and the macro will
  //   evaluate to true for any given difference x 
  //
  #define IS_IN_WINDOW(x) (    ((intersectionMode() == SameDoc())  \
                            || ((intersectionMode() == Adjacent()) \
                                  && (( x >= fixed_separators._separators[Adjacent()]._intersectionWindow.first )    \
                                  && (  x <= fixed_separators._separators[Adjacent()]._intersectionWindow.second ))) \
                            || ((intersectionMode() == Near()) \
                                  && ((x != 0) \
                                  && ( x >= fixed_separators._separators[Near()]._intersectionWindow.first )      \
                                  && ( x <= fixed_separators._separators[Near()]._intersectionWindow.second ) ))  \
                            || ((intersectionMode() == Flexi()) \
                                  && (x >= currentIntersectionWindow.first)      \
                                  && (x <= currentIntersectionWindow.second) ) ) \
                             ? true : false)


  // Check relative position when first list represents tag pairs
  //
  //   Evaluates to true iff the position of posting j of the second input
  //   list is between the positions of postings i and i+1, respectively, in
  //   the first input list. Used when the first list represents XML-style
  //   tag pairs.
  //
  #define IS_BETWEEN_PAIRS(i,j) (input1._positions[i] < input2_positions[j]       \
                                  && input1._positions[i+1] > input2_positions[j] \
                                  ? true : false)

  // Check word range
  //
  //   Evaluates to true iff the word id of posting j of the second input
  //   list is within wordRange (one of the arguments to this method)
  //
  #define WORD_IS_IN_RANGE(j) (input2_wordIdsOriginal[j] <= wordRangeHigh && input2_wordIdsOriginal[j] >= wordRangeLow)

  // Write match(es) to result list
  //
  //   This macro evaluates to code to be executed when the two input lists
  //   match at postings i and j. Writes posting to result buffer at
  //   position _bufferPosition (TODO: bad name) and advances that position
  //   by one. 
  //
  //   Note: The posting i of the first input list is used solely for the
  //   purpose of scoring (checking of positions etc. is done outside of
  //   this macro).
  //
  //   Note: the marco is actually pretty simple, it stretches over so many
  //   line because of the many assert and because it also work without
  //   score lists and/ort without position lists. For the default case with
  //   positions and scores, the main code executed is the call to aggregate
  //   in the body of the "if (MODE & WITH_POS) ..." clause
  //
  //   Note: the positions in the two input lists are not advanced by this
  //   macro, this is the task of the caller of this macro; see the code in
  //   this method further down.
  //
  //   Old comments by Ingmar which I don't understand: no duplicates are written; 
  //
  #define MATCH_FOUND(j,i)\
  {\
    assert(j >= 0); \
    assert(j < input2_docIds.size()); \
    assert((!(MODE & WITH_POS)) || (j < input2_positions.size())); \
    if (((signed long) j > lastMatchJ) && ((!(MODE & WITH_POS)) || (result._bufferPosition == 0) ||\
       (result._docIds[result._bufferPosition-1] != input2_docIds[j] ) ||\
       (result._positions[result._bufferPosition-1] !=  input2_positions[j] ) ||\
       (result._wordIdsOriginal[result._bufferPosition-1] != input2_wordIdsOriginal[j])))\
    {\
      assert(result._bufferPosition < result._docIds.size());\
      assert(result._bufferPosition < result._wordIdsOriginal.size());\
      assert(input2_docIds[j] != INFTY_DOCID);\
      result._docIds[result._bufferPosition] = input2_docIds[j]; \
      result._wordIdsOriginal[result._bufferPosition] = input2_wordIdsOriginal[j]; \
      if (MODE & WITH_SCORES) \
      {\
        if (intersectionMode() != Full() )\
        {\
          assert(i>=0); \
          assert((unsigned int) i<input1._scores.size()); \
          assert((unsigned int) j < input2_scores.size()); \
          if (MODE & WITH_POS) \
          {\
            result._scores[result._bufferPosition] = \
              locationScoreAggregationMethod.aggregate(input1._scores[i], \
                                                       input2_scores[j], \
                                                       input1._positions[i]-input2_positions[j], \
                                                       input2_wordIdsOriginal[j] == wordRangeLow); \
          }\
          else \
          {\
            result._scores[result._bufferPosition] = \
              locationScoreAggregationMethod.aggregate(input1._scores[i], \
                                                       input2_scores[j], \
                                                       input2_wordIdsOriginal[j] == wordRangeLow); \
          } \
        } \
        else \
        {\
          result._scores[result._bufferPosition] = input2_scores[j] + HAS_EXACT_MATCH_BONUS(input2_wordIdsOriginal[j] == wordRangeLow); \
        }\
       assert(j<input2_scores.size()); \
       assert(input2_scores[j] > 0); \
      } \
      if(MODE & WITH_POS) \
      {\
        result._positions[result._bufferPosition] = input2_positions[j]; \
      }\
      ++result._bufferPosition; \
      lastMatchJ = j; \
    }\
    assert(result._bufferPosition <= result._docIds.size());\
  } 
  // end of big MATCH_FOUND(.,.) macro


  // Check non-match for Ingmar's "not" operator
  //
  //   Evaluates to true iff current lastDocWithMatch (TODO: exact
  //   definition; shouldn't it be lastDocIdWithMatch?) is different from
  //   the doc id of posting i of the first input list
  //
  #define IS_NOT_MATCH(i,j) (input1._docIds[i] != lastDocWithMatch)
  // Old comments from Ingmar, which I (Holger) don't understand:
  //
  // INTRODUCE WORD RANGE HERE AS WELL
  // is lastMatchingPosition from the candidateList or from the input2_positions?
  // really is from candidate lsits
  // have some bool variable to tell, if the last doc/position really was a match
  // can still be a "not" match if there was an overlapping document but the wordrange didn't fit
  // what about position not?
  // #define IS_NOT_MATCH(ii,jj) ((input1._docIds[ii] != lastMatchingDocId) || (input1._positions[ii] != lastMatchingPosition) || (checkWordIds() && (!WORD_IS_IN_RANGE(jj) )))

  // MACRO USED INSTEAD OF 'MATCH_FOUND' FOR CASE OF 'not' operator
  // jj is NOT used for scoring
  #define NEGATIVE_MATCH_FOUND(ii)\
  {\
    assert(result._bufferPosition < result._docIds.size());\
    assert(result._docIds.size() == result._wordIdsOriginal.size());\
    assert(ii >= 0);\
    assert(ii < input1._docIds.size());\
    assert((!(MODE & WITH_POS)) || (ii < input1._positions.size()));\
    assert((result._bufferPosition == 0) || (result._docIds.size() > (result._bufferPosition-1)));\
    assert(result._docIds.size() == result._wordIdsOriginal.size());\
    assert(result._bufferPosition < result._docIds.size());\
    assert(ii < input1._docIds.size());\
    if ((!(MODE & WITH_POS)) || (result._bufferPosition == 0) ||\
           (result._docIds[result._bufferPosition-1] != input1._docIds[ii] ) ||\
           (result._positions[result._bufferPosition-1] !=  input1._positions[ii] ) ||\
           (result._wordIdsOriginal[result._bufferPosition-1] != input1._wordIdsOriginal[ii]));\
    {\
      result._docIds[result._bufferPosition] = input1._docIds[ii];\
      result._wordIdsOriginal[result._bufferPosition] = input1._wordIdsOriginal[ii];\
      if (MODE & WITH_SCORES)\
         {\
            assert( intersectionMode() != Full() );\
            assert(ii>=0);\
            assert((unsigned int) ii<input1._scores.size());\
            result._scores[result._bufferPosition] = input1._scores[ii];\
            if(MODE & WITH_POS)\
               {\
                 result._positions[result._bufferPosition] = input1._positions[ii];\
               }\
                 ++result._bufferPosition;\
        }\
    }\
  }
  


  assert(result._docIds.size() == result._wordIdsOriginal.size());

  // 2. CHECK BUFFER SIZES AND RESIZE IF NECESSARY
  //
  //    TODO INGMAR: For not intersection the memory is different!!! Only depends on the input list
  //
  resizeAndReserveTimer.cont();

  // 2.1 Normal intersection mode
  if ((!notIntersectionMode()) && (result._docIds.capacity() 
                                    < result._docIds.size() + input2_docIds.size()))
  {
    log << "! " /* MSG_BEG */ << "resizing intersection buffer (capacity is smaller than " 
        << result._docIds.size() + input2_docIds.size() 
        <<  " elements) ... " << flush;
    assert(result._wordIdsOriginal.capacity() 
            < (result._wordIdsOriginal.size() + input2_wordIdsOriginal.getNofElements()) );
    // resize + init input2_docIds buffer
    {
      result._docIds.reserve((unsigned long) ceil(1.0*( result._docIds.size() + 2*input2_docIds.size() )));
    }
    // resize + init input2_wordIdsOriginal buffer
    {
      result._wordIdsOriginal.reserve((unsigned long) ceil(1.0*( result._wordIdsOriginal.size() + 2*input2_wordIdsOriginal.getNofElements() )));
    }
    // resize + init score buffer
    if (MODE & WITH_SCORES) 
    {
      result._scores.reserve((unsigned long) ceil(1.0*( result._scores.size() + 2*input2_scores.size() )));
    }
    // resize + init positions buffer
    if (MODE & WITH_POS) 
    {
      result._positions.reserve((unsigned long) ceil(1.0*( result._positions.size() + 2*input2_positions.size() )));
    }
    log << "done (now " << commaStr(result._docIds.capacity()) << " for docs and " 
         << commaStr(result._wordIdsOriginal.capacity()) << " for words)" << flush << endl /* MSG_END */;

  }

  // 2.2 Ingmar's "not" intersection mode
  else if ((notIntersectionMode()) && (result._docIds.capacity() < (input1._docIds.size() )))
  {
    assert(!input1._docIds.isFullList());
    log << MSG_BEG << "resizing intersection buffer (capacity is smaller than " << (input1._docIds.size() )  << " elements) ... " << flush;
    assert(result._wordIdsOriginal.capacity() < (input1._wordIdsOriginal.getNofElements()) );
    // resize + init input2_docIds buffer
    {
      result._docIds.reserve((unsigned long) input1._docIds.size() );
      assert(result._docIds.capacity() > 0);
    }
    // resize + init input2_wordIdsOriginal buffer
    {
      result._wordIdsOriginal.reserve((unsigned long) input1._wordIdsOriginal.getNofElements() );
    }
    // resize + init score buffer
    if (MODE & WITH_SCORES) 
    {
      result._scores.reserve((unsigned long) input1._scores.size() );
    }
    // resize + init positions buffer
    if (MODE & WITH_POS) 
    {
      result._positions.reserve((unsigned long) input1._positions.size() );
    }
    log << "done (now " << commaStr(result._docIds.capacity()) << " for docs and " 
         << commaStr(result._wordIdsOriginal.capacity()) << " for words)" << flush << MSG_END;
  }
  resizeAndReserveTimer.stop();
  assert(result._docIds.size() == result._wordIdsOriginal.size());\

  #ifndef NDEBUG
  log << " done resizing " << endl;
  #endif

  const register unsigned long sizeDoclist = input2_docIds.size();
  const register unsigned long sizeCandidateDoclist = input1._docIds.size();


  // 3. CASE: FIRST LIST = LIST OF ALL DOC IDS
  //
  //   Then elements of second list are simply appended to result
  //   However, word ids are still range-checked (if check specified)
  
  if((input1._docIds.isFullList()) || ((input2_docIds.size() == 0) && (notIntersectionMode() ) && (!input1._docIds.isFullList())))
  {
    assert(result._docIds.size() == result._wordIdsOriginal.size());\
                                                           ////          assert((input1._docIds.size() == input1._wordIdsOriginal.size())||( (input1._docIds.size() == input1._wordIdsOriginal.size() +1 ) && (input1._docIds.back() == INFTY_DOCID)));
                                                           assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
    #ifndef NDEBUG
    log << " trivial intersection " << endl;
    log << " is full list ?" << input1._docIds.isFullList() << endl;
    #endif 

    // 3.1 Case: first list = full list + the word ids need *not* be checked.
    //
    //   then append all postings from the second to the result list 
    //
    if ((!checkWordIds()) || ((input2_docIds.size() == 0) && (notIntersectionMode() )))
    {
      appendTimer.cont();
      result._docIds.append(input2_docIds); // to which timer should this contribute?
      result._wordIdsOriginal.append(input2_wordIdsOriginal);
      assert((!(MODE & WITH_SCORES)) || input2_scores.isPositive());
      assert((!(MODE & WITH_SCORES)) || result._scores.isPositive());
      if(MODE & WITH_SCORES) {result._scores.append(input2_scores);}
      assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
      if(MODE & WITH_POS) {result._positions.append(input2_positions);}
      result._bufferPosition = result._docIds.size();
      appendTimer.stop();
      assert(result._docIds.size() == result._wordIdsOriginal.size());
      assert(result._docIds.size() > 0);
    }// end case: candidate input2_docIds is full list and no filtering (of words) needs to be done

    // 3.2 Case: first list = full list + the wors ids *do* need to be checked 
    //
    //   then append only those postings with word id in the given range
    //
    else
    {
      // crucial!! call resize here (to something large enough); TODO: why crucial?
      resizeAndReserveTimer.cont();
      if (!notIntersectionMode())
      {
        result._docIds.resize( result._docIds.size() + input2_docIds.size() );
        result._wordIdsOriginal.resize( result._wordIdsOriginal.size() + input2_wordIdsOriginal.getNofElements() );
        if(MODE & WITH_SCORES) {result._scores.resize(result._scores.size() + input2_scores.size(),1 );}
        if(MODE & WITH_POS) {result._positions.resize(result._positions.size() + input2_positions.size() );}
      } 
      else
      {
        result._docIds.resize( result._docIds.size() + input1._docIds.size() );
        result._wordIdsOriginal.resize( result._wordIdsOriginal.size() + input1._wordIdsOriginal.getNofElements() );
        if(MODE & WITH_SCORES) {result._scores.resize(result._scores.size() + input1._scores.size(),1 );}
        if(MODE & WITH_POS) {result._positions.resize(result._positions.size() + input1._positions.size() );}
      }
      resizeAndReserveTimer.stop();
      appendTimer.cont();
      //intersectionTimer.cont();
      register unsigned long j=0;
      register signed long lastMatchJ = -1;
      const signed char i = -1; // not used but needed to compile with MATCH_FOUND macro              
      /* no "intersection" but only checking of word ids needs to be done */
      while(j<sizeDoclist)
      {
        assert(input2_wordIdsOriginal[j] >= 0);     
        if((!checkWordIds())||(WORD_IS_IN_RANGE(j)))
        {
          assert(j<input2_wordIdsOriginal.size());
          assert(result._bufferPosition < result._docIds.size());
          assert(!notIntersectionMode()); // should never get here with "not" intersection
          if(!notIntersectionMode()) { MATCH_FOUND(j,i); }
        }
        j++;
      }
      appendTimer.stop();
      //              intersectionTimer.stop();
      resizeAndReserveTimer.cont();
      result._docIds.resize( result._bufferPosition);
      result._wordIdsOriginal.resize( result._bufferPosition);
      if(MODE & WITH_SCORES) {result._scores.resize( result._bufferPosition);}
      if(MODE & WITH_POS) {result._positions.resize( result._bufferPosition);}
      resizeAndReserveTimer.stop();
    }

    // Note: intersectedVolume already takes care of by append above
    assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
    assert(result._docIds.size() == result._wordIdsOriginal.size());
  } 


  // 4. CASE: NORMAL FIRST LIST (not the list of all doc ids)
  //
  //   Do a standard (zipper-like) list intersection in this case 
  //   Range-check word ids as specified

  else
  {
    assert(input1.check());
    assert(input1._status & QueryResult::IN_USE);
    assert(result._docIds.size() == result._wordIdsOriginal.size());
    ////          assert((input1._docIds.size() == input1._wordIdsOriginal.size())||( (input1._docIds.size() == input1._wordIdsOriginal.size() +1 ) && (input1._docIds.back() == INFTY_DOCID)));
    #ifndef NDEBUG
    log << " non-trivial intersection " << endl;
    #endif 

    intersectionTimer.cont();
    assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
    const unsigned long positionLength = input2_positions.size();
    assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
    // crucial!! call resize here (to something large enough) // THIS DOES THE MEMSET
    assert(result._docIds.size() == result._wordIdsOriginal.size());
    result._docIds.resize( result._docIds.size() + input2_docIds.size());
    result._wordIdsOriginal.resize( result._wordIdsOriginal.size() + input2_wordIdsOriginal.getNofElements());
    assert(result._docIds.size() == result._wordIdsOriginal.size());
    //fill input2_scores intersection buffer xwith 1's to ensure positive elements (for error checking only)
    if (MODE & WITH_SCORES) {result._scores.resize(result._scores.size() + input2_scores.size(),1);}
    if (MODE & WITH_POS) {result._positions.resize(result._positions.size() + input2_positions.size());}

    // current index for first posting list (input1)
    register unsigned long i          = 0;
    // current index for second posting list (input2)
    register unsigned long j          = 0;
    // TODO: what's this?
    register   signed long lastMatchJ = -1;

    // A "lastMatchingDocId" is needed to cope with lists with duplicates
    DocId lastMatchingDocId = INFTY_DOCID;
    // needed for NOT intersection
    DocId lastDocWithMatch = INFTY_DOCID;
    // NEW 11Oct13 (baumgari): GCC complains, that lastMatchingPosition is an
    // unused variable. Uncommented it.
    // A "lastMatchingPosition" is needed for 'not' to cope with lists with duplicates
    // This position is the last matching position in the candidate list which caused a 'match'
    // Position lastMatchingPosition = (Position) -1; // new 13Nov06
    // This will be the last position in the candidate list
    // only needs to be set if docs matched.
    // update when i changes
    Position lastCandidatePosition = (Position) -1; // initial value irrelvant
    // update when j changes
    // ... can't think of a better name :-(
    Position lastCurrentPosition = (Position) -1; // initial value irrelvant

    assert(result._docIds.size() == result._wordIdsOriginal.size());

    //
    // MAIN LOOP (over all document ids/positions)
    //
    while (true)
    { 
      // Check sortedness of input2
      assert( j == 0 
               || j == sizeDoclist 
               || input2_docIds[j] > input2_docIds[j-1] 
               || (!(MODE & WITH_POS)) 
               || input2_positions[j] >= input2_positions[j-1]);

      // Case 1: went "to far" in input2 
      //
      //   TODO: explain "to far" (Ingmar's wording). My current understanding
      //   is that in this case, j points one *after* the current position,
      //   which is somewhat unintuitive.
      //
      if (i != sizeCandidateDoclist 
            && ((j == sizeDoclist && i < sizeCandidateDoclist) 
                   || (input1._docIds[i] < input2_docIds[j])))
      {
        // Comment for "not" intersection: If
        // input1._docIds[i] != lastMatchingDocId, then we
        // have a "not" match for i.  Add a 'not' check before i is incremented

        // Case: repeated doc id in input1
        if ((MODE & WITH_DUPS) && (input1._docIds[i] == lastMatchingDocId))
        {
          assert(j > 0);
          // Case: without positions
          if (!(MODE & WITH_POS))
          {
            if((!checkWordIds())||(WORD_IS_IN_RANGE(j-1)))
            {
              lastDocWithMatch = input1._docIds[i];
              if(!notIntersectionMode()) { MATCH_FOUND(j-1,i); }
            }
          }
          // Case: with positions
          else
          {
            // Note: here it could happen that a match is reported AGAIN! ... take care of this later
            assert(i < input1._positions.size());
            assert(j > 0);
            if (((intersectionMode() != PAIRS)) && (IS_IN_WINDOW((signed int) lastCurrentPosition - (signed int) input1._positions[i])))//check Positions
            {
              if((!checkWordIds())||(WORD_IS_IN_RANGE(j-1)))
              {
                lastDocWithMatch = input1._docIds[i];
                if(!notIntersectionMode()) { MATCH_FOUND(j-1,i); }
              }                           
            } // passed position check
            assert( !(intersectionMode() == PAIRS) || (i+1 <= input1._positions.size()));
            assert( !(intersectionMode() == PAIRS) || (i+1 <= input1._docIds.size()));
            if ((intersectionMode() == PAIRS) && (IS_BETWEEN_PAIRS(i,j-1)) && ((!checkWordIds())||(WORD_IS_IN_RANGE(j-1))) )
            {
              assert(input1._docIds[i] ==  input1._docIds[i+1]);
              if(!notIntersectionMode()) { MATCH_FOUND(j-1,i); }
            }
          }
        }

        // TODO: what is happening here?
        if (MODE & WITH_POS)
        {
          assert(i<input1._positions.size());
          lastCandidatePosition = input1._positions[i];
        }
        if (notIntersectionMode() && IS_NOT_MATCH(i,j))
        {
          assert(result._bufferPosition < result._docIds.size());
          NEGATIVE_MATCH_FOUND(i);
        }
        ++i;
        if (intersectionMode() == PAIRS) {if (notIntersectionMode() && IS_NOT_MATCH(i,j)) { NEGATIVE_MATCH_FOUND(i); } ++i; }
      }


      // Case 2: went "to far" in input1 
      //
      //   TODO: Again, explain "to far"
      //
      else if ( j != sizeDoclist 
                  && (((sizeCandidateDoclist==i)&&(sizeDoclist>j)) 
                        || (input1._docIds[i]>input2_docIds[j])))
      {
        // Case: repeated doc id in input2; TODO: why not check MODE & WITH_DUPS here?
        if (input2_docIds[j] == lastMatchingDocId)
        {
          // Case: without positions
          if (!(MODE & WITH_POS))
          {
            if ((!checkWordIds())||(WORD_IS_IN_RANGE(j)))
            {
              if (!notIntersectionMode()) { MATCH_FOUND(j,i-1); }
            }
          }
          // Case: with positions
          else
          {
            assert(j<input2_positions.size());
            if (((intersectionMode() != PAIRS)) && (IS_IN_WINDOW((signed int) input2_positions[j] - (signed int) lastCandidatePosition)))//check Positions
            {
              if((!checkWordIds())||(WORD_IS_IN_RANGE(j)))
              {
                if(!notIntersectionMode()) { MATCH_FOUND(j,i-1); }
              }                           
            } 
          }
          assert( !(intersectionMode() == PAIRS) || (i-1 <= input1._positions.size()));
          assert( !(intersectionMode() == PAIRS) || (i-1 <= input1._docIds.size()));
          assert(i-2 >= 0);
          if ((intersectionMode() == PAIRS) && (IS_BETWEEN_PAIRS(i-2,j)) && ((!checkWordIds())||(WORD_IS_IN_RANGE(j)))  )
          {
            assert(input1._docIds[i-2] ==  input1._docIds[i-1]);
            if(!notIntersectionMode()) { MATCH_FOUND(j,i-2); }
          }
        }

        // Advance by one in input2
        if (MODE & WITH_POS)
        {
          assert(j<input2_positions.size());
          lastCurrentPosition = input2_positions[j];
        }
        ++j;
      }


      // Case: doc ids at input1[i] and input2[j] match
      // 
      else
      {
        assert(((i==sizeCandidateDoclist)&& (j==sizeDoclist)) || (input1._docIds[i] == input2_docIds[j]));
        assert(i <= input1._docIds.size());
        // Case: not yet at end of input1 (and hence also not at end of input2?)
        if(i < input1._docIds.size())
        {
          // Note: doc id needs to be counted as match, even if word id is outside range; TODO: why?
          
          // Case: without positions
          if (!(MODE & WITH_POS))
          {
            lastMatchingDocId = input1._docIds[i];
            if((!checkWordIds())||(WORD_IS_IN_RANGE(j)))
            {
              lastDocWithMatch = input1._docIds[i];
              if(!notIntersectionMode()) { MATCH_FOUND(j,i); }
            }
            if (notIntersectionMode() && IS_NOT_MATCH(i,j)) { NEGATIVE_MATCH_FOUND(i); }
            if (!notIntersectionMode()) {++i;}
            ++j;
          }

          // Case: with positions
          else
          {
            // TODO: BETTER TO MOVE THESE CHECKS BELOW 
            if (lastMatchingDocId == input1._docIds[i])
            {
              assert(i<input1._positions.size());
              assert((!(MODE & WITH_POS)) || (j<input2_positions.size()));
              if((j>0) && (input1._positions[i] < input2_positions[j])&&(input2_docIds[j-1] == lastMatchingDocId))
              {
                // check if last j-1 position is ok for current i
                if( (intersectionMode() != PAIRS) && (IS_IN_WINDOW((signed int) input2_positions[j-1] - (signed int) input1._positions[i])))//check Positions
                {
                  if((!checkWordIds())||(WORD_IS_IN_RANGE(j-1)))
                  {
                    assert(result._bufferPosition <= result._docIds.size()); // TODO (after sigir submission)
                    if(!notIntersectionMode()) { MATCH_FOUND(j-1,i); }
                  }                               
                } // passed position check
                assert( !(intersectionMode() == PAIRS) || (i+1 < input1._positions.size()));
                assert( !(intersectionMode() == PAIRS) || (i+1 < input1._docIds.size()));
                if ((intersectionMode() == PAIRS) && (IS_BETWEEN_PAIRS(i,j-1)) && ((!checkWordIds())||(WORD_IS_IN_RANGE(j-1))) )
                {
                  assert(input1._docIds[i] ==  input1._docIds[i+1]);
                  assert(result._bufferPosition <= result._docIds.size()); // TODO (after sigir submission)
                  if(!notIntersectionMode()) { MATCH_FOUND(j-1,i); }
                }
              }
              if((i>0) && (input1._positions[i] >  input2_positions[j])&&(input1._docIds[i-1]==lastMatchingDocId ))
              {
                assert(i>0);
                // check if last i-1 position is ok for current j
                if (((intersectionMode() != PAIRS)) && (IS_IN_WINDOW((signed int) input2_positions[j] - (signed int) input1._positions[i-1])))//check Positions
                {
                  if((!checkWordIds())||(WORD_IS_IN_RANGE(j)))
                  {
                    assert(result._bufferPosition <= result._docIds.size()); // TODO (after sigir submission)
                    if(!notIntersectionMode()) { MATCH_FOUND(j,i-1); }
                  }                               
                } // passed position check
                assert( !(intersectionMode() == PAIRS) || (i-1 < input1._positions.size()));
                assert( !(intersectionMode() == PAIRS) || (i-1 < input1._docIds.size()));
                assert(i-2 >= 0);
                if ((intersectionMode() == PAIRS) && (IS_BETWEEN_PAIRS(i-2,j)) && ((!checkWordIds())||(WORD_IS_IN_RANGE(j))))
                {
                  assert(input1._docIds[i-2] ==  input1._docIds[i-1]);
                  assert(result._bufferPosition <= result._docIds.size()); // TODO (after sigir submission)
                  if(!notIntersectionMode()) { MATCH_FOUND(j,i-2); }
                }
              }                       
            } // end case: lastMatchingDocId == input1._docIds[i

            lastMatchingDocId = input1._docIds[i];
            // lastMatchingPosition = input1._positions[i]; // new 13Nov06
            assert(i<input1._positions.size());
            assert((!(MODE & WITH_POS )) || (j<input2_positions.size()));
            if (((intersectionMode() != PAIRS)) && (IS_IN_WINDOW((signed int) input2_positions[j] - (signed int) input1._positions[i])))//check Positions
            {
              if((!checkWordIds())||(WORD_IS_IN_RANGE(j)))
              {
                lastDocWithMatch = input1._docIds[i];
                if(!notIntersectionMode()) { MATCH_FOUND(j,i); }
              }                               
            } // passed position check

            assert( !(intersectionMode() == PAIRS) || (i+1 < input1._positions.size()));
            assert( !(intersectionMode() == PAIRS) || (i+1 < input1._docIds.size()));
            if ((intersectionMode() == PAIRS) && (IS_BETWEEN_PAIRS(i,j)) && ((!checkWordIds())||(WORD_IS_IN_RANGE(j))))
            {
              assert(input1._docIds[i] ==  input1._docIds[i+1]);
              if(!notIntersectionMode()) { MATCH_FOUND(j,i); }
            }

            if( input1._positions[i] < input2_positions[j])
            {
              // for "normal" intersections have to move i now
              // but for "not" intersection we have to postpone this move as much as possible
              if(!notIntersectionMode())
              {
                lastCandidatePosition = input1._positions[i];
                if (notIntersectionMode() && IS_NOT_MATCH(i,j)) { NEGATIVE_MATCH_FOUND(i); }
                ++i;
                if (intersectionMode() == PAIRS) { if (notIntersectionMode() && IS_NOT_MATCH(i,j)) { NEGATIVE_MATCH_FOUND(i); } ++i;}
              }
              else
              {
                // case: not intersection
                assert(intersectionMode() != PAIRS);
                ++j;
              }
            }
            else if( input1._positions[i] >  input2_positions[j])
            {
              // check if last i-1 position is ok for current j
              lastCurrentPosition = input2_positions[j];
              ++j;
            }
            else
            {
              assert(input1._positions[i]  ==  input2_positions[j]);
              // only increment one of the two, and NOT the candidateposition
              // remember: the candidatePostions could be a subset of the input2_positions
              // so saem positions can occur
              if(j<positionLength)
              {
                lastCurrentPosition = input2_positions[j];
                ++j;
              }
              else
              {
                lastCandidatePosition = input1._positions[i];
                if (notIntersectionMode() && IS_NOT_MATCH(i,j)) { NEGATIVE_MATCH_FOUND(i); }
                ++i; // was one line higher. Why?
                if (intersectionMode() == PAIRS) { if (notIntersectionMode() && IS_NOT_MATCH(i,j)) { NEGATIVE_MATCH_FOUND(i); } ++i;}
              }
            }
          }//end case: do need to check positions               
        }//end case:match and end of lists has NOT been reached
        else
        {//begin case: match and end of both lists has been reached
          lastMatchingDocId = INFTY_DOCID;
          lastDocWithMatch = INFTY_DOCID;
          // lastMatchingPosition = (Position) -1;
          break;
        }//end case: match and end of both lists has been reached
      }// end case: doc ids match
      assert(result._docIds.size() == result._wordIdsOriginal.size());
    }// end: loop over all doc ids in both lists

    //
    // 'CLEAN UP' AFTER MAIN INTERSECTION WORK
    //
    assert(result._docIds.size() == result._wordIdsOriginal.size());

    // RESIZE ALL FOUR BUFFERS
    //
    result._docIds.resize( result._bufferPosition); // 1. resize input2_docIds buffer
    result._wordIdsOriginal.resize( result._bufferPosition);// 2. resize input2_wordIdsOriginal buffer
    if(MODE & WITH_SCORES)
    {
      assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
      assert(result._bufferPosition <=   result._scores.size());
      result._scores.resize(result._bufferPosition);// 3. resize input2_scores buffer
    }
    if(MODE & WITH_POS)
    {
      result._positions.resize(result._bufferPosition);// 4. resize input2_positions buffer
    }
    // INCREMENT COUNTERS FOR INTERSECTED VOLUME
    if(!(MODE & WITH_POS)) {intersectedVolume += (sizeof(DocId)*(i+j));}
    else {intersectedVolume += ((sizeof(DocId)+sizeof(Position))*(i+j));}
    assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
    assert(i == sizeCandidateDoclist);
    assert(j == sizeDoclist);

    // REMOVE THE LAST (ARTIFICIAL) DOC ID (= SENTINEL) FROM END OF LISTS
    assert((input1._docIds.size()==0)||(input1._docIds.back() != INFTY_DOCID));
    assert((input2_docIds.size() == 0) || (input2_docIds.back() != INFTY_DOCID));
    intersectionTimer.stop();
  }
  // END CASE: NORMAL FIRST LIST (not list of all doc ids)

  
  // 5. SOME POST CHECKS
  assert(notIntersectionMode() || checkWordlistBuffer(wordRange));
  assert(result._docIds.size() == result._bufferPosition);
  assert(result._docIds.size() == result._wordIdsOriginal.size());
  assert((input1._docIds.size()==0)||(input1._docIds.back() != INFTY_DOCID));
  assert((input2_docIds.size() == 0) || (input2_docIds.back() != INFTY_DOCID));
  assert((result._docIds.size() == 0) || (result._docIds.back() != INFTY_DOCID));
  assert((!(MODE & WITH_SCORES))||(result._scores.isPositive()));
  assert(notIntersectionMode() || checkWordlistBuffer(wordRange));
  #ifndef NDEBUG
  log << "! end intersect()" << endl;
  #endif
  assert( (!(MODE & WITH_POS)) || ( input1._docIds.size() == input1._positions.size()));
  assert( (!(MODE & WITH_POS)) || ( result._docIds.size() == result._positions.size()));
  assert( (!(MODE & WITH_POS)) || ( input2_positions.size() == input2_docIds.size()));

} 



//! Intersect two posting lists, nicer interface; for use in experiments etc.
/*!
 *    Note: the two methods above have a hard-to-understand signature. This
 *    method has the signature one would expect: 
 *
 *    \param inputList1  the first input list
 *    \param inputList2  the secon input list
 *    \param resultList  the result list
 *    \param separator   the separator that defines what counts as an intersection
 */
/*
template <unsigned char MODE>
void CompleterBase<MODE>::intersect(const QueryResult& inputList1,
                                    const QueryResult& inputList2,
                                          QueryResult& resultList,
                                    const Separator    separator)
{
  // Explanation for the non-obvious arguments:
  //
  //   doNotCheckWordIds  : in HYB the given list may be a superset of the
  //                        list one actual wants to consider. The latter is
  //                        specified by a range of word ids, given as the
  //                        last argument to the following function call. If
  //                        the calling function knows that all ids are
  //                        within that word range it can specify
  //                        doNotCheckWordIds which makes the intersect
  //                        (slightly) more efficient
  //
  //   andIntersection    : gives the typical intersect behaviour, where an
  //                        item is included in the result if it is included
  //                        in both the input lists. When specifying
  //                        notIntersection, the output is just the
  //                        complement of this. In a query this is specified
  //                        as "queryword1 -queryword2".
  //
  //   sumProxAggregation : specified how the scores from two items from the
  //                        input lists should be aggregated to a single
  //                        score for the result list. TODO: explain what
  //                        sumProxAggregation does!?
  //
  //   score              : Scores can be of type DiskScore (a single byte,
  //                        for space efficiency) or of type Score (unsigned
  //                        int = 4 bytes). This argument serves to tell
  //                        which of the two types are used here. Would
  //                        actually be enough to specify this as a template
  //                        parameter, but at the time of the first writing
  //                        of the functions above, we could not figure out
  //                        the right syntax for doing this (hint: "typename"
  //                        resp. "template" must be used) 
  //   
  //   wordRange          : see the explanation of doNotCheckWordIds above.
  //                        Not considered when called with
  //                        doNotCheckWordIds, and so we can pass any object 
  //                        of type WordRange here. Note that the default
  //                        constructor gives the empty range. The infinite
  //                        range would be obtained by wordRange(-1, x) for
  //                        any x >= 0.
  //
  Score score = 0;                                   // see explanation, value does not matter
  WordRange wordRange;                               // see explanation, value does not matter
  SumProxAggregation sumProxAggregation;             // see explanation
  CompleterBase<MODE>::intersect(separator,
                                 doNotCheckWordIds,  // see explanation
                                 andIntersection,    // see explanation
                                 sumProxAggregation, // see explanation
                                 score,              // see explanation, value does not matter
                                 inputList1,
                                 inputList2._docIds,
                                 inputList2._positions,
                                 inputList2._scores,
                                 inputList2._wordIdsOriginal,
                                 resultList,
                                 wordRange           // see explanation, value does not matter
                                );
}
*/


//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template void CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>::
                intersectTwoPostingLists
                 (const QueryResult&       input1,
                  const QueryResult&       input2,
                        QueryResult&       result,
                  const Separator&         separator,    
                  const ScoreAggregation   scoreAggregation,
                  const WordRange&         wordIdRange);
