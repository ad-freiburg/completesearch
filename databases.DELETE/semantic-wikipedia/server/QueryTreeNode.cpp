// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <sstream>
#include <string>
#include <set>
#include "./QueryTreeNode.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"

using std::string;
using ad_utility::AsStringComparatorLt;
using ad_utility::AsStringPtrComparatorLt;

namespace ad_semsearch
{
// _____________________________________________________________________________
void QueryTreeNode::computeResult(IntermediateQueryResult* result) const
{
  AD_CHECK_GT(_disjuncts.size(), 0);
  LOG(DEBUG) << "Processing subtree " << asString() << endl;
  if (_disjuncts.size() > 1)
  {
    AD_THROW(Exception::NOT_YET_IMPLEMENTED, "Disjuncts not supported yet.");
  }
  *result = _disjuncts.begin()->getResult();
}

// _____________________________________________________________________________
string QueryTreeNode::asString() const
{
  std::ostringstream os;
  os << "<NODE";
  for (set<Disjunct, AsStringComparatorLt>::const_iterator it =
      _disjuncts.begin(); it != _disjuncts.end(); ++it)
  {
    os << ' ' << it->asString();
  }
  os << '>';
  return os.str();
}

// _____________________________________________________________________________
void QueryTreeNode::getHitsForEntity(Id entityId, HitList* result) const
{
  for (set<Disjunct, ad_utility::AsStringComparatorLt>::const_iterator it =
      _disjuncts.begin(); it != _disjuncts.end(); ++it)
  {
    if (it->getResult()._entities.contains(entityId))
    {
      it->getHitsForEntity(entityId, result);
      return;
    }
  }
}
}
