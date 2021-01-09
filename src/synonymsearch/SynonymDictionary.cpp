// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include <string.h>
#include <sstream>
#include <vector>
#include <string>
#include "./SynonymDictionary.h"

// _____________________________________________________________________________
// TODO(bast): handle cases like file not found etc. properly. As a minimum, the
// program should exit with an error message (it should never produce a
// segmentation fault). In the mid term, we should do proper exception handling.
SynonymDictionary::SynonymDictionary()
{
  _synMap.set_empty_key("");
  _synMap2.set_empty_key("");
}

// _____________________________________________________________________________
void SynonymDictionary::readFromFile(const string& fileName)
{
  FILE* file;
  file = fopen(fileName.c_str(), "r");
  if (file == NULL)
  {
    ostringstream os;
    os << "fopen synonym dictionary \"" << fileName << "\"";
    perror(os.str().c_str());
    exit(1);
  }
  char buffer[LINE_LENGTH];
  char* word;
  uint32_t hashValue = 2;

  // Read lines from file, till there are no lines to read anymore.
  while (fgets(buffer, LINE_LENGTH, file))
  {
    char *savep;
    // Iff the buffered line contains no '\n', word becomes NULL.
    word = strchr(buffer, '\n');
    // Iff the line had more chars than 10*10^6.
    assert(word != NULL);
    word = strtok_r(buffer, " ,\n", &savep);
    while (word != NULL)
    {
      // When word has a trailing '*' mark the first bit in hashValue.
      if (strchr(word, '*') == word)
      {
        hashValue = hashValue | 0x80000000;
        ++word;
      }
      // Iff word wasn't mapped yet: map word to an even hash value.
      if (_synMap.count(word) == 0)
      {
        _synMap[word] = hashValue;
      }
      // Iff word was already mapped at least once.
      else
      {
        // Iff word was mapped exactly one time till now, i.e. iff the words
        // hash value is an even number.
        if (_synMap[word] % 2 == 0)
        {
          _synMap2[word].push_back(_synMap[word]);
          _synMap[word]++;
          _synMap2[word].push_back(hashValue);
        }
        // Iff word was mapped more than one time till now.
        else
        {
          _synMap2[word].push_back(hashValue);
        }
      }
      word = strtok_r(NULL, " ,\n", &savep);
			// Reset asterisk bit.
    	hashValue = hashValue & 0x7FFFFFFF;
    }
    hashValue += 2;
  }

#ifndef NDEBUG
    // Output map to screen for debug.
    cout << "-------------------" << endl;
    cout << "Hash map to screen." << endl;
    cout << "-------------------" << endl;

  dense_hash_map<string, uint32_t>::iterator it;

  for (it = _synMap.begin(); it != _synMap.end(); it++)
  {
    cout << (*it).first << " is in group ";
    if (((*it).second % 2) != 0)
    {
      for (int i = 0; i < _synMap2[(*it).first].size(); i++)
      {
        cout << " " << _synMap2[(*it).first][i];
      }
    }
    else { cout << (*it).second; }
    cout << endl;
  }
#endif
}

// _____________________________________________________________________________
void SynonymDictionary::getSynonymGroupIds(const string& word,
                                           vector<uint32_t>& result)
{
	result.clear();
  // Iff word is mapped push back groups.
  if (_synMap.count(word) != 0)
  {
    // Iff word is mapped only one time.
    if (_synMap[word] % 2 == 0)
    {
      result.push_back(_synMap[word]);
    }
    // Iff word is mapped multiple times.
    else
    {
      // Copy the vector<int> in _synMap2 to result.
      result = _synMap2[word];
    }
  }
}

// _____________________________________________________________________________
bool SynonymDictionary::stripAsteriskBitFromGroupId(uint32_t* groupId)
{
  assert(groupId);
  bool result = *groupId & 0x80000000;
  *groupId = *groupId & 0x7FFFFFFF;
  return result;
}
