// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_FASTSS_H_
#define FUZZYSEARCH_FASTSS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string>
#include <vector>
#include <utility>
#include <google/sparse_hash_map>
#include <google/dense_hash_map>
#include <unordered_map>
// #include <ext/hash_map>
// #include <hash_map>
// using __gnu_cxx::hash_map;

#include "../fuzzysearch/FuzzySearchAlgorithm.h"
#include "../fuzzysearch/StringDistances.h"
#include "../fuzzysearch/Utils.h"

using std::string;
using std::vector;

#define FASTSS_WORD_MATCHING_FULL  1
#define FASTSS_WORD_MATCHING_TRUNC 2
#define FASTSS_COMPLETION_MATCHING 3

namespace FuzzySearch
{
// pack word id and prefix. Used to represent consecutive run of words with a
// single 32bit integer
typedef uint32_t PrefixRange;

template <class T>
class FastSS : public FuzzySearchAlgorithm<T>
{
 private:

  // recursively index str
  void index(const T& str, uint16_t beg, int depth);

  // void index(const T& str, int depth);

  // recursively index str using a sorted dictionary instead of hash table
  void index1(const T& str, int depth);

  // indexes a word using deletion neighborhood
  void indexWord(const T& str, const PrefixRange& prefix);

  // indexes a word using deletion neighborhood using a sorted dictionary
  // instead of hash_table
  void indexWord1(const T& str, const PrefixRange& prefix);

  // calcs space for a word using d. n.
  void calcSpace(const T& str, uint16_t beg, int depth);

  // calcs space for a word using d. n.
  void calculateSpace(const T& str, const PrefixRange& prefix);

  // recursively find matches
  void findMatches(const T& str, uint16_t, int depth);

  // reserve memory for the word-id pointers
  size_t reserveMemory()
  {
    typename HashMap::iterator it;
    size_t totalPointers = 0;
    // size_t totalWordIds = 0;
    size_t oneElLists = 0;
    for (it = _delNeigh.begin(); it != _delNeigh.end(); it++)
    {
      off_t size = it->second[0];
      // totalWordIds += it->second[1];
      vector<size_t>().swap(it->second);
      it->second.reserve(size);
      totalPointers += size;
      if (size == 1)
        ++oneElLists;
    }
    std::cout << " [ " << totalPointers << " pointers (" << oneElLists
         << " lists with single pointer, avg: "
         << 1.0 * totalPointers / _delNeigh.size() << "), "
         << _totalTransformationLength << " raw hash table size, "
         << _delNeigh.size() << " hash entries ]" << std::endl;
    // std::cout << totalWordIds << " total word ids" << std::endl;
    return totalPointers;
  }

  // Set the threshold depending on the length of the current word being
  // indexed. If fixed threshold is used then use that value
  void setDynThreshold(size_t length)
  {
    if (_fixedThreshold < 0)
    {
      if (length >= 11)
        _threshold = 3;
      else
      if (length > 5)
        _threshold = 2;
      else
        _threshold = 1;
    }
    else
      _threshold = _fixedThreshold;
  }

  // computes the prefix lengths of consecutive strings in the input vocabulary
  void calculatePrefixLengths(const vector<T>& strings);

  // pack word-id and number of consecutive words with equal prefixes into a
  // single unsigned int
  inline uint32_t getWordId(uint32_t n)
  {
    return n >> NOF_BITS_PREFIX;
  }

  inline void setWordId(uint32_t* n, uint32_t wordId)
  {
    *n = (*n & mask) | (wordId << NOF_BITS_PREFIX);
  }

  inline uint32_t getPrefix(uint32_t n)
  {
    return n & mask;
  }

  inline void setPrefix(uint32_t* n, uint32_t prefix)
  {
    *n = (*n & mask1) | prefix;
  }

  // truncation length parameter of the algorithm
  unsigned int _truncateLength;

  // word id of the current word being indexed
  int _currentWordId;

  PrefixRange _currentPrefixRange;

  // levenshtein distance computed by using the standard dynamic programming
  // algorithm
  PlainEditDistance _editDistanceCalculator;

  // levenshtein distance computed by using myers' bit parallel algorithm
  CMyersEdistFastPair myersed;

  // prefix levenshtein distance
  ExtensionEditDistance _extensionDistanceCalculator;

  // used mark words that have been seen so far
  vector<bool> _seenWords1;

  // used to clear _seenWords1 for the new query
  vector<int> _seenWords2;

  // used to compute the unigram frequency distance
  vector<int> _letterCounts;

  // used to compute the unigram frequency distancev
  vector<int> _letterCountsBackup;

  // keeps the prefix ranges computed in the current vocabulary
  vector<PrefixRange> _prefixRanges;

  // pointer to the vector of found matches
  vector<pair<int, double> > _matches;

  // similarity threshold
  double _threshold;

  // if non-negative then mode 2 and 3 will have fixed instead of
  // variable threshold (that depend on the query length)
  double _fixedThreshold;

  // 1 - word matching (full deletion neighborhood)
  // 2 - word matching (truncated deletion neighborhood)
  // 3 - completion matching (truncated deletion neighborhood)
  int16_t _mode;

  // used hash map type for the deletion neighborhoods
  typedef std::unordered_map<T, vector<size_t>, StringHash<T> > HashMap;

  // hash table containing the deletion neighborhoods
  HashMap _delNeigh;  // NOLINT

  // needed for packing word-id and number of consecutive words
  // with equal prefixes into a single uint
  static const uint32_t NOF_BITS_WORDID = 23;
  static const uint32_t NOF_BITS_PREFIX = 32 - NOF_BITS_WORDID;
  static const uint32_t mask = (1U << NOF_BITS_PREFIX) - 1;
  static const uint32_t mask1 = ~mask;

  // alternative dictionary for less space
  vector<T> _transformations;

  // alternative postings of wordIds for less space
  vector<vector<int> > _wordIds;

  // pointer to the current indexed vocabulary
  const vector<T>* _vocabulary;

  // current query string
  const T* _queryString;

  // stats
  size_t _totalPointers;

  // stats
  size_t _totalTransformationLength;

  // lengths of consecutive strings in the input vocabulary
  vector<unsigned char> _prefixLengths;

  // indicates whether the current word being indexed is short
  bool _shortWordIndexed;

  // needed for findMatches()
  int _lastI;
  bool _lastMatched;
  int _pos;
  int _prefixLength;

 public:

  // the default constructor
  FastSS()
  {
    init(2, 0, 7);
  }

  FastSS(int16_t mode, double threshold)
  {
    init(mode, threshold, 7);
  }

  FastSS(int16_t mode, double threshold, int truncLength)
  {
    init(mode, threshold, truncLength);
  }

  // initialize important stuff of the object
  void init(int16_t mode, double threshold, int truncLength)
  {
    _truncateLength = truncLength;
    _mode = mode;
    _threshold = threshold;
    _totalTransformationLength = 0;
    _totalPointers = 0;
    nofDistanceComputations = 0;
    _fixedThreshold = -1;
    _letterCounts.resize(256 * 256);
    _letterCountsBackup.resize(256 * 256);
    if (_mode == 3)
      FuzzySearchAlgorithm<T>::_isCompletionDistanceUsed = true;
    else
      FuzzySearchAlgorithm<T>::_isCompletionDistanceUsed = false;
  }

  // sets the mode of the fastss object (see above)
  // (re)indexing is required!
  void setMode(int mode) { init(mode, _threshold, _truncateLength); }

  // returns the mode (see above for avail. modes)
  int getMode() { return _mode; }

  // sets the threshold (if applicable)
  // (re)indexing might be required
  void setThreshold(double threshold) { _threshold = threshold; }

  // see "_fixedThreshold"
  void setFixedThreshold(double threshold) { _fixedThreshold = threshold; }

  // returns the used threshold (if applicable)
  double getThreshold() { return _threshold; }

  // index words in vocabulary using del. neighborhood
  void buildIndex(const vector<T>& vocabulary, bool reserveMemory);

  // find all matches w.r.t. str using the del. neigh. index
  void findClosestWords(const T& query,
                        const vector<T>& vocabulary,
                        const vector<T>& indexedWords,
                        bool* queryIsInLexicon,
                        vector<int>* closestWordsIds,
                        vector<double>* distances);

  // return the appropriate distance used (see base class)
  int getDistanceType()
  {
    switch (_mode)
    {
      case 0: return 1;
      case 1: return 0;
      case 2: return 0;
      case 3: return 2;
      case 4: return 0;
      default: return -1;
    }
  }

  // save the vocabulary and the deletion neighborhood index to disk
  void saveDataStructureToFile(const string& filename);

  // save the lexicon as a part of the filename
  void saveTheLexiconPart(FILE* outputFile, const vector<T>& clusterCenters);

  // load the vocabulary and the deletion neighborhood index from disk
  void loadDataStructureFromFile(const string& filename,
                                 vector<T>* vocabulary);

  // write a line/string into a file depending on the encoding
  static void writeLine(FILE* f, const T& str);

  // read a line/string into a file depending on the encoding
  static void readLine(FILE* f, T* line, bool toLower);

  // compute prefix ranges for vocabulary
  void computePrefixes(const vector<T>& vocabulary,
      unsigned int len, vector<PrefixRange>* prefixes);

  // stats
  size_t nofDistanceComputations;

  // total time needed for distance calculations
  Timer distanceCalcTimer;
};
}

#endif  // FUZZYSEARCH_FASTSS_H_
