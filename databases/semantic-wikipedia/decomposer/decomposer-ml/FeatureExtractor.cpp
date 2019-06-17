// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Elmar Haussmann <haussmae>

#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "./FeatureExtractor.h"

// #include "boost/date_time/posix_time/posix_time.hpp"
// include all types plus i/o
// using namespace boost::posix_time;


using std::map;
using std::pair;
using std::multimap;
using std::vector;

namespace ad_decompose
{
// ____________________________________________________________________________
FeatureExtractor::FeatureExtractor(FeatureMap * fMap,
    FeatureExtractorConfig const & fexConfig, bool seesAllBrTags,
    bool processWordCounts) :
  _fexConfig(fexConfig), _fMap(fMap), _seesAllBrTags(seesAllBrTags),
      _processWordCounts(processWordCounts)
{
  boost::split(_countCTypes, fexConfig.countCTypes, boost::is_any_of(" "));
  boost::split(_countPTags, fexConfig.countPTypes, boost::is_any_of(" "));
  boost::split(_countWForms, fexConfig.countWTypes, boost::is_any_of(" "));
  boost::split(_brTypes, fexConfig.brTypes, boost::is_any_of(" "));

  boost::split(_patternCTypes, fexConfig.patternCTypes, boost::is_any_of(" "));
  boost::split(_patternPTags, fexConfig.patternPTypes, boost::is_any_of(" "));
  boost::split(_patternWForms, fexConfig.patternWTypes, boost::is_any_of(" "));

  std::vector<string> words;
  // First split along ",". Then sequences of words are split along spaces.
  boost::split(words, fexConfig.words, boost::is_any_of(","));
  for (size_t i = 0; i < words.size(); ++i)
  {
    std::vector<string> wordparts;
    boost::split(wordparts, words[i], boost::is_any_of(" "));
    std::vector<string> rest(wordparts.begin() + 1, wordparts.end());
    _words.insert(std::make_pair<string, vector<string> >(wordparts[0], rest));
  }
}

// ____________________________________________________________________________
void FeatureExtractor::newSentence(Sentence<DefaultToken> const & sentence)
{
  _currentSentence = &sentence;
  _countMaps.clear();
  // We only need to do this if the FeatureExtractor supports word counts.
  if (_processWordCounts)
  {
    // Preprocess the word counts.
    vector<DefaultToken *> const & words = _currentSentence->getWords();

    map<string, int> initialMap;
    set<string>::const_iterator it;
    // Initialize all to 0
    for (it = _countCTypes.begin(); it != _countCTypes.end(); ++it)
      initialMap.insert(make_pair(*it, 0));
    for (it = _countPTags.begin(); it != _countPTags.end(); ++it)
      initialMap.insert(make_pair(*it, 0));
    for (it = _countWForms.begin(); it != _countWForms.end(); ++it)
      initialMap.insert(make_pair(*it, 0));

    map<string, int>::iterator mapIt;
    // Words and postags before pos.
    for (size_t i = 0; i < words.size(); ++i)
    {
      if (i != 0)
      {
        // Copy previous word's map.
        initialMap = _countMaps[i - 1];
      }
      // Search in the set.
      if ((it = _countWForms.find(words[i]->tokenString)) != _countWForms.end())
      {
        // Increase in the map.
        (initialMap.find(*it))->second++;
      }
      // Search in the set.
      if ((it = _countPTags.find(words[i]->posTag)) != _countPTags.end())
      {
        // Increase in the map.
        (initialMap.find(*it))->second++;
      }
      // The current word is the first in a new phrase and matches the
      // phrase type we are after.
      if (words[i]->_phrase->getWordsStartIndex() == i && ((it
          = _countCTypes.find(words[i]->_phrase->getType()))
          != _countCTypes.end()))
      {
        // Increase in the map
        (initialMap.find(*it))->second++;
      }
      _countMaps.push_back(initialMap);
    }
  }
}

// ____________________________________________________________________________
void FeatureExtractor::appendBrTag(size_t pos, const string & brTag)
{
  return;
}

// ____________________________________________________________________________
void FeatureExtractor::extractWordFeatures(FeatureVector * fv, size_t pos,
    string const & prefix) const
{
  vector<DefaultToken *> const & words = _currentSentence->getWords();
  assert(pos < words.size());
  int posI = static_cast<int> (pos);
  int sizeI = static_cast<int> (words.size());
  int start = (posI + _fexConfig.wwLeft < 0 ? 0 : (posI + _fexConfig.wwLeft));
  int end = (posI + _fexConfig.wwRight >= sizeI ? sizeI - 1 : posI
      + _fexConfig.wwRight);
  if (end < 0)
    return;
  pair<multimap<string, vector<string> >::const_iterator, multimap<string,
      vector<string> >::const_iterator> ret;
  multimap<string, vector<string> >::const_iterator it;
  for (size_t i = static_cast<size_t> (start); i <= static_cast<size_t> (end);
      ++i)
  {
    bool found = false;
    // The matched word sequence.
    string word = words[i]->tokenString;
    // The length of the matched sequence.
    size_t l = 1;
    ret = _words.equal_range(words[i]->tokenString);
    // Go trough all matches starting with words[i].
    for (it = ret.first; it != ret.second; ++it)
    {
      vector<string> const & rest = it->second;
      // If we have no remaining words to check we are done.
      if (rest.size() == 0)
        found = true;
      // Now search for the remaining words.
      // For j = 0 we are already at the second word in the sequence.
      for (size_t j = 0; j < rest.size() && j + i + 1 < words.size(); ++j)
      {
        if (rest[j] == words[i + j + 1]->tokenString)
        {
          found = true;
          // Only update if this is a large sequence than before.
          if (j + 2 > l)
          {
            word += "_" + rest[j];
            l = j + 2;
          }
        }
        // If we don't match one word we are done with that sequence.
        else
        {
          found = false;
          break;
        }
      }
    }
    if (found)
      addFeatureToVector(prefix + _fexConfig.pre_ww, i - pos, word, fv);
  }
}

// ____________________________________________________________________________
void FeatureExtractor::extractPosFeatures(FeatureVector * fv, size_t pos,
    string const & prefix) const
{
  vector<DefaultToken *> const & words = _currentSentence->getWords();
  assert(pos < words.size());
  int posI = static_cast<int> (pos);
  int sizeI = static_cast<int> (words.size());
  int start = (posI + _fexConfig.pwLeft < 0 ? 0 : (posI + _fexConfig.pwLeft));
  int end = (posI + _fexConfig.pwRight >= sizeI ? sizeI - 1 : posI
      + _fexConfig.pwRight);
  if (end < 0)
    return;
  for (size_t i = static_cast<size_t> (start); i <= static_cast<size_t> (end);
      ++i)
  {
    addFeatureToVector(prefix + _fexConfig.pre_pw, i - pos, words[i]->posTag,
        fv);
  }
}

// ____________________________________________________________________________
void FeatureExtractor::extractChunkFeatures(FeatureVector * fv, size_t pos,
    string const & prefix) const
{
  vector<Phrase<DefaultToken> *> const & phrases =
      _currentSentence->getPhrases();
  vector<DefaultToken *> const & words = _currentSentence->getWords();
  assert(pos < words.size());
  int sizeI = static_cast<int> (phrases.size());
  int phrasePos =
      static_cast<int> (words[pos]->_phrase->getPhraseIndexPosition());
  int start = (phrasePos + _fexConfig.cwLeft < 0 ? 0 : (phrasePos
      + _fexConfig.cwLeft));
  int end = (phrasePos + _fexConfig.cwRight >= sizeI ? sizeI - 1 : phrasePos
      + _fexConfig.cwRight);
  if (end < 0)
    return;
  for (size_t i = static_cast<size_t> (start); i <= static_cast<size_t> (end);
      ++i)
  {
    addFeatureToVector(prefix + _fexConfig.pre_cw, i - phrasePos,
        phrases[i]->getType(), fv);
  }
}

// ____________________________________________________________________________
void FeatureExtractor::extractBrFeatures(FeatureVector * fv, size_t pos,
    string const & prefix) const
{
  vector<DefaultToken *> const & words = _currentSentence->getWords();
  int posI = static_cast<int> (pos);
  int sizeI = static_cast<int> (words.size());
  int start = (posI + _fexConfig.bwLeft < 0 ? 0 : (posI + _fexConfig.bwLeft));
  int end = (posI + _fexConfig.bwRight >= sizeI ? sizeI - 1 : pos
      + _fexConfig.bwRight);
  if (end < 0)
    return;
  for (size_t i = static_cast<size_t> (start); i <= static_cast<size_t> (end);
      ++i)
  {
    if (words[i]->brTag == "*")
      continue;
    else
    {
      // Split along comma.
      std::vector<string> brTags;
      boost::split(brTags, words[i]->brTag, boost::is_any_of(","));
      for (size_t j = 0; j < brTags.size(); ++j)
      {
        // Only add it if it is part of the whitelist in the configuration.
        // Or if we see all tags.
        if (_seesAllBrTags || _brTypes.find(brTags[j]) != _brTypes.end())
          addFeatureToVector(prefix + _fexConfig.pre_bw, i - pos, brTags[j],
              fv);
      }
    }
    // std::cout << " Adding " << words[i].brTag << std::endl;
  }
}

// ____________________________________________________________________________
void FeatureExtractor::extractDynBrFeatures(FeatureVector * fv, size_t pos,
    string const & prefix) const
{
  vector<DefaultToken *> const & words = _currentSentence->getWords();
  int start = (pos + _fexConfig.dynbrLeft < 0 ? 0
      : (pos + _fexConfig.dynbrLeft));
  int end = (pos + _fexConfig.dynbrRight >= words.size() ? words.size() - 1
      : pos + _fexConfig.dynbrRight);
  if (end < 0)
    return;
  for (size_t i = static_cast<size_t> (start); i <= static_cast<size_t> (end);
      ++i)
  {
    if (words[i]->brTag == "*")
      continue;
    else
    {
      // Split along comma.
      std::vector<string> brTags;
      boost::split(brTags, words[i]->brTag, boost::is_any_of(","));
      for (size_t j = 0; j < brTags.size(); ++j)
      {
        // Only add it if it is part of the whitelist in the configuration.
        if (brTags[j] == _fexConfig.dynbrType)
          addFeatureToVector(_fexConfig.pre_bw, i - pos, brTags[j], fv);
      }
    }
    // std::cout << " Adding " << words[i].brTag << std::endl;
  }
}

// ____________________________________________________________________________
void FeatureExtractor::extractCountFeatures(FeatureVector * fv, size_t pos,
    string const & prefix) const
{
  assert(pos <_countMaps.size());
  map<string, int>::const_iterator mapIt;
  map<string, int> const & lastMap = _countMaps.back();
  // If pos == 0 all occurances before are 0
  if (pos == 0)
  {
    for (mapIt = _countMaps[pos].begin(); mapIt != _countMaps[pos].end();
        ++mapIt)
    {
      addFeatureToVector(_fexConfig.pre_count + "before:", 0, mapIt->first,
          fv);
      addFeatureToVector(_fexConfig.pre_count + "after:", (lastMap.find(
          mapIt->first))->second, mapIt->first, fv);
    }
  }
  // Else the occurances before are retrieved from the element before.
  else
  {
    for (mapIt = _countMaps[pos - 1].begin(); mapIt
        != _countMaps[pos - 1].end(); ++mapIt)
    {
      addFeatureToVector(prefix + _fexConfig.pre_count + "before:",
          mapIt->second, mapIt->first, fv);
    }
    // But for the counts after the counts from the current position
    // must be substracted.
    for (mapIt = _countMaps[pos].begin(); mapIt != _countMaps[pos].end();
        ++mapIt)
    {
      addFeatureToVector(prefix + _fexConfig.pre_count + "after:",
          (lastMap.find(mapIt->first))->second - mapIt->second, mapIt->first,
          fv);
    }
  }
}

// ____________________________________________________________________________
FeatureVector FeatureExtractor::extractWordFeatures(size_t pos) const
{
  assert(_currentSentence != NULL);
  // ptime start(microsec_clock::local_time());

  FeatureVector fv;
  extractWordFeatures(&fv, pos);
  extractPosFeatures(&fv, pos);
  extractChunkFeatures(&fv, pos);
  if (_processWordCounts)
    extractCountFeatures(&fv, pos);
  // This will extract features on the set of whitelisted brTags
  // possibly classified in previous runs of other classifiers.
  extractBrFeatures(&fv, pos);
  // We only need to do this in training when we need to filter
  // on the existing tags. In classification mode the dynamic
  // features are a subset of the br features extracted previously.
  if (!_seesAllBrTags)
    extractDynBrFeatures(&fv, pos);

  // ptime end(microsec_clock::local_time());

  // time_duration extracting = end - start;
  // std::cout << "FexWord: " << extracting.total_microseconds() << " µs.\n";

  return fv;
}

// ____________________________________________________________________________
FeatureVector FeatureExtractor::extractClauseFeatures(size_t start, size_t end)
  const
{
  assert(_currentSentence != NULL);

  // ptime start_t(microsec_clock::local_time());

  FeatureVector fv;
  // std::cout << "Extracting start features.\n";
  extractWordFeatures(&fv, start, "S");
  extractPosFeatures(&fv, start, "S");
  extractChunkFeatures(&fv, start, "S");
  if (_processWordCounts)
    extractCountFeatures(&fv, start, "S");
  // extractBrFeatures(&fv, start, "S");
  // std::cout << "Extracting end features.\n";
  extractWordFeatures(&fv, end, "E");
  extractPosFeatures(&fv, end, "E");
  extractChunkFeatures(&fv, end, "E");
  if (_processWordCounts)
    extractCountFeatures(&fv, end, "E");
  // extractBrFeatures(&fv, end, "E");
  // std::cout << "Extracting pattern features.\n";
  extractPatternFeatures(&fv, start, end, "");
  // We only need to do this in training when we need to filter
  // on the present tags. In normal classification everything
  // visiable can be used.
  if (!_seesAllBrTags)
  {
    extractDynBrFeatures(&fv, start, "S");
    extractDynBrFeatures(&fv, end, "E");
  }

  // ptime end_t(microsec_clock::local_time());

  // time_duration extracting = end_t - start_t;
  // std::cout << "FexClause: " << extracting.total_microseconds() << " µs.\n";


  return fv;
}

// ____________________________________________________________________________
void FeatureExtractor::extractPatternFeatures(FeatureVector * fv, size_t start,
    size_t end, string const & prefix) const
{
  vector<DefaultToken *> const & words = _currentSentence->getWords();

  assert(end < words.size());
  assert(start >= 0);
  assert(start <= end);
  string pattern = "";
  set<string>::const_iterator it;

  Phrase<DefaultToken> const * previousPhrase = NULL;

  for (size_t i = start; i <= end; ++i)
  {
    // Append the word form.
    if ((it = _patternWForms.find(words[i]->tokenString))
        != _patternWForms.end())
    {
      pattern += (words[i]->tokenString + "|");
    }
    // Append the POS tag.
    if ((it = _patternPTags.find(words[i]->posTag)) != _patternPTags.end())
    {
      pattern += (words[i]->posTag + "|");
    }
    // If this is a new phrase append the phrase type.
    if (words[i]->_phrase != previousPhrase)
    {
      if ((it = _patternCTypes.find(words[i]->_phrase->getType()))
          != _patternCTypes.end())
      {
        pattern += (words[i]->_phrase->getType() + "|");
      }
    }
    previousPhrase = words[i]->_phrase;
  }
  if (pattern.size() > 0)
  {
    // std::cout << "Pattern: " << pattern << "\n";
    pattern.erase(pattern.size() - 1);
    addFeatureToVector(prefix + _fexConfig.pre_pattern, 0, pattern, fv);
  }
}

// ____________________________________________________________________________
void FeatureExtractor::addFeatureToVector(std::string const & prefix, int pos,
    std::string ident, FeatureVector * fv) const
{
  static std::stringstream s;
  static int id;
  s << prefix << pos << ":" << ident;
  id = _fMap->getFeatureId(s.str());
  if (id != -1)
  {
    fv->addFeature(id, 1.0);
  }
  s.str("");
}

// ____________________________________________________________________________
std::map<int, string> const FeatureMap::getReverseMap() const
{
  map<int, string> reverseMap;
  for (dense_fmap::const_iterator it = _fMap.begin(); it != _fMap.end(); ++it)
    reverseMap.insert(make_pair(it->second, it->first));
  return reverseMap;
}

// ____________________________________________________________________________
int FeatureMap::getFeatureId(std::string const & featureString)
{
  //  map<string, int>::const_iterator it = _featureMap.find(featureString);

  dense_fmap::const_iterator it = _fMap.find(featureString);

  if (it == _fMap.end())
  {
    // If the map isn't locked, add a new entry.
    if (!_lockFeatureMap)
    {
      _fMap[featureString] = _nextIndex;
      return _nextIndex++;
    }
    // We didn't add the feature.
    return -1;
  }
  else
  {
    return it->second;
  }
}

// ____________________________________________________________________________
void FeatureMap::writeFeatureMap()
{
  // If the map was locked - return because nothing new can be written.
  if (_lockFeatureMap)
    return;
  // Before outputting sort the map by feature index
  map<int, string> reverseMap = getReverseMap();
  std::ofstream fmFileS(_fmFileName.c_str(), std::ifstream::out);
  std::map<int, string>::const_iterator it = reverseMap.begin();
  while (it != reverseMap.end())
  {
    fmFileS << (*it).first << "\t" << (*it).second << std::endl;
    it++;
  }
  fmFileS.close();
}

// ____________________________________________________________________________
void FeatureMap::loadFeatureMap()
{
  std::ifstream fmFileS(_fmFileName.c_str(), std::ifstream::in);
  assert(fmFileS.good());
  string line;
  while (std::getline(fmFileS, line))
  {
    string::size_type tabPos = line.find('\t');
    if (tabPos == string::npos)
    {
      std::cerr << "Wrong format in feature map file " << _fmFileName
          << std::endl;
      exit(1);
    }
    string feature = line.substr(tabPos + 1, string::npos);
    int index = atoi(line.substr(0, tabPos).c_str());
    if (index >= _nextIndex)
      _nextIndex = index + 1;
    _fMap[feature]= index;
  }
  fmFileS.close();
}

// ____________________________________________________________________________
FeatureMap::FeatureMap(const std::string & fmFileName, bool lockFeatureMap)
  : _lockFeatureMap(lockFeatureMap), _fmFileName(fmFileName)
{
  _nextIndex = 1;
  // If the file exists use read from it.
  std::ifstream fmFile(_fmFileName.c_str(), std::ifstream::in);
  if (fmFile.good())
  {
    loadFeatureMap();
  }
  else if (lockFeatureMap)
  {
    std::cerr << "Error loading feature map " << fmFileName << std::endl;
    exit(1);
  }
  fmFile.close();
}
}
