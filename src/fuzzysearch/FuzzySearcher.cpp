// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Marjan Celikik <celikik>, Hannah Bast <bast>/*

#include <vector>
#include <string>
#include <fstream>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include "../fuzzysearch/FuzzySearcher.h"

namespace FuzzySearch
{
// ____________________________________________________________________________
int FuzzySearcherBase::getFuzzySearchIndexType(const std::string& fileName)
{
  fstream f(fileName.c_str(), ios::in);
  if (f.fail())
  {
    cerr << "* " << fileName << " is missing!" << endl;
    return -1;
  }
  int val;
  f >> val;
  f.clear();
  return val;
}

// ____________________________________________________________________________
bool FuzzySearcherUtf8::init(const std::string& baseName)
{
  _baseName = baseName;
  // Load the fuzzy search data structure.
  string fuzzySearchDataStructureFileName = baseName +
      ".fuzzysearch-datastructure";
  _mode = getFuzzySearchIndexType(fuzzySearchDataStructureFileName);
  if (_mode == 0)  // permuted lexicon
    _fsAlgorithm = new PermutedLexicon<wstring>();
  else
    if (_mode == 1)
      _fsAlgorithm = new FastSS<wstring>();
    else
    {
      cerr << "* ERROR: Unknown fuzzysearch data structure type!" << endl;
      return false;
    }
  _fsAlgorithm->loadDataStructureFromFile(fuzzySearchDataStructureFileName,
       &clusterCenters);
  // Load the cluster ids (one list for each cluster center).
  string fuzzySearchClusterIdsFileName = baseName + ".fuzzysearch-clusters";
  readClusterIds<wstring>(fuzzySearchClusterIdsFileName, clusterCenters,
      &clusterIdsPerClusterCenter, &wordIdsPerCluster);
  fstream f((_baseName + ".fuzzysearch-clustercentroids").c_str(),
      std::ios::in);
  if (!f.fail())
  {
    isClusterCentroid.resize(clusterCenters.size());
    for (size_t i = 0; i < isClusterCentroid.size(); i++)
      isClusterCentroid[i] = false;
    while (true)
    {
      string line;
      f >> line;
      if (f.eof())
        break;
      int wordId = atoi(line.c_str());
      CS_ASSERT_GE(static_cast<int>(isClusterCentroid.size()), wordId);
      isClusterCentroid[wordId] = true;
    }
  }
  else
  {
    cout << "* ERROR: cluster-centroids file does not exist!" << endl;
    return false;
  }
  f.close();
  fstream f1((_baseName + ".fuzzysearch-centroid-frequencies").c_str(),
      std::ios::in);
  _frequencyMap = new std::unordered_map<string, int, StringHashFunctionMarjan>();
  if (!f1.fail())
  {
    while (true)
    {
      string line;
      int tf;
      f1 >> line;
      if (f1.eof())
        break;
      f1 >> tf;
      (*_frequencyMap)[line] = tf;
    }
    f1.close();
    cout << "* Cluster-centroid frequencies are read from a file. " << endl;
  }
  return true;
}

// ____________________________________________________________________________
bool FuzzySearcherIso88591::init(const std::string& baseName)
{
  _baseName = baseName;
  // Load the fuzzy search data structure.
  string fuzzySearchDataStructureFileName = baseName +
      ".fuzzysearch-datastructure";
  _mode = getFuzzySearchIndexType(fuzzySearchDataStructureFileName);
  if (_mode == 0)  // permuted lexicon
    _fsAlgorithm = new PermutedLexicon<string>();
  else
    if (_mode == 1)
      _fsAlgorithm = new FastSS<string>();
    else
    {
      cerr << "* ERROR: Unknown fuzzysearch data structure type!" << endl;
      return false;
    }
  // Load the cluster ids (one list for each cluster center).
  string fuzzySearchClusterIdsFileName = baseName + ".fuzzysearch-clusters";
  _fsAlgorithm->loadDataStructureFromFile(fuzzySearchDataStructureFileName,
       &clusterCenters);
  readClusterIds<string>(fuzzySearchClusterIdsFileName, clusterCenters,
      &clusterIdsPerClusterCenter, &wordIdsPerCluster);
  fstream f((_baseName + ".fuzzysearch-clustercentroids").c_str(),
      std::ios::in);
  if (!f.fail())
  {
    isClusterCentroid.resize(clusterCenters.size());
    for (size_t i = 0; i < isClusterCentroid.size(); i++)
      isClusterCentroid[i] = false;
    while (true)
    {
      string line;
      f >> line;
      if (f.eof())
        break;
      int wordId = atoi(line.c_str());
      CS_ASSERT_GE(static_cast<int>(isClusterCentroid.size()), wordId);
      isClusterCentroid[wordId] = true;
    }
  }
  else
  {
    cout << "* ERROR: cluster-centroids file does not exist!" << endl;
    return false;
  }
  f.close();
  fstream f1((_baseName + ".fuzzysearch-centroid-frequencies").c_str(),
      std::ios::in);
  _frequencyMap = new std::unordered_map<string, int, StringHashFunctionMarjan>();
  if (!f1.fail())
  {
    while (true)
    {
      string line;
      int tf;
      f1 >> line;
      if (f1.eof())
        break;
      f1 >> tf;
      (*_frequencyMap)[line] = tf;
    }
    f1.close();
    cout << "* Cluster-centroid frequencies are read from a file. " << endl;
  }
  return true;
}

// ____________________________________________________________________________
bool orderBySecondCoord(const pair<int, double>& x, const pair<int, double>& y)
{
  return x.second < y.second;
}

// ____________________________________________________________________________
void FuzzySearcherUtf8::findClosestWords(const std::string& query,
                                         bool useTrivialAlg,
                                         bool* queryIsInLexicon,
                                         vector<int>* closestWordsIds,
                                         vector<double>* distances)
{
  CS_ASSERT(_fsAlgorithm != NULL);
  std::wstring queryAsWstring;
  string2wstring(query, &queryAsWstring);
  if (!useTrivialAlg)
    _fsAlgorithm->findClosestWords(queryAsWstring, clusterCenters,
        clusterCenters, queryIsInLexicon, closestWordsIds, distances);
  else
  {
    vector<pair<int, double> > matches;
    for (size_t i = 0; i < clusterCenters.size(); i++)
    {
      double thr = getThreshold();
      if (thr >= 1.0)
      {
        if (query.length() < 6)
          thr = 1;
        else
          if (query.length() < 11)
            thr = 2;
          else
            thr = 3;
      }
      if (thr >= 1)
      {
        if (abs(static_cast<int>(query.length()) -
            static_cast<int>(clusterCenters[i].length())) > thr)
          continue;
      }
      else
      {
        if (abs(static_cast<int>(query.length()) -
            static_cast<int>(clusterCenters[i].length())) > thr * query.length())  // NOLINT
          continue;
      }
      string ccAsString;
      wstring2string(clusterCenters[i], &ccAsString);
      double dist = getDistance(query, ccAsString);
      if (dist <= thr)
        matches.push_back(make_pair(i, dist));
    }
    sort(matches.begin(), matches.end(), orderBySecondCoord);
    closestWordsIds->clear();
    distances->clear();
    for (size_t i = 0; i < matches.size(); i++)
    {
      closestWordsIds->push_back(matches[i].first);
      distances->push_back(matches[i].second);
    }
  }
}

// ____________________________________________________________________________
void FuzzySearcherIso88591::findClosestWords(const std::string& query,
                                             bool useTrivialAlg,
                                             bool* queryIsInLexicon,
                                             vector<int>* closestWordsIds,
                                             vector<double>* distances)
{
  CS_ASSERT(_fsAlgorithm != NULL);
  if (!useTrivialAlg)
    _fsAlgorithm->findClosestWords(query, clusterCenters, clusterCenters,
        queryIsInLexicon, closestWordsIds, distances);
  else
  {
    vector<pair<int, double> > matches;
    for (size_t i = 0; i < clusterCenters.size(); i++)
    {
      double thr = getThreshold();
      if (thr >= 1.0)
      {
        if (query.length() < 6)
          thr = 1;
        else
          if (query.length() < 11)
            thr = 2;
          else
            thr = 3;
      }
      if (thr >= 1)
      {
        if (abs(static_cast<int>(query.length()) -
            static_cast<int>(clusterCenters[i].length())) > thr)
          continue;
      }
      else
      {
        if (abs(static_cast<int>(query.length()) -
            static_cast<int>(clusterCenters[i].length())) > thr * query.length())  // NOLINT
          continue;
      }
      double dist = getDistance(query, clusterCenters[i]);
      if (dist <= thr)
        matches.push_back(make_pair(i, dist));
    }
    sort(matches.begin(), matches.end(), orderBySecondCoord);
    closestWordsIds->clear();
    distances->clear();
    for (size_t i = 0; i < matches.size(); i++)
    {
      closestWordsIds->push_back(matches[i].first);
      distances->push_back(matches[i].second);
    }
  }
}

// ____________________________________________________________________________
template <class T>
void FuzzySearcherBase::readClusterIds(const string& fileName,
                                       const vector<T>& words,
                                       vector<vector<int> >* clusterIdsPerWord,
                                       vector<vector<int> >* wordIdsPerCluster)
{
  cout << "* Reading cluster ids from file \"" << fileName
       << "\" ... " << flush;
  CS_ASSERT(NULL != clusterIdsPerWord);
  CS_ASSERT_EQ(0, clusterIdsPerWord->size());
  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL)
  {
    cout << "unable to read cluster file!" << endl;
    return;
  }
  char* line = new char[CWF_MAX_LINE_LENGTH + 2];
  size_t lineNumber = 0;
  T previousWord;
  T word;
  size_t wordId = 0;
  while (true)
  {
    char* ret = fgets(line, CWF_MAX_LINE_LENGTH + 2, file);
    if (ret == NULL) break;
    lineNumber++;
    size_t pos = 0;
    while (line[pos] != '\t' && pos < CWF_MAX_LINE_LENGTH) pos++;
    if (pos == CWF_MAX_LINE_LENGTH)
    {
      perror("ERROR: missing tab in line. Aborting");
      exit(1);
    }
    CS_ASSERT_EQ('\t', line[pos]);
    line[pos] = 0;
    // string word = line;
    convertString(line, &word, pos);
    size_t clusterId = atoi(line + pos + 1);
    // If word occurs for the first time, start a new vector of cluster ids.
    CS_ASSERT_LT(0, word.size());
    if (word != previousWord)
    {
      wordId = clusterIdsPerWord->size();
//      if (wordId >= words.size())
//      {
//        cout << "words[wordId]: " << words[wordId] << endl;
//        cout << "word         : " << word << endl;
//      }
      CS_ASSERT_GT(words.size(), wordId);
      if (_usingNormalization)
      {
        while (words[wordId] != WordClusteringBuilder<T>::normalizeWord(word))
        {
          wordId++;
          vector<int> temp;
          clusterIdsPerWord->push_back(temp);
          if (wordId >= words.size())
          {
            cout << endl << "ERROR: Inconsistency data structure - clusters! ";
            exit(1);
          }
        }
      }
      else
      {
        while (words[wordId] != word)
        {
          wordId++;
          vector<int> temp;
          clusterIdsPerWord->push_back(temp);
          if (wordId >= words.size())
          {
            cout << endl << "ERROR: Inconsistency data structure - clusters! ";
            exit(1);
          }
        }
      }

      if (clusterId >= wordIdsPerCluster->size())
        wordIdsPerCluster->resize(clusterId + 1);
      (*wordIdsPerCluster)[clusterId].push_back(wordId);
      vector<int> clusterIds(1);
      clusterIds[0] = clusterId;
      clusterIdsPerWord->push_back(clusterIds);
    }
    // If word occurs for the second, third, etc. time, just push the
    // cluster id to the vector that is already there.
    else
    {
      clusterIdsPerWord->back().push_back(clusterId);
      if (clusterId >= wordIdsPerCluster->size())
        wordIdsPerCluster->resize(clusterId + 1);
      (*wordIdsPerCluster)[clusterId].push_back(wordId);
    }
    previousWord = word;
  }
  delete[] line;
  cout << "done, #clustered words = "  << clusterIdsPerWord->size() << endl;
  // check if bdname.fuzzysearch-fwrange is there, if so, read it
  std::fstream f((_baseName+".fuzzysearch-fwrange").c_str(), ios::in);
  if (!f.fail())
  {
    int x;
    f >> x;
    _clusterIdFWBeg = x;
    f >> x;
    _clusterIdFWEnd = x;
    f.close();
    cout << "* cluster-id range for frequent words: ("
         << _clusterIdFWBeg << ", " << _clusterIdFWEnd <<  ")" << endl;
  }
  else
  {
    _clusterIdFWBeg = _clusterIdFWEnd = -1;
  }
}
}
