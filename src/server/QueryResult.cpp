#include <queue>
#include "QueryResult.h"
#include "ConcurrentLog.h"

//! Possible kinds of results (describing the way it was computed)
string QueryResult::UNDEFINED = "[how result was computed: undefined]";
string QueryResult::NORMAL = "normal: intersect with block(s) read from disk";
string QueryResult::FROM_HISTORY = "fetched from history";
string QueryResult::FROM_HISTORY_TOPK_AGAIN =
    "fetched from history with top-k recomputed";
string QueryResult::FROM_HISTORY_FILTERED_1 = "filtered from history (type 1)";
string QueryResult::FROM_HISTORY_FILTERED_2 = "filtered from history (type 2)";
string QueryResult::FROM_HISTORY_FILTERED_3 = "filtered from history (type 3)";
string QueryResult::FROM_HISTORY_INTERSECT =
    "intersected results from history";
string QueryResult::ALL_POSTINGS = "[dummy result containing all postings]";
string QueryResult::OTHER = "[how result was computed: other]";

//! Special query result returned for empty query
QueryResult emptyQueryResult;

//! Create empty result; status is UNDER_CONSTRUCTION
QueryResult::QueryResult()
{
  _howResultWasComputed = UNDEFINED;
  _status = UNDER_CONSTRUCTION;
  wasInHistory = false;
  nofTotalHits = 0;
  nofTotalCompletions = 0;
  isLockedForWriting = false;
  isLockedForReading = false;
  _bufferPosition = 0;
}
;

//! Create query result containing (conceptually) all postings; for processing one-word queries
QueryResult::QueryResult(bool fullList)
{
  _howResultWasComputed = ALL_POSTINGS;
  _status = FINISHED;
  wasInHistory = false;
  nofTotalHits = 0;
  nofTotalCompletions = 0;
  isLockedForWriting = false;
  isLockedForReading = false;
  _bufferPosition = 0;
  if (fullList)
  {
    // Only these two should be "needed" in this case
    _topDocIds = DocList(true);
    _docIds = DocList(true);
    //          _positions = Vector<Position>(true);
  }
}
;

//! Copy constructor; used for join (intersection of word list) and for not operator (intersection with complement)
QueryResult::QueryResult(const QueryResult& orig)
{
  isLockedForWriting = false;
  isLockedForReading = false;
  _status = orig._status;
  wasInHistory = orig.wasInHistory;
  resultWasFilteredFrom = orig.resultWasFilteredFrom;
  nofTotalHits = orig.nofTotalHits;
  nofTotalCompletions = orig.nofTotalCompletions;
  _bufferPosition = 0; // should be reset before use anyways (and only finished results or empty results are copied)
  _topDocIds = orig._topDocIds;
  _topDocScores = orig._topDocScores;
  _lastBlockScores = orig._lastBlockScores;
  _query = orig._query;
  _prefixCompleted = orig._prefixCompleted;
  _topDocScores = orig._topDocScores;
  _lastBlockScores = orig._lastBlockScores;
  _topWordIds = orig._topWordIds;
  _docIds = orig._docIds;
  _wordIdsOriginal = orig._wordIdsOriginal;
  _positions = orig._positions;
  _scores = orig._scores;
  _topCompletions = orig._topCompletions;
  _topWordDocCounts = orig._topWordDocCounts;
  _topWordOccCounts = orig._topWordOccCounts;
  _topWordScores = orig._topWordScores;
  _howResultWasComputed = orig._howResultWasComputed;

  assert((_status & QueryResult::FINISHED) ||
      ((_status == QueryResult::UNDER_CONSTRUCTION) && (isEmpty())));
}

//! Set to empty result
void QueryResult::clear()
{
  _topDocIds.clear();
  _topWordIds.clear();
  _docIds.clear();
  _wordIdsOriginal.clear();
  _wordIdsMapped.clear();
  _scores.clear();
  _positions.clear();
  _topDocScores.clear();
  _lastBlockScores.clear();
  _topCompletions.clear();
}

//! Set error message (if empty, writes default message)
void QueryResult::setErrorMessage(string msg)
{
  errorMessage
      = msg.length() > 0 ? msg
          : "UNKNOWN ERROR: application is supposed to provide details but failed to do so";
}

//! Number of bytes consumed by this result (essentially the total size of the postings lists)
size_t QueryResult::sizeInBytes() const
{
  return _topDocIds.sizeInBytes() + _topDocScores.sizeInBytes()
      + _lastBlockScores.sizeInBytes() + _query.sizeInBytes()
      + _prefixCompleted.size() + _topWordIds.sizeInBytes()
      + _docIds.sizeInBytes()
      + _wordIdsMapped.sizeInBytes()
      + _wordIdsOriginal.sizeInBytes()
      + _positions.sizeInBytes() + _scores.sizeInBytes()
      + _topCompletions.sizeInBytes();
}

//! Free all extra (reserved) space, once result is finished
void QueryResult::freeExtraSpace()
{
  LOG << AT_BEGINNING_OF_METHOD << endl;
  _topDocIds.unreserve();
  _topWordIds.unreserve();
  _topDocScores.unreserve();
  _lastBlockScores.unreserve();
  _docIds.unreserve();
  _wordIdsMapped.unreserve();
  _wordIdsOriginal.unreserve();
  _positions.unreserve();
  _scores.unreserve();
  _topWordDocCounts.unreserve();
  _topWordOccCounts.unreserve();
  _topWordScores.unreserve();
  LOG << AT_END_OF_METHOD << endl;
}

//! Check that the parallel vectors are of the same length; throw exception otherwise.
/*!
 *    \param MODE  the mode of the completer; e.g. for a completer without
 *    scores the length of the score vector is always zero.
 */
void QueryResult::checkSameNumberOfDocsWordsPositionsScores(unsigned char MODE) const
{
  CS_ASSERT(_wordIdsOriginal.size() == _docIds.size());
  if (MODE & WITH_POS)
    CS_ASSERT(_positions.size() == _wordIdsOriginal.size())
  else
  CS_ASSERT(_positions.size() == 0);
  if (MODE & WITH_SCORES)
    CS_ASSERT(_scores.size() == _wordIdsOriginal.size())
  else
  CS_ASSERT(_scores.size() == 0);
}

//! Simplistic sanity check; i.p. checks that status is IS_FINISHED
/*!
 *   WARNING: this check will be used in various CS_ASSERTs and must not take any
 *   significant time!!!
 */
bool QueryResult::check() const
{
  if (!(_status & QueryResult::FINISHED))
  {
    return false;
  }
  if (!(_docIds.isEmpty()
      || !_wordIdsOriginal.isEmpty()
      || _docIds.isFullList()))
  {
    return false;
  }
  if (!(!_docIds.isEmpty()
      || _wordIdsOriginal.isEmpty()))
  {
    return false;
  }
  if (!((_topDocIds.isEmpty() && _topCompletions.isEmpty()
      && _docIds.isEmpty()) || (!_topDocIds.isEmpty()
      && !_topCompletions.isEmpty() && !_docIds.isEmpty())
      || (_topDocIds.isFullList())))
    ;
  if (_docIds.size()
      != _wordIdsOriginal.size())
  {
    return false;
  }
  if (!((_scores.size() == 0) || (_scores.size()
      == _docIds.size())))
  {
    return false;
  }
  if (!((_positions.size() == 0) || (_positions.size()
      == _docIds.size())))
  {
    return false;
  }
  return true;
}

//! Check if the max word and max document score are equal (must only be true when all aggrgations are max)
bool QueryResult::sameMax() const
{
  if ((_topDocScores.size() == 0) && (_topCompletions.size() == 0))
    return true;
  if ((_topDocScores.size() * _topCompletions.size()) == 0)
    return false;
  if (_topDocScores.max() == _topCompletions.getCompletionScores().max())
  {
    return true;
  }
  return false;
} // end: sameMax()


//! True iff result is empty (zero matching postings)
bool QueryResult::isEmpty() const
{
  assert(_docIds.isEmpty() || !_wordIdsOriginal.isEmpty() || _docIds.isFullList());
  assert(!_docIds.isEmpty() || _wordIdsOriginal.isEmpty());
  assert((_topDocIds.isEmpty() && _topCompletions.isEmpty() && _docIds.isEmpty()) || (!_topDocIds.isEmpty() && !_topCompletions.isEmpty() && !_docIds.isEmpty() ) || (_topDocIds.isFullList()));
  return (_docIds.isEmpty()) ? true : false;
}

// Sort the 'long' lists of the QueryResult.
void QueryResult::sortLists()
{
  assert(_docIds.size() ==
      _wordIdsOriginal.size());
  if (_docIds.size() == 0)
    return;

  // Case: with scores.
  if (_scores.size() > 0)
  {
    assert(_scores.size() ==
        _docIds.size());
    if (_positions.size() > 0)
    {
      // NOTE: This is the typical case (with scores and positions).
      assert(_positions.size() ==
          _docIds.size());
      _docIds.sortParallel(_positions,
          _scores, _wordIdsOriginal);
      // Eliminate duplicate special postings (postings with word id
      // SPECIAL_WORD_ID).
      {
        size_t j = 0;
        DocList& docIds = _docIds;
        WordList& wordIds = _wordIdsOriginal;
        Vector<Score>& scores = _scores;
        Vector<Position>& positions = _positions;
        for (size_t i = 0; i < _wordIdsOriginal.size(); ++i)
        {
          if (wordIds[i] == SPECIAL_WORD_ID && i > 0 && wordIds[i - 1]
              == SPECIAL_WORD_ID && docIds[i] == docIds[i - 1])
            continue;
          if (j < i)
          {
            docIds[j] = docIds[i];
            wordIds[j] = wordIds[i];
            scores[j] = scores[i];
            positions[j] = positions[i];
          }
          ++j;
        }
        CS_ASSERT_LE(j, docIds.size());
        CS_ASSERT_LE(j, wordIds.size());
        CS_ASSERT_LE(j, scores.size());
        CS_ASSERT_LE(j, positions.size());
        docIds.resize(j);
        wordIds.resize(j);
        scores.resize(j);
        positions.resize(j);
      }

    }
    else
    {
      _docIds.sortParallel(_scores,
          _wordIdsOriginal);
      LOG << "! WARNING: duplicate postings not eliminated without positions"
          << endl;
    }
  }

  // Case: without scores.
  else
  {
    if (_positions.size() > 0)
    {
      assert(_positions.size() ==
          _docIds.size());
      _docIds.sortParallel(_positions,
          _wordIdsOriginal);
    }
    else
    {
      _docIds.sortParallel(_wordIdsOriginal);
    }
    LOG << "! WARNING: duplicate postings not eliminated without scores"
        << " and positions" << endl;
  }

  assert(_docIds.isSorted());
}

// Merge k results into one via k-way merge.
void QueryResult::mergeResultLists(const vector<QueryResult>& inputLists,
    QueryResult* result)
{
  LOG << AT_BEGINNING_OF_METHOD << endl;
  // for (size_t i = 0; i < inputLists.size(); ++i)
  //   inputLists[i].show();

  assert(result != NULL);
  if (inputLists.size() == 0)
  {
    return;
  }
  if (inputLists.size() == 1)
  {
    *result = inputLists[0];
    return;
  }

  // Introduce aliases for the four result lists and make sure they are empty.
  // TODO(bast): I would rather have an assert here that they are empty.
  DocList& docIds = result->_docIds;
  WordList& wordIds = result->_wordIdsOriginal;
  WordList& wordIdsMapped = result->_wordIdsMapped;
  Vector<Position>& positions = result->_positions;
  Vector<Score>& scores = result->_scores;
  docIds.clear();
  wordIds.clear();
  wordIdsMapped.clear();
  positions.clear();
  scores.clear();

  priority_queue<Triple, vector<Triple> , CompareTriple> pq;
  vector<unsigned int> listPointers;
  listPointers.resize(inputLists.size());
  Triple tempTriple;
  size_t resultLength = 0;
  for (unsigned int i = 0; i < inputLists.size(); i++)
    if (inputLists[i]._docIds.size() > 0)
    {
      tempTriple.docId = inputLists[i]._docIds[0];
      tempTriple.position = inputLists[i]._positions[0];
      tempTriple.list = i;
      pq.push(tempTriple);
      // LOG << "! DEBUG MERGE: push(" << tempTriple.docId
      //     << ", " << tempTriple.position
      //     << ", " << tempTriple.list << ")" << endl;
      listPointers[i]++;
      resultLength += inputLists[i]._docIds.size();
    }
  if (resultLength == 0)
    return;
  docIds.reserve(resultLength);
  wordIds.reserve(resultLength);
  positions.reserve(resultLength);
  scores.reserve(resultLength);
  while (pq.size() > 0)
  {
    tempTriple = pq.top();
    pq.pop();
    int pos = listPointers[tempTriple.list] - 1;

    const DocId& docId =
        inputLists[tempTriple.list]._docIds[pos];
    const WordId& wordId =
        inputLists[tempTriple.list]._wordIdsOriginal[pos];
    const Position& position =
        inputLists[tempTriple.list]._positions[pos];
    const Score& score = inputLists[tempTriple.list]._scores[pos];

    // Make sure that identical postings are pushed only once. In particular
    // this makes sure that the special posting (with word id SPECIAL_WORD_ID)
    // gets pushed only once per document.
    bool repeatedPosting = docIds.size() > 0 && wordIds.back() == wordId
        && docIds.back() == docId && positions.back() == position;
    if (!repeatedPosting)
    {
      docIds.push_back(docId);
      wordIds.push_back(wordId);
      positions.push_back(position);
      scores.push_back(score);
    }

    if (inputLists[tempTriple.list]._docIds.size()
        > listPointers[tempTriple.list])
    {
      tempTriple.docId
          = inputLists[tempTriple.list]. _docIds[listPointers[tempTriple.list]];
      tempTriple.position
          = inputLists[tempTriple.list]. _positions[listPointers[tempTriple.list]];
      listPointers[tempTriple.list]++;
      pq.push(tempTriple);
      // LOG << "! DEBUG MERGE: push(" << tempTriple.docId
      //     << ", " << tempTriple.position
      //     << ", " << tempTriple.list << ")" << endl;
    }
  }
  // LOG << "Result:" << endl;
  // result->show();
}

// Merge k *results into one via k-way merge
// As input takes vector of pointers to QueryResult
// Needed for fuzzysearch
void QueryResult::mergeResultLists(const vector<const QueryResult*>& inputLists,
                                   QueryResult* result)
{
  mergeResultLists(inputLists, 0, inputLists.size(), result);
}

// Merge k *results into one via k-way merge
// As input takes vector of pointers to QueryResult
// Needed for fuzzysearch's groupMerging
void QueryResult::mergeResultLists(const vector<const QueryResult*>& inputLists,
                                   int from,
                                   int to,
                                   QueryResult* result)
{
  priority_queue<Triple, vector<Triple>, CompareTriple> pq;
  vector<unsigned int> listPointers;
  listPointers.resize(inputLists.size());
  Triple tempTriple;
  size_t resultLength = 0;
  // Introduce aliases for the four result lists and make sure they are empty.
  // TODO(bast): I would rather have an assert here that they are empty.
  DocList& docIds = result->_docIds;
  WordList& wordIds = result->_wordIdsOriginal;
  WordList& wordIdsMapped = result->_wordIdsMapped;
  Vector<Position>& positions = result->_positions;
  Vector<Score>& scores = result->_scores;
  docIds.clear();
  wordIds.clear();
  wordIdsMapped.clear();
  positions.clear();
  scores.clear();
  if (inputLists.size() == 1)
  {
    *result = (*inputLists[0]);
    return;
  }
  from = MAX(0, from);
  to = MIN(static_cast<int>(inputLists.size()), to);
  for (int i = from; i < to; i++)
    if ((*inputLists[i])._docIds.size() > 0)
    {
      tempTriple.docId = (*inputLists[i])._docIds[0];
      tempTriple.position = (*inputLists[i])._positions[0];
      tempTriple.list = i;
      pq.push(tempTriple);
      listPointers[i]++;
      resultLength += (*inputLists[i])._docIds.size();
    }
  if (resultLength == 0)
    return;
  docIds.reserve(resultLength);
  wordIds.reserve(resultLength);
  positions.reserve(resultLength);
  scores.reserve(resultLength);
  while (pq.size() > 0)
  {
    tempTriple = pq.top();
    pq.pop();
    int pos = listPointers[tempTriple.list] - 1;

    const DocId& docId =
        (*inputLists[tempTriple.list])._docIds[pos];
    const WordId& wordId =
        (*inputLists[tempTriple.list])._wordIdsOriginal[pos];
    const Position& position =
        (*inputLists[tempTriple.list])._positions[pos];
    const Score& score =
        (*inputLists[tempTriple.list])._scores[pos];

    // Make sure that identical postings are pushed only once. In particular
    // this makes sure that the special posting (with word id SPECIAL_WORD_ID)
    // gets pushed only once per document.
    bool repeatedPosting = docIds.size() > 0 && wordIds.back() == wordId
        && docIds.back() == docId && positions.back() == position;
    if (!repeatedPosting)
    {
      docIds.push_back(docId);
      wordIds.push_back(wordId);
      positions.push_back(position);
      scores.push_back(score);
    }

    if ((*inputLists[tempTriple.list])._docIds.size()
        > listPointers[tempTriple.list])
    {
      tempTriple.docId = (*inputLists[tempTriple.list]).
          _docIds[listPointers[tempTriple.list]];
      tempTriple.position = (*inputLists[tempTriple.list]).
          _positions[listPointers[tempTriple.list]];
      listPointers[tempTriple.list]++;
      pq.push(tempTriple);
    }
  }
}

// used by groupMerge to sort the (pointers to) lists by sizes
bool sortByListSizes(QueryResult* x, QueryResult* y)
{
  return x->_docIds.size() > y->_docIds.size();
}

//! Merge two results into one
void QueryResult::mergeResultLists(const QueryResult& input1,
    const QueryResult& input2, QueryResult& output)
{
#ifndef NDEBUG
  cout << "! merging results in Vector.h " << endl;
#endif
  assert( input1._docIds.isSorted() );
  assert( input2._docIds.isSorted() );
  const DocList& doclist1 = input1._docIds;
  const DocList& doclist2 = input2._docIds;
  DocList& outdoclist = output._docIds;

  const Vector<Position>& poslist1 = input1._positions;
  const Vector<Position>& poslist2 = input2._positions;
  Vector<Position>& outposlist = output._positions;
  bool usePositions = (poslist1.size() > 0) || (poslist2.size() > 0);

  const Vector<Score>& scorelist1 = input1._scores;
  const Vector<Score>& scorelist2 = input2._scores;
  Vector<Score>& outscorelist = output._scores;

  const WordList& wordlist1 = input1._wordIdsOriginal;
  const WordList& wordlist2 = input2._wordIdsOriginal;
  WordList& outwordlist = output._wordIdsOriginal;

  outdoclist.resize(doclist1.size() + doclist2.size());
  outwordlist.resize(wordlist1.size() + wordlist2.size());
  outposlist.resize(poslist1.size() + poslist2.size());
  outscorelist.resize(scorelist1.size() + scorelist2.size());
  assert((input1._docIds.size() == 0) || (input1._docIds.back() != INFTY_DOCID));
  assert((input2._docIds.size() == 0) || (input2._docIds.back() != INFTY_DOCID));

  const unsigned long long int doclist1Size = doclist1.size();
  const unsigned long long int doclist2Size = doclist2.size();
  unsigned long long int numPostingsDropped = 0;
  // input1._docIds.push_back(INFTY_DOCID);
  // input2._docIds.push_back(INFTY_DOCID);

#define ADD_FROM_1(ii,kk)\
  {\
    assert(ii<doclist1Size);\
    assert(ii<doclist1.size());\
    assert(kk<outdoclist.size());\
    outdoclist[kk] = doclist1[ii];\
    outwordlist[kk] = wordlist1[ii];\
    outscorelist[kk] = scorelist1[ii];\
    if (usePositions) {outposlist[kk] = poslist1[ii];}\
  }
#define ADD_FROM_2(jj,kk)\
  {\
    assert(jj<doclist2Size);\
    assert(jj<doclist2.size());\
    assert(kk<outdoclist.size());\
    outdoclist[kk] = doclist2[jj];\
    outwordlist[kk] = wordlist2[jj];\
    outscorelist[kk] = scorelist2[jj];\
    if (usePositions) {outposlist[kk] = poslist2[jj];}\
  }

#ifndef NDEBUG
  cout << "! starting actual merging results in Vector.h " << endl;
#endif

  register DocId i = 0, j = 0, k = 0;
  while (true)
  {
    assert(i<=doclist1Size);
    assert(j<=doclist2Size);
    //          assert( ((i<doclist1Size  ) && ( j<doclist2Size )) || ((j==doclist2Size)&&(i<doclist1Size) )  ||  (( j<doclist2Size) && (i==doclist1Size) ));
    if ((i < doclist1Size) && ((((j == doclist2Size) && (i < doclist1Size))
        || (doclist1[i] < doclist2[j]))))
    {
      ADD_FROM_1(i,k);
      ++k;
      ++i;
    }
    else if ((j < doclist2Size) && ((((i == doclist1Size)
        && (j < doclist2Size)) || (doclist1[i] > doclist2[j]))))
    {
      ADD_FROM_2(j,k);
      ++k;
      ++j;
    }
    else
    {
      assert(doclist1Size == doclist1.size());
      assert((i==doclist1Size) || (j<doclist2Size));
      assert(((i==doclist1Size) && (j==doclist2Size)) || (doclist1[i] == doclist2[j]));
      //              if (doclist1[i] == INFTY_DOCID) {break;}
      if (i == doclist1Size)
      {
        break;
      }
      assert(i<doclist1Size);
      assert(j<doclist2Size);
      assert(i<poslist1.size());
      assert(j<poslist2.size());
      if (poslist1[i] < poslist2[j])
      {
        ADD_FROM_1(i,k);
        ++k;
        ++i;
      }
      else if (poslist1[i] > poslist2[j])
      {
        ADD_FROM_2(j,k);
        ++k;
        ++j;
      }
      else
      {
        // SAME DOC AND SAME POSITION IN BOTH LISTS!
        assert(poslist1[i] == poslist2[j]);
        //assert(wordlist1[i] == wordlist2[j]);// assumes that same doc+same pos
        ADD_FROM_1(i,k);
        ++i;
        ++j;
        ++k;
        assert((i-1) < wordlist1.size());
        assert((j-1) < wordlist2.size());
        // BUG(hannah): Here is the only place in the loop where it can happen
        // that a posting from one of the lists is dropped and not copied to the
        // output list. But the output lists are resized to the sum of the sizes
        // of the two input lists. For each posting dropped, the output list
        // will therefore have a trailing all-zero posting in the end, which is
        // wrong. In the UI this shows as an empty completion and a hit which
        // cannot be retrieved, because document id 0 is invalid and hence can
        // not be found. There are two alternatives two solve this problem:
        //
        // 1. Remove the if below and always add both postings, even if they are
        // completely the same. The question is whether there was / is a reason
        // to discard identical postings. Since this whole function here is used
        // only once in the whole code, namely for processing an OR query, I
        // think the only reason was / is efficiency. And it is indeed really
        // stupid, when having a big OR, to have the same posting repeated many
        // times.
        //
        // 2. Count the number of postings dropped (just increase a counter when
        // the ADD_FROM_2 is not executed below), and then resize down the
        // output lists in the end.  Given what I said at the end of 1. I prefer
        // that solution and have tentatively implemented it.
        if (wordlist1[i - 1] != wordlist2[j - 1])
        {
          ADD_FROM_2(j-1,k);
          ++k;
        }
        else
        {
          numPostingsDropped++;
        }
      }
    }
  }

  // If postings were dropped, resize the output list accordingly. Check that
  // the trailing postings before the resize are indeed zero postings.
  if (numPostingsDropped > 0)
  {
    assert(outdoclist.size() == outwordlist.size());
    assert(outdoclist.size() == outposlist.size());
    assert(outdoclist.size() == outscorelist.size());
    assert(outdoclist.size() >= numPostingsDropped);
    assert(outdoclist[outdoclist.size() - numPostingsDropped] == 0);
    assert(outwordlist[outwordlist.size() - numPostingsDropped] == 0);
    assert(outposlist[outposlist.size() - numPostingsDropped] == 0);
    assert(outscorelist[outscorelist.size() - numPostingsDropped] == 0);
    outdoclist.resize(outdoclist.size() - numPostingsDropped);
    outwordlist.resize(outwordlist.size() - numPostingsDropped);
    outposlist.resize(outposlist.size() - numPostingsDropped);
    outscorelist.resize(outscorelist.size() - numPostingsDropped);
  }

#ifndef NDEBUG
  cout << "! done actual merging results in Vector.h " << endl;
#endif

  //      input1._docIds.pop_back();
  //      input2._docIds.pop_back();
  assert((input1._docIds.size() == 0) || (input1._docIds.back() != INFTY_DOCID));
  assert((input2._docIds.size() == 0) || (input2._docIds.back() != INFTY_DOCID));
}

//! Join two results = intersect lists of word ids; for queries of the kind \c auth[sigir sigmod]
/*!
 *    Note 1: inputs are passed by value, since they will be (re)sorted; TODO: unavoidable?
 *    Note 2: sorts using stl sort; TODO: why is that a problem or noteworthy?
 *    Note 3: must be static, called without an object from CompleterBase::processJoinQuery
 *
 *    Historical note: last argument used to be called useLinear meaning the
 *    merge join. This was a bad misnomer, since the has join has linear
 *    running time, while the merge join takes superlinear time.
 */
// void QueryResult::intersectWordlists(QueryResult input1, QueryResult input2,
//    QueryResult& result, const int howToJoin)
// {
//
// }

//! TODO: Needed where? For debugging only?
void QueryResult::writeRankedResultToStream(ostringstream& outputStream,
    bool writeWords) const
{
  if (writeWords)
  {
    assert(nofTotalCompletions == (WordId)_topCompletions.size());
    if (_topCompletions.size() == 0)
    {
      //              outputStream << "----------" << endl;
    }// end case: no completions
    else
    {
      Score lastScore = _topCompletions[0].second;
      for (unsigned long i = 0; i < _topCompletions.size(); i++)
      {
        outputStream << string(_topCompletions[i].first) << "\t"
            << _topCompletions[i].second << endl;
        assert(_topCompletions[i].second <= lastScore);
        lastScore = _topCompletions[i].second;
      }
      //              outputStream << endl;
    }
    outputStream << "----------" << endl;
  }// end case: have to write words to the result
  else
  {
    assert(nofTotalHits == (DocId) _topDocIds.size());
    if (_topDocIds.size() == 0)
    {
      outputStream << "----------" << endl;
    }
    else
    {
      Score lastScore = _topDocScores[0];
      for (unsigned long i = 0; i < _topDocIds.size(); i++)
      {
        outputStream << "docid\t" << _topDocIds[i] << endl;
        assert(_topDocScores[i] <= lastScore);
        lastScore = _topDocScores[i];
      }
      outputStream << endl;
    }
  }// end case: have to write doc ids to the result
}

//! Show result in details; for debugging purposes only I guess
void QueryResult::show(unsigned int maxNumItemsToShowPerList) const
{
  cout << endl;
  cout << "Query result (" << flush;
  // For a nice print the encoded characters should be decoded at this point.
  // For more information see CompleterBase:237 8Feb13.
  if (_query.length() > 0)
    cout << "query = \"" << _query.getQueryString()
    //cout << "query = \"" << decodeHexNumbers(_query.getQueryString())
         << "\", " << flush;
  if (_prefixCompleted.length() > 0)
    cout << "last part = \"" << _prefixCompleted << "\", " << flush;
  cout << "status = " << int(_status) << "):" << endl;
  bool showOnlyPostings = nofTotalHits == 0 && nofTotalCompletions == 0;
  if (showOnlyPostings == false)
  {
    cout << "#matching docs  : " << nofTotalHits << endl;
    cout << "#matching words : " << nofTotalCompletions << endl;
  }
  if (1)
  {
    cout << "Postings, unmapped word ids  (" << setw(7)
        << _wordIdsOriginal.size() << ") : ";
    _wordIdsOriginal.show(maxNumItemsToShowPerList);
    cout << "Postings, mapped word ids    (" << setw(7)
        << _wordIdsMapped.size() << ") : ";
    _wordIdsMapped.show(maxNumItemsToShowPerList);
    // cout << "[no method to show word ids yet]" << endl;
    cout << "Postings, doc ids            (" << setw(7)
        << _docIds.size() << ") : ";
    _docIds.show(maxNumItemsToShowPerList);
    cout << "Postings, positions          (" << setw(7) << _positions.size()
        << ") : ";
    _positions.show(maxNumItemsToShowPerList);
    cout << "Postings, scores             (" << setw(7) << _scores.size()
        << ") : ";
    _scores.show(maxNumItemsToShowPerList);
  }
  if (showOnlyPostings == false)
  {
    cout << "Last Block Scores   (" << setw(7) << _lastBlockScores.size()
        << ")" << endl;
    //      cout << "Last block scores (" << _lastBlockScores.size() << ") : "; _lastBlockScores.show();
    cout << "Top-ranked doc ids     (" << setw(4) << _topDocIds.size()
        << ") : ";
    _topDocIds.show(maxNumItemsToShowPerList);
    cout << "  - and their scores   (" << setw(4) << _topDocScores.size()
        << ") : ";
    _topDocScores.show(maxNumItemsToShowPerList);
    cout << "Top-ranked completions (" << setw(4) << _topCompletions.size()
        << ") : ";
    _topCompletions.show();
    cout << endl;
  }
  cout << endl;
}
//! TODO: Needed where? For debugging only?
void QueryResult::dumpResultToStream(ostringstream& outputStream,
    Vocabulary& vocabulary) const
{
  if (_wordIdsOriginal.size() == 0)
  {
    outputStream << endl << endl << endl; // three lines for three missing results
  }
  else
  {
    _docIds.show(outputStream, UINT_MAX);
    _wordIdsOriginal.show(outputStream, vocabulary, UINT_MAX);
    _wordIdsMapped.show(outputStream, vocabulary, UINT_MAX);
    _scores.show(outputStream, UINT_MAX);
  }
  outputStream << endl;
} 

std::string QueryResult::asStringFlat(void) const
{
  std::stringstream ss;
  ss << _prefixCompleted << ", "
     << _status << ", "
     << "'" << errorMessage << "', "
     << wasInHistory << ", "
     << "'" << resultWasFilteredFrom << "', "
     << nofTotalHits << ", "
     << nofTotalCompletions << ", "
     << _docIds.asString() << ", "
     << _wordIdsOriginal.asString() << ", "
     //<< _wordIdsMapped.asString() << ", "
     << _scores.asString() << ", "
     << _positions.asString();
  return ss.str();
}

std::string QueryResult::asString(void) const
{
  std::stringstream ss;
  ss << "_query                  : " << _query.asString()
     << std::endl
     << "_prefixCompleted        : " << _prefixCompleted
     << std::endl
     << "_status                 : " << _status
     << std::endl
     << "errormessage            : " << errorMessage
     << std::endl
     << "wasInHistory            : " << wasInHistory
     << std::endl
     << "resultWasFilteredFrom   : " << resultWasFilteredFrom
     << std::endl
     << "nofTotalHits            : " << nofTotalHits
     << std::endl
     << "nofTotalCompletions     : " << nofTotalCompletions
     << std::endl
     << "_howResultWasComputed   : " << _howResultWasComputed
     << std::endl
     << "_docIds                 : " << _docIds.asString()
     << std::endl
     << "_wordIdsOriginal        : " << _wordIdsOriginal.asString()
     << std::endl
     << "_wordIdsMapped          : " << _wordIdsMapped.asString()
     << std::endl
     << "_positions              : " << _positions.asString()
     << std::endl
     << "_scores                 : " << _scores.asString()
     << std::endl
     << "_topDocIds              : " << _topDocIds.asString()
     << std::endl
     << "_topWordIds             : " << _topWordIds.asString()
     << std::endl
     << "_topDocScores           : " << _topDocScores.asString()
     << std::endl
     << "_lastBlockScores        : " << _lastBlockScores.asString()
     << std::endl
     << "_topCompletions         : " << _topCompletions.asString()
     << std::endl
     << "_topWordDocCounts       : " << _topWordDocCounts.asString()
     << std::endl
     << "_topWordOccCounts       : " << _topWordOccCounts.asString() 
     << std::endl
     << "_topWordScores          : " << _topWordScores.asString()
     << std::endl;
  return ss.str();
}

