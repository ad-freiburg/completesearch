#ifndef __INTERSECT_H__
#define __INTERSECT_H__

#include "QueryResult.h"
#include "WordRange.h"
#include "ScoreAggregators.h"
#include "Separator.h"

//! Intersect two posting lists  NEW March 2008 by Holger
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
void intersectTwoPostingLists(const QueryResult&     input1,
                              const QueryResult&     input2,
                                    QueryResult&     result,
                              const Separator&       separator,    
                              const WordRange&       wordIdRange       = infiniteWordIdRange,
                              const ScoreAggregation scoreAggregation  = SCORE_AGG_SUM);


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
                                                const ScoreAggregator& aggregateScores);

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
                                                const ScoreAggregator& aggregateScores);



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
                                                const unsigned int     outputMode);


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
                                       const ScoreAggregator& aggregateScores);


//! Check the result of an intersection (incomplete and deprecated)
void intersectCheck(const QueryResult& input1,
                    const QueryResult& input2,
                    const QueryResult& result);

//! Check equality of two posting lists (used for testing only)
void checkEquality(const QueryResult& input1,
                   const QueryResult& input2);

#endif
