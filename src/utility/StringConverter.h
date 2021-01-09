// Copyright 2010, Weitkaemper Technology
//
// Author: Wolfgang Bartsch <weitkaemper>
//
// Contains the class StringConverter which does some string replacements
// by a given list of find-replace pairs defined in the files
// utf8.map and iso8859-1.map.


#ifndef UTILITY_STRINGCONVERTER_H_
#define UTILITY_STRINGCONVERTER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using std::string;
using std::vector;
using std::ostream;



class StringConverter
{
 public:
  // Enumerators for conversion operations:
  enum Conversion
  {
    CONVERSION_MIN = 0,
    CONVERSION_TO_NORM,
    CONVERSION_TO_LOWER,
    CONVERSION_TO_UPPER,
    CONVERSION_TO_MASK,
    CONVERSION_FROM_MASK,
    // Add new enumerators for other conversions here
    CONVERSION_MAX
  };

  // Enumerators for encodings:
  enum Encoding
  {
    ENCODING_MIN = 0,
    ENCODING_UTF8,
    ENCODING_ISO8859_1,
    // Add new enumerators for other encodings here
    ENCODING_MAX
  };

  // Max length of line in map files
  static const size_t MAX_LENGTH_OF_LINE = 200;
  // Max UTF8 codepoint
  // Up to now we normalize only unicode codepoints lower 2048
  // The codepoints above are left unchanged
  static const size_t MAX_UTF8_CODEPOINT = 2047;
  static const size_t MAX_ISO_CODEPOINT = 255;
  // Only some ASCII characters are url encoded
  static const size_t MAX_URL_ENCODE_CODEPOINT = 127;
  static const char   FIELD_SEP = '\t';           // map file separator
  static const size_t NUMB_OF_FIELDS_UTF8 = 11;   // fields in utf8 map file
  static const size_t NUMB_OF_FIELDS_ISO =   9;   // fields in iso map file


  StringConverter();

  // Initializes the converter. This method has to be called at least once
  // before using the StringConverter object.
  // NEW 09Feb12 Ina: Added optional path to maps to init function. For more
  // information see cpp.
  bool init(string pathToCharacterDefinitionFiles = "");

  // Converts the string sToConvert with encoding enc to conversion con
  // and returns the converted string.
  //
  // Sample usage:
  // string s = "René Descartes"
  // convert(s, StringConverter::ENCODING_ISO8859_1,
  //            StringConverter::CONVERSION_TO_NORM);
  // returns "rene descartes"
  //
  string convert(const string& sToConvert,
                 Encoding      enc,
                 Conversion    con);

  // Returns a string in which reserved characters are url encoded.
  // (i.e. reserved characters have been replaced with a percent (%)
  // sign followed by two hex digits.
  // All reserved characters are ASCII-characters, therefore this method
  // works for ISO and UTF8
  //
  // Sample usage:
  // urlEncode("She loves C&A!") returns "She%20lovesC%26A%21"
  //
  string urlEncode(const string& sToConvert);

  // Converts several date string formats to the format dd-mm-yyyy
  // Possible input formats are:
  // [d]d.[m]m.yyyy  Expl.: 7.12.2000 --> 07-12-2000
  // [d]d-[m]m-yyyy  Expl.: 19.7.2010 --> 19-07-2010
  // yyyy-[m]m-[d]d  Expl.: 2010-1-1  --> 01-01-2010
  // Returns empty string in case of illegal input format
  string convertDate(const string& sToConvert);

  // Replaces all non alphanumerical characters in sIn with the
  // ASCII-Caracter cRepl with the exception of the characters whose
  // codepoints are given in vnSaveList.
  //
  // Sample usage:
  // vector<size_t> vnSaveCodepoints;
  // // Save SPACE, '$' and '&':
  // vnSavedCodepoints.push_back(32); // SPACE
  // vnSavedCodepoints.push_back(36); // $
  // vnSavedCodepoints.push_back(38); // &
  // string sStringToReplace = " !\"#$%&'()*+,-./0123456789:;<=>?@";
  // string sResult = replaceNonAlphanum(sStringToReplace,
  //                                     StringConverter::ENCODING_UTF8,
  //                                     '#',
  //                                     vnSavedCodepoints);
  // // Now: sResult is:  " ###$#&#########0123456789#######";
  //
  string replaceNonAlphanum(const string&         sIn,
                            Encoding              enc,
                            char                  cRepl,
                            bool                  bKeepByteNumb,
                            const vector<size_t>& vnSaveList);

  // Prints the UTF8-characters starting at codepoint nFirst up
  // to codepoint nLast converted by the conversion con to
  // stream osOut (just for control)
  void printUtf8Mapping(size_t     nFirst,
                        size_t     nLast,
                        ostream&   osOut,
                        Conversion con);

  // Prints the UTF8-characters starting at codepoint nFirst up
  // to codepoint nLast converted by the conversion con to
  // stream osOut (just for control)
  void printIso8859_1_Mapping(size_t     nFirst,
                              size_t     nLast,
                              ostream&   osOut,
                              Conversion con);

  // Returns the character sequence of the Unicode character with
  // Codepoint nCp.
  // Rememeber: a UTF8 character may consist of one to four bytes (=octets)
  // therefore the string returned may have length one to four.
  //
  // Expl.:  getUtf8Encoding(85) returns "U"
  //
  string getUtf8Encoding(size_t nCp);

  //
  bool utf8ToIso8859_1(const string& sUtf8, char cNonIso, string* sIso8859_1);
  bool iso8859_1ToUtf8(const string& sIso8859_1, string* sUtf8);

  // Returns last error, if any occured
  string getLastError();

  // Returns if StringConverter has already been initialized to avoid
  // reading in the maps more than once.
  bool _bAlreadyInitialized;





 private:
  // Reads map file _sCharacterDefinitionFile_Utf8 and
  // initializes datastructures
  bool _readUtf8initFile();

  // Get next char according to the encoding.
  size_t nextChar(const Encoding& encoding,
                  string::const_iterator& position,
                  const string::const_iterator& end);

  // Reads map file  _sCharacterDefinitionFile_Iso8859_1 and
  // initializes datastructures
  bool _readIso8859_1_initFile();

  // Initializes the url encoding vector _codepointToUrlEncode[]
  void _initUrlEncode();

  // Maps codepoints to their conversion counterparts
  //
  // Expl.:
  //
  // Codepoint 217 is: LATIN CAPITAL LETTER U WITH GRAVE
  // Codepoint 117 is: LATIN SMALL LETTER U
  // Codepoint  85 is: LATIN CAPITAL LETTER U
  //
  //  _utf8ConversionTable[CONVERSION_TO_NORM][217] = 117
  //  _utf8ConversionTable[CONVERSION_TO_UPPER][117] = 85
  //  _utf8ConversionTable[CONVERSION_TO_LOWER][85] = 117
  //
  // The Iso8859-1-Codepoints are the same as the first 256
  // Unicode-Codepoints. But this is not the case for other Iso
  // character encodings.
  //
  vector<vector<size_t> > _utf8ConversionTable;
  vector<vector<size_t> > _iso8859_1_ConversionTable;

  // Maps codepoints to conversion characters
  // That means: we map codepoints directly to strings representing the
  // conversion character instead of codepoints. We map to strings because
  // a UTF character may consist of one to four bytes.
  //
  // Expl.:
  //
  // Codepoint 217 is: LATIN CAPITAL LETTER U WITH GRAVE
  //
  // _utf8CpToConversionChar[CONVERSION_TO_NORM][217] = "u";
  //
  vector<vector<string> > _utf8CpToConversionChar;
  vector<vector<string> > _iso8859_1_CpToConversionChar;


  // Map codepoints to characters sequences
  //
  // Expl.:
  //
  // _codepointToUtf8[85]  = 'U'
  // _codepointToUtf8[117] = 'u'
  //
  // (we have to use the string class instead of char because
  // a UTF8-coded unicode character can consist of 1 to 4 characters.
  vector<string> _codepointToUtf8;
  vector<string> _codepointToIso8859_1;

  // Map codepoints to url encode string
  //
  // Expl.:
  //
  //  _codepointToUrlEncode[63] = "%3f"; // QUESTION MARK
  //
  vector<string> _codepointToUrlEncode;


  // Map codepoints of alphanumerical characters to true
  //
  // Expl.:
  // _isAlphanumCodepointUtf8[36] = false (CP 36 is '$' )
  // _isAlphanumCodepointUtf8[51] = true  (CP 51 is '3')
  // _isAlphanumCodepointUtf8[65] = true  (CP 65 is 'A')
  //
  // We use the unicode name of the character to decide, if
  // the character is alphanumerical or not.
  // In case of an alphanumerical character, the unicode name
  // contains the strings 'LETTER' or 'DIGIT'.
  //
  vector<bool> _isAlphanumCodepointUtf8;
  vector<bool> _isAlphanumCodepointIso8859_1;

  // NEW 09Feb12 Ina: Path to map files. For more see init in cpp.
  string                         _pathToCharacterDefinitionFiles;
  // Name of utf8 map file
  string                         _sCharacterDefinitionFile_Utf8;
  // Name of iso8859-1 map file
  string                         _sCharacterDefinitionFile_Iso8859_1;
  // Description of last error occured with the object
  string                         _sLastError;
};



#endif  // UTILITY_STRINGCONVERTER_H_
