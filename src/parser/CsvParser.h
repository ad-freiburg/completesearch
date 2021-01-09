// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Authors: Jens Hoffmann <hoffmaje>, Hannah Bast <bast>.

#ifndef PARSER_CSVPARSER_H_
#define PARSER_CSVPARSER_H_

// Class for parsing a CSV file and producing a .words and a .docs file suitable
// for further processing with CompleteSearch.
//
// The options are specified in a CsvParserOptions object. This also contains
// the name of the file(s) to be parsed.
//
// Input file <basename>.csv:
//
// Column separator is specified in options. First line contains the names of
// the fields. Subsequent lines contain the individual records, one per line.
//
// Outputfile <basename>.words:
//
// For each word in each field specified via the --fulltext option, output the
// corresponding line to the words file (<word><TAB><doc
// id><TAB><score><TAB><position>). The doc id is 1 for the first line, 2 for
// the second line, etc. The score is currently 0 for all words. The position is
// simply counted upwards starting from 1.
//
// Outputfile <basename>.docs:
//
// For each line (after the first) in the CSV file, output a line to
// <basename>.docs (<doc id><TAB>u:<url><TAB>t:<title><TAB>H:<text>).
// As <url> put URL#<doc id>. As <title> put
// <show field="field1">...</show><show field="field2">...</show>,
// where <show field="fieldi">...</show> gives the contents of the i-th field
// specified by the --show option. As <text> put the concatenation of the text
// of all fields specified by the --excerpts option.

#include <vector>
#include <string>
#include "./CsvParserOptions.h"
#include "./ParserBase.h"
#include "./SimpleTextParser.h"
#include "./StringConversion.h"

class CsvParser : public ParserBase
{
 public:
  // Create parser object with given options.
  CsvParser();
  // Parse options from command line. First calls
  // ParserBase::parseCommandLineOptions and then parses options specific to
  // CsvParser. Then also sets various thingies according to these options.
  void parseCommandLineOptions(int argc, char** argv);
  // Print help on options.
  void printUsage(void) { _options.printUsage(); }
  // Run Parser. Reads file <basename>.csv and produces files
  // <basename>.words, <basename>.docs, and <basename>.parse-log. The latter
  // contains a log of peculiar things (warnings) that happened during
  // parsing.
  void parse();
 private:
  // A pair of the content string and Index of a field.
  struct FieldItem
  {
    FieldItem(const string& content, size_t index)
      : fieldContent(content), fieldIndex(index) {}
    string fieldContent;
    size_t fieldIndex;
  };

  // Parser options.
  CsvParserOptions _options;
  // Check iff the line read from the .csv file contains a '\n' and replace it
  // by a '\0'.
  bool containsNewLine(char* buffer);


  // Parsing record to <basename>.words and <basename>.doc.
  void writeFieldsToWordsAndDocsFile(unsigned docID, vector<FieldItem>* fields);

  // Two helper functions to parse fields for secondary separator.
  void writeFieldItemToWordsFile(unsigned docID,
                                 unsigned colID,
                                 string* fieldItem);
  void writeFieldsToDocsFile(unsigned docID, vector<FieldItem>* fields);

  // Function directly writes to file <basename>.words.
  void writeToWordsFile(const string& word,
                        unsigned docID,
                        unsigned score,
                        unsigned position);
  // For each word <e> in the given field, write the word <prefix><e><suffix>
  // to the words file (with the given doc id and score).
  void writeConcatToWordsFile(const string& field,
                              const string& prefix,
                              const string& suffix,
                              unsigned docID,
                              unsigned score);
  // Write each word in the given field to the words file.
  void writeFulltextToWordsFile(const string& field,
                                unsigned docID,
                                unsigned score);
  // For each word in the field, write the word with the contents of the whole
  // field appended to the words file.
  void writePhraseCompletionToWordsFile(const string& field,
                                        unsigned docID,
                                        unsigned score);
  // For each word <theta> in the field, write ":filter:<fieldName>:<theta>"
  // to the words file. If appendFieldContents == true, append (after another :)
  // the whole field contents (without lowercasing).
  void writeFilterToWordsFile(const string& field,
                              const string& fieldName,
                              unsigned docID,
                              unsigned score,
                              bool appendFieldContents);
  // Helper function for writeFacetToWordsFile, that removes delim chars.
  void removeDelimChars(string* word);
  // For the given string field write ":facets:<fieldName>:<field>, where
  // any space character in field is replaced by '_'.
  void writeFacetToWordsFile(const string& facetWord,
                             const string& field,
                             const string& fieldName,
                             unsigned docID,
                             unsigned score);
  // NEW 17May12 (Ina):
  // Autocomplete can't handle the new words format right now. Therefore I added
  // an option to use the old words format with ct:, cn:, etc.
  // For the given string field write
  // "ct/cn/ce:<fieldName>:<fieldToLower>:<field>, where any space character in
  // field is replaced by '_'.
  void writeFacetInOldWordsFormatToWordsFile(const string& field,
                                             const string& fieldName,
                                             unsigned docID,
                                             unsigned score);
  // Interpret the fields content as date of the form "DD-MM-YYYY" and write
  // :ordering:<fieldName>:<YYYYMMDD> to the words file.
  void writeOrderingDateToWordsFile(const string& field,
                                    const string& fieldName,
                                    unsigned docID,
                                    unsigned score);
  // Write the entire field in the form :ordering:<fieldName>:<field> to the
  // words file.
  void writeOrderingLiteralToWordsFile(const string& field,
                                       const string& fieldName,
                                       unsigned docID,
                                       unsigned score);
  // Interpret the given field as floating point number and write the number
  // in the form :ordering:<fieldName>:<number> where, for example for the
  // field 123,12345 number has the form 0000012312 when the option
  // --ordering="fieldName:8.2" was given.
  void writeOrderingPrecisionToWordsFile(const string& field,
      const string& fieldName,
      unsigned docID,
      unsigned score,
      const CsvField::Precision& precision);

  // The index support some artificial words, which can be read out from the
  // server and interpreted. For example it's possible to write the encoding,
  // the date if index construction or the output formats of each specified
  // info/show field in the index.
  void addGlobalInformationToWordsFile();

  // Function for escaping nonproper xml. This is done to
  // achieve proper xml, which can be used without cdata tags. Obviously it's
  // necessary to unescape it later again.
  string escapeXml(const string& text) const;
  // Function for escaping nonjson text. This is done to achieve proper json.
  string escapeJson(const string& text) const;

  // The current position.
  size_t _position;
  // TODO(hoffmaje): Perhaps better use locally.
  static const char* _fileExtension;
  // This fields holds the options set for any csv-field.
  vector<CsvField> _fieldOptions;
  // Needed to parse strings.
  SimpleTextParser _simpleTextParser;
  // Needed to conversion to lower case.
  StringConversion _stringConversion;
};

#endif  // PARSER_CSVPARSER_H_
