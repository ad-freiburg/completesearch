// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_PLAINONTOLOGYSERIALIZER_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_PLAINONTOLOGYSERIALIZER_H_

#include <stxxl/vector>
#include <string>
#include <vector>

#include "../server/IndexMetaData.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "./Facts.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! A simple serializer that writes BasicPostings to a file.
//! No compression is done, only word and context Ids are written.
class PlainOntologySerializer
{
  public:
    PlainOntologySerializer()
    {
    }
    virtual ~PlainOntologySerializer()
    {
    }

    //! Writes the relation lists into the file.
    //! The file will look like this:
    //! <rel0_block0><rel0_b1><rel1_b0>...<relationMetaData><startOfMetaData>
    //! whereas <relationMetaData> looks like this:
    //! <nextMeta><relId><lhsTypeId><rhsTypeId><RelationBlockMetaData0>...<...N>
    //! RelationBlockMetaData holds the offsets for a particular block.
    //! Each block looks like this:
    //! <maxLhsId><nofElements><startOflhsIds><startofRhsIds><lastRhsId>
    void serialize(const stxxl::vector<RelationFact>& facts,
            vector<RelationMetaData>& metaData,
            const ad_utility::HashSet<Id>& relationsToBeSplitIntoBlocksByLhs,
            const string& outputFileName);

  private:
    typedef stxxl::vector<RelationFact>::const_iterator TupleIterator;

    off_t _currentOffset;
    vector<RelationMetaData> _metaData;
    vector<RelationBlockMetaData> _currentRelationBlockMetaData;

    //! Serialize one relation. The relation is marked by
    //! the iterators begin and end, including begin, excluding end.
    void serializeRelation(const stxxl::vector<RelationFact>& facts,
        RelationMetaData& metaData, bool splitRelationByLhs,
        const TupleIterator& begin, const TupleIterator& end,
        ad_utility::File* file);

    //! Serialize one block. The block is marked by
    //! the iterators begin and end, including begin, excluding end.
    void serializeBlock(const stxxl::vector<RelationFact>& facts,
        const TupleIterator& begin, const TupleIterator& end,
        ad_utility::File* file);

    void writeLhsList(const stxxl::vector<RelationFact>& facts,
        const TupleIterator& begin, const TupleIterator& end,
        ad_utility::File* file);

    void writeRhsList(const stxxl::vector<RelationFact>& facts,
        const TupleIterator& begin, const TupleIterator& end,
        ad_utility::File* file);
};
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_PLAINONTOLOGYSERIALIZER_H_
