// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#ifndef SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_PERFORMANCEEVALUATOR_H_
#define SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_PERFORMANCEEVALUATOR_H_


#include <gtest/gtest.h>
#include <math.h>
#include <getopt.h>
#include <string>
#include <vector>
#include "../../server/HYBCompleter.h"
#include "../../fuzzysearch/FuzzySearcher.h"
#include "../../server/Timer.h"
#include "../Globals.h"
#include "./Utilities.h"
#include "./CompleterBase.TestStatistics.h"
#include "./CompleterBase.IntersectAlgorithms.h"

namespace IntersectPerformanceEvaluation
{
  class PerformanceEvaluatorQueryStatistics;

  // Class for evaluating the performance of various intersect/union algorithms
  // on various (real, random, or otherwise synthetic) query sets.
  class PerformanceEvaluator
  {
    typedef PerformanceEvaluatorQueryStatistics QueryStatistics;
    public:
    // The various algorithms.
    enum AlgorithmId
    {
      SIMPLE = 1
    };

    // standard constructor...
    PerformanceEvaluator(const string& indexTarget,
                         const string& indexBaseName,
                         const string& queriesBaseName,
                         const string& queriesFileTarget,
                         const string& statTarget,
                         int verbosity,
                         int loop);

    // Process queries from <base name>.queries and write results to <base
    // name>.perf-statistics and <base name>perf-statistics-summary. Calls
    // processSingleIntersectQuery for each query.
    void processIntersectQueries(AlgorithmId algorithmId);

    // Get the postings list for a given query token (word or prefix or
    // artificial word for random list).
    void getListForQuery(const string& query, QueryResult*& result);

    // get all the query words that this performance evaluator has read from
    // the queries file
    void getAllQueries(std::vector<std::vector<string> > * queries);

    // Process a single intersect query.
    void processSingleIntersectQuery(const QueryResult& list1,
                                     const QueryResult& list2,
                                     AlgorithmId algorithmId,
                                     StatisticsPool* statPool);

    // TODO(Robin): move to intersect algorithms or utilities...
    // calculate intersect gaps...
    void calculateIntersectGaps(const QueryResult& list1,
                                const QueryResult& list2,
                                double& gapSum1,
                                double& gapSum2,
                                double& gapSum3);

    // several getters...
    string getStatBaseName();
    int getRepetitions();
    int getVerbosity();

   private:
     // print the usage if user doesnt enter correct commands for
     // commandline constructor...
     void printUsage();


    size_t getFilesLineNumber(const string &fileName);

    size_t stringToInt(const string &str);
    HybCompleter<WITH_SCORES + WITH_POS + WITH_DUPS>* _completer;
    string _fileBaseName;
    string _indexTarget;
    string _queriesBaseName;
    string _queriesFileTarget;
    string _statTarget;

     // the level of verbosity on the console, default = 1
     // 1 means just progress output
     // 2 means detailed output during the execution
     int _verbosity;
     // the level of test repeating
     // How oft each query will be tested, default = 1
     int _loop;
  };
}
#endif  // SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_PERFORMANCEEVALUATOR_H_

