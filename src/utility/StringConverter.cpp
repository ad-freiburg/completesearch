// Copyright 2010, Weitkaemper Technology
//
// Author: Wolfgang Bartsch <weitkaemper>


#include "utility/StringConverter.h"
#include <boost/lexical_cast.hpp>
#include <cstdio>
#include <cstdlib>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "utility/WkSupport.h"
#include "utility/utf8.h"
#include "server/Exception.h"

using std::setw;
using std::dec;
using std::hex;
using std::endl;

// Define static variables.
const size_t StringConverter::MAX_LENGTH_OF_LINE;
const size_t StringConverter::MAX_UTF8_CODEPOINT;
const size_t StringConverter::MAX_ISO_CODEPOINT;
const size_t StringConverter::MAX_URL_ENCODE_CODEPOINT;
const char   StringConverter::FIELD_SEP;
const size_t StringConverter::NUMB_OF_FIELDS_UTF8;
const size_t StringConverter::NUMB_OF_FIELDS_ISO;

// _____________________________________________________________________________
StringConverter::StringConverter()
{
  _sCharacterDefinitionFile_Utf8        = "utf8.map";       // utf8 map file
  _sCharacterDefinitionFile_Iso8859_1   = "iso8859-1.map";  // iso map file
  _bAlreadyInitialized = false;
  _pathToCharacterDefinitionFiles = "";
}


// _____________________________________________________________________________
bool StringConverter::init(string pathToCharacterDefinitionFiles)
{
  // NEW 15Dec11 Ina: bAlreadyInitialized has been static. This lead to a
  // segmentation fault when using more of one instance, since there was a call
  // on the utf8 and iso maps, which have been empty. Therefore made it public.
  // Now it's possible to use more than one instance of the StringConverter.
  // This is necessary in dblp.parser, which uses XmlParserNew, that does
  // normalization (with two different instances).
  if (_bAlreadyInitialized == true) return true;

  // NEW 09Feb2012 Ina: The path to the maps is now more flexibel, since I added
  // the necessary option to CsvParser and ParserBase. For this reason there is
  // an optional parameter now in function init().
  // _pathToCharacterDefinitionFiles is by default empty so it can be used as
  // before:
  // First it's tried to open the maps in the given path (new), then in the
  // current directory and if that doesn't work too, it's tried to open it in
  // ../codebase/utility.
  // If nothing is working an error is thrown that
  // _pathToCharacterDefinitionFiles + _sCharacterDefinitionFile_* cannot be
  // found.
  _pathToCharacterDefinitionFiles = pathToCharacterDefinitionFiles;
  if (_pathToCharacterDefinitionFiles != "")
  {
    size_t posLastLetter = _pathToCharacterDefinitionFiles.size() - 1;
    // If trailing slash doesn't exist, append it.
    if (_pathToCharacterDefinitionFiles[posLastLetter] != '/')
      _pathToCharacterDefinitionFiles += "/";
  }

  _bAlreadyInitialized = true;
  _initUrlEncode();

  return (_readUtf8initFile() && _readIso8859_1_initFile());
}

size_t StringConverter::nextChar(const Encoding& enc,
                                 string::const_iterator& pos,
                                 const string::const_iterator& end)
{
  size_t letter;
  if (enc == ENCODING_UTF8)
  {
    letter = utf8::next(pos, end);
  }
  else if (enc == ENCODING_ISO8859_1)
  {
    letter = (unsigned char)*pos;
    ++pos;
  }
  return letter;
}

// _____________________________________________________________________________
string StringConverter::convert(const string& sToConvert,
                                Encoding      enc,
                                Conversion    con)
{
  string                 sRes;
  string::const_iterator sIter = sToConvert.begin();


  if (con <= CONVERSION_MIN || con >= CONVERSION_MAX
      || (enc != ENCODING_UTF8 && enc != ENCODING_ISO8859_1))
  {
    // Invalid conversion, just send back the original string
    return sToConvert;
  }

  // Initialize conversiontable and maxCodepoint according to the given
  // encoding.
  vector<vector<string> >& conversionTable =
     (enc == ENCODING_UTF8)
    ? _utf8CpToConversionChar
    : _iso8859_1_CpToConversionChar;

  vector<vector<size_t > >& conversionToCodepointTable =
     (enc == ENCODING_UTF8)
    ? _utf8ConversionTable
    : _iso8859_1_ConversionTable;

  size_t maxCodepoint =
     (enc == ENCODING_UTF8)
    ? MAX_UTF8_CODEPOINT
    : MAX_ISO_CODEPOINT;

  CS_ASSERT_LT(con, conversionTable.size());


  // NEW (baumgari) 13Jun13: Rewritten the conversion handling. Instead of first
  // differing between the encoding, we first differ between the conversions an
  // then between the encodings to minimize code replication.
  if (con != CONVERSION_FROM_MASK)
  {
    // Codepoints need up to 5 characters per letter.
    if (con == CONVERSION_TO_MASK)
      sRes.reserve(sToConvert.length() * 5);
    else
    {
      sRes.reserve(sToConvert.length() * 2);
    }
    size_t firstLetter;
    size_t secondLetter = maxCodepoint + 1;

    if (sIter != sToConvert.end())
    {
      secondLetter = nextChar(enc, sIter, sToConvert.end());
    }

    while (secondLetter != maxCodepoint + 1)
    {
      firstLetter = secondLetter;
      if (sIter != sToConvert.end())
        secondLetter = nextChar(enc, sIter, sToConvert.end());
      // In case we reached the end of the word, firstLetter contains the last letter.
      // Therefore second letter should contain something nonsense.
      else
      {
        secondLetter = maxCodepoint + 1;
      }

      // Hack to get ä-> ae, ö -> oe, u -> ue
      // (to be replaced by a more flexible and faster  way)
      if (con == CONVERSION_TO_NORM)
      {
        bool isUmlaut = true;
        // NEW (baumgari) 28Oct14:
        // Before: We did normalize ae and äe to ä, but did not normalize e.g.
        // áe and aé to ä. This lead to the problem, that e.g. "saéz" could not be
        // found by looking up "saez". Now the first letter and the second
        // letter are normalized first, such that every possible diacritic is
        // normalized the same way. ae, ä, äe, áé, etc. do all map to ä now.
        size_t firstLetterNorm = firstLetter;
        size_t secondLetterNorm = secondLetter;
        if (firstLetter <= maxCodepoint)
        {
          CS_ASSERT_LT(firstLetter, conversionToCodepointTable[con].size());
          CS_ASSERT_GE(firstLetter, 0);
          firstLetterNorm = conversionToCodepointTable[con][firstLetter];
        }
        if (secondLetter <= maxCodepoint)
        {
          CS_ASSERT_LT(secondLetter, conversionToCodepointTable[con].size());
          CS_ASSERT_GE(secondLetter, 0);
          secondLetterNorm = conversionToCodepointTable[con][secondLetter];
        }

        // NEW (baumgari) 10Jan14: We should not just convert ae to ä/a, but
        // also äe to ä/a. Example: staehler is normalized to stahler and
        // stäehler is normalized to staehler right now. Usually such names
        // belong to the same person and we want to find the person by
        // searching for any of stahler, staehler, stähler, stäehler.
        // NEW (baumgari) 06Mar13:
        // Hack to get ae/äe -> ä, oe/öe -> ö, ue/üe -> ü.
        // whereas a == 97, u == 117, o == 111, e == 101
        // ä -> a/ä, ö -> o/ö, ü -> u/ü is done automatically by the map.
        if (firstLetterNorm == 97 && secondLetterNorm == 101)
          firstLetter = 228;  // ae, äe, áé, etc. -> ä
        else if (firstLetterNorm == 111 && secondLetterNorm == 101)
          firstLetter = 246;  // oe, öé, etc. -> ö
        else if (firstLetterNorm  == 117 && secondLetterNorm == 101)
          firstLetter = 252;  // ue, üe, etc. -> ü
        else
        {
          isUmlaut = false;
        }
        // In case we parsed two letters at the same time (e.g ae) we need to
        // ensure to not parse the second letter two times.
        if (isUmlaut)
        {
          if (sIter < sToConvert.end())
            secondLetter = nextChar(enc, sIter, sToConvert.end());
          else
          {
            secondLetter = maxCodepoint + 1;
          }
        }
        if (firstLetter == 223)
        {
          sRes += "ss";
          continue;
        }
      }
      if (    con == CONVERSION_TO_NORM
           || con == CONVERSION_TO_LOWER
           || con == CONVERSION_TO_UPPER)
      {
        if (firstLetter <= maxCodepoint)
        {
          CS_ASSERT_LT(firstLetter, conversionTable[con].size());
          CS_ASSERT_GE(firstLetter, 0);
          sRes += conversionTable[con][firstLetter];
        }
        else
        {
          // What to do with unknown characters?
          sRes += " ";
        }
      }
      // NEW (baumgari) 13Jun13: Added a new conversion to mask letters and another
      // conversion to unmask letters. This is done by using the codepoints. Each
      // character is replaced by "%<codepoint>".
      else if (con == CONVERSION_TO_MASK)
      {
        if (firstLetter <= maxCodepoint)
        {
          sRes += string("%");
          ostringstream strm;
          strm << firstLetter;
          sRes += strm.str();
        }
        else
        {
          // What to do with unknown characters?
          sRes += " ";
        }
      }
    }
  }
  // NEW (baumgari) 13Jun13: Added a new conversion to mask letters and another
  // conversion to unmask letters. This is done by using the codepoints. Each
  // character is replaced by "%<codepoint>".
  else if (con == CONVERSION_FROM_MASK)
  {
    vector<string>& codepointToChar =
      (enc == ENCODING_UTF8)
      ? _codepointToUtf8
      : _codepointToIso8859_1;

    size_t letter;
    bool isNumber = false;
    bool isEndOfNumber = false;
    string number;
    number.reserve(4);
    if (sIter != sToConvert.end())
      letter = nextChar(enc, sIter, sToConvert.end());

    while (letter != maxCodepoint + 1)
    {
      if (letter <= maxCodepoint)
      {
        CS_ASSERT_LT(letter, codepointToChar.size());
        CS_ASSERT_GE(letter, 0);
        string& ch = codepointToChar[letter];
        // Is it the start of a number?
        if (ch == "%")
        {
          if (isNumber) isEndOfNumber = true;
          isNumber = true;
        }
        // Is it a number?
        else if (isNumber && (ch >= "0" && ch <= "9"))
          number += ch;
        // Is it a letter?
        else if (isNumber)
        {
          isNumber = false;
          isEndOfNumber = true;
        }
        // Is it the end of the word and the last letter is masked?
        if (sIter == sToConvert.end() && isNumber) isEndOfNumber = true;
        if (isEndOfNumber)
        {
          // Convert "string number" to a real number.
          size_t codepoint = boost::lexical_cast<size_t>(number);
          CS_ASSERT_LT(codepoint, codepointToChar.size());
          CS_ASSERT_GE(codepoint, 0);
          sRes += codepointToChar[codepoint];
          if (!isNumber)
          {
            sRes += ch;
            isNumber = false;
          }
          isEndOfNumber = false;
          number.clear();
        }
        else if (!isNumber)
        {
          sRes += ch;
        }
      }
      else
      {
        sRes += " ";
      }
      if (sIter != sToConvert.end())
        letter = nextChar(enc, sIter, sToConvert.end());
      else
        letter = maxCodepoint + 1;
    }
  }
  return sRes;
}


// _____________________________________________________________________________
string StringConverter::urlEncode(const string& sToConvert)
{
  string sRes;

  sRes.reserve(sToConvert.length());
  for (size_t n = 0; n < sToConvert.length(); n++)
  {
    size_t nVal = (unsigned char)sToConvert[n];

    if (nVal <= MAX_URL_ENCODE_CODEPOINT)
    {
      sRes += _codepointToUrlEncode[nVal];
    }
    else
    {
      sRes += sToConvert[n];
    }
  }
  return sRes;
}


// _____________________________________________________________________________
string StringConverter::convertDate(const string& sIn)
{
  // Converts the following formats:
  // dd.mm.yyyy
  // dd-mm-yyyy
  // yyyy-mm-dd
  // All with and without leading zeros in dd and mm,
  // but yyyy has always to have 4 digits.
  // into the format:
  // dd-mm-yyyy with leading zeros in dd and mm.
  //
  // Expls.:
  // 7.6.2010 -> 07-06-2010
  // 2010-9-6 -> 06-09-2010
  //
  // Returns empty string in case of error
  //
  string sInCopy = sIn;
  size_t nPos;

  while ((nPos = sInCopy.find(".")) != string::npos)
  {
    sInCopy[nPos] = '-';
  }

  vector<string> vsToken;
  string         sRet = "";

  putTokenInVector(sInCopy, '-', &vsToken);

  if (vsToken.size() != 3)
  {
    return sRet;
  }

  string sYear;
  string sMonth;
  string sDay;
  if (vsToken[0].size() == 4)
  {
    // yyyy-mm-dd
    sYear = vsToken[0];
    sMonth = vsToken[1];
    sDay = vsToken[2];
  }
  else
  {
    sYear = vsToken[2];
    sMonth = vsToken[1];
    sDay = vsToken[0];
  }
  if (sDay.length() == 0 || sMonth.length() == 0 ||
      sYear.length() != 4)
  {
    return sRet;
  }
  if (sDay.length() > 2 || sMonth.length() > 2) return sRet;

  if (sDay.length() == 1) sDay = "0" + sDay;
  if (sMonth.length() == 1) sMonth = "0" + sMonth;

  sRet = sDay + "-" + sMonth + "-" + sYear;
  return sRet;
}


// _____________________________________________________________________________
string StringConverter::replaceNonAlphanum(const string&         sIn,
                                           Encoding              enc,
                                           char                  cRepl,
                                           bool                  bKeepByteNumb,
                                           const vector<size_t>& vnSaveList)
{
  string                 sRes;
  string::const_iterator sIter = sIn.begin();

  sRes.reserve(sIn.length());

  if (enc == ENCODING_UTF8)
  {
    while (sIter != sIn.end())
    {
      size_t nCp;
      bool   bInSaveList = false;

      nCp = utf8::next(sIter, sIn.end());
      for (size_t n = 0; n < vnSaveList.size(); n++)
      {
        if (nCp == vnSaveList[n])
        {
          bInSaveList = true;
        }
      }
      string sUtf8Char = getUtf8Encoding(nCp);

      if (nCp <= MAX_UTF8_CODEPOINT && (bInSaveList == true || _isAlphanumCodepointUtf8[nCp] == true))
      {
        sRes += sUtf8Char;
      }
      else
      {
        if (bKeepByteNumb == true)
        {
          for (size_t n = 0; n < sUtf8Char.length(); n++)
          {
            sRes += cRepl;
          }
        }
        else { sRes += cRepl; }
      }
    }
  }

  if (enc == ENCODING_ISO8859_1)
  {
    sRes.resize(sIn.length());
    size_t m = 0;
    while (sIter != sIn.end())
    {
      size_t nCp = (unsigned char)*sIter;
      bool   bInSaveList = false;

      for (size_t n = 0; n < vnSaveList.size(); n++)
      {
        if (nCp == vnSaveList[n])
        {
          bInSaveList = true;
        }
      }
      if (nCp <= MAX_ISO_CODEPOINT && (bInSaveList == true || _isAlphanumCodepointIso8859_1[nCp] == true))
      {
        sRes[m++] =  static_cast<char>(nCp);
      }
      else
      {
        sRes[m++] = cRepl;
      }
      sIter++;
    }
  }
  return sRes;
}


// _____________________________________________________________________________
void StringConverter::printUtf8Mapping(size_t nStart,
                                       size_t nEnd,
                                       ostream& out,
                                       Conversion con)
{
  size_t nCodepoint;

  if (con >= CONVERSION_MAX || nStart > MAX_UTF8_CODEPOINT)
  {
    return;
  }
  if (nEnd > MAX_UTF8_CODEPOINT)
  {
    nEnd = MAX_UTF8_CODEPOINT;
  }
  for (size_t n = nStart; n <= nEnd; n++)
  {
    if (_utf8ConversionTable[con][n] > 0)
    {
      nCodepoint = _utf8ConversionTable[con][n];
      out << setw(4) << dec << n << "->"
          << setw(4) << dec << nCodepoint << "     "
          << setw(3) << hex << n << "->"
          << setw(3) << hex << nCodepoint << dec << "     "
          <<  _codepointToUtf8[n] << "   "
          <<   _codepointToUtf8[nCodepoint] << endl;
    }
  }
}


// _____________________________________________________________________________
void StringConverter::printIso8859_1_Mapping(size_t nStart,
                                             size_t nEnd,
                                             ostream& out,
                                             Conversion con)
{
  size_t nCodepoint;

  if (con >= CONVERSION_MAX || nStart > MAX_ISO_CODEPOINT)
  {
    return;
  }
  if (nEnd > MAX_ISO_CODEPOINT)
  {
    nEnd = MAX_ISO_CODEPOINT;
  }

  for (size_t n = nStart; n <= nEnd; n++)
  {
    if (_iso8859_1_ConversionTable[con][n] > 0)
    {
      nCodepoint = _iso8859_1_ConversionTable[con][n];
      out << setw(4) << dec << n << "->"
          << setw(4) << dec << nCodepoint << "     "
          << setw(3) << hex << n << "->"
          << setw(3) << hex << nCodepoint << dec << "     "
          <<  _codepointToIso8859_1[n] << "   "
          <<  _codepointToIso8859_1[nCodepoint] << endl;
    }
  }
}


// _____________________________________________________________________________
string StringConverter::getUtf8Encoding(size_t nCp)
{
  string           sTmp;
  string           sTmp1;
  string::iterator sIterEnd;

  sTmp.resize(5, '\0');
  sIterEnd = utf8::append(nCp, sTmp.begin());
  sTmp1.append(sTmp.begin(), sIterEnd);
  return sTmp1;
}


// _____________________________________________________________________________
bool StringConverter::utf8ToIso8859_1(const string& sUtf8, char cNonIso,
                                      string*       sIso8859_1)
{
  // NEW 15Dec11 Ina: Changed szLine from char* to string, since it lead to an
  // segmentation fault (invalid pointer when calling the destructor).
  // Since the trailing '0' is not needed anymore the size is decreased by one.
  // Moreover the deletion of the pointer is not necessary anymore.
  string                 szLine;
  size_t                 n = 0;
  bool                   bAllIsoCharacters = true;
  string::const_iterator sIter = sUtf8.begin();
  szLine.resize(sUtf8.length());

  while (sIter != sUtf8.end())
  {
    size_t cp;
    cp = utf8::next(sIter, sUtf8.end());
    if (cp > 255)
    {
      bAllIsoCharacters = false;
      szLine[n] = cNonIso;
    }
    else
    {
      szLine[n] = static_cast<char>(cp);
    }
    n++;
  }
  *sIso8859_1 = szLine;
  return bAllIsoCharacters;
}

// _____________________________________________________________________________
bool StringConverter::iso8859_1ToUtf8(const string& sIso8859_1, string* sUtf8)
{
  // NEW 15Dec11 Ina: Changed pszResult from char* to string, since it lead to
  // an segmentation fault (invalid pointer when calling the destructor).
  // Since the trailing '0' is not needed anymore the size is decreased by one.
  // Moreover the deletion of the pointer is not necessary anymore.
  string pszResult;
  pszResult.resize(sIso8859_1.length() * 2);

  size_t m = 0;
  for (size_t n = 0; n < sIso8859_1.length(); n++)
  {
    unsigned char c = sIso8859_1[n];
    if (c < 128)
    {
      pszResult[m++] = sIso8859_1[n];
    }
    else if (c < 192)
    {
      pszResult[m++] = static_cast<char>(194);
      pszResult[m++] = sIso8859_1[n];
    }
    else if (c <= 255)
    {
      pszResult[m++] = static_cast<char>(195);
      pszResult[m++] = sIso8859_1[n] - 64;
    }
  }
  *sUtf8 = pszResult;

  // Return value is always true because each ISO8859-1 character
  // has a UTF8 encoding.
  return true;
}


string StringConverter::getLastError()
{
  return _sLastError;
}







//
//
// PRIVATE METHODS
//
//


// _____________________________________________________________________________
bool StringConverter::_readUtf8initFile()
{
  // Initializes the vectors
  // _utf8ConversionTable
  // _utf8CpToConversionChar
  // _codepointToUtf8
  // _isAlphanumCodepointUtf8
  // by reading map file _sCharacterDefinitionFile_Utf8.

  _sLastError = "";

  _utf8ConversionTable.resize(CONVERSION_MAX);
  _utf8CpToConversionChar.resize(CONVERSION_MAX);


  for (size_t n = 0; n < CONVERSION_MAX; n++)
  {
    _utf8ConversionTable[n].resize(MAX_UTF8_CODEPOINT+1);
    _utf8CpToConversionChar[n].resize(MAX_UTF8_CODEPOINT+1);
  }

  _codepointToUtf8.resize(MAX_UTF8_CODEPOINT+1);
  _isAlphanumCodepointUtf8.resize(MAX_UTF8_CODEPOINT+1);

  // We init the conversion tables by mapping each codepoint
  // and each character onto itself.
  for (size_t n = 0; n < CONVERSION_MAX; n++)
  {
    for (size_t m = 0; m <= MAX_UTF8_CODEPOINT; m++)
    {
      _utf8ConversionTable[n][m] = m;
      _utf8CpToConversionChar[n][m] = getUtf8Encoding(m);
    }
  }
  for (size_t m = 0; m <= MAX_UTF8_CODEPOINT; m++)
  {
    _codepointToUtf8[m] =  _utf8CpToConversionChar[CONVERSION_TO_NORM][m];
    _isAlphanumCodepointUtf8[m] = true;
  }


  // Read the map files
  // NEW 09Feb12 Ina: Added possibility to specify the path. For more
  // information see function init();

  string path = _pathToCharacterDefinitionFiles
                + _sCharacterDefinitionFile_Utf8;
  FILE* file = fopen(path.c_str(), "r");

  if (file == NULL && _pathToCharacterDefinitionFiles != "")
  {
    // TODO(bartsch): should be more flexible
    file = fopen(_sCharacterDefinitionFile_Utf8.c_str(), "r");
  }

  if (file == NULL)
  {
    // TODO(bartsch): should be more flexible
    string sTmp = "../codebase/utility/" + _sCharacterDefinitionFile_Utf8;
    file = fopen(sTmp.c_str(), "r");
  }

  if (file == NULL)
  {
    _sLastError = "Can't open character mapping file \'";
    _sLastError += _pathToCharacterDefinitionFiles
                 + _sCharacterDefinitionFile_Utf8  + "\'";
    return false;
  }

  string sLine = "";
  size_t nLineCount = 1;
  char   szLine[MAX_LENGTH_OF_LINE];
  char*  cptrRet = NULL;
  while (true)
  {
    cptrRet = fgets(szLine, MAX_LENGTH_OF_LINE, file);
    if (cptrRet == NULL)
    {
      break;
    }
    sLine = szLine;

    if (sLine.length() < 2 || sLine[0] == '#')
    {
      // Lines starting with '#' are comments
      nLineCount++;
      continue;
    }

    if (sLine.find("\n") == string::npos)
    {
      // _sLastError = "";
      _sLastError = "Line too long: " + intToString(nLineCount);
      _sLastError += " in file " + _sCharacterDefinitionFile_Utf8;
      return false;
    }
    sLine.erase(sLine.length() - 1);
    sLine = replaceLinefeed(sLine);

    // Expected line format:
    //
    //  <codepoint (dec)> [TAB] = decimal unicode codepoint
    //  <codepoint(hex)>  [TAB] = hexadecimal unicode codepoint
    //  <char>            [TAB] = utf8-character (byte sequence)
    //  <utf8>            [TAB] = utf8-characters in hex representation
    //  <name>            [TAB] = verbose unicode name
    //  <norm-cp>         [TAB] = decimal unicode codepoint of norm character
    //  <norm-char>       [TAB] = norm character (byte sequence)
    //  <upcase-cp>       [TAB] = codepoint of upcase variant
    //  <upcase-char>     [TAB] = upcase variant of the character
    //  <lowcase-cp>      [TAB] = codepoint of lowcase variant
    //  <lowcase-char>          = lowcase variant of the character

    vector<string> vsToken;

    if (putTokenInVector(sLine, FIELD_SEP, &vsToken) !=  NUMB_OF_FIELDS_UTF8)
    {
      _sLastError = "Illegal format in line " + intToString(nLineCount);
      _sLastError += " in file " + _sCharacterDefinitionFile_Utf8;
      return false;
    }

    size_t nCp          = atol(vsToken[0].c_str());
    size_t nCpNormChar  = atol(vsToken[5].c_str());
    size_t nCpUpperChar = atol(vsToken[7].c_str());
    size_t nCpLowerChar = atol(vsToken[9].c_str());


    if ( !(((0 <= nCp) && (nCp <= MAX_UTF8_CODEPOINT)) &&
            ((0 <= nCpNormChar) && (nCpNormChar <= MAX_UTF8_CODEPOINT)) &&
            ((0 <= nCpUpperChar) && (nCpUpperChar <= MAX_UTF8_CODEPOINT)) &&
            ((0 <= nCpLowerChar) && (nCpLowerChar <= MAX_UTF8_CODEPOINT))))
    {
      _sLastError = "Illegal codepoint in line " + intToString(nLineCount);
      _sLastError += " in file " + _sCharacterDefinitionFile_Utf8;
      return false;
    }

    _codepointToUtf8[nCp] = vsToken[2];

    _utf8CpToConversionChar[CONVERSION_TO_NORM][nCp] = vsToken[6];
    _utf8CpToConversionChar[CONVERSION_TO_UPPER][nCp] = vsToken[8];
    _utf8CpToConversionChar[CONVERSION_TO_LOWER][nCp] = vsToken[10];

    _utf8ConversionTable[CONVERSION_TO_NORM][nCp] = nCpNormChar;
    _utf8ConversionTable[CONVERSION_TO_UPPER][nCp] = nCpUpperChar;
    _utf8ConversionTable[CONVERSION_TO_LOWER][nCp] = nCpLowerChar;

    // vsToken[4] contains the Unicode name of the character.
    // We use this to decide, if the character ist alphanumerical:
    if (vsToken[4].find("LETTER") == string::npos &&
        vsToken[4].find("DIGIT") == string::npos)
    {
      // character is not alphanumerical:
      _isAlphanumCodepointUtf8[nCp] = false;
    }

    nLineCount++;
  }  //  while(true)

  fclose(file);
  return true;
}


// _____________________________________________________________________________
bool StringConverter::_readIso8859_1_initFile()
{
  // Initializes the vectors
  // _iso8859_1_ConversionTable
  // _iso8859_1_CpToConversionChar
  // _codepointToIso8859_1
  // _isAlphanumCodepointIso8859_1
  // by reading map file _sCharacterDefinitionFile_Iso8859_1

  _sLastError = "";

  _iso8859_1_ConversionTable.resize(CONVERSION_MAX);
  _iso8859_1_CpToConversionChar.resize(CONVERSION_MAX);

  for (size_t n = 0; n < CONVERSION_MAX; n++)
  {
    _iso8859_1_ConversionTable[n].resize(MAX_ISO_CODEPOINT+1);
    _iso8859_1_CpToConversionChar[n].resize(MAX_ISO_CODEPOINT+1);
  }

  _codepointToIso8859_1.resize(MAX_ISO_CODEPOINT+1);
  _isAlphanumCodepointIso8859_1.resize(MAX_ISO_CODEPOINT+1);

  // We init the conversion tables by mapping each codepoint
  // and each character onto itself.
  for (size_t n = 0; n < CONVERSION_MAX; n++)
  {
    for (size_t m = 0; m <= MAX_ISO_CODEPOINT; m++)
    {
      _iso8859_1_ConversionTable[n][m] = m;
      _iso8859_1_CpToConversionChar[n][m] = static_cast<char>(m);
    }
  }

  for (size_t m = 0; m <= MAX_ISO_CODEPOINT; m++)
  {
    _codepointToIso8859_1[m] =
          _iso8859_1_CpToConversionChar[CONVERSION_TO_NORM][m];
    _isAlphanumCodepointIso8859_1[m] = true;
  }

  // Read the map files
  // NEW 09Feb12 Ina: Added possibility to specify the path. For more
  // information see function init();
  string path = _pathToCharacterDefinitionFiles
                + _sCharacterDefinitionFile_Iso8859_1;
  FILE* file = fopen(path.c_str(), "r");

  if (file == NULL && _pathToCharacterDefinitionFiles != "")
  {
    // TODO(bartsch): should be more flexible
    file = fopen(_sCharacterDefinitionFile_Iso8859_1.c_str(), "r");
  }

  if (file == NULL)
  {
    // TODO(bartsch): should be more flexible
    string sTmp = "../utility/" + _sCharacterDefinitionFile_Iso8859_1;
    file = fopen(sTmp.c_str(), "r");
  }

  if (file == NULL)
  {
    _sLastError = "Can't open character mapping file \'";
    _sLastError += _pathToCharacterDefinitionFiles
                 + _sCharacterDefinitionFile_Iso8859_1  + "\'";
    return false;
  }

  string sLine;
  size_t nLineCount = 1;
  char   szLine[MAX_LENGTH_OF_LINE];
  char*  cptrRet = NULL;

  while (true)
  {
    cptrRet = fgets(szLine, MAX_LENGTH_OF_LINE, file);
    if (cptrRet == NULL)
    {
      break;
    }
    sLine = szLine;

    if (sLine.length() < 2 || sLine[0] == '#')
    {
      // Lines starting with '#' are comments
      nLineCount++;
      continue;
    }

    if (sLine.find("\n") == string::npos)
    {
      // _sLastError = "";
      _sLastError = "Line too long: " + intToString(nLineCount);
      _sLastError += " in file " + _sCharacterDefinitionFile_Utf8;
      return false;
    }
    sLine.erase(sLine.length() - 1);
    sLine = replaceLinefeed(sLine);


    // Expected line format:
    //
    //  <codepoint (dec)> [TAB] = decimal iso codepoint
    //  <char>            [TAB] = iso-character
    //  <name>            [TAB] = verbose unicode name of iso character
    //  <norm-cp>         [TAB] = decimal unicode codepoint of iso norm
    //                            character
    //  <norm-char>       [TAB] = iso norm character
    //  <upcase-cp>       [TAB] = codepoint of upcase variant
    //  <upcase-char>     [TAB] = upcase variant of the character
    //  <lowcase-cp>      [TAB] = codepoint of lowcase variant
    //  <lowcase-char>          = lowcase variant of the character

    vector<string> vsToken;

    if (putTokenInVector(sLine, FIELD_SEP, &vsToken) !=  NUMB_OF_FIELDS_ISO)
    {
      _sLastError = "Illegal format in line " + intToString(nLineCount);
      _sLastError += " in line " + _sCharacterDefinitionFile_Iso8859_1;
      return false;
    }

    size_t nCp          = atol(vsToken[0].c_str());
    size_t nCpNormChar  = atol(vsToken[3].c_str());
    size_t nCpUpperChar = atol(vsToken[5].c_str());
    size_t nCpLowerChar = atol(vsToken[7].c_str());


    if ( !(((0 <= nCp) && (nCp <= MAX_ISO_CODEPOINT)) &&
           ((0 <= nCpNormChar) && (nCpNormChar <= MAX_ISO_CODEPOINT)) &&
           ((0 <= nCpUpperChar) && (nCpUpperChar <= MAX_ISO_CODEPOINT)) &&
           ((0 <= nCpLowerChar) && (nCpLowerChar <= MAX_ISO_CODEPOINT))))
    {
      _sLastError = "Illegal codepoint in line " + intToString(nLineCount);
      _sLastError += " in line " + _sCharacterDefinitionFile_Iso8859_1;
      return false;
    }

    _codepointToIso8859_1[nCp] = vsToken[1];

    _iso8859_1_CpToConversionChar[CONVERSION_TO_NORM][nCp] = vsToken[4];
    _iso8859_1_CpToConversionChar[CONVERSION_TO_UPPER][nCp] = vsToken[6];
    _iso8859_1_CpToConversionChar[CONVERSION_TO_LOWER][nCp] = vsToken[8];

    _iso8859_1_ConversionTable[CONVERSION_TO_NORM][nCp] = nCpNormChar;
    _iso8859_1_ConversionTable[CONVERSION_TO_UPPER][nCp] = nCpUpperChar;
    _iso8859_1_ConversionTable[CONVERSION_TO_LOWER][nCp] = nCpLowerChar;

    // vsToken[2] contains the Unicode name of the character.
    // We use this to decide, if the character ist alphanumerical:
    if (vsToken[2].find("LETTER") == string::npos &&
        vsToken[2].find("DIGIT") == string::npos)
    {
      // character is not alphanumerical:
      _isAlphanumCodepointIso8859_1[nCp] = false;
    }

    nLineCount++;
  }  // while(getline(ifMappingFile, sLine))

  fclose(file);
  return true;
}


// _____________________________________________________________________________
void StringConverter:: _initUrlEncode()
{
  _codepointToUrlEncode.resize(MAX_URL_ENCODE_CODEPOINT+1);

  for (size_t n = 0; n <= MAX_URL_ENCODE_CODEPOINT; n++)
  {
    _codepointToUrlEncode[n] = static_cast<char>(n);
  }
  _codepointToUrlEncode[32] = "%20";  // SPACE
  _codepointToUrlEncode[33] = "%21";  // EXCLAMATION MARK
  _codepointToUrlEncode[34] = "%22";  // QUOTATION MARK
  _codepointToUrlEncode[35] = "%23";  // NUMBER SIGN
  _codepointToUrlEncode[36] = "%24";  // DOLLAR SIGN
  _codepointToUrlEncode[37] = "%25";  // PERCENT SIGN
  _codepointToUrlEncode[38] = "%26";  // AMPERSAND
  _codepointToUrlEncode[39] = "%27";  // APOSTROPHE
  _codepointToUrlEncode[40] = "%28";  // LEFT PARENTHESIS
  _codepointToUrlEncode[41] = "%29";  // RIGHT PARENTHESIS
  _codepointToUrlEncode[42] = "%2a";  // ASTERISK
  _codepointToUrlEncode[43] = "%2b";  // PLUS SIGN
  _codepointToUrlEncode[44] = "%2c";  // COMMA
  // _codepointToUrlEncode[45] = "%2d";  // HYPHEN-MINUS
  // _codepointToUrlEncode[46] = "%2e";  // FULL STOP
  _codepointToUrlEncode[47] = "%2f";  // SOLIDUS

  _codepointToUrlEncode[58] = "%3a";  // COLON
  _codepointToUrlEncode[59] = "%3b";  // SEMICOLON
  _codepointToUrlEncode[60] = "%3c";  // LESS-THAN SIGN
  _codepointToUrlEncode[61] = "%3d";  // EQUALS SIGN
  _codepointToUrlEncode[62] = "%3e";  // GREATER-THAN SIGN
  _codepointToUrlEncode[63] = "%3f";  // QUESTION MARK
  _codepointToUrlEncode[64] = "%40";  // COMMERCIAL AT

  _codepointToUrlEncode[91] = "%5b";  // LEFT SQUARE BRACKET
  _codepointToUrlEncode[92] = "%5c";  // REVERSE SOLIDUS
  _codepointToUrlEncode[93] = "%5d";  // RIGHT SQUARE BRACKET
  _codepointToUrlEncode[94] = "%5e";  // CIRCUMFLEX ACCENT
  //  _codepointToUrlEncode[95] = "%5f";  // LOW LINE
  _codepointToUrlEncode[96] = "%60";  // GRAVE ACCENT

  _codepointToUrlEncode[123] = "%7b";  // LEFT CURLY BRACKET
  _codepointToUrlEncode[124] = "%7c";  // VERTICAL LINE
  _codepointToUrlEncode[125] = "%7d";  // RIGHT CURLY BRACKET
  // _codepointToUrlEncode[126] = "%7e"; // TILDE
  _codepointToUrlEncode[127] = "%7f";
}
