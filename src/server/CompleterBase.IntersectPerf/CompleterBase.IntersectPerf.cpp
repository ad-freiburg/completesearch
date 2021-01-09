// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...


#include <gtest/gtest.h>
#include <getopt.h>
#include <math.h>
#include <string>
#include <utility>
#include "../../server/HYBCompleter.h"
#include "../../fuzzysearch/FuzzySearcher.h"
#include "../../server/Timer.h"
#include "../Globals.h"

#include "./CompleterBase.PerformanceEvaluator.h"
namespace IntersectPerformanceEvaluation
{
  string baseNameTmp;


  // Some terminology:
  // * query = a word OR a prefix OR an artificial word like R10MD100M
  // that means arandom list.
  // * token = a part of a query (in our case the space-separated part).
  // * posting = one item from a list; in the case of CompleteSearch these are
  // 4-tupels consisting of doc id, word id, position, score.
  // //namespace IntersectPerformanceEvaluation
  // //{






  // ___________________________________________________________________________
  // ___________________________________________________________________________
  // ___________________________________________________________________________
  TEST(CompleterBaseIntersectPerf, intersect)
  {
    string indexFileName = baseNameTmp + ".hybrid";
    string vocabularyFileName = baseNameTmp + ".vocabulary";

    // Create completer object from these files.
    const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
    HYBIndex index(indexFileName, vocabularyFileName, MODE);
    History history;
    FuzzySearch::FuzzySearcherUtf8 fuzzySearcher;
    HybCompleter<MODE> completer(index, history, fuzzySearcher);
    completer._metaInfo.show();

    // Get two lists (blocks) from index. Results of queries are also blocks,
    // hence we have QueryResult objects here.
    Query query1 = string("rob*");
    Query query2 = string("sim*");
    QueryResult* list1 = NULL;
    QueryResult* list2 = NULL;
    completer.processQuery(query1, list1);
    completer.processQuery(query2, list2);
    if (list1 == NULL || list2 == NULL)
    {
      printf("ERROR: empty query result\n");
      exit(1);
    }
    cout << "List1: [#" << list1->_docIds.size() << "]"
         << endl;
    cout << "List2: [#" << list2->_docIds.size() << "]"
         << endl;

    // Now intersect the two lists / blocks.
    Separator separator(" ", pair<signed int, signed int>(-1, -1), 0);
    QueryResult list3;
    completer.intersectTwoPostingListsNew(*list1, *list2, list3, separator);
    cout << "List3: [#" << list3._docIds.size() << "]"
         << endl;
  }





  // //}  // namespace IntersectPerformanceEvaluation
  // //using IntersectPerformanceEvaluation::PerformanceEvaluator;
  // ___________________________________________________________________________
  // Print Usage
  void printUsage()
  {
    cout << endl
         << "USAGE : ./CompleterBase.IntersectPerf [options]"
         << endl
         << endl
         << "Options are:"
         << endl
         << endl
         << " --(i)ndex-path:               Pathe for index files (*.hybrid, "
            "*.vocabulary) (FORMATE: directory + BaseName) REQUIRED"
         << endl
         << endl
         << " --(q)ueries-path:             Pathe for queries file"
         << " (FORMATE: directory + BaseName) (default = ./index-Base-Name)."
         << endl
         << endl
         << " --(s)tatistic-directory:      Directory for statistics files"
         << " (*.perf-statistics, *.perf-statistics-summary) "
         << " (default: queries-directory)."
         << endl
         << endl
         << " --(v)erbosity:                Level of verbosity"
         << " (0 for no output, 1 for progress output only, "
            "2 for detailed output)"
         << " (default: 1)."
         << endl
         << endl
         << " --(r)epeat:                   Level of repeating."
         << " How often each query will be tested"
         << " (default: 1)."
         << endl
         << endl;
  }




  // ___________________________________________________________________________
  // Devide target and baseName of given path
  void parsePath(const string &path, string *baseName, string *target)
  {
    size_t found = path.find_last_of("/");
    if (found != string::npos)
    {
      *target = path.substr(0, found + 1);
      *baseName = path.substr(found + 1);
    }
    else
    {
      *baseName = path;
      *target = "";
    }
    // cout << "path " << path << endl;
    // cout << "target " << *target << endl;
    // cout << "baseName " << *baseName << endl;
  }
  // ___________________________________________________________________________
  // PerformanceEvaluator Object will be created (testing main)
  void makeTest(int argc, char **argv)
  {
    // Index file atributes
    string indexBaseName = "";
    string indexPath = "";
    string indexTarget = "";

    // Query file atributes
    string queryBaseName = "";
    string queryPath = "";
    string queryTarget = "";

    // Statistics file atributes
    string statBaseName = "";
    string statPath = "";
    string statTarget = "";

    // standard verbosity level is 1 for just progress output
    int verbosity = 1;
    // standard looping level is 1 for repeating of output
    int loop = 1;
    // Needed for getopt_long.
    int optChr;
    optind  = 1;

    while (1)
    {
      int optionIndex = 0;

      // Define command-line options.
      static struct option longOptions[] =
      {
        {"help", no_argument, 0, 'h'},
        {"index-path", required_argument, 0, 'i'},
        {"queries-path", required_argument, 0, 'q'},
        {"statistics-directory", required_argument, 0, 's'},
        {"verbosity", required_argument, 0, 'v'},
        {"repeat", required_argument, 0, 'r'},
        {0, 0, 0, 0}
      };

      optChr = getopt_long(argc, argv, "hi:q:s:v:r:", longOptions,
                                                      &optionIndex);

      if (optChr == -1) break;

      switch (optChr)
      {
        case 0:
          break;
        case 'h':
          printUsage();
          exit(1);
          break;
        case 'i':
          indexPath = string(optarg);
          break;
        case 'q':
          queryPath = string(optarg);
          break;
        case 's':
          statTarget = string(optarg);
          if (optarg[strlen(optarg)-1] != '/')
            statTarget += "/";
          break;
        case 'v':
          verbosity = atoi(optarg);
          break;
        case 'r':
          loop = atoi(optarg);
          break;
        default:
          cerr << "unknown option: " << optChr << " ??" << endl;
          printUsage();
          exit(1);
          break;
      }
    }

    if ((indexPath == "")
     || (verbosity <= -1)
     || (loop <= 0))
    {
      printUsage();
      exit(1);
    }

    // Define index file atributes
    parsePath(indexPath, &indexBaseName, &indexTarget);

    // Define query file atributes
    if (queryPath == "")
    {
      queryBaseName = indexBaseName;
    }
    else
    {
      parsePath(queryPath, &queryBaseName, &queryTarget);
    }

    // Define statistics file atributes
    if (statTarget == "")
    {
      statTarget = queryTarget;
    }

    PerformanceEvaluator *test1 = new PerformanceEvaluator(indexTarget,
                                                           indexBaseName,
                                                           queryBaseName,
                                                           queryTarget,
                                                           statTarget,
                                                           verbosity,
                                                           loop);

    test1->processIntersectQueries(PerformanceEvaluator::SIMPLE);
  }
}
  // ___________________________________________________________________________
  int main(int argc, char **argv)
  {
    IntersectPerformanceEvaluation::makeTest(argc, argv);

    // //testing::InitGoogleTest(&argc, argv);
    // //return RUN_ALL_TESTS();
  }
