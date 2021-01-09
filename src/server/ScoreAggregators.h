#ifndef __SCORE_AGGREGATORS_H__
#define __SCORE_AGGREGATORS_H__

#include "Globals.h"

#define TITLE_MATCH_BONUS 64
#define EXACT_MATCH_BONUS 4 //mehl* -> mehl$
#define PHRASE_MATCH_BONUS 16
#define NEAR_MATCH_BONUS 8
// This Macro is also used: 1. for the first word  and 2. for filtering
#define HAS_EXACT_MATCH_BONUS(x) ((x) == (true) ? (EXACT_MATCH_BONUS) : (0) ) // bonus for exact word. x is a bool

//! Types of score aggregations, as simple integer ids
/*!
 *   NEW 06Apr08 (Holger): have ids for the various score aggregators, so that
 *   they can be chosen at runtim. Translation to ScoreAggregation objects (TODO:
 *   rename to ScoreAggregator) is done in new wrapper funtions in Intersect.h/cpp
 *   resp. IntersectOld.h/cpp
 *
 *   Note: NONE is new and stands for no aggregation at all, e.g. in faceted
 *   search, when the last query word is venue:* and we are not interested in
 *   the hits but just the completions.
 */
enum ScoreAggregation { SCORE_AGG_NONE, SCORE_AGG_SUM, SCORE_AGG_MAX, SCORE_AGG_SUM_WITH_BONUS };

//! Compute the aggregate of two scores x and y as x + y (used as template argument in intersect)
class SumAggregation
{
 public:
  template<class T, class S>
    inline T aggregate(const T& arg1, const S& arg2) const
    {
      assert(sizeof(T) >= sizeof(S));
      return (T) arg1 + (T) arg2;
    }

  // The third parameter is the distance between the two positions (candidatePositions[ii]-positionlist[jj])
  template<class T, class S, class U>
    inline T aggregate(const T& arg1, const S& arg2, const U& arg3) const
    {
      assert(sizeof(T) >= sizeof(S));
      return (T) arg1 + (T) arg2;
    }

  // The third parameter is the distance between the two positions (candidatePositions[ii]-positionlist[jj])
  // fourth parameter (bool) is true if the word is the first in the desired range
  template<class T, class S, class U>
    inline T aggregate(const T& arg1, const S& arg2, const U& arg3, const bool& arg4) const
    {
      assert(sizeof(T) >= sizeof(S));
      return (T) arg1 + (T) arg2;
    }
};


//! Compute the aggregate of two scores x and y as max{x, y} (used as template argument in intersect)
class MaxAggregation
{
 public:
  template<class T, class S>
    inline T aggregate(const T& arg1, const S& arg2) const
    {
      assert(sizeof(T) >= sizeof(S));
      return (T) MAX(arg1,(T) arg2);
    }

  // The third parameter is the distance between the two positions (candidatePositions[ii]-positionlist[jj])
  template<class T, class S, class U>
    inline T aggregate(const T& arg1, const S& arg2, const U& arg3) const
    {
      assert(sizeof(T) >= sizeof(S));
      return (T) MAX(arg1,(T) arg2);
    }

  // The third parameter is the distance between the two positions (candidatePositions[ii]-positionlist[jj])
  // fourth parameter (bool) is true if the word is the first in the desired range
  template<class T, class S, class U>
    inline T aggregate(const T& arg1, const S& arg2, const U& arg3, const bool& arg4) const
    {
      assert(sizeof(T) >= sizeof(S));
      return (T) MAX(arg1,(T) arg2);
    }

};


//! Compute the aggregate of two scores x and y as x + y plus a proximity / exact word match bonus (used as template argument in intersect)
/*!
 *    Currently three kind of boni are considered
 *
 *    1. The two words occur right next to each other               => add PHRASE_MATCH_BONUS
 *    2. The two words occur within NEIGHBORHOOD_SIZE of each other => add NEAR_MATCH_BONUS
 *    3. The second word is an exact word match                     => add HAS_EXACT_MATCH_BONUS
 *
 *    If 1. applies, 2. is not considered. 3. may be combined with both 1. and 2.
 */
class SumProxAggregation
{
 public:
  template<class T, class S>
    inline T aggregate(const T& arg1, const S& arg2) const
    {      
      return  ((T) arg1 + (T) arg2 ) ;
    }

  // The third parameter is the distance between the two positions (candidatePositions[ii]-positionlist[jj])
  template<class T, class S, class U>
    inline T aggregate(const T& arg1, const S& arg2, const U& arg3) const
    {
      if (arg3 == (U) - 1) return (T) arg1 + (T) arg2 + PHRASE_MATCH_BONUS;
      else                 return (T) arg1 + (T) arg2 + ( ABS(arg3) < NEIGHBORHOOD_SIZE ?  NEAR_MATCH_BONUS : 0 );
    }

  // The third parameter is the distance between the two positions (candidatePositions[ii]-positionlist[jj])
  // fourth parameter (bool) is true if the word is the first in the desired range
  // !! Also search for HAS_EXACT_MATCH_BONUS in CompleterBase. Scores are also changed ..
  //  ... for the very first word
  //  ... for filtering
  template<class T, class S, class U>
    inline T aggregate(const T& arg1, const S& arg2, const U& arg3, const bool& arg4) const
    {
      if (arg3 == (U) - 1) return (T) arg1 + (T) arg2 + HAS_EXACT_MATCH_BONUS(arg4) + PHRASE_MATCH_BONUS;
      else                 return (T) arg1 + (T) arg2 + HAS_EXACT_MATCH_BONUS(arg4) + ( ABS(arg3) < NEIGHBORHOOD_SIZE ?  NEAR_MATCH_BONUS : 0 );
    }
};



#endif
