// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Ina Baumgarten <baumgari>, Hannah Bast <bast>

#include "./StringConversion.h"
#include <wctype.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

// _____________________________________________________________________________
void StringConversion::setEncoding(Encoding encoding)
{
  _encoding = encoding;
  if (encoding == ISO_8859_1) setlocale(LC_ALL, "en_US.iso88591");
  if (encoding == UTF_8)      setlocale(LC_ALL, "en_US.utf8");
}


// _____________________________________________________________________________
void StringConversion::utf8ToLower(string* word)
{
// TODO(baumgari): Not working, since it doesn't regonizing how long the
// multibyte sequence is. Possible with mbrtowc. Moreover length of multibyte
// sequence has to be add to variable i.
//  for(size_t i = 0; i < word->size(); i++)
//  {
//    char tmp = (*word)[i];
//    if (static_cast<unsigned int>(tmp) > 127)
//    {
//      std::cout << static_cast<unsigned int>(tmp) << " " << std::endl;
//      wchar_t charlower;
//      char2wchar(tmp, &charlower);
//      charlower = towlower(charlower);
//      wchar2char(charlower, &tmp);
//      (*word)[i] = tmp;
//    }
//    (*word)[i] = tolower(tmp);
//  }
  std::wstring wordlower;
  string2wstring(*word, &wordlower);
  size_t wordlength = wordlower.length();
  for (size_t i = 0; i < wordlength; i++)
  {
    wordlower[i] = std::towlower(wordlower[i]);
  }
  wstring2string(wordlower, word);
}
// _____________________________________________________________________________
// void StringConversion::char2wchar(const char ch, wchar_t* wch)
// {
//  std::mbrtowc(wch, &ch, MB_CUR_MAX, NULL);
// }
//
// _____________________________________________________________________________
//  void StringConversion::wchar2char(const wchar_t wch, char* ch)
//  {
//  std::wctomb(ch, wch);
//  }
//

// _____________________________________________________________________________
void StringConversion::string2wstring(const string& s, wstring* ws)
{
  std::vector<wchar_t> wcs(s.length());
  std::mbstowcs(&wcs[0], s.c_str(), s.length());
  *ws = wstring(wcs.begin(), wcs.end());
}

// _____________________________________________________________________________
void StringConversion::wstring2string(const std::wstring& ws, string* s)
{
  std::vector<char> mbs(ws.length());
  std::wcstombs(&mbs[0], ws.c_str(), ws.length());
  *s = string(mbs.begin(), mbs.end());
}

// _____________________________________________________________________________
void StringConversion::iso88591ToLower(string* word)
{
  for (size_t i = 0; i < word->size(); i++)
    (*word)[i] = std::tolower((*word)[i]);
}


// _____________________________________________________________________________
void StringConversion::toLower(string* word)
{
  iso88591ToLower(word);
}
