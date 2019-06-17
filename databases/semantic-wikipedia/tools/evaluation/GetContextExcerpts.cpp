// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

#include "../../utils/File.h"
#include "../../utils/HashSet.h"

#include "../../codebase/server/CompleterBase.h"
#include "../../codebase/server/HYBCompleter.h"
#include "../../codebase/server/INVCompleter.h"

#include "../../codebase/fuzzysearch/FuzzySearcher.h"
#include "../../codebase/fuzzysearch/WordClusteringBuilder.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

TimedHistory someEmptyHistory;

const int MODE = WITH_DUPS + WITH_POS + WITH_SCORES;

// Available options.
struct option options[] =
{
{ "output", required_argument, NULL, 'o' },
{ "input", required_argument, NULL, 'i' },
{ "index-base", required_argument, NULL, 'x' },
{ NULL, 0, NULL, 0 } };

// Main function.
int main(int argc, char** argv)
{
  cout << endl << "<PROGRAM NAME>, version " << __DATE__ << " " << __TIME__
      << endl << endl;

  string inputFile = "";
  string outputFile = "";
  string indexBase = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "i:o:x:", options, NULL);
    if (c == -1)
      break;
    switch (c)
    {
      case 'i':
        inputFile = optarg;
        break;
      case 'o':
        outputFile = optarg;
        break;
      case 'x':
        indexBase = optarg;
        break;
      default:
        cout << endl << "! ERROR in processing options (getopt returned '" << c
            << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
            << endl << endl;
        exit(1);
    }
  }

  // Start the Program
  // Create a Completer with the index
  string indexFileName = indexBase + ".hybrid";
  string vocabularyFileName = indexBase + ".vocabulary";
  extern bool fuzzySearchEnabled;
  fuzzySearchEnabled = false;
  cout << indexFileName << " - " << vocabularyFileName << endl;
  // History emptyHistory;
  try
  {
    // fuzzySearchEnabled = false;
    HYBIndex index(indexFileName, vocabularyFileName, MODE);
    QueryResult* result = NULL;

    HybCompleter<MODE> completer(&index,
        &emptyHistory, &nullFuzzySearcher);
    cout << "Made Completer object" << endl << flush;
    completer.showMetaInfo();
    size_t cacheSize = 16 * 1024 * 1024;   // cache size of excerpts db in bytes
    ExcerptsGenerator eg(indexBase + ".docs.DB", cacheSize);

    // Query Parameters
    QueryParameters qpForEntities;
    qpForEntities.nofCompletionsToSend = UINT_MAX;
    qpForEntities.nofTopCompletionsToCompute = UINT_MAX;
    qpForEntities.nofExcerptsPerHit = 1;
    qpForEntities.nofHitsToSend = 1;

    QueryParameters qpForExcerpts;
    qpForExcerpts.nofExcerptsPerHit = 999;
    qpForExcerpts.nofHitsToSend = UINT_MAX;
    qpForExcerpts.nofTopHitsToCompute = UINT_MAX;
    qpForExcerpts.excerptRadius = 150;

    semsearch::File input(inputFile.c_str(), "r");
    semsearch::File output(outputFile.c_str(), "w");
    Query q;
    result = NULL;
    string line;
    char buf[4000];
    // Process line-wise, each line is a query
    while (input.readLine(&line, buf, 4000))
    {
      // Process the query
      q.setQueryString(line);
      cout << "Query: " << q << endl;
      try
      {
        completer.setQueryParameters(qpForEntities);
        completer.processComplexQuery(q, result);
        // cout << "Raw result size: " << qr->_docIds.size() << endl;
        //        cout << "Top Words #: " << qr->nofTotalCompletions << endl;
      } catch(Exception e)
      {
        cout << e.getErrorMessage() << endl << flush;
        cout << e.errorCodeAsString() << endl << flush;
        cout << e.getErrorDetails() << endl << flush;
        cout << e.getFullErrorMessage() << endl << flush;
      }

      // Get the entities from the result
      vector<string> entities;
      for (size_t i = 0; i < result->nofTotalCompletions; ++i)
      {
        WordId wordId = result->_topWordIds[i];
//        cout << "WordId: " << wordId << endl;
        entities.push_back((*completer._vocabulary)[wordId]);
      }
      std::cout << "Found " << entities.size() << " entities " << endl;
      // Set status to finished, else we get a history-related
      // error with the next queries.
      result->_status = QueryResult::FINISHED;
      QueryResult * resultExcerpt = NULL;
      // For each entity
      for (size_t i = 0; i < entities.size(); ++i)
      {
        Query qForExcerpts;
        // New query with the same query string but only this particular entity
        // Also get all excerpts
        qForExcerpts.setQueryString(line + " " +
            entities[i].substr(0, entities[i].rfind(':')) + "*");
        std::cout << "Query: " << qForExcerpts << endl;
        completer.setQueryParameters(qpForExcerpts);
        completer.processComplexQuery(qForExcerpts, resultExcerpt);
        vector<HitData> hits;
        eg.getExcerpts(qForExcerpts, qpForExcerpts, *resultExcerpt, hits);

        std::cout << "Hits: " << hits.size() << endl;
        // Write query, entity and all exceprts to output
        for (size_t j = 0; j < hits.size(); ++j)
        {
          std::cout << hits[j].title << endl;
          for (size_t e = 0; e < hits[j].excerpts.size(); ++e)
          {
            std::cout << " Excerpt " << e << ": " << hits[j].excerpts[e]
                << endl;
          }
        }
      }
    }
  } catch(Exception e)
  {
    cout << "XXXXXX FINAL CATCH BLOCK XXXXXX" << endl << flush;
    cout << e.getErrorMessage() << endl << flush;
    cout << e.errorCodeAsString() << endl << flush;
    cout << e.getErrorDetails() << endl << flush;
    cout << e.getFullErrorMessage() << endl << flush;
  }

  return 0;
}
