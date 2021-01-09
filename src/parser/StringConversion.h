// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Hannah Bast <bast>

#ifndef PARSER_STRINGCONVERSION_H__
#define PARSER_STRINGCONVERSION_H__

#include <wctype.h>
#include <string>

using std::string;
using std::wstring;

//! Class that provides functionality for various string conversion, for
//! example, converting a string to lower case wrt a certain encoding.
class StringConversion
{
 public:
  enum Encoding
  {
    ISO_8859_1 = 0,
    UTF_8      = 1
  };

  //! Set encoding.
  void setEncoding(Encoding encoding);

  //! Change all uppercase letters to lowercase letters in that word.
  void toLower(string* word);
  //! Convert UTF-8 string to lower case.
  //!
  //! Note that the string passed as argumens is modified.
  //! @todo(bast): This function currently does nothing. Ina should provide an
  //! implementation, and, very important, a unit test. The implementation
  //! should make use of the functions mbrtowc (to read the next multibyte
  //! sequence from an ordinary string into a wchar_t) and towlower (to
  //! convert a wchar_t to lower case).
  void utf8ToLower(string* word);
  void iso88591ToLower(string* word);

  //! http://www.c-plusplus.de/forum/viewtopic-var-t-is-249809.html
  //! Converts string to wstring.
  void wstring2string(const wstring& ws, string* s);
  //! Converts string to wstring.
  void string2wstring(const string& s, wstring* ws);

  // void char2wchar(const char ch, wchar_t* wch);
  // void wchar2char(wchar_t wch, char* ch);
 private:
  Encoding _encoding;
};

#endif  // PARSER_STRINGCONVERSION_H__
