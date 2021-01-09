#include <hash_map>
#include "CompleterBase.h"

// _____________________________________________________________________________
template<unsigned char MODE>
void CompleterBase<MODE>::joinTwoPostingLists(const QueryResult& input1,
    const QueryResult& input2, QueryResult& result, const int joinMethod)
{
  LOG << AT_BEGINNING_OF_METHOD << "; using " << (joinMethod
      == QueryParameters::MERGE_JOIN ? "merge join" : "hash join") << endl;

  if (input1._wordIdsOriginal.size() == 0)
  {
    return;
  }
  if (input2._wordIdsOriginal.size() == 0)
  {
    return;
  }

#ifndef NDEBUG
  cout << "!! non-trivial word intersection" << endl;
#endif

  //
  // CASE 1: Use merge join
  //
  if (joinMethod == QueryParameters::MERGE_JOIN)
  {
    doMergeJoin(input1, input2, result);
  }

  //
  // CASE 2: Use hash join
  //
  else if (joinMethod == QueryParameters::HASH_JOIN)
  {
    doHashJoin(input1, input2, result);
  }

  //
  // CASE: invalid join type
  //
  else
  {
    ostringstream os;
    os << "invalid join type (" << joinMethod << ")";
    CS_THROW(Exception::OTHER, os.str());
  }

  // Sort this result result by doc ids, needed for both variants of intersection; TODO: explain why!?
  LOG << IF_VERBOSITY_HIGHER << "! sorting join result by doc id" << endl;
  result.sortLists();
  result._docIds.markAsSorted(true);

  LOG << AT_END_OF_METHOD << "; result has " << result.getNofPostings()
      << " postings" << endl;
}
template<>
void CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>::doHashJoin(
    const QueryResult& input1, const QueryResult& input2, QueryResult& result)
{
#define SUPER_MAX_WORD_ID 2147483647
  WordId minInput1 = SUPER_MAX_WORD_ID, maxInput1 = 0, minInput2 =
      SUPER_MAX_WORD_ID, maxInput2 = 0;
  register unsigned long k;

#ifndef NDEBUG
  cout << "! find prefix range for input1" << endl;
#endif
  //          GlobalTimers::intersectWordlistsTimer1.cont();

  // We need to treat postings if the SPECIAL_WORD_ID differently,
  // because we do not want to join on them.
  typedef vector<Score> ScoreVector;
  __gnu_cxx ::hash_map<DocId, ScoreVector> specialPostingsMap;

  // FIND THE PREFIX RANGE FOR INPUT1

  // NEW 11Jan2011 (Bjoern):
  // Treat the special words separately.
  for (k = 0; k < input1._wordIdsOriginal.size(); ++k)
  {
    if (input1._wordIdsOriginal[k] == SPECIAL_WORD_ID)
    {
      specialPostingsMap[input1._docIds[k]].push_back(
          input1._scores[k]);
      continue;
    }

    if (input1._wordIdsOriginal[k] > maxInput1)
    {
      maxInput1 = input1._wordIdsOriginal[k];
    }
    if (input1._wordIdsOriginal[k] < minInput1)
    {
      minInput1 = input1._wordIdsOriginal[k];
    }
  }

#ifndef NDEBUG
  cout << "! find prefix range for input2" << endl;
#endif
  // FIND THE PREFIX RANGE FOR INPUT2
  for (k = 0; k < input2._wordIdsOriginal.size(); ++k)
  {
    if (input2._wordIdsOriginal[k] == SPECIAL_WORD_ID)
    {
      specialPostingsMap[input2._docIds[k]].push_back(
          input2._scores[k]);
      continue;
    }

    if (input2._wordIdsOriginal[k] > maxInput2)
    {
      maxInput2 = input2._wordIdsOriginal[k];
    }
    if (input2._wordIdsOriginal[k] < minInput2)
    {
      minInput2 = input2._wordIdsOriginal[k];
    }
  }
#undef SUPER_MAX_WORD_ID
  //          GlobalTimers::intersectWordlistsTimer1.stop();

#ifndef NDEBUG
  cout << "! done with prefix ranges" << endl;
#endif

  WordId min = MAX(minInput1,minInput2);
  WordId max = MIN(maxInput1,maxInput2);
  WordId range = max - min;
  if (range < 0)
  {
    return;
  } // nothing to do.
  assert(range <= (maxInput1-minInput1));
  assert(range <= (maxInput2-minInput2));

  vector<bool> bitvector1(range + 1, false), bitvector2(range + 1, false);

  const QueryResult *temp1, *temp2; // if one uses references here then
  //the compiler complains about missing initialization
  if (input1._wordIdsOriginal.size()
      < input2._wordIdsOriginal.size())
  {
    temp1 = &input1;
    temp2 = &input2;
  }
  else
  {
    temp1 = &input2;
    temp2 = &input1;
  }
  const QueryResult& list1 = *temp1;
  const QueryResult& list2 = *temp2;

#ifndef NDEBUG
  cout << "! going through list1 for first time" << endl;
#endif
  //          GlobalTimers::intersectWordlistsTimer2.cont();
  // GO THROUGH LIST1 (the shorter list) for the 1st time
  for (k = 0; k < list1._wordIdsOriginal.size(); ++k)
  {
    if ((list1._wordIdsOriginal[k] <= max)
        && (list1._wordIdsOriginal[k] >= min))
    {
      bitvector1[list1._wordIdsOriginal[k] - min] = true;
    }
  }// end first iteration over list1 (the shorter list)
  //          GlobalTimers::intersectWordlistsTimer2.stop();
#ifndef NDEBUG
  cout << "! going through list2" << endl;
#endif
  //          GlobalTimers::intersectWordlistsTimer3.cont();
  // GO THROUGH LIST2 (the longer list)
  for (k = 0; k < list2._wordIdsOriginal.size(); ++k)
  {
    if ((list2._wordIdsOriginal[k] <= max)
        && (list2._wordIdsOriginal[k] >= min))
    {
      if (bitvector1[list2._wordIdsOriginal[k] - min] == true)
      {
        bitvector2[list2._wordIdsOriginal[k] - min] = true; // now holds the intersection
        // MATCH FOUND IN LIST2
        result._wordIdsOriginal.push_back(
            list2._wordIdsOriginal[k]);
        DocId docId = list2._docIds[k];
        result._docIds.push_back(docId);
        result._scores.push_back(list2._scores[k]);
        result._positions.push_back(list2._positions[k]);
        // If there is a posting with SPECIAL_WORD_ID and this DocId,
        // also MOVE (copy & erase) it to the result.
        ScoreVector vec = specialPostingsMap[docId];
        if (vec.size() > 0)
        {
          for (size_t i = 0; i < vec.size(); ++i)
          {
            result._wordIdsOriginal.push_back(SPECIAL_WORD_ID);
            result._docIds.push_back(docId);
            result._scores.push_back(vec[i]);
            result._positions.push_back(SPECIAL_POSITION);
          }
          specialPostingsMap.erase(docId);
        }
      }
    }
  }// end iteration over list2 (the longer list)
  //          GlobalTimers::intersectWordlistsTimer3.stop();

#ifndef NDEBUG
  cout << "! going through list1 for second time" << endl;
#endif
  //          GlobalTimers::intersectWordlistsTimer4.cont();
  // GO THROUGH LIST1 (the shorter list) for the 2nd time
  for (k = 0; k < list1._wordIdsOriginal.size(); ++k)
  {
    if ((list1._wordIdsOriginal[k] <= max)
        && (list1._wordIdsOriginal[k] >= min))
    {
      if (bitvector2[list1._wordIdsOriginal[k] - min] == true)
      {
        // MATCH FOUND IN LIST1
        result._wordIdsOriginal.push_back(
            list1._wordIdsOriginal[k]);

        DocId docId = list1._docIds[k];
        result._docIds.push_back(docId);
        result._scores.push_back(list1._scores[k]);
        result._positions.push_back(list1._positions[k]);

        // If there is a posting with SPECIAL_WORD_ID and this DocId,
        // also MOVE (copy & erase) it to the result.
        ScoreVector vec = specialPostingsMap[docId];
        if (vec.size() > 0)
        {
          for (size_t i = 0; i < vec.size(); ++i)
          {
            result._wordIdsOriginal.push_back(SPECIAL_WORD_ID);
            result._docIds.push_back(docId);
            result._scores.push_back(vec[i]);
            result._positions.push_back(SPECIAL_POSITION);
          }
          specialPostingsMap.erase(docId);
        }
      }
    }
  }// end second iteration over list1 (the shorter list)
  //          GlobalTimers::intersectWordlistsTimer4.stop();
} // end: "hash" intersection with bit vectors

template<>
void CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>::doMergeJoin(
    QueryResult input1, QueryResult input2, QueryResult& result)
{
  // We need to treat postings if the SPECIAL_WORD_ID differently,
  // because we do not want to join on them.
  typedef vector<Score> ScoreVector;
  __gnu_cxx ::hash_map<DocId, ScoreVector> specialPostingsMap;

  //
  // SORT INPUT LISTS BY WORD ID (but sort doc ids in parallel)
  //
  //   If we're using scores/positions also have to sort the other parallel lists
  //
  // TODO: PASS THIS TIMER AS ARGUMENT
  //          GlobalTimers::intersectWordlistsTimer1.cont();
  if (input1._scores.size() > 0)
  { // begin case: using scores
    assert(input2._scores.size() > 0);
    if (input1._positions.size() > 0)
    {
      input1._wordIdsOriginal.sortParallel(
          input1._docIds, input1._scores,
          input1._positions);
      input2._wordIdsOriginal.sortParallel(
          input2._docIds, input2._scores,
          input2._positions);
    }
    else
    {
      input1._wordIdsOriginal.sortParallel(
          input1._docIds, input1._scores);
      input2._wordIdsOriginal.sortParallel(
          input2._docIds, input2._scores);
    }
  } // end case: using scores
  else
  { // begin case: NOT using scores
    assert(input2._scores.size() == 0);
    if (input1._positions.size() > 0)
    {
      input1._wordIdsOriginal.sortParallel(
          input1._docIds, input1._positions);
      input2._wordIdsOriginal.sortParallel(
          input2._docIds, input2._positions);
    }
    else
    {
      input1._wordIdsOriginal.sortParallel(
          input1._docIds);
      input2._wordIdsOriginal.sortParallel(
          input2._docIds);
    }
  } // end case: NOT using scores
  //          GlobalTimers::intersectWordlistsTimer1.stop();

  //          GlobalTimers::intersectWordlistsTimer2.cont();
  // ADD SENTINEL
#define SUPER_MAX_WORD_ID 2147483647
  input1._wordIdsOriginal.push_back(SUPER_MAX_WORD_ID);
  input2._wordIdsOriginal.push_back(SUPER_MAX_WORD_ID);

  // CHECK INPUT LISTS
  assert(input1._wordIdsOriginal.isSorted());
  assert(input2._wordIdsOriginal.isSorted());
  assert(result._wordIdsOriginal.size() == 0);
  //          GlobalTimers::intersectWordlistsTimer2.stop();

  register unsigned long i = 0, j = 0;

  // NEW 11.Jan 11 (Bjoern):
  // Before intersecting the word lists, remove all postings of
  // the special word. They should be kept and merged
  // later on.
  while (input1._wordIdsOriginal[i] == SPECIAL_WORD_ID)
  {

    specialPostingsMap[input1._docIds[i]].push_back(
        input1._scores[i]);
    ++i;
  }
  while (input2._wordIdsOriginal[j] == SPECIAL_WORD_ID)
  {
    specialPostingsMap[input2._docIds[j]].push_back(
        input2._scores[i]);
    ++j;
  }

  WordId lastMatch = -1;

  //          GlobalTimers::intersectWordlistsTimer3.cont();
  //
  // MAIN INTERSECTION LOOP
  //
  while (true)
  {
    // CASE: WENT TO FAR IN SECOND LIST (doc id in first list is smaller)
    if (input1._wordIdsOriginal[i]
        < input2._wordIdsOriginal[j])
    {
      if (input1._wordIdsOriginal[i] == lastMatch)
      {
        result._wordIdsOriginal.push_back(
            input1._wordIdsOriginal[i]);

        DocId docId = input1._docIds[i];
        result._docIds.push_back(docId);
        result._scores.push_back(input1._scores[i]);
        result._positions.push_back(input1._positions[i]);

        // If there is a posting with SPECIAL_WORD_ID and this DocId,
        // also MOVE (copy & erase) it to the result.
        ScoreVector vec = specialPostingsMap[docId];
        if (vec.size() > 0)
        {
          for (size_t k = 0; k < vec.size(); ++k)
          {
            result._wordIdsOriginal.push_back(SPECIAL_WORD_ID);
            result._docIds.push_back(docId);
            result._scores.push_back(vec[k]);
            result._positions.push_back(SPECIAL_POSITION);
          }
          specialPostingsMap.erase(docId);
        }
      }
      ++i;
    }
    // CASE: WENT TO FAR IN FIRST LIST (doc id in second list is smaller)

    else if (input1._wordIdsOriginal[i]
        > input2._wordIdsOriginal[j])
    {
      if (input2._wordIdsOriginal[j] == lastMatch)
      {
        result._wordIdsOriginal.push_back(
            input2._wordIdsOriginal[j]); // push back TWICE !

        DocId docId = input2._docIds[j];
        result._docIds.push_back(docId);
        result._scores.push_back(input2._scores[j]);
        result._positions.push_back(input2._positions[j]);

        // If there is a posting with SPECIAL_WORD_ID and this DocId,
        // also MOVE (copy & erase) it to the result.
        ScoreVector vec = specialPostingsMap[docId];
        if (vec.size() > 0)
        {
          for (size_t k = 0; k < vec.size(); ++k)
          {
            result._wordIdsOriginal.push_back(SPECIAL_WORD_ID);
            result._docIds.push_back(docId);
            result._scores.push_back(vec[k]);
            result._positions.push_back(SPECIAL_POSITION);
          }
          specialPostingsMap.erase(docId);
        }
      }
      ++j;
    }
    // CASE: MATCH FOUND (same doc id in both lists)

    else
    {
      assert(input1._wordIdsOriginal[i] == input2._wordIdsOriginal[j]);
      if (input1._wordIdsOriginal[i] == SUPER_MAX_WORD_ID)
      {
        break;
      }

      lastMatch = input1._wordIdsOriginal[i];
      // WRITE BOTH PAIRS FROM BOTH LISTS TO OUTPUT
      result._wordIdsOriginal.push_back(
          input1._wordIdsOriginal[i]);
      result._wordIdsOriginal.push_back(
          input2._wordIdsOriginal[j]); // push back TWICE !

      DocId docId1 = input1._docIds[i];
      result._docIds.push_back(docId1);
      DocId docId2 = input2._docIds[j];
      result._docIds.push_back(docId2);

      result._scores.push_back(input1._scores[i]);
      result._scores.push_back(input2._scores[j]);

      result._positions.push_back(input1._positions[i]);
      result._positions.push_back(input2._positions[j]);

      // If there is a posting with SPECIAL_WORD_ID and this DocId,
      // also MOVE (copy & erase) it to the result.
      ScoreVector vec = specialPostingsMap[docId1];
      if (vec.size() > 0)
      {
        for (size_t k = 0; k < vec.size(); ++k)
        {
          result._wordIdsOriginal.push_back(SPECIAL_WORD_ID);
          result._docIds.push_back(docId1);
          result._scores.push_back(vec[k]);
          result._positions.push_back(SPECIAL_POSITION);
        }
        specialPostingsMap.erase(docId1);
      }
      vec = specialPostingsMap[docId2];
      if (vec.size() > 0)
      {
        for (size_t k = 0; k < vec.size(); ++k)
        {
          result._wordIdsOriginal.push_back(SPECIAL_WORD_ID);
          result._docIds.push_back(docId2);
          result._scores.push_back(vec[k]);
          result._positions.push_back(SPECIAL_POSITION);
        }
        specialPostingsMap.erase(docId2);
      }
      ++i;
      ++j;
    }
  }// END MAIN INTERSECTION LOOP
#undef SUPER_MAX_WORD_ID
  //          GlobalTimers::intersectWordlistsTimer3.stop();
}// end: linear intersection


//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template void
CompleterBase<WITH_SCORES + WITH_POS + WITH_DUPS>::joinTwoPostingLists(
    const QueryResult& input1, const QueryResult& input2, QueryResult& result,
    const int joinMethod);
