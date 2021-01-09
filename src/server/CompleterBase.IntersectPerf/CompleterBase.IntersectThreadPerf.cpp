// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...
// class for testing the performance of a multithreaded intersection
// (using boost and pthread)
// algorithm compared to a single-threaded intersection

#include <getopt.h>
#include <pthread.h>
#include <math.h>
#include <boost/thread.hpp>
#include <string>
#include <vector>
#include "../../server/Timer.h"
#include "./CompleterBase.PerformanceEvaluator.h"
#include "./Utilities.h"
#include "../QueryResult.h"
#include "./CompleterBase.IntersectAlgorithms.h"
#include "./CompleterBase.TestStatistics.h"

unsigned seed;
namespace IntersectPerformanceEvaluation
{
  // find the splitting points of the arrays, i.e. the points at which the lists
  // can be splitted for intersecting the splits in a threaded algorithm
  // splits are always INCLUSIVE endpoints of a part, so in a list with
  // 100 elements, and splits 0,25,53,74,100
  // the parts are: 0-25, 26-53, 54-74, 75-99
  void findSplittingPoints(int* inputList1, size_t arrayLength1,
                           int* inputList2, size_t arrayLength2,
                           size_t numberOfParts,
                           vector<size_t>* splits1, vector<size_t>* splits2)
  {
    // it can be assumed that splits1 and 2 have the length
    // of the number of parts - 1...
    // length of the parts needs to be a multiple of 4 ...
    size_t partLength = arrayLength1 / (numberOfParts * 4);
    partLength *= 4;

    // split the first array simply in numberOfParts parts
    splits1->push_back(0);
    for (size_t i = 0; i < numberOfParts - 1; i++)
      splits1->push_back(partLength * (i + 1));
    splits1->push_back(arrayLength1 - 4);  // last doc id


    // now find the splitting points in the second array!
    size_t docIdToFind;
    size_t lowerSearchBound = 0;
    size_t upperSearchBound = arrayLength2 / 4;
    size_t nextElementToTry = 0;
    size_t correctSplitIndex;
     splits2->push_back(0);
     for (size_t i = 0; i < numberOfParts - 1 &&
                        lowerSearchBound < upperSearchBound; i++)
     {
        // +1 because of the 0 at the start of splits1...
        docIdToFind = inputList1[splits1->at(i + 1)];

        while (true)
        {
          // if uppersaerchbound and lowersearchbound are only one number apart
          // the docID cannot be in list 1 and we can use the lower search bound
          // as the next splitting index
          if (upperSearchBound == lowerSearchBound + 1)
          {
            correctSplitIndex = lowerSearchBound * 4;
            break;
          }
          nextElementToTry = (upperSearchBound + lowerSearchBound) / 2;
          // found correct doc id, still have to go up in second list
          // so that all duplicates will be taken
          if (inputList2[nextElementToTry * 4] == docIdToFind)
          {
            while (inputList2[nextElementToTry * 4] == docIdToFind &&
                   nextElementToTry < arrayLength2 / 4)
              nextElementToTry++;
            if (nextElementToTry < arrayLength2 / 4)
              correctSplitIndex = (nextElementToTry - 1) * 4;
            else
              correctSplitIndex = arrayLength2 - 4;  // last doc id
           break;
          }
          else if (inputList2[4 * nextElementToTry] < docIdToFind)
          {
            lowerSearchBound = nextElementToTry;
          }
          else  // inputList2[4 * nextElementToTry] > docIdToFind
          {
            upperSearchBound = nextElementToTry;
          }
        }
        // remember split and reset bounds for next split search
        splits2->push_back(correctSplitIndex);
        lowerSearchBound = (correctSplitIndex / 4) + 4;
        upperSearchBound = arrayLength2 / 4;
     }

    // check for empty splits at the end
    // if yes, just set the end of the 2nd array to prevent
    // undefined behavior
    if (splits2->size() < numberOfParts)
    {
      for (size_t i = splits2->size(); i < numberOfParts; i++)
        splits2->push_back(arrayLength2 - 4);  // last doc id
    }
    // push back for last split
    splits2->push_back(arrayLength2 - 4);
  }
// _____________________________________________________________________________
// intersect two given input lists (given as NX4Array multithreadedly with bosst
  void pThreadIntersection(int* inputList1, size_t arrayLength1,
                           int* inputList2, size_t arrayLength2,
                           size_t threads, ArrayWithSize* results)
  {
    // first find the splitting points
    vector<size_t> splits1;
    vector<size_t> splits2;
    findSplittingPoints(inputList1, arrayLength1, inputList2, arrayLength2,
                        threads, &splits1, &splits2);
    size_t startIndex1;
    size_t endIndex1;
    size_t startIndex2;
    size_t endIndex2;

    pthread_t threadsArray[threads];  //NOLINT
    ThreadParam tPar[threads];  //NOLINT

    // distribute the itnersection to the threads according to the splits....
    for (size_t threadNr = 0; threadNr < threads; threadNr++)
    {
      // calculate start and end indices for the next thread!
      startIndex1 = splits1[threadNr];
      if (threadNr > 0)
        startIndex1 += 4;  // start one docId after last Thread!
      endIndex1 = splits1[threadNr + 1];
      startIndex2 = splits2[threadNr];
      if (threadNr > 0)
        startIndex2 += 4;  // start one docId after last Thread!
      endIndex2 = splits2[threadNr + 1];
      // allocate maximally possible result array and start next thread !
      results[threadNr].array = new int[endIndex2 - startIndex2  + 4];
      /* printf("1 thread before intersect startindex1: %8zu endIndex1: %8zu\n",
       *                                                startIndex1, endIndex1);

      printf("                             startindex2: %8zu endIndex2: %8zu\n",
       *                                  startIndex2, endIndex2);*/


      ThreadParam buf = {inputList1, arrayLength1, startIndex1, endIndex1,
                     inputList2, arrayLength2, startIndex2, endIndex2,
                     &(results[threadNr])};
      tPar[threadNr] = buf;
      int rc = pthread_create(&threadsArray[threadNr], NULL,
                              simpleIntersectNx4ArrayInLimitsPthread,
                              &(tPar[threadNr]));
    }
    // wait for all threads to finish intersecting
    for (int i = 0; i < threads; i++)
      pthread_join(threadsArray[i], NULL);
  }

// _____________________________________________________________________________
// intersect two given input lists (given as NX4Array multithreadedly with bosst
  void boostIntersection(int* inputList1, size_t arrayLength1,
                         int* inputList2, size_t arrayLength2,
                         size_t threads, ArrayWithSize* results)
  {
    // first find the splitting points
    vector<size_t> splits1;
    vector<size_t> splits2;
    findSplittingPoints(inputList1, arrayLength1, inputList2, arrayLength2,
                        threads, &splits1, &splits2);
    size_t startIndex1;
    size_t endIndex1;
    size_t startIndex2;
    size_t endIndex2;

    boost::thread_group threadGroup;
    boost::thread* boostThread;

    // distribute the itnersection to the threads according to the splits....
    for (size_t threadNr = 0; threadNr < threads; threadNr++)
    {
      // calculate start and end indices for the next thread!
      startIndex1 = splits1[threadNr];
      if (threadNr > 0)
        startIndex1 += 4;  // start one docId after last Thread!
      endIndex1 = splits1[threadNr + 1];
      startIndex2 = splits2[threadNr];
      if (threadNr > 0)
        startIndex2 += 4;  // start one docId after last Thread!
      endIndex2 = splits2[threadNr + 1];
      // allocate maximally possible result array and start next thread !
      results[threadNr].array = new int[endIndex2 - startIndex2  + 4];
      /* printf("1 thread before intersect startindex1: %8zu endIndex1: %8zu\n",
       *                                                startIndex1, endIndex1);

      printf("                             startindex2: %8zu endIndex2: %8zu\n",
       *                                  startIndex2, endIndex2);*/
      boostThread = new boost::thread(simpleIntersectNx4ArrayInLimits,
                                      inputList1,
                                      startIndex1, endIndex1,
                                      inputList2,
                                      startIndex2, endIndex2,
                                      &(results[threadNr]));
      threadGroup.add_thread(boostThread);
    }
    // wait for all threads to finish intersecting
    threadGroup.join_all();
  }

  void pseudoThreadIntersection(int* inputList1, size_t arrayLength1,
                                int* inputList2, size_t arrayLength2,
                                size_t threads, ArrayWithSize* results)
  {
    // first find the splitting points
    vector<size_t> splits1;
    vector<size_t> splits2;
    findSplittingPoints(inputList1, arrayLength1, inputList2, arrayLength2,
                        threads, &splits1, &splits2);
    size_t startIndex1;
    size_t endIndex1;
    size_t startIndex2;
    size_t endIndex2;

    // distribute the itnersection to the threads according to the splits....
    for (size_t threadNr = 0; threadNr < threads; threadNr++)
    {
      // calculate start and end indices for the next thread!
      startIndex1 = splits1[threadNr];
      if (threadNr > 0)
        startIndex1 += 4;  // start one docId after last Thread!
      endIndex1 = splits1[threadNr + 1];
      startIndex2 = splits2[threadNr];
      if (threadNr > 0)
        startIndex2 += 4;  // start one docId after last Thread!
      endIndex2 = splits2[threadNr + 1];
      // allocate maximally possible result array and start next thread !
      results[threadNr].array = new int[endIndex2 - startIndex2  + 4];
      /* printf("1 thread before intersect startindex1: %8zu endIndex1: %8zu\n",
       *                                                startIndex1, endIndex1);

      printf("                             startindex2: %8zu endIndex2: %8zu\n",
       *                                  startIndex2, endIndex2);*/

      simpleIntersectNx4ArrayInLimits(inputList1, startIndex1, endIndex1,
                                      inputList2, startIndex2, endIndex2,
                                      &(results[threadNr]));
    }
  }
  // _________________________TESTS__________________________________________
  // for tests
  // TODO(Robin): make this properly as a google test in utilities somehow?
  void testTransFormFunctions()
  {
    // check correctness of transformations by transforming twice
    // and then checking...
    // create array with nx4 tuples...
    size_t size = 2000000;
    int* array = new int[4 * size];
    int* compareArray = new int[4 * size];

    // make list
    size_t nextNumber = 0;

    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array[4 * i + 3] = nextNumber;
    }
    // transform into query list
    QueryResult newResult;
    transformNx4ArrayIntoQueryList(array, 4 * size, &newResult);
    // first comparison : original array and new result...
    DocList docIds = newResult._docIds;
    WordList wordIds = newResult._wordIds;
    PositionList positions = newResult._positions;
    ScoreList scores = newResult._scores;
    for (size_t i = 0; i < size; i++)
    {
      if (array[4 * i] != docIds[i])
        printf("Fehler an Stelle %zu, array: %d, docId: %d\n", i,
                                                array[4 * i], docIds[i]);
      if (array[4 * i + 1] != wordIds[i])
        printf("Fehler an Stelle %zu, array: %d, wordId: %d\n", i,
                                                array[4 * i + 1], wordIds[i]);
      if (array[4 * i + 2] != positions[i])
        printf("Fehler an Stelle %zu, array: %d, position: %d\n", i,
                                                array[4 * i + 2], positions[i]);
      if (array[4 * i + 3] != scores[i])
        printf("Fehler an Stelle %zu, array: %d, score: %d\n", i,
                                                array[4 * i + 3], scores[i]);
    }

    // retransform into array...
    // compare both arrays, should be the same...
    transformQueryListToNX4Arrays(newResult, compareArray);
    for (int i = 0; i < 4 * size; i++)
    {
      if (array[i] != compareArray[i])
        printf("Fehler an Stelle %d, array: %d, compareArray: %d\n", i,
                                                 array[i], compareArray[i]);
    }

    printf("Correctness Check Of Transform functions completed\n");
    delete array;
    delete compareArray;
  }
  // ___________________________________________________________________________
  // test the function taht checks wheter intersection is correct!
  void testCorrectnessFunction()
  {
    // check correctness of transformations by transforming twice
    // and then checking...
    // create array with nx4 tuples...
    size_t size = 2000000;

    QueryResult list1;
    QueryResult list2;
    QueryResult result;

    // make first list
    int* array = new int[4 * size];
    size_t nextNumber = 0;

    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array[4 * i + 3] = nextNumber;
    }
    // transform into query list...
    transformNx4ArrayIntoQueryList(array, 4 * size, &list1);

    // make 2nd list
    nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array[4 * i + 3] = nextNumber;
    }
    // transform into query list...
    transformNx4ArrayIntoQueryList(array, 4 * size, &list2);

    patternAlgorithmIntersect(list1, list2, result);

    bool intersectionCorrect = checkIntersectionCorrectness(list1, list2,
                                                            result);
    printf("Intersection is correct at CheckCorrectnessFunction: %s\n",
                                        intersectionCorrect ?  "true":"false");
    delete array;
  }
  // ___________________________________________________________________________
  // checks whether simple Nx4 Array  Intersect is working correctly
  // and producing correct intersection (compares to other algorithm)
  void testSimpleIntersectNx4Array()
  {
    // check correctness of transformations by transforming twice
    // and then checking...
    // create array with nx4 tuples...
    size_t size = 100000;

    QueryResult list1;
    QueryResult list2;
    QueryResult result;

    int* array1 = new int[4 * size];
    int* array2 = new int[4 * size];
    int* resultArray = new int[4 * size];

    // make first list
    size_t nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array1[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array1[4 * i + 3] = nextNumber;
    }

    // make 2nd list
    nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array2[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array2[4 * i + 3] = nextNumber;
    }
    ArrayWithSize resultArrayWithSize;
    resultArrayWithSize.array = resultArray;


    simpleIntersectNx4Array(array1, size * 4, array2, size * 4,
                            &resultArrayWithSize);
    // determine length of result
    size_t resultLength = resultArrayWithSize.size;


    // transform the arrays into query lists...
    transformNx4ArrayIntoQueryList(array1, 4 * size, &list1);
    transformNx4ArrayIntoQueryList(array2, 4 * size, &list2);
    transformNx4ArrayIntoQueryList(resultArray, resultLength, &result);

    bool intersectionCorrect = checkIntersectionCorrectness(list1, list2,
                                                            result);
    printf("Intersection is correct at testSimpleIntersectNx4Array: %s\n",
                                         intersectionCorrect ?  "true":"false");
    delete array1;
    delete array2;
    delete resultArray;
  }
  // ___________________________________________________________________________
  void testArrayCorrectnessFunction()
  {
    size_t size = 100000;
    int* array1 = new int[4 * size];
    int* array2 = new int[4 * size];
    int* resultArray = new int[4 * size];

    // make first list
    size_t nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array1[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array1[4 * i + 3] = nextNumber;
    }

    // make 2nd list
    nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array2[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array2[4 * i + 3] = nextNumber;
    }

    // make intersection
    ArrayWithSize resultArrayWithSize;
    resultArrayWithSize.array = resultArray;

    simpleIntersectNx4Array(array1, size * 4, array2, size * 4,
                            &resultArrayWithSize);

    // print out test results
    size_t resultLength = resultArrayWithSize.size;
    printf("Result has length: %zu\n", resultLength);
    bool intersectionCorrect =
    checkArrayIntersectionCorrectness(array1, size * 4, array2, size * 4,
                                      &resultArrayWithSize);
    printf("Intersection is correct at testArrayCorrectnessFunction: %s\n",
                                         intersectionCorrect ?  "true":"false");
    // test with false resultarray:
    if (resultLength != 0)
    {
      resultArray[resultLength/2] = resultArray[resultLength/2] +
                                    rand_r(&seed) % 5 + 1;
      bool intersectionCorrect =
      checkArrayIntersectionCorrectness(array1, size * 4, array2, size * 4,
                                        &resultArrayWithSize);
      printf("false intersection is correct at testArrayCorrectnessFunction(should be false): %s\n", //NOLINT
                intersectionCorrect ?  "true":"false");
    }
    delete array1;
    delete array2;
    delete resultArray;
  }

  // ___________________________________________________________________________
  void makeSplitsAndTest(int* array1, size_t arrayLength1,
                         int* array2, size_t arrayLength2, size_t parts)
  {
    vector<size_t> splits1;
    vector<size_t> splits2;
    Timer timer;
    timer.start();
    findSplittingPoints(array1, arrayLength1, array2, arrayLength2,
                        parts, &splits1, &splits2);
    timer.stop();
    printf("split in %7.2f ms\n", timer.usecs() / static_cast<double>(1000.0));
    // debug output...

    for (size_t i = 1; i < parts; i++)
    {
      printf("Split Nr. %2zu: split1: %8zu split2: %8zu\n", i,
                                                        splits1[i], splits2[i]);
      printf("Array 1 um splitting punkt: %8d \033[1m%8d\033[0m %8d\n",
            array1[splits1[i] -4], array1[splits1[i]], array1[splits1[i] + 4]);
      printf("Array 2 um splitting punkt: %8d \033[1m%8d\033[0m %8d\n",
             array2[splits2[i] -4], array2[splits2[i]], array2[splits2[i] + 4]);
      bool splitCorrect = (array1[splits1[i]] >= array2[splits2[i]] &&
                           array1[splits1[i]] < array2[splits2[i] + 4]);
      printf("Split correct: %s\n\n",
             splitCorrect ?  "true":"\033[31mfalse\033[0m");
    }
  }

  // ___________________________________________________________________________
  // TODO(Robin): make gtest?
  void testFindSplittingPoints(PerformanceEvaluator* perfEvaluator)
  {
    size_t size = 10000000;
    int* array1 = new int[4 * size];
    int* array2 = new int[4 * size];
    // define amount of splittings
    size_t parts = 16;
    // make first list
    size_t nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array1[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array1[4 * i + 3] = nextNumber;
    }

    // make 2nd list
    nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array2[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array2[4 * i + 3] = nextNumber;
    }
    makeSplitsAndTest(array1, size * 4, array2, size * 4, parts);


    delete array1;
    delete array2;

    // now test with real words
    QueryResult* list1 = NULL;
    QueryResult* list2 = NULL;
    perfEvaluator->getListForQuery("rob*", list1);
    perfEvaluator->getListForQuery("sim*", list2);
    size_t sizeList1 = list1->_docIds.size() * 4;
    size_t sizeList2 = list2->_docIds.size() * 4;
    array1 = new int[sizeList1];
    array2 = new int[sizeList2];
    transformQueryListToNX4Arrays(*list1, array1);
    transformQueryListToNX4Arrays(*list2, array2);
    makeSplitsAndTest(array1, sizeList1, array2, sizeList2, parts);
    delete array1;
    delete array2;

    list1 = NULL;
    list2 = NULL;
    perfEvaluator->getListForQuery("a*", list1);
    perfEvaluator->getListForQuery("b*", list2);
    sizeList1 = list1->_docIds.size() * 4;
    sizeList2 = list2->_docIds.size() * 4;
    array1 = new int[sizeList1];
    array2 = new int[sizeList2];
    transformQueryListToNX4Arrays(*list1, array1);
    transformQueryListToNX4Arrays(*list2, array2);
    makeSplitsAndTest(array1, sizeList1, array2, sizeList2, parts);
    delete array1;
    delete array2;
  }
  // ___________________________________________________________________________
  void testBoostIntersect()
  {
      size_t size = 500000;
    int* array1 = new int[4 * size];
    int* array2 = new int[4 * size];

    int* resultArraySimpleIntersect = new int[size * 4];
    // define amount of splittings
    size_t parts = 12;
    // vector for all the results of the intersections of the parts...
    ArrayWithSize* results = new ArrayWithSize[parts];


    // make first list
    size_t nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array1[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array1[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array1[4 * i + 3] = nextNumber;
    }

    // make 2nd list
    nextNumber = 0;
    for (size_t i = 0; i < size; i++)
    {
      nextNumber += rand_r(&seed) % 3;
      array2[4 * i] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 1] = nextNumber;

      nextNumber += rand_r(&seed) % 3;
      array2[4 * i + 2] = nextNumber;

      // +1 because scores need to be positive
      nextNumber += rand_r(&seed) % 3 + 1;
      array2[4 * i + 3] = nextNumber;
    }

    Timer timer;
    size_t iterations = 5;
    size_t boostTime = 0;
    size_t concatenationTime = 0;
    for (size_t i = 0; i < iterations; i++)
    {
      timer.start();
      boostIntersection(array1, size * 4, array2, size * 4, parts, results);
      timer.stop();

      if (i >= 2) boostTime += timer.usecs();

      timer.start();

      // concatenate the results, calculate result size first......
      size_t resultArraySize = 0;
      for (size_t j = 0; j < parts; j++)
        resultArraySize += results[j].size;
      ArrayWithSize result;
      result.array = new int[resultArraySize];
      concatenateResults(results, parts, &result);
      timer.stop();
      if (i >= 2) concatenationTime += timer.usecs();
    }
    printf("boost intersection time: %7.2f\n",
           boostTime / (1000.0 * (iterations - 2)));
    printf("concatenation time:      %7.2f\n",
           concatenationTime/(1000.0 * (iterations - 2)));
    printf("total time:              %7.2f\n",
           (boostTime + concatenationTime) / (1000.0 * (iterations - 2)));



    size_t simpleTime = 0;
    Timer simpleTimer;
    ArrayWithSize resultForSimpleIntersect;
    resultForSimpleIntersect.array = resultArraySimpleIntersect;
    for (size_t i = 0; i < iterations; i++)
    {
      simpleTimer.reset();
      simpleTimer.start();
      simpleIntersectNx4Array(array1, size * 4, array2, size * 4,
                              &resultForSimpleIntersect);
      simpleTimer.stop();
      if (i >= 2) simpleTime += simpleTimer.usecs();
    }
    printf("simple intersection time: %7.2f\n",
           simpleTime / (1000.0 * (iterations - 2)));

    // concatenate the results...
    size_t resultArraySize = 0;
    for (size_t i = 0; i < parts; i++)
      resultArraySize += results[i].size;

    ArrayWithSize result;
    result.array = new int[resultArraySize];

    concatenateResults(results, parts, &result);

    // clean the old results up
    for (size_t i = 0; i < parts; i++)
      delete results[i].array;

    // compare for correctness
    bool intersectionCorrect = checkArrayIntersectionCorrectness(array1,
                                                                 size * 4,
                                                                 array2,
                                                                 size * 4,
                                                                 &result);

    printf("Intersection is correct at testBoostIntersect: %s\n",
                                         intersectionCorrect ?  "true":"false");
    printf("\n\n");
    delete array1;
    delete array2;
    delete resultArraySimpleIntersect;
    delete result.array;
    delete results;
  }

  // ___________________________END TESTS_______________________________________

  // performanceevaluate the threaded intersection algorithms
  // use performance evaluator to get the query lists etc.
  void perfThreadedIntersect(PerformanceEvaluator performanceEvaluator,
                             size_t threads)
  {
    // get all queries from the query file
    vector<vector<string> > queries;
    performanceEvaluator.getAllQueries(&queries);

    // construct statistics pool and timer for recording times...
    StatisticsPool statPool;
    Timer timer;
    bool correct;  // for check whether intersection is correct

    QueryResult* list1;
    QueryResult* list2;
    std::string query1;
    std::string query2;
    int* array1;
    int* array2;
    ArrayWithSize* results = new ArrayWithSize[threads];
    // save cursor position at start of line for progress bar positioning later
    printf("\n\033[s");
    // loop through all queries and intersect the lists
    for (size_t i = 0; i < queries.size(); i++)
    {
      list1 = NULL;
      list2 = NULL;
      query1 = queries[i][0];
      query2 = queries[i][1];
      performanceEvaluator.getListForQuery(query1, list1);
      performanceEvaluator.getListForQuery(query2, list2);

      // calc intersect gaps for statistics..
      double gapAverage1 = 0;
      double gapAverage2 = 0;
      double gapAverage3 = 0;
      performanceEvaluator.calculateIntersectGaps(*list1, *list2,
                                         gapAverage1, gapAverage2, gapAverage3);

      statPool.startMeasurement();
      statPool.setAttributeForThisMeasurement("Word 1", query1);
      statPool.setAttributeForThisMeasurement("Word 2", query2);
      statPool.setNrAttributeForThisMeasurement("Listsize1",
                                              list1->_docIds.size() / 1000000.0,
                                              3);
      statPool.setNrAttributeForThisMeasurement("Listsize2",
                                              list2->_docIds.size() / 1000000.0,
                                              3);
      // set dummy attribute so that attributes appear in proper order later..
      statPool.setNrAttributeForThisMeasurement("Result", -1, 3);
      statPool.setNrAttributeForThisMeasurement("Gap1", gapAverage1, 1);
      statPool.setNrAttributeForThisMeasurement("Gap2", gapAverage2, 1);
      statPool.setNrAttributeForThisMeasurement("Gap3", gapAverage3, 1);

      // transform lists into arrays
      size_t arrayLength1 = list1->_docIds.size() * 4;
      size_t arrayLength2 = list2->_docIds.size() * 4;
      array1 = new int[arrayLength1];
      array2 = new int[arrayLength2];

      transformQueryListToNX4Arrays(*list1, array1);
      transformQueryListToNX4Arrays(*list2, array2);

      // make intersection as many times as requested by the user...
      // record statistics with querystatistics object
      size_t repetitions = performanceEvaluator.getRepetitions();
      // draw progress bar for progress showing
      size_t progressBarWidth = 30;
      for (size_t loop = 0; loop < repetitions; loop++)
      {
      if (performanceEvaluator.getVerbosity() == 1)
        {
          // show which list is being intersected, always erase line
          // and jump back to saved cursor position
          // (see above before outer loop) at start of line
          // also jump up one line due to new line for progress bar!
          printf("\033[u\033[1A\033[2KIntersecting List %zu/%zu (loop %zu/%zu) "
                  "...", i + 1, queries.size(), loop + 1, repetitions);
          // draw first progressbar
          if (loop == 0 && i + 1 == 1)
          {
            printf("\n");
            displayProgressBar(0, progressBarWidth);
            // go up one line so progressbar is overwritten again..
            printf("\033[1A");
          }
          fflush(stdout);
        }
        // do intersection, measure time
        statPool.startRun();

        timer.start();
        pseudoThreadIntersection(array1, arrayLength1, array2, arrayLength2,
                                 threads, results);
        timer.stop();
        statPool.addTime("pseudothread", timer.usecs() / 1000.0);

        ArrayWithSize resultForSimple;
        resultForSimple.array = new int[arrayLength2];
        timer.start();
        simpleIntersectNx4Array(array1, arrayLength1, array2, arrayLength2,
                                &resultForSimple);
        timer.stop();

        statPool.addTime("simple1", timer.usecs() / 1000.0);
        correct = checkArrayIntersectionCorrectness(array1, arrayLength1,
                                                    array2, arrayLength2,
                                                    &resultForSimple);
        if (!correct) printf("intersection 1 not correct!\n");
        delete resultForSimple.array;

        timer.start();
        boostIntersection(array1, arrayLength1, array2, arrayLength2, threads,
                          results);
        timer.stop();
        size_t boostTime = timer.usecs();
        statPool.addTime("boost", boostTime / 1000.0);
        timer.start();
        // concatenate the results and measure time,
        // calculate result size first....
        ArrayWithSize result;
        size_t resultArraySize = 0;
        for (size_t j = 0; j < threads; j++)
        {
          resultArraySize += results[j].size;
        }
        result.array = new int[resultArraySize];
        concatenateResults(results, threads, &result);
        timer.stop();
        // delete used arrays
        for (size_t j = 0; j < threads; j++)
        {
          delete results[j].array;
        }
        statPool.setNrAttributeForThisMeasurement("Result",
                                                  resultArraySize / 4000000.0,
                                                  2);
        statPool.addTime("boostTotal", (boostTime + timer.usecs()) / 1000.0);

        correct = checkArrayIntersectionCorrectness(array1, arrayLength1,
                                                         array2, arrayLength2,
                                                         &result);
        if (!correct) printf("intersection 2 not correct!\n");



        timer.start();
        simpleIntersectNx4Array(array1, arrayLength1, array2, arrayLength2,
                                &result);
        timer.stop();
        statPool.addTime("simple2", timer.usecs() / 1000.0);

        correct = checkArrayIntersectionCorrectness(array1, arrayLength1,
                                                    array2, arrayLength2,
                                                    &result);
        if (!correct) printf("intersection 3 not correct!\n");


        // statPool.addTime("pseudoThread", timer.usecs()/1000.0);
        delete result.array;
                // print progress
        if (performanceEvaluator.getVerbosity() == 1)
        {
          printf("\n");
          // the progress in percent ie 0.1 for 10% progress
          // loop + 1 because loop starts with 0.....
          // this way it makes sense and its 1 at the end :)
          float progressPerCent = (static_cast<float>(
                                  ((i * repetitions) + loop + 1)))
                                  /
                                  (static_cast<float> (
                                  queries.size() * repetitions));
          displayProgressBar(progressPerCent, progressBarWidth);
        }
      }
      delete array1;
      delete array2;
    }
    delete results;
    printf("\n\n");  // done :)
    // write statistics to correct output file
    statPool.writeStatisticsToFile(
    performanceEvaluator.getStatBaseName() + ".perf-statistics");
    if (performanceEvaluator.getVerbosity() >= 1)
      statPool.printStatistics();
  }
}
// _____________________________________________________________________________
// just print the usage, taken from performance evaluator
// and added threads option
void printUsage()
  {
    cout << endl
         << "USAGE : ./CompleterBase.IntersectThreadPerf [options]"
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
         << endl
         << " --(t)hreads:                   Number of threads."
         << " How many threads the algorithm will use"
         << " (default: 10)."
         << endl
         << endl;
  }
// _____________________________________________________________________________
// parse the command line arguments, create a new Performance Evaluator
// while doing this
void parseCommandLineArguments(int argc, char** argv,
    IntersectPerformanceEvaluation::PerformanceEvaluator*& performanceEvaluator,
        size_t& threads)
{
  // FIRST PART:
  // read the values


  // Index file atributes
  string indexBaseName = "";
  string indexPath = "";
  string indexTarget = "";

  // Query file atributes
  string queryBaseName = "";
  string queryPath = "";
  string queryTarget = "";

  // Statistics file atributes
  string statTarget = "";

  // standard verbosity level is 1 for just progress output
  int verbosity = 1;
  // standard looping level is 1 for repeating of output
  int loop = 1;
  // standard amount of threads..
  threads = 10;
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
      {"threads", required_argument, 0, 't'},
      {0, 0, 0, 0}
    };

    optChr = getopt_long(argc, argv, "hi:q:s:v:r:t:", longOptions,
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
      case 't':
        threads = atoi(optarg);
        break;
      default:
        cerr << "unknown option: " << optChr << " ??" << endl;
        printUsage();
        exit(1);
        break;
    }
  }

  if ((indexPath == "") || (verbosity <= - 1) || (loop <= 0))
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
  performanceEvaluator =
          new IntersectPerformanceEvaluation::PerformanceEvaluator(indexTarget,
                                                  indexBaseName,
                                                  queryBaseName,
                                                  queryTarget,
                                                  statTarget,
                                                  verbosity,
                                                  loop);
}
// _____________________________________________________________________________
int main(int argc, char **argv)
{
  // set random seed
  srand(time(NULL));
  seed = time(NULL);
  // TODO(Robin): Make into own test-file
  // test the transform functions for correctness
  IntersectPerformanceEvaluation::testTransFormFunctions();
  // test wheter correctness function works correctly
  IntersectPerformanceEvaluation::testCorrectnessFunction();
  // test if simple intersect is producing correct intersection
  IntersectPerformanceEvaluation::testSimpleIntersectNx4Array();
  // test if array correctness function works correctly
  IntersectPerformanceEvaluation::testArrayCorrectnessFunction();


  // start performance evaluator with given arguments
  IntersectPerformanceEvaluation::PerformanceEvaluator* performanceEvaluator =
                                                        NULL;
  size_t threads = 0;
  parseCommandLineArguments(argc, argv, performanceEvaluator, threads);

  // //IntersectPerformanceEvaluation::testFindSplittingPoints(&perfEvaluator);
  IntersectPerformanceEvaluation::testBoostIntersect();
  IntersectPerformanceEvaluation::perfThreadedIntersect(*performanceEvaluator,
                                                        threads);
  delete performanceEvaluator;
}
