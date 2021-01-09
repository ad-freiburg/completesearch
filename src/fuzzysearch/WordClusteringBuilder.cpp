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
#include <unordered_set>

#include "../fuzzysearch/WordClusteringBuilder.h"
#include "../server/Exception.h"

namespace FuzzySearch
{
using std::cout;
using std::fstream;

template <class T>
size_t WordClusteringBuilder<T>::totalNumberOfOccurences;

// ____________________________________________________________________________
template <class T>
static bool orderByFirstCoord(const pair<T, int>& x,
                              const pair<T, int>& y)
{
  return x.first < y.first;
}

// ____________________________________________________________________________
template <class T>
static bool orderByFirstCoordReverse(const pair<T, int>& x,
                                     const pair<T, int>& y)
{
  return x.first > y.first;
}

// ____________________________________________________________________________
template <class T>
static bool orderBySecondCoord(const pair<T, int>& x,
                               const pair<T, int>& y)
{
  return x.second < y.second;
}

// ____________________________________________________________________________
template <class T>
static bool orderBySecondCoordReverse(const pair<T, int>& x,
                                      const pair<T, int>& y)
{
  return x.second > y.second;
}

// ____________________________________________________________________________
static bool sortBySecondCoordDec(const pair<int, int>& x,
                                 const pair<int, int>& y)
{
  return x.second > y.second;
}

// ____________________________________________________________________________
template <class T>
bool WordClusteringBuilder<T>::isWordGarbage(const char* word)
{
  double digits = 0,
        maxNonVowelRun = 0,
        nonVowelRun = 0;
  int cont = 1;
  int maxCont = 1;
  uint32_t len = strlen(word);
  for (uint32_t i = 0; i < len; i++)   // check the digit ratio and
  {                                    // the longest non-vowel run
    if (word[i] == '_' || word[i] == ' ')
      return true;
    if (isdigit(word[i]))
    {
      digits++;
    }
    if (!isVowel(word[i]) && (static_cast<unsigned char>(word[i]) < 128))
    {
      nonVowelRun++;
      if (maxNonVowelRun < nonVowelRun)
        maxNonVowelRun = nonVowelRun;
    }
    else
      nonVowelRun = 0;
    if (i > 0)
    {
      if (word[i] == word[i-1])
      {
        cont++;
        if (cont > maxCont)
          maxCont = cont;
      }
      else
      {
        cont = 1;
      }
    }
  }
  if (digits / len >= 0.5 || 1.0 * maxCont / len > 0.5 ||
      ((/*digits > 1 ||*/ maxNonVowelRun >= 4 || maxCont >= 4)))
    return true;
  return false;
}

// ____________________________________________________________________________
template <>
int WordClusteringBuilder<string>::readVocabularyFromDisk(
                                               const string& vocFilename,
                                               bool filterGarbage,
                                               int maxNofWords,
                                               vector<string>* vocabulary,
                                               vector<int>* frequencies)
{
  cout << endl;
  cout << "Reading vocabulary and frequencies from " << vocFilename
      << "... " << flush;
  string line_str;
  unsigned int freq;
  CS_ASSERT(vocabulary != NULL);
  vocabulary->clear();
  fstream f;
  // fstream f1;
  f.open(vocFilename.c_str(), ios::in);
  // f1.open(freqFilename.c_str(), ios::in);
  if (!f)
  {
    cout << endl << "ERROR: Could not open file. Aborting ..." << endl;
    exit(1);
  }
  // 1. get stopwords from a file
  std::unordered_map<string, bool, StringHashFunction> stopWords;
  fstream swf("stop_words", ios::in);
  if (!swf.fail())
  {
    cout << "[ Getting stop-words from a file ... " << flush;
    while (!swf.eof())
    {
      swf >> line_str;
      stopWords[line_str] = true;
    }
    swf.close();
    cout << "done, " << stopWords.size() << " stop-words ]" << endl << flush;
  }
  else
    getStopWords(&stopWords);
  int garbageWords = 0;
  std::unordered_map<string, bool, StringHashFunction> vocaHashMap;

  // 2. do the actual reading, line by line with frequencies, checking
  //    for stopwords and garbage
  WordClusteringBuilder::totalNumberOfOccurences = 0;
  while (static_cast<int>(vocabulary->size()) <= maxNofWords)
  {
    f >> line_str >> freq;
    WordClusteringBuilder::totalNumberOfOccurences += freq;
    if (f.eof())
    {
      f.close();
      break;
    }
    if (filterGarbage && isWordGarbage(line_str.c_str()))
    {
      garbageWords++;
      continue;
    }
    if (line_str.length() < MIN_WORD_LEN || line_str.length() > MAX_WORD_LEN
        || stopWords.count(line_str) > 0)
      continue;
    char* p = (char*)line_str.c_str();  // NOLINT
    while (*p != 0) { p += utf8_tolower(p); }
    if (vocaHashMap.count(line_str) == 0)
    {
      vocabulary->push_back(line_str);
      frequencies->push_back(freq);
      vocaHashMap[line_str] = true;
    }
  }
  cout << "[ " << (*vocabulary).size() << " words, " << garbageWords
       << " garbage words ]" << endl;
  return WordClusteringBuilder::totalNumberOfOccurences;
}

// ____________________________________________________________________________
template <>
int WordClusteringBuilder<wstring>::readVocabularyFromDisk(
                                               const string& vocFilename,
                                               bool filterGarbage,
                                               int maxNofWords,
                                               vector<wstring>* vocabulary,
                                               vector<int>* frequencies)
{
  vector<string> tmpVocabulary;
  WordClusteringBuilder::totalNumberOfOccurences =
      WordClusteringBuilder<string>::readVocabularyFromDisk(vocFilename,
      filterGarbage, maxNofWords, &tmpVocabulary, frequencies);
  vocabulary->clear();
  vocabulary->reserve(tmpVocabulary.size());
  wstring tmpwstr;
  int utf8ConversionFailures = 0;
  for (unsigned int i = 0; i < tmpVocabulary.size(); i++)
  {
    if (!string2wstring(tmpVocabulary[i], &tmpwstr))
      utf8ConversionFailures++;
    vocabulary->push_back(tmpwstr);
  }
  if (utf8ConversionFailures > 0)
    cout << "[ Warning: " << utf8ConversionFailures << " words failed to "
        "convert to utf-8! Check your set encoding! ]" << endl;
  return totalNumberOfOccurences;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::readVocabularyFromDisk(
                                               const string& filename,
                                               bool filterGarbage,
                                               int maxNofWords,
                                               bool sortVocabulary,
                                               vector<string>* vocabulary)
{
  cout << endl;
  cout << "Reading from vocabulary file " << filename << "... " << flush;
  string line_str;
  CS_ASSERT(vocabulary != NULL);
  vocabulary->clear();
  fstream f;
  f.open(filename.c_str(), ios::in);
  if (!f)
  {
    cout << endl << "ERROR: Could not open " << filename
         << ". Aborting ..." << endl;
    exit(1);
  }

  // 1. get stopwords from a file
  std::unordered_map<string, bool, StringHashFunction> stopWords;
  fstream swf("stop_words", ios::in);
  if (!swf.fail())
  {
    cout << "[ Getting stop-words from a file ... " << flush;
    while (!swf.eof())
    {
      swf >> line_str;
      stopWords[line_str] = true;
    }
    swf.close();
    cout << "done, " << stopWords.size() << " stop-words ]" << endl << flush;
  }
  else
    getStopWords(&stopWords);
  int garbageWords = 0;
  std::unordered_map<string, bool, StringHashFunction> vocaHashMap;

  // 2. do the actual reading line by line, without frequencies plus sort the
  //    vocabulary
  size_t totalDictSize = 0;
  while (static_cast<int>(vocabulary->size()) <= maxNofWords)
  {
    getline(f, line_str);
    if (f.eof())
      break;
    if (filterGarbage && (isWordGarbage(line_str.c_str())
        || stopWords.count(line_str) > 0))
    {
      garbageWords++;
      continue;
    }
    if (line_str.length() < MIN_WORD_LEN || line_str.length() > MAX_WORD_LEN)
      continue;
    totalDictSize += line_str.length();
    char* p = (char*)line_str.c_str();  // NOLINT
    while (*p != 0) { p += utf8_tolower(p); }
    if (vocaHashMap.count(line_str) == 0)
    {
      (*vocabulary).push_back(line_str);
      vocaHashMap[line_str] = true;
    }
  }
  f.close();

  // 3. sort it (if needed)
  if (sortVocabulary)
    sort(vocabulary->begin(), vocabulary->end());
  cout << "[ " << (*vocabulary).size() << " words (" << totalDictSize
       << " B), " << garbageWords
       << " garbage words ]" << endl;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::readVocabularyFromDisk(
                                               const string& filename,
                                               bool filterGarbage,
                                               int maxNofWords,
                                               bool sortVocabulary,
                                               vector<wstring>* vocabulary)
{
  vector<string> tmpVocabulary;
  WordClusteringBuilder<string>::readVocabularyFromDisk(filename,
      filterGarbage, maxNofWords, sortVocabulary, &tmpVocabulary);
  vocabulary->clear();
  vocabulary->reserve(tmpVocabulary.size());
  wstring tmpwstr;
  int utf8ConversionFailures = 0;
  for (unsigned int i = 0; i < tmpVocabulary.size(); i++)
  {
    if (!string2wstring(tmpVocabulary[i], &tmpwstr))
      utf8ConversionFailures++;
    vocabulary->push_back(tmpwstr);
  }
  if (utf8ConversionFailures > 0)
    cout << "[ Warning: " << utf8ConversionFailures << " words failed to "
        "convert to utf-8! Check your set encoding! ]" << endl;
}

// legacy
// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::getCorrectWordIds(const string& dictFileName,
    const vector<T>& vocabulary,
    vector<int>* correctWordIds)
{
    fstream dictFile;
    (*correctWordIds).clear();
    dictFile.open(dictFileName.c_str(), ios::in);
    std::unordered_map<T, bool, StringHash<T> > dictHashMap;  // NOLINT
    if (!dictFile.fail())
    {
      cout << "Reading the dictionary from " << dictFileName
           << "... " << flush;
      T word;
      unsigned int count = 0;
      while (count < INT_MAX)
      {
        readLine(&dictFile, &word, false);
        // dictFile >> word;
        if (dictFile.eof())
          break;
        if (word.length() > MAX_WORD_LEN || word.length() < MIN_WORD_LEN)
          continue;
        // char* p = (char*)word.c_str();  // NOLINT
        // while (*p != 0) { p += utf8_tolower(p); }
        dictHashMap[word] = true;
        count++;
      }
      // _isClusterCenter.clear();
      _isClusterCenter.resize(vocabulary.size());
      // _isCorrectWord.clear();
      // _isCorrectWord.resize(vocabulary.size());
      for (uint32_t i = 0; i < vocabulary.size(); i++)
      {
        word = vocabulary[i];
        if (dictHashMap.count(word) > 0)
        {
          (*correctWordIds).push_back(i);
          // _isClusterCenter[i] = true;
          // _isCorrectWord[i] = true;
        }
      }
      cout << "[ " << (*correctWordIds).size() << " valid words ]" << endl;
      dictFile.close();
    }
    else
    {
      cerr << endl << "ERROR reading from dictionary filename: "
           << dictFileName << ". Exiting." << endl;
      exit(1);
    }
}

// legacy
// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::pickClusterCenters(const vector<T>& vocabulary,
    const vector<int>& correctWordIds,
    FuzzySearchAlgorithm<T>& fsAlg,
    double coverage, vector<T>*
    clusterCenters)
{
  cout << "Picking cluster centroids ... " << endl << flush;
  if (vocabulary.size() == 0 || correctWordIds.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  CS_ASSERT_LE(coverage, 1);
  vector<bool> pickedWords;
  // PermutedLexicon permutedLexicon;
  // buildPermutedLexicon(vocabulary, 4, &permutedLexicon);
  ProgressIndicator progressIndicator(coverage * vocabulary.size(), 10);
  fsAlg.buildIndex(vocabulary, false);
  double covered = 0;
  srand(0);
  vector<int> closestWords;
  vector<double> distances;
  if (_isClusterCenter.size() != vocabulary.size())
    _isClusterCenter.resize(vocabulary.size());
  unsigned int count = 0;
  int nextWord = 0;
  clusterCenters->clear();
  pickedWords.resize(vocabulary.size());
  unsigned int seed = 157;  // time(NULL);
  Timer timer;
  timer.start();
  cout << "working ... " << flush;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
  {
    pickedWords[i] = false;
    _isClusterCenter[i] = false;
  }
  while (covered / vocabulary.size() < coverage)
  {
    // 1. first pick the correct words as cluster centroids
    // (clustering them too, i.e. in this variant not each correct word will
    // be a cluster centroid)
    if (count < correctWordIds.size())
    {
      nextWord = correctWordIds[count++];
      while (pickedWords[nextWord])
      {
        if (count == correctWordIds.size())
        {
          nextWord = -1;
          break;
        }
        nextWord = correctWordIds[count++];
        if (count == correctWordIds.size())
        {
          nextWord = -1;
          break;
        }
      }
    }
    else
    {
      // break;
      nextWord = -1;
    }

    // 2. pick the rest of the words as cluster centroids
    if (nextWord == -1)
    {
      nextWord = rand_r(&seed) % vocabulary.size();
      CS_ASSERT_GT(static_cast<int>(pickedWords.size()), nextWord);
      while (pickedWords[nextWord] /*_isClusterCenter[nextWord]*/)
      {
        nextWord = rand_r(&seed) % vocabulary.size();
        /*
        nextWord++;
        if (nextWord >= pickedWords.size())
          nextWord = 0;
        */
      }
    }
    // find the environment / range of closest words
    CS_ASSERT_GT(static_cast<int>(vocabulary.size()), nextWord);
    fsAlg.findClosestWords(vocabulary[nextWord],
                           vocabulary,
                           vocabulary,
                           NULL,
                           &closestWords,
                           &distances);
    // experimental (does not work so well and it's not executed anyway)
    /*
    if (_isCorrectWord[nextWord] && closestWords.size() < 0)
    {
      vector<int> closestWordsTemp;
      vector<int> closestWordsMax = closestWords;
      int maxWord = -1;
      for (unsigned int j = 0; j < closestWords.size(); j++)
      {
        if (!_isClusterCenter[closestWords[j]])
        {
          findClosestWordsPermutedLexicon(vocabulary[closestWords[j]],
                                          vocabulary, permutedLexicon,
                                          vocabulary, NULL, 30,
                                          &closestWordsTemp,
                                          &distances);
          if (closestWordsTemp.size() > closestWordsMax.size())
          {
            maxWord = closestWords[j];
            closestWordsMax = closestWordsTemp;
          }
        }
      }
      if (maxWord > -1)
      {
        nextWord = maxWord;
        closestWords = closestWordsMax;
      }
    }
    */
    // 3. mark the similar words + the chosen centroid as picked
    pickedWords[nextWord] = true;
    _isClusterCenter[nextWord] = true;
    covered++;
    for (unsigned int i = 0; i < closestWords.size(); i++)
    {
      CS_ASSERT_GT(static_cast<int>(pickedWords.size()), closestWords[i]);
      if (!pickedWords[closestWords[i]])
      {
        // if (!_isClusterCenter[i])
        covered++;
        pickedWords[closestWords[i]] = true;
      }
    }
    progressIndicator.update(covered);
  }
  // sort(clusterCenters->begin(), clusterCenters->end());
  for (unsigned int i = 0; i < _isClusterCenter.size(); i++)
    if (_isClusterCenter[i])
      clusterCenters->push_back(vocabulary[i]);
  timer.stop();
  cout << endl
       << "done picking cluster centroids. ("
       << timer.msecs() / 1000.0 << " secs.) "
       << clusterCenters -> size() << " centroids picked ("
       << 1.0 * clusterCenters->size() / vocabulary.size() * 100 << " %)"
       << endl << flush;
}

// legacy
// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::pickClusterCenters(const vector<T>& vocabulary,
                                            FuzzySearchAlgorithm<T>& fsAlg,
                                            double coverage,
                                            int cutOff,
                                            vector<T>* clusterCenters,
                                            vector<int>* unclusteredWords)
{
  cout << "Picking cluster centroids (w.r.t. frequencies) ... "
       << endl << flush;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  CS_ASSERT_LE(coverage, 1);
  // PermutedLexicon permutedLexicon;
  // buildPermutedLexicon(vocabulary, 4, &permutedLexicon);
  fsAlg.buildIndex(vocabulary, false);
  double covered = 0;
  vector<int> closestWords;
  vector<double> distances;
  ProgressIndicator progressIndicator(coverage * vocabulary.size(), 10);
  if (_isClusterCenter.size() != vocabulary.size())
    _isClusterCenter.resize(vocabulary.size());
  int nextWord = vocabulary.size();
  clusterCenters->clear();
  _pickedWords.resize(vocabulary.size());
  list<int> clusterCentersList;
  Timer timer;
  timer.start();
  cout << "working ... " << flush;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
  {
    _pickedWords[i] = false;
    _isClusterCenter[i] = false;
  }
  while (covered / vocabulary.size() < coverage)
  {
    progressIndicator.update(covered);
    nextWord--;
    while (_pickedWords[nextWord])
    {
      nextWord--;
      if (nextWord == -1)
        break;
    }
    if (nextWord == -1)
      break;
    fsAlg.findClosestWords(vocabulary[nextWord],
                           vocabulary,
                           vocabulary,
                           NULL,
                           &closestWords,
                           &distances);
    if (!_pickedWords[nextWord])
      covered++;
    _pickedWords[nextWord] = true;
    _isClusterCenter[nextWord] = true;
    clusterCentersList.push_front(nextWord);
    for (unsigned int i = 0; i < closestWords.size(); i++)
    {
      if (!_pickedWords[closestWords[i]])
      {
        covered++;
        _pickedWords[closestWords[i]] = true;
      }
    }
  }
  _minimumFrequency = INT_MAX;
  list<int>::iterator iter;
  for (iter = clusterCentersList.begin(); iter != clusterCentersList.end();
      iter++)
    clusterCenters->push_back(vocabulary[*iter]);
  if (unclusteredWords != NULL)
    for (unsigned int i = 0; i < vocabulary.size(); i++)
      if (!_pickedWords[i])
        unclusteredWords->push_back(i);
  timer.stop();
  cout << endl
       << "done picking cluster centroids. ("
       << timer.msecs() / 1000.0 << " secs.) "
       << clusterCenters -> size() << " centroids picked ("
       << 1.0 * clusterCenters->size() / vocabulary.size() * 100 << " %)"
       << endl << flush;
}

// legacy
// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::pickClusterCenters(const vector<T>& vocabulary,
                                            const vector<int>& frequencies,
                                            FuzzySearchAlgorithm<T>& fsAlg,
                                            int minFrequency,
                                            int cutOff,
                                            vector<T>* clusterCenters)
{
  cout << "Picking cluster centroids (w.r.t. freq./nocov.) ... "
       << endl
       << flush;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  int firstWord = vocabulary.size()-1;
  CS_ASSERT(frequencies.size() == vocabulary.size());
  while (frequencies[firstWord] >= minFrequency)
    firstWord--;
  fsAlg.buildIndex(vocabulary, false);
  vector<int> closestWords;
  vector<double> distances;
  ProgressIndicator progressIndicator(vocabulary.size()-firstWord, 10);
  if (_isClusterCenter.size() != vocabulary.size())
    _isClusterCenter.resize(vocabulary.size());
  int nextWord = vocabulary.size();
  clusterCenters->clear();
  _pickedWords.resize(vocabulary.size());
  list<int> clusterCentersList;
  Timer timer;
  timer.start();
  cout << "working ... " << flush;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
  {
    _pickedWords[i] = false;
    _isClusterCenter[i] = false;
  }
  while (nextWord > firstWord)
  {
    progressIndicator.update(vocabulary.size()-nextWord);
    nextWord--;
    while (_pickedWords[nextWord])
    {
      nextWord--;
      if (nextWord == -1)
        break;
    }
    if (nextWord == -1)
      break;
    fsAlg.findClosestWords(vocabulary[nextWord],
                                    vocabulary,
                                    vocabulary,
                                    NULL,
                                    &closestWords,
                                    &distances);
    _pickedWords[nextWord] = true;
    _isClusterCenter[nextWord] = true;
    clusterCentersList.push_front(nextWord);
    for (unsigned int i = 0; i < closestWords.size(); i++)
      if (!_pickedWords[closestWords[i]])
        _pickedWords[closestWords[i]] = true;
  }
  _minimumFrequency = INT_MAX;
  list<int>::iterator iter;
  for (iter = clusterCentersList.begin(); iter != clusterCentersList.end();
      iter++)
    clusterCenters->push_back(vocabulary[*iter]);
  /*
  if (unclusteredWords != NULL)
    for (unsigned int i = 0; i < vocabulary.size(); i++)
      if (!_pickedWords[i])
        unclusteredWords->push_back(i);
  */
  _pickedWords.clear();
  timer.stop();
  cout << endl
       << "done picking cluster centroids. ("
       << timer.msecs() / 1000.0 << " secs.) "
       << clusterCenters->size() << " centroids picked ("
       << 1.0 * clusterCenters->size() / vocabulary.size() * 100 << " %)"
       << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::pickClusterCenters(
                        const vector<T>& vocabulary,
                        const vector<int>& frequencies,
                        int minFrequency,
                        vector<T>* clusterCenters,
                        vector<int>* freq)
{
  cout << "Picking cluster centroids (w.r.t. frequencies/simple) ... "
       << endl << flush;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  _minimumFrequency = minFrequency;
  if (_isClusterCenter.size() != vocabulary.size())
    _isClusterCenter.resize(vocabulary.size());
  int nextWord = vocabulary.size() - 1;
  clusterCenters->clear();
  list<pair<int, int> > clusterCentersList;
  Timer timer;
  timer.start();
  cout << "working ... " << flush;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
    _isClusterCenter[i] = false;
  while (frequencies[nextWord] >= minFrequency && nextWord >= 0)
  {
    _isClusterCenter[nextWord] = true;
    clusterCentersList.push_front(make_pair(nextWord, frequencies[nextWord]));
    nextWord--;
  }
  list<pair<int, int> >::iterator iter;
  for (iter = clusterCentersList.begin(); iter != clusterCentersList.end();
      iter++)
  {
    clusterCenters->push_back(vocabulary[(*iter).first]);
    freq->push_back((*iter).second);
  }
  timer.stop();
  cout << endl
       << "done picking cluster centroids. (" << timer.msecs() / 1000.0
       << " secs.) " << clusterCenters -> size() << " centroids picked ("
       << 1.0 * clusterCenters->size() / vocabulary.size() * 100 << " %)"
       << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::sortParallel(
    vector<T>* vocabulary,
    vector<int>* freq,
    int col,
    bool reverse)
{
  vector<pair<T, int> > tempVec;
  tempVec.resize(vocabulary->size());
  for (size_t i = 0; i < vocabulary->size(); i++)
  {
    tempVec[i].first  = (*vocabulary)[i];
    tempVec[i].second = (*freq)[i];
  }
  if (col == 1)
  {
    if (!reverse)
      sort(tempVec.begin(), tempVec.end(), orderByFirstCoord<T>);
    else
      sort(tempVec.begin(), tempVec.end(), orderByFirstCoordReverse<T>);
  }
  else
  {
    if (!reverse)
      sort(tempVec.begin(), tempVec.end(), orderBySecondCoord<T>);
    else
      sort(tempVec.begin(), tempVec.end(), orderBySecondCoordReverse<T>);
  }
  for (size_t i = 0; i < vocabulary->size(); i++)
  {
    (*vocabulary)[i] = tempVec[i].first;
    (*freq)[i] = tempVec[i].second;
  }
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::sortParallel(vector<int>* vocabulary,
                         vector<int>* freq)
{
  vector<pair<int, int> > tempVec;
  tempVec.resize(vocabulary->size());
  for (size_t i = 0; i < vocabulary->size(); i++)
  {
    tempVec[i].first  = (*vocabulary)[i];
    tempVec[i].second = (*freq)[i];
  }
  sort(tempVec.begin(), tempVec.end(), sortBySecondCoordDec);
  for (size_t i = 0; i < vocabulary->size(); i++)
  {
    (*vocabulary)[i] = tempVec[i].first;
    (*freq)[i] = tempVec[i].second;
  }
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::pickClusterCentersCompl(
                        const vector<T>& vocabulary,
                        const vector<int>& frequencies,
                        int minFrequency,
                        int prefixLen,
                        bool prefixLenStrict,
                        vector<T>* clusterCenters)
{
  cout << "Picking cluster centroids (w.r.t. frequencies/simple/compl) ... "
       << flush;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  _minimumFrequency = minFrequency;
  if (_isClusterCenter.size() != vocabulary.size())
    _isClusterCenter.resize(vocabulary.size());
  int nextWord = 0;
  clusterCenters->clear();
  vector<pair<int, int> > clusterCentersList;
  Timer timer;
  timer.start();
  vector<bool> isClusterCenterOld = _isClusterCenter;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
    _isClusterCenter[i] = false;
  int currentPrefixLen = -1;
  cout << "working ... " << flush;
  clusterCenters->clear();
  size_t vol = 0;
  int prevWord = -1;
  T prevWordStr;
  while (nextWord < static_cast<int>(vocabulary.size()))
  {
    if (frequencies[nextWord] >= minFrequency)
    {
      currentPrefixLen = INT_MAX;
      if (prevWord != -1)
      {
          prevWordStr = vocabulary[prevWord];
          currentPrefixLen = 0;
          size_t len = MY_MIN(vocabulary[nextWord].length(),
              prevWordStr.length());
          for (size_t i = 0; i < len; i++)
          {
            if (vocabulary[nextWord][i] == prevWordStr[i])
              currentPrefixLen++;
            else
              break;
          }
      }
      if (currentPrefixLen < prefixLen
          /*&& currentPrefixLen < vocabulary[nextWord].length()*/)
      {
          if (prefixLenStrict)
          {
            if (static_cast<int>(vocabulary[prevWord].length()) >= prefixLen)
              clusterCentersList.push_back(std::make_pair(prevWord, vol));
          }
          else
            clusterCentersList.push_back(std::make_pair(prevWord, vol));
        vol = 0;
      }
      vol += frequencies[nextWord];
      _isClusterCenter[nextWord] = true;
      prevWord = nextWord;
    }
    nextWord++;
  }
  if (vocabulary[nextWord - 1] != vocabulary[clusterCentersList.back().first])
    clusterCentersList.push_back(std::make_pair(nextWord - 1, vol));
  sort(clusterCentersList.begin(), clusterCentersList.end(),
      sortBySecondCoordDec);
  for (size_t i = 0; i < clusterCentersList.size(); i++)
  {
    clusterCenters->push_back(vocabulary[clusterCentersList[i].first]);
  }
  timer.stop();
  cout << endl
       << "done picking cluster centroids. (" << timer.msecs() / 1000.0
       << " secs.) " << clusterCenters -> size() << " centroids picked ("
       << 1.0 * clusterCenters->size() / vocabulary.size() * 100 << " %)"
       << endl << flush;
}

bool isStar(const string& str, int pos)
{
  return (str[pos] == '*');
}

bool isStar(const wstring& str, int pos)
{
  return (str[pos] == L'*');
}

void addStar(int pos, string* str)
{
  (*str)[pos] = '*';
}

void addStar(int pos, wstring* str)
{
  (*str)[pos] = L'*';
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::addBlocksSubs(const T& str,
                   uint16_t beg,
                   int depth,
                   int maxDepth,
                   std::unordered_set<T, StringHash<T> >& h,
                   vector<T>* centroids)
{
  if (str.length() == 0)
    return;
  if (depth == maxDepth)
  {
    if (h.find(str) == h.end())
    {
      h.insert(str);
      centroids->push_back(str);
    }
  }
  if (depth >= maxDepth)
    return;
  for (int i = beg; i < static_cast<int>(str.length()); i++)
  {
    T tempStr;
    tempStr.resize(str.length());
    int counter = 0;
    bool call = false;
    for (int j = 0; j < static_cast<int>(str.length()); j++)
    {
      if (j == i && !isStar(str, j))
      {
        addStar(counter++, &tempStr);
        // tempStr[counter++] = '*';
        call = true;
      }
      else
        tempStr[counter++] = str[j];
    }
    if (call)
      addBlocksSubs(tempStr, i, depth + 1, maxDepth, h, centroids);
  }
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::pickClusterCentersComplStar(
                        const vector<T>& vocabulary,
                        const vector<int>& frequencies,
                        size_t minFrequency,
                        size_t prefixLength,
                        int nofErrors,
                        bool setAsMainClusterCentroids,
                        vector<vector<T> >* clusterCenters,
                        vector<T>* newVoca,
                        vector<int>* newFreq)
{
  cout << "Picking cluster centroids (w.r.t. frequencies/star/compl) ... "
       << flush;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  _minimumFrequency = minFrequency;
  if (setAsMainClusterCentroids)
  {
    if (_isClusterCenter.size() != vocabulary.size())
      _isClusterCenter.resize(vocabulary.size());
    for (size_t j = 0; j < _isClusterCenter.size(); j++)
      _isClusterCenter[j] = false;
  }
  T prefix;
  size_t vol = 0;
  size_t discarded = 0;
  size_t total = 0;
  vector<T> prefixes;
  vector<int> freqs;
  vector<int> wordsInPrefix;
  Timer timer;
  timer.start();
  clusterCenters->clear();
  if (newVoca != NULL && newFreq != NULL)
  {
    newVoca->clear();
    newFreq->clear();
  }
  for (size_t i = 0; i < vocabulary.size(); i++)
  {
    wordsInPrefix.push_back(i);
    if (prefix.length() < prefixLength
        || (prefix != vocabulary[i].substr(0, MY_MIN(prefixLength, vocabulary[i].length())))  // NOLINT
        || (i == vocabulary.size() - 1))
    {
      if (prefix.length() > 0)
      {
        if (prefix.length() == prefixLength)
        {
          if (vol >= minFrequency)
          {
            // (*hm)[prefix] = vol;
            prefixes.push_back(prefix);
            freqs.push_back(vol);
            for (size_t j = 0; j < wordsInPrefix.size(); j++)
            {
              if (setAsMainClusterCentroids)
                _isClusterCenter[wordsInPrefix[j]] = true;
              if (newVoca != NULL && newFreq != NULL)
              {
                newVoca->push_back(vocabulary[wordsInPrefix[j]]);
                newFreq->push_back(frequencies[wordsInPrefix[j]]);
              }
            }
            total++;
          }
          else
          {
            // if (hm_rare != NULL)
            //  (*hm_rare)[prefix] = vol;
            discarded++;
          }
        }
        wordsInPrefix.clear();
        vol = 0;
      }
      prefix = vocabulary[i].substr(0,
          MY_MIN(prefixLength, vocabulary[i].length()));
      vol += frequencies[i];
    }
    else
    {
      vol += frequencies[i];
    }
  }
  sortParallel(&prefixes, &freqs, 2);
  if (newVoca != NULL && newFreq != NULL)
    sortParallel(newVoca, newFreq, 1);
  std::unordered_set<T, StringHash<T> > hashSet;
  if (nofErrors == 0)
    clusterCenters->resize(1);
  size_t nofCCPicked = 0;
  for (int i = prefixes.size() - 1; i >= 0; i--)
  {
    if (nofErrors > 0)
    {
      clusterCenters->resize(clusterCenters->size() + 1);
      addBlocksSubs(prefixes[i], 0, 0, nofErrors, hashSet,
          &clusterCenters->back());
      nofCCPicked += clusterCenters->back().size();
    }
    else
    {
      (*clusterCenters)[0].push_back(prefixes[i]);
      nofCCPicked++;
    }
  }
  timer.stop();
  cout << endl
       << "done picking cluster centroids. (" << timer.msecs() / 1000.0
       << " secs.) " << nofCCPicked << " centroids picked ("
       << 1.0 * nofCCPicked / vocabulary.size() * 100 << " %)"
       << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::chopWords(const vector<T>& vocabulary,
               size_t prefixLength,
               int nofErrors,
               vector<T>* choppedWords)
{
  vector<T> prefixes;
  for (size_t i = 0; i < vocabulary.size(); i++)
  {
    T prefix = vocabulary[i].substr(0,
        MY_MIN(prefixLength, vocabulary[i].length()));
    if (prefix.length() < prefixLength)
      continue;
    if (prefixes.empty())
      prefixes.push_back(prefix);
    else
    {
      if (prefix != prefixes.back())
        prefixes.push_back(prefix);
    }
  }
  choppedWords->clear();
  if (nofErrors <= 0)
  {
    *choppedWords = prefixes;
    return;
  }
  std::unordered_set<T, StringHash<T> > hashSet;
  for (size_t i = 0; i < prefixes.size(); i++)
  {
    addBlocksSubs(prefixes[i], 0, 0, nofErrors, hashSet, choppedWords);
  }
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::buildWordClusteringFrequentWords(
                              const vector<T>& vocabulary,
                              vector<int>& frequencies,
                              int maxNofClustersPerWord,
                              int minFrequency,
                              vector<T>* clusterCenters,
                              vector<vector<int> >* clusters,
                              vector<int>* unclusteredWords)
{
  cout << "Building clustering for the frequent words ... "
       << endl << flush;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: empty vocabulary!" << endl;
    exit(1);
  }
  vector<T> frequentWords;
  size_t spaceOverhead = 0;
  double avgFreq = 0;
  for (int i = vocabulary.size() - 1; i >= 0; i--)
  {
    avgFreq += frequencies[i];
    if (frequencies[i] < minFrequency)
      break;
    frequentWords.push_back(vocabulary[i]);
  }
  avgFreq /= frequencies.size();
  FastSS<T> fsAlg(2, 0);
  fsAlg.buildIndex(frequentWords, false);
  vector<int> closestWords;
  vector<double> distances;
  ProgressIndicator progressIndicator(frequentWords.size(), 10);
  // if (_isClusterCenter.size() != vocabulary.size());
  //   _isClusterCenter.resize(vocabulary.size());
  size_t nextWord = 0;
  clusterCenters->clear();
  // clusters->clear();
  vector<int> pickedWords;
  pickedWords.resize(vocabulary.size());
  _isClusterCenter.resize(vocabulary.size());
  list<int> clusterCentersList;
  Timer timer;
  timer.start();
  cout << "working ... " << flush;
  for (unsigned int i = 0; i < frequentWords.size(); i++)
  {
    _isClusterCenter[vocabulary.size() - 1 - i] = true;
  }
  for (unsigned int i = 0; i < pickedWords.size(); i++)
    pickedWords[i] = maxNofClustersPerWord;
  int covered = 0;
  while (nextWord < frequentWords.size())
  {
    // find next word with "available" cluster assignments
    while (pickedWords[vocabulary.size() - 1 - nextWord] == 0)
    {
      nextWord++;
      if (nextWord == frequentWords.size())
        break;
    }
    if (nextWord == frequentWords.size())
      break;
    fsAlg.findClosestWords(frequentWords[nextWord],
                           frequentWords,
                           frequentWords,
                           NULL,
                           &closestWords,
                           &distances);
    // string str;
    // toString(frequentWords[nextWord], &str);
    // cout << str << "\t" << closestWords.size() << endl;
    if (pickedWords[vocabulary.size() - 1 - nextWord] > 0)
      covered++;
    clusterCentersList.push_front(nextWord);
    if (closestWords.size() > 1)
    {
      vector<int> tempCluster;
      for (size_t i = 0; i < closestWords.size(); i++)
      {
        if (pickedWords[vocabulary.size() - 1 - closestWords[i]] > 0)
        {
          tempCluster.push_back(vocabulary.size() - 1 - closestWords[i]);
        }
      }
      if (tempCluster.size() > 1)
      {
        clusters->push_back(tempCluster);
        for (size_t j = 0; j < tempCluster.size(); j++)
        {
          spaceOverhead += frequencies[tempCluster[j]];
          pickedWords[tempCluster[j]]--;
          covered++;
        }
      }
    }
    nextWord++;
    progressIndicator.update(nextWord);
  }
  _minimumFrequency = INT_MAX;
  list<int>::iterator iter;
  for (iter = clusterCentersList.begin(); iter != clusterCentersList.end();
      iter++)
    clusterCenters->push_back(vocabulary[*iter]);
  if (unclusteredWords != NULL)
    for (unsigned int i = 0; i < vocabulary.size(); i++)
      if (pickedWords[i] > 0)
        unclusteredWords->push_back(i);
  timer.stop();
  cout << endl
       << "done. ("
       << timer.msecs() / 1000.0 << " secs.) "
       << clusterCenters->size() << " centroids picked ("
       << 100.0 * clusterCenters->size() / frequentWords.size() << " %)"
       << ". Space overhead: "
       << 1.0 * spaceOverhead / totalNumberOfOccurences + 1 << "x" << endl
       << endl << flush;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::buildWordClusteringOld(
                              const vector<T>& vocabulary,
                              const vector<T>& clusterCenters,
                              vector<int>& frequencies,
                              FuzzySearchAlgorithm<T>& fsAlg,
                              int maxNofClustersPerWord,
                              bool clusterTheCentroids,
                              vector<vector<int> >* clusters,
                              vector<int>* unclusteredWords)
{
  cout << " [ Building clusters ] ... " << flush;
  // 1. initialize and check the needed arrays and variables
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: The vocabulary is empty - no words to cluster!" << endl;
    exit(1);
  }
  CS_ASSERT_EQ(_isClusterCenter.size(), vocabulary.size());
  uint32_t nonEmptyCl = 0;
  vector<int> closestClusters;
  vector<double> distances;
  Timer timer;
  unsigned int nofClusteredWords = 0;  // clusterCenters.size();
  timer.start();
  T queryStr;
  (*clusters).clear();
  (*clusters).resize(clusterCenters.size());
  ProgressIndicator progressIndicator(vocabulary.size(), 10);
  unsigned int cluster = 0;
  unsigned int nofClusters;
  unsigned int totalNofClusters = 0;  // vocabulary.size();
  size_t spaceOverhead = 0;
  size_t freqSum = 0;
  uint32_t freqSumPartial = 0;
  uint32_t lastIndex = 0;
  // 2. compute frequency groups if frequency is present
  // frequency group tells in how many cluster should a word
  // belong, depending on its frequency
  vector<int> freqGroups;
  if (frequencies.size() == vocabulary.size())
  {
    for (unsigned int i = 0; i < vocabulary.size(); i++)
    {
      freqSum += frequencies[i];
      if (frequencies[i] <= _minimumFrequency)
      {
        freqSumPartial += frequencies[i];
        lastIndex = i;
      }
    }
    uint32_t freqSumPart = freqSumPartial / 5;  // take five groups
    uint32_t currentSum = 0;                    // (should be enough)
    int group = 0;
    freqGroups.resize(vocabulary.size());
    for (unsigned int i = 0; i <= lastIndex; i++)  // compute the freq. groups
    {
      currentSum += frequencies[i];
      freqGroups[i] = group;
      if (currentSum >= freqSumPart)
      {
        currentSum = 0;
        group++;
      }
    }
    for (unsigned int i = lastIndex + 1; i < vocabulary.size(); i++)
      freqGroups[i] = -1;
  }

  // 3. do the actual cluster building
  for (unsigned int i = 0; i < vocabulary.size(); i++)  // add the cl. center as
  {                                                     // the first word in the
    if (_isClusterCenter[i])                            // cluster
    {
      if (cluster == clusterCenters.size())
        break;
      CS_ASSERT(vocabulary[i] == clusterCenters[cluster]);
      (*clusters)[cluster++].push_back(i);
      // cluster++;
    }
  }
  // 3.a the main loop for the clustering (goes for all words in the vocab.)
  for (uint32_t i = 0; i < vocabulary.size(); i++)  // actual clustering
  {
    // if a word is not picked then it won't belong to any cluster anyway
    if (_pickedWords.size() == vocabulary.size())
      if (!_pickedWords[i])
      {
        spaceOverhead += frequencies[i];
        continue;
      }
    nofClusters = 0;
    if (!_isClusterCenter[i] || clusterTheCentroids)
    {
      // find the closest cluster centroids of a word
      fsAlg.findClosestWords(vocabulary[i],
                             vocabulary,
                             clusterCenters,
                             NULL,
                             &closestClusters,
                             &distances);
      if (!_isClusterCenter[i] && closestClusters.size() > 0)
        nofClusteredWords++;
      // limit the number of cluster for this word (if frequency data is
      // present)
      if (frequencies.size() == vocabulary.size())
      {
        switch (freqGroups[i])
        {
        /*
          case 0  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(4,
                          static_cast<int>(closestClusters.size()));
                    else
                      nofClusters = MY_MIN(5,
                          static_cast<int>(closestClusters.size()));
                    break;
          case 1  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(4,
                          static_cast<int>(closestClusters.size()));
                    else
                      nofClusters = MY_MIN(5,
                          static_cast<int>(closestClusters.size()));
                    break;
          case 2  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(3,
                          static_cast<int>(closestClusters.size()));
                    else
                      nofClusters = MY_MIN(4,
                          static_cast<int>(closestClusters.size()));
                    break;
          case 3  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(3,
                          static_cast<int>(closestClusters.size()));
                    else
                      nofClusters = MY_MIN(4,
                          static_cast<int>(closestClusters.size()));
                    break;
          default : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(2,
                          static_cast<int>(closestClusters.size()));
                    else
                      nofClusters = MY_MIN(3,
                          static_cast<int>(closestClusters.size()));
                    break;
              */

          case 0  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(10,
                          (static_cast<int>(closestClusters.size())));
                    else
                      nofClusters = MY_MIN(10,
                          (static_cast<int>(closestClusters.size())));
                    break;
          case 1  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(10,
                          (static_cast<int>(closestClusters.size())));
                    else
                      nofClusters = MY_MIN(10,
                          (static_cast<int>(closestClusters.size())));
                    break;
          case 2  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(8,
                          (static_cast<int>(closestClusters.size())));
                    else
                      nofClusters = MY_MIN(9,
                          (static_cast<int>(closestClusters.size())));
                    break;
          case 3  : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(7,
                          (static_cast<int>(closestClusters.size())));
                    else
                      nofClusters = MY_MIN(8,
                          (static_cast<int>(closestClusters.size())));
                    break;
          default : if (vocabulary[i].length() >= 9)
                      nofClusters = MY_MIN(7,
                          (static_cast<int>(closestClusters.size())));
                    else
                      nofClusters = MY_MIN(8,
                          (static_cast<int>(closestClusters.size())));
                    break;
        }
      }
      else
        nofClusters = MY_MIN(maxNofClustersPerWord,
            (static_cast<int>(closestClusters.size())));

      nofClusters = MY_MIN(maxNofClustersPerWord,
          (static_cast<int>(closestClusters.size())));
      // if (frequencies[i] > 50)
      //   nofClusters = 1;

      if (unclusteredWords != NULL)
        if (nofClusters == 0 && !_isClusterCenter[i])
        {
          unclusteredWords->push_back(i);
        }

      totalNofClusters += closestClusters.size();
      // 3.b put the current word into the (selected) clusters
      for (uint32_t j = 0; j < nofClusters; j++)
      {
        if ((*clusters)[closestClusters[j]].size() > 0)
        {
          /*
          string tmpstr;
          toString(clusterCenters[closestClusters[j]], &tmpstr);
          if (tmpstr == "sound")
          {
            printStr(vocabulary[i]);
            cout << endl;
          }
          */
          if ((*clusters)[closestClusters[j]][0] != static_cast<int>(i))
          {
            (*clusters)[closestClusters[j]].push_back(i);
            spaceOverhead += frequencies[i];
          }
        }
        else
        {
          (*clusters)[closestClusters[j]].push_back(i);
          spaceOverhead += frequencies[i];
        }
      }
    }
    // if (frequencies.size() == vocabulary.size())
    // {
    //   if (nofClusters == 0)
    //     spaceOverhead += frequencies[i];
    //   else
    //     spaceOverhead += ((nofClusters /*+ (_isClusterCenter[i] ? 1 : 0)*/)
    //         * frequencies[i]);
    // }
    progressIndicator.update(i);
  }
  timer.stop();
  double avgClusterSize = 0;
  for (uint32_t i = 0; i < (*clusters).size(); i++)
  {
    avgClusterSize += (*clusters)[i].size();
  }
  avgClusterSize /= (*clusters).size();

  for (uint32_t i = 0; i < (*clusters).size(); i++)
    if ((*clusters)[i].size() > 1)
      nonEmptyCl++;
  cout << endl << " [ done. (" << timer.secs() << " secs) ]" << endl;
  cout << endl;
  cout << "  " << nonEmptyCl << " non-empty clusters" << endl;
  cout << "  " << clusterCenters.size() << " cluster centroids" << endl;
  cout << "  " << nofClusteredWords << " words clustered" << endl;
  cout << "  " << static_cast<int>(vocabulary.size()) - nofClusteredWords
       <<" words unclustered" << endl;
  cout << "  " << 1.0 * totalNofClusters / vocabulary.size()
       << " clusters/word" << endl;
  cout << "  " << avgClusterSize << " is the avg. cluster size" << endl;
  cout << "  " << 1.0 * totalNofClusters / nofClusteredWords
       << " avg. clusters per clustered word" << endl;
  if (frequencies.size() == vocabulary.size())
  {
    cout << "  " << 1.0 * spaceOverhead / totalNumberOfOccurences
         << "x is the estimated "
         << "space overhead" << endl;
  }
  cout << endl;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::buildWordClustering(
                              const vector<T>& vocabulary,
                              const vector<T>& clusterCenters,
                              vector<int>& frequencies,
                              FuzzySearchAlgorithm<T>& fsAlg,
                              int maxNofClustersPerWord,
                              bool clusterTheCentroids,
                              vector<vector<int> >* clusters,
                              vector<int>* unclusteredWords)
{
  cout << " [ Building clusters ] ... " << flush;
  // 1. initialize and check the needed arrays and variables
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: The vocabulary is empty - no words to cluster!" << endl;
    exit(1);
  }
  CS_ASSERT_EQ(_isClusterCenter.size(), vocabulary.size());
  uint32_t nonEmptyCl = 0;
  vector<int> closestWords;
  vector<double> distances;
  vector<int16_t> clusterCounts;
  clusterCounts.resize(vocabulary.size());
  Timer timer;
  unsigned int nofClusteredWords = 0;
  timer.start();
  (*clusters).clear();
  ProgressIndicator progressIndicator(clusterCenters.size(), 10);
  unsigned int nofSimilarWords;
  unsigned int totalNofClusters = 0;  // vocabulary.size();
  size_t spaceOverhead = 0;

  // 2.a the main loop for the clustering (goes for all words in the vocab.)
  nofClusteredWords = clusterCenters.size();
  for (size_t i = 0; i < clusterCenters.size(); i++)  // actual clustering
  {
    // find the closest cluster words of a cluster centroid
    fsAlg.findClosestWords(clusterCenters[i],
                           vocabulary,
                           vocabulary,
                           NULL,
                           &closestWords,
                           &distances);
    nofSimilarWords = closestWords.size();

    // 2.b put the current word into the (selected) clusters
    vector<int> tempCluster;
    for (uint32_t j = 0; j < nofSimilarWords; j++)
    {
      if (!_isClusterCenter[closestWords[j]] || clusterTheCentroids)
      {
        if (clusterCounts[closestWords[j]] < maxNofClustersPerWord)
          tempCluster.push_back(closestWords[j]);
      }
    }
    if (tempCluster.size() > 1)
    {
      // make tempClusters.size() / clusterSize clusters instead of one
      size_t c1 = 0;
      bool repeat = true;
      int clusterSize = INT_MAX;
      while (repeat)
      {
        vector<int> tempCluster1;
        for (int i = 0; i < clusterSize; i++)
        {
          size_t index = i + c1;
          if (index >= tempCluster.size())
          {
            repeat = false;
            break;
          }
          tempCluster1.push_back(tempCluster[index]);
        }
        if (tempCluster1.size() > 0)
          clusters->push_back(tempCluster1);
        c1 += clusterSize;
      }
      // clusters->push_back(tempCluster);
      totalNofClusters++;
      for (size_t j = 0; j < tempCluster.size(); j++)
      {
        clusterCounts[tempCluster[j]]++;
        spaceOverhead += frequencies[tempCluster[j]];
      }
    }
    // progressIndicator.update(i);
  }
  for (size_t i = 0; i < vocabulary.size(); i++)
    if (clusterCounts[i] == 0 && unclusteredWords != NULL)
      unclusteredWords->push_back(i);
    else
      nofClusteredWords++;
  timer.stop();

  // 3. Display some statistics about the word clustering
  double avgClusterSize = 0;
  for (uint32_t i = 0; i < (*clusters).size(); i++)
    avgClusterSize += (*clusters)[i].size();
  avgClusterSize /= (*clusters).size();
  for (uint32_t i = 0; i < (*clusters).size(); i++)
    if ((*clusters)[i].size() > 1)
      nonEmptyCl++;
  cout << endl << " [ done. (" << timer.secs() << " secs) ]" << endl;
  cout << endl;
  cout << "  " << nonEmptyCl << " non-empty clusters" << endl;
  cout << "  " << clusterCenters.size() << " cluster centroids" << endl;
  cout << "  " << nofClusteredWords << " words clustered" << endl;
  if (unclusteredWords != NULL)
    cout << "  " << unclusteredWords->size()
         <<" words unclustered" << endl;
  cout << "  " << 1.0 * totalNofClusters / vocabulary.size()
       << " clusters/word" << endl;
  cout << "  " << avgClusterSize << " is the avg. cluster size" << endl;
  cout << "  " << 1.0 * totalNofClusters / nofClusteredWords
       << " avg. clusters per clustered word" << endl << flush;
  if (frequencies.size() == vocabulary.size())
  {
    cout << "  " << 1.0 * spaceOverhead / totalNumberOfOccurences
         << "x is the estimated "
         << "space overhead" << endl << flush;
  }
  cout << endl;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::buildCompletionClustering(
                 const vector<T>& vocabulary,
                 const vector<T>& clusterCenters,
                 vector<int>& frequencies,
                 FuzzySearchAlgorithm<T>& fsAlg,
                 int prefixLength,
                 int maxNofClustersPerWord,
                 bool clusterTheCentroids,
                 double maxDistance,
                 int minFrequency,
                 vector<vector<int> >* clusters,
                 vector<int>* unclusteredWords,
                 vector<int>* unclusteredFreq)
{
  cout << " [ Building completion clusters / prefix length "
       << prefixLength << " ... " << flush;

  // 1. initialize and check the needed arrays and variables
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: The vocabulary is empty - no words to cluster!" << endl;
    exit(1);
  }
  CS_ASSERT_GT(clusterCenters.size(), 0);
  CS_ASSERT_EQ(_isClusterCenter.size(), vocabulary.size());
  CS_ASSERT_EQ(vocabulary.size(), frequencies.size());
  size_t nonEmptyCl = 0;
  size_t spaceOverhead = 0;
  vector<int> closestClusters;
  vector<double> distances;
  unsigned int nofClusteredWords = 0;
  unsigned int totalNofClusters = 0;
  double avgFreq = 0;
  for (size_t i = 0; i < vocabulary.size(); i++)
    avgFreq += frequencies[i];
  avgFreq /= frequencies.size();
  vector<int16_t> counts;
  counts.resize(vocabulary.size());
  ProgressIndicator progressIndicator(clusterCenters.size(), 10);

  // 3. do the actual cluster building
  // 3.a the main loop for the clustering (goes for all words in the vocab.)
  Timer timer;
  timer.start();
  for (size_t i = 0; i < clusterCenters.size(); i++)  // actual clustering
  {
    if (true)
    {
      // find the closest cluster centroids of a prefix
      T prefix = clusterCenters[i].substr(0, MY_MIN(prefixLength,
          static_cast<int>(clusterCenters[i].length())));
      fsAlg.findClosestWords(prefix,
                             vocabulary,
                             vocabulary,
                             NULL,
                             &closestClusters,
                             &distances);
      totalNofClusters += closestClusters.size();

      // 3.b put the current word into the (selected) clusters
      vector<int> tempCluster;
      for (size_t j = 0; j < closestClusters.size(); j++)
      {
        if ((!_isClusterCenter[closestClusters[j]]
              /*|| frequencies[closestClusters[j]] <= minFrequency*/) &&
               counts[closestClusters[j]] < maxNofClustersPerWord
               && distances[j] <= maxDistance)
        {
          tempCluster.push_back(closestClusters[j]);
        }
      }
      if (tempCluster.size() > 1)
      {
        clusters->push_back(tempCluster);
        for (size_t j = 0; j < tempCluster.size(); j++)
        {
          spaceOverhead += frequencies[tempCluster[j]];
          counts[tempCluster[j]]++;
        }
      }
    }
    progressIndicator.update(i);
  }
  if (unclusteredWords != NULL)
  {
    unclusteredWords->clear();
    unclusteredFreq->clear();
    for (size_t i = 0; i < vocabulary.size(); i++)
      if (counts[i] == 0)
      {
        unclusteredWords->push_back(i);
        unclusteredFreq->push_back(frequencies[i]);
      }
  }
  nofClusteredWords = vocabulary.size() - unclusteredWords->size();
  timer.stop();

  // 4. Display some staticstics about the clustering
  double avgClusterSize = 0;
  for (uint32_t i = 0; i < (*clusters).size(); i++)
  {
    avgClusterSize += (*clusters)[i].size();
  }
  avgClusterSize /= (*clusters).size();
  for (uint32_t i = 0; i < (*clusters).size(); i++)
    if ((*clusters)[i].size() > 1)
      nonEmptyCl++;
  cout << endl << " [ done. (" << timer.secs() << " secs) ]" << endl;
  cout << endl;
  cout << "  " << nonEmptyCl << " non-empty clusters" << endl;
  cout << "  " << clusterCenters.size() << " cluster centroids" << endl;
  cout << "  " << nofClusteredWords << " words clustered" << endl;
  cout << "  " << static_cast<int>(vocabulary.size()) - nofClusteredWords
       <<" words unclustered" << endl;
  cout << "  " << 1.0 * totalNofClusters / vocabulary.size()
       << " clusters/word" << endl;
  cout << "  " << avgClusterSize << " is the avg. cluster size" << endl;
  cout << "  " << 1.0 * totalNofClusters / nofClusteredWords
       << " avg. clusters per clustered word" << endl;
  if (frequencies.size() == vocabulary.size())
  {
    cout << "  " << 1.0 * spaceOverhead / totalNumberOfOccurences
         << "x is the estimated "
         << "space overhead." << endl;
  }
  cout << endl;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::buildCompletionClustering(
                 const vector<T>& origVocabulary,
                 const vector<T>& clusterCenters,
                 vector<int>& frequencies,
                 bool clusterFrequentWords,
                 int prefixLength,
                 int maxNofClustersPerWord,
                 int minFrequency,
                 double distance,
                 vector<vector<int> >* clusters,
                 vector<int>* unclusteredWords,
                 vector<int>* unclusteredFreq)
{
  vector<vector<T> > temp;
  temp.resize(1);
  temp[0] = clusterCenters;
  buildCompletionClustering(origVocabulary, temp, frequencies,
      clusterFrequentWords, prefixLength, maxNofClustersPerWord,
      minFrequency, distance, clusters, unclusteredWords,
      unclusteredFreq);
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::buildCompletionClustering(
                 const vector<T>& origVocabulary,
                 const vector<vector<T> >& clusterCenters,
                 vector<int>& frequencies,
                 bool clusterFrequentWords,
                 int prefixLength,
                 int maxNofClustersPerWord,
                 int minFrequency,
                 double distance,
                 vector<vector<int> >* clusters,
                 vector<int>* unclusteredWords,
                 vector<int>* unclusteredFreq)
{
  cout << " [ Building completion clusters / prefix length "
       << prefixLength << ", " << flush;
  vector<T> vocabulary;
  vector<int> mapping;
  FastSS<T> fsAlg(3, 0);
  fsAlg.setFixedThreshold(1);

  // 1. separate cluster centroids from non-cluster centroids in the vocabulary
  // and build a f.s. index on this vocabulary
  for (size_t j = 0; j < origVocabulary.size(); j++)
  {
    if ((clusterFrequentWords && _isClusterCenter[j])
        || (!clusterFrequentWords && !_isClusterCenter[j]))
    {
      vocabulary.push_back(origVocabulary[j]);
      mapping.push_back(j);
    }
  }
  size_t nofClusterCenters = 0;
  for (size_t i = 0; i < clusterCenters.size(); i++)
    for (size_t j = 0; j < clusterCenters[i].size(); j++)
      nofClusterCenters++;
  cout << "vocabulary size " << vocabulary.size()
       << " and " << nofClusterCenters
       << " centroids."<< endl;
  if (vocabulary.size() == 0)
  {
    cerr << "ERROR: The vocabulary is empty - no words to cluster!" << endl;
    exit(1);
  }
  fsAlg.buildIndex(vocabulary, false);

  // 2. initialize and check the needed arrays and variables
  CS_ASSERT_EQ(_isClusterCenter.size(), origVocabulary.size());
  uint32_t nonEmptyCl = 0;
  vector<double> distances;
  vector<uint16_t> counts;
  vector<uint16_t> countsOverall;
  vector<int> closestWords;
  vector<int> seenIds;
  vector<int> maxNofClustersArr;
  unsigned int nofClusteredWords = 0;  // clusterCenters.size();
  unsigned int nofClosestWords;
  unsigned int totalNofClusters = 0;
  unsigned int nofUnclustered = 0;
  size_t spaceOverhead = 0;
  size_t avgFreq = 0;
  for (size_t i = 0; i < origVocabulary.size(); i++)
    avgFreq += frequencies[i];
  avgFreq /= frequencies.size();
  counts.resize(origVocabulary.size());
  countsOverall.resize(origVocabulary.size());
  maxNofClustersArr.resize(vocabulary.size());
  size_t maxNofClustersOverall = maxNofClustersPerWord;
  size_t maxNofClusters = 0;
  maxNofClustersOverall = maxNofClustersPerWord;
  if (clusterCenters.size() > 1)
    maxNofClusters = 1;
  else
    maxNofClusters = maxNofClustersPerWord;
  maxNofClusters = 1;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
  {
    // double f = frequencies[i] / avgFreq;
    // if (f < avgFreq / 4)
    //   maxNofClustersArr[i] = 5;
    // else
    // if (f < avgFreq / 2)
    //   maxNofClustersArr[i] = 4;
    // else
    // if (f < avgFreq)
    //   maxNofClustersArr[i] = 3;
    // else
    //   if (f < 10 * avgFreq)
    //     maxNofClustersArr[i] = 2;
    //   else
    //     maxNofClustersArr[i] = 1;
    maxNofClustersArr[i] = maxNofClustersPerWord;
  }

  // 3. do the actual cluster building
  // 3.a the main loop for the clustering
  ProgressIndicator progressIndicator(nofClusterCenters, 10);
  Timer timer;
  timer.start();
  size_t counter = 0;
  for (size_t i = 0; i < clusterCenters.size(); i++)  // actual clustering
  {
    for (size_t k = 0; k < clusterCenters[i].size(); k++)  // actual clustering
    {
      nofClosestWords = 0;
      // find the closest cluster centroids of a prefix
      T prefix = clusterCenters[i][k].substr(0, MY_MIN(prefixLength,
          static_cast<int>(clusterCenters[i][k].length())));
      fsAlg.findClosestWords(prefix,
                             vocabulary,
                             vocabulary,
                             NULL,
                             &closestWords,
                             &distances);
      nofClosestWords = closestWords.size();

      // cout << "! "; printStr(prefix); cout << endl;
      // for (size_t k = 0; k < nofClosestWords; k++)
      // {
      //  printStr(vocabulary[closestWords[k]]); cout << endl;
      // }

      // 3.b put the current word into the (selected) clusters
      vector<int> tmpCluster;
      for (uint32_t j = 0; j < nofClosestWords; j++)
      {
        if (distances[j] <= distance
            && counts[mapping[closestWords[j]]] < maxNofClusters
            && countsOverall[mapping[closestWords[j]]] < maxNofClustersOverall
            /*&& frequencies[mapping[closestWords[j]]] <= minFrequency*/
           )
        {
          tmpCluster.push_back(mapping[closestWords[j]]);
        }
      }
      if (tmpCluster.size() > 1)
      {
        // cout << endl;
        // cout << "! "; printStr(prefix); cout << endl;
        clusters->push_back(tmpCluster);
        totalNofClusters++;
        for (size_t j = 0; j < tmpCluster.size(); j++)
        {
          spaceOverhead += frequencies[tmpCluster[j]];
          counts[tmpCluster[j]]++;
          // printStr(origVocabulary[tmpCluster[j]]); cout << endl;
          if (clusterCenters.size() > 1)
          {
            countsOverall[tmpCluster[j]]++;
            seenIds.push_back(tmpCluster[j]);
          }
        }
      }
      progressIndicator.update(++counter);
    }
    if (clusterCenters.size() > 1)
    {
      for (size_t j = 0; j < seenIds.size(); j++)
        counts[seenIds[j]] = 0;
      seenIds.clear();
    }
  }
  if (unclusteredWords != NULL && unclusteredFreq != NULL)
  {
    unclusteredWords->clear();
    unclusteredFreq->clear();
  }
  for (size_t i = 0; i < vocabulary.size(); i++)
    if (counts[mapping[i]] == 0 && countsOverall[mapping[i]] == 0)
    {
      nofUnclustered++;
      if (unclusteredWords != NULL && unclusteredFreq != NULL)
      {
        unclusteredWords->push_back(mapping[i]);
        unclusteredFreq->push_back(frequencies[mapping[i]]);
      }
    }
  timer.stop();

  // 4. Output some statistics about the clustering
  nofClusteredWords = vocabulary.size() - nofUnclustered;
  double avgClusterSize = 0;
  for (uint32_t i = 0; i < (*clusters).size(); i++)
  {
    avgClusterSize += (*clusters)[i].size();
  }
  avgClusterSize /= (*clusters).size();
  for (uint32_t i = 0; i < (*clusters).size(); i++)
    if ((*clusters)[i].size() > 1)
      nonEmptyCl++;
  cout << endl << " [ done. (" << timer.secs() << " secs) ]" << endl;
  cout << endl;
  cout << "  " << nonEmptyCl << " non-empty clusters" << endl;
  cout << "  " << nofClusterCenters << " cluster centroids" << endl;
  cout << "  " << nofClusteredWords << " words clustered" << endl;
  cout << "  " << static_cast<int>(vocabulary.size()) - nofClusteredWords
       <<" words unclustered" << endl;
  cout << "  " << 1.0 * totalNofClusters / vocabulary.size()
       << " clusters/word" << endl;
  cout << "  " << avgClusterSize << " is the avg. cluster size" << endl;
  cout << "  " << 1.0 * totalNofClusters / nofClusteredWords
       << " avg. clusters per clustered word" << endl;
  if (frequencies.size() == origVocabulary.size())
  {
    cout << "  " << 1.0 * spaceOverhead / totalNumberOfOccurences
         << "x is the estimated space overhead." << endl;
  }
  cout << "  " << clusters->size() - 1 << " is the last cluster-id" << endl;
  cout << endl;
}

// ____________________________________________________________________________
// This function is not being used. Consider deleting it.
template <class T>
void WordClusteringBuilder<T>::processUnclusteredCompl(
                        const vector<T>& vocabulary,
                        FuzzySearchAlgorithm<T>& fsAlg,
                        int prefixLen,
                        vector<T>* clusterCenters,
                        vector<vector<int> >* clustersRareWords)
{
  cout << "Picking cluster centroids (w.r.t. frequencies/compl/rest) ... "
       << endl << flush;
  if (vocabulary.size() == 0)
    return;
  if (_isClusterCenter.size() != vocabulary.size())
    _isClusterCenter.resize(vocabulary.size());
  int nextWord = 0;
  clusterCenters->clear();
  list<int> clusterCentersList;
  Timer timer;
  timer.start();
  vector<bool> picked;
  picked.resize(vocabulary.size());
  for (unsigned int i = 0; i < vocabulary.size(); i++)
    _isClusterCenter[i] = false;
  int currentPrefixLen = -1;
  T prevWord;
  // vector<CyclicPermutationSubstringWithIndex<T> > permutedLexiconCompl;
  // buildPermutedLexicon(vocabulary, 3, 3, 15, &permutedLexiconCompl);
  // FastSS<T> fsAlg(3, 0);
  fsAlg.buildIndex(vocabulary, false);
  vector<int> closestClusters;
  vector<double> distances;
  ProgressIndicator pi(vocabulary.size(), 10);
  cout << " [ " << flush;
  while (nextWord < static_cast<int>(vocabulary.size()))
  {
    if (!picked[nextWord])
    {
      currentPrefixLen = 0;
      if (clusterCentersList.size() > 0)
      {
        prevWord = vocabulary[clusterCentersList.front()];
        size_t len = MY_MIN(vocabulary[nextWord].length(), prevWord.length());
        for (size_t i = 0; i < len; i++)
        {
          if (vocabulary[nextWord][i] == prevWord[i])
            currentPrefixLen++;
        }
      }
      if (currentPrefixLen < prefixLen)
      {
        clusterCentersList.push_front(nextWord);
        picked[nextWord] = true;
        T prefix = vocabulary[nextWord].substr(0,
           MY_MIN(prefixLen, static_cast<int>(vocabulary[nextWord].length())));
        fsAlg.findClosestWords(prefix,
                               vocabulary,
                               vocabulary,
                               NULL,
                               &closestClusters,
                               &distances);
        vector<int> cluster;
        cluster.push_back(nextWord);
        for (size_t j = 0; j < closestClusters.size(); j++)
        {
          if (!picked[closestClusters[j]])
          {
            cluster.push_back(closestClusters[j]);
          }
          picked[closestClusters[j]] = true;
        }
        if (cluster.size() > 1)
          clustersRareWords->push_back(cluster);
      }
      _isClusterCenter[nextWord] = true;
    }
    nextWord++;
    pi.update(nextWord);
  }
  list<int>::iterator iter;
  for (iter = clusterCentersList.begin(); iter != clusterCentersList.end();
      iter++)
    clusterCenters->push_back(vocabulary[*iter]);
  timer.stop();
  cout << " ]" << endl;
  cout << "[ done. (" << timer.msecs() / 1000.0 << " secs.) "
       << clusterCenters -> size() << " centroids picked ("
       << 1.0 * clusterCenters->size() / vocabulary.size() * 100 << " %) ]"
       << endl << endl << flush;
}

// ____________________________________________________________________________
template <>
string WordClusteringBuilder<string>::normalizeWord(const string& word)
{
  string tmpStr;
  for (unsigned int i = 0; i < word.length(); i++)
  {
    bool change2 = false;
    bool change1 = false;
    if (i < word.length()-2)
    {
      if (word[i] == 's' && word[i+1] == 'c' && word[i+2] == 'h')
      {
        tmpStr += 's';
        change2 = true;
        i++;
        continue;
      }
    }
    if (i < word.length()-1)
    {
      if (word[i] == word[i+1])
      {
        tmpStr += word[i];
        change2 = true;
      }
      else
      if (word[i] == 'u' && word[i+1] == 'e')
      {
        tmpStr += 'u';
        change2 = true;
      }
      else
      if (word[i] == 'a' && word[i+1] == 'e')
      {
        tmpStr += 'a';
        change2 = true;
      }
      else
      if (word[i] == 's' && word[i+1] == 's')
      {
        tmpStr += 's';
        change2 = true;
      }
      else
      if (word[i] == 'o' && word[i+1] == 'e')
      {
        tmpStr += 'o';
        change2 = true;
      }
      else
      if (word[i] == 't' && word[i+1] == 'h')
      {
        tmpStr += 't';
        change2 = true;
      }
      else
      if (word[i] == 's' && word[i+1] == 'h')
      {
        tmpStr += 's';
        change2 = true;
      }
      else
      if (word[i] == 'c' && word[i+1] == 'h')
      {
        tmpStr += 'c';
        change2 = true;
      }
      else
      if (word[i] == 'c' && word[i+1] == 'k')
      {
        tmpStr += 'k';
        change2 = true;
      }
      else
      if (word[i] == 'p' && word[i+1] == 'h')
      {
        tmpStr += 'f';
        change2 = true;
      }
      if (change2)
      {
        i++;
        continue;
      }
    }
    if (!change2)
    {
      if (word[i] == 'y')
      {
        tmpStr += 'i';
        change1 = true;
      }
    }
    if (!change2 && !change1)
      tmpStr += word[i];
  }
  return tmpStr;
}

// ____________________________________________________________________________
template <>
wstring WordClusteringBuilder<wstring>::normalizeWord(const wstring& word)
{
  string wordAsStr;
  wstring2string(word, &wordAsStr);
  wordAsStr = WordClusteringBuilder<string>::normalizeWord(wordAsStr);
  wstring tmpwstr;
  string2wstring(wordAsStr, &tmpwstr);
  return tmpwstr;
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::normalizeVocabulary(
                           const vector<T>& vocabulary,
                           vector<T>* normalizedVocabulary)
{
  normalizedVocabulary->resize(vocabulary.size());
  for (unsigned int i = 0; i < vocabulary.size(); i++)
    (*normalizedVocabulary)[i] = normalizeWord(vocabulary[i]);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::writeClustersToFile(
                                      const vector<string>& clusteredWords,
                                      const vector<int>& clusterIds,
                                      const string& filename)
{
  FILE* f = fopen(filename.c_str(), "w");
  CS_ASSERT_EQ(clusteredWords.size(), clusterIds.size());
  for (size_t i = 0; i < clusteredWords.size(); i++)
    fprintf(f, "%s\t%i\n", clusteredWords[i].c_str(), clusterIds[i]);
  fclose(f);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::writeClustersToFile(
                                      const vector<wstring>& clusteredWords,
                                      const vector<int>& clusterIds,
                                      const string& filename)
{
  vector<string> tmpVocabulary;
  tmpVocabulary.reserve(clusteredWords.size());
  string tmpstr;
  for (unsigned int i = 0; i < clusteredWords.size(); i++)
  {
    wstring2string(clusteredWords[i], &tmpstr);
    tmpVocabulary.push_back(tmpstr);
  }
  WordClusteringBuilder<string>::writeClustersToFile(tmpVocabulary,
      clusterIds, filename);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::writeClustersToFile(
                                         const vector<vector<int> >& clusters,
                                         const vector<string>& vocabulary,
                                         bool writeSingletons,
                                         const string& filename)
{
  fstream outputFile;
  // fstream outputFile1;
  outputFile.open(filename.c_str(), ios::out);
  // outputFile1.open((filename+".ids").c_str(), ios::out);
  Timer timer;
  timer.start();
  if (outputFile.fail())
  {
    cerr << "ERROR: file " << filename << ", couldn't be opened!"
         << " Aborting ..." << endl;
    exit(1);
  }
  int clusterId = 0;
  for (uint32_t i = 0; i < clusters.size(); i++)
  {
    if  (writeSingletons || clusters[i].size() > 1)
    {
      // outputFile1 << clusters[i].size() << endl;
      for (uint32_t j = 0; j < clusters[i].size(); j++)
      {
        outputFile << vocabulary[clusters[i][j]] << "\t" << clusterId
            << endl << flush;
        // outputFile1 << clusters[i][j] << endl;
      }
      clusterId++;
    }
  }
  outputFile.flush();
  outputFile.close();
  // outputFile1.flush();
  // outputFile1.close();
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::writeClustersToFile(
                                         const vector<vector<int> >& clusters,
                                         const vector<wstring>& vocabulary,
                                         bool writeSingletons,
                                         const string& filename)
{
  vector<string> tmpVocabulary;
  tmpVocabulary.reserve(vocabulary.size());
  string tmpstr;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
  {
    wstring2string(vocabulary[i], &tmpstr);
    tmpVocabulary.push_back(tmpstr);
  }
  WordClusteringBuilder<string>::writeClustersToFile(clusters, tmpVocabulary,
      writeSingletons, filename);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::appendClustersToFile(
                                          const vector<vector<int> >& clusters,
                                          const vector<string>& vocabulary,
                                          bool writeSingletons,
                                          int startingId,
                                          const string& filename)
{
  ofstream outputFile;
  outputFile.open(filename.c_str(), ios::app);
  Timer timer;
  timer.start();
  if (outputFile.fail())
  {
    cerr << "ERROR: file " << filename << ", couldn't be opened for appending!"
         << " Aborting ..." << endl;
    exit(1);
  }
  int clusterId = 0;
  for (uint32_t i = 0; i < clusters.size(); i++)
  {
    if  (writeSingletons || clusters[i].size() > 1)
    {
      for (uint32_t j = 0; j < clusters[i].size(); j++)
      {
        outputFile << vocabulary[clusters[i][j]] << "\t"
                   << (startingId + clusterId) << endl << flush;
      }
      clusterId++;
    }
  }
  outputFile.flush();
  outputFile.close();
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::appendClustersToFile(
                                          const vector<vector<int> >& clusters,
                                          const vector<wstring>& vocabulary,
                                          bool writeSingletons,
                                          int startingId,
                                          const string& filename)
{
  vector<string> tmpVocabulary;
  tmpVocabulary.reserve(vocabulary.size());
  string tmpstr;
  for (unsigned int i = 0; i < vocabulary.size(); i++)
  {
    wstring2string(vocabulary[i], &tmpstr);
    tmpVocabulary.push_back(tmpstr);
  }
  WordClusteringBuilder<string>::appendClustersToFile(clusters, tmpVocabulary,
      writeSingletons, startingId, filename);
}

// ____________________________________________________________________________
template <class T>
void WordClusteringBuilder<T>::getStopWords(
    std::unordered_map<string, bool, StringHashFunction>* stopWords)
{
  (*stopWords)["the"] = true;
  (*stopWords)["when"] = true;
  (*stopWords)["how"] = true;
  (*stopWords)["this"] = true;
  (*stopWords)["that"] = true;
  (*stopWords)["how"] = true;
  (*stopWords)["all"] = true;
  (*stopWords)["again"] = true;
  (*stopWords)["and"] = true;
  (*stopWords)["are"] = true;
  (*stopWords)["also"] = true;
  (*stopWords)["once"] = true;
  (*stopWords)["twice"] = true;
  (*stopWords)["back"] = true;
  (*stopWords)["both"] = true;
  (*stopWords)["will"] = true;
  (*stopWords)["but"] = true;
  (*stopWords)["because"] = true;
  (*stopWords)["here"] = true;
  (*stopWords)["have"] = true;
  (*stopWords)["how"] = true;
  (*stopWords)["may"] = true;
  (*stopWords)["now"] = true;
  (*stopWords)["since"] = true;
  (*stopWords)["then"] = true;
  (*stopWords)["too"] = true;
  (*stopWords)["well"] = true;
  (*stopWords)["what"] = true;
  (*stopWords)["with"] = true;
  (*stopWords)["next"] = true;
  (*stopWords)["they"] = true;
  (*stopWords)["was"] = true;
  (*stopWords)["which"] = true;
  (*stopWords)["however"] = true;
  (*stopWords)["not"] = true;
  (*stopWords)["did"] = true;
  (*stopWords)["who"] = true;
  (*stopWords)["their"] = true;
  (*stopWords)["very"] = true;
  (*stopWords)["become"] = true;
  (*stopWords)["who"] = true;
  (*stopWords)["from"] = true;
  (*stopWords)["become"] = true;
  (*stopWords)["always"] = true;
  (*stopWords)["never"] = true;
  (*stopWords)["back"] = true;
  (*stopWords)["behind"] = true;
  (*stopWords)["being"] = true;
  (*stopWords)["above"] = true;
  (*stopWords)["about"] = true;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::toString(const string& xstr, string* str)
{
  *str = xstr;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::toString(const wstring& xstr, string* str)
{
  string tmpstr;
  wstring2string(xstr, &tmpstr);
  *str = tmpstr;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::printStr(const string& str)
{
  cout << str;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::printStr(const wstring& wstr)
{
  string str;
  wstring2string(wstr, &str);
  cout << str;
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::writeVocabularyToFile(
                                    const vector<string>& vocabulary,
                                    const vector<int>& freq,
                                    const string& filename)
{
  CS_ASSERT_EQ(vocabulary.size(), freq.size());
  FILE *f = fopen(filename.c_str(), "w");
  for (size_t i = 0; i < vocabulary.size(); i++)
    fprintf(f, "%s\t%d\n", vocabulary[i].c_str(), freq[i]);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<wstring>::writeVocabularyToFile(
                                    const vector<wstring>& vocabulary,
                                    const vector<int>& freq,
                                    const string& filename)
{
  vector<string> vocaAsStr;
  vocaAsStr.resize(vocabulary.size());
  string wordAsStr;
  for (size_t i = 0; i < vocabulary.size(); i++)
  {
    wstring2string(vocabulary[i], &wordAsStr);
    vocaAsStr[i] = wordAsStr;
  }
  WordClusteringBuilder<string>::writeVocabularyToFile(vocaAsStr, freq,
      filename);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::readLine(fstream* f, string* line,
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
void WordClusteringBuilder<wstring>::readLine(fstream* f, wstring* line,
                                           bool toLower)
{
  string linestr;
  WordClusteringBuilder<string>::readLine(f, &linestr, toLower);
  string2wstring(linestr, line);
}

// ____________________________________________________________________________
template <>
void WordClusteringBuilder<string>::readLine(FILE* f, string* line,
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
void WordClusteringBuilder<wstring>::readLine(FILE* f, wstring* line,
                                           bool toLower)
{
  string linestr;
  WordClusteringBuilder<string>::readLine(f, &linestr, toLower);
  string2wstring(linestr, line);
}

// explicit instantiation
template class WordClusteringBuilder<std::string>;
template class WordClusteringBuilder<std::wstring>;
}
