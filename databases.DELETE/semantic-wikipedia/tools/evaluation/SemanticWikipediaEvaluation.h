// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TOOLS_EVALUATION_SEMANTICWIKIPEDIAEVALUATION_H_
#define SEMANTIC_WIKIPEDIA_TOOLS_EVALUATION_SEMANTICWIKIPEDIAEVALUATION_H_

#include <string>
#include <vector>
#include "../../codebase/semantic-wikipedia-utils/HashMap.h"

using std::string;
using std::vector;

class SemanticWikipediaEvaluation
{
  public:

    SemanticWikipediaEvaluation(const string& hostIP, int port);

    // Do the evaluation. Reads ground truth and queries from input
    // and produced the output.
    // Input is of the form <query><TAB><Entity>.
    // Output is of the form <query><TAB><Entity><TAB><result>
    // where <result> is one out of TRUE/FALSE/MISSING.
    void evaluate(string inputFileName, string outputFileName);

    void readRedirectMap(const std::string& redirectMapFileName);

    void enableTopK(const int k)
    {
      _k = k;
    }

  private:

    string _hostIP;
    int _port;
    int _k;
    vector<string> _result;
    string _currentQuery;
    typedef ad_utility::HashMap<string, string> TitleEntityMap;
    TitleEntityMap _titleEntityMap;

    // Does a comparison of expected and actual for a single query.
    // This method adds the result to the member variable
    // _result.
    void compareInputAndResult(const vector<string>& expected,
        const vector<string>& actual);

    // Writes the result to the output file.
    void writeResultToFile(const string& outputFileName);

    void doQuery(const string& query, vector<string>* resultSet);
};

class EntityComparator
{
  public:

    bool operator()(const string& x, const string& y) const;

    // Returns 0 if the strings are equal, -1 if the left
    // string is "smaller" and 1 if the right one is smaller.
    // Smaller means lexicographically smaller,
    // disregarding the case.
    int compare(const string& x, const string& y) const;
};

class EntityEquality
{
  public:

    bool operator()(const string& x, const string& y) const;
};
#endif  // SEMANTIC_WIKIPEDIA_TOOLS_EVALUATION_SEMANTICWIKIPEDIAEVALUATION_H_
