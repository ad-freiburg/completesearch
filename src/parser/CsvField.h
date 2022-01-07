// Copyright 2009-2012, Albert-Ludwigs-University Freiburg
//
// Author: unknown

#ifndef PARSER_CSVFIELD_H_
#define PARSER_CSVFIELD_H_

// Defines the class CsvField holding the options for a single column of a
// csv-file.

#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::vector;
using std::pair;

enum FormatEnum
{
  XML,
  JSON,
  TEXT
};

class CsvField
{
 public:
  typedef std::pair<size_t, size_t> Precision;

  CsvField()
  {
    _fulltext = false;
    _excerpt = false;
    _phraseCompletion = false;
    _filter = false;
    _filterPlus = false;
    _facet = false;
    _facetId = false;
    _order_date = false;
    _order_literal = false;
    _order_precision = false;
    _score = 1;
    _name = "";
    _format = TEXT;
    _multipleItemsAllowed = false;
  }

  // Getter.
  static const vector<pair<FormatEnum, vector<bool> > >& getStaticShowList()
  {
    return _showList;
  }
  const string& getName() const { return _name; }
  unsigned getColumn() const { return _column; }
  unsigned getFormat() const { return _format; }
  bool getFulltext() const { return _fulltext; }
  bool getExcerpt() const { return _excerpt; }
  // HACK(bast): score must never be zero (buildIndex fails otherwise).
  // NEW 09.02.2021: Where does it fail?
  unsigned getScore() const { return _score; }  // _score == 0 ? 1 : _score; }
  bool getPhraseCompletion() const { return _phraseCompletion; }
  bool getFilter() const { return _filter; }
  bool getFilterPlus() const { return _filterPlus; }
  bool getFacet() const { return _facet; }
  bool getFacetId() const { return _facetId; }
  bool getMultipleItems() const { return _multipleItemsAllowed; }
  bool getOrderingDate() const { return _order_date; }
  bool getOrderingLiteral() const { return _order_literal; }
  bool getOrderingPrecision() const { return _order_precision; }
  Precision getPrecision() const { return _precision; }

  // Setter.
  void setName(const string& value) { _name = value; }
  void setColumn(const unsigned value) { _column = value; }
  void setFulltext(const string& value)
  {
    if (value.compare("true") == 0) _fulltext = true;
    else
      _fulltext = false;
  }
  void setExcerpt(const string& value)
  {
    if (value.compare("true") == 0) _excerpt = true;
    else
      _excerpt = false;
  }
  // NEW 04.01.13 (baumgari): Changed the setter function of show, since we
  // cannot use it in a generalized way anymore, because we need a vector of
  // bools now.
  static void setStaticShowList(const unsigned int showListIndex,
                                const unsigned int columnIndex,
                                const unsigned int numOfColumns)
  {
    // Add a new show list and initialize it with xml and a all values set to
    // false.
    while (showListIndex >= _showList.size())
    {
      _showList.push_back(make_pair(XML, vector<bool>(numOfColumns, false)));
    }
    _showList[showListIndex].second[columnIndex] = true;
  }

  static void setStaticShowListOutputFormat(
      const unsigned int showListIndex,
      const string& value)
  {
    // This assert should just occur, if setStaticShowList() wasn't called
    // properly.
    assert(showListIndex < _showList.size());
    string format = value;
    for (size_t i = 0; i < format.size(); i++)
      format[i] = tolower(format[i]);
    if (format.compare("xml") == 0)
      _showList[showListIndex].first = XML;
    else if (format.compare("json") == 0)
      _showList[showListIndex].first = JSON;
  }

  static void resetStaticShowList()
  {
    _showList.clear();
  }


  /*
  void setShow(const string& value) { 
    if (value.compare("true") == 0) _show = true; 
    else _show = false;
  }*/
  void setScore(const string& value) { _score = atoi(value.c_str()); }
  void setPhraseCompletion(const string& value)
  {
    if (value.compare("true") == 0) _phraseCompletion = true;
    else
      _phraseCompletion = false;
  }
  void setFilter(const string& value)
  {
    if (value.compare("true") == 0) _filter = true;
    else
      _filter = false;
  }
  void setFilterPlus(const string& value)
  {
    if (value.compare("true") == 0) _filterPlus = true;
    else
      _filterPlus = false;
  }
  void setFacet(const string& value)
  {
    if (value.compare("true") == 0) _facet = true;
    else
      _facet = false;
  }
  void setFacetId(const string& value)
  {
    if (value.compare("true") == 0) _facetId = true;
    else
      _facetId = false;
  }

  void setFormat(const string& value)
  {
    string format = value;
    for (size_t i = 0; i < format.size(); i++)
      format[i] = tolower(format[i]);
    if (format.compare("xml") == 0) _format = XML;
    else if (format.compare("json") == 0)
      _format = JSON;
    else if (format.compare("text") == 0)
      _format = TEXT;
  }

  void setMultipleItems(const string& value)
  {
    if (value.compare("true") == 0) _multipleItemsAllowed = true;
    else
      _multipleItemsAllowed = false;
  }

  // TODO(bast): drop requirement that argument needs to be of length 3.
  // TODO(bast): Add a test for this and other non-trivial functions in this
  // class. Check also in CsvParserOptions.{h,cpp}.
  void setOrdering(const string& value)
  {
    if (value.compare("date") == 0) _order_date = true;
    else if (value.compare("literal") == 0) _order_literal = true;
    else if (value.size() == 3 && value.find('.') != string::npos)
    {
      // std::cout << "CsvField.h: line 85: Precission set!" << std::endl;
      _order_precision = true;
      _precision.first = atoi(&value[0]);
      _precision.second = atoi(&value[2]);
    }
  }

 private:
  static vector<pair<FormatEnum, vector<bool> > > _showList;
  string _name;
  unsigned _column;
  unsigned _format;
  bool _fulltext;
  bool _excerpt;
  bool _multipleItemsAllowed;
  unsigned _score;
  bool _phraseCompletion;
  bool _filter;
  bool _filterPlus;
  bool _facet;
  bool _facetId;
  bool _order_date;
  bool _order_literal;
  bool _order_precision;
  Precision _precision;
};

typedef void (CsvField::*CsvFieldSetter)(const string& value);

#endif  // PARSER_CSVFIELD_H_
