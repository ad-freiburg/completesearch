// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_UTILS_STRINGUTILS_H_
#define SEMANTIC_WIKIPEDIA_UTILS_STRINGUTILS_H_

#include <stdint.h>
#include <string>

#include "./CharMaps.h"

using std::string;

namespace ad_utility
{
//! Utility functions for string. Can possibly be changed to
//! a templated version using std::basic_string<T> instead of
//! std::string. However, it is not required so far.

//! Safe startWith function. Returns true iff prefix is a
//! prefix of text. Using a larger pattern than text.size()
//! will return false. Case sensitive.
inline bool startsWith(const string& text, const char* prefix,
    size_t patternSize);

//! Safe startWith function. Returns true iff prefix is a
//! prefix of text. Using a larger pattern than text.size()
//! will return false. Case sensitive.
inline bool startsWith(const string& text, const string& prefix);

//! Safe startWith function. Returns true iff prefix is a
//! prefix of text. Using a larger pattern than text.size()
//! will return false. Case sensitive.
inline bool startsWith(const string& text, const char* prefix);

//! Removes all special chars from the given string.
inline string noSpecialChars(const string& orig);

//! Gets a normalized version of the string in a way which means
//! that no special chars are kept and everything is converted
//! to lowercase.
inline string getNormalizedLowercase(const string& orig);

//! Gets the lowercase version of the string.
inline string getLowercase(const string& orig);


// *****************************************************************************
// Definitions:
// *****************************************************************************

// _____________________________________________________________________________
bool startsWith(const string& text, const char* prefix, size_t prefixSize)
{
  if (prefixSize > text.size())
  {
    return false;
  }
  for (size_t i = 0; i != prefixSize; ++i)
  {
    if (text[i] != prefix[i])
    {
      return false;
    }
  }
  return true;
}
// _____________________________________________________________________________
bool startsWith(const string& text, const string& prefix)
{
  return startsWith(text, prefix.data(), prefix.size());
}
// _____________________________________________________________________________
string noSpecialChars(const string& orig)
{
  string retVal;
  for (size_t i = 0; i < orig.size(); ++i)
  {
    if (W_CHAR_MAP[static_cast<int> (orig[i])] == 'w') retVal += orig[i];
  }
  return retVal;
}
// _____________________________________________________________________________
bool startsWith(const string& text, const char* prefix)
{
  return startsWith(text, prefix, std::char_traits<char>::length(prefix));
}
// _____________________________________________________________________________
string getNormalizedLowercase(const string& orig)
{
  string retVal;
  for (size_t i = 0; i < orig.size(); ++i)
  {
    if (W_CHAR_MAP[static_cast<uint8_t> (orig[i])] == 'w')
    {
      retVal += tolower(orig[i]);
    }
  }
  return retVal.size() > 0 ? retVal : getLowercase(orig);
}
string getLowercase(const string& orig)
{
  string retVal;
  for (size_t i = 0; i < orig.size(); ++i)
  {
    retVal += tolower(orig[i]);
  }
  return retVal;
}
}
#endif  // SEMANTIC_WIKIPEDIA_UTILS_STRINGUTILS_H_
