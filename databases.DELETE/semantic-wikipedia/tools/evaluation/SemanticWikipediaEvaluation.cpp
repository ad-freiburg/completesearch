// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <set>
#include "./SemanticWikipediaEvaluation.h"
#include "./SemanticWikipediaQuery.h"

using std::string;
using std::vector;
using std::set;
using std::pair;
using std::cerr;

// _____________________________________________________________________________
SemanticWikipediaEvaluation::SemanticWikipediaEvaluation(const string& hostIP,
    int port)
{
  _hostIP = hostIP;
  _port = port;
  _k = -1;
}
// _____________________________________________________________________________
void SemanticWikipediaEvaluation::evaluate(string inputFileName,
    string outputFileName)
{
  assert(inputFileName.size() > 0);
  assert(outputFileName.size() > 0);

  // Read from the input file.
  std::ifstream inputS(inputFileName.c_str(), std::ios::in);
  string line;

  vector<string> input;

  while (std::getline(inputS, line))
  {
    input.push_back(line);
  }
  // Close input handle.
  inputS.close();

  set<string> queriesDone;
  vector<string> currentEntities;

  for (size_t l = 0; l < input.size(); ++l)
  {
    line = input[l];
    // Split the line at the tab.
    string::size_type tabPos = line.find('\t');
    if (tabPos == string::npos || tabPos == 0 || tabPos == line.size())
    {
      cerr << "Incorrect input format in line:\n\t" << line << "\n";
      assert(false);
    }
    string query = line.substr(0, tabPos);
    string entity = line.substr(tabPos + 1, line.size() - (tabPos + 1));

    // Check if the entity name has to be changed because the original
    // title is only a redirect.
    TitleEntityMap::const_iterator itMap = _titleEntityMap.find(entity);
    if (itMap != _titleEntityMap.end())
    {
      entity = itMap->second;
    }

    // Use the lower case version of the entity with
    // white-spaces replaced by underscores as key in the map.
    std::string::iterator it = entity.begin();
    while (it != entity.end())
    {
      if (isspace(*it))
      {
        *it = '_';
      }
      else
      {
        *it = static_cast<char> (tolower(*it));
      }
      ++it;
    }


    if (l == 0)
    {
      _currentQuery = query;
    }

    if (queriesDone.find(query) != queriesDone.end())
    {
      cerr << "Incorrect input format! Input has to be sorted by query!\n";
      assert(false);
    }

    // If the entry corresponds to a new query, process the last one
    // since its input should be completely read now.
    if (query != _currentQuery)
    {
      std::cout << "Processing query: " << _currentQuery << "... "
          << std::flush;
      vector<string> resultSet;
      doQuery(_currentQuery, &resultSet);
      compareInputAndResult(currentEntities, resultSet);
      queriesDone.insert(_currentQuery);
      std::cout << "done." << std::endl << std::flush;
      _currentQuery = query;
      currentEntities.clear();
    }
    currentEntities.push_back(entity);
  }
  // Process the last, remaining query.
  std::cout << "Processing query: " << _currentQuery << "... " << std::flush;
  vector<string> resultSet;
  doQuery(_currentQuery, &resultSet);
  compareInputAndResult(currentEntities, resultSet);
  std::cout << "done." << std::endl << std::flush;
  // Write the result to the output file.
  writeResultToFile(outputFileName);
}
// _____________________________________________________________________________
void SemanticWikipediaEvaluation::doQuery(const string& queryString, vector<
    string>* resultSet)
{
  SemanticWikipediaQuery query(queryString, _hostIP, _port);
  if (_k > 01)
  {
    query.setNumberOfCompletionsParam(_k);
  }
  query.queryForEntities(resultSet);
}
// _____________________________________________________________________________
void SemanticWikipediaEvaluation::compareInputAndResult(
    const vector<string>& input, const vector<string>& result)
{
  vector<string> expected(input);
  vector<string> actual(result);
  EntityComparator comp;
  EntityEquality eq;
  // Sort expected and actual into A and B and remove duplicates
  std::sort(expected.begin(), expected.end(), comp);
  expected.erase(std::unique(expected.begin(), expected.end(), eq),
      expected.end());
  std::sort(actual.begin(), actual.end(), comp);
  actual.erase(std::unique(actual.begin(), actual.end(), eq), actual.end());
  // Do a simple merge of two sorted lists. Write flags according to
  // which list (or possible both) the word is from.
  // Therefore go through both lists.
  size_t e = 0;
  size_t a = 0;
  size_t eSize = expected.size();
  size_t aSize = actual.size();
  assert(eSize > 0);
  // If there are no actual results, report everything as MISSING
  if (aSize < 1)
  {
    while (e < eSize)
    {
      std::ostringstream os;
      os << _currentQuery << "\t" << expected[e] << "\t" << "MISSING";
      _result.push_back(os.str());
      e++;
    }
    return;
  }
  while (true)
  {
    int compRes = comp.compare(expected[e], actual[a]);
    if (compRes == 0)
    {
      // If a word is equal, write the word and TRUE.
      std::ostringstream os;
      os << _currentQuery << "\t" << expected[e] << "\t" << "TRUE";
      _result.push_back(os.str());
      // Advance in both lists.
      if (e < eSize - 1)
      {
        e++;
      }
      else
      {
        // Write all remaining actual with FALSE
        while (a < aSize - 1)
        {
          a++;
          std::ostringstream os;
          os << _currentQuery << "\t" << actual[a] << "\t" << "FALSE";
          _result.push_back(os.str());
        }
        return;
      }

      if (a < aSize - 1)
      {
        a++;
      }
      else
      {
        // Write all remaining expected with MISSING
        while (e < eSize - 1)
        {
          e++;
          std::ostringstream os;
          os << _currentQuery << "\t" << expected[e] << "\t" << "MISSING";
          _result.push_back(os.str());
        }
        return;
      }
    }

    // If the word in expected is lexicographically smaller,
    // write this and MISSING. Advance in expected if possible.
    if (compRes < 0)
    {
      std::ostringstream os;
      os << _currentQuery << "\t" << expected[e] << "\t" << "MISSING";
      _result.push_back(os.str());
      if (e < eSize - 1)
      {
        e++;
      }
      else
      {
        // Write all remaining actual with FALSE
        while (a < aSize)
        {
          std::ostringstream os;
          os << _currentQuery << "\t" << actual[a] << "\t" << "FALSE";
          _result.push_back(os.str());
          a++;
        }
        return;
      }
    }

    // If the word in actual is lexicographically smaller,
    // write this word and FALSE; Advance in actual.
    if (compRes > 0)
    {
      std::ostringstream os;
      os << _currentQuery << "\t" << actual[a] << "\t" << "FALSE";
      _result.push_back(os.str());
      if (a < aSize - 1)
      {
        a++;
      }
      else
      {
        // Write all remaining actual with FALSE
        while (e < eSize)
        {
          std::ostringstream os;
          os << _currentQuery << "\t" << expected[e] << "\t" << "MISSING";
          _result.push_back(os.str());
          e++;
        }
        return;
      }
    }
  }
}
// _____________________________________________________________________________
void SemanticWikipediaEvaluation::writeResultToFile(
    const string& outputFileName)
{
  FILE* outputFile = fopen(outputFileName.c_str(), "w");
  for (size_t i = 0; i < _result.size(); ++i)
  {
    fprintf(outputFile, "%s\n", _result[i].c_str());
  }
}
// ____________________________________________________________________________
void SemanticWikipediaEvaluation::readRedirectMap(
    const std::string& redirectMapFileName)
{
  std::cerr << "Reading the redirect-map from file " << redirectMapFileName
      << "...\t" << std::flush;

  // Read the title - entity mapping.
  std::ifstream fs(redirectMapFileName.c_str(), std::ios::in);
  // Read line-wise.
  std::string line;
  while (std::getline(fs, line))
  {
    // Separate titles and entity by tabs.
    std::string::size_type indextofTab = line.find('\t');
    if (!(indextofTab > 0 && indextofTab != line.npos))
    {
      std::cerr << "discovered broken line: " << line << std::endl
          << std::flush;
      continue;
    }
    // assert(indextofTab > 0 && indextofTab != line.npos);
    std::string title = line.substr(0, indextofTab);
    std::string entity = line.substr(indextofTab + 1, line.length()
        - indextofTab);

    // Add the info to the hash-map member.
    _titleEntityMap[title] = entity;
  }
  fs.close();
  std::cerr << "Read " << _titleEntityMap.size() << " title-entity pairs."
      << std::endl << std::flush;
}
// _____________________________________________________________________________
bool EntityComparator::operator()(const string& x, const string& y) const
{
  return compare(x, y) < 0;
}
// _____________________________________________________________________________
int EntityComparator::compare(const string& left, const string& right) const
{
  for (string::const_iterator lit = left.begin(), rit = right.begin(); lit
      != left.end() && rit != right.end(); ++lit, ++rit)
  {
    if (tolower(*lit) < tolower(*rit))
      return -1;
    else if (tolower(*lit) > tolower(*rit))
      return 1;
  }
  if (left.size() < right.size())
    return -1;
  if (left.size() > right.size())
    return 1;
  return 0;
}
// _____________________________________________________________________________
bool EntityEquality::operator()(const string& x, const string& y) const
{
  EntityComparator comp;
  return comp.compare(x, y) == 0;
}
