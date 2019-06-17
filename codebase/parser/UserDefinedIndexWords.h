// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#ifndef PARSER_USERDEFINEDINDEXWORDS_H_
#define PARSER_USERDEFINEDINDEXWORDS_H_

#include <gtest/gtest_prod.h>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <utility>

#include "./SimpleTextParser.h"
#include "./StringConversion.h"

using std::pair;
using std::vector;
using std::unordered_map;
using std::unordered_set;

class StringHashFunction
{
 public:
  size_t operator()(const string& s) const
  {
    size_t HashValue = 0;
    for (size_t i = 0; i < s.length(); i++)
      HashValue = 31*HashValue + s[i];
    return HashValue;
  }
}; /* end of class StringHashFunction */

/** Class for finding user-defined index words.
 *  UserDefinedIndexWords may contain an arbitrary mix of word and non-word
 *  characters (e.g. C++ or Q&A or Gmbh&Co). 
 *  @todo(bast): The current algorithm does *not* work for special words that
 *  consist *entirely* of non-word characters, for example, 1&1.
 */
class UserDefinedIndexWords
{
  friend class UserDefinedIndexWordsTest;
 public:
  UserDefinedIndexWords();
  // A pair of integers.
  typedef pair<int, int> PosVec;
  /** Initialise this class with a file containing a list of
   *  user-defined words.
   *  The file shall have a user-defined word in each line.
   *  @param[in] properNamesFileName file name of file containing user-defined
   *  words.
   */
  void init(const string& properNamesFileName);
  // Reset this object to initial state.
  void clear();
  /** Given a text and the beginning and end of a token (the base),
   *  check if there is an extension of the base to the left and/or right that
   *  is a user-defined word. If so, modify wordStart and wordEnd accordingly.
   *  @param[in] text Text that probably contains user-defined words.
   *  @param[out] wordStart Probably extended word start position.
   *  @param[out] wordEnd Probably extended word end position.
   */
  FRIEND_TEST(UserDefinedIndexWordsTest, extendToUserDefinedWord);
  bool extendToUserDefinedWord(const string& text,
                               size_t* wordStart,
                               size_t* wordEnd);
 private:
  // Tests iff the given string base is a base string belonging to a proper
  // name.
  // Returns true, iff the given base is a base string belonging to a proper
  // name. Returns false else. The vector posVectors is used as a container
  // for the positioning vectors containing to the given base. So, the base
  // "C" of the words "C++" and ".C&A" will fill posVectors with (0,2),
  // (-1,2).
  FRIEND_TEST(UserDefinedIndexWordsTest, isProperBase);
  bool isProperBase(const string& base, vector<PosVec>& posVectors);
  // Tests iff the given string name is a proper name defined in the proper
  // names file.
  FRIEND_TEST(UserDefinedIndexWordsTest, isProperName);
  bool isProperName(const string& name);
  // Parse file that contains line by line the user defined words.
  void parseUserDefinedIndexWordsFile(void);
  // For a given word the function returns its base and one vector. For
  // example: For the word $%foo=== we would retrieve 'foo' as base and (-2,3)
  // as retVector.
  int getPosVecFromString(const string& word,
                          string& base,
                          PosVec& retVector);
  // Adds the given word to this class.
  int insert(string* word, int call);

  // Class variables.
  string _properNamesFileName;
  unordered_map<string, vector<PosVec>, StringHashFunction> _properNamesMap;
  unordered_set<string, StringHashFunction> _properNamesSet;
  StringConversion _stringConversion;
};

#endif  // PARSER_USERDEFINEDINDEXWORDS_H_
