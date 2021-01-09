#ifndef __SYNONYM_DICTIONARY_H__
#define __SYNONYM_DICTIONARY_H__ 

// A data structure for managing groups of synonyms. Supports the following
// operations:
//
// 1. Read synonym groups from a given text file. Give each group its unique id.
// Note that a word may belong to more than one group but that this happens rarely.
// If there are s synonym groups, the group ids are 0, ..., s-1.
//
// 2. For a given word, return the ids of the synonym groups to which it belongs
// (the empty set, if the word belongs to no synonym group).

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <google/dense_hash_map>
#include <string>
#include <assert.h>
#include <stdint.h>

using namespace std;
using google::dense_hash_map;  

typedef dense_hash_map<string, uint32_t> SynMap;
typedef dense_hash_map<string, vector<uint32_t> > MultiSynMap;

class SynonymDictionary
{
 public:
  // Default constructor.
  SynonymDictionary();

  // Returns a list of values that in each case correspond to a group of
  // synonyms.
  void getSynonymGroupIds(const string& word, vector<uint32_t>& result);
  // Strips off and returns the 31th/highest bit of the given groupId. A search
	// for a word belonging to a synonym-group *and* having the 31th bit in its
	// groupId set, shall not list the other members of that group.
	bool stripAsteriskBitFromGroupId(uint32_t* groupId);
  // Read synonym groups from given file. The file format is as follows.
  // 1. One synonym group per line.
  // 2. Words are separated by a comma.
  // 3. Any whitespace is ignored.
  // 4. Lines starting with # are treated as comment lines and are ignored.
  // 5. Words with a trailing '*' will have the 31. bit of its hash value set.
  void readFromFile(const string& fileName);

  // Return the number of synonym groups.
  int numSynonymGroups() { return _synMap.size(); }

 private:
  // Map each word from a synonym group to the id of its group. The first map is
  // only for the id of the first group. The second map is for any further ids.
  // Since most words belong to only one synonym group, the second map will have
  // much fewer elements then the first.
  // Implementation detail: Use the lowest bit of the int for the first map to
  // indicate whether that synonym belongs to exactly one synonym group or to
  // more than one synonym group.
  SynMap _synMap;
  MultiSynMap _synMap2;
  static const int LINE_LENGTH = 1000000;
};

#endif
