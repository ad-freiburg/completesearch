// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_FACTS_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_FACTS_H_

#include <limits>
#include "../server/Identifiers.h"
#include "../server/IndexMetaData.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"

namespace ad_semsearch
{
class RelationFact
{
  public:
    Id _relationId;
    Id _lhs;
    Id _rhs;

    RelationFact()
    {
    }

    RelationFact(Id relationId, Id lhs, Id rhs)
    : _relationId(relationId), _lhs(lhs), _rhs(rhs)
    {
    }

    static RelationFact min_value()
    {
      RelationFact min;
      min._relationId = min._lhs = min._rhs = std::numeric_limits<Id>::min();
      return min;
    }

    static RelationFact max_value()
    {
      RelationFact max;
      max._relationId = max._lhs = max._rhs = std::numeric_limits<Id>::max();
      return max;
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_FACTS_H_
