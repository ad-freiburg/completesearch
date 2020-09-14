// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_INTERMEDIATEQUERYRESULT_H_
#define SEMANTIC_WIKIPEDIA_SERVER_INTERMEDIATEQUERYRESULT_H_

#include <string>
#include <sstream>
#include "./EntityList.h"
#include "./PostingList.h"
#include "./Relation.h"

#include "../codebase/semantic-wikipedia-utils/HashSet.h"

using std::string;

namespace ad_semsearch
{
class IntermediateQueryResult
{
  public:

    enum Status
    {
     FINISHED = 0,
     OTHER = 1
    };

    IntermediateQueryResult()
    :_entities(), _postings(), _matchingRelationEntries(),
     _subtreeEntities(), _status(OTHER)
    {
    }

    EntityList _entities;
    PostingList _postings;
    Relation _matchingRelationEntries;
    ad_utility::HashSet<Id> _subtreeEntities;
    Status _status;

    string asString() const
    {
      std::ostringstream oss;
      oss << "Query Result with " << _entities.size() << " elements: "
          << _entities.asString() << ". Status is "
          << (_status == FINISHED ? "FINISHED." : "not finished.");
      return oss.str();
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_INTERMEDIATEQUERYRESULT_H_
