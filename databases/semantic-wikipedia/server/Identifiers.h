// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_IDENTIFIERS_H_
#define SEMANTIC_WIKIPEDIA_SERVER_IDENTIFIERS_H_

#include "../codebase/semantic-wikipedia-utils/Globals.h"

namespace ad_semsearch
{
//! Available types of IDs. The semantic search processes lists of
//! arbitrary IDs. IDs are represented as 64bit integers.
//! However, sometimes ontology elements (i.e. entities) have to
//! be distinguished from words although they occur in the same list.
//! Therefore the most significant bits are reserved as type-flags.
class IdType
{
  public:
    enum TYPE
    {
      WORD_ID, ONTOLOGY_ELEMENT_ID, CONTEXT_ID
    };
};

//! Gets the Id "0". In fact, numbers greater than 0 may be returned
//! because the most significant bits represent the type of the ID.
inline Id getFirstId(IdType::TYPE type)
{
  switch (type)
  {
  case IdType::WORD_ID:
    return 0;
  case IdType::ONTOLOGY_ELEMENT_ID:
    return (Id(1) << ((sizeof(Id) * 8) - 1));
  case IdType::CONTEXT_ID:
    // Currently, context IDs can always be distinguished from
    // other IDs from the context they occur in. May be changed in
    // the future, though;
    return 0;
  default:
    return 0;
  }
}

//! Check whether a given id is of the specified type.
inline bool isIdOfType(Id id, IdType::TYPE type)
{
  switch (type)
  {
  case IdType::WORD_ID:
    return (id & (Id(1) << ((sizeof(Id) * 8) - 1))) == 0;
  case IdType::ONTOLOGY_ELEMENT_ID:
    return (id & (Id(1) << ((sizeof(Id) * 8) - 1))) > 0;
  case IdType::CONTEXT_ID:
    return (id & (Id(1) << ((sizeof(Id) * 8) - 1))) == 0;
  default:
    return false;
  }
}

//! Get the pure value of the Id without any flag bits set.
inline Id getPureValue(Id id)
{
  return id & (~(Id(1) << ((sizeof(Id) * 8) - 1)));
}
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_IDENTIFIERS_H_
