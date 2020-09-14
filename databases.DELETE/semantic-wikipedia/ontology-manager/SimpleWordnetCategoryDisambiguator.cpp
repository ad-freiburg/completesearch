// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

// Quick 'n Dirty tool for unifying the names
// of wordnet categories. Low code standards, TEMPORARY SOLUTION.
// TODO(buchholb): To be replaced with a solution that also
// manages to resolve categories like
// "wardrobe" or "chess" that relate to concrete entities.
// This tool uses an index as used by SUSI to make its decision.
// However, the "wardrobe-problem" has to be used pre-parsing
// because they relate to entity recognition.

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/server/HYBCompleter.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

const int MODE = WITH_DUPS + WITH_POS + WITH_SCORES;

string getSecondLastPartOf(string const& path)
{
  size_t lastColon = path.rfind(':');
  size_t secondLastColon = path.rfind(':', lastColon - 1);
  return path.substr(secondLastColon + 1, lastColon - (secondLastColon + 1));
}

void processCategory(string & lastClassname, string & lastNumber,
    vector<string> & previousNumbers, vector<string> & previousPaths,
    ad_utility::File & output, HybCompleter<MODE>& completer)
{
  if (previousPaths.size() >= 2)
  {
    ad_utility::HashSet<int> maxIndexes;
    size_t maxSize = 0;
    // Take all paths and query the completer for them.
    // Remember the one with the most hits.
    for (int i = 0; i < static_cast<int> (previousPaths.size()); ++i)
    {
      Query q(string(":e:") + previousPaths[i] + ":*");
      QueryResult* qr = NULL;
      cout << "About to query for " << q.getQueryString() << "..." << flush;
      try
      {
        completer.processComplexQuery(q, qr);
      }
      catch(Exception e)
      {
        cout << e.getErrorDetails() << endl << flush;
      }
      size_t size = qr->nofTotalHits;
      cout << "done, result size: " << size << endl << flush;

      // If there is no clear winner, all categories get the
      // name and will be mapped to the same name. This is necessary
      // in order to preserve ontology information on entities like
      // "chess" that have been recognized in the text.
      if (size > maxSize)
      {
        maxSize = size;
        maxIndexes.clear();
      }
      if (size == maxSize)
      {
        maxIndexes.insert(i);
      }
    }

    // Now assign new names
    for (int i = 0; i < static_cast<int> (previousPaths.size()); ++i)
    {
      if (maxIndexes.count(i) > 0)
      {
        // The winning path gets the name.
        output.writeLine(
            "wordnet_" + lastClassname + "_" + previousNumbers[i] + "\t"
                + lastClassname);
      }
      else
      {
        // All other categories now map to category_(directParent)
        output.writeLine(
            "wordnet_" + lastClassname + "_" + previousNumbers[i] + "\t"
                + lastClassname + "_(" + getSecondLastPartOf(previousPaths[i])
                + ")");
      }
    }
  }
  else
  {
    // Handle the inital case.
    if (!previousPaths.size() == 0)
    {
      // Trivial case. Everything is as we need it already,
      // just drop the wordnet prefix and the number.
      output.writeLine(
          "wordnet_" + lastClassname + "_" + lastNumber + "\t" + lastClassname);
    }
  }
}

// Available options.
struct option options[] = { {"paths-file", required_argument, NULL, 'p'},
                            {"index-base", required_argument, NULL, 'i'},
                            {"output", required_argument, NULL, 'o'},
                            {NULL, 0, NULL, 0}};

// Main function.
int main(int argc, char** argv)
{
  cout << endl << "SimpleWordnetCategoryUnifier, version " << __DATE__ << " "
      << __TIME__ << endl << endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string pathsFile = "";
  string ouputFile = "";
  string indexBase = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "i:p:o:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'o':
      ouputFile = optarg;
      break;
    case 'p':
      pathsFile = optarg;
      break;
    case 'i':
      indexBase = optarg;
      break;
    default:
      cout << endl << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  // Get default file names or exit when no default is supported
  if (ouputFile.size() == 0)
  {
    cout << endl << "Missing required argument: --ouput / -o " << endl << endl;
    exit(1);
  }

  if (pathsFile.size() == 0)
  {
    cout << endl << "Missing required argument: --paths-file / -p " << endl
        << endl;
    exit(1);
  }

  if (indexBase.size() == 0)
  {
    cout << endl << "Missing required argument: --index-base / -i " << endl
        << endl;
    exit(1);
  }

  // Actual Code for this mini-tool

  // Create a Completer with the index
  string indexFileName = indexBase + ".hybrid";
  string vocabularyFileName = indexBase + ".vocabulary";

  HYBIndex index(indexFileName, vocabularyFileName, MODE);
  //  FuzzySearch::FuzzySearcherBase* fuzzySearcher;
  //  fuzzySearcher = new FuzzySearch::FuzzySearcherUtf8;
  HybCompleter<MODE> completer(&index, &emptyHistory, &nullFuzzySearcher);

  //  // Hack to avoid computation of top hits in processComplexQuery
  //  SemanticQueryParameters qp;
  //  completer.setQueryParameters(qp);

  // Process the paths file
  ad_utility::File paths(pathsFile.c_str(), "r");
  ad_utility::File output(ouputFile.c_str(), "w");

  // Read the paths file line by line
  char buf[2048];
  string line;

  string classname;
  string wordnetnumber;
  string path;

  string lastClassname;
  string lastNumber;
  vector<string> previousNumbers;
  vector<string> previousPaths;

  while (paths.readLine(&line, buf, 2048))
  {
    // Parse the line
    // Separate entities, ID and path by tabs.
    // The input file look like this:
    // entity<tab>id<tab>path
    size_t indextofTab = line.find('\t');
    assert(indextofTab != string::npos);
    size_t indextofTab2 = line.find('\t', indextofTab + 1);
    assert(indextofTab2 != string::npos);
    classname = line.substr(0, indextofTab);
    wordnetnumber = line.substr(indextofTab + 1,
        indextofTab2 - (indextofTab + 1));
    path = line.substr(indextofTab2 + 1);

    // Detect consecutive lines that have the same word but different numbers
    // and collect the information.
    if (classname == lastClassname)
    {
      // Same name and same number are just for alternate paths.
      // We don't need to consider them here.
      if (!(wordnetnumber == lastNumber))
      {
        previousPaths.push_back(path);
        previousNumbers.push_back(wordnetnumber);
      }
    }
    else
    {
      // Process the previous category
      processCategory(lastClassname, lastNumber, previousNumbers,
          previousPaths, output, completer);

      // Clean and set everything up for the next name
      previousNumbers.clear();
      previousPaths.clear();
      previousPaths.push_back(path);
      previousNumbers.push_back(wordnetnumber);
    }
    lastClassname = classname;
    lastNumber = wordnetnumber;
  }

  // Handle the final category name.
  processCategory(lastClassname, lastNumber, previousNumbers, previousPaths,
      output, completer);
  return 0;
}
