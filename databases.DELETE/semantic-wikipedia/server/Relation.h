// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_RELATION_H_
#define SEMANTIC_WIKIPEDIA_SERVER_RELATION_H_

#include <vector>
#include <sstream>
#include <string>
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./List.h"

using std::vector;
using std::string;

namespace ad_semsearch
{
//! Class representing postings from a full-text index.
class RelationEntry
{
  public:

    RelationEntry()
    {
    }

    RelationEntry(Id lhs, Id rhs)
    : _lhs(lhs), _rhs(rhs)
    {
    }

    string asString()
    {
      std::ostringstream os;
      os << "(" << getPureValue(_lhs) << " - " << getPureValue(_rhs) << ")";
      return os.str();
    }

    Id _lhs;
    Id _rhs;
};

// List representing a relation as read from the ontology index
typedef List<RelationEntry>  Relation;
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_RELATION_H_
