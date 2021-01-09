// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Marjan Celikik <celikik>, Hannah Bast <bast>

#ifndef FUZZYSEARCH_FUZZYSEARCHER_H_
#define FUZZYSEARCH_FUZZYSEARCHER_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "../server/Exception.h"
#include "../fuzzysearch/Utils.h"
#include "../fuzzysearch/FuzzySearchAlgorithm.h"
#include "../fuzzysearch/FastSS.h"
#include "../fuzzysearch/PermutedLexicon.h"
#include "../fuzzysearch/WordClusteringBuilder.h"
#include "../fuzzysearch/StringDistances.h"

namespace FuzzySearch
{
// Fuzzy searcher abstract base class.
class FuzzySearcherBase
{
 public:
  // Function for setting up a closest words finder from the precomputed files
  // (<basename>.fuzzysearch-datastructure and <basename>.fuzzysearch-clusters)
  // so that it can answer queries.
  virtual bool init(const std::string& baseName) = 0;

  // Abstract findClosestWords function.
  virtual void findClosestWords(const std::string& query,
                                bool useTrivialAlg,
                                bool* queryIsInLexicon,
                                vector<int>* closestWordsIds,
                                vector<double>* distances) = 0;

  // use normalization?
  void useNormalization(bool normalization)
  {
    _usingNormalization = normalization;
  }

  // normalize the word
  virtual string normalizeWord(const string& word) = 0;

  // returns true if 'autocompletion distance' is used
  virtual bool completionMatching() = 0;

  // returns the i-th word from the cluster centroids
  virtual string getClusterCentroid(int i) = 0;

  // return the number of cluster centroids of the saved datastructure
  virtual size_t getNofClusterCentroids() = 0;

  // returns true if normalization is used
  bool usingNormalization()
  {
    return _usingNormalization;
  }

  // the ids of the words inside the i-th cluster
  vector<vector<int> > clusterIdsPerClusterCenter;

  // returns the distance between two strings
  double getDistance(const string& str1, const string& str2)
  {
    if (_encoding == WordClusteringBuilder<wstring>::UTF_8)
    {
      wstring wstr1;
      wstring wstr2;
      string2wstring(str1, &wstr1);
      string2wstring(str2, &wstr2);
      if (completionMatching())
        return _ld.calculate(wstr1, wstr2, MAX_ED);
      else
        return _extDist.calculate(wstr1, wstr2, MAX_ED);
    }
    else
    {
      if (completionMatching())
        return _ld.calculate(str1, str2, MAX_ED);
      else
        return _extDist.calculate(str1, str2, MAX_ED);
    }
  }

  // returns the permuted lexicon type
  int getFuzzySearchIndexType(const string& fileName);

  // true if the cluster id of frequent words
  bool isFrequentWordClusterId(int clusterId)
  {
    if (clusterId >= _clusterIdFWBeg && clusterId <= _clusterIdFWEnd)
      return true;
    return false;
  }

  // returns the used threshold (if applicable)
  virtual double getThreshold() = 0;

  // returns a frequency of a cluster centroid;
  virtual int getFrequency(const string& word) = 0;

  // virtual destructor
  virtual ~FuzzySearcherBase() {}

  // holding the word-ids inside a cluster
  // needed for compunormalizeWordting the relevant clusters
  // at query time
  vector<vector<int> > wordIdsPerCluster;

  // true if the word is a cluster centroid
  vector<bool> isClusterCentroid;

 protected:
  // convert char* to string (trivial)
  static void convertString(const char* cstr, string* str, size_t len)
  {
    *str = cstr;
  }

  // convert char* to wstring
  static void convertString(const char* cstr, wstring* wstr, size_t len)
  {
    static wchar_t tmpwchar[MAX_WORD_LEN + 1];
    size_t c = mbstowcs(tmpwchar, cstr, MAX_WORD_LEN);
    if (c > MAX_WORD_LEN)
    {
      string str = cstr;
      wstring tmp(str.begin(), str.end());
      *wstr = tmp;
    }
    else
      *wstr = tmpwchar;
  }

  // Read cluster ids from given file into given vector<vector<int> >.
  // NOTE: The name of the words will actually be ignored, we will just assume
  // that the first word in the file has id 0, the second (different) word has
  // id 1, and so on.
  // If the words vector is not empty, check that the i-th different word in the
  // file is indeed equal to the i-th word in words.
  template <class T>
  void readClusterIds(const string& fileName,
                      const vector<T>& words,
                      vector<vector<int> >* clusterIdsPerWord,
                      vector<vector<int> >* wordIdsPerCluster);

  // data structure mode (read from disk)
  int _mode;

  // indicates if normalization is used for the saved data str.
  bool _usingNormalization;

  // the used encoding
  int _encoding;

  // the base name of the collection
  string _baseName;

  // clusters ids for frequent words begin here
  int _clusterIdFWBeg;

  // clusters ids for frequent words end here
  int _clusterIdFWEnd;

  // levenshtein distance object
  PlainEditDistance _ld;

  // extension (prefix) distance object
  ExtensionEditDistance _extDist;
};

// Subclass for fuzzy search in UTF8 strings.
class FuzzySearcherUtf8 : public FuzzySearcherBase
{
 public:

  // default constructor
  FuzzySearcherUtf8()
  {
    _encoding = WordClusteringBuilder<wstring>::UTF_8;
    _frequencyMap = NULL;
  }

  // destructor
  ~FuzzySearcherUtf8()
  {
    delete _frequencyMap;
  }

  // wrapper function for finding all similar words
  virtual void findClosestWords(const std::string& query,
                                bool useTrivialAlg,
                                bool* queryIsInLexicon,
                                vector<int>* closestWordsIds,
                                vector<double>* distances);

  // see above.
  virtual bool init(const std::string& baseName);

  // normalize a word (here use string instead of wstring)
  string normalizeWord(const string& word)
  {
    return WordClusteringBuilder<string>::normalizeWord(word);
  }

  // return the number of cluster centroids
  // TODO(celikik): The name of this function should be changed to something
  // more appropriate. As well as clusterCenters.
  size_t getNofClusterCentroids()
  {
    return clusterCenters.size();
  }

  // returns true if 'autocompletion distance' is used
  bool completionMatching()
  {
    return _fsAlgorithm->completionMatching();
  }

  // returns the i-th word from the cluster centroids
  // TODO(celikik): will this function cause slowdown?
  string getClusterCentroid(int i)
  {
    assert(static_cast<int>(clusterCenters.size()) > i);
    string ccAsString;
    wstring2string(clusterCenters[i], &ccAsString);
    return ccAsString;
  }

  // returns the used threshold (if applicable)
  double getThreshold()
  {
    if (_fsAlgorithm != NULL)
      return _fsAlgorithm->getThreshold();
    else
      return INT_MAX;
  }

  // returns the frequency of a cluster centroid
  int getFrequency(const string& word)
  {
    if (_frequencyMap == NULL)
      return 0;
    if ((*_frequencyMap).count(word) != 0)
      return (*_frequencyMap)[word];
    else
      return 0;
  }

  // its vocabulary (read from disk)
  vector<std::wstring> clusterCenters;

  // contains the frequencies of the cluster centroids
  std::unordered_map<string, int, StringHashFunctionMarjan>* _frequencyMap;

 private:
  // fuzzy search index
  FuzzySearchAlgorithm<wstring> *_fsAlgorithm;
};

// Subclass for fuzzy search in ISO88591 strings.
class FuzzySearcherIso88591 : public FuzzySearcherBase
{
 public:

  // default constructor
  FuzzySearcherIso88591()
  {
    _encoding = WordClusteringBuilder<wstring>::ISO_8859_1;
    _frequencyMap = NULL;
  }

  // destructor
  ~FuzzySearcherIso88591()
  {
    delete _frequencyMap;
  }

  // wrapper function for finding all similar words
  virtual void findClosestWords(const std::string& query,
                                bool useTrivialAlg,
                                bool* queryIsInLexicon,
                                vector<int>* closestWordsIds,
                                vector<double>* distances);

  // see above.
  virtual bool init(const std::string& baseName);

  // normalize a word
  string normalizeWord(const string& word)
  {
    return WordClusteringBuilder<string>::normalizeWord(word);
  }

  // returns true if 'autocompletion distance' is used
  bool completionMatching()
  {
    return _fsAlgorithm->completionMatching();
  }

  // returns the i-th word from the cluster centroids
  string getClusterCentroid(int i)
  {
    assert(static_cast<int>(clusterCenters.size()) > i);
    return clusterCenters[i];
  }

  // return the number of cluster centroids
  size_t getNofClusterCentroids()
  {
    return clusterCenters.size();
  }

  // returns the used threshold (if applicable)
  double getThreshold()
  {
    if (_fsAlgorithm != NULL)
      return _fsAlgorithm->getThreshold();
    else
      return INT_MAX;
  }

  // returns the frequency of a cluster centroid
  int getFrequency(const string& word)
  {
    if (_frequencyMap == NULL)
      return 0;
    if ((*_frequencyMap).count(word) != 0)
      return (*_frequencyMap)[word];
    else
      return 0;
  }

  // its vocabulary (read from disk)
  vector<std::string> clusterCenters;

  // contains the frequencies of the cluster centroids
  std::unordered_map<string, int, StringHashFunctionMarjan>* _frequencyMap;

 private:
  // fuzzy search index
  FuzzySearchAlgorithm<string> *_fsAlgorithm;
};
}

#endif  // FUZZYSEARCH_FUZZYSEARCHER_H_
