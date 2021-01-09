// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

/*
 * File for the various intersect algorithms
 */

#ifndef SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_INTERSECTALGORITHMS_H_
#define SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_INTERSECTALGORITHMS_H_
#include "../QueryResult.h"
#include "./Utilities.h"
#include "../Timer.h"
namespace IntersectPerformanceEvaluation
{
  // make the simple intersection algorithm with one array that always has
  // 4 ints stored in sequential order, like this:
  // | first element doc id | first element word id | first element score |
  // first element position | second element doc id | second element word id
  // .....
  void simpleIntersectNx4Array(int* inputList1, size_t arrayLength1,
                               int* inputList2, size_t arrayLength2,
                               ArrayWithSize* result);

  // intersect with the simple algorithm above, but only
  // intersect parts of the lists, given by
  // the index limits (intersection will include lower and upper index!)
  // make the simple intersection algorithm with one array that always has
  // 4 ints stored in sequential order, like this:
  // | first element doc id | first element word id | first element score |
  // first element position | second element doc id | second element word id
  // .....
  void simpleIntersectNx4ArrayInLimits(int* inputList1,
                                       size_t from1, size_t to1,
                                       int* inputList2,
                                       size_t from2, size_t to2,
                                       ArrayWithSize* result);
  void* simpleIntersectNx4ArrayInLimitsPthread(void *par);
  // simple intersection
  void patternAlgorithmIntersect(const QueryResult& list1,
                                 const QueryResult& list2,
                                       QueryResult& result);

    void patternAlgorithmIntersectMod1(const QueryResult& list1,
                                       const QueryResult& list2,
                                             QueryResult& result);
    void patternAlgorithmIntersectMod2(const QueryResult& list1,
                                       const QueryResult& list2,
                                             QueryResult& result);
    void patternAlgorithmUnion(const QueryResult& list1,
                               const QueryResult& list2,
                                     QueryResult& result);
  // check whether the given results of an intersection are correct!
  bool checkIntersectionCorrectness(const QueryResult& list1,
                                    const QueryResult& list2,
                                    const QueryResult& result);
  // check whether the given results of an array-intersection are correct!
  bool checkArrayIntersectionCorrectness(int* inputList1, size_t arrayLength1,
                                         int* inputList2, size_t arrayLength2,
                                         ArrayWithSize* result);

  // concatenate the results into one, result already needs array
  // with corretly preallocated size
  void concatenateResults(ArrayWithSize* results, size_t nrOfResults,
                          ArrayWithSize* result);
}

#endif  // SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_INTERSECTALGORITHMS_H_

