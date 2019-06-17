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
#include "./PlainSerializer.h"
#include "../codebase/semantic-wikipedia-utils/File.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
// _____________________________________________________________________________
void PlainSerializer::serialize(const stxxl::vector<Posting>& postings,
    const string& fileName)
{
  assert(postings.size() > 0);

  ad_utility::File file(fileName.c_str(), "w");

  // Write a file in the following way:
  // <block0><block1>...<blockMetaData><startOfMetaData>
  // whereas each block looks like this:
  // <wordIds><contextIds><Scores><Positions>
  // and BlockMetaData looks like this:
  // <MaxWordId><NofPostings><WordsFrom><ContextsFrom><ScoresFrom><PosFrom><End>

  // Collect MetaData always while writing a block and add it to the vector
  // when the block is finished.
  _currentOffset = 0;
  _metaData.clear();

  Id currentBlock = postings[0]._blockId;
  PostingIterator begin = postings.begin();
  for (PostingIterator current = postings.begin();
      current != postings.end(); ++current)
  {
    if (current->_blockId != currentBlock)
    {
      serializeBlock(postings, begin, current, &file);
      currentBlock = current->_blockId;
      begin = current;
    }
  }
  // serialize the last Block
  serializeBlock(postings, begin, postings.end(), &file);

  // Write MetaData & finish
  for (size_t i = 0; i < _metaData.size(); ++i)
  {
    const BlockMetaData& meta = _metaData[i];
    file.write(&meta._maxWordId, sizeof(meta._maxWordId));
    file.write(&meta._nofPostings, sizeof(meta._nofPostings));
    file.write(&meta._startOfWordList, sizeof(meta._startOfWordList));
    file.write(&meta._startOfContextList, sizeof(meta._startOfContextList));
    file.write(&meta._startOfScoreList, sizeof(meta._startOfScoreList));
    file.write(&meta._startOfPositionList, sizeof(meta._startOfPositionList));
    file.write(&meta._posOfLastPosition, sizeof(meta._posOfLastPosition));
  }
  file.write(&_currentOffset, sizeof(_currentOffset));
}
// _____________________________________________________________________________
void PlainSerializer::serializeBlock(const stxxl::vector<Posting>& postings,
    const PostingIterator& begin, const PostingIterator& end,
    ad_utility::File* file)
{
  assert(end - begin > 0);
  BlockMetaData meta;

  meta._nofPostings = end - begin;
  meta._maxWordId = getFirstId(IdType::WORD_ID);

  bool noWord = true;
  for (PostingIterator it = begin; it < end; ++it)
  {
    if (isIdOfType(it->_wordId, IdType::WORD_ID))
    {
      noWord = false;
      if (it->_wordId > meta._maxWordId)
      {
        meta._maxWordId = it->_wordId;
      }
    }
  }

  if (noWord)
  {
    for (PostingIterator it = begin; it < end; ++it)
    {
      if (it->_wordId > meta._maxWordId)
      {
        meta._maxWordId = it->_wordId;
      }
    }
  }

  // Write wordList
  meta._startOfWordList = _currentOffset;
  writeWordList(postings, begin, end, file);

  // Write contexList
  meta._startOfContextList = _currentOffset;
  writeContextList(postings, begin, end, file);

  // Write scores
  meta._startOfScoreList = _currentOffset;
  writeScoreList(postings, begin, end, file);

  // Write postions
  meta._startOfPositionList = _currentOffset;
  writePositionList(postings, begin, end, file);
  meta._posOfLastPosition = _currentOffset - sizeof(Position);

  // Add metaData
  _metaData.push_back(meta);
}
// _____________________________________________________________________________
void PlainSerializer::writeWordList(const stxxl::vector<Posting>& postings,
    const PostingIterator& begin, const PostingIterator& end,
    ad_utility::File* file)
{
  for (PostingIterator it = begin; it < end; ++it)
  {
    file->write(&it->_wordId, sizeof(Id));
  }
  _currentOffset += (end - begin) * sizeof(Id);
}
// _____________________________________________________________________________
void PlainSerializer::writeContextList(const stxxl::vector<Posting>& postings,
    const PostingIterator& begin, const PostingIterator& end,
    ad_utility::File* file)
{
  for (PostingIterator it = begin; it < end; ++it)
  {
    file->write(&it->_contextId, sizeof(Id));
  }
  _currentOffset += (end - begin) * sizeof(Id);
}
// _____________________________________________________________________________
void PlainSerializer::writeScoreList(const stxxl::vector<Posting>& postings,
    const PostingIterator& begin, const PostingIterator& end,
    ad_utility::File* file)
{
  for (PostingIterator it = begin; it < end; ++it)
  {
    file->write(&it->_score, sizeof(Score));
  }
  _currentOffset += (end - begin) * sizeof(Score);
}
// _____________________________________________________________________________
void PlainSerializer::writePositionList(
    const stxxl::vector<Posting>& postings, const PostingIterator& begin,
    const PostingIterator& end, ad_utility::File* file)
{
  for (PostingIterator it = begin; it < end; ++it)
  {
    file->write(&it->_pos, sizeof(Position));
  }
  _currentOffset += (end - begin) * sizeof(Position);
}
}

