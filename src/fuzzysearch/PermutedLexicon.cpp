// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#include <math.h>
#include <ctype.h>
#include <cstdio>
#include <iostream>  // NOLINT
#include <iomanip>
#include <fstream>  // NOLINT
#include <vector>
#include <sstream>
#include <algorithm>
#include <string>
#include <utility>
#include <list>
#include <unordered_map>

#include "../fuzzysearch/PermutedLexicon.h"
#include "../server/Exception.h"

namespace FuzzySearch
{
using std::cout;
using std::fstream;

template <class T> const vector<T>*
CyclicPermutationWithIndex<T>::strings = NULL;
template <class T> const vector<T>*
CyclicPermutationSubstringWithIndex<T>::strings = NULL;
vector<vector<int> > Mod::mod;

// ____________________________________________________________________________
static bool orderClosestWords(const pair<int, double>& x,
                              const pair<int, double>& y)
{
  if (x.second < y.second)
    return true;
  else
    return false;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::init(int mode, double threshold)
{
  FuzzySearchAlgorithm<T>::_mode = mode;
  _threshold = threshold;
  Mod::init();
  if (FuzzySearchAlgorithm<T>::_mode == 3)
  {
    FuzzySearchAlgorithm<T>::_isCompletionDistanceUsed = true;
  }
  else
  {
    FuzzySearchAlgorithm<T>::_isCompletionDistanceUsed = false;
  }
  nofDistanceComputations = 0;
  thr1 = 1;
  thr2 = 2;
  _fixedThreshold = -1;
  _cutOff = 50;
  _maxPrefixQueryLength = 5;
  _fuzzyWordMatchingExact = false;
  initPermutedLexiconFilters();
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestWords(const T& query,
                                          const vector<T>& vocabulary,
                                          const vector<T>& indexedWords,
                                          bool* queryIsInLexicon,
                                          vector<int>* closestWordsIds,
                                          vector<double>* distances)
{
  if (FuzzySearchAlgorithm<T>::_mode <= 1)
  {
    findClosestWordsPermutedLexiconPart(query, vocabulary, indexedWords,
        _cutOff, queryIsInLexicon, closestWordsIds, distances);
  }
  else
  {
    findClosestCompletions(query, indexedWords, closestWordsIds, distances);
    if (queryIsInLexicon != NULL)
      *queryIsInLexicon = FuzzySearchAlgorithm<T>::inVocabulary(query,
          *_vocabulary);
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestWordsPermutedLexicon(
                                const T& query,
                                const vector<T>& vocabulary,
                                const vector<T>& clusterCenters,
                                int cutOff,
                                bool* queryIsInLexicon,
                                vector<int>* closestWordsIds,
                                vector<double>* distances)
{
  if (query.length() > MAX_WORD_LEN)
    return;
  if (_permutedLexiconWords.size() == 0)
    return;
  const int range = _permutedLexiconWords.size();   // maximum depth
  int beginPos;
  int endPos;
  int cut;
  int correctWordId;
  int maxLength;
  int prefixLength;
  int currentPrefixLength;
  unsigned char index;
  double difference;
  double dist;
  pair<int, double> clId;

  if (wordsSeen.size() == 0)
    wordsSeen.resize(MAX_WORD_LEN);
  if (wordsSeen1.size() == 0)
    wordsSeen1.resize(MAX_WORD_LEN);

  // 1. initialize the needed simple data structures + book-keeping
  vector<pair<int, double> > temp;
  CS_ASSERT(closestWordsIds != NULL);
  (*closestWordsIds).clear();
  CS_ASSERT(distances != NULL);
  (*distances).clear();
  _cyclicPermutation.wordPtr = &query;
  int len1 = query.length();
  if (_alreadyCalculated.size() < clusterCenters.size())
    _alreadyCalculated.resize(clusterCenters.size());
  for (unsigned k = 0; k < _seenWords.size(); k++)
  {
    _alreadyCalculated[_seenWords[k]].first = false;
    _alreadyCalculated[_seenWords[k]].second.first = false;
    _alreadyCalculated[_seenWords[k]].second.second = false;
  }
  _seenWords.clear();
  for (uint32_t j = 0; j < query.length(); j++)
  {
    index = static_cast<unsigned char>(query[j]);
    _letterCounts[index]++;
    _letterCountsBackup[index]++;
  }
  nofDistanceComputations = 0;
  if (queryIsInLexicon != NULL)
    (*queryIsInLexicon) = this->inVocabulary(query, vocabulary);

  // 2. the actual work (finding approximate matches) starts here
  int minPrefixLength;
  if (FuzzySearchAlgorithm<T>::_mode == 0)
  {
    minPrefixLength = _minPrefix[query.length()];
  }
  else
  {
    minPrefixLength = _minPrefix25dist[query.length()];
    if (_fixedThreshold >= 0)
      _threshold = _fixedThreshold;
    else
    {
      if (query.length() >= 11)
        _threshold = 3;
      else
        if (query.length() >= 6)
          _threshold = 2;
        else
          _threshold = 1;
    }
  }
  wordsSeen1[minPrefixLength]++;
  for (size_t i = 0; i < query.length(); i++)
  {
    _cyclicPermutation.shift = i;

    // 2.a binary search in the permuted lexicon for the current permutation
    int low  = 0;
    int high = _permutedLexiconWords.size() - 1;
    int mid  = 0;

    while (low < high)
    {
      mid = (low + high) / 2;
      if (CyclicPermutationWithIndex<T>::
          comparator1(_cyclicPermutation, _permutedLexiconWords[mid]))
        low = mid + 1;
      else
        high = mid;
    }
    mid = low;

    const T& s1 = clusterCenters[_permutedLexiconWords[mid].wordIndex];
    const T& s2 = *_cyclicPermutation.wordPtr;

    prefixLength = 0;
    int16_t len = MY_MIN(s1.length(), s2.length());
    for (int j = 0; j < len; j++)
      if (s1[Mod::calc(j + _permutedLexiconWords[mid].shift, s1.length())] ==
          s2[Mod::calc(j + i, s2.length())])
        prefixLength++;
      else
        break;

    if (mid > 0)
    {
      const T& s3 = clusterCenters[_permutedLexiconWords[mid-1].wordIndex];
      if (prefixLength < static_cast<int>(s3.length()))
      {
        int prefixLength1 = prefixLength;
        len = MY_MIN(s2.length(), s3.length());
        for (int j = prefixLength; j < len; j++)
          if (s3[Mod::calc(j + _permutedLexiconWords[mid-1].shift, s3.length())] == s2[Mod::calc(j + i, s2.length())])  // NOLINT
            prefixLength1++;
          else
            break;
        if (prefixLength1 > prefixLength)
        {
          mid--;
          prefixLength = prefixLength1;
        }
      }
    }

    // 2.b explore the neighborhood of the current permutation downwards
    currentPrefixLength = prefixLength;
    cut = 0;
    endPos = MY_MIN(static_cast<int>(_permutedLexiconWords.size() - 1),
        mid + range);
    for (int j = mid; j <= endPos; j++)
    {
      // if (j > mid)
      //   currentPrefixLength = MY_MIN(currentPrefixLength,
      // _prefixLengths[j]);

      if (_prefixLengths[j] != currentPrefixLength)
      {
        currentPrefixLength = MY_MIN(currentPrefixLength, _prefixLengths[j]);
        if (minPrefixLength > currentPrefixLength)
          break;
      }
      wordsSeen[minPrefixLength]++;
      correctWordId = _permutedLexiconWords[j].wordIndex;
      if (_alreadyCalculated[correctWordId].first)
      {
        if (!_fuzzyWordMatchingExact)
        {
          if (_alreadyCalculated[correctWordId].second.first)
            cut = 0;
          else
            if (!_alreadyCalculated[correctWordId].second.second)
              cut++;
        }
        continue;
      }

        // execute this block if plain edit distance is used
        int len2 = clusterCenters[correctWordId].length();
        difference = abs(static_cast<int>(len1 - len2));
        maxLength = MY_MAX(len1, len2);
        // length filter
        // if (difference <= MAX_ED && difference / maxLength <= _threshold)
        if (difference <= _threshold)
        {
          int a;
          int16_t permShift = abs(static_cast<int>(_permutedLexiconWords[j].shift  // NOLINT
              - _cyclicPermutation.shift));
          // if (permShift > MAX_ED || 1.0 * permShift / maxLength
          //    > _threshold)
          if (permShift > _threshold)
            continue;
          if (FuzzySearchAlgorithm<T>::_mode == 0)
            a = applyFilters(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength);
          else
            a = commonCharsFilter(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength,
                maxLength - _threshold);
          if (a > 0)
          {
            nofDistanceComputations++;
            // if ((query.length() <= 32) && (clusterCenters[correctWordId].length() <= 32))  // NOLINT
            //  dist = myersed.calculate(query, clusterCenters[correctWordId], false);  // NOLINT
            // else
            dist = _ld.calculate(query, clusterCenters[correctWordId],
                _threshold);
            if (dist <= _threshold)
            {
              _similarPairs++;
              cut = 0;
              if (dist != 0 || queryIsInLexicon != NULL)
              {
                _alreadyCalculated[correctWordId].first  = true;
                _alreadyCalculated[correctWordId].second.first = true;
                _seenWords.push_back(correctWordId);
                clId.first  = correctWordId;
                clId.second = dist;
                temp.push_back(clId);
              }
            }
            else
            {
              _alreadyCalculated[correctWordId].first  = true;
              _seenWords.push_back(correctWordId);
              if (!_fuzzyWordMatchingExact)
              {
                cut++;
                if (cut > cutOff)
                  break;
              }
            }
          }
          else
          {
            // if (a < 0)
            //   break;
            if (!_fuzzyWordMatchingExact)
            {
              cut++;
              if (cut > cutOff)
                break;
            }
          }
        }
        else
        {
          // _alreadyCalculated[correctWordId].first  = true;
          // _alreadyCalculated[correctWordId].second.second = true;
          // _seenWords.push_back(correctWordId);
          // cut++;
        }
    }

    // 2.c explore the neighborhood of the current permutation upwards
    cut = 0;

    beginPos = MY_MAX(0, mid - range);
    currentPrefixLength = prefixLength;
    for (int j = mid; j >= beginPos; j--)
    {
      // if (j < mid)
      //  currentPrefixLength = MY_MIN(currentPrefixLength,
      // _prefixLengths[j+1]);

      if (_prefixLengths[j+1] != currentPrefixLength)
      {
        currentPrefixLength = MY_MIN(currentPrefixLength, _prefixLengths[j+1]);
        if (minPrefixLength > currentPrefixLength)
          break;
      }
      wordsSeen[minPrefixLength]++;
      correctWordId = _permutedLexiconWords[j].wordIndex;
      if (_alreadyCalculated[correctWordId].first)
      {
        if (!_fuzzyWordMatchingExact)
        {
          if (_alreadyCalculated[correctWordId].second.first)
            cut = 0;
          else
            if (!_alreadyCalculated[correctWordId].second.second)
              cut++;
        }
        continue;
      }

        // execute this block if plain edit distance is used
        int len2 = clusterCenters[correctWordId].length();
        difference = abs(static_cast<int>(len1 - len2));
        maxLength = MY_MAX(len1, len2);
        // length filter
        // if (difference <= MAX_ED && difference / maxLength <= _threshold)
        if (difference <= _threshold)
        {
          int a;
          int16_t permShift = abs(static_cast<int>(_permutedLexiconWords[j].shift  // NOLINT
              - _cyclicPermutation.shift));
          // if (permShift > MAX_ED || 1.0 * permShift / maxLength > _threshold)
          if (permShift > _threshold)
            continue;
          if (FuzzySearchAlgorithm<T>::_mode == 0)
            a = applyFilters(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength);
          else
            a = commonCharsFilter(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength,
                maxLength - _threshold);
          if (a > 0)
          {
            nofDistanceComputations++;
            // if ((query.length() <= 32)
            // && (clusterCenters[correctWordId].length() <= 32))
            //  dist = myersed.calculate(query,
            // clusterCenters[correctWordId], false);
            // else
            dist = _ld.calculate(query, clusterCenters[correctWordId],
                _threshold);
            if (dist <= _threshold)
            {
              _similarPairs++;
              cut = 0;
              if (dist != 0 || queryIsInLexicon != NULL)
              {
                _alreadyCalculated[correctWordId].first  = true;
                _alreadyCalculated[correctWordId].second.first = true;
                _seenWords.push_back(correctWordId);
                clId.first  = correctWordId;
                clId.second = dist;
                temp.push_back(clId);
              }
            }
            else
            {
              _alreadyCalculated[correctWordId].first  = true;
              _seenWords.push_back(correctWordId);
              if (!_fuzzyWordMatchingExact)
              {
                cut++;
                if (cut > cutOff)
                  break;
              }
            }
          }
          else
          {
            // if (a < 0)
            //  break;
            if (!_fuzzyWordMatchingExact)
            {
              cut++;
              if (cut > cutOff)
                break;
            }
          }
        }
        else
        {
          // _alreadyCalculated[correctWordId].first  = true;
          // _seenWords.push_back(correctWordId);
          // _alreadyCalculated[correctWordId].second.second = true;
          // cut++;
        }
    }
  }

  for (uint32_t j = 0; j < query.length(); j++)
  {
    index = static_cast<unsigned char>(query[j]);
    _letterCounts[index] = 0;
    _letterCountsBackup[index] = 0;
  }

  // 3. sort the matches w.r.t. edit distance and put them in two separate
  sort(temp.begin(), temp.end(), orderClosestWords);
  for (unsigned int  i = 0; i < temp.size(); i++)
  {
    closestWordsIds -> push_back(temp[i].first);
    distances -> push_back(temp[i].second);
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestWordsPermutedLexiconPart(
                                const T& query,
                                const vector<T>& vocabulary,
                                const vector<T>& clusterCenters,
                                int cutOff,
                                bool* queryIsInLexicon,
                                vector<int>* closestWordsIds,
                                vector<double>* distances)
{
  if (query.length() > MAX_WORD_LEN)
    return;
  if (_permutedLexiconWords.size() == 0)
    return;
  const int range = _permutedLexiconWords.size();   // maximum depth
  int beginPos;
  int endPos;
  int cut;
  int correctWordId;
  int maxLength;
  int prefixLength;
  int currentPrefixLength;
  unsigned char index;
  double difference;
  double dist;
  pair<int, double> clId;

  if (wordsSeen.size() == 0)
    wordsSeen.resize(MAX_WORD_LEN);
  if (wordsSeen1.size() == 0)
    wordsSeen1.resize(MAX_WORD_LEN);

  // 1. initialize the needed simple data structures + book-keeping
  vector<pair<int, double> > temp;
  CS_ASSERT(closestWordsIds != NULL);
  (*closestWordsIds).clear();
  CS_ASSERT(distances != NULL);
  (*distances).clear();
  _cyclicPermutation.wordPtr = &query;
  int len1 = query.length();
  if (_alreadyCalculated.size() < clusterCenters.size())
    _alreadyCalculated.resize(clusterCenters.size());
  for (unsigned k = 0; k < _seenWords.size(); k++)
  {
    _alreadyCalculated[_seenWords[k]].first = false;
    _alreadyCalculated[_seenWords[k]].second.first = false;
    _alreadyCalculated[_seenWords[k]].second.second = false;
  }
  _seenWords.clear();
  for (uint32_t j = 0; j < query.length(); j++)
  {
    index = static_cast<unsigned char>(query[j]);
    _letterCounts[index]++;
    _letterCountsBackup[index]++;
  }
  nofDistanceComputations = 0;
  if (queryIsInLexicon != NULL)
    (*queryIsInLexicon) = this->inVocabulary(query, vocabulary);

  // 2. the actual work (finding approximate matches) starts here
  int minPrefixLength;
  if (FuzzySearchAlgorithm<T>::_mode == 0)
  {
    minPrefixLength = _minPrefix[query.length()];
  }
  else
  {
    minPrefixLength = _minPrefix25dist[query.length()];
    if (_fixedThreshold >= 0)
      _threshold = _fixedThreshold;
    else
    {
      if (query.length() >= 11)
        _threshold = 3;
      else
        if (query.length() >= 6)
          _threshold = 2;
        else
          _threshold = 1;
    }
  }
  wordsSeen1[minPrefixLength]++;

  vector<int> partPositions;
  vector<int> partLens;
  int parts = _threshold + 1;
  int l = query.length() / parts;
  bool useLCS = false;
  if (_threshold <= 1 || minPrefixLength > l)
    useLCS = true;
  else
  {
    partLens.resize(parts, l);
    partPositions.resize(parts, 0);
    if (query.length() % parts == 0)
    {
      for (size_t i = 1; i < partPositions.size(); i++)
        partPositions[i] = l + partPositions[i - 1];
    }
    else
    {
      int r = query.length() % parts;
      int i = 0;
      while (r > 0)
      {
        partLens[i] += 1;
        ++i;
        --r;
      }
      for (size_t i = 1; i < partPositions.size(); i++)
        partPositions[i] = partLens[i - 1] + partPositions[i - 1];
    }
  }

  size_t nn = partPositions.size();
  if (useLCS)
    nn = query.length();
  for (size_t ii = 0; ii < nn; ii++)
  {
    size_t i;
    if (useLCS)
    {
      i = ii;
    }
    else
    {
      i = partPositions[ii];
      minPrefixLength = partLens[ii];
    }
    _cyclicPermutation.shift = i;

    // 2.a binary search in the permuted lexicon for the current permutation
    int low  = 0;
    int high = _permutedLexiconWords.size() - 1;
    int mid  = 0;

    while (low < high)
    {
      mid = (low + high) / 2;
      if (CyclicPermutationWithIndex<T>::
          comparator1(_cyclicPermutation, _permutedLexiconWords[mid]))
        low = mid + 1;
      else
        high = mid;
    }
    mid = low;

    const T& s1 = clusterCenters[_permutedLexiconWords[mid].wordIndex];
    const T& s2 = *_cyclicPermutation.wordPtr;

    prefixLength = 0;
    int16_t len = MY_MIN(s1.length(), s2.length());
    for (int j = 0; j < len; j++)
      if (s1[Mod::calc(j + _permutedLexiconWords[mid].shift, s1.length())] ==
          s2[Mod::calc(j + i, s2.length())])
        prefixLength++;
      else
        break;

    if (mid > 0)
    {
      const T& s3 = clusterCenters[_permutedLexiconWords[mid-1].wordIndex];
      if (prefixLength < static_cast<int>(s3.length()))
      {
        int prefixLength1 = prefixLength;
        len = MY_MIN(s2.length(), s3.length());
        for (int j = prefixLength; j < len; j++)
          if (s3[Mod::calc(j + _permutedLexiconWords[mid-1].shift, s3.length())] == s2[Mod::calc(j + i, s2.length())])  // NOLINT
            prefixLength1++;
          else
            break;
        if (prefixLength1 > prefixLength)
        {
          mid--;
          prefixLength = prefixLength1;
        }
      }
    }

    // 2.b explore the neighborhood of the current permutation downwards
    currentPrefixLength = prefixLength;
    cut = 0;
    endPos = MY_MIN(static_cast<int>(_permutedLexiconWords.size() - 1),
        mid + range);
    for (int j = mid; j <= endPos; j++)
    {
      // if (j > mid)
      //   currentPrefixLength = MY_MIN(currentPrefixLength,
      // _prefixLengths[j]);

      if (_prefixLengths[j] != currentPrefixLength)
      {
        currentPrefixLength = MY_MIN(currentPrefixLength, _prefixLengths[j]);
        if (minPrefixLength > currentPrefixLength)
          break;
      }
      wordsSeen[minPrefixLength]++;
      correctWordId = _permutedLexiconWords[j].wordIndex;
      if (_alreadyCalculated[correctWordId].first)
      {
        if (!_fuzzyWordMatchingExact)
        {
          if (_alreadyCalculated[correctWordId].second.first)
            cut = 0;
          else
            if (!_alreadyCalculated[correctWordId].second.second)
              cut++;
        }
        continue;
      }

        // execute this block if plain edit distance is used
        int len2 = clusterCenters[correctWordId].length();
        difference = abs(static_cast<int>(len1 - len2));
        maxLength = MY_MAX(len1, len2);
        // length filter
        // if (difference <= MAX_ED && difference / maxLength <= _threshold)
        if (difference <= _threshold)
        {
          int a;
          int16_t permShift = abs(static_cast<int>(_permutedLexiconWords[j].shift  // NOLINT
              - _cyclicPermutation.shift));
          // if (permShift > MAX_ED || 1.0 * permShift / maxLength
          //    > _threshold)
          if (permShift > _threshold)
            continue;
          if (FuzzySearchAlgorithm<T>::_mode == 0)
            a = applyFilters(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength);
          else
            a = commonCharsFilter(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength,
                maxLength - _threshold);
          if (a > 0)
          {
            /*
            dist = _threshold + 1;
            if (_threshold <= 2)
              if (maxLength - currentPrefixLength <= _threshold)
              {
                if (((_cyclicPermutation.shift + currentPrefixLength) % len1 == currentPrefixLength)
                    && (_permutedLexiconWords[j].shift + currentPrefixLength) % len2  == currentPrefixLength)
                  dist = maxLength - currentPrefixLength;
                else
                  if (((_cyclicPermutation.shift) % len1 == 0) &&
                      (_permutedLexiconWords[j].shift % len2 == 0))
                    dist = maxLength - currentPrefixLength;
                  else
                    if (query[0] == clusterCenters[_permutedLexiconWords[j].wordIndex][0])
                      dist = maxLength - currentPrefixLength;
              }
            */
            nofDistanceComputations++;
            // if ((query.length() <= 32)
            // && (clusterCenters[correctWordId].length() <= 32))  // NOLINT
            //  dist = myersed.calculate(query, clusterCenters[correctWordId], false);  // NOLINT
            // else
            dist = _ld.calculate(query, clusterCenters[correctWordId],
                _threshold);
            if (dist <= _threshold)
            {
              _similarPairs++;
              cut = 0;
              if (dist != 0 || queryIsInLexicon != NULL)
              {
                _alreadyCalculated[correctWordId].first  = true;
                _alreadyCalculated[correctWordId].second.first = true;
                _seenWords.push_back(correctWordId);
                clId.first  = correctWordId;
                clId.second = dist;
                temp.push_back(clId);
              }
            }
            else
            {
              _alreadyCalculated[correctWordId].first  = true;
              _seenWords.push_back(correctWordId);
              if (!_fuzzyWordMatchingExact)
              {
                cut++;
                if (cut > cutOff)
                  break;
              }
            }
          }
          else
          {
            // if (a < 0)
            //   break;
            if (!_fuzzyWordMatchingExact)
            {
              cut++;
              if (cut > cutOff)
                break;
            }
          }
        }
        else
        {
          // _alreadyCalculated[correctWordId].first  = true;
          // _alreadyCalculated[correctWordId].second.second = true;
          // _seenWords.push_back(correctWordId);
          // cut++;
        }
    }

    // 2.c explore the neighborhood of the current permutation upwards
    cut = 0;

    beginPos = MY_MAX(0, mid - range);
    currentPrefixLength = prefixLength;
    for (int j = mid; j >= beginPos; j--)
    {
      // if (j < mid)
      //  currentPrefixLength = MY_MIN(currentPrefixLength,
      // _prefixLengths[j+1]);

      if (_prefixLengths[j+1] != currentPrefixLength)
      {
        currentPrefixLength = MY_MIN(currentPrefixLength, _prefixLengths[j+1]);
        if (minPrefixLength > currentPrefixLength)
          break;
      }
      wordsSeen[minPrefixLength]++;
      correctWordId = _permutedLexiconWords[j].wordIndex;
      if (_alreadyCalculated[correctWordId].first)
      {
        if (!_fuzzyWordMatchingExact)
        {
          if (_alreadyCalculated[correctWordId].second.first)
            cut = 0;
          else
            if (!_alreadyCalculated[correctWordId].second.second)
              cut++;
        }
        continue;
      }

        // execute this block if plain edit distance is used
        int len2 = clusterCenters[correctWordId].length();
        difference = abs(static_cast<int>(len1 - len2));
        maxLength = MY_MAX(len1, len2);
        // length filter
        // if (difference <= MAX_ED && difference / maxLength <= _threshold)
        if (difference <= _threshold)
        {
          int a;
          int16_t permShift = abs(static_cast<int>(_permutedLexiconWords[j].shift  // NOLINT
              - _cyclicPermutation.shift));
          // if (permShift > MAX_ED || 1.0 * permShift / maxLength > _threshold)
          if (permShift > _threshold)
            continue;
          if (FuzzySearchAlgorithm<T>::_mode == 0)
            a = applyFilters(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength);
          else
            a = commonCharsFilter(_cyclicPermutation, _permutedLexiconWords,
                clusterCenters, j, currentPrefixLength, maxLength,
                maxLength - _threshold);
          if (a > 0)
          {
            /*
            dist = _threshold + 1;
            if (_threshold <= 2)
              if (maxLength - currentPrefixLength <= _threshold)
              {
                if (((_cyclicPermutation.shift + currentPrefixLength) % len1
                     == currentPrefixLength)
                    && (_permutedLexiconWords[j].shift + currentPrefixLength)
                    % len2 == currentPrefixLength)
                  dist = maxLength - currentPrefixLength;
                else
                  if (((_cyclicPermutation.shift) % len1 == 0) &&
                      (_permutedLexiconWords[j].shift % len2 == 0))
                    dist = maxLength - currentPrefixLength;
                  else
                    if (query[0]
                     == clusterCenters[_permutedLexiconWords[j].wordIndex][0])
                      dist = maxLength - currentPrefixLength;
              }
            */
            nofDistanceComputations++;
            // if ((query.length() <= 32)
            // && (clusterCenters[correctWordId].length() <= 32))  // NOLINT
            //  dist = myersed.calculate(query,
            // clusterCenters[correctWordId], false);  // NOLINT
            // else
            dist = _ld.calculate(query, clusterCenters[correctWordId],
                _threshold);
            if (dist <= _threshold)
            {
              _similarPairs++;
              cut = 0;
              if (dist != 0 || queryIsInLexicon != NULL)
              {
                _alreadyCalculated[correctWordId].first  = true;
                _alreadyCalculated[correctWordId].second.first = true;
                _seenWords.push_back(correctWordId);
                clId.first  = correctWordId;
                clId.second = dist;
                temp.push_back(clId);
              }
            }
            else
            {
              _alreadyCalculated[correctWordId].first  = true;
              _seenWords.push_back(correctWordId);
              if (!_fuzzyWordMatchingExact)
              {
                cut++;
                if (cut > cutOff)
                  break;
              }
            }
          }
          else
          {
            // if (a < 0)
            //  break;
            if (!_fuzzyWordMatchingExact)
            {
              cut++;
              if (cut > cutOff)
                break;
            }
          }
        }
        else
        {
          // _alreadyCalculated[correctWordId].first  = true;
          // _seenWords.push_back(correctWordId);
          // _alreadyCalculated[correctWordId].second.second = true;
          // cut++;
        }
    }
  }

  for (size_t j = 0; j < query.length(); j++)
  {
    index = static_cast<unsigned char>(query[j]);
    _letterCounts[index] = 0;
    _letterCountsBackup[index] = 0;
  }

  // 3. sort the matches w.r.t. edit distance and put them in two separate
  sort(temp.begin(), temp.end(), orderClosestWords);
  for (size_t i = 0; i < temp.size(); i++)
  {
    closestWordsIds -> push_back(temp[i].first);
    distances -> push_back(temp[i].second);
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestCompletionsInc(
    const T& query,
    const vector<T>& clusterCenters,
    vector<int>* closestWordsIds,
    vector<double>* distances,
    vector<int>* indexes)
{
  closestWordsIds->clear();
  distances->clear();
  int thr = 1;
  if (query.length() > 5)
    thr = 2;
  thr = 2;
  nofDistanceComputations = 0;
  double dist;
  vector<int> newIndexes;
  if (indexes->size() == 0)
  {
    indexes->resize(tempResults.size());
    for (size_t i = 0; i < tempResults.size(); i++)
      (*indexes)[i] = i;
  }
  newIndexes.reserve(indexes->size());
  /*
  int prefixLength;
  Result& result0 = tempResults[(*indexes)[0]];
  nofDistanceComputations++;
  dist = _extDistanceCalculator.calculate_next<T>(query,
    clusterCenters[result0.wordId], result0.lastRow);
  if (dist > thr)
  {
    result0.valid = false;
  }
  else
  {
    closestWordsIds->push_back(result0.wordId);
    distances->push_back(dist);
  }
  */
  for (size_t i = 0; i < indexes->size(); i++)
  {
    Result& result = tempResults[(*indexes)[i]];
    // if (result.valid)
    // {
    //   assert(_resultPrefixLengths.size() > i);
    //   if (tempResults[i-1].valid &&
    //       (_resultPrefixLengths[i] >= query.length() ||
    //       (_resultPrefixLengths[i] == query.length() &&
    //       (tempResults[i-1].dist == result.dist))))
    //   {
    //     result.dist = tempResults[i-1].dist;
    //     result.lastRow = tempResults[i-1].lastRow;
    //     closestWordsIds->push_back(result.wordId);
    //     distances->push_back(dist);
    //   }
    //   else
    //   {
        nofDistanceComputations++;
        dist = _extDistanceCalculator.calculate_next<T>(query,
          clusterCenters[result.wordId], result.lastRow);
        if (dist <= thr)
        {
          closestWordsIds->push_back(result.wordId);
          distances->push_back(dist);
          newIndexes.push_back((*indexes)[i]);
        }
        // if (dist > thr)
        // {
        //   result.valid = false;
        // }
        // else
        // {
        //   closestWordsIds->push_back(result.wordId);
        //   distances->push_back(dist);
        //   newIndexes.push_back((*indexes)[i]);
        // }
    //   }
    // }
  }
  *indexes = newIndexes;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::calculatePrefixLengths(
    const vector<T>& clusterCenters,
    const vector<Result>& results)
{
  _resultPrefixLengths.resize(results.size());
  _resultPrefixLengths[0] = 0;
  for (size_t i = 0; i < results.size() - 1; i++)
  {
    const T& s1 = clusterCenters[results[i].wordId];
    const T& s2 = clusterCenters[results[i+1].wordId];
    int length = MY_MIN(s1.length(), s2.length());
    _resultPrefixLengths[i+1] = 0;
    for (int j = 0; j < length; j++)
    {
      if (s1[j] == s2[j])
        _resultPrefixLengths[i+1]++;
      else
        break;
    }
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestCompletions(
    const T& query,
    const vector<T>& clusterCenters,
    vector<int>* closestWordsIds,
    vector<double>* distances,
    vector<Result>* results)
{
  size_t thr = query.length() > 5 ? 2 : 1;
  thr = 2;
  closestWordsIds->clear();
  distances->clear();
  results->clear();
  _matches.clear();
  _clearAlreadyCalculated = true;
  _calledFromAnotherFun = true;
  int len = query.length();
  if (query.length() > _maxPrefixQueryLength)
    len = _maxPrefixQueryLength;
  size_t beg = MY_MAX(3, len - thr);
  size_t end = len + 1;
  for (size_t i = beg; i <= end; i++)
  {
      _vecPrefixLengthsIndex = i-3;
      if (_vecPrefixLengthsIndex >= 0 && _vecPrefixLengthsIndex
          < _permutedLexiconsCompletions.size())
      {
        assert(_permutedLexiconsCompletions[i].size() > 0);
        findClosestCompletions(query,
            _permutedLexiconsCompletions[_vecPrefixLengthsIndex],
            clusterCenters, closestWordsIds, distances, results);
        _clearAlreadyCalculated = false;
      }
  }
  _clearAlreadyCalculated = true;
  _calledFromAnotherFun = false;
  // sort(_matches.begin(), _matches.end(), orderClosestWords);
  // calculatePrefixLengths(clusterCenters, *results);
  tempResults = *results;
  for (unsigned int  i = 0; i < _matches.size(); i++)
  {
    closestWordsIds->push_back(_matches[i].first);
    distances->push_back(_matches[i].second);
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestCompletions(
    const T& query,
    const vector<CyclicPermutationSubstringWithIndex<T> >& permutedLexicon,
    const vector<T>& clusterCenters,
    vector<int>* closestWordsIds,
    vector<double>* distances,
    vector<Result>* results)
{
    const int range = 500000;   // maximum depth
    int beginPos;
    int endPos;
    size_t correctWordId;
    double dist;

    // 1. initialize the needed simple data structures + book-keeping
    CS_ASSERT(closestWordsIds != NULL);
    CS_ASSERT(distances != NULL);
    _cyclicPermutation.wordPtr = &query;

    if (_clearAlreadyCalculated)
    {
      if (_alreadyCalculated.size() < clusterCenters.size())
        _alreadyCalculated.resize(clusterCenters.size());
      for (size_t k = 0; k < _seenWords.size(); k++)
        _alreadyCalculated[_seenWords[k]].first = false;
      _seenWords.clear();
      (*closestWordsIds).clear();
      (*distances).clear();
      _matches.clear();
      results->clear();
      nofDistanceComputations = 0;
    }

    const vector<uint8_t>& prefixLengths =
        _vecPrefixLengths[_vecPrefixLengthsIndex];
    Result tempResult;
    tempResult.valid = true;
    if (query.length() > 5)
    {
      _threshold = 2;
      tempResult.lastRow.resize(5);
    }
    else
    {
      _threshold = 1;
      tempResult.lastRow.resize(3);
    }
    _threshold = 2;
    tempResult.lastRow.resize(5);

    // 2. the actual work (finding approximate matches) starts here
    for (uint16_t i = _skipFirst[query.length()]; i < query.length(); i++)
    {
      _cyclicPermutation.shift = i;

      // cutOff = checkForCommonTrigram(_cyclicPermutation);

      // 2.a binary search in the permuted lexicon for the current permutation
      int low  = 0;
      int high = permutedLexicon.size() - 1;
      int mid  = 0;
      while (low < high)
      {
        mid = (low + high) / 2;
        if (CyclicPermutationSubstringWithIndex<T>::
            comparator1(_cyclicPermutation, permutedLexicon[mid]))
          low = mid + 1;
        else
          high = mid;
      }
      mid = low;

      // ------------------
      const T& s1 = clusterCenters[permutedLexicon[mid].wordIndex];
      const T& s2 = *_cyclicPermutation.wordPtr;

      int prefixLength;
      uint8_t currentPrefixLength;

      prefixLength = 0;
      int16_t len = MY_MIN(s1.length(), s2.length());
      for (int j = 0; j < len; j++)
        if (s1[Mod::calc(j + permutedLexicon[mid].beginning
            + permutedLexicon[mid].shift, permutedLexicon[mid].len)] ==
            s2[Mod::calc(j + i, s2.length())])
          prefixLength++;
        else
          break;

      if (mid > 0)
      {
        const T& s3 = clusterCenters[permutedLexicon[mid-1].wordIndex];
        if (prefixLength < static_cast<int>(s3.length()))
        {
          int prefixLength1 = prefixLength;
          len = MY_MIN(s2.length(), s3.length());
          for (int j = prefixLength; j < len; j++)
            if (s3[Mod::calc(j + permutedLexicon[mid-1].beginning
                + permutedLexicon[mid-1].shift, permutedLexicon[mid-1].len)] ==
                s2[Mod::calc(j + i, s2.length())])
              prefixLength1++;
            else
              break;
          if (prefixLength1 > prefixLength)
          {
            mid--;
            prefixLength = prefixLength1;
          }
        }
      }
      // ------------------
      currentPrefixLength = prefixLength;
      endPos = MY_MIN(static_cast<int>(permutedLexicon.size() - 1),
          mid + range);
      for (int j = mid; j <= endPos; j++)
      {
        if (j > mid && prefixLengths[j] != currentPrefixLength)
        {
          currentPrefixLength = MY_MIN(currentPrefixLength, prefixLengths[j]);
          if (currentPrefixLength < _minPrefix25dist[query.length()])
            break;
        }

        if (permutedLexicon[j].len > _threshold + query.length())
          continue;

        int16_t permShift = abs(static_cast<int>(permutedLexicon[j].shift
            - _cyclicPermutation.shift));
        if (permShift > _threshold)
          continue;

        correctWordId = permutedLexicon[j].wordIndex;
        if (_alreadyCalculated[correctWordId].first)
          continue;
        nofDistanceComputations++;
        dist = _extDistanceCalculator.calculate<T>(query,
            clusterCenters[correctWordId], _threshold, tempResult.lastRow);
        if (dist <= _threshold)
        {
          includeWord(correctWordId, dist);
          tempResult.wordId = correctWordId;
          tempResult.dist = dist;
          results->push_back(tempResult);
        }
        else
        {
          _alreadyCalculated[correctWordId].first  = true;
          _seenWords.push_back(correctWordId);
        }
        size_t size = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();  // NOLINT
        if (size > 0)
        {
          size_t beg = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
          size_t end = size > 1 ? _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :  // NOLINT
              _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
          for (correctWordId = beg; correctWordId <= end; correctWordId++)
          {
            if (_alreadyCalculated[correctWordId].first)
              continue;
            nofDistanceComputations++;
            dist = _extDistanceCalculator.calculate<T>(query,
                clusterCenters[correctWordId], _threshold, tempResult.lastRow);
            if (dist <= _threshold)
            {
              includeWord(correctWordId, dist);
              tempResult.wordId = correctWordId;
              tempResult.dist = dist;
              results->push_back(tempResult);
            }
            else
            {
              _alreadyCalculated[correctWordId].first  = true;
              _seenWords.push_back(correctWordId);
            }
          }
        }
      }

      // 2.c explore the neighborhood of the current permutation upwards
      currentPrefixLength = prefixLength;
      beginPos = MY_MAX(0, mid - range);
      for (int j = mid; j >= beginPos; j--)
      {
        if (j < mid && prefixLengths[j+1] != currentPrefixLength)
        {
          currentPrefixLength = MY_MIN(currentPrefixLength, prefixLengths[j+1]);
          if (currentPrefixLength < _minPrefix25dist[query.length()])
            break;
        }

        if (permutedLexicon[j].len > _threshold + query.length())
          continue;

        int16_t permShift = abs(static_cast<int>(permutedLexicon[j].shift
            - _cyclicPermutation.shift));
        if (permShift > _threshold)
          continue;

        correctWordId = permutedLexicon[j].wordIndex;
        if (_alreadyCalculated[correctWordId].first)
          continue;
        nofDistanceComputations++;
        dist = _extDistanceCalculator.calculate<T>(query,
            clusterCenters[correctWordId], _threshold, tempResult.lastRow);
        if (dist <= _threshold)
        {
          includeWord(correctWordId, dist);
          tempResult.wordId = correctWordId;
          tempResult.dist = dist;
          results->push_back(tempResult);
        }
        else
        {
          _alreadyCalculated[correctWordId].first  = true;
          _seenWords.push_back(correctWordId);
        }
        size_t size = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();  // NOLINT
        if (size > 0)
        {
          size_t beg = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
          size_t end = size > 1 ? _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :  // NOLINT
              _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
          for (correctWordId = beg; correctWordId <= end; correctWordId++)
          {
            if (_alreadyCalculated[correctWordId].first)
              continue;
            nofDistanceComputations++;
            dist = _extDistanceCalculator.calculate<T>(query,
                clusterCenters[correctWordId], _threshold, tempResult.lastRow);
            if (dist <= _threshold)
            {
              includeWord(correctWordId, dist);
              tempResult.wordId = correctWordId;
              tempResult.dist = dist;
              results->push_back(tempResult);
            }
            else
            {
              _alreadyCalculated[correctWordId].first = true;
              _seenWords.push_back(correctWordId);
            }
          }
        }
      }
    }
    if (!_calledFromAnotherFun)
    {
      // sort(_matches.begin(), _matches.end(), orderClosestWords);
      for (unsigned int  i = 0; i < _matches.size(); i++)
      {
        closestWordsIds -> push_back(_matches[i].first);
        distances -> push_back(_matches[i].second);
      }
    }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestCompletions(
    const T& query,
    const vector<T>& clusterCenters,
    vector<int>* closestWordsIds,
    vector<double>* distances)
{
  size_t thr = query.length() > 5 ? 2 : 1;
  thr = 1;
  closestWordsIds->clear();
  distances->clear();
  _matches.clear();
  _clearAlreadyCalculated = true;
  _calledFromAnotherFun = true;
  int len = query.length();
  if (query.length() > _maxPrefixQueryLength)
    len = _maxPrefixQueryLength;
  size_t beg = MY_MAX(3, len - thr);
  size_t end = len + 1;
  for (size_t i = beg; i <= end; i++)
  {
      _vecPrefixLengthsIndex = i-3;
      if (_vecPrefixLengthsIndex >= 0 && _vecPrefixLengthsIndex
          < _permutedLexiconsCompletions.size())
      {
        CS_ASSERT_GT(_permutedLexiconsCompletions[i].size(), 0);
        findClosestCompletions(query,
            _permutedLexiconsCompletions[_vecPrefixLengthsIndex],
            clusterCenters, closestWordsIds, distances);
        _clearAlreadyCalculated = false;
      }
  }
  _clearAlreadyCalculated = true;
  _calledFromAnotherFun = false;
  sort(_matches.begin(), _matches.end(), orderClosestWords);
  for (unsigned int  i = 0; i < _matches.size(); i++)
  {
    closestWordsIds->push_back(_matches[i].first);
    distances->push_back(_matches[i].second);
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::findClosestCompletions(
    const T& query,
    const vector<CyclicPermutationSubstringWithIndex<T> >& permutedLexicon,
    const vector<T>& clusterCenters,
    vector<int>* closestWordsIds,
    vector<double>* distances)
{
  const int range = 500000;   // maximum depth
  int beginPos;
  int endPos;
  size_t correctWordId;
  double dist;

  // 1. initialize the needed simple data structures + book-keeping
  CS_ASSERT(closestWordsIds != NULL);
  CS_ASSERT(distances != NULL);

  T queryx;
  if (query.length() > 10)
    _threshold = 3;
  else
  if (query.length() > 5)
    _threshold = thr2;
  else
    _threshold = thr1;
  _threshold = 1;
  if (query.length() > _maxPrefixQueryLength)
    queryx = query.substr(0, _maxPrefixQueryLength);
  else
    queryx = query;

  _cyclicPermutation.wordPtr = &queryx;

  if (_clearAlreadyCalculated)
  {
    if (_alreadyCalculated.size() < clusterCenters.size())
      _alreadyCalculated.resize(clusterCenters.size());
    for (size_t k = 0; k < _seenWords.size(); k++)
      _alreadyCalculated[_seenWords[k]].first = false;
    _seenWords.clear();
    (*closestWordsIds).clear();
    (*distances).clear();
    _matches.clear();
    nofDistanceComputations = 0;
  }
  int pos = 0;

  const vector<uint8_t>& prefixLengths = _vecPrefixLengths[_vecPrefixLengthsIndex];  // NOLINT

  // 2. the actual work (finding approximate matches) starts here
  for (uint16_t i = _skipFirst[queryx.length()]; i < queryx.length(); i++)
  {
    _cyclicPermutation.shift = i;

    // 2.a binary search in the permuted lexicon for the current permutation
    int low  = 0;
    int high = permutedLexicon.size() - 1;
    int mid  = 0;
    while (low < high)
    {
      mid = (low + high) / 2;
      if (CyclicPermutationSubstringWithIndex<T>::
          comparator1(_cyclicPermutation, permutedLexicon[mid]))
        low = mid + 1;
      else
        high = mid;
    }
    mid = low;

    // ------------------
    const T& s1 = clusterCenters[permutedLexicon[mid].wordIndex];
    const T& s2 = *_cyclicPermutation.wordPtr;

    int prefixLength;
    uint8_t currentPrefixLength;

    prefixLength = 0;
    int16_t len = MY_MIN(s1.length(), s2.length());
    for (int j = 0; j < len; j++)
      if (s1[Mod::calc(j + permutedLexicon[mid].beginning +
          permutedLexicon[mid].shift, permutedLexicon[mid].len)] ==
          s2[Mod::calc(j + i, s2.length())])
        prefixLength++;
      else
        break;

    if (mid > 0)
    {
      const T& s3 = clusterCenters[permutedLexicon[mid-1].wordIndex];
      if (prefixLength < static_cast<int>(s3.length()))
      {
        int prefixLength1 = prefixLength;
        len = MY_MIN(s2.length(), s3.length());
        for (int j = prefixLength; j < len; j++)
          if (s3[Mod::calc(j + permutedLexicon[mid-1].beginning +
              permutedLexicon[mid-1].shift, permutedLexicon[mid-1].len)] ==
              s2[Mod::calc(j + i, s2.length())])
            prefixLength1++;
          else
            break;
        if (prefixLength1 > prefixLength)
        {
          mid--;
          prefixLength = prefixLength1;
        }
      }
    }
    // ------------------

    currentPrefixLength = prefixLength;
    endPos = MY_MIN(static_cast<int>(permutedLexicon.size() - 1), mid + range);
    for (int j = mid; j <= endPos; j++)
    {
      // if (j > mid)
      //  currentPrefixLength = MY_MIN(currentPrefixLength, prefixLengths[j]);
      //  currentPrefixLength = prefixLengths[j];

      if (j > mid && prefixLengths[j] != currentPrefixLength)
      {
        currentPrefixLength = MY_MIN(currentPrefixLength, prefixLengths[j]);
        if (currentPrefixLength < _minPrefix25dist[queryx.length()])
          break;
      }

      // if (permutedLexicon[j].len > _threshold + query.length())
      //   continue;

      int16_t permShift = abs(static_cast<int>(permutedLexicon[j].shift
          - _cyclicPermutation.shift));
      if (permShift > _threshold)
        continue;

      int lendiff = abs(static_cast<int>(currentPrefixLength
                - MY_MAX(queryx.length(), permutedLexicon[j].len)));
      if (_threshold <= 2 && lendiff <= _threshold)
      {
        // if (permShift > 0 && currentPrefixLength < query.length())
        //   continue;

        // if (permShift == 0 && currentPrefixLength >= queryx.length() - 1)
        //  if (_cyclicPermutation.shift + currentPrefixLength
        //      <= static_cast<int>(queryx.length()) &&
        //      permutedLexicon[j].shift + currentPrefixLength
        //      <= permutedLexicon[j].len)
          if (static_cast<int>(queryx.length() - _cyclicPermutation.shift) ==
              permutedLexicon[j].len - permutedLexicon[j].shift)
          {
            correctWordId = permutedLexicon[j].wordIndex;
            dist = lendiff;
            if (_alreadyCalculated[correctWordId].first)
              continue;

            includeWord(correctWordId, dist);
            size_t size = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();  // NOLINT
            if (size > 0)
            {
              size_t beg = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];  // NOLINT
              size_t end = size > 1 ? _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :  // NOLINT
                  _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
              for (correctWordId = beg; correctWordId <= end; correctWordId++)
              {
                // correctWordId =
                // _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][k];
                if (_alreadyCalculated[correctWordId].first)
                  continue;
                includeWord(correctWordId, dist);
              }
            }
            continue;
          }
      }

      correctWordId = permutedLexicon[j].wordIndex;
      if (_alreadyCalculated[correctWordId].first)
        continue;

      // here filter ...

      nofDistanceComputations++;
      dist = _extDistanceCalculator.calculate<T>(query,
          clusterCenters[correctWordId], _threshold, pos);
      if (dist <= _threshold)
        includeWord(correctWordId, dist);
      else
      {
        _alreadyCalculated[correctWordId].first  = true;
        _seenWords.push_back(correctWordId);
      }
      size_t size = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();
      if (size > 0)
      {
        size_t beg = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
        size_t end = size > 1 ? _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :  // NOLINT
            _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
        for (correctWordId = beg; correctWordId <= end; correctWordId++)
        {
          if (_alreadyCalculated[correctWordId].first)
            continue;
          nofDistanceComputations++;
          dist = _extDistanceCalculator.calculate<T>(query,
              clusterCenters[correctWordId], _threshold, pos);
          if (dist <= _threshold)
            includeWord(correctWordId, dist);
          else
          {
            _alreadyCalculated[correctWordId].first  = true;
            _seenWords.push_back(correctWordId);
          }
        }
      }
    }

    // 2.c explore the neighborhood of the current permutation upwards
    currentPrefixLength = prefixLength;
    beginPos = MY_MAX(0, mid - range);
    for (int j = mid; j >= beginPos; j--)
    {
      // if (j < mid)
        // currentPrefixLength = MY_MIN(currentPrefixLength,
        // prefixLengths[j+1]);
        // currentPrefixLength = prefixLengths[j];

      if (j < mid && prefixLengths[j+1] != currentPrefixLength)
      {
        currentPrefixLength = MY_MIN(currentPrefixLength, prefixLengths[j+1]);
        if (currentPrefixLength < _minPrefix25dist[queryx.length()])
          break;
      }

      // if (permutedLexicon[j].len > _threshold + query.length())
      //  continue;

      int16_t permShift = abs(static_cast<int>(permutedLexicon[j].shift
          - _cyclicPermutation.shift));
      if (permShift > _threshold)
        continue;

      /*
      if (_threshold == 1)
      {
        // if (permShift > 0 && currentPrefixLength < query.length()-1)
        //   continue;

        if (permShift == 0 && currentPrefixLength >= queryx.length() - 1)
          if (_cyclicPermutation.shift + currentPrefixLength
              <= static_cast<int>(queryx.length()) &&
              permutedLexicon[j].shift + currentPrefixLength
              <= permutedLexicon[j].len)
          {
            dist = (currentPrefixLength >= queryx.length() ? 0 : 1);
            correctWordId = permutedLexicon[j].wordIndex;
            if (_alreadyCalculated[correctWordId].first)
              continue;
            includeWord(correctWordId, dist);
            size_t size =
             _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();
            if (size > 0)
            {
              size_t beg =
               _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
              size_t end = size > 1 ?
               _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :
                  _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
              for (correctWordId = beg; correctWordId <= end; correctWordId++)
              {
                // correctWordId =
                // _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][k];
                if (_alreadyCalculated[correctWordId].first)
                  continue;
                includeWord(correctWordId, dist);
              }
            }
            continue;
          }
      }
      */

      int lendiff = abs(static_cast<int>(currentPrefixLength
                - MY_MAX(queryx.length(), permutedLexicon[j].len)));
      if (_threshold <= 2 && lendiff <= _threshold)
      {
        // if (permShift > 0 && currentPrefixLength < query.length())
        //   continue;

        // if (permShift == 0 && currentPrefixLength >= queryx.length() - 1)
        //  if (_cyclicPermutation.shift + currentPrefixLength
        //      <= static_cast<int>(queryx.length()) &&
        //      permutedLexicon[j].shift + currentPrefixLength
        //      <= permutedLexicon[j].len)
          if (static_cast<int>(queryx.length() - _cyclicPermutation.shift) ==
              permutedLexicon[j].len - permutedLexicon[j].shift)
          {
            correctWordId = permutedLexicon[j].wordIndex;
            dist = lendiff;
            if (_alreadyCalculated[correctWordId].first)
              continue;
            includeWord(correctWordId, dist);

            size_t size = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();  // NOLINT
            if (size > 0)
            {
              size_t beg = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];  // NOLINT
              size_t end = size > 1 ? _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :  // NOLINT
                  _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
              for (correctWordId = beg; correctWordId <= end; correctWordId++)
              {
                // correctWordId =
                // _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][k];
                if (_alreadyCalculated[correctWordId].first)
                  continue;
                includeWord(correctWordId, dist);
              }
            }
            continue;
          }
      }

      correctWordId = permutedLexicon[j].wordIndex;
      if (_alreadyCalculated[correctWordId].first)
        continue;

      // here filter ...

      nofDistanceComputations++;
      dist = _extDistanceCalculator.calculate<T>(query,
          clusterCenters[correctWordId], _threshold, pos);
      if (dist <= _threshold)
        includeWord(correctWordId, dist);
      else
      {
        _alreadyCalculated[correctWordId].first  = true;
        _seenWords.push_back(correctWordId);
      }
      size_t size = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex].size();
      if (size > 0)
      {
        size_t beg = _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
        size_t end = size > 1 ? _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][1] :  // NOLINT
            _groupsOfWordIds[permutedLexicon[j].wordGroupIndex][0];
        for (correctWordId = beg; correctWordId <= end; correctWordId++)
        {
          if (_alreadyCalculated[correctWordId].first)
            continue;
          nofDistanceComputations++;
          dist = _extDistanceCalculator.calculate<T>(query,
              clusterCenters[correctWordId], _threshold, pos);
          if (dist <= _threshold)
            includeWord(correctWordId, dist);
          else
          {
            _alreadyCalculated[correctWordId].first = true;
            _seenWords.push_back(correctWordId);
          }
        }
      }
    }
  }
  if (!_calledFromAnotherFun)
  {
    sort(_matches.begin(), _matches.end(), orderClosestWords);
    for (unsigned int  i = 0; i < _matches.size(); i++)
    {
      closestWordsIds -> push_back(_matches[i].first);
      distances -> push_back(_matches[i].second);
    }
  }
}

// ____________________________________________________________________________
template <class T>
inline void PermutedLexicon<T>::includeWord(int correctWordId, double dist)
                                            // vector<int>* closestWordsIds)
{
  _similarPairs++;
  _alreadyCalculated[correctWordId].first  = true;
  // _alreadyCalculated[correctWordId].second.first = true;
  _seenWords.push_back(correctWordId);
  _match.first  = correctWordId;
  _match.second = dist;
  _matches.push_back(_match);
  // closestWordsIds->push_back(correctWordId);
}

// ____________________________________________________________________________
template <class T>
inline int PermutedLexicon<T>::applyFilters(
                            const CyclicPermutationWithPointer<T>& cyclicPerm,
                            const PermutedLexiconWords& permutedLexicon,
                            const vector<T>& clusterCenters,
                            uint32_t id2,
                            int p,
                            size_t maxLen)
{
  const T* s2 = &(clusterCenters[permutedLexicon[id2].wordIndex]);

  // substring length filter
  bool lcs = (p >= _minPrefixes[maxLen]);

  // common number of letters filter
  if (lcs)
  {
    unsigned char ind;
    uint16_t cmnLetters = p;
    for (uint32_t j = p; j < s2 -> length(); j++)
    {
      ind = static_cast<unsigned char>(s2 -> at(Mod::calc((j +
          permutedLexicon[id2].shift), s2 -> length())));
      if (_letterCounts[ind] > 0)
        cmnLetters++;
      _letterCounts[ind]--;
    }
    for (uint32_t j = p; j < s2 -> length(); j++)
    {
      ind = static_cast<unsigned char>(s2 -> at(Mod::calc((j +
          permutedLexicon[id2].shift), s2 -> length())));
      _letterCounts[ind] = _letterCountsBackup[ind];
    }
    if (cmnLetters >= maxLen - _threshold * maxLen)
      return true;
    else
      return false;
  }
  return false;
}

// ____________________________________________________________________________
template <class T>
inline bool PermutedLexicon<T>::commonCharsFilter(
                            const CyclicPermutationWithPointer<T>& cyclicPerm,
                            const PermutedLexiconWords& permutedLexicon,
                            const vector<T>& clusterCenters,
                            uint32_t id2,
                            int p,
                            size_t maxLen,
                            float minNofLetters)
{
  const T* s2 = &(clusterCenters[permutedLexicon[id2].wordIndex]);
  unsigned char ind;
  uint16_t cmnLetters = p;
  for (uint32_t j = p; j < s2 -> length(); j++)
  {
    ind = static_cast<unsigned char>(s2 -> at(Mod::calc((j +
        permutedLexicon[id2].shift), s2 -> length())));
    if (_letterCounts[ind] > 0)
      cmnLetters++;
    _letterCounts[ind]--;
  }
  for (uint32_t j = p; j < s2 -> length(); j++)
  {
    ind = static_cast<unsigned char>(s2 -> at(Mod::calc((j +
        permutedLexicon[id2].shift), s2 -> length())));
    _letterCounts[ind] = _letterCountsBackup[ind];
  }
  if (cmnLetters >= minNofLetters)
    return true;
  else
    return false;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::buildIndex(const vector<T>& vocabulary, bool reserved)
{
  _vocabulary = &vocabulary;
  if (FuzzySearchAlgorithm<T>::_mode <= 1)
  {
    PermutedLexiconWords tmp;
    _permutedLexiconWords = tmp;  // delete old permuted lexicon
    buildPermutedLexicon(vocabulary, 4, &_permutedLexiconWords);
  }
  else
  {
    vector<PermutedLexiconCompletions> tmp;
    _permutedLexiconsCompletions = tmp;  // delete old permuted lexicon
    buildPermutedLexicons(vocabulary, 3, 3, _maxPrefixQueryLength + 1);
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::buildPermutedLexicon(const vector<T>& clusterCenters,
                          const uint32_t minMissLength,
                          PermutedLexiconWords* permutedLexicon)
{
  cout << " [ Generating fuzzy word matching permuted lexicon: ]" << endl;
  Timer timer;
  timer.start();
  uint32_t totalPermutations = 0;
  T tempStr;
  cout << "  [ Creating the permuted words ... ]"  << endl;
  const vector<T>* vocabulary = &clusterCenters;
  CyclicPermutationWithIndex<T>::strings = &clusterCenters;
  for (uint32_t i = 0; i < (*vocabulary).size(); i++)
  {
    if ((*vocabulary)[i].length() < minMissLength)
      continue;
    totalPermutations += (*vocabulary)[i].length();
  }
  cout << "  [ Total permutations: " << totalPermutations
       << ". Raw lexicon size: "
       << totalPermutations * sizeof(CyclicPermutationWithIndex<T>)
       << " B ]"  << endl;
  CyclicPermutationWithIndex<T> tempPerm;
  permutedLexicon->clear();
  permutedLexicon->reserve(totalPermutations);
  for (uint32_t i = 0; i < (*vocabulary).size(); i++)
  {
    if ((*vocabulary)[i].length() < minMissLength)
      continue;
    for (uint32_t j = 0; j < (*vocabulary)[i].length(); j++)
    {
      tempPerm.init(i, j);
      permutedLexicon->push_back(tempPerm);
    }
  }
  cout << "  [ Sorting " << permutedLexicon->size() << " strings ... "
       << flush;
  stable_sort((*permutedLexicon).begin(), (*permutedLexicon).end(),
       CyclicPermutationWithIndex<T>::comparator);
  cout << "done. ]" << endl << flush;
  calculatePrefixLengths(clusterCenters, *permutedLexicon);
  timer.stop();
  cout << " [ done. (" << timer.secs() << " sec.) ]" << endl;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::calculatePrefixLengths(
                         const vector<T>& clusterCenters,
                         const PermutedLexiconWords& permutedLexicon)
{
  cout << "  [ Calculating permuted-lexicon prefix lengths ... " << flush;
  if (permutedLexicon.size() > 0)
  {
    _prefixLengths.resize(permutedLexicon.size());
    _prefixLengths[0] = 0;
    for (size_t i = 0; i < permutedLexicon.size() - 1; i++)
    {
      const T& s1 = clusterCenters[permutedLexicon[i].wordIndex];
      const T& s2 = clusterCenters[permutedLexicon[i+1].wordIndex];
      for (size_t j = 0; j < MY_MIN(s1.length(), s2.length()); j++)
      {
        if (s1[Mod::calc(j + permutedLexicon[i].shift, s1.length())] ==
            s2[Mod::calc(j + permutedLexicon[i+1].shift, s2.length())])
          _prefixLengths[i+1]++;
        else
          break;
      }
    }
  }
  cout << "done ]" << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::buildPermutedLexicons(
           const vector<T>& clusterCenters,
           const uint32_t minMissLength,
           const size_t minPrefixLen,
           const size_t maxPrefixLen)
{
  _permutedLexiconsCompletions.clear();
  _groupsOfWordIds.clear();
  _permutedLexiconsCompletions.resize(maxPrefixLen-minPrefixLen+1);
  _wordGroupsId = 0;
  size_t totalPerms = 0;
  size_t totalPermsActual = 0;
  cout << " [ Constructing permuted lexicons for fuzzy completion matching ]"
       << endl;
  for (size_t i = minPrefixLen; i <= maxPrefixLen; i++)
  {
    buildPermutedLexicon(clusterCenters, 4, i, i,
        &_permutedLexiconsCompletions[i-minPrefixLen]);
    totalPermsActual += _permutedLexiconsCompletions[i-minPrefixLen].size();
  }
  cout << endl;
  cout << "Total number of actual permutations : "
        << totalPermsActual << endl;
  for (size_t i = 0; i < _groupsOfWordIds.size(); i++)
     totalPerms += _groupsOfWordIds[i].size();
  cout << "Total number of permutations        : " << totalPerms << endl;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::buildPermutedLexicon(
           const vector<T>& clusterCenters,
           const uint32_t minMissLength,
           const size_t minPrefixLen,
           const size_t maxPrefixLen,
           vector<CyclicPermutationSubstringWithIndex<T> >* permutedLexicon)
{
  if (clusterCenters.size() == 0)
    return;
  Timer timer;
  timer.start();
  T tempStr;
  if (minPrefixLen == maxPrefixLen)
    cout << "  [ Prefix Length: " << maxPrefixLen << " ]" << endl;
  const vector<T>* vocabulary = &clusterCenters;
  CyclicPermutationSubstringWithIndex<T>::strings = &clusterCenters;
  size_t totalPermutations = 0;
  std::unordered_map<pair<T, int>, int, StringIntHash<T> > htt;
  CyclicPermutationSubstringWithIndex<T> tempPerm;
  pair<T, int> tmpPair;
  for (uint32_t i = 0; i < (*vocabulary).size(); i++)
  {
    if ((*vocabulary)[i].length() < minMissLength)
      continue;
    for (uint32_t j = minPrefixLen; j <= MY_MIN(maxPrefixLen,
        (*vocabulary)[i].length()); j++)
    {
      for (uint32_t k = 0; k <= j; k++)
      {
        tempPerm.init(i, k, 0, j);
        // tempPerm.words.clear();
        tmpPair.first = tempPerm.getPermAsString();
        tmpPair.second = k;
        if (htt.count(tmpPair) <= 0)
        {
          htt[tmpPair] = 0;
          totalPermutations++;
        }
        else
          htt[tmpPair]++;
      }
    }
  }
  if (htt.size() == 0)
    return;
  std::unordered_map<pair<T, int>, int, StringIntHash<T> > htt0;
  htt = htt0;
  permutedLexicon->clear();
  permutedLexicon->reserve(totalPermutations);

  std::unordered_map<pair<T, int>, int, StringIntHash<T> > ht;
  _groupsOfWordIds.reserve(_groupsOfWordIds.size()+totalPermutations);
  for (uint32_t i = 0; i < (*vocabulary).size(); i++)
  {
    if ((*vocabulary)[i].length() < minMissLength)
      continue;
    for (uint32_t j = minPrefixLen; j <= MY_MIN(maxPrefixLen,
        (*vocabulary)[i].length()); j++)
    {
      for (uint32_t k = 0; k <= j; k++)
      {
        tempPerm.init(i, k, 0, j);
        tmpPair.first = tempPerm.getPermAsString();
        tmpPair.second = k;
        if (ht.count(tmpPair) <= 0)
        {
          _groupsOfWordIds.resize(_wordGroupsId + 1);
          tempPerm.wordGroupIndex = _wordGroupsId;
          ht[tmpPair] = _wordGroupsId;
          permutedLexicon->push_back(tempPerm);
          _wordGroupsId++;
        }
        else
        {
          vector<int>& array = _groupsOfWordIds[ht[tmpPair]];
          if (array.size() < 2)
            _groupsOfWordIds[ht[tmpPair]].push_back(i);
          else
            array[1]++;
        }
      }
    }
  }
  cout << "  [ Sorting " << permutedLexicon->size() << " strings ... "
       << flush;
  stable_sort((*permutedLexicon).begin(), (*permutedLexicon).end(),
      CyclicPermutationSubstringWithIndex<T>::comparator);
  timer.stop();
  cout << "done. ]" << endl;
  calculatePrefixLengths(clusterCenters, *permutedLexicon);
  cout << " [ Total time: " << timer.secs() << " sec. ]" << endl;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::calculatePrefixLengths(
    const vector<T>& clusterCenters,
    const vector<CyclicPermutationSubstringWithIndex<T> >& permutedLexicon)
{
  cout << "  [ Calculating permuted-lexicon prefix lengths ... " << flush;
  vector<uint8_t> prefixLengths;
  if (permutedLexicon.size() > 0)
  {
    prefixLengths.resize(permutedLexicon.size());
    prefixLengths[0] = 0;
    for (size_t i = 0; i < permutedLexicon.size() - 1; i++)
    {
      const T& s1 = clusterCenters[permutedLexicon[i].wordIndex];
      const T& s2 = clusterCenters[permutedLexicon[i+1].wordIndex];
      size_t length = MY_MIN(permutedLexicon[i].len, permutedLexicon[i+1].len);
      for (size_t j = 0; j < length; j++)
      {
        if (s1[Mod::calc(j + permutedLexicon[i].beginning +
            permutedLexicon[i].shift, permutedLexicon[i].len)] ==
            s2[Mod::calc(j + permutedLexicon[i+1].beginning +
                permutedLexicon[i+1].shift, permutedLexicon[i+1].len)])
          prefixLengths[i+1]++;
        else
          break;
      }
    }
    _vecPrefixLengths.push_back(prefixLengths);
  }
  cout << "done ]" << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::initPermutedLexiconFilters(int maxED)
{
  _minPrefixes.resize(MAX_WORD_LEN + 1);
  _minPrefix.resize(MAX_WORD_LEN + 1);
  _minPrefix25dist.resize(MAX_WORD_LEN + 1);
  for (uint32_t i = 0; i <= MAX_WORD_LEN; i++)
    _minPrefix[i] = INT_MAX;
  for (int i = 3; i <= MAX_WORD_LEN; i++)
  {
    _minPrefixes[i] = (int)(ceil(i - MY_MIN(maxED, floor(_threshold * i))) / MY_MIN(maxED, floor(_threshold * i)));  //NOLINT
    for (int k = i; k <= i + MY_MIN(maxED, _threshold * k); k++)
      _minPrefix[i] = MY_MIN(_minPrefix[i], (int)ceil((k - MY_MIN(maxED, floor(_threshold * k))) / MY_MIN(maxED, floor(_threshold * k))));  //NOLINT
  }
  if (_threshold == 0)  // set threshold to the default value if not already set
    _threshold = 0.28;
  _skipFirst.resize(MAX_WORD_LEN + 1);
  for (int i = 0; i <= 5; i++)
    _skipFirst[i] = 0;
  for (int i = 6; i <= MAX_WORD_LEN; i++)
    _skipFirst[i] = ((i - 2) % 2 == 0 ? 1 : 0);
  for (int i = 0; i <= MAX_WORD_LEN; i++)
  {
    int ed = thr1;
    if (i > 5)
      ed = thr2;
    if (i > 10)
      ed = 3;
    _minPrefix25dist[i] = ceil(1.0 * i / ed) - 1;
  }
  _seenWords.reserve(1000);
  _letterCountsBackup.resize(256);
  _letterCounts.resize(256);
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::setFixedThreshold(int threshold)
{
  if (threshold >= 0)
  {
    thr1 = thr2 = threshold;
    _fixedThreshold = threshold;
    for (int i = 0; i <= MAX_WORD_LEN; i++)
    {
      if (threshold > 0)
        _minPrefix25dist[i] = ceil(1.0 * i / _fixedThreshold) - 1;
      else
        _minPrefix25dist[i] = i;
    }
  }
  else
  {
    thr1 = 1;
    thr2 = 2;
    for (int i = 0; i <= MAX_WORD_LEN; i++)
    {
      int ed = thr1;
      if (i > 5)
        ed = thr2;
      if (i > 10)
        ed = 3;
      _minPrefix25dist[i] = ceil(1.0 * i / ed) - 1;
    }
  }
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::saveDataStructureToFile(const string& filename)
{
  FILE* outputFile = fopen(filename.c_str(), "w");
  if (FuzzySearchAlgorithm<T>::_mode <= 1)
  {
    if (_permutedLexiconWords.size() == 0)
      return;
    cout << "Saving fuzzy-search data structure to disk ... " << flush;
    fprintf(outputFile, "0\n%f\n%d\n%d\n%d\n", _threshold,
        FuzzySearchAlgorithm<T>::_mode,
        static_cast<int>(_vocabulary->size()),
        static_cast<int>(_permutedLexiconWords.size()));
    saveTheLexiconPart(outputFile, *_vocabulary);
    for (uint32_t i = 0; i < _permutedLexiconWords.size(); i++)
    {
      fprintf(outputFile, "%d\n%d\n", _permutedLexiconWords[i].wordIndex,
          (uint32_t)_permutedLexiconWords[i].shift);
    }
  }
  else
  {
    if (_permutedLexiconsCompletions.size() == 0)
      return;
    cout << "Saving fuzzy-search data structure to disk ... " << flush;
    fprintf(outputFile, "0\n%f\n%d\n%d\n%d\n", _threshold,
        FuzzySearchAlgorithm<T>::_mode,
        static_cast<int>(_vocabulary->size()),
        static_cast<int>(_permutedLexiconsCompletions.size()));
    fprintf(outputFile, "%zu\n", _groupsOfWordIds.size());
    fprintf(outputFile, "%u\n", _maxPrefixQueryLength);
    saveTheLexiconPart(outputFile, *_vocabulary);
    for (size_t j = 0; j < _permutedLexiconsCompletions.size(); j++)
    {
      const PermutedLexiconCompletions& permutedLexicon =
          _permutedLexiconsCompletions[j];
      fprintf(outputFile, "%zu\n", permutedLexicon.size());
      for (uint32_t i = 0; i < permutedLexicon.size(); i++)
      {
        fprintf(outputFile, "%d %d ", permutedLexicon[i].wordIndex,
            (uint32_t)permutedLexicon[i].shift);
        fprintf(outputFile, "%d ", permutedLexicon[i].beginning);
        fprintf(outputFile, "%d ", permutedLexicon[i].len);
        fprintf(outputFile, "%d ", permutedLexicon[i].wordGroupIndex);
        int size = _groupsOfWordIds[permutedLexicon[i].wordGroupIndex].size();
        fprintf(outputFile, "%d", size);
        if (size == 1)
          fprintf(outputFile, " %d",
              _groupsOfWordIds[permutedLexicon[i].wordGroupIndex][0]);
        else
          if (size == 2)
            fprintf(outputFile, "  %d %d",
                _groupsOfWordIds[permutedLexicon[i].wordGroupIndex][0],
                _groupsOfWordIds[permutedLexicon[i].wordGroupIndex][1]);
        fprintf(outputFile, "\n");
      }
    }
  }
  fclose(outputFile);
  cout << "done." << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void PermutedLexicon<T>::loadDataStructureFromFile(const string& filename,
                                                   vector<T>*
                                                   clusterCenters)
{
  cout << "* Reading permuted lexicon data structure from \""
       << filename << "\" ... " << endl << flush;
  fstream inputFile;
  inputFile.open(filename.c_str(), ios::in);
  if (!inputFile.is_open())
  {
    perror("open: ");
    exit(1);
  }
  uint32_t readNum;
  double readDouble;
  T readStr;

  // get the type
  inputFile >> readNum;
  if (readNum != 0)
  {
    cerr << "ERROR: Wrong file format! Exiting ..." << endl;
    exit(1);
  }
  // get the threshold
  inputFile >> readDouble;
  double threshold = readDouble;

  // get the mode
  inputFile >> readNum;
  int mode = readNum;
  cout << "* The mode is: " << readNum << endl << flush;

  if (FuzzySearchAlgorithm<T>::_mode <= 1)
  {
    // 1. get the number of words in the lexicon and in the permuted lexicon
    clusterCenters->clear();
    _permutedLexiconWords.clear();
    inputFile >> readNum;
    clusterCenters->resize(readNum);
    inputFile >> readNum;
    _permutedLexiconWords.resize(readNum);

    // 2. read the lexicon (i.e. the cluster centroids)
    for (uint32_t i = 0; i < clusterCenters->size(); i++)
    {
      readLine(&inputFile, &readStr, false);
      (*clusterCenters)[i] = readStr;
    }

    // 3. read the permuted lexicon
    CyclicPermutationWithIndex<T> cyclicPerm;
    for (uint32_t i = 0; i < _permutedLexiconWords.size(); i++)
    {
      inputFile >> readNum;
      cyclicPerm.wordIndex = readNum;
      inputFile >> readNum;
      cyclicPerm.shift = (uint32_t)readNum;
      _permutedLexiconWords[i] = cyclicPerm;
    }
    CyclicPermutationWithIndex<T>::strings = clusterCenters;
    inputFile.close();
    string tempStr;
    cout << "* done (# centroids = " << clusterCenters->size() << ", "
         << "# cyclic perms = " << _permutedLexiconWords.size() <<  ") ";
    calculatePrefixLengths(*clusterCenters, _permutedLexiconWords);
    // checkPermutedLexiconIntegrity(*_permutedLexiconWords);
    cout << endl;
  }
  else
  {
    inputFile.close();
    _permutedLexiconsCompletions.clear();
    const int MAX_LENGTH = 1000000;  // it can sometimes be very very long!
    char buff[MAX_LENGTH + 2];
    stringstream sstr;
    int intValue;
    T stringValue;
    FILE* fin = fopen(filename.c_str(), "r");
    if (fin == NULL)
    {
      perror("fopen:");
      exit(1);
    }
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // type
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // thr
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // mode
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // voc
    sstr << buff;
    sstr >> intValue;
    clusterCenters->resize(intValue);
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // perm
    sstr << buff;
    sstr >> intValue;
    _permutedLexiconsCompletions.resize(intValue);
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // groups
    sstr << buff;
    sstr >> intValue;
    _groupsOfWordIds.resize(intValue);
    assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // _max...
    sstr << buff;
    sstr >> intValue;
    _maxPrefixQueryLength = intValue;

    // 1. read the lexicon
    for (uint32_t i = 0; i < clusterCenters->size(); i++)
    {
      readLine(fin, &stringValue, false);
      (*clusterCenters)[i] = stringValue;
    }

    size_t totalPerms = 0;
    for (size_t j = 0; j < _permutedLexiconsCompletions.size(); j++)
    {
      // 2. read the perm. lexicon
      // format: id, shift, beg, end, word-gr-index, nof word-ids, word-ids ...
      PermutedLexiconCompletions permutedLexicon;
      assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, fin) != NULL);  // perm
      sstr << buff;
      sstr >> intValue;
      totalPerms += intValue;
      permutedLexicon.resize(intValue);
      CyclicPermutationSubstringWithIndex<T> cyclicPerm;
      for (uint32_t i = 0; i < permutedLexicon.size(); i++)
      {
        assert(fgets(buff, MAX_LENGTH, fin) != NULL);
        stringstream sstr;
        sstr << buff;
        sstr >> intValue;
        // assert(intValue < static_cast<int>(permutedLexicon.size()));
        cyclicPerm.wordIndex = intValue;
        sstr >> intValue;
        cyclicPerm.shift = intValue;
        sstr >> intValue;
        cyclicPerm.beginning = intValue;
        sstr >> intValue;
        cyclicPerm.len = intValue;
        sstr >> intValue;
        cyclicPerm.wordGroupIndex = intValue;
        sstr >> intValue;
        int size = intValue;
        _groupsOfWordIds[cyclicPerm.wordGroupIndex].reserve(size);
        for (int j = 0; j < size; j++)
        {
          sstr >> intValue;
          assert(intValue < static_cast<int>(clusterCenters->size()));
          _groupsOfWordIds[cyclicPerm.wordGroupIndex].push_back(intValue);
        }
        permutedLexicon[i] = cyclicPerm;
      }
      calculatePrefixLengths(*clusterCenters, permutedLexicon);
      _permutedLexiconsCompletions[j] = permutedLexicon;
    }
    CyclicPermutationSubstringWithIndex<T>::strings = clusterCenters;
    _vocabulary = clusterCenters;
    cout << "* done (# centroids = " << clusterCenters->size() << ", "
         << "# cyclic perms = " << totalPerms <<  ") ";
    cout << endl;
  }
  init(mode, threshold);
}


// ____________________________________________________________________________
template <>
void PermutedLexicon<string>::saveTheLexiconPart(
                                 FILE* outputFile,
                                 const vector<string>& clusterCenters)
{
  for (uint32_t i = 0; i < clusterCenters.size(); i++)
  {
    fprintf(outputFile, "%s\n", clusterCenters[i].c_str());
  }
}

// ____________________________________________________________________________
template <>
void PermutedLexicon<wstring>::saveTheLexiconPart(
                                 FILE* outputFile,
                                 const vector<wstring>& clusterCenters)
{
  string tmpstr;
  for (uint32_t i = 0; i < clusterCenters.size(); i++)
  {
    wstring2string(clusterCenters[i], &tmpstr);
    fprintf(outputFile, "%s\n", tmpstr.c_str());
  }
}

// ____________________________________________________________________________
template <>
void PermutedLexicon<string>::readLine(FILE* f, string* line,
                                          bool toLower)
{
  char buff[CWF_MAX_LINE_LENGTH + 2];
  assert(fgets(buff, CWF_MAX_LINE_LENGTH + 2, f) != NULL);
  buff[strlen(buff) - 1] = 0;
  *line = buff;
  if (toLower)
  {
    char* p = (char*)line->c_str();  // NOLINT
    while (*p != 0) { p += utf8_tolower(p); }
  }
}

// ____________________________________________________________________________
template <>
void PermutedLexicon<wstring>::readLine(FILE* f, wstring* line,
                                           bool toLower)
{
  string linestr;
  PermutedLexicon<string>::readLine(f, &linestr, toLower);
  string2wstring(linestr, line);
}

// ____________________________________________________________________________
template <>
void PermutedLexicon<string>::readLine(fstream* f, string* line,
                                          bool toLower)
{
  *f >> *line;
  if (toLower)
  {
    char* p = (char*)line->c_str();  // NOLINT
    while (*p != 0) { p += utf8_tolower(p); }
  }
}

// ____________________________________________________________________________
template <>
void PermutedLexicon<wstring>::readLine(fstream* f, wstring* line,
                                           bool toLower)
{
  string linestr;
  PermutedLexicon<string>::readLine(f, &linestr, toLower);
  string2wstring(linestr, line);
}

// ____________________________________________________________________________
template<>
void PermutedLexicon<string>::writeLine(FILE* f, const string& query)
{
  fprintf(f, "%s\n", query.c_str());
}

// ____________________________________________________________________________
template<>
void PermutedLexicon<wstring>::writeLine(FILE* f, const wstring& query)
{
  string tmpstr;
  wstring2string(query, &tmpstr);
  fprintf(f, "%s\n", tmpstr.c_str());
}

// explicit instantiation
template class PermutedLexicon<std::string>;
template class PermutedLexicon<std::wstring>;
}
