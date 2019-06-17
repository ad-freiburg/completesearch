// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_POSTINGLIST_H_
#define SEMANTIC_WIKIPEDIA_SERVER_POSTINGLIST_H_

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
class TextPosting
{
  public:

    TextPosting()
    {
    }

    TextPosting(Id id, Id contextid, Score score, Position position)
    : _id(id), _contextId(contextid), _score(score), _position(position)
    {
    }

    string asString()
    {
      std::ostringstream os;
      os << "("
          << (isIdOfType(_id, IdType::ONTOLOGY_ELEMENT_ID) ? "EntityId: "
              : "WordId: ") << getPureValue(_id) << ", ContextId: "
          << _contextId << ", Score: " << static_cast<int> (_score)
          << ", Pos: " << static_cast<int> (_position)
          << ")";
      return os.str();
    }

    Id _id;
    Id _contextId;
    Score _score;
    Position _position;
};

// List representing a raw list of postings from the full-text index.
typedef List<TextPosting> PostingList;
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_POSTINGLIST_H_
