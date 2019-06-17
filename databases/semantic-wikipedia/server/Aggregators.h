// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_AGGREGATORS_H_
#define SEMANTIC_WIKIPEDIA_SERVER_AGGREGATORS_H_

#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./Identifiers.h"

namespace ad_semsearch
{
template<class Numerical>
class SumAggregator
{
  public:
    Numerical operator()(Numerical one, Numerical two) const
    {
      return one + two;
    }
};
template<class Numerical>
class PlusOneAggregator
{
  public:
    Numerical operator()(Numerical one, Numerical two) const
    {
      return one + Numerical(1);
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_AGGREGATORS_H_
