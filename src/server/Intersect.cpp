#include "Intersect.h"

//! Provide operator which takes a word id and always returns true
/*!
 *    Note: used as a template parameter in the new intersect when all word ids
 *    in the second list are known to be within the limits given by the query
 */
class AnyWordIdOk
{ 
 public: 
  bool operator()(WordId w) const { return true; }
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



/*
const Separator sameDocSeparator(" ", pair<int,int>(-1,-1), 0);
const Separator nearSeparator("..", pair<int,int>(-10,-10), 2);

//! Intersect mode
 *!
 *   Should replace Ingmar's completely ad hoc, messed up, mis-designed etc. etc.
 *   Separator class.
 *
       IntersectMode::IntersectMode() { set(sameDocSeparator, OUTPUT_MATCHES); }
       IntersectMode::IntersectMode(const Separator& separator, int outputMode) { set(separator, outputMode); }
void   IntersectMode::set(const Separator& separator, int outputMode) { _separator = separator; _outputMode = outputMode; }
int    IntersectMode::getLeft() const { return _separator._intersectionWindow.first; }
int    IntersectMode::getRight() const { return _separator._intersectionWindow.second; }
int    IntersectMode::getSeparatorIndex() const { return _separator._separatorIndex; }
int    IntersectMode::getOutputMode() const { return _outputMode; }
char   IntersectMode::getOutputModeAsChar() const { string codes = "XMNAX"; return codes[MIN(_outputMode, 4)]; }
string IntersectMode::getModeAsString() const  
{ 
  ostringstream os;
  os  << getLeft() << ".." << getRight() << " " << getOutputModeAsChar();
  return os.str();
}

SumAggregation sumScores;
const IntersectMode sameDoc(sameDocSeparator, OUTPUT_MATCHES);
*/



//! Intersect two posting lists (method called from outside)
void intersectTwoPostingLists(const QueryResult&     input1,
                              const QueryResult&     input2,
                                    QueryResult&     result,
                              const Separator&       separator,
                              const WordRange&       wordIdRange,
                              const ScoreAggregation scoreAggregation)
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
    THROW(Exception::NOT_YET_IMPLEMENTED, os.str());
  }
}



//! Intersect two posting lists; partially templated version 
/*!
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingLists to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsTemplated.
 */
template<class ScoreAggregator>
void intersectTwoPostingListsPartiallyTemplated(const QueryResult&     input1,
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
    // Note: restricting the range of word ids might not give the expected result,
    // if the output mode is not the default OUTPUT_MATCHES. TODO: give an example.
    // Anyway we allow it.

    // if (separator.getOutputMode() == Separator::OUTPUT_NON_MATCHES)
    //   THROW(Exception::OTHER, "may not restrict word range with output mode OUTPUT_NON_MATCHES");
    // if (separator.getOutputMode() == Separator::OUTPUT_ALL)
    //   THROW(Exception::OTHER, "may not restrict word range with output mode OUTPUT_ALL");
    CheckWordId checkWordId(wordIdRange.firstElement(), wordIdRange.lastElement());
    intersectTwoPostingListsPartiallyTemplated
      (input1, input2, result, separator, checkWordId, aggregateScores);
  }
}



//! Intersect two posting lists; partially templated version 
/*!
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingLists to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsTemplated.
 */
template<class ScoreAggregator,
         class CheckWordId>
void intersectTwoPostingListsPartiallyTemplated(const QueryResult&     input1,
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



//! Intersect two posting lists; partially templated version 
/*!
 *    Used in intermediate calls on the way from the outside call of
 *    intersectTwoPostingLists to the fully templated (hence more efficient but
 *    complicated to call directly) intersectTwoPostingListsTemplated.
 */
template<class ScoreAggregator,
         class CheckWordId,
         class CheckPosition>
void intersectTwoPostingListsPartiallyTemplated(const QueryResult&     input1,
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
    intersectTwoPostingListsTemplated
      <CheckWordId, CheckPosition, ScoreAggregator, Separator::OUTPUT_NON_MATCHES>
      (input1, input2, result, checkWordId, checkPosition, aggregateScores);
  }
  else if (outputMode == Separator::OUTPUT_ALL)
  {
    intersectTwoPostingListsTemplated
      <CheckWordId, CheckPosition, ScoreAggregator, Separator::OUTPUT_ALL>
      (input1, input2, result, checkWordId, checkPosition, aggregateScores);
  }
  else
  {
    ostringstream os;
    os << "invalid output mode (" << outputMode << ")";
    THROW(Exception::OTHER, os.str());
  }
}



typedef Vector<Position> PositionList;
typedef Vector<Score>    ScoreList;
typedef pair<unsigned int, unsigned int> UIntPair;


//! Intersect two posting lists, fully templated version 
/*!
 *    This method contains the actual code doing the intersection.
 *
 *    TODO: Make notes on that the templating is really important for efficiency
 */
template<class        CheckWordId, 
         class        CheckPosition, 
         class        ScoreAggregator,
         unsigned int outputMode>
void intersectTwoPostingListsTemplated(const QueryResult&     input1,
                                       const QueryResult&     input2,
                                             QueryResult&     result,
                                       const CheckWordId&     checkWordId,
                                       const CheckPosition&   checkPosition,
                                       const ScoreAggregator& aggregateScores)
{
  const DocList&      docIds1    = input1._docIds;
  //const WordList&     wordIds1   = input1._docIds;
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
  docIds3   .reserve(docIds3.size() + len2);
  wordIds3  .reserve(wordIds3.size() + len2);
  positions3.reserve(positions3.size() + len2);
  scores3   .reserve(scores3.size() + len2);
  unsigned int i = 0;
  unsigned int j = 0;
  //WordId wordId;
  //int posDiff; 
  //Score score;
  //DocId lastDocId1;
  bool needToCheckPositions = checkPosition.needToCheckPositions();
    //const bool needToCheckPositions = checkPosition(UINT_MAX) ? false : true;
  ASSERT(checkPosition.getRight() >= checkPosition.getLeft());
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
  DocId docId;
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
  while (true)
  {
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
    if (i == len1) break;

    // Main Loop, Step 2: Advance in second list 
    //
    // Note: for OUTPUT_NON_MATCHES or OUTPUT_ALL, add to output
    //
    //   L1 : D10 D12         D27 D39         
    //   L2 :         D13 D24         D39 D39 D56
    //   #1           j=0 j=1         j=2
    //   #2                                   j=4 j=5=len2
    //
    while (j < len2 && docIds2[j] < docIds1[i]) 
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
   
    // Main Loop, Step 3: Same doc ids
    //
    //   Case 1: no need to look at positions. (See computation of
    //   needToCheckPositions above.)
    //
    if (needToCheckPositions == false)
    {
      while (j < len2 && docIds1[i] == docIds2[j])
      {
        #ifdef CHECK_INTERSECT
        trace.push_back(2);
        #endif
        // For OUTPUT_NON_MATCHES, don't add to output
        if (outputMode == Separator::OUTPUT_MATCHES || outputMode == Separator::OUTPUT_ALL)
        {
          if (checkWordId(wordIds2[j]))
          {
            docIds3.   push_back(docIds2[j]);
            wordIds3.  push_back(wordIds2[j]);
            positions3.push_back(positions2[j]);
            scores3.   push_back(aggregateScores.aggregate(scores1[i], scores2[j]));
            #ifdef CHECK_INTERSECT
            trace.push_back(3);
            #endif
          }
        }
        ++j;
      }
        

      if (j == len2) break;
    }

    // Main Loop, Step 3: Same doc ids
    //
    //   Case 2: have to look at positions. Merge them!
    //
    else if (docIds1[i] == docIds2[j])
    {
      docId = docIds1[i];

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
                docIds3.   push_back(docIds2[j]);
                wordIds3.  push_back(wordIds2[j]);
                positions3.push_back(positions2[j]);
                scores3.   push_back(aggregateScores.aggregate(scores1[i-1], scores2[j]));
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
  docIds3   .unreserve();
  wordIds3  .unreserve();
  positions3.unreserve();
  scores3   .unreserve();

  // Check the result, using the "trace" computed along with the above
  #ifdef CHECK_INTERSECT
  cout << "Checking intersect result ... " << flush;
  {
    ASSERT(trace.size() <= docIds1.size() + docIds2.size() + docIds3.size());
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
        ASSERT(i < docIds1.size());
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
          THROW(Exception::CHECK_FAILED, os.str());
        }
        lastDocId    = docIds1[i];
        lastPosition = positions1[i];
        lastList     = 1;
        lastIndex    = i;
        ++i;
      }
      else if (trace[l] == 2)
      {
        ASSERT(j < docIds2.size());
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
          THROW(Exception::CHECK_FAILED, os.str());
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
        ASSERT(k < docIds3.size());
        ASSERT(i > 0);
        ASSERT(j > 0);
        ASSERT(docIds3[k] == docIds1[i-1]);
        ASSERT(docIds3[k] == docIds2[j-1]);
        ASSERT(checkWordId(wordIds3[k]));
        ASSERT(positions3[k] == positions2[j-1]);
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
          THROW(Exception::CHECK_FAILED, os.str());
        }
        ++k;
      }
      else
      {
        THROW(Exception::OTHER, "wrong value in check trace");
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
void intersectCheck(const QueryResult&     input1,
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
        THROW(Exception::OTHER, os.str());
      }
      else if (i + 2 >= all.size() || all[i+2].first != docId)
      {
        ostringstream os;
        os << "doc id " << docId << " from result list not found in input list " << 3 - all[i+1].second.first;
        THROW(Exception::OTHER, os.str());
      }
      ASSERT(i + 2 < all.size() && all[i+1].second.first == 1 && all[i+2].second.first == 2);
    }
  }

  // check all doc ids common to input1 and input2
  /*
  for (unsigned int i = 0; i + 1 < all.size(); ++i)
  {
    docId = all[i].first;
    if (all[i].second.first == 1 && all[i+1].first == docId)
    {
      ASSERT(all[i+1].second.first == 2);
      unsigned int jj1 == all[i].second.second;
      unsigned int jj2 == all[i+1].second.second;
      ASSERT(jj1 < docIds1.size() && docIds1[jj1] == docId);
      ASSERT(jj2 < docIds2.size() && docIds2[jj2] == docId);
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
void checkEquality(const QueryResult& input1,
                   const QueryResult& input2)
{
  //ASSERT(input1.check() == true);
  //ASSERT(input2.check() == true);

  cout << "check equality: " << flush;
  const DocList&      docIds1    = input1._docIds;
  const WordList&     wordIds1   = input1._wordIdsOriginal;
  const PositionList& positions1 = input1._positions;
  const DocList&      docIds2    = input2._docIds;
  const WordList&     wordIds2   = input2._wordIdsOriginal;
  const PositionList& positions2 = input2._positions;

  ASSERT(docIds1.size() == wordIds1.size());
  ASSERT(wordIds1.size() == positions1.size());
  ASSERT(docIds2.size() == wordIds2.size());
  ASSERT(wordIds2.size() == positions2.size());

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
