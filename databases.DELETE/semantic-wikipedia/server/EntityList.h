// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_ENTITYLIST_H_
#define SEMANTIC_WIKIPEDIA_SERVER_ENTITYLIST_H_

#include <vector>
#include <string>
#include <sstream>
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./List.h"
#include "./Identifiers.h"

using std::vector;
using std::string;

namespace ad_semsearch
{
//! List Element.
//! An entity represented by its ID with a score that usually
//! results from aggregating several entity postings with scores.
class EntityWithScore
{
  public:
    EntityWithScore()
    {
    }

    EntityWithScore(Id id, AggregatedScore score) :
      _id(id), _score(score)
    {
    }

    ~EntityWithScore()
    {
    }

    string asString()
    {
      std::ostringstream os;
      os << "(EntityId: " << getPureValue(_id) << ", Score: "
          << static_cast<int> (_score) << ")";
      return os.str();
    }

    bool operator<(const EntityWithScore& other) const
    {
      return _id < other._id;
    }

    const AggregatedScore& getScore() const
    {
      return _score;
    }

    Id _id;
    AggregatedScore _score;
};

// List representing a list of entities like it is used
// as (intermediate) query result in many places.
// The list is well-formed iff entities are unique and
// ordered.
class EntityList: public List<EntityWithScore>
{
  public:

    EntityList()
    {
    }

    ~EntityList()
    {
    }

    bool isWellFormed()
    {
      bool wellFormed = true;
      if (size() > 0)
      {
        Id last = List<EntityWithScore>::operator[](0)._id;
        for (size_t i = 1; i < size(); ++i)
        {
          wellFormed = wellFormed && last
              < List<EntityWithScore>::operator[](i)._id;
          last = List<EntityWithScore>::operator[](i)._id;
        }
      }
      return wellFormed;
    }

    //! Check if the results conatins a certain entity.
    bool contains(Id entityId) const
    {
      if (_data.size() == 0)
      {
        return false;
      }
      // TODO(buchholb): make use of the fact that entity lists are sorted.
      LOG(WARN)
          << "Using trivial implementation in EntityList::contains()."
              " Switch to binary or exp search!"
          << std::endl;
      size_t i = 0;
      while (_data[i]._id < entityId)
      {
        ++i;
      }
      return _data[i]._id == entityId;
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_ENTITYLIST_H_
