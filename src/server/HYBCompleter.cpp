#include "server/HYBCompleter.h"
#include "server/CustomScorer.h"

//! Needed for HybCompleter default constructor below
const vector<off_t>  emptyByteOffsetsForBlocks;
const Vector<WordId> emptyBoundaryWordIds;

extern float parallelSortTimePerMillionIntegers;
extern int statusCode;

// NOTE: This is an old comment; the include has been commented out
//
// BEWARE: including a cpp file below!!! (CompleterBase.cpp)
//
// EXPLANATION: CompleterBase.cpp contains the definitions of the template
// functions of the CompleterBase class (declared in CompleterBase.h). I tried
// to compile CompleterBase.cpp and HybCompleter.cpp separately and then link
// them in the end, but the linker could not resolve everything (namely not the
// calls to "intersect") in that case, presumably because HybCompleter calls
// methods of CompleterBase and vice versa.
//
// NOTE: It's actually fine this way, except if another completer, e.g.
// INVCompleter would do the same thing. Than the linker would complain about
// multiple definitions of a few globals defined in CompleterBase.cpp, which
// should then be moved to a separate file (or the template methods moved back
// from CompleterBase.cpp to CompleterBase.h, but only for that class, for all
// the other classes it's fine).

//#include "CompleterBase.cpp"

#define LOG CompleterBase<MODE>::log

// _____________________________________________________________________________
template <unsigned char MODE>
HybCompleter<MODE>::HybCompleter()
  : _byteOffsetsForBlocks(emptyByteOffsetsForBlocks),
    _boundaryWordIds(emptyBoundaryWordIds)
{
  CompleterBase<MODE>::_compressionBuffer = NULL;
  // cout << "! The default constructor of HybCompleter should never"
  //      << "be called (I think)" << endl;
  // exit(1);
}

//! CONSTRUCT FROM HYB INDEX DATA
template <unsigned char MODE>
HybCompleter<MODE>::HybCompleter
 (HYBIndex*         indexData,
  TimedHistory*          passedHistory,
  FuzzySearch::FuzzySearcherBase* fuzzySearcher,
  unsigned long int initIntBuffSize,
  unsigned long int initUniBuffSize,
  unsigned long int initCompressBuffSize)
:
  CompleterBase<MODE>::CompleterBase(passedHistory,
                                     &indexData->_vocabulary,
                                     indexData->getIndexFileName(),
                                     &indexData->_metaInfo,
                                     fuzzySearcher),
  _byteOffsetsForBlocks(indexData->_byteOffsetsForBlocks),
  _boundaryWordIds(indexData->_boundaryWordIds)
{
  #ifndef NDEBUG
  LOG << " HybCompleter constructor from index," << flush;
  LOG << " vocabulary size: " << CompleterBase<MODE>::_vocabulary.size() << endl;
  #endif
  CompleterBase<MODE>::setInitialBufferSizes(initIntBuffSize, initUniBuffSize, initCompressBuffSize);
  CompleterBase<MODE>::reserveCompressionBuffersAndResetPointersAndCounters();
}


//! COPY CONSTRUCTOR
template <unsigned char MODE>
HybCompleter<MODE>::HybCompleter(const HybCompleter<MODE>& orig)
  : CompleterBase<MODE>::CompleterBase(orig),
    _byteOffsetsForBlocks(orig._byteOffsetsForBlocks),
    _boundaryWordIds(orig._boundaryWordIds)
{
  #ifndef NDEBUG
  LOG << " HybCompleter copy constructor" << endl;
  #endif

  #ifndef NDEBUG
  LOG << " Vocabulary check in HybCompleter.h copy constructor: " << endl;
  LOG << CompleterBase<MODE>::_vocabulary.size() << endl;
  LOG << " Vocabulary check in HybCompleter.h copy constructor (orig): " << endl;
  LOG << orig._vocabulary.size() << endl;
  #endif
}



//! DESTRUCTOR
template<unsigned char MODE>
HybCompleter<MODE>::~HybCompleter()
{
  #ifndef NDEBUG
  LOG << "! HYB Destructor called " << endl;
  #endif
  if (CompleterBase<MODE>::_indexStructureFile.isOpen()) CompleterBase<MODE>::_indexStructureFile.close();
  CompleterBase<MODE>::freeCompressionBuffer();
}


//! Process basic prefix completion query (given doc ids D, and word range W)
/*!
 *    Note: This is the HYB Index implementation of this abstract method from
 *    CompleterBase
 *
 *    \param inputList  the given posting list D
 *    \param wordRange  the given word range W
 *    \param result     the result list (gives D' and W')
 *    \param separator  determines the intersection mode
 */
template<unsigned char MODE>
void HybCompleter<MODE>::processBasicQuery
      (const QueryResult& inputList,
       const WordRange&   wordRange,
             QueryResult& resultList,
       const Separator&   separator)
{
  LOG << AT_BEGINNING_OF_METHOD << "; separator is " << separator.infoString() << endl;

  // Note: inputList = conceptual full list NOT YET IMPLEMENTED
    //if (inputList.isFullResult()) CS_THROW(Exception::NOT_YET_IMPLEMENTED, "D = full list");

  // 1. Compute range of blocks covering given word range; TODO: stupid function name
  CS_ASSERT(wordRange.isEmptyRange() == false);
  BlockId firstBlockId;
  BlockId lastBlockId;
  blockRangeForNonEmptyWordRange(wordRange, firstBlockId, lastBlockId);
  CS_ASSERT(lastBlockId >= firstBlockId);
  CS_ASSERT(lastBlockId + 2 < _byteOffsetsForBlocks.size());
  bool startsAtFirstBlockExactly = (wordRange.firstElement() == _boundaryWordIds[firstBlockId]);
  bool endsAtLastBlockExactly = lastBlockId == _boundaryWordIds.size() - 1
                                 ? wordRange.lastElement() + 1 == (WordId)(CompleterBase<MODE>::_metaInfo->getNofWords())
                                 : wordRange.lastElement() + 1 == _boundaryWordIds[lastBlockId + 1];

  // 2. Process query for each of these blocks; note that intersect appends
  QueryResult currentBlock;
  for (BlockId currentBlockId = firstBlockId; currentBlockId <= lastBlockId; currentBlockId++)
  {
    // Check if the query processing took to long and abort if necessary.
    off_t totalProcessingTimeInMsecs =
        CompleterBase<MODE>::getTotalProcessingTimeInUsecs() / 1000.0;
    if (totalProcessingTimeInMsecs >= queryTimeout)
    {
      CompleterBase<MODE>::statusCode = 413;
      ostringstream os;
      os << "time elapsed: " << totalProcessingTimeInMsecs << " msecs > "
         << queryTimeout << " msecs";
      CS_THROW(Exception::QUERY_TIMEOUT, os.str());
    }

    // In some cases all word ids are in range and need not be checked by intersection
    bool allWordIdsFromBlockInRange =
          ((currentBlockId == firstBlockId && startsAtFirstBlockExactly && lastBlockId > firstBlockId)
            || (currentBlockId > firstBlockId && currentBlockId < lastBlockId)
            || (currentBlockId == lastBlockId && firstBlockId < lastBlockId && endsAtLastBlockExactly)
            || (firstBlockId == lastBlockId && startsAtFirstBlockExactly && endsAtLastBlockExactly));
    // TODO: the terminology "inifinite range" is misleading here
    // NEW 11Oct13 (baumgari): Uncommentend since unused variable.
    /* WordRange wordRangeForThisBlock = allWordIdsFromBlockInRange
                                        ? infiniteWordIdRange
                                        : wordRange; */

    // Get block (must clear result from previous round, otherwise getDataForBlockId will throw exception)
    currentBlock.clear();
    getDataForBlockId(currentBlockId, currentBlock);

    // Process query for this block; Note: will append to resultList
    //
    // Case 1: inputList is not the list of all postings
    if (inputList.isFullResult() == false)
    {
      CompleterBase<MODE>::intersectTwoPostingLists
       (inputList,
        currentBlock,
        resultList,
        separator,
        CompleterBase<MODE>::_queryParameters.docScoreAggDifferentQueryParts,
        wordRange);
    }
    // Case 2: inputList is the list of all postings
    else
    {
      DocList&      currentBlockDocIds    = currentBlock._docIds;
      WordList&     currentBlockWordIds   = currentBlock._wordIdsOriginal;
      PositionList& currentBlockPositions = currentBlock._positions;
      ScoreList&    currentBlockScores    = currentBlock._scores;
      DocList&      resultListDocIds      = resultList._docIds;
      WordList&     resultListWordIds     = resultList._wordIdsOriginal;
      PositionList& resultListPositions   = resultList._positions;
      ScoreList&    resultListScores      = resultList._scores;
      // check that all lists are of the same size
      // CompleterBase<MODE>::log << IF_VERBOSITY_HIGH << "! check current block, mode is " << int(MODE) << endl;
      currentBlock.checkSameNumberOfDocsWordsPositionsScores(MODE);
      // CompleterBase<MODE>::log << IF_VERBOSITY_HIGH << "! check result list, mode is " << int(MODE) << endl;
      resultList.checkSameNumberOfDocsWordsPositionsScores(MODE);
      // now append currentBlock to resultList, respecting wordRange
      for (unsigned int i = 0; i < currentBlockDocIds.size(); ++i)
      {
        // TODO: if all word ids are in range, there is no need for an if here
        if (wordRange.isInRange(currentBlockWordIds[i]))
        {
          resultListDocIds .push_back(currentBlockDocIds[i]);
          resultListWordIds.push_back(currentBlockWordIds[i]);
          if (MODE & WITH_POS)    resultListPositions.push_back(currentBlockPositions[i]);
          if (MODE & WITH_SCORES) resultListScores   .push_back(currentBlockScores[i] +
              (currentBlockWordIds[i] == CompleterBase<MODE>::_lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
        }
        // if (i == 0)
        // {
        //   CompleterBase<MODE>::log << "* DEBUG DEBUG DEBUG : _lastBestMatchWordId = "
        //                            << CompleterBase<MODE>::_lastBestMatchWordId << endl;
        //   CompleterBase<MODE>::log << "* DEBUG DEBUG DEBUG : block size  = "
        //                            << currentBlockDocIds.size() << endl;
        // }
        // if (currentBlockWordIds[i] == CompleterBase<MODE>::_lastBestMatchWordId)
        //   CompleterBase<MODE>::log << "* DEBUG DEBUG DEBUG : match at posting #" << i << endl;
      }
    }
  }

  // 3. If more than one block processed, have to sort
  if (firstBlockId < lastBlockId)
  {
    // NEW 09Oct13 (baumgari): Sorting can take a very long while and we want to
    // ensure, that a query doesn't tie the server. It's hard to
    // measure the time on sorting, since that would be done on almost every
    // iteration. Note: Sorting is done in Vector.h by using the sort function
    // of the algorithm library. To use that a boolean function is defined, which
    // decides (for a pair of values), which one is greater than the other. This
    // is the only reasonable place to add a timeout check, but this would slow
    // down the sorting itself. We use an heuristic solution instead. Sorting
    // takes at least as long as the decompressing. Therefore we just throw an
    // arrow, if the decompressing already took half of the maximal allowed time
    // ( == queryTimeout).
    size_t n = resultList._docIds.size()
             + resultList._wordIdsOriginal.size()
	     + resultList._positions.size()
	     + resultList._scores.size();
    float numOfMillionsToSort = n / (1 * 1000 * 1000);

    if (numOfMillionsToSort > 1) 
    {
      CompleterBase<MODE>::threadTimer.stop();
      CompleterBase<MODE>::threadTimer.cont();
      off_t totalTime = CompleterBase<MODE>::getTotalProcessingTimeInUsecs();
      off_t expectedTimeForSorting = (numOfMillionsToSort * parallelSortTimePerMillionIntegers);
      if (((totalTime + expectedTimeForSorting) / 1000) > queryTimeout)
      {
        ostringstream os;
        os << "expect sorting to exceed query timeout; "
           << "elapsed time [" << totalTime / 1000 << " msecs]"
           << " + estimated sorting time [" << expectedTimeForSorting / 1000 << " msecs]"
           << " > queryTimeout [" << queryTimeout << " msecs]";
        CompleterBase<MODE>::statusCode = 413;
        CS_THROW(Exception::QUERY_TIMEOUT, os.str());
      }
    }
 
    CompleterBase<MODE>::stlSortTimer.cont();
    // TODO: sortLists is a misnomer now, because we refer to a QueryResult as a
    // single list of postings now
    resultList.sortLists();
    CompleterBase<MODE>::stlSortTimer.stop();
  }

  resultList._docIds.markAsSorted(true); //also obsolete but keep for some checks
}


//! Process basic prefix completion query; OLD CODE BY INGMAR
/*
template<unsigned char MODE>
void HybCompleter<MODE>::allMatchesForWordRangeAndCandidates
      (const Separator&   splitSeparator,
             bool         notIntersectionMode,
       const WordRange&   wordRange,
       const QueryResult& candidateLists,
             QueryResult& matchingLists,
       const QueryResult* listsForPrefixFromHistory)
{
  assert((candidateLists._docIds.size() == 0)
      ||(candidateLists._docIds.back() != INFTY_DOCID));
  assert(!(MODE & WITH_SCORES)
      || candidateLists._scores.size()
      == candidateLists._docIds.size() );
  assert( !(MODE & WITH_POS)
      || candidateLists._positions.size()
      == candidateLists._docIds.size() );
  assert(!wordRange.isEmptyRange());
  #ifndef NDEBUG
  signed char separatorMode = splitSeparator._separatorIndex;
  #endif
  assert(candidateLists._status & IS_FINISHED);
  assert(notIntersection());
  assert(!andIntersection());
  assert( (separatorMode != FULL) || (!notIntersectionMode));
  assert(matchingLists._docIds.size() == 0);
  assert( candidateLists._docIds.isSorted() );
  assert( (candidateLists._docIds.isFullList()
            && candidateLists.isFullResult())
         || (!candidateLists._docIds.isFullList()
            && !candidateLists.isFullResult()) );
  assert( candidateLists._docIds.isFullList()
         || candidateLists._docIds.size() > 0 );
  assert( !(MODE & WITH_SCORES)
         || candidateLists._scores.size()
              == candidateLists._docIds.size() );
  assert( !(MODE & WITH_POS)
         || candidateLists._positions.size()
              == candidateLists._docIds.size() );
  BlockId firstBlockId, lastBlockId;

  // TODO FOR INGMAR: why the following two cases? and what does "last prefix" refer to?

  // CASE 1: LAST PREFIX WAS IN HISTORY
  if (listsForPrefixFromHistory != NULL)
  {
    #ifndef NDEBUG
    LOG << "! using last prefix from history ... " << flush;
    #endif

    if (listsForPrefixFromHistory->isLockedForWriting)
      CS_THROW(Exception::RESULT_LOCKED_FOR_WRITING, "");
    listsForPrefixFromHistory->isLockedForReading = true;

    if(!listsForPrefixFromHistory->check()) CS_THROW(Exception::BAD_QUERY_RESULT, "");
    if (MODE & WITH_SCORES) assert(candidateLists._scores.size() == candidateLists._docIds.size());
    if (MODE & WITH_POS)    assert(candidateLists._positions.size() == candidateLists._docIds.size());
    cout << "! NEW INTERSECT: called from case \"last prefix was in history\" of HybCompleter" << endl;
    CompleterBase<MODE>::intersectTwoPostingLists(candidateLists,
                                                  *listsForPrefixFromHistory,
                                                  matchingLists,
                                                  splitSeparator,
                                                  SCORE_AGG_SUM,
                                                  infiniteWordIdRange);
    #ifndef NDEBUG
    LOG << " ... done" << endl << flush;
    #endif
  }

  // CASE 2: LAST PREFIX WAS NOT IN HISTORY
  else
  {
    #ifndef NDEBUG
    LOG << "! reading block from disk ... " << flush;
    #endif
    // TODO FOR INGMAR: call this getBlockRange, or computeBlockRange
    //                  no need fot the For... when wordRange is one of the arguments
    assert(!wordRange.isEmptyRange());
    blockRangeForNonEmptyWordRange(wordRange, firstBlockId, lastBlockId);
    assert( lastBlockId >= firstBlockId );
    assert( lastBlockId + 2 < _byteOffsetsForBlocks.size() );
    // LOG << "\n first relevant blockId : " << firstBlockId << "\n";
    // LOG << "\n last relevant blockId : " << lastBlockId << "\n";
    assert( CompleterBase<MODE>::_metaInfo.getNofWords() > 0);
    bool startsAtFirstBlockExactly = (_boundaryWordIds[firstBlockId] == wordRange.firstElement());
    bool endsAtLastBlockExactly = (lastBlockId == _boundaryWordIds.size()-1 && wordRange.lastElement()
        == (WORDID)(CompleterBase<MODE>::_metaInfo.getNofWords()-1))
      || (_boundaryWordIds[lastBlockId+1] == wordRange.lastElement()+1);

    DocList currentDoclist;
    WordList currentWordlist;
    Vector<Position> currentPositionlist;
    Vector<DiskScore> currentScorelist;
    assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));

    // PROCESS FIRST BLOCK
    BlockId currentBlock = firstBlockId;
    getDataForBlockId(currentBlock, currentDoclist, currentPositionlist, currentScorelist, currentWordlist);
    // CASE 1: all word ids in range
    if ( startsAtFirstBlockExactly && (lastBlockId > firstBlockId || endsAtLastBlockExactly) )
    {
      assert ( (separatorMode != FULL) || !notIntersectionMode);
      assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
      if (notIntersectionMode)
      {
        assert( !(MODE & WITH_SCORES)
                   || candidateLists._scores.size()
                        == candidateLists._docIds.size() );
        assert( !(MODE & WITH_POS)
                   || candidateLists._positions.size()
                         == candidateLists._docIds.size() );
        CompleterBase<MODE>::intersect
          (splitSeparator,
           doNotCheckWordIds,
           notIntersection,
           LOCATION_SCORE_AGGREGATION,
           DiskScore(),
           candidateLists,
           currentDoclist,
           currentPositionlist,
           currentScorelist,
           currentWordlist,
           matchingLists,
           WordRange(-1,1));
      }
      else
      {
        assert ( (separatorMode != FULL) || !notIntersectionMode);
        assert( !(MODE & WITH_SCORES)
                   || candidateLists._scores.size()
                        == candidateLists._docIds.size() );
        assert( !(MODE & WITH_POS)
                   || candidateLists._positions.size()
                         == candidateLists._docIds.size() );
        CompleterBase<MODE>::intersect
         (splitSeparator,
          doNotCheckWordIds,
          andIntersection,
          LOCATION_SCORE_AGGREGATION,
          DiskScore(),
          candidateLists,
          currentDoclist,
          currentPositionlist,
          currentScorelist,
          currentWordlist,
          matchingLists,
          WordRange(-1,1));
      }
      assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
    }

    // CASE 2: have to check word is
    else
    {
      assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
      assert(candidateLists._scores.isPositive());
      assert(currentScorelist.isPositive());
      if (notIntersectionMode)
      {
        assert ( (separatorMode != FULL) || !notIntersectionMode);
        assert( !(MODE & WITH_SCORES)
                   || candidateLists._scores.size()
                        == candidateLists._docIds.size() );
        assert( !(MODE & WITH_POS)
                   || candidateLists._positions.size()
                        == candidateLists._docIds.size() );
        CompleterBase<MODE>::intersect
         (splitSeparator,
          doCheckWordIds,
          notIntersection,
          LOCATION_SCORE_AGGREGATION,
          DiskScore(),
          candidateLists,
          currentDoclist,
          currentPositionlist,
          currentScorelist,
          currentWordlist,
          matchingLists,
          wordRange);
      }
      else
      {
        assert ( (separatorMode != FULL) || !notIntersectionMode);
        assert((candidateLists._docIds.size() == 0)
                 ||(candidateLists._docIds.back() != INFTY_DOCID));
        assert( !(MODE & WITH_SCORES)
                 || candidateLists._scores.size()
                      == candidateLists._docIds.size() );
        assert( !(MODE & WITH_POS)
                 || candidateLists._positions.size()
                      == candidateLists._docIds.size() );
        CompleterBase<MODE>::intersect
         (splitSeparator,
          doCheckWordIds,
          andIntersection,
          LOCATION_SCORE_AGGREGATION,
          DiskScore(),
          candidateLists,
          currentDoclist,
          currentPositionlist,
          currentScorelist,
          currentWordlist,
          matchingLists,
          wordRange);
      }
      assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
    }

    // TODO FOR INGMAR: it took me several minute to understand the above if-else (in the original formatting). The
    //                  case distinction (which is only because of templating, right?) should be in the intersect
    //                  wrapper, wich should take the first two arguments as an int resp. bool. That would make
    //                  things much clearer.
    // TODO FOR INGMAR: why did you call it "filtering" in the comments (which totally confused me, because this is
    //                  not the filtering from the paper), while you call it "checkWordsIds" in the arguments (which
    //                  is a good name). #@*$!
    //                  to understand these comments, you will have to compare with the old version

    // PROCESS ALL MIDDLE BLOCKS (where all word ids are in the given word range)
    //if( lastBlockId > firstBlockId +1 )
    for (currentBlock = firstBlockId + 1; currentBlock < lastBlockId; currentBlock++)
    {
      getDataForBlockId(currentBlock, currentDoclist, currentPositionlist, currentScorelist, currentWordlist);
      // intersect without filtering
      #ifndef NDEBUG
      for (unsigned int i=0;i<currentWordlist.size();i++) {assert(wordRange.isInRange(currentWordlist[i]));}
      #endif
      assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
      if (notIntersectionMode)
      {
        assert ( (separatorMode != FULL) || !notIntersectionMode);
        assert( !(MODE & WITH_SCORES) || candidateLists._scores.size() == candidateLists._docIds.size() );
        assert( !(MODE & WITH_POS) || candidateLists._positions.size() == candidateLists._docIds.size() );
        CompleterBase<MODE>::intersect(splitSeparator,
                                       doNotCheckWordIds,
                                       notIntersection,
                                       LOCATION_SCORE_AGGREGATION,
                                       DiskScore(),
                                       candidateLists,
                                       currentDoclist,
                                       currentPositionlist,
                                       currentScorelist,
                                       currentWordlist,
                                       matchingLists,
                                       WordRange(-1,1));
      }
      else
      {
        assert ( (separatorMode != FULL) || !notIntersectionMode);
        assert( !(MODE & WITH_SCORES) || candidateLists._scores.size() == candidateLists._docIds.size() );
        assert( !(MODE & WITH_POS) || candidateLists._positions.size() == candidateLists._docIds.size() );
        CompleterBase<MODE>::intersect(splitSeparator,
                                       doNotCheckWordIds,
                                       andIntersection,
                                       LOCATION_SCORE_AGGREGATION,
                                       DiskScore(),
                                       candidateLists,
                                       currentDoclist,
                                       currentPositionlist,
                                       currentScorelist,
                                       currentWordlist,
                                       matchingLists,
                                       WordRange(-1,1));
      }
        assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
        // currentDoclist.clear(); currentWordlist.clear();
    }

    if (firstBlockId < lastBlockId)
    {
      currentBlock = lastBlockId;
      getDataForBlockId(currentBlock, currentDoclist, currentPositionlist, currentScorelist, currentWordlist);

      if(endsAtLastBlockExactly)
      {
#ifndef NDEBUG
        for(unsigned int i=0;i<currentWordlist.size();i++) {assert(wordRange.isInRange(currentWordlist[i]));}
#endif
        // intersect without filtering
        assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
        if (notIntersectionMode)
        {
          assert ( (separatorMode != FULL) || !notIntersectionMode);
          assert( !(MODE & WITH_SCORES) || candidateLists._scores.size() == candidateLists._docIds.size() );
          assert( !(MODE & WITH_POS) || candidateLists._positions.size() == candidateLists._docIds.size() );
          CompleterBase<MODE>::intersect(splitSeparator,
                                         doNotCheckWordIds,
                                         notIntersection,
                                         LOCATION_SCORE_AGGREGATION,
                                         DiskScore(),
                                         candidateLists,
                                         currentDoclist,
                                         currentPositionlist,
                                         currentScorelist,
                                         currentWordlist,
                                         matchingLists,
                                         WordRange(-1,1));
        }
        else
        {
          assert ( (separatorMode != FULL) || !notIntersectionMode);
          assert( !(MODE & WITH_SCORES) || candidateLists._scores.size() == candidateLists._docIds.size() );
          assert( !(MODE & WITH_POS) || candidateLists._positions.size() == candidateLists._docIds.size() );
          CompleterBase<MODE>::intersect(splitSeparator,
                                         doNotCheckWordIds,
                                         andIntersection,
                                         LOCATION_SCORE_AGGREGATION,
                                         DiskScore(),
                                         candidateLists,
                                         currentDoclist,
                                         currentPositionlist,
                                         currentScorelist,
                                         currentWordlist,
                                         matchingLists,
                                         WordRange(-1,1));
        }
        assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
          //              currentDoclist.clear(); currentWordlist.clear();
      }
      else
      {
        // intersect with filtering
        assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
        if (notIntersectionMode)
        {
          assert ( (separatorMode != FULL) || !notIntersectionMode);
          assert( !(MODE & WITH_SCORES) || candidateLists._scores.size() == candidateLists._docIds.size() );
          assert( !(MODE & WITH_POS) || candidateLists._positions.size() == candidateLists._docIds.size() );
          CompleterBase<MODE>::intersect(splitSeparator,
                                         doCheckWordIds,
                                         notIntersection,
                                         LOCATION_SCORE_AGGREGATION,
                                         DiskScore(),
                                         candidateLists,
                                         currentDoclist,
                                         currentPositionlist,
                                         currentScorelist,
                                         currentWordlist,
                                         matchingLists,
                                         wordRange);
        }
        else
        {
          assert ( (separatorMode != FULL) || !notIntersectionMode);
          assert( !(MODE & WITH_SCORES) || candidateLists._scores.size() == candidateLists._docIds.size() );
          assert( !(MODE & WITH_POS) || candidateLists._positions.size() == candidateLists._docIds.size() );
          CompleterBase<MODE>::intersect(splitSeparator,
                                         doCheckWordIds,
                                         andIntersection,
                                         LOCATION_SCORE_AGGREGATION,
                                         DiskScore(),
                                         candidateLists,
                                         currentDoclist,
                                         currentPositionlist,
                                         currentScorelist,
                                         currentWordlist,
                                         matchingLists,
                                         wordRange);
        }
        assert(notIntersectionMode || CompleterBase<MODE>::checkWordlistBuffer(wordRange));
          //              currentDoclist.clear(); currentWordlist.clear();
      }
    }
    #ifndef NDEBUG
    LOG << " ... done" << endl << flush;
    #endif
  }// end case: last prefix was not in history


  // HAVE TO SORT ? If yes, then do so.
  if (!( (listsForPrefixFromHistory) || (firstBlockId == lastBlockId)))
  {
    CompleterBase<MODE>::stlSortTimer.cont();
    matchingLists.sortLists();
    CompleterBase<MODE>::stlSortTimer.stop();
  }
  matchingLists._docIds.markAsSorted(true); //also obsolete but keep for some checks

} // end: allMatchesForWordRangeAndCandidates
*/


//! Compute range of block ids covering given word range
/*
 *   Note: the range is [first, last] including both first and last!
 */
template<unsigned char MODE>
void HybCompleter<MODE>::blockRangeForNonEmptyWordRange
      (const WordRange& wordRange,
             BlockId&   first,
             BlockId&   last) const
{
  assert(!wordRange.isEmptyRange());

  BlockId low = 0;
  BlockId high = _boundaryWordIds.size()-1;

  assert(wordRange.firstElement() >= 0);
  assert(wordRange.firstElement() <= wordRange.lastElement());

  /*
  CompleterBase<MODE>::log << IF_VERBOSITY_HIGH
    << "In blockRangeForNonEmptyWordRange, wordRange.firstElement() = "
    << wordRange.firstElement() << ", wordRange.lastElement() = "
    << wordRange.lastElement() << endl;
  CompleterBase<MODE>::log << IF_VERBOSITY_HIGH
    << "first word:" << CompleterBase<MODE>::_vocabulary.operator[](wordRange.firstElement())
    << ", last word:" << CompleterBase<MODE>::_vocabulary.operator[](wordRange.lastElement())
    << endl;
  */

  /* binary search for the first matching block id */
  while(low < high)
  {
    if (wordRange.firstElement() > _boundaryWordIds[(low+high)/2])
      low = (low + high)/2 + 1;
    else
      high = (low + high)/2;
  }
  // low should now be the first element which is not smaller than the lower wordRange

  assert(low == high);
  if((low>0)&&(_boundaryWordIds[low]>wordRange.firstElement() )) {low--; high--;}
  assert( _boundaryWordIds[low] <= wordRange.firstElement());
  assert( _boundaryWordIds[low+1] > wordRange.firstElement());
  //assert( _boundaryWordIds[low-1] < wordRange.firstElement());
  assert( _boundaryWordIds[high] <= wordRange.lastElement());
  assert((low==0) || ( _boundaryWordIds[low-1] < wordRange.firstElement()));
  // low should now be the block id of the first relevant block

  /* linearly go through the rest until the last block id is found
     (.. which takes 0 time but could be done by binary search on the rest) */
  while (high+1 < _boundaryWordIds.size() && _boundaryWordIds[high+1] <= wordRange.lastElement())
  {
    ++high;
  }
  // NEW 02Aug10 (Hannah): The correction was also wrong!
  // NEW 08Apr08 (Holger): This looked wrong
  assert(high < _boundaryWordIds.size());
  assert(high + 1 == _boundaryWordIds.size() || _boundaryWordIds[high + 1] > wordRange.lastElement());
    // assert(high == _boundaryWordIds.size() || _boundaryWordIds[high] >= wordRange.lastElement()); // this fails, too!
    //assert(high == _boundaryWordIds.size() || _boundaryWordIds[high+1] >= wordRange.lastElement()); // this fails
  assert(high == 0 || _boundaryWordIds[high - 1] < wordRange.lastElement());
  first = low;
  last = high;

  #ifdef HYB_DEBUG
  LOG << "_boundaryWordIds[low] = " << _boundaryWordIds[low]
      << ", _boundaryWordIds[high] = " << _boundaryWordIds[high]
      << ", low = " << low << ", high = "<< high  << endl;
  LOG << "high boundary word: "
      << CompleterBase<MODE>::_vocabulary.operator[](_boundaryWordIds[high])
      << ", low boundary word: "
      << CompleterBase<MODE>::_vocabulary.operator[](_boundaryWordIds[low])
      << endl;
  LOG << "high boundary word-1: "
      << CompleterBase<MODE>::_vocabulary.operator[](_boundaryWordIds[high]-1)
      << ", low boundary word +1: "
      << CompleterBase<MODE>::_vocabulary.operator[](_boundaryWordIds[low]+1)
      << endl;
  LOG << "high-1 boundary word: "
      << CompleterBase<MODE>::_vocabulary.operator[](_boundaryWordIds[high-1])
      << ", low+1 boundary word: "
      << CompleterBase<MODE>::_vocabulary.operator[](_boundaryWordIds[low+1])
      << endl;
  #endif

  assert(last + 2< _byteOffsetsForBlocks.size());
}


//! Read given block from disk; NEW INTERFACE: read into a QueryResult now
/*!
 *    \param blockId  block id; must be less than [what?]
 *    \param block    result lists; must be empty, exception thrown otherwise
 *
 *    Precondition: result list must be empty, to avoid accidental overwriting.
 *
 *    TODO: don't call the old method, but integrate it into this method and
 *    then discard the old code.
 *
 *    TODO: copying of score list accounted for in resizeAndReserveTimer, is
 *    that a good idea? Also check whether the copying via reserve and push_back
 *    is really efficient.
 */
template<unsigned char MODE>
void HybCompleter<MODE>::getDataForBlockId(BlockId blockId, QueryResult& block)
{
  // Block must be empty
  CS_ASSERT(block.isEmpty());

  // Use Ingmar's old method to fetch the individual lists, with scores of type DiskScore
  Vector<DiskScore> diskScores;
  Vector<Score>& scores = block._scores;
  const WordList& wordIds = block._wordIdsOriginal;
  getDataForBlockId(blockId,
                    block._docIds,
                    block._positions,
                    diskScores,
                    block._wordIdsOriginal);

  // Copy list of disk scores (1 byte each) to list of block scores (4 bytes each)
  CompleterBase<MODE>::resizeAndReserveTimer.cont();
  scores.resize(diskScores.size());
  CS_ASSERT_EQ(diskScores.size(), wordIds.size());
  CS_ASSERT_EQ(diskScores.size(), scores.size());
  // NEW 11.11.11 (Hannah): Optionally substitute custom scores. See CustomScore.h for a
  // more detailed explanation. The globalCustomScorer is set in
  // CompletionServer.cpp, if and only if startCompletionServer is called with
  // --read-custom-scores.
  if (globalCustomScorer != NULL && wordIds.size() > 0)
  {
    string firstWord = this->getWordFromVocabulary(wordIds[0]);
    string lastWord = this->getWordFromVocabulary(wordIds[wordIds.size() - 1]);
    if (firstWord.size() > 5) firstWord.erase(4).append("..");
    if (lastWord.size() > 5) lastWord.erase(4).append("..");
    // TODO(bast): currently counting the number of replaced scores for
    // debugging purposes. Remove ASAP for better efficiency!
    size_t numScoresReplaced = 0;
    for (size_t i = 0; i < diskScores.size(); ++i)
    {
      scores[i] = globalCustomScorer->getScore(wordIds[i], diskScores[i]);
      if (scores[i] != diskScores[i]) numScoresReplaced++;
    }
    if (numScoresReplaced > 0)
    {
      LOG << "* NEW(CustomScorer): overwritten " << commaStr(numScoresReplaced)
          << " disk scores with custom scores (block \"" << firstWord 
          << "\" - \"" << lastWord << "\", size = " << commaStr(scores.size())
          << ")" << endl;
    }
  }
  else
  {
    for (size_t i = 0; i < diskScores.size(); ++i)
      scores[i] = diskScores[i];
  }
  CompleterBase<MODE>::resizeAndReserveTimer.stop();

  // Check that indeed all scores were copied
  CS_ASSERT(block._scores.size() == block._docIds.size());
}



//! Read given block from disk (docs, words, positions, scores)
template<unsigned char MODE>
void HybCompleter<MODE>::getDataForBlockId
      (BlockId            blockId,
       DocList&           doclist,
       Vector<Position>&  positionlist,
       Vector<DiskScore>& scorelist,
       WordList&          wordlist)
{
  // + 2 here because:
  //   the last but one byte offset points to the boundary word IDs vector
  //   the last byte offset points to the meta info
  assert(_byteOffsetsForBlocks.size() > blockId+2);

  ++CompleterBase<MODE>::nofBlocksReadFromFile;

  off_t offsetForDoclist, offsetForPositionlist=2, offsetForWordlist, offsetForScorelist=0;
  unsigned long int nofDocsInCompressedList;
  unsigned long int nofPositionsInCompressedList;
  unsigned long int nofWordsInCompressedList;
  unsigned long int nofScoresInList;

  #ifndef NDEBUG
  LOG << "! start reading from file ... " << flush;
  #endif

  //
  // R.0 Read offsets of the various lists (doc ids, positions, word ids, scores)
  //

  CompleterBase<MODE>::volumeReadFromFile +=   _byteOffsetsForBlocks[blockId+1] - _byteOffsetsForBlocks[blockId];
  CompleterBase<MODE>::fileReadTimer.cont();
  // READ OFFSETS FOR LISTS (currently not used) AND READ NOF DOCS IN COMPRESSED LIST
  // seek is only done once in the read below. All other reads are from the current position onwards
  // .... FOR DOCLIST
  CompleterBase<MODE>::_indexStructureFile.read(&offsetForDoclist, sizeof(off_t), _byteOffsetsForBlocks[blockId]);
  // .... FOR POSITIONLIST
  if(MODE & WITH_POS) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForPositionlist,sizeof(off_t));}
  // .... FOR WORDLIST
  CompleterBase<MODE>::_indexStructureFile.read(&offsetForWordlist,sizeof(off_t));
  // ...  FOR SCORELIST
  if(MODE & WITH_SCORES) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForScorelist,sizeof(off_t));}
  if(!(MODE & WITH_POS)) { assert(offsetForPositionlist ==2);offsetForPositionlist=offsetForWordlist;}
  else {assert(offsetForWordlist > offsetForPositionlist); }

  //
  // R.1 Read list of compressed doc ids (will be uncompressed later)
  //

  // Read number of doc ids and resize compression buffer if necessary
  CompleterBase<MODE>::_indexStructureFile.read(&nofDocsInCompressedList,sizeof(unsigned long int));
  CompleterBase<MODE>::fileReadTimer.stop();
  assert(offsetForDoclist < offsetForWordlist);
  assert((!(MODE & WITH_POS))||(offsetForDoclist < offsetForPositionlist));
  assert((!(MODE & WITH_POS))||(offsetForWordlist > offsetForPositionlist));
  assert((!(MODE & WITH_SCORES)) || (offsetForScorelist > offsetForWordlist));
  if(  _byteOffsetsForBlocks[blockId+1]
          - offsetForDoclist
          - NUMBER_OF_LISTS_PER_BLOCK*sizeof(unsigned long int)
       > (size_t) CompleterBase<MODE>::compressionBufferSize())
  {
    CompleterBase<MODE>::resizeCompressionBuffer
      ((unsigned long) ceil(1.3*( _byteOffsetsForBlocks[blockId+1]
                                    - offsetForDoclist - NUMBER_OF_LISTS_PER_BLOCK )));
  }
  CompleterBase<MODE>::fileReadTimer.cont();
  // Read the doc ids
  CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer,
                                                (size_t) offsetForPositionlist
                                                  - offsetForDoclist
                                                  - sizeof(unsigned long int));

  //
  // R.2 Read list of compressed positions (optional)
  //

  if (MODE & WITH_POS)
  {
    assert(offsetForPositionlist + (off_t) sizeof(unsigned long int) < offsetForWordlist);
    // Read number of positions; check that equal to the number of doc ids
    CompleterBase<MODE>::_indexStructureFile.read(&nofPositionsInCompressedList,
                                                  sizeof(unsigned long int));
    if (nofDocsInCompressedList != nofPositionsInCompressedList)
    {
      ostringstream os;
      os << "number of docs (" << nofDocsInCompressedList << ")"
         << " does not match number of positions (" << nofPositionsInCompressedList << ")"
         << " in block with id " << blockId;
      CS_THROW(Exception::UNCOMPRESS_ERROR, os.str());
    }
    assert(nofDocsInCompressedList == nofPositionsInCompressedList);
    // Read the compressed positions
    CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer
                                                    + offsetForPositionlist
                                                    - offsetForDoclist
                                                    - sizeof(unsigned long int),
                                                  (size_t) offsetForWordlist
                                                    - offsetForPositionlist
                                                    - sizeof(unsigned long int) );
  }

  //
  // R.3 Read list of compressed word ids
  //

  CompleterBase<MODE>::_indexStructureFile.read(&nofWordsInCompressedList,sizeof(unsigned long int));
  assert(nofDocsInCompressedList == nofWordsInCompressedList);
  assert(offsetForDoclist + (off_t) sizeof(unsigned long int) < offsetForWordlist);
  if (MODE & WITH_SCORES)
  {
    assert(offsetForScorelist > offsetForWordlist + (off_t) sizeof(unsigned long int));
    if (MODE & WITH_POS)
    {
      assert(offsetForPositionlist + (off_t) sizeof(unsigned long) < offsetForWordlist);
      CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer
                                                      + offsetForWordlist
                                                      - offsetForDoclist
                                                      - 2*sizeof(unsigned long int),
                                                    (size_t) offsetForScorelist
                                                      - offsetForWordlist
                                                      - sizeof(unsigned long int) );
    }
    else
    {
      CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer
                                                      + offsetForWordlist
                                                      - offsetForDoclist
                                                      - sizeof(unsigned long int),
                                                    (size_t) offsetForScorelist
                                                      - offsetForWordlist
                                                      - sizeof(unsigned long int) );
    }
  }
  else
  {
    if (MODE & WITH_POS)
    {
      CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer
                                                      + offsetForWordlist
                                                      - offsetForDoclist
                                                      - 2*sizeof(unsigned long int),
                                                    (size_t) _byteOffsetsForBlocks[blockId+1]
                                                      - offsetForWordlist
                                                      - sizeof(unsigned long int) );
    }
    else
    {
      CompleterBase<MODE>::_indexStructureFile.read(CompleterBase<MODE>::_compressionBuffer
                                                      + offsetForWordlist
                                                      - offsetForDoclist
                                                      - sizeof(unsigned long int),
                                                    (size_t) _byteOffsetsForBlocks[blockId+1]
                                                      - offsetForWordlist
                                                      - sizeof(unsigned long int) );
    }
  }

  //
  // R.4 Read list of scores (optional)
  //
  //   Scores are uncompressed, but of type DiskScore
  //

  if (MODE & WITH_SCORES)
  {	
    assert(offsetForWordlist + sizeof(unsigned long int) < offsetForScorelist);
    assert( _byteOffsetsForBlocks[blockId+1] > offsetForScorelist);
    // Read number of scores
    CompleterBase<MODE>::_indexStructureFile.read(&nofScoresInList,sizeof(unsigned long int));
    assert(nofDocsInCompressedList == nofScoresInList);
    scorelist.resize(nofScoresInList);
    CompleterBase<MODE>::_indexStructureFile.read(&scorelist[0],
                                                  (size_t) _byteOffsetsForBlocks[blockId+1]
                                                    - offsetForScorelist
                                                    - sizeof(unsigned long int) );
    assert(scorelist.isContiguous());
    assert(scorelist.isPositive());
    CompleterBase<MODE>::scorelistVolumeRead += nofScoresInList*sizeof(DiskScore);

  }

  CompleterBase<MODE>::fileReadTimer.stop();
  #ifndef NDEBUG
  LOG << "!  ... done " << endl << flush;
  #endif

  //
  // D.1 Decompress list of doc ids
  //

  #ifndef NDEBUG
  LOG << "! start decompressing docs ... " << flush;
  #endif
  CompleterBase<MODE>::resizeAndReserveTimer.cont();
  doclist.resize(nofDocsInCompressedList);
  CompleterBase<MODE>::resizeAndReserveTimer.stop();
  CompleterBase<MODE>::doclistDecompressionTimer.cont();
  _doclistCompressionAlgorithm.decompress(CompleterBase<MODE>::_compressionBuffer, &doclist[0], nofDocsInCompressedList);
  CompleterBase<MODE>::doclistDecompressionTimer.stop();
  assert(doclist.isContiguous());
  assert(doclist.isSorted());

  #ifndef NDEBUG
  LOG << "!  ... done " << endl << flush;
  for(unsigned long j = 0; j< doclist.size(); j++) {assert(doclist[j] <= CompleterBase<MODE>::_metaInfo.getMaxDocID());}
  #endif

  CompleterBase<MODE>::doclistVolumeDecompressed += (nofDocsInCompressedList*sizeof(DocId));

  #ifndef NDEBUG
  LOG << "! start decompressing positions ... " << flush;
  #endif

  //
  // D.2 Decompress list of positions
  //

  if (MODE & WITH_POS)
  {
    assert(nofPositionsInCompressedList == nofDocsInCompressedList);
    positionlist.resize(nofPositionsInCompressedList);
    // TODO: Explain what is checked here
    // HACK: changed last > to >= (without understanding what I am doing)
    if (!((offsetForPositionlist - offsetForDoclist -sizeof(unsigned long int)  > 0 )
	  && (*((unsigned long*) (CompleterBase<MODE>::_compressionBuffer + offsetForPositionlist
		 - offsetForDoclist - sizeof(unsigned long int))) >= nofPositionsInCompressedList)))
		 //- offsetForDoclist - sizeof(unsigned long int))) > nofPositionsInCompressedList)))
    {
      ostringstream os;
      os << "Problem uncompressing position list"
         << ": offPosList = "     << offsetForPositionlist
         << "; offDocList = "     << offsetForDoclist
         << "; *compBuff = "      << *(unsigned long*)(CompleterBase<MODE>::_compressionBuffer
                                        + offsetForPositionlist - offsetForDoclist - sizeof(unsigned long int))
         << "; nofPosCompList = " << nofPositionsInCompressedList;
      CS_THROW(Exception::UNCOMPRESS_ERROR, os.str());
    }
    assert(offsetForPositionlist - offsetForDoclist -sizeof(unsigned long int)  > 0);
    //assert((offsetForPositionlist - offsetForDoclist -sizeof(unsigned long int))/sizeof(DocId) < nofDocsInCompressedList);//assumes "proper" compression
    assert(*((unsigned long*) (CompleterBase<MODE>::_compressionBuffer + offsetForPositionlist - offsetForDoclist -sizeof(unsigned long int))) >= nofPositionsInCompressedList);
    CompleterBase<MODE>::positionlistDecompressionTimer.cont();
    _positionlistCompressionAlgorithm.decompress(CompleterBase<MODE>::_compressionBuffer + offsetForPositionlist - offsetForDoclist -sizeof(unsigned long int), &positionlist[0], nofPositionsInCompressedList,2);//2 indicates: gaps with artificial boundaries
    CompleterBase<MODE>::positionlistDecompressionTimer.stop();
    assert(positionlist.isContiguous());
    CompleterBase<MODE>::positionlistVolumeDecompressed += (nofPositionsInCompressedList*sizeof(Position));
  }
  #ifndef NDEBUG
  LOG << "!  ... done " << endl << flush;
  #endif

  //
  // D.3 Decompress list of word ids
  //

  #ifndef NDEBUG
  LOG << "! start decompressing words ... " << flush;
  #endif
  CompleterBase<MODE>::resizeAndReserveTimer.cont();
  wordlist.resize(nofWordsInCompressedList);
  CompleterBase<MODE>::resizeAndReserveTimer.stop();
  CompleterBase<MODE>::wordlistDecompressionTimer.cont();
  if (MODE & WITH_POS)
  {
    _wordlistCompressionAlgorithm.decompress(CompleterBase<MODE>::_compressionBuffer
                                               + offsetForWordlist
                                               - offsetForDoclist
                                               - 2*sizeof(unsigned long int),
                                             &wordlist[0],
                                             nofWordsInCompressedList);
  }
  else
  {
    _wordlistCompressionAlgorithm.decompress(CompleterBase<MODE>::_compressionBuffer
                                               + offsetForWordlist
                                               - offsetForDoclist
                                               - sizeof(unsigned long int),
                                             &wordlist[0],
                                             nofWordsInCompressedList);
  }
  #ifndef NDEBUG
  LOG << "!  ... done " << endl << flush;
  #endif
  assert(wordlist.isContiguous());
  CompleterBase<MODE>::wordlistDecompressionTimer.stop();
  #ifndef NDEBUG
  for(unsigned long j = 0; j< wordlist.size(); j++) {assert(wordlist[j] >= 0); assert(wordlist[j] <CompleterBase<MODE>::_metaInfo.getNofWords()); assert((blockId==0) || (wordlist[j]>0) );}
  #endif
  CompleterBase<MODE>::wordlistVolumeDecompressed += (nofDocsInCompressedList*sizeof(WORDID));
  // NEW 19Jan11 (Ina): Added log output, since uncompressing takes quite long right now. And we want to analyze what goes wrong.
  if (blockId < _boundaryWordIds.size())
  {
    WordId low = _boundaryWordIds[blockId];
    LOG << IF_VERBOSITY_HIGH << "! Decompressed block [\"" << CompleterBase<MODE>::_vocabulary->operator[](low);
    if (blockId + 1 < _boundaryWordIds.size())
    {
      WordId high = _boundaryWordIds[blockId+1];
      LOG << "\"-\"" << CompleterBase<MODE>::_vocabulary->operator[](high) << "\"]";
    }
    else LOG << "\"]";
    LOG << ", " << commaStr(nofWordsInCompressedList) << " index items, "
        << commaStr((nofDocsInCompressedList*sizeof(WORDID)) / 1024) << " KB (uncompressed)." << endl;
  }
}
//end: getDataForBlockId






// ********************
//
// BELOW HERE OLD STUFF
//
// ********************



//! PRINT DOC LIST LENGTHS FOR ALL BLOCKS
template<unsigned char MODE>
void HybCompleter<MODE>::printAllListLengths()
{
  for (unsigned long int i = 0; i < CompleterBase<MODE>::_metaInfo->getNofBlocks(); i++)
    printListLengthForBlockId((BlockId)(i));
}

//! PRINT DOC LIST LENGTHS FOR GIVEN BLOCK   (for debugging, still used?)
template<unsigned char MODE>
void HybCompleter<MODE>::printListLengthForBlockId(BlockId blockId)
 {
   DocList doclist;
   Vector<Position> positionlist;
   Vector<DiskScore> scorelist;
   WordList wordlist;
   // + 2 here as the last but one points to the boundary word IDs vector and the last to the meta info
   assert(_byteOffsetsForBlocks.size() > blockId+2);

   off_t offsetForDoclist, offsetForPositionlist=2, offsetForWordlist, offsetForScorelist=0;
   unsigned long int nofDocsInCompressedList;


  // READ OFFSETS FOR LISTS (currently not used) AND READ NOF DOCS IN COMPRESSED LIST
  // seek is only done once in the read below. All other reads are from the current position onwards
   // .... FOR DOCLIST
  CompleterBase<MODE>::_indexStructureFile.read(&offsetForDoclist,sizeof(off_t),_byteOffsetsForBlocks[blockId]);
  // .... FOR POSITIONLIST
  if(MODE & WITH_POS) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForPositionlist,sizeof(off_t));}
  // .... FOR WORDLIST
  CompleterBase<MODE>::_indexStructureFile.read(&offsetForWordlist,sizeof(off_t));
  // ...  FOR SCORELIST
  if(MODE & WITH_SCORES) { CompleterBase<MODE>::_indexStructureFile.read(&offsetForScorelist,sizeof(off_t));}
  if(!(MODE & WITH_POS)) { assert(offsetForPositionlist ==2);offsetForPositionlist=offsetForWordlist;}
  else {assert(offsetForWordlist > offsetForPositionlist); }

  // READ NOF DOCS IN LIST
  CompleterBase<MODE>::_indexStructureFile.read(&nofDocsInCompressedList,sizeof(unsigned long int));
  assert(nofDocsInCompressedList >0);
  LOG << nofDocsInCompressedList << endl;

 } //end: printListLengthForBlockId



//! GET DATA FOR ALL BLOCKS AND PRINT (for test purposes only)
template <unsigned char MODE>
void HybCompleter<MODE>::getAllBlocks()
{
  assert(CompleterBase<MODE>::_metaInfo.getNofBlocks() > 0);
  assert(CompleterBase<MODE>::_metaInfo.getNofBlocks() + 2 == _byteOffsetsForBlocks.size());
  DocList dummyDoclist;
  Vector<Position> dummyPositionlist;
  Vector<DiskScore> dummyScorelist;
  WordList dummyWordlist;

  unsigned long int totalNofPairs = 0;
  for(unsigned long i = 0; i < CompleterBase<MODE>::_metaInfo->getNofBlocks(); i++)
  {
    getDataForBlockId(i, dummyDoclist, dummyPositionlist, dummyScorelist, dummyWordlist);
    totalNofPairs += dummyDoclist.size();
    LOG << "\n doclist for block " << i << " : \n";
    dummyDoclist.show(); LOG << "\n";
    LOG << "\n wordlist for block " << i << " : \n";
    dummyWordlist.show(*CompleterBase<MODE>::_vocabulary); LOG << "\n";
    dummyDoclist.clear();
    dummyWordlist.clear();
  }
   LOG << "\n total nof Pairs : " << totalNofPairs << "\n";
}//end: getAllBlocks()


//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template class HybCompleter<WITH_SCORES + WITH_POS + WITH_DUPS>;

