// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <iostream>

#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./Index.h"
#include "./Engine.h"
#include "./Query.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
  {"ontology-base", required_argument, NULL, 'o'},
  {"fulltext-base", required_argument, NULL, 't'},
  {NULL, 0, NULL, 0}
};

namespace ad_semsearch
{
class QueryLogicPerformanceTest
{
  public:
    void doTest(QueryExecutionContext* qec)
    {
      try
      {
        {
          // Scientists
          LOG(INFO) << endl << "--- SCIENTISTS ---" << endl;
          ad_semsearch::Query query;
          query.setExecutionContext(qec);
          query.constructQueryTreeFromTriplesString(
              "$1 :r:is-a :e:scientist:Scientist", "$1");
          LOG(INFO) << query.asString() << endl;
          displayInfoForQuery(query);
        }
        {
          // Relativity person
          LOG(INFO) << "--- RELATIVITY PERSONS ---" << endl;
          ad_semsearch::Query query;
          query.setExecutionContext(qec);
          query.constructQueryTreeFromTriplesString(
              "$1 :r:is-a :e:person:Person; $1 :r:occurs-with relativity",
              "$1");
          LOG(INFO) << query.asString() << endl;
          displayInfoForQuery(query);
        }
        {
          // Relat* person
          LOG(INFO) << "--- RELAT* PERSONS ---" << endl;
          ad_semsearch::Query query;
          query.setExecutionContext(qec);
          query.constructQueryTreeFromTriplesString(
              "$1 :r:is-a :e:person:Person; $1 :r:occurs-with relat*", "$1");
          LOG(INFO) << query.asString() << endl;
          displayInfoForQuery(query);
        }
        {
          // Plants with edible leaves
          LOG(INFO) << "--- PLANTS WITH EDIBLE LEAVES ---" << endl;
          ad_semsearch::Query query;
          query.setExecutionContext(qec);
          query.constructQueryTreeFromTriplesString(
              "$1 :r:is-a :e:plant:Plant; $1 :r:occurs-with edible leaves",
              "$1");
          LOG(INFO) << query.asString() << endl;
          displayInfoForQuery(query);
        }
        {
          // Scientists born in Ulm.
          LOG(INFO) << "--- SCIENTISTS BORN IN ULM ---" << endl;
          ad_semsearch::Query query;
          query.setExecutionContext(qec);
          query.constructQueryTreeFromTriplesString(
              "$1 :r:is-a :e:scientist:Scientist; "
                  "$1 :r:born-in $2; "
                  "$2 :r:equals :e:ulm:Ulm", "$1");
          LOG(INFO) << query.asString() << endl;
          displayInfoForQuery(query);
        }
        {
          // Politician friend (scientist manhatten project)
          LOG(INFO)
              << "--- POLITICIANS THAT ARE FRIENDS WITH "
              << "A SCIENTIST ASSICIATED WITH THE MANHATTAN PROJECT ---"
              << endl;
          ad_semsearch::Query query;
          query.setExecutionContext(qec);
          query.constructQueryTreeFromTriplesString(
              "$1 :r:is-a :e:politician:Politician; "
                  "$1 :r:occurs-with friend* $2; "
                  "$2 :r:is-a :e:scientist:Scientist;"
                  "$2 :r:occurs-with manhattan project", "$1");
          LOG(INFO) << query.asString() << endl;
          displayInfoForQuery(query);
        }
      }
      catch(const Exception& e)
      {
        LOG(ERROR)
        << e.getFullErrorMessage() << endl;
      }
    }
  private:

    void displayInfoForQuery(const ad_semsearch::Query& query)
    {
      const IntermediateQueryResult& treeResult =
          query.getResultForQueryTree();
      LOG(INFO) << treeResult.asString() << endl;
      LOG(INFO) << "Top is " << query.getExecutionContext()->getIndex()
          .getOntologyWordById(treeResult._entities[0]._id) << endl;
      QueryResult queryResult;
      query.createQueryResult(&queryResult);
      for (size_t i = 0; i < queryResult._hitGroups.size(); ++i)
      {
        LOG(INFO) << "------------------------------------" << endl;
        LOG(INFO)
            << "Group: "
            << ad_utility::getLastPartOfString(
                queryResult._hitGroups[i]._entity, ':') << endl;
        for (size_t j = 0; j < queryResult._hitGroups[i]._hits.size(); ++j)
        {
          LOG(INFO) << queryResult._hitGroups[i]._hits[j] << endl;
        }
      }
      cout << endl << endl << endl;
    }
};
}
// Main function.
int main(int argc, char** argv)
{
    std::cout << std::endl << EMPH_ON << "QueryLogicPerformanceTest, version "
      << __DATE__ << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string ontologyBase = "";
  string fulltextBase = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "o:t:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'o':
      ontologyBase = optarg;
      break;
    case 't':
      fulltextBase = optarg;
      break;
    default:
      cout << endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  // Start the Program
  if (ontologyBase.size() == 0 || fulltextBase.size() == 0)
  {
    std::cerr << "Usage: " << std::endl
        << "QueryLogicPerformanceTest -o <ontologyBasename>"
        <<" -t <fulltextBasenme>"
        << endl;
    exit(1);
  }
  ad_semsearch::Index index;
  index.registerOntologyIndex(ontologyBase);
  index.registerFulltextIndex(fulltextBase, true);
  ad_semsearch::Engine engine;
  ad_semsearch::QueryExecutionContext qec(index, engine);
  ad_semsearch::QueryLogicPerformanceTest qlpt;
  qlpt.doTest(&qec);

  return 0;
}
