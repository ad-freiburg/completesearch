// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_PLAINSERIALIZER_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_PLAINSERIALIZER_H_

#include <stxxl/vector>
#include <string>
#include <vector>

#include "../server/IndexMetaData.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "./Postings.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! A simple serializer that writes BasicPostings to a file.
//! No compression is done, only word and context Ids are written.
class PlainSerializer
{
  public:
    PlainSerializer()
    {
    }
    virtual ~PlainSerializer()
    {
    }

    //! Writes the list of postings into the file.
    //! The file will look like this:
    //! <block0><block1>...<blockMetaData><startOfMetaData>
    //! whereas each block looks like this:
    //! <wordIds><contextIds><Scores><Positions>
    //! and BlockMetaData looks like this:
    //! <MaxWordId><NofPostings><WordsFrom><ContextsFrom><ScoresFrom><End>
    void serialize(const stxxl::vector<Posting>& postings,
        const string& fileName);

  private:
    typedef stxxl::vector<Posting>::const_iterator PostingIterator;

    off_t _currentOffset;
    vector<BlockMetaData> _metaData;

    //! Serialize one block from postings. The block is marked by
    //! the iterators begin and end, including begin, excluding end.
    void serializeBlock(const stxxl::vector<Posting>& postings,
        const PostingIterator& begin, const PostingIterator& end,
        ad_utility::File* file);

    void writeWordList(const stxxl::vector<Posting>& postings,
        const PostingIterator& begin, const PostingIterator& end,
        ad_utility::File* file);

    void writeContextList(const stxxl::vector<Posting>& postings,
            const PostingIterator& begin, const PostingIterator& end,
            ad_utility::File* file);

    void writeScoreList(const stxxl::vector<Posting>& postings,
            const PostingIterator& begin, const PostingIterator& end,
            ad_utility::File* file);

    void writePositionList(const stxxl::vector<Posting>& postings,
            const PostingIterator& begin, const PostingIterator& end,
            ad_utility::File* file);
};
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_PLAINSERIALIZER_H_
