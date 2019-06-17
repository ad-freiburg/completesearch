// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Authors: Jens Hoffmann <hoffmaje>, Hannah Bast <bast>.

#include <getopt.h>
#include <errno.h>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include "parser/ParserBase.h"
#include "parser/CsvParserOptions.h"

// NEW 04Jan13 (baumgari): _showList is a static member of CsvField, which is
// declared here to avoid creating a new cpp.
vector<pair<FormatEnum, vector<bool> > > CsvField::_showList;

// ____________________________________________________________________________
CsvParserOptions::CsvParserOptions()
{
  _columnSeparator = TAB;
  _fieldSeparator = 0;
  _noShowPrefix = 0;
  _oldWordsFormat = false;
  CsvField::resetStaticShowList();
}


// ____________________________________________________________________________
void CsvParserOptions::printUsage() const
{
  cout << "Usage: ./CsvParserMain [with options as follows]"
       << endl
       << endl
       << "--field-separator            : a single character that separates"
       <<                               " fields (default = TAB)."
       << endl
       << "--within-field-separator     : a single character that separates"
       <<                               " multiple instances of a fields; can"
       <<                               " be the empty string (default = empty)"
       << endl
       << "--full-text                  : fields to be indexed for full text"
       <<                               " search."
       << endl
       << "--show                       : fields to be returned in result."
       <<                               " The output format of a show list can"
       <<                               " be specified by appending :xml or"
       <<                               " :json."
       <<                               " Multiple show lists can be separated"
       <<                               " by ; (take care of escaping)."
       << endl
       << "--excerpts                   : fields for which excerpts will be"
       <<                               " available."
       << endl
       << "--phrase                     : fields for which completion is to"
       <<                               " whole field."
       << endl
       << "--filter                     : fields for which to write words like"
       <<                               " :filter:<field>:<word> to the index."
       << endl
       << "--filter-plus                : like --filter, but writing words like"
       <<                               " :filter:<fld>:<word>:<fld-content>"
       << endl
       << "--facets                     : fields to add as categories / facets."
       << endl
       << "--facetids                   : outputs the same words as --facets,"
       <<                               " but with prefix facetid instead of"
       <<                               " facet; one can then use two different"
       <<                               " block divisions (in the HYB index)"
       <<                               " for the two types of words."
       << endl
       << "--no-show-prefix             : a single character in front of field"
       <<                               " items, which should not be added as"
       <<                               " facet or show item."
       << endl
       << "--ordering                   : how to order given fields."
       << endl
       << "--score                      : the score for the given fields."
       << endl
       << "--old-words-format           : use ct: and cn: instead of :facet:"
       << endl
       << "--field-format               : format of the given field in the csv"
       <<                               ", supported formats: xml, json"
       <<                               " (default: xml)."
       << endl
       << "--allow-multiple-items       : fields, which may have more than one"
       <<                               " item. A typical case is \"author\"."
       << endl << endl;
  ParserBase::printUsage();
  cout << endl;
}


// ____________________________________________________________________________
void CsvParserOptions::getFieldName(unsigned int column, string* result) const
{
  assert(result != NULL);
  if (column < result->size())
  {
    result->clear();
    *result = _fieldOptions[column].getName();
  }
}


// ____________________________________________________________________________
int CsvParserOptions::getColumnNumber(const string& fieldName)
{
  int result = -1;
  for (unsigned int i = 0; i < _fieldOptions.size(); i++)
  {
    if (fieldName == _fieldOptions[i].getName())
    {
      result = i;
      break;
    }
  }
  return result;
}

// ____________________________________________________________________________
void CsvParserOptions::parseListOfOptArgs_show(const string& showList)
{
  // The comma is the delimiter for separating fields.
  // NEW 04.01.13 (baumgari): In case of multiple show lists detect them by
  // searching for the delimiter ;.
  // ; is the delimiter to separate show lists
  // NEW 06Mar14 (baumgari): By appending :xml or :json it is possible to
  // specify the output of a show list.
  const char showFieldDelim = ',';
  const char showListDelim = ';';
  const char outputDelim = ':';

  string fieldName;
  unsigned int showListIndex = 0;
  bool isOutputFormatField = false;
  size_t start = 0;
  int column = -1;
  for (size_t i = 0; i <= showList.size(); i++)
  {
    const char ch = (i == showList.size() ? ';' : showList[i]);
    if (ch == showFieldDelim || ch == showListDelim || ch == outputDelim)
    {
      fieldName = showList.substr(start, i - start);
      if (!fieldName.empty() && !isOutputFormatField)
      {
        column = getColumnNumber(fieldName);

        if (column == -1)
        {
          cerr << "CsvParserOptions:125: Field does not exist: " << fieldName
               << endl;
          exit(1);
        }
        // Call setter function to enable the option for field column in list i.
        CsvField::setStaticShowList(showListIndex, column,
                                    _fieldOptions.size());
        fieldName.clear();
        column = -1;
      }
      else if (!fieldName.empty() && isOutputFormatField)
      {
        CsvField::setStaticShowListOutputFormat(showListIndex, fieldName);
        fieldName.clear();
      }
      isOutputFormatField = false;
      if (ch == outputDelim) isOutputFormatField = true;
      if (ch == showListDelim) showListIndex++;
      start = i + 1;
    }
  }
}

// ____________________________________________________________________________
void CsvParserOptions::parseListOfOptArgs_Typ0(const string& optarg,
                                               CsvFieldSetter setterFn)
{
  int column = -1;
  size_t start = 0;
  size_t end = 0;
  string fieldName;

  while (end != string::npos)
  {
    end = optarg.find(',', start);
    fieldName = optarg.substr(start, end - start);
    column = getColumnNumber(fieldName);

    if (column == -1)
    {
      cerr << "CsvParserOptions:106: Field does not exist: " << fieldName
           << endl;
      exit(1);
    }
    // Call setter function to enable the option.
    (_fieldOptions[column].*(setterFn))("true");
    start = end + 1;
  }
}


// ____________________________________________________________________________
void CsvParserOptions::parseListOfOptArgs_Typ1(const string& optarg,
                                               CsvFieldSetter setterFn)
{
  int column = -1;
  size_t start = 0;
  size_t end = 0;
  string fieldName;
  string option;
  while (end != string::npos)
  {
    // Get the name of the column.
    end = optarg.find(':', start);
    fieldName = optarg.substr(start, end - start);
    column = getColumnNumber(fieldName);
    if (column == -1)
    {
      cerr << "CsvParserOptions.cpp: 116: The given field name '"
           << fieldName << "' does not exist." << endl;
      exit(1);
    }
    start = end + 1;
    // Get the prepended option.
    end = optarg.find(',', start);
    option = optarg.substr(start, end - start);
    // cout << "CsvParserOptions::line 124: option =" << option << endl;
    // Call setter function to enable option.
    (_fieldOptions[column].*(setterFn))(option);
    start = end + 1;
  }
}

// ____________________________________________________________________________
void CsvParserOptions::parseFromCommandLine(int argc, char** argv,
                                            const string& csvFileName)
{
  #ifdef DEBUG_CSV_PARSER_OPTIONS
  cout << "CsvParserOptions::parseFromCommandLine ... " << flush;
  #endif
  string fulltext;
  string excerpts;
  string show;
  string score;
  string phraseCompletion;
  string filter;
  string filterPlus;
  string fieldFormat;
  string outputFormat;
  string facets;
  string facetids;
  string ordering;
  string multipleItemsFields;

  FILE *csvFile = NULL;

  // Read remaining command-line options. (Those for ParserBase have already
  // been read at this point.)
  opterr = 0;
  optind = 1;
  while (true)
  {
    // Define command-line options.
    static struct option longOptions[] =
    {
      {"help",                   0, NULL, 'h'},
      {"full-text",              1, NULL, 'f'},
      {"show",                   1, NULL, 's'},
      {"excerpts",               1, NULL, 'e'},
      {"field-separator",        1, NULL, 'c'},
      {"within-field-separator", 1, NULL, 'C'},
      {"score",                  1, NULL, 'S'},
      {"phrase",                 1, NULL, 'p'},
      {"filter",                 1, NULL, 'F'},
      {"filter-plus",            1, NULL, 'P'},
      {"facets",                 1, NULL, 'a'},
      {"facetids",               1, NULL, 'i'},
      {"no-show-prefix",         1, NULL, 'x'},
      {"ordering",               1, NULL, 'o'},
      {"old-words-format",       1, NULL, 'w'},
      {"field-format",           1, NULL, 't'},
      {"allow-multiple-items"  , 1, NULL, 'M'},
      { NULL,                    0, NULL,  0 }
    };
    int c = getopt_long(argc, argv, "hn:f:s:C:e:c:S:p:F:P:a:i:x:o:m:t:w:M",
                        longOptions, NULL);
    // cout << "CsvParserOptions::parseCommendLineOptions ["
    //      << c << "|" << (char)(c) << "]" << endl;
    if (c == -1) break;
    switch (c)
    {
      case 0:
        break;
      case 'h':
        printUsage();
        break;
      case 'f':
        fulltext = string(optarg);
        break;
      case 's':
        show = string(optarg);
        break;
      case 'e':
      excerpts = string(optarg);
      break;
      case 'c':
        if ( strcmp(optarg, "TAB") == 0 ) _columnSeparator = TAB;
        else if ( strcmp( optarg, "SPACE") == 0) _columnSeparator = SPACE;
        else
        {
          #ifdef DEBUG_CSV_PARSER_OPTIONS
          cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
               << endl
               << "CsvParserOptions.cpp:211: column separator: "
               << static_cast<int>(optarg[0])
               << endl
               << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
               << endl << endl;
          #endif
          _columnSeparator = optarg[0];
        }
        break;
      case 'C':
        _fieldSeparator = optarg[0];
        break;
      case 'S':
        score = string(optarg);
        break;
      case 'p':
        phraseCompletion = string(optarg);
        break;
      case 'F':
        filter = string(optarg);
        break;
      case 'P':
        filterPlus = string(optarg);
        break;
      case 'a':
        facets = string(optarg);
        break;
      case 'i':
        facetids = string(optarg);
        break;
      case 'x':
        _noShowPrefix = optarg[0];
        break;
      case 'o':
        ordering = string(optarg);
        break;
      case 'w':
        _oldWordsFormat = true;
        break;
      case 't':
        fieldFormat = string(optarg);
        break;
      case 'M':
        multipleItemsFields = string(optarg);
        break;
    }
  }
  // Okay. Finished parsing command line options. Now read first line in
  // <basename>.csv to determine the field name.
  // TODO(hoffmaje:) Log fopen error properly.
  char* buffer = new char[CSV_MAX_LINE_LENGTH];
  csvFile = fopen(csvFileName.c_str(), "r");
  if (csvFile == NULL)
  {
    snprintf(buffer, CSV_MAX_LINE_LENGTH, "fopen failed: %s",
             csvFileName.c_str());
    perror(buffer);
    exit(1);
  }
  // Read first line in csv-file to determine the field-names.
  string word;
  int column = 0;
  char delimiters[3] = { _columnSeparator, EOL, '\0' };
  CsvField csvField;
  size_t start = 0;
  size_t end = 0;

  // Parse the first line of the .csv file.
  char* ret = fgets(buffer, CSV_MAX_LINE_LENGTH, csvFile);
  if (ret == NULL)
  {
    perror("Can't read from file");
    exit(errno);
  }
  string record = string(buffer);

  SimpleTextParser simpletextparser;
  simpletextparser.setSeparators(delimiters);

  while (end < record.size() - 1)
  {
    simpletextparser.parseText(record, &start, &end);
    word = record.substr(start, end - start);
    csvField.setName(word);
    csvField.setColumn(column);
    _fieldOptions.push_back(csvField);
    column++;
  }

  // Field names are determined. Go on parsing all those arguments that
  // are lists, containing some (or no) fieldnames. They need some index value.
  if (fulltext != "") parseListOfOptArgs_Typ0(fulltext, &CsvField::setFulltext);
  if (excerpts != "") parseListOfOptArgs_Typ0(excerpts, &CsvField::setExcerpt);
  if (show != "")     parseListOfOptArgs_show(show);
  if (score != "")    parseListOfOptArgs_Typ1(score, &CsvField::setScore);
  if (fieldFormat != "")  parseListOfOptArgs_Typ1(fieldFormat,
                                                  &CsvField::setFormat);
  if (phraseCompletion != "") parseListOfOptArgs_Typ0(phraseCompletion,
                                 &CsvField::setPhraseCompletion);
  if (filter != "")   parseListOfOptArgs_Typ0(filter, &CsvField::setFilter);
  if (filterPlus != "") parseListOfOptArgs_Typ0(filterPlus,
                                                &CsvField::setFilterPlus);
  if (facets != "")   parseListOfOptArgs_Typ0(facets, &CsvField::setFacet);
  if (facetids != "") parseListOfOptArgs_Typ0(facetids, &CsvField::setFacetId);
  if (ordering != "") parseListOfOptArgs_Typ1(ordering, &CsvField::setOrdering);
  if (multipleItemsFields != "")
    parseListOfOptArgs_Typ0(multipleItemsFields, &CsvField::setMultipleItems);
}

void CsvParserOptions::readFromFile(const string& fileName)
{
  // TODO(hoffmaje): Implement it!
  cout << "Method CsvParserOptions::readFromFile not yet implemented." << endl;
  exit(1);
}
