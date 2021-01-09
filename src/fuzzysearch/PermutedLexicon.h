// Copyright [2010] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_PERMUTEDLEXICON_H_
#define FUZZYSEARCH_PERMUTEDLEXICON_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <utility>
#include <fstream>
// #include <ext/hash_map>
// #include <ext/hash_set>

#include "../fuzzysearch/FuzzySearchAlgorithm.h"
#include "../fuzzysearch/CyclicPermutation.h"
#include "../fuzzysearch/StringDistances.h"
#include "../fuzzysearch/Utils.h"

namespace FuzzySearch
{
// experimental
class Result
{
  public:
    int wordId;
    double dist;
    vector<int16_t> lastRow;
    bool valid;
};

template <class T>
class PermutedLexicon : public FuzzySearchAlgorithm<T>
{
  public:
  // permuted lexicon type for word matching
  typedef vector<CyclicPermutationWithIndex<T> > PermutedLexiconWords;

  // permuted lexicon type for completion matching
  typedef vector<CyclicPermutationSubstringWithIndex<T> >
  PermutedLexiconCompletions;

    // default constructor
    PermutedLexicon()
    {
      init(1, 0);
    }

    // constructor
    // mode 0: fuzzy word matching with normalized edit distance
    // mode 1: fuzzy word matching with 2/5 distance
    // mode 2: fuzzy completion matching with 2/5 ext. distance
    // mode 3: fuzzy completion matching with normalized ext. distance
    PermutedLexicon(int mode, double threshold)
    {
      init(mode, threshold);
    }

    // Get k words that are close to the given query word, which itself may be,
    // but does not have to be in the lexicon.
    // The k words are not
    // necessarily the k words with the smallest edit distance to the given
    // query word, but they are close to this set. The k words are returned
    // as a vector of word ids. Also returned is a flag saying whether the
    // query word was in the lexicon or not.
    void findClosestWords(const T& query,
                          const vector<T>& vocabulary,
                          const vector<T>& indexedWords,
                          bool* queryIsInLexicon,
                          vector<int>* closestWordsIds,
                          vector<double>* distances);

    // build the fuzzy search index
    void buildIndex(const vector<T>& vocabulary, bool reserved);

    // save the vocabulary and the index to disk
    void saveDataStructureToFile(const string& filename);

    // load the vocabulary and the index from disk
    void loadDataStructureFromFile(const string& filename,
                                   vector<T>* vocabulary);

    // find similar words using permuted lexicon
    void findClosestWordsPermutedLexicon(
                                    const T& query,
                                    const vector<T>& vocabulary,
                                    const vector<T>& clusterCenters,
                                    int cutOff,
                                    bool* queryIsInLexicon,
                                    vector<int>* closestWordsIds,
                                    vector<double>* distances);

    // find similar words using permuted lexicon
    // but used a signature based on partition the
    // pattern instead of LCS.
    void findClosestWordsPermutedLexiconPart(
                                    const T& query,
                                    const vector<T>& vocabulary,
                                    const vector<T>& clusterCenters,
                                    int cutOff,
                                    bool* queryIsInLexicon,
                                    vector<int>* closestWordsIds,
                                    vector<double>* distances);

    // find similar completions using p. l.
    void findClosestCompletions(const T& query,
                                const vector<T>& clusterCenters,
                                vector<int>* closestWordsIds,
                                vector<double>* distances);

    // experimental
    void findClosestCompletionsInc(
        const T& query,
        const vector<T>& clusterCenters,
        vector<int>* closestWordsIds,
        vector<double>* distances,
        vector<int>* indexes);

    // experimental
    void findClosestCompletions(
        const T& query,
        const vector<T>& clusterCenters,
        vector<int>* closestWordsIds,
        vector<double>* distances,
        vector<Result>* results);

    // experimental
    void findClosestCompletions(
        const T& query,
        const vector<CyclicPermutationSubstringWithIndex<T> >& permutedLexicon,
        const vector<T>& clusterCenters,
        vector<int>* closestWordsIds,
        vector<double>* distances,
        vector<Result>* results);

    // experimental
    vector<int16_t> _resultPrefixLengths;

    // experimental
    vector<Result> tempResults;

    // experimental
    void calculatePrefixLengths(
        const vector<T>& clusterCenters,
        const vector<Result>& results);

    // experimental
    vector<size_t> wordsSeen;
    vector<size_t> wordsSeen1;

    // return the appropriate distance used (see base class)
    int getDistanceType()
    {
      switch (FuzzySearchAlgorithm<T>::_mode)
      {
        case 0: return 1;
        case 1: return 0;
        case 2: return 2;
        case 3: return 3;
        default: return -1;
      }
    }

    // returns the used threshold (if applicable)
    double getThreshold() { return _threshold; }

    // set the cutoff param.
    void setCutOff(int cutoff) { _cutOff = cutoff; }

    // write a line/string into a file depending on the encoding
    static void writeLine(FILE* f, const T& str);

    // read a line/string into a file depending on the encoding
    static void readLine(FILE* f, T* line, bool toLower);

    // read a line/string into a file depending on the encoding
    static void readLine(fstream* f, T* line, bool toLower);

    // change max prefix query length, see _maxPrefixQueryLength
    void setMaxPermutedLexiconLength(int newVal)
    {
      _maxPrefixQueryLength = newVal;
    }

    // true = fuzzy word matching is not heuristic anymore
    void setFuzzyWordMatchingExact(bool newVal)
    {
      _fuzzyWordMatchingExact = newVal;
    }

    // use fixed threshold instead of variable
    // has no effect if in normalized e.d. mode
    // set it to -1 to switch to variable thr.
    void setFixedThreshold(int threshold);

    // threshold for words <= 5 (not used for normalized e.d.)
    int thr1;

    // threshold for words > 5 (not used for normalized e.d.)
    int thr2;

    // holds the number of distance calcs. performed (stat)
    size_t nofDistanceComputations;

  private:
    // initialize some global vars
    void init(int mode, double threshold);

    // build permuted lexicon for fuzzy word matching
    void buildPermutedLexicon(const vector<T>& clusterCenters,
                              const uint32_t minMissLength,
                              PermutedLexiconWords* permutedLexicon);

    // build permuted lexicon for fuzzy completion matching
    void buildPermutedLexicons(const vector<T>& clusterCenters,
                               const uint32_t minMissLength,
                               const size_t minPrefixLen,
                               const size_t maxPrefixLen);

    // build permuted lexicon for a certain query length (compl. matching)
    void buildPermutedLexicon(
             const vector<T>& clusterCenters,
             const uint32_t minMissLength,
             const size_t minPrefixLen,
             const size_t maxPrefixLen,
             vector<CyclicPermutationSubstringWithIndex<T> >* permutedLexicon);

    // find the similar completions for a certain query length
    void findClosestCompletions(
        const T& query,
        const vector<CyclicPermutationSubstringWithIndex<T> >& permutedLexicon,
        const vector<T>& clusterCenters,
        vector<int>* closestWordsIds,
        vector<double>* distances);

    // calculates the prefix lengths of the strings from the permuted lexicon
    void calculatePrefixLengths(const vector<T>& clusterCenters,
                                const PermutedLexiconWords& permutedLexicon);

    // calculates the prefix lengths of the strings from the permuted lexicon
    // for fuzzy completion matching
    void calculatePrefixLengths(
        const vector<T>& clusterCenters,
        const vector<CyclicPermutationSubstringWithIndex<T> >& permutedLexicon);

    // initialize some of the filters
    void initPermutedLexiconFilters(int maxED = 3);

    // lcs and number of common letters filter
    int applyFilters(const CyclicPermutationWithPointer<T>& cyclicPerm,
                     const PermutedLexiconWords& permutedLexicon,
                     const vector<T>& clusterCenters,
                     uint32_t id2,
                     int p,
                     size_t maxLen);

    // separate common letters filter
    bool commonCharsFilter(const CyclicPermutationWithPointer<T>& cyclicPerm,
                           const PermutedLexiconWords& permutedLexicon,
                           const vector<T>& clusterCenters,
                           uint32_t id2,
                           int p,
                           size_t maxLen,
                           float minNofLetters);

    // save the lexicon as a part of the filename
    void saveTheLexiconPart(FILE* outputFile, const vector<T>& clusterCenters);

    // used by: findSimilarCompletions()
    void includeWord(int correctWordId, double dist);

    // pointer to the indexed vocabulary
    const vector<T>* _vocabulary;

    // the used distance threshold
    double _threshold;

    // fixed threshold, needed for setFixedThreshold()
    // used when fixed instead of variable is desired
    int _fixedThreshold;

    // when to stop scanning the neighborhood
    int _cutOff;

    // myers bit-parallel edit distance
    CMyersEdistFastPair myersed;

    // permuted lexicon for fuzzy word matching
    PermutedLexiconWords _permutedLexiconWords;

    // permuted lexicons for fuzzy completion matching
    vector<PermutedLexiconCompletions> _permutedLexiconsCompletions;

    // if true than fuzzy word matching is exact
    bool _fuzzyWordMatchingExact;

    // holds the groups of word-ids that correspond to a certain
    // cyclic permutation for fuzzy completion matching
    vector<vector<int> > _groupsOfWordIds;

    // current word group id (needed for indexing)
    int _wordGroupsId;

    // prefix lengths of the words from the permuted lexicons
    vector<vector<uint8_t> > _vecPrefixLengths;

    size_t _vecPrefixLengthsIndex;

    // holds the number of similar pairs found (stat)
    size_t _similarPairs;

    // hold the lengths of consequent rotations from
    // the word perm. lexicon
    vector<uint8_t> _prefixLengths;

    // needed for common number of letters filter
    vector<int> _letterCounts;

    // needed for common number of letters filter
    vector<int> _letterCountsBackup;

    // needed for substring length filter
    vector<int> _minPrefixes;

    // needed for substring length filter
    vector<int> _minPrefix;

    // needed for substring length filter
    vector<int> _minPrefix25dist;

    // needed for: findClosestWordsPermutedLexicon()
    vector<pair<bool, pair<bool, bool> > > _alreadyCalculated;

    // needed for: findClosestWordsPermutedLexicon()
    vector<int> _seenWords;

    // needed for: findClosestWordsPermutedLexicon()
    T _misspellingStr;

    // needed for: findClosestWordsPermutedLexicon()
    CyclicPermutationWithPointer<T> _cyclicPermutation;

    // helper array used for calculating the similar completions faster
    vector<uint8_t> _skipFirst;

    // used by: findSimilarCompletions()
    bool _clearAlreadyCalculated;

    // used by: findSimilarCompletions()
    bool _calledFromAnotherFun;

    // needed for: findClosestCompletions()
    // holds the current (id, distance) pair
    pair<int, double> _match;

    // needed for: findClosestCompletions()
    // holds the so-far found matches
    vector<pair<int, double> > _matches;

    // extension distance calculator needed for:
    // findClosestCompletions()
    ExtensionEditDistance _extDistanceCalculator;

    // levenshtein distance object
    PlainEditDistance _ld;

    // query with length longer than that are computed
    // using truncation
    uint16_t _maxPrefixQueryLength;
};
}

#endif  // FUZZYSEARCH_PERMUTEDLEXICON_H_
