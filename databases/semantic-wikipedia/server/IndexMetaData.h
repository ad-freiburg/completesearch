// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_INDEXMETADATA_H_
#define SEMANTIC_WIKIPEDIA_SERVER_INDEXMETADATA_H_

#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Exception.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/Comparators.h"
#include "./List.h"
#include "./Identifiers.h"

using std::vector;
using std::string;
using ad_utility::File;
using ad_utility::HashMap;

namespace ad_semsearch
{
//! Meta Data written for each block.
//! This kind of data is located at the end of an index file
//! and provides all information necessary for deciding
//! which parts of the file to read in order to restore a block.
//! See comments of FulltextMetaData for further info on the layout
//! of this meta data when written as binary index file.
class BlockMetaData
{
  public:
    Id _maxWordId;
    size_t _nofPostings;

    off_t _startOfWordList;
    off_t _startOfContextList;
    off_t _startOfScoreList;
    off_t _startOfPositionList;
    off_t _posOfLastPosition;

    bool operator==(const BlockMetaData& other) const
    {
      return _maxWordId == other._maxWordId && _nofPostings
          == other._nofPostings && _startOfWordList == other._startOfWordList
          && _startOfContextList == other._startOfContextList
          && _startOfScoreList == other._startOfScoreList
          && _startOfPositionList == other._startOfPositionList
          && _posOfLastPosition == other._posOfLastPosition;
    }
};

//! Relation Meta Data written for each block.
//! This is written as part of RelationMetaData.
//! Part of the OntologyMetaData and as such part of the binary
//! index file. See comments for class OntologyMetaData for
//! information of the particular binary layout.
class RelationBlockMetaData
{
  public:
    Id _maxLhs;
    size_t _nofElements;

    off_t _startOfLhsData;
    off_t _startOfRhsData;
    off_t _posOfLastRhs;


    bool operator==(const RelationBlockMetaData& other) const
    {
      return _maxLhs == other._maxLhs&& _nofElements
          == other._nofElements && _startOfLhsData == other._startOfLhsData
          && _startOfRhsData == other._startOfRhsData
          && _posOfLastRhs == other._posOfLastRhs;
    }
};

//! Meta Data written for each relation.
//! This has keeps information such as relationId, types of
//! both sides of the relation and a list of block offsets
//! (size 1 to n) of the blocks that represent the relation on disk.
//! Part of the OntologyMetaData and as such part of the binary
//! index file. See comments for class OntologyMetaData for
//! information of the particular binary layout.
class RelationMetaData
{
  public:
    Id _relationId;
    Id _lhsType;
    Id _rhsType;

    // Can have either zero, one or more elements.
    // One element is used for relations that are not split into block.
    // The vector is empty when tracking relation data during index construction
    // and before it has been serialized.
    // It is nonempty when the info is used in the index of a running server.
    vector<RelationBlockMetaData> _blockInfo;

    const RelationBlockMetaData& getBlockInfo(const Id wordId) const
    {
      // Do a binary search.
      RelationBlockMetaData dummy;
      dummy._maxLhs = wordId;
      vector<RelationBlockMetaData>::const_iterator it = std::lower_bound(
          _blockInfo.begin(), _blockInfo.end(), dummy,
          ad_utility::MaxLhsComparatorLt());

      // Lower bound finds the first not smaller, however we
      // need the last one that is still smaller or equal.
      if (it->_maxLhs != wordId)
      {
        --it;
      }
      AD_CHECK(it >= _blockInfo.begin());
      return *it;
    }

    RelationMetaData()
    {
    }

    RelationMetaData(Id relationId, Id lhsType, Id rhsType)
    : _relationId(relationId), _lhsType(lhsType), _rhsType(rhsType)
    {
    }

    bool operator==(const RelationMetaData& other) const
    {
      return _relationId == other._relationId
          && _lhsType == other._lhsType
          && _rhsType == other._rhsType;
    }
};

//! Meta data written for each full-text index. Consists of several
//! items of BlockMetaData and enables a connection from
//! a word ID to the associated BlockMetaData.
//! The layout on disk is as follows:
//! <acutal blocks... ><blockMeta1><blockMeta2>...<blockMetaN><PosOfFirstMeta>
//! whereas each <blockMeta> looks like this:
//! <MaxWordId><NofPostings><WordsFrom><ContextsFrom><ScoresFrom>
//! <positionsFrom><posOfLastPos>
class FulltextMetaData
{
  public:

    //! Get the corresponding block meta data for some word or entity Id range.
    //! Currently assumes that the range lies in a single block.
    const BlockMetaData& getBlockInfoByWordRange(const Id lower,
        const Id upper) const
    {
      AD_CHECK_GE(upper, lower);
      AD_CHECK_GT(_blockInfo.size(), 0);
      AD_CHECK_EQ(_blockInfo.size(), _blockUpperBoundWordIds.size());


      // Binary search in the sorted _blockUpperBoundWordIds vector.
      vector<Id>::const_iterator it = std::lower_bound(
          _blockUpperBoundWordIds.begin(), _blockUpperBoundWordIds.end(),
          lower);

      AD_CHECK(it != _blockUpperBoundWordIds.end());

      // Use the info to retrieve an index.
      size_t index = it - _blockUpperBoundWordIds.begin();

      // Ensure that:
      // 1. We have found a block that matches
      // 2. The upper-bound is also in that block.
      AD_CHECK_LE(upper, _blockUpperBoundWordIds[index]);
      return _blockInfo[index];
    }

    //! Initialized IndexMetaData from some index file.
    //! The meta data should be located in indexFile from metaFrom (inclusive)
    //! to metaTo (exclusive).
    void initFromFile(File* indexFile)
    {
      off_t metaFrom;
      off_t metaTo = indexFile->getLastOffset(&metaFrom);
      AD_CHECK_LT(metaFrom, metaTo);
      _blockInfo.clear();
      _blockUpperBoundWordIds.clear();

      off_t currentOffset = metaFrom;
      while (currentOffset < metaTo)
      {
        BlockMetaData blockMeta;
        indexFile->read(&blockMeta._maxWordId, sizeof(blockMeta._maxWordId),
            currentOffset);
        currentOffset += sizeof(blockMeta._maxWordId);
        indexFile->read(&blockMeta._nofPostings,
            sizeof(blockMeta._nofPostings), currentOffset);
        currentOffset += sizeof(blockMeta._nofPostings);
        indexFile->read(&blockMeta._startOfWordList,
            sizeof(blockMeta._startOfWordList), currentOffset);
        currentOffset += sizeof(blockMeta._startOfWordList);
        indexFile->read(&blockMeta._startOfContextList,
            sizeof(blockMeta._startOfContextList), currentOffset);
        currentOffset += sizeof(blockMeta._startOfContextList);
        indexFile->read(&blockMeta._startOfScoreList,
            sizeof(blockMeta._startOfScoreList), currentOffset);
        currentOffset += sizeof(blockMeta._startOfScoreList);
        indexFile->read(&blockMeta._startOfPositionList,
            sizeof(blockMeta._startOfPositionList), currentOffset);
        currentOffset += sizeof(blockMeta._startOfPositionList);
        indexFile->read(&blockMeta._posOfLastPosition,
            sizeof(blockMeta._posOfLastPosition), currentOffset);
        currentOffset += sizeof(blockMeta._posOfLastPosition);
        _blockInfo.push_back(blockMeta);
        _blockUpperBoundWordIds.push_back(blockMeta._maxWordId);
      }
    }

    size_t getBlockCount()
    {
      return _blockInfo.size();
    }

    //! Do the calculation. Shouldn't be call during production
    //! but is nice for debug purposes
    size_t calculateTotalNumberOfPostings()
    {
      size_t sum = 0;
      for (size_t i = 0; i < _blockInfo.size(); ++i)
      {
        sum += _blockInfo[i]._nofPostings;
      }
      return sum;
    }

    FRIEND_TEST(FulltextIndexMetaDataTest, initFromFileTest);
    FRIEND_TEST(FulltextIndexMetaDataTest, getBlockInfoByWordRangeTest);

  private:
    vector<Id> _blockUpperBoundWordIds;
    vector<BlockMetaData> _blockInfo;
};

//! OntologyMetaData is held for the ontology index.
//! It contains information on which relation is split
//! into blocks and which isn't.
//! Also it contains offsets for relations and their blocks
//! so that the meta data will allow access to the correct portions
//! of the index file for each unit (i.e. Realation & RelationBlock)
//! OntologyMetaData is part of the index file and kept in the end.
//! The layout is as follows:
//! <RelationMetaData1><RelationMetaData2>...<RelationMetaDataN>
//! whereas each <RelationMetaData> looks like this:
//! <nextMeta><relId><lhsTypeId><rhsTypeId><RelationBlockMetaData0>...<...N>
//! RelationBlockMetaData holds the offsets for a particular block.
//! Each of them looks like this:
//! <maxLhsId><nofElements><startOflhsIds><startofRhsIds><lastRhsId>
class OntologyMetaData
{
  public:

    //! Get the meta data for a given relation by relationId.
    //! The relation Id is the wordId of the relationName in the
    //! associated ontology vocabulary.
    //! Possible relationNames are of the form:
    //! :r:born-in or :r:born-in_(reversed)
    //! see: REVERSED_RELATION_SUFFIX constant in Globals.h.
    const RelationMetaData& getRelationMetaData(Id relationId) const
    {
      HashMap<Id, RelationMetaData>::const_iterator it = _relationData.find(
          relationId);
      if (it == _relationData.end())
      {
        AD_THROW(Exception::UNKNOWN_RELATION_ID,
            "Couldn't find a relation with the Id passed as argument");
      }
      return it->second;
    }

    //! Initialize from some index file.
    //! The meta data should be located in indexFile from metaFrom (inclusive)
    //! to metaTo (exclusive).
    FRIEND_TEST(OntologyMetaDataTest, initFromFileTest);
    void initFromFile(File* indexFile)
    {
      off_t metaFrom;
      off_t metaTo = indexFile->getLastOffset(&metaFrom);
      AD_CHECK_LT(metaFrom, metaTo);
      _relationData.clear();
      off_t nextMetaData = metaFrom;
      off_t currentOffset = metaFrom;
      while (nextMetaData < metaTo)
      {
        RelationMetaData relMetaData;
        indexFile->read(&nextMetaData, sizeof(nextMetaData), currentOffset);
        currentOffset += sizeof(nextMetaData);
        indexFile->read(&relMetaData._relationId,
            sizeof(relMetaData._relationId), currentOffset);
        currentOffset += sizeof(relMetaData._relationId);
        indexFile->read(&relMetaData._lhsType, sizeof(relMetaData._lhsType),
            currentOffset);
        currentOffset += sizeof(relMetaData._lhsType);
        indexFile->read(&relMetaData._rhsType, sizeof(relMetaData._rhsType),
            currentOffset);
        currentOffset += sizeof(relMetaData._rhsType);
        while (currentOffset < nextMetaData)
        {
          // <maxLhsId><nofElements><startOflhsIds><startofRhsIds><lastRhsId>
          RelationBlockMetaData blockData;
          indexFile->read(&blockData._maxLhs, sizeof(blockData._maxLhs),
              currentOffset);
          currentOffset += sizeof(blockData._maxLhs);
          indexFile->read(&blockData._nofElements,
              sizeof(blockData._nofElements), currentOffset);
          currentOffset += sizeof(blockData._nofElements);
          indexFile->read(&blockData._startOfLhsData,
              sizeof(blockData._startOfLhsData), currentOffset);
          currentOffset += sizeof(blockData._startOfLhsData);
          indexFile->read(&blockData._startOfRhsData,
              sizeof(blockData._startOfRhsData), currentOffset);
          currentOffset += sizeof(blockData._startOfRhsData);
          indexFile->read(&blockData._posOfLastRhs,
              sizeof(blockData._posOfLastRhs), currentOffset);
          currentOffset += sizeof(blockData._posOfLastRhs);
          relMetaData._blockInfo.push_back(blockData);
        }
        AD_CHECK_EQ(currentOffset, nextMetaData);
        AD_CHECK(_relationData.find(relMetaData._relationId)
            == _relationData.end());
        _relationData[relMetaData._relationId] = relMetaData;
      }
    }

    size_t getRelationCount()
    {
      return _relationData.size();
    }

  private:
    FRIEND_TEST(IndexTest, registerOntologyIndexTest);
    FRIEND_TEST(IndexTest, readFullRelationTest);
    FRIEND_TEST(IndexTest, readRelationBlockTest);
    HashMap<Id, RelationMetaData> _relationData;
};
}
#endif  // SEMANTIC_WIKIPEDIA_SERVER_INDEXMETADATA_H_
