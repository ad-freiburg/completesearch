// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>  // NOLINT
#include <fstream>  // NOLINT
#include <vector>
#include <algorithm>
#include <string>
#include <utility>
#include <list>
#include <sstream>

#include "../fuzzysearch/FastSS.h"
#include "../fuzzysearch/Timer.h"
#include "../fuzzysearch/WordClusteringBuilder.h"

namespace FuzzySearch
{
using std::string;
using std::wstring;
using std::cout;
using std::endl;
using std::flush;
using std::vector;

// ____________________________________________________________________________
template <class T>
void FastSS<T>::index(const T& str, uint16_t beg, int depth)
{
  if (str.length() == 0)
    return;
  // index only subseq. of length |w|-\delta
  // TODO(celikik): check if it works right w.r.t filters
  if (depth == _threshold || _shortWordIndexed)
  {
    vector<size_t>& list = _delNeigh[str];
    // if (list.size() == 0)
    //  _totalTransformationLength += str.length();
    if (list.size() > 0)
    {
      if (_currentPrefixRange != list.back())
        list.push_back(_currentPrefixRange);
    }
    else
      list.push_back(_currentPrefixRange);
    _totalPointers++;
  }
  if (depth >= _threshold)
    return;
  for (int i = beg; i < static_cast<int>(str.length()); i++)
  {
    T tempStr;
    tempStr.resize(str.length() - 1);
    int counter = 0;
    for (int j = 0; j < static_cast<int>(str.length()); j++)
    {
      if (j != i)
        tempStr[counter++] = str[j];
    }
    index(tempStr, i, depth + 1);
  }
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::calcSpace(const T& str, uint16_t beg, int depth)
{
  if (str.length() == 0)
    return;
  // index only subseq. of length |w|-\delta
  if (depth == _threshold || _shortWordIndexed)
  {
    vector<size_t>& list =_delNeigh[str];
    if (list.size() == 0)
    {
      list.resize(2);
      list[0] = 1;
      list[1] = _currentPrefixRange;
      // list[1] += MY_MAX(1, getPrefix(_currentPrefix));
      _totalTransformationLength += str.length();
    }
    else
    {
      if (_currentPrefixRange != list[1])
      {
        list[0]++;
        list[1] = _currentPrefixRange;
      }
      // list[1] += MY_MAX(1, getPrefix(_currentPrefix));
    }
  }
  if (depth >= _threshold)
    return;
  for (int i = beg; i < static_cast<int>(str.length()); i++)
  {
    T tempStr;
    tempStr.resize(str.length() - 1);
    int counter = 0;
    for (int j = 0; j < static_cast<int>(str.length()); j++)
    {
      if (j != i)
        tempStr[counter++] = str[j];
    }
    calcSpace(tempStr, i, depth + 1);
  }
}

/*
// ____________________________________________________________________________
template <class T>
void FastSS<T>::index(const T& str, int depth)
{
  if (str.length() == 0)
    return;
  <Prefix>& list = _delNeigh[str];
  // if (list.size() == 0)
  //  _totalTransformationLength += str.length();
  list.push_back(_currentPrefix);
  _totalPointers++;
  if (depth >= _threshold)
    return;
  for (int i = 0; i < static_cast<int>(str.length()); i++)
  {
    T tempStr;
    tempStr.resize(str.length() - 1);
    int counter = 0;
    for (int j = 0; j < static_cast<int>(str.length()); j++)
    {
      if (j != i)
        tempStr[counter++] = str[j];
    }
    index(tempStr, depth + 1);
  }
}
*/

// ____________________________________________________________________________
template <class T>
void FastSS<T>::index1(const T& str, int depth)
{
  /*
  if (str.length() == 0)
    return;
  int low  = 0;
  int high = _transformations.size() - 1;
  int mid  = 0;
  while (low < high)
  {
    mid = (low + high) / 2;
    if (str > _transformations[mid])
      low = mid + 1;
    else
      high = mid;
  }
  mid = low;
  vector<int>& list = _wordIds[mid];
  if (list.size() == 0)
    _totalTransformationLength += str.length();
  list.push_back(_currentWordId);
  _totalPointers++;
  if (depth >= _threshold)
    return;
  for (int i = 0; i < static_cast<int>(str.length()); i++)
  {
    T tempStr;
    tempStr.resize(str.length() - 1);
    int counter = 0;
    for (int j = 0; j < static_cast<int>(str.length()); j++)
    {
      if (j != i)
        tempStr[counter++] = str[j];
    }
    index1(tempStr, depth + 1);
  }
  */
}

// ____________________________________________________________________________
template <class T>
bool sortByStr(const pair<T, int>& x, const pair<T, int>& y)
{
  return x.first < y.first;
}

// ____________________________________________________________________________
bool sortByDouble(const pair<int, double>& x, const pair<int, double>& y)
{
  return x.second < y.second;
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::buildIndex(const vector<T>& vocabulary, bool reserveMemory)
{
  if (vocabulary.size() == 0)
  {
    std::cerr << "WARNING: Passed empty vocabulary for indexing "
        "to the FastSS object!" << endl;
    // exit(1);
  }
  if (vocabulary.size() > (1 << NOF_BITS_WORDID))
  {
    std::cerr << "ERROR: Vocabulary size larger than maximum supported "
              << "vocabulary size - " << (1 << NOF_BITS_WORDID) << " !"
              << std::endl;
    exit(1);
  }
  // do initializations of the required variables and arrays
  _vocabulary = &vocabulary;
  _seenWords1.resize(vocabulary.size());
  _seenWords2.reserve(100000);
  HashMap tmp;
  _delNeigh = tmp;  // delete the old hash table
  _delNeigh.rehash(vocabulary.size());  // NOTE(bast): was hash_map.resize
  cout << "[ Indexing started. ]" << endl;
  ProgressIndicator pi(_vocabulary->size(), 10);

  // index in mode 1 (word matching with fixed threshold, full d. neighborhood)
  if (_mode == FASTSS_WORD_MATCHING_FULL)
  {
    cout << " [ Mode: plain edit distance / full index ]" << endl;
    vector<PrefixRange> prefixes;
    computePrefixes(vocabulary, MAX_WORD_LEN, &prefixes);
    pi.setTotal(vocabulary.size());
    if (reserveMemory)
    {
      cout << " [ Counting memory ... " << flush;
      for (size_t j = 0; j < prefixes.size(); j++)
      {
        size_t wordId = getWordId(prefixes[j]);
        assert(wordId < vocabulary.size());
        const T& str = vocabulary[wordId];
        setDynThreshold(str.length());
        calculateSpace(str, prefixes[j]);
        pi.update(j);
      }
      cout << " ]" << endl << flush;
      cout << " [ Reserving memory ... ]" << endl << flush;
      this->reserveMemory();
    }
    cout << " [ Creating deletion neighborhoods ... " << flush;
    pi.clear();
    for (size_t j = 0; j < prefixes.size(); j++)
    {
      size_t wordId = getWordId(prefixes[j]);
      assert(wordId < vocabulary.size());
      const T& str = vocabulary[wordId];
      setDynThreshold(str.length());
      indexWord(str, prefixes[j]);
      pi.update(j);
    }
    cout << " ]" << endl << flush;
  }
  else
  // index in mode 2 (dynamic threshold with truncation / word matching)
  if (_mode == FASTSS_WORD_MATCHING_TRUNC)
  {
    cout << " [ Mode: dynamic threshold with truncation / word matching. ]"
         << endl;
    // vector<Prefix> prefixes;
    _prefixRanges.clear();
    computePrefixes(vocabulary, _truncateLength, &_prefixRanges);
    for (size_t i = 1; i < _prefixRanges.size(); i++)
      assert(_prefixRanges[i-1] != _prefixRanges[i]);
    size_t total = _prefixRanges.size();
    pi.setTotal(total);
    if (reserveMemory)
    {
      cout << " [ Counting memory ... " << flush;
      pi.clear();
      int counter = 0;
      for (size_t j = 0; j < _prefixRanges.size(); j++)
      {
        size_t wordId = getWordId(_prefixRanges[j]);
        assert(getWordId(_prefixRanges[j]) < vocabulary.size());
        const T& str = vocabulary[wordId];
        setDynThreshold(str.length());
        // if (str.length() > 8)
        //  _truncateLength = 7;
        // else
        //  _truncateLength = 6; // NOLINT
        calculateSpace(str.substr(0, MY_MIN(_truncateLength, str.length())),
            j);
        pi.update(counter++);
      }
      cout << " ]" << endl << flush;
      cout << " [ Reserving memory ... ]" << endl << flush;
      this->reserveMemory();
    }
    cout << " [ Creating deletion neighborhoods ... " << flush;
    pi.clear();
    int counter = 0;
    for (size_t j = 0; j < _prefixRanges.size(); j++)
    {
      size_t wordId = getWordId(_prefixRanges[j]);
      assert(wordId < vocabulary.size());
      const T& str = vocabulary[wordId];
      setDynThreshold(str.length());
      // if (str.length() > 8)
      //  _truncateLength = 7;
      // else
      //  _truncateLength = 6;
      if (j > 0)
        assert(_prefixRanges[j-1] != _prefixRanges[j]);
      indexWord(str.substr(0, MY_MIN(_truncateLength, str.length())),
          j);
      pi.update(counter++);
    }
    cout << " ]" << endl << flush;
  }
  else
  // index in mode 3 (dynamic distance threshold with truncation / prefix
  // matching.)
  if (_mode == FASTSS_COMPLETION_MATCHING)
  {
    // check if the vocabulary is sorted in alph. increasing order
    for (size_t i = 1; i < vocabulary.size(); i++)
      if (vocabulary[i-1] < vocabulary[i])
      {
        std::cerr << "WARNING: the vocabulary should be given in"
            " lexicographically increasing order!" << std::endl;
        break;
        // exit(1);
      }
    cout << " [ Mode: dynamic distance threshold with truncation /"
        " prefix matching. ]" << endl;
    _prefixRanges.clear();
    vector<unsigned char> truncLen;
    for (size_t i = 3; i < _truncateLength + 1; i++)
    {
      size_t prevSize = _prefixRanges.size();
      computePrefixes(vocabulary, i, &_prefixRanges);
      for (size_t j = prevSize; j < _prefixRanges.size(); j++)
        truncLen.push_back(i);
    }
    pi.setTotal(_prefixRanges.size());
    if (reserveMemory)
    {
      cout << " [ Counting memory ... " << flush;
      pi.clear();
      int counter = 0;
      for (size_t j = 0; j < _prefixRanges.size(); j++)
      {
        size_t wordId = getWordId(_prefixRanges[j]);
        assert(wordId < vocabulary.size());
        const T& str = vocabulary[wordId];
        setDynThreshold(str.length());
        assert(truncLen.size() > j);
        calculateSpace(str.substr(0, MY_MIN(truncLen[j], str.length())), j);
        pi.update(counter++);
      }
      cout << " ]" << endl << flush;
      cout << " [ Reserving memory ... ]" << endl << flush;
      this->reserveMemory();
    }
    cout << " [ Creating deletion neighborhoods ... " << flush;
    pi.clear();
    int counter = 0;
    for (size_t j = 0; j < _prefixRanges.size(); j++)
    {
      size_t wordId = getWordId(_prefixRanges[j]);
      assert(wordId < vocabulary.size());
      const T& str = vocabulary[wordId];
      setDynThreshold(str.length());
      indexWord(str.substr(0, MY_MIN(truncLen[j], str.length())), j);
      pi.update(counter++);
    }
    cout << " ]" << endl << flush;
    calculatePrefixLengths(vocabulary);
  }
  else
  {
    std::cerr << "Unknown mode!" << endl;
    exit(1);
  }
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::indexWord(const T& str, const PrefixRange& prefix)
{
  _currentPrefixRange = prefix;
  if (str.length() >= _truncateLength)
    _shortWordIndexed = false;
  else
    _shortWordIndexed = true;
  index(str, 0, 0);
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::indexWord1(const T& str, const PrefixRange& prefix)
{
  _currentPrefixRange = prefix;
  if (str.length() >= _truncateLength)
    _shortWordIndexed = false;
  else
    _shortWordIndexed = true;
  index1(str, 0);
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::calculateSpace(const T& str, const PrefixRange& prefix)
{
  _currentPrefixRange = prefix;
  if (str.length() >= _truncateLength)
    _shortWordIndexed = false;
  else
    _shortWordIndexed = true;
  calcSpace(str, 0, 0);
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::findClosestWords(const T& query,
                                 const vector<T>& vocabulary,
                                 const vector<T>& indexedWords,
                                 bool* queryIsInLexicon,
                                 vector<int>* closestWordsIds,
                                 vector<double>* distances)
{
  closestWordsIds->clear();
  distances->clear();
  if (_delNeigh.size() == 0)
    return;
  _matches.clear();
  for (size_t i = 0; i < _seenWords2.size(); i++)
    _seenWords1[_seenWords2[i]] = false;
  _seenWords2.clear();
  _queryString = &query;
  _lastI = -2;
  _lastMatched = false;
  _pos = 0;
  _prefixLength = INT_MAX;
  distanceCalcTimer.reset();
  nofDistanceComputations = 0;
  if (queryIsInLexicon != NULL)
    (*queryIsInLexicon) = this->inVocabulary(query, vocabulary);
  setDynThreshold(query.length());
  if (_mode == FASTSS_COMPLETION_MATCHING
      || _mode == FASTSS_WORD_MATCHING_TRUNC)
  {
    T str1 = query;
    // if (query.length() > 8)
    //  _truncateLength = 7;
    // else
    //  _truncateLength = 6;
    if (query.length() > _truncateLength)
    {
      str1 = query.substr(0, _truncateLength);
      for (size_t j = 4; j < query.length(); j++)
      {
        unsigned char index = static_cast<unsigned char>(query[j]);
        _letterCounts[index]++;
        _letterCountsBackup[index]++;
      }
    }
    findMatches(str1, 0, 0);
    if (query.length() > _truncateLength)
    {
      for (uint32_t j = 4; j < query.length(); j++)
      {
        unsigned char index = static_cast<unsigned char>(query[j]);
        _letterCounts[index] = 0;
        _letterCountsBackup[index] = 0;
      }
    }
  }
  else
    if (_mode == FASTSS_WORD_MATCHING_FULL)
    {
      findMatches(query, 0, 0);
    }
    else
    {
      std::cerr << "Unknown mode!" << endl;
      exit(1);
    }
  sort(_matches.begin(), _matches.end(), sortByDouble);
  for (size_t i = 0; i < _matches.size(); i++)
  {
    closestWordsIds->push_back(_matches[i].first);
    distances->push_back(_matches[i].second);
  }
}

// Code used for debugging due to string and wstring
// bool test(const string& s)
// {
//   return (s == "complexity");
// }
//
// bool test(const wstring& s)
// {
//   return (s == L"complexity");
// }
//
// void printStr(const string& s)
// {
//   std::cout << " " << s;
// }
//
// void printStr(const wstring& s)
// {
//   std::wcout << L" " << s;
// }

// ____________________________________________________________________________
template <class T>
void FastSS<T>::findMatches(const T& str, uint16_t beg, int depth)
{
  if (str.length() == 0)
    return;
  if (_delNeigh.count(str) > 0)
  {
    vector<size_t>& postingList = _delNeigh[str];
    double d = 0;
    if (_mode == FASTSS_COMPLETION_MATCHING)
    {
      for (size_t j = 0; j < postingList.size(); j++)
      {
        int beg = getWordId(_prefixRanges[postingList[j]]);
        int end = beg + getPrefix(_prefixRanges[postingList[j]]);
        for (int i = beg; i <= end; i++)
        {
          if (_seenWords1[i])
            continue;
          _seenWords1[i] = true;
          _seenWords2.push_back(i);
          // speed up when |q|=3 and thr <= 2: check all possible cases
          // instead of computing filters and distance
          // TODO(celikik): not complete!
          if ((*_queryString).length() == 3)
          {
            if (_threshold == 2)
            {
              if (str.length() == 1)
              {
                if ((*_queryString)[0] != (*_vocabulary)[i][0] ||
                    (*_queryString)[0] != (*_vocabulary)[i][1] ||
                    (*_queryString)[0] != (*_vocabulary)[i][2] ||
                    (*_queryString)[1] != (*_vocabulary)[i][1] ||
                    (*_queryString)[1] != (*_vocabulary)[i][2] ||
                    (*_queryString)[2] != (*_vocabulary)[i][2])
                {
                  _matches.push_back(std::make_pair(i, 2));
                  continue;
                }
                continue;
              }
            }
            else
            if (_threshold == 1 && str.length() == 2 && depth == 1)
            {
              if ((*_vocabulary)[i].length() == 2)
              {
                _matches.push_back(std::make_pair(i, 1));
                continue;
              }
              else
                if (str.length() == 3)
                {
                  _matches.push_back(std::make_pair(i, 0));
                  continue;
                }
              if ((*_queryString)[0] != (*_vocabulary)[i][0])
              {
                if (((*_queryString)[1] == (*_vocabulary)[i][1] && (*_queryString)[2] == (*_vocabulary)[i][2]) ||  // NOLINT
                    ((*_queryString)[1] == (*_vocabulary)[i][0] && (*_queryString)[2] == (*_vocabulary)[i][1]))  // NOLINT
                {
                  _matches.push_back(std::make_pair(i, 1));
                  continue;
                }
              }
              else
              {
                if (((*_queryString)[1] == (*_vocabulary)[i][1]) || ((*_queryString)[2] == (*_vocabulary)[i][2])  // NOLINT
                    || ((*_queryString)[2] == (*_vocabulary)[i][1]))
                {
                  _matches.push_back(std::make_pair(i, 1));
                  continue;
                }
              }
              continue;
            }
          }

          if (i == _lastI + 1 && _lastMatched)
          {
            if (_prefixLengths[i] == (*_vocabulary)[_lastI].length())
            {
              nofDistanceComputations++;
              // distanceCalcTimer.cont();
              d = _extensionDistanceCalculator.calculate(*_queryString,
                  (*_vocabulary)[i], _threshold, _pos);
              // distanceCalcTimer.stop();
              _matches.push_back(std::make_pair(i, d));
              _prefixLength = INT_MAX;
              _lastI = i;
              continue;
            }
            else
            {
              _prefixLength = MY_MIN(_prefixLengths[i], _prefixLength);
              if (_prefixLength >= _pos || i == end)
              {
                int d2 = MY_MAX(d, _pos - _prefixLength);
                // TODO(celikik): I am not quite sure what's going on here.
                if (d2 <= _threshold)
                  _matches.push_back(std::make_pair(i, d2));
                _lastI = i;
                continue;
              }
            }
          }
          _lastI = i;
          // unigram frequency distance (common number of letters filter)
          const T& s = (*_vocabulary)[i];
          if (_queryString->length() > _truncateLength)
          {
            unsigned char ind;
            uint16_t cmnLetters = str.length();
            size_t len = MY_MIN(s.length(),
                _queryString->length() + _truncateLength - str.length());
            for (uint32_t j = str.length(); j < len; j++)
            {
              ind = static_cast<unsigned char>(s[j]);
              if (_letterCounts[ind] > 0)
                cmnLetters++;
              _letterCounts[ind]--;
            }
            for (uint32_t j = str.length(); j < len; j++)
            {
              ind = static_cast<unsigned char>(s[j]);
              _letterCounts[ind] = _letterCountsBackup[ind];
            }
            if (cmnLetters < _queryString->length() - 2)
            {
              _lastMatched = false;
              continue;
            }
          }

          nofDistanceComputations++;
          // distanceCalcTimer.cont();
          d = _extensionDistanceCalculator.calculate(*_queryString,
              (*_vocabulary)[i], _threshold, _pos);
          // distanceCalcTimer.stop();
          if (d <= _threshold)
          {
            _matches.push_back(std::make_pair(i, d));
            _lastMatched = true;
            _prefixLength = INT_MAX;
          }
          else
            _lastMatched = false;
        }
      }
    }
    else
    {
      if (_mode == FASTSS_WORD_MATCHING_FULL)
      {
        for (size_t j = 0; j < postingList.size(); j++)
        {
          int beg = getWordId(postingList[j]);
          int end = beg + getPrefix(postingList[j]);
          for (int i = beg; i <= end; i++)
          {
            if (_seenWords1[i])
              continue;
            _seenWords1[i] = true;
            _seenWords2.push_back(i);
            nofDistanceComputations++;
            // if ((_queryString->length() <= 32)
            // && ((*_vocabulary)[i].length() <= 32))
            //  d = myersed.calculate(*_queryString, (*_vocabulary)[i], false);
            // else
            d = _editDistanceCalculator.calculate(*_queryString,
                  (*_vocabulary)[i], _threshold);
            if (d <= _threshold)
              _matches.push_back(std::make_pair(i, d));
          }
        }
      }
      else if (_mode == FASTSS_WORD_MATCHING_TRUNC)
      {
        for (size_t j = 0; j < postingList.size(); j++)
        {
          int beg = getWordId(_prefixRanges[postingList[j]]);
          int end = beg + getPrefix(_prefixRanges[postingList[j]]);
          for (int i = beg; i <= end; i++)
          {
            if (_seenWords1[i])
              continue;
            _seenWords1[i] = true;
            _seenWords2.push_back(i);
            const T& s = (*_vocabulary)[i];
            if (abs(static_cast<int>(_queryString->length()) -
                static_cast<int>((*_vocabulary)[i].length())) <= _threshold)
            {
              // unigram frequency distance (common number of letters filter)
              // if (s.length() > 8)
              //  _truncateLength = 7;
              // else
              //  _truncateLength = 6;
              if (_queryString->length() > _truncateLength)
              {
                unsigned char ind;
                uint16_t cmnLetters = str.length();
                size_t len = s.length();
                for (size_t j = str.length(); j < len; j++)
                {
                  ind = static_cast<unsigned char>(s[j]);
                  if (_letterCounts[ind] > 0)
                    cmnLetters++;
                  _letterCounts[ind]--;
                }
                for (size_t j = str.length(); j < len; j++)
                {
                  ind = static_cast<unsigned char>(s[j]);
                  _letterCounts[ind] = _letterCountsBackup[ind];
                }
                if (cmnLetters < MY_MAX(_queryString->length(),
                    s.length()) - _threshold)
                  continue;
              }
              nofDistanceComputations++;
              // if ((_queryString->length() <= 32)
              // && ((*_vocabulary)[i].length() <= 32))
              //   d = myersed.calculate(*_queryString,
              // (*_vocabulary)[i], false);
              // else
               d = _editDistanceCalculator.calculate(*_queryString,
                   (*_vocabulary)[i], _threshold);
              if (d <= _threshold)
                _matches.push_back(std::make_pair(i, d));
            }
          }
        }
      }
    }
  }
  if (depth >= _threshold)
    return;
  for (int i = beg; i < static_cast<int>(str.length()); i++)
  {
    T tempStr;
    tempStr.resize(str.length() - 1);
    int counter = 0;
    for (int j = 0; j < static_cast<int>(str.length()); j++)
    {
      if (j != i)
        tempStr[counter++] = str[j];
    }
    findMatches(tempStr, i, depth + 1);
  }
}

// ____________________________________________________________________________
template<>
void FastSS<string>::writeLine(FILE* f, const string& query)
{
  fprintf(f, "%s\n", query.c_str());
}

// ____________________________________________________________________________
template<>
void FastSS<wstring>::writeLine(FILE* f, const wstring& query)
{
  string tmpstr;
  wstring2string(query, &tmpstr);
  fprintf(f, "%s\n", tmpstr.c_str());
}

// ____________________________________________________________________________
template <>
void FastSS<string>::readLine(FILE* f, string* line,
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
void FastSS<wstring>::readLine(FILE* f, wstring* line,
                                           bool toLower)
{
  string linestr;
  FastSS<string>::readLine(f, &linestr, toLower);
  string2wstring(linestr, line);
}


// ____________________________________________________________________________
template <>
void FastSS<string>::saveTheLexiconPart(
                                 FILE* outputFile,
                                 const vector<string>& clusterCenters)
{
  for (uint32_t i = 0; i < clusterCenters.size(); i++)
  {
    fprintf(outputFile, "%s\n", clusterCenters[i].c_str());
  }
  std::cout << clusterCenters.size() << " words saved." << std::endl;
}

// ____________________________________________________________________________
template <>
void FastSS<wstring>::saveTheLexiconPart(
                                 FILE* outputFile,
                                 const vector<wstring>& clusterCenters)
{
  string tmpstr;
  for (uint32_t i = 0; i < clusterCenters.size(); i++)
  {
    wstring2string(clusterCenters[i], &tmpstr);
    fprintf(outputFile, "%s\n", tmpstr.c_str());
  }
  std::cout << clusterCenters.size() << " words saved." << std::endl;
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::saveDataStructureToFile(const string& filename)
{
  cout << "Saving FastSS fuzzy-search data structure to disk ... " << flush;
  FILE* outputFile = fopen(filename.c_str(), "w");
  fprintf(outputFile, "1\n%f\n%d\n%d\n%d\n%d\n%u\n",
      _threshold,
      static_cast<int>((*_vocabulary).size()),
      static_cast<int>(_delNeigh.size()),
      static_cast<int>(_prefixRanges.size()),
      _mode,
      _truncateLength);
  FastSS<T>::saveTheLexiconPart(outputFile, (*_vocabulary));
  typename HashMap::const_iterator it;
  for (it = _delNeigh.begin(); it != _delNeigh.end(); it++)
  {
    writeLine(outputFile, it->first);
    fprintf(outputFile, "%zu ", it->second.size());
    for (size_t i = 0; i < it->second.size(); i++)
      if (i < it->second.size() - 1)
        fprintf(outputFile, "%zu ", it->second[i]);
      else
        fprintf(outputFile, "%zu\n", it->second[i]);
  }
  for (size_t i = 0; i < _prefixRanges.size(); i++)
    fprintf(outputFile, "%u\n", _prefixRanges[i]);
  fclose(outputFile);
  cout << "done" << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::loadDataStructureFromFile(
                               const string& filename,
                               vector<T>* vocabulary)
{
  cout << "* Reading FastSS-based data structure from \""
       << filename << "\" ... " << flush;
  const int MAX_LENGTH = 1000000;  // it can sometimes be very very long!
  char buff[MAX_LENGTH + 2];
  std::stringstream sstr;
  int intValue;
  double doubleValue;
  T stringValue;
  FILE* fin = fopen(filename.c_str(), "r");
  if (fin == NULL)
  {
    perror("fopen:");
    exit(1);
  }
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> intValue;
  if (intValue != 1)  // single completion fuzzysearch
  {
    std::cerr << "ERROR: The file does not have the appropriate format!"
         << endl << endl;
    exit(1);
  }
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> doubleValue;
  _threshold = doubleValue;
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> intValue;
  vocabulary->clear();
  vocabulary->resize(intValue);
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> intValue;
  size_t nofTransformation = intValue;
  _delNeigh.clear();
  _delNeigh.rehash(nofTransformation);  // NOTE(bast): was hash_map.resize
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> intValue;
  _prefixRanges.resize(intValue);
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> intValue;
  int16_t mode = intValue;
  assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
  sstr << buff;
  sstr >> intValue;
  init(mode, _threshold, intValue);

  // 1. read the lexicon
  for (uint32_t i = 0; i < vocabulary->size(); i++)
  {
    FastSS<T>::readLine(fin, &stringValue, false);
    (*vocabulary)[i] = stringValue;
  }
  std::cout << vocabulary->size() << " words read." << std::endl;

  // 2. read the (truncated) deletion neighborhood index
  _delNeigh.clear();
  for (uint32_t i = 0; i < nofTransformation; i++)
  {
    FastSS<T>::readLine(fin, &stringValue, false);
    if (stringValue.size() > MAX_WORD_LEN)
    {
      std::cerr << endl << "Error while reading the deletion neighborhood "
          "index. Line too long!" << endl;
      exit(1);
    }
    vector<size_t>& list = _delNeigh[stringValue];
    assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
    std::stringstream sstr;
    sstr << buff;
    sstr >> intValue;
    list.resize(intValue);
    for (size_t j = 0; j < list.size(); j++)
    {
      sstr >> intValue;
      assert(intValue >= 0);
      list[j] = intValue;
    }
  }

  // 3. read the vector of prefix ranges
  for (size_t i = 0; i < _prefixRanges.size(); i++)
  {
    assert(fgets(buff, MAX_LENGTH + 2, fin) != NULL);
    std::stringstream sstr;
    sstr << buff;
    sstr >> intValue;
    assert(intValue >= 0);
    _prefixRanges[i] = intValue;
  }
  fclose(fin);
  _vocabulary = vocabulary;
  _seenWords1.resize(vocabulary->size());
  _seenWords2.reserve(100000);
  if (_mode == FASTSS_COMPLETION_MATCHING)
    calculatePrefixLengths(*vocabulary);
  cout << endl << "* " << vocabulary->size() << " words, "<< _delNeigh.size()
       << " subsequences, " << _prefixRanges.size()
       << " prefix ranges." << endl;
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::computePrefixes(const vector<T>& vocabulary,
                                unsigned int prefixLen,
                                vector<PrefixRange>* prefixes)
{
  // NOTE(bast): pr was unintitialized, which could be a problem in setWordId
  // and setPrefix below, which both have pr appear on the RHS, too.
  PrefixRange pr = 0;
  for (size_t i = 0; i < vocabulary.size(); i++)
  {
    int first = i;
    int last = i;
    T prefix = vocabulary[i].substr(0,
        MY_MIN(prefixLen, vocabulary[i].length()));
    if (last < static_cast<int>(vocabulary.size() - 1))
      while (prefix == vocabulary[last+1].substr(0,
          MY_MIN(prefixLen, vocabulary[last+1].length())))
      {
        last++;
        if (last == static_cast<int>(vocabulary.size() - 1))
          break;
      }
    i = last;
    setWordId(&pr, first);
    setPrefix(&pr, std::min<int>((1U << NOF_BITS_PREFIX) - 1, last - first));
    prefixes->push_back(pr);
  }
  std::cout << " [ Prefix dictionary has " << prefixes->size() << " prefixes ]"
            << std::endl;
}

// ____________________________________________________________________________
template <class T>
void FastSS<T>::calculatePrefixLengths(const vector<T>& strings)
{
  cout << " [ Calculating vocabulary prefix lengths ... " << flush;
  if (strings.size() > 0)
  {
    _prefixLengths.resize(strings.size());
    _prefixLengths[0] = 0;
    for (size_t i = 0; i < strings.size() - 1; i++)
    {
      const T& s1 = strings[i];
      const T& s2 = strings[i+1];
      size_t len = MY_MIN(s1.length(), s2.length());
      for (size_t j = 0; j < len; j++)
      {
        if (s1[j] == s2[j])
          _prefixLengths[i+1]++;
        else
          break;
      }
    }
  }
  cout << "done ]" << endl << flush;
}

// explicit instantiation
template class FastSS<std::string>;
template class FastSS<std::wstring>;
}
