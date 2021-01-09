// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#ifndef SERVER_COMPLETERBASE_INTERSECTPERF_UTILITIES_H_
#define SERVER_COMPLETERBASE_INTERSECTPERF_UTILITIES_H_
#include <gtest/gtest.h>
#include <stdio.h>
#include <string>
#include "../QueryResult.h"
// just supersimple struct to remember the size of an array,
// size is manually set...
struct ArrayWithSize
{
  // ~ArrayWithSize();  (this didnt really work as i thought :)(robin))
  int* array;
  size_t size;
};

typedef struct
{
  int* inputList1;
  int  arrayLength1;
  int  from1;
  int  to1;
  int* inputList2;
  int  arrayLength2;
  int  from2;
  int  to2;
  ArrayWithSize* result;
}ThreadParam;
// function for displaying a green-blue progressbar, only helper function
void displayProgressBar(float progressPerCent, int widthOfProgressBar);
// the possible query types..
enum queryTypes
{
  REALWORDS = 0,
  RANDOMLISTS = 1,
  PSEUDORANDOM = 2,
  MIXED = 3
};

// Determine the type of geven query normal or artifical
// (0 normal; 1 random; 2 pseudo random)
queryTypes getQueryType(const std::string &query);
// Devide target and baseName of a given path
void parsePath(const std::string &path, std::string *baseName,
                                        std::string *target);

// TODO(Robin): how to make it so that array size can be unknown to caller
// of function?
// transform a query result list into an array with n times 4tuples
void transformQueryListToNX4Arrays(const QueryResult& list, int* array);

// transform an array with n times 4tuples into a query list
void transformNx4ArrayIntoQueryList(int* array, int arrayLength,
                                    QueryResult* list);
#endif  // SERVER_COMPLETERBASE_INTERSECTPERF_UTILITIES_H_
