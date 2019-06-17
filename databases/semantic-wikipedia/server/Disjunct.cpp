// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <sstream>
#include "./Disjunct.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"

using std::string;
using std::vector;
using std::set;
using ad_utility::AsStringComparatorLt;
using ad_utility::AsStringPtrComparatorLt;

namespace ad_semsearch
{
// _____________________________________________________________________________
void Disjunct::computeResult(IntermediateQueryResult* result) const
{
  LOG(DEBUG) << "Processing subtree " << asString() << endl;
  size_t nofTriples = _isaConjuncts.size() + _owConjuncts.size()
      + _relConjuncts.size() + _eqConjuncts.size();
  AD_CHECK_GT(nofTriples, 0);

  // Check if this disjunct is a trivial conjunction of 1 element.
  if (nofTriples == 1)
  {
    if (_eqConjuncts.size() == 1)
    {
      *result = _eqConjuncts.begin()->getResult();
    }
    if (_isaConjuncts.size() == 1)
    {
      *result = _isaConjuncts.begin()->getResult();
    }
    if (_relConjuncts.size() == 1)
    {
      *result = _relConjuncts.begin()->getResult();
    }
    if (_owConjuncts.size() == 1)
    {
      *result = _owConjuncts.begin()->getResult();
    }
    return;
  }

  // Find an ordering. Good: start with the smallest, intersecting with
  // small ones is nice.
  // TODO(buchholb): Heuristic to determine actual intersection sizes.
  // or verify that any gains are negligible.
  LOG(DEBUG)
      << "Creating vector of the results of all " << nofTriples << " triples"
      << endl;
  vector<const EntityList*> allResults;
  allResults.reserve(nofTriples);
  for (set<IsATriple, ad_utility::AsStringComparatorLt>::const_iterator
      it = _isaConjuncts.begin(); it != _isaConjuncts.end(); ++it)
  {
    allResults.push_back(&it->getResult()._entities);
  }
  for (set<OccursWithTriple, ad_utility::AsStringComparatorLt>::const_iterator
      it = _owConjuncts.begin(); it != _owConjuncts.end(); ++it)
  {
    allResults.push_back(&it->getResult()._entities);
  }
  for (set<RelationTriple, ad_utility::AsStringComparatorLt>::const_iterator
      it = _relConjuncts.begin(); it != _relConjuncts.end(); ++it)
  {
    allResults.push_back(&it->getResult()._entities);
  }
  for (set<EqualsTriple, ad_utility::AsStringComparatorLt>::const_iterator
      it = _eqConjuncts.begin(); it != _eqConjuncts.end(); ++it)
  {
    allResults.push_back(&it->getResult()._entities);
  }
  // Sort by result size. Lowest first.
  std::sort(allResults.begin(), allResults.end(),
      ad_utility::SizePtrComparatorLt());

  AD_CHECK(allResults.size() >= 2);
  LOG(DEBUG)
      << "Intersecting " << allResults.size() << " subtree results..." << endl;
  getEngine().intersectEntityLists(*allResults[0], *allResults[1],
      &result->_entities);
  for (size_t i = 2; i < allResults.size(); ++i)
  {
    EntityList tmp;
    getEngine().intersectEntityLists(result->_entities, *allResults[i], &tmp);
    result->_entities = tmp;
  }
  result->_status = IntermediateQueryResult::FINISHED;
  LOG(DEBUG)
      << "Intersection done. Disjunct result has size: "
      << result->_entities.size() << endl;
}

// _____________________________________________________________________________
void Disjunct::getHitsForEntity(Id entityId, HitList* result) const
{
  for (set<IsATriple, ad_utility::AsStringComparatorLt>::const_iterator it =
      _isaConjuncts.begin(); it != _isaConjuncts.end(); ++it)
  {
    HitList childHits;
    it->getHitsForEntity(entityId, &childHits);
    for (size_t i = 0; i < childHits.size(); ++i)
    {
      result->push_back(childHits[i]);
    }
  }
  for (set<OccursWithTriple, AsStringComparatorLt>::const_iterator it =
      _owConjuncts.begin(); it != _owConjuncts.end(); ++it)
  {
    HitList childHits;
    it->getHitsForEntity(entityId, &childHits);
    for (size_t i = 0; i < childHits.size(); ++i)
    {
      result->push_back(childHits[i]);
    }
  }
  for (set<RelationTriple, AsStringComparatorLt>::const_iterator it =
      _relConjuncts.begin(); it != _relConjuncts.end(); ++it)
  {
    HitList childHits;
    it->getHitsForEntity(entityId, &childHits);
    for (size_t i = 0; i < childHits.size(); ++i)
    {
      result->push_back(childHits[i]);
    }
  }
  for (set<EqualsTriple, AsStringComparatorLt>::const_iterator it =
      _eqConjuncts.begin(); it != _eqConjuncts.end(); ++it)
  {
    HitList childHits;
    it->getHitsForEntity(entityId, &childHits);
    for (size_t i = 0; i < childHits.size(); ++i)
    {
      result->push_back(childHits[i]);
    }
  }
}

// _____________________________________________________________________________
string Disjunct::asString() const
{
  std::ostringstream os;
  os << "<D";
  for (set<IsATriple, ad_utility::AsStringComparatorLt>::const_iterator it =
      _isaConjuncts.begin(); it != _isaConjuncts.end(); ++it)
  {
    os << ' ' << it->asString();
  }
  for (set<OccursWithTriple, AsStringComparatorLt>::const_iterator it =
      _owConjuncts.begin(); it != _owConjuncts.end(); ++it)
  {
    os << ' ' << it->asString();
  }
  for (set<RelationTriple, AsStringComparatorLt>::const_iterator it =
      _relConjuncts.begin(); it != _relConjuncts.end(); ++it)
  {
    os << ' ' << it->asString();
  }
  for (set<EqualsTriple, AsStringComparatorLt>::const_iterator it =
      _eqConjuncts.begin(); it != _eqConjuncts.end(); ++it)
  {
    os << ' ' << it->asString();
  }
  os << ">";
  return os.str();
}
}
