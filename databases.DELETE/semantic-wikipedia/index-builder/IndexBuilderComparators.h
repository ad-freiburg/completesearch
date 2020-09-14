// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_INDEXBUILDERCOMPARATORS_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_INDEXBUILDERCOMPARATORS_H_

#include "../server/Identifiers.h"
#include "./Facts.h"
#include "./Postings.h"

namespace ad_semsearch
{
//! Comparator for postings. Assumes that the postings type
//! has fields _block, _wordId and _contextId just like a basic posting.
//! In each block, ontology elements will be listed in the back.
template<class PostingType>
class PostingComparatorEntitiesLast
{
  public:
    inline bool
        operator()(const PostingType& lhs, const PostingType& rhs) const;

    static PostingType min_value()
    {
      return PostingType::min_value();
    }

    static PostingType max_value()
    {
      return PostingType::max_value();
    }
};

//! Comparator for postings. Assumes that the postings type
//! has fields _block, _wordId and _contextId just like a basic posting.
//! Ontology elements are mixed with normal words inside the blocks.
template<class PostingType>
class PostingComparatorEntitiesMixedIn
{
  public:
    inline bool
        operator()(const PostingType& lhs, const PostingType& rhs) const;

    static PostingType min_value()
    {
      return PostingType::min_value();
    }

    static PostingType max_value()
    {
      return PostingType::max_value();
    }
};

//! Comparator for postings.
//! Compares context -> id -> pos -> score by value.
class DefaultPostingComparator
{
  public:
    inline bool
        operator()(const Posting& lhs, const Posting& rhs) const;

    static Posting min_value()
    {
      return Posting::min_value();
    }

    static Posting max_value()
    {
      return Posting::max_value();
    }
};

//! Comparator for ontology facts.
//! Achieves an ordering: relationId > lhs > rhs.
//! Any fact is expected to have fields
//! _relationId _lhs, and _rhs. Additional fields like scores for reliability
//! or whatever might be added in the future, are ignored. If there
//! every should be something that has to be considered in a comparison,
//! a specialization will have to be made.
template<class FactType>
class FactsComp
{
  public:
    virtual ~FactsComp()
    {
    }

    virtual inline bool operator()(const FactType& first,
        const FactType& second) const;

    static FactType min_value()
    {
      return FactType::min_value();
    }

    static FactType max_value()
    {
      return FactType::max_value();
    }
};
template<class FactType>
class OrderFactsByLhsComparator : public FactsComp<FactType>
{
  public:
    virtual inline bool operator()(const FactType& first,
        const FactType& second) const;

    virtual ~OrderFactsByLhsComparator()
    {
    }
};
class RelationMetaDataComp
{
  public:

    bool operator()(const RelationMetaData& lhs,
        const RelationMetaData& rhs) const
    {
      return lhs._relationId < rhs._relationId;
    }
};
//! Definitions of longer in-lined methods:
// _____________________________________________________________________________
template<class PostingType>
bool PostingComparatorEntitiesLast<PostingType>::operator()(
    const PostingType& lhs, const PostingType& rhs) const
{
  if (lhs._blockId == rhs._blockId)
  {
    if (isIdOfType(lhs._wordId, IdType::ONTOLOGY_ELEMENT_ID)
        == isIdOfType(rhs._wordId, IdType::ONTOLOGY_ELEMENT_ID))
    {
      if (lhs._contextId == rhs._contextId)
      {
        return lhs._wordId < rhs._wordId;
      }
      else
      {
        return lhs._contextId < rhs._contextId;
      }
    }
    else
    {
      return !isIdOfType(lhs._wordId, IdType::ONTOLOGY_ELEMENT_ID);
    }
  }
  else
  {
    return lhs._blockId < rhs._blockId;
  }
}
// _____________________________________________________________________________
template<class PostingType>
bool PostingComparatorEntitiesMixedIn<PostingType>::operator()(
    const PostingType& lhs, const PostingType& rhs) const
{
  if (lhs._blockId == rhs._blockId)
  {
    if (lhs._contextId == rhs._contextId)
    {
      return getPureValue(lhs._wordId) < getPureValue(rhs._wordId);
    }
    else
    {
      return lhs._contextId < rhs._contextId;
    }
  }
  else
  {
    return lhs._blockId < rhs._blockId;
  }
}
// _____________________________________________________________________________
bool DefaultPostingComparator::operator()(const Posting& lhs,
    const Posting& rhs) const
{
  if (lhs._blockId == rhs._blockId)
  {
    if (lhs._contextId == rhs._contextId)
    {
      if (lhs._wordId == rhs._wordId)
      {
        if (lhs._pos == rhs._pos)
        {
          return lhs._score < rhs._score;
        }
        else
        {
          return lhs._pos < rhs._pos;
        }
      }
      else
      {
        return lhs._wordId < rhs._wordId;
      }
    }
    else
    {
      return lhs._contextId < rhs._contextId;
    }
  }
  else
  {
    return lhs._blockId < rhs._blockId;
  }
}
// _____________________________________________________________________________
template<class FactType>
bool FactsComp<FactType>::operator()(const FactType& lhs,
    const FactType& rhs) const
{
  if (lhs._relationId == rhs._relationId)
  {
    if (lhs._lhs == rhs._lhs)
    {
      return lhs._rhs < rhs._rhs;
    }
    else
    {
      return lhs._lhs < rhs._lhs;
    }
  }
  else
  {
    return lhs._relationId < rhs._relationId;
  }
}
// _____________________________________________________________________________
template<class FactType>
bool OrderFactsByLhsComparator<FactType>::operator()(
    const FactType& lhs, const FactType& rhs) const
{
  return lhs._lhs < rhs._lhs;
}
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_INDEXBUILDERCOMPARATORS_H_
