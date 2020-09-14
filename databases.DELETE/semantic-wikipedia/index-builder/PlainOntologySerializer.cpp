// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <assert.h>
#include <stxxl/vector>
#include <string>
#include <vector>
#include "../server/Identifiers.h"
#include "../server/IndexMetaData.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./PlainOntologySerializer.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
// _____________________________________________________________________________
void PlainOntologySerializer::serialize(
    const stxxl::vector<RelationFact>& facts,
    vector<RelationMetaData>& metaData,
    const ad_utility::HashSet<Id>& relationsToBeSplitIntoBlocksByLhs,
    const string& outputFileName)
{
  assert(facts.size() > 0);

  ad_utility::File file(outputFileName.c_str(), "w");

  // Collect MetaData always while writing a relation and add it to the vector
  // when the relation is finished.
  _currentOffset = 0;
  _metaData.clear();

  Id currentRelation = facts[0]._relationId;
  TupleIterator begin = facts.begin();
  size_t relNum = 0;
  for (TupleIterator current = facts.begin();
      current != facts.end(); ++current)
  {
    if (current->_relationId != currentRelation)
    {
      bool splitRealtion = relationsToBeSplitIntoBlocksByLhs.count(
          metaData[relNum]._relationId);
      serializeRelation(facts, metaData[relNum], splitRealtion, begin,
          current, &file);
      ++relNum;
      currentRelation = current->_relationId;
      begin = current;
    }
  }
  // serialize the last Relation
  bool splitRealtion = relationsToBeSplitIntoBlocksByLhs.count(
      metaData[relNum]._relationId);
  serializeRelation(facts, metaData[relNum], splitRealtion, begin,
      facts.end(), &file);

  // Write MetaData & finish
  off_t startOfMeta = _currentOffset;
  for (size_t i = 0; i < _metaData.size(); ++i)
  {
    const RelationMetaData& meta = _metaData[i];
    off_t nextRelationMetaDataOffset = _currentOffset + sizeof(off_t) + 3
        * sizeof(Id) + (meta._blockInfo.size() * (sizeof(Id) + sizeof(size_t)
        + 3 * sizeof(off_t)));

    file.write(&nextRelationMetaDataOffset, sizeof(off_t));
    _currentOffset += sizeof(off_t);
    file.write(&meta._relationId, sizeof(Id));
    file.write(&meta._lhsType, sizeof(Id));
    file.write(&meta._rhsType, sizeof(Id));
    _currentOffset += 3 * sizeof(Id);
    for (size_t j = 0; j < meta._blockInfo.size(); ++j)
    {
      const RelationBlockMetaData& blockMeta = meta._blockInfo[j];
      file.write(&blockMeta._maxLhs, sizeof(Id));
      file.write(&blockMeta._nofElements, sizeof(size_t));
      file.write(&blockMeta._startOfLhsData, sizeof(off_t));
      file.write(&blockMeta._startOfRhsData, sizeof(off_t));
      file.write(&blockMeta._posOfLastRhs, sizeof(off_t));
      _currentOffset += sizeof(Id) + sizeof(size_t) + 3 * sizeof(off_t);
    }
  }
  file.write(&startOfMeta, sizeof(off_t));
}
// _____________________________________________________________________________
void PlainOntologySerializer::serializeRelation(
    const stxxl::vector<RelationFact>& facts,
    RelationMetaData& relationMetaData, bool splitRealtionByLhs,
    const TupleIterator& begin,
    const TupleIterator& end, ad_utility::File* file)
{
  assert(end - begin > 0);
  assert(relationMetaData._relationId == begin->_relationId);
  _currentRelationBlockMetaData.clear();

  if (splitRealtionByLhs)
  {
    TupleIterator blockStart = begin;
    TupleIterator blockEnd = begin;
    while (blockStart != end)
    {
      while (blockEnd->_lhs == blockStart->_lhs)
      {
        ++blockEnd;
      }
      serializeBlock(facts, blockStart, blockEnd, file);
      blockStart = blockEnd;
    }
  } else
  {
    serializeBlock(facts, begin, end, file);
  }
  relationMetaData._blockInfo = _currentRelationBlockMetaData;
  _metaData.push_back(relationMetaData);
}
// _____________________________________________________________________________
void PlainOntologySerializer::serializeBlock(
    const stxxl::vector<RelationFact>& facts,
    const TupleIterator& begin, const TupleIterator& end,
    ad_utility::File* file)
{
  assert(end - begin > 0);
  RelationBlockMetaData meta;

  meta._nofElements = end - begin;
  meta._maxLhs = (end - 1)->_lhs;

  // Write Lhs list
  meta._startOfLhsData = _currentOffset;
  writeLhsList(facts, begin, end, file);

  // Write Rhs list
  meta._startOfRhsData = _currentOffset;
  writeRhsList(facts, begin, end, file);
  meta._posOfLastRhs = _currentOffset - sizeof(Id);

  // Add metaData
  _currentRelationBlockMetaData.push_back(meta);
}
// _____________________________________________________________________________
void PlainOntologySerializer::writeLhsList(
    const stxxl::vector<RelationFact>& facts, const TupleIterator& begin,
    const TupleIterator& end, ad_utility::File* file)
{
  for (TupleIterator it = begin; it < end; ++it)
  {
    file->write(&it->_lhs, sizeof(Id));
  }
  _currentOffset += (end - begin) * sizeof(Id);
}
// _____________________________________________________________________________
void PlainOntologySerializer::writeRhsList(
    const stxxl::vector<RelationFact>& facts, const TupleIterator& begin,
    const TupleIterator& end, ad_utility::File* file)
{
  for (TupleIterator it = begin; it < end; ++it)
  {
    file->write(&it->_rhs, sizeof(Id));
  }
  _currentOffset += (end - begin) * sizeof(Id);
}
}

