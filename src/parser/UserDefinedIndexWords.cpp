// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include "./UserDefinedIndexWords.h"
#include <errno.h>
#include <iostream>
#include <vector>
#include <string>

using std::cerr;
using std::endl;

// _____________________________________________________________________________
bool isAsciiLetter(char ch)
{
  // TODO(foobar): Umlaute (ch < 0)???
  if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122)
      || ch < 0 || (ch >= 48 && ch <= 57))
    return true;
  return false;
}

// _____________________________________________________________________________
UserDefinedIndexWords::UserDefinedIndexWords()
{}

// _____________________________________________________________________________
// Returns 0 on success, else -1.
int UserDefinedIndexWords::getPosVecFromString(const string& word,
                                               string& base,
                                               PosVec& sepCountVector)
{
  assert(word.length());

  size_t pos = 0;
  size_t cnt  = 0;
  size_t i = 0;
  int retValue = 0;

  sepCountVector.first = 0;
  sepCountVector.second = 0;
  // Find base-string, i.e.: .2-Hallo!:: -> Hallo and determine the number of
  // non ASCII characters left and right the base string.
  while (!isAsciiLetter(word[i]) && i < word.size())
  {
    ++i;
    --sepCountVector.first;
  }

  // When found some base.
  if (i < word.size())
  {
    pos = i;
    while (isAsciiLetter(word[i]) && word[i] != '\0')
    {
      ++i;
      ++cnt;
    }
    sepCountVector.second = word.length() - cnt + sepCountVector.first - 1;
    base = word.substr(pos, cnt);
    // cout << "Read word: " << word << "; Base: " << base << "; first: "
    //      << sepCountVector.first << "; second: " << sepCountVector.second
    //      << endl;
  }
  else
  {
    std::cerr << "UserDefinedIndexWords.cpp:50: " << word
              << " can't be used as proper name." << std::endl;
    retValue = -1;
  }
  return retValue;
}

// _____________________________________________________________________________
int UserDefinedIndexWords::insert(string* word, int call)
{
  int val;
  string base;
  PosVec posVector;
  vector<PosVec> posVectors;

  _properNamesSet.insert(word->c_str());
  // Parse read string.
  val = getPosVecFromString(*word, base, posVector);
  if (val == 0)
  {
    // Look up for the given base string.
    if (_properNamesMap.find(base.c_str()) == _properNamesMap.end())
    {
      posVectors.resize(0);
      posVectors.push_back(posVector);
      _properNamesMap[base.c_str()] = posVectors;
    }
    else
    {
      posVectors = _properNamesMap[base.c_str()];
      posVectors.push_back(posVector);
      _properNamesMap[base.c_str()] = posVectors;
    }
  }

  // If this is the first call to this function also insert toLower(word).
  if (call == 0)
  {
    _stringConversion.toLower(word);
    return insert(word, 1);
  }
  return 0;
}

// _____________________________________________________________________________
void UserDefinedIndexWords::parseUserDefinedIndexWordsFile()
{
  FILE* properNamesFile = NULL;
  char buffer[256];
  string word;

  // Open file.
  if ((properNamesFile = fopen(_properNamesFileName.c_str(), "r")) == NULL)
  {
    snprintf(buffer, sizeof(buffer), "fopen failed: %s",
             _properNamesFileName.c_str());
    perror(buffer);
    exit(errno);
  }

  // Read file line by line.
  while (fgets(buffer, sizeof buffer, properNamesFile) != NULL)
  {
    word = string(buffer);
    // Erase new line character.
    word[word.size() - 1] = '\0';
    insert(&word, 0);
  }
}

// _____________________________________________________________________________
void UserDefinedIndexWords::init(const string& properNamesFileName)
{
  _properNamesFileName = properNamesFileName;
  parseUserDefinedIndexWordsFile();
}

// _____________________________________________________________________________
void UserDefinedIndexWords::clear()
{
  _properNamesMap.clear();
  _properNamesSet.clear();
  _properNamesFileName = "";
}

// _____________________________________________________________________________
// Do lookup in hash-map for base string of proper name.
bool UserDefinedIndexWords::isProperBase(const string& base,
                                         vector<PosVec>& posVectors)
{
  bool retValue = false;
  posVectors.resize(0);
  if (_properNamesMap.find(base.c_str()) != _properNamesMap.end())
  {
    retValue = true;
    posVectors = _properNamesMap[base.c_str()];
  }
  return retValue;
}

// _____________________________________________________________________________
// Do lookup in hash_set for complete proper name.
bool UserDefinedIndexWords::isProperName(const string& name)
{
  return
    ((_properNamesSet.find(name.c_str()) == _properNamesSet.end()) ? false
                                                                   : true);
}

// _____________________________________________________________________________
bool UserDefinedIndexWords::extendToUserDefinedWord(const string& text,
                                                          size_t* word_start,
                                                          size_t* word_end)
{
  string word = "";
  vector<UserDefinedIndexWords::PosVec> positions;
  vector<UserDefinedIndexWords::PosVec>::iterator it;
  UserDefinedIndexWords::PosVec position;

  word = text.substr(*word_start, *word_end - *word_start);

  if (!isProperBase(word, positions)) return false;
  for (it = positions.begin(); it < positions.end(); ++it)
  {
    if (static_cast<int>(*word_start) + static_cast<int>((*it).first) < 0 ||
        static_cast<int>(*word_end) + static_cast<int>((*it).second) >
        static_cast<int>(text.size())) continue;

    word = text.substr(*word_start + (*it).first,
        -1*(*it).first + (*word_end - *word_start) + (*it).second);
    if (isProperName(word))
    {
      *word_start += (*it).first;
      *word_end += (*it).second;
      return true;
    }
  }
  return false;
}
