// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Marjan Celikik <celikik>

#include <string.h>
#include <string>
#include "../fuzzysearch/Utils.h"

namespace FuzzySearch
{
// ____________________________________________________________________________
int utf8_tolower(char* p)
{
  // CASE: normal character
  if (*p > 0)
  {
    *p = tolower(*p);
    return 1;
  }

  // CASE: some more complex thingy
  wchar_t wc;
  int len = mbtowc(&wc, p, 4);
  if (len == 1)
  {
    *p = tolower(*p);
    return 1;
  }
  else if (len > 1)
  {
    wc = towlower(wc);
    int len2 = wctomb(p, wc);
    if (len2 > len)
    {
      if (strlen(p) > 50) p[50] = 0;
      // exit(1);
    }
    return len;
  }
  else
  {
    return 1;
  }
}

// ____________________________________________________________________________
bool string2wstring(const string& str, wstring* wstr)
{
  static wchar_t tmpwchar[MAX_WORD_LEN + 1];
  size_t c = mbstowcs(tmpwchar, str.c_str(), MAX_WORD_LEN);
  if (c > MAX_WORD_LEN)
  {
    wstring tmp(str.begin(), str.end());
    *wstr = tmp;
    return false;
  }
  else
  {
    *wstr = tmpwchar;
    return true;
  }
}

// ____________________________________________________________________________
bool wstring2string(const wstring& wstr, string* str)
{
  static char tmpchar[(MAX_WORD_LEN + 1) * 4];
  size_t c = wcstombs(tmpchar, wstr.c_str(), MAX_WORD_LEN);
  if (c > MAX_WORD_LEN)
  {
    string tmp(wstr.begin(), wstr.end());
    *str = tmp;
    return false;
  }
  else
  {
    *str = tmpchar;
    return true;
  }
}

// ____________________________________________________________________________
bool isStrictPrefix(const string& s1, const string& s2)
{
  if (s1.length() >= s2.length())
    return false;
  for (size_t i = 0; i < s1.length(); i++)
    if (s1[i] != s2[i])
      return false;
  return true;
}
}

