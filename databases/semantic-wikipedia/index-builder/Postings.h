// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_POSTINGS_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_POSTINGS_H_

#include <string>
#include <sstream>
#include <limits>
#include "../server/Identifiers.h"

using std::string;

namespace ad_semsearch
{
class BasicPosting
{
  public:
    BasicPosting()
    {
    }

    BasicPosting(Id block, Id id, Id contextId) :
      _blockId(block), _wordId(id), _contextId(contextId)
    {
    }

    virtual ~BasicPosting()
    {
    }

    Id _blockId;
    Id _wordId;
    Id _contextId;

    static BasicPosting min_value()
    {
      BasicPosting min;
      min._blockId = min._wordId = min._contextId
          = std::numeric_limits<Id>::min();
      return min;
    }

    static BasicPosting max_value()
    {
      BasicPosting max;
      max._blockId = max._wordId = max._contextId
          = std::numeric_limits<Id>::max();
      return max;
    }

    virtual bool operator==(const BasicPosting& other) const
    {
      return _blockId == other._blockId && _wordId == other._wordId
          && _contextId == other._contextId;
    }

    //! Non-virtual, should be overwritten in subclasses instead.
    string asString()
    {
      std::ostringstream os;
      os << "Basic Posting" << " - ";
      if (isIdOfType(_blockId, IdType::ONTOLOGY_ELEMENT_ID))
      {
        os << "Entity-Block Id: " << getPureValue(_blockId);
      }
      else
      {
        os << "Block Id: " << getPureValue(_blockId);
      }
      if (isIdOfType(_wordId, IdType::ONTOLOGY_ELEMENT_ID))
      {
        os << ", Entity-Word Id: " << getPureValue(_wordId);
      }
      else
      {
        os << ", Word Id: " << getPureValue(_wordId);
      }
      os << ", Context Id: " << _contextId;
      return os.str();
    }
};

class Posting: public BasicPosting
{
  public:

    Posting()
    {
    }

    Posting(Id block, Id id, Id contextId) :
      BasicPosting(block, id , contextId), _score(0), _pos(0)
    {
    }

    Posting(Id block, Id id, Id contextId, Score score, Id pos) :
      BasicPosting(block, id , contextId), _score(score), _pos(pos)
    {
    }

    static Posting min_value()
    {
      Posting min;
      min._blockId = min._wordId = min._contextId = min._score = min._pos
          = std::numeric_limits<Id>::min();
      return min;
    }

    static Posting max_value()
    {
      Posting max;
      max._blockId = max._wordId = max._contextId = max._score = max._pos
          = std::numeric_limits<Position>::max();
      return max;
    }

    Score _score;
    Position _pos;

    virtual bool operator==(const Posting& other) const
    {
      return _blockId == other._blockId && _wordId == other._wordId
          && _contextId == other._contextId && _score == other._score && _pos
          == other._pos;
    }

    //! Non-virtual, should be overwritten in subclasses instead.
    string asString()
    {
      std::ostringstream os;
      os << "Posting" << " - ";
      if (isIdOfType(_blockId, IdType::ONTOLOGY_ELEMENT_ID))
      {
        os << "Entity-Block Id: " << getPureValue(_blockId);
      }
      else
      {
        os << "Block Id: " << getPureValue(_blockId);
      }
      if (isIdOfType(_wordId, IdType::ONTOLOGY_ELEMENT_ID))
      {
        os << ", Entity-Word Id: " << getPureValue(_wordId);
      }
      else
      {
        os << ", Word Id: " << getPureValue(_wordId);
      }
      os << ", Context Id: " << _contextId;
      os << ", Score: " << static_cast<int>(_score);
      os << ", Pos: " << _pos;
      return os.str();
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_POSTINGS_H_
