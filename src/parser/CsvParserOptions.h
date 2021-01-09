// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Authors: Jens Hoffmann <hoffmaje>, Hannah Bast <bast>.

#ifndef PARSER_CSVPARSEROPTIONS_H_
#define PARSER_CSVPARSEROPTIONS_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "parser/CsvField.h"
#include "parser/SimpleTextParser.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::flush;
using std::cerr;

// CSV parser options.
//
// Currently the following options are supported:
//
// --basename : base names of all files involved. The file to be parsed is
// called <basename>.csv, the output files will be named <basename>.words and
// <basename>.docs. The default basename is 'data'.
//
// --fulltext : name of fields to be indexed for full-text search, separated by
// commas.
//
// --show : name of fields to be shown in a query result, separated by comma.
// NOTE: this is data that is returned in the query result for each hit. Hence
// it should not be very large. For example, it should not be 1 MB of text as
// in a computer science article. Rather this is for short descriptions, titles,
// etc. There can be multiple show field lists, so different outputs can be
// provided to the user. Those lists are separated by semicolon. Moreover it's
// possible to specify the output format of a show list by appending :xml or
// :json, whereas xml is the default.
// Example: --show=field1,field2;field2,field3:json. This means, that the first
// show list is filled with the content of field1 and field2 using the default
// output format xml. The second show list is filled by field2 and field3 and
// is built as json string.
//
// --excerpts: name of fields for which excerpts of the text are shown. NOTE:
// this is for fields with potentially very large texts. Since only excerpts are
// shown, the query results will always be of moderate size, independent of the
// total size of these texts.
//
// --csv-separator : symbol that separates column is the CSV file. Default is a
// comma. The default seperator is '\t'.
//
// NEW 27No09:
//
// --scores : list of fields with score for each field, for example,
// --score="field2:10,field5:3".
//
// --phrase-completion : list of fields for which contents is to be completed as
// whole phrase (as opposed to completion of individual words in the field).
// Output words of the form nintendo:Nintendo_Family_Computer,
// family:Nintento_Family_Computer, computer:Nintento_Family_Computer.
//
// --facets : list of fields to be taken as facets for faceted search. Output
// words of the form :facet:Produktname:Nintento_Family_Computer.
//
// --filter : list of fields for which we can do filtering = search words only
// in that particular field. Output words of the form
// :filter:Produktname:nintendo, :filter:Produktname:family, etc.
//
// --ordering : list of fields according to which we want to order the results,
// together with a specification of the type of data. For now, let us support
// fixed-precision scalars and dates. For example --ordering="field1:8.2,
// field5:date, field7:literal". Output words of the form
// :ordering:field1:0000013582 (for the number 135,8264 in 8.2 precision),
// :ordering:field5:20091127 (for the date 27Nov09), ordering:field7:A305 (for
// the literal data A305). For now, assume that dates are in a fixed format (say
// DD-MM-YYYY) and log a warning if they are not.
//
// NEW 31Jan14 (baumgari):
//
// --field-format: list of fields according to how they are formatted
// (available: text (default), xml, json). For example:
// --field-format=author:text,xmlrecord:xml,record:json.
//
// --allow-multiple-entries: list of fields which may have multiple entries. A
// common field for that is e.g. author, since one document may have multiple
// authors. For those fields we expect the _withinFieldDelimiter to separate the
// fields. In other fields the _withinFieldDelimiter is allowed to be used.
// Fields, which allow to have multiple entries are handled differently, if the
// output format is Json and the field should be shown. In this case the field
// is always handled as array (e.g. "author": [] and
// "author": ["Peter", "Paul"]). Example:
// --allow-multiple-entries=author,editor.

#define TAB '\t'
#define SPACE ' '
#define EOL '\n'
#define CSV_MAX_LINE_LENGTH 10 * 1024 * 1024

class CsvParserOptions
{
 public:

  // Default constructor sets separators, etc. to default values.
  CsvParserOptions();

  // Print help on options.
  void printUsage() const;

  // Parsing command line options. Needs to read the first line from the CSV
  // file to be able to make sense of the options like --show, --facets,
  // --ordering, etc. Hence the csvFileName argument.
  void parseFromCommandLine(int argc, char** argv, const string& csvFileName);

  // Read options from file.
  // TODO(hoffmaje): Not implemented yet.
  void readFromFile(const string& fileName);

  // Write options to file.
  // TODO(hoffmaje): Not implemented yet.
  void writeToFile(const string& fileName);

  // Getter functions.
  char getColumnSeperator() const { return _columnSeparator; }
  char getFieldSeparator() const { return _fieldSeparator; }
  char getNoShowPrefix() const { return _noShowPrefix; }
  const vector<CsvField>& getFieldOptions() const { return _fieldOptions; }
  void getFieldName(unsigned int column, string* result) const;
  bool isOldWordsFormat() const { return _oldWordsFormat; }

 private:
  // The separator between columns in the CSV file.
  // TODO(bast): This should be called fieldsSeparator, since we refer to what
  // is called columns here as fields!
  char _columnSeparator;

  // The separator tha can be used inside of fields.
  // TODO(bast): This is a terrible misnomer, since this is actually the
  // within-a-field separator and not the separator separating fields as the
  // name suggests.
  char _fieldSeparator;

  // NEW 06Dec12 (baumgari): Character which can be put in front of an item in a
  // field to stop it to be added as facet or as show item.
  // This might be useful, if we want words to be stored as filter, but not as facet.
  char _noShowPrefix;

  // NEW 17May12 (Ina):
  // Specifies if the old words format (ct:, cn:, etc.), instead of :facet:
  // should be used, since the new format is not working with autocomplete right
  // now.
  bool _oldWordsFormat;

  // Our textparser.
  SimpleTextParser _simpleTextParser;

  // ----------------------------------------------------
  // Collect all options in a vector<Field>.
  vector<CsvField> _fieldOptions;
  // --------------------------------------------------

  // Parsing a list of arguments of the form: <column1>,<column2>,...
  void parseListOfOptArgs_Typ0(const string& optarg, CsvFieldSetter setterFn);
  // Parsing a list of arguments of the form:
  // <column1>:<option1>,<column2>:<option2>,...
  void parseListOfOptArgs_Typ1(const string& optarg, CsvFieldSetter setterFn);
  // Parsing a list of arguments of the form: <column1>,<column2>,...;<column1>,
  // <column2>,... and a list with the same length (or an empty list)
  // to specify multiple show lists.
  void parseListOfOptArgs_show(const string& showList);
  // Returns the column number of the given field name.
  int getColumnNumber(const string& fieldName);
};

#endif  // PARSER_CSVPARSEROPTIONS_H_
