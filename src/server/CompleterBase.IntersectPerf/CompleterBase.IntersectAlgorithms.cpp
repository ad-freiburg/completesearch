// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#include <algorithm>
#include "./CompleterBase.IntersectAlgorithms.h"
// MY STUFF, JUST AN IDEA!! (ROBIN)
// : INTERSECTION ALGORITHMS FOR PERF TEST AND IMPROVEMENT (ROBINS)



// _____________________________________________________________________________
// interPolAlgorithm: when ids are not equal in list1[i] and list2[j], try to
// find out how far you have to jump, jump half that much and if you are still
// smaller, keep going, else just jump by 1...
void interPolAlgorithm(const QueryResult& list1,
                                            const QueryResult& list2,
                                                  QueryResult& result)
{
  const DocList&      docIds1    = list1._docIds;
  const ScoreList&    scores1    = list1._scores;
  const DocList&      docIds2    = list2._docIds;
  const WordList&     wordIds2   = list2._wordIds;
  const PositionList& positions2 = list2._positions;
  const ScoreList&    scores2    = list2._scores;
        DocList&      docIds3    = result._docIds;
        WordList&     wordIds3   = result._wordIds;
        PositionList& positions3 = result._positions;
        ScoreList&    scores3    = result._scores;
  unsigned int len1 = docIds1.size();
  unsigned int len2 = docIds2.size();
  WordId _lastBestMatchWordId;
  bool needToCheckPositions = false;
  unsigned int i = 0;
  unsigned int j = 0;
  // remember max doc ids for interpolation search
  size_t maxDocID1 = docIds1[docIds1.size() - 1] + 1;
  size_t maxDocID2 = docIds2[docIds2.size() - 1] + 1;
  // for the difference between the doc ids of list 1 and list 2

  int docIdDifference = 0;
  // the index where probably the two lists match...
  unsigned int predictedIndex = 0;
  unsigned int tempStepWidth1 = maxDocID1 / docIds1.size();
  if (tempStepWidth1 == 0)
    tempStepWidth1 = 1;
  const unsigned int stepWidth1 = tempStepWidth1;
  printf("maxDocID1: %zu, docID1Size: %zu\n", maxDocID1, docIds1.size());
  printf("maxDocID2: %zu, docID2Size: %zu\n\n", maxDocID2, docIds2.size());
  unsigned int tempStepWidth2 = maxDocID2 / docIds2.size();
  if (tempStepWidth2 == 0)
    tempStepWidth2 = 1;
  const unsigned int stepWidth2 = tempStepWidth2;
  // for testing..
  size_t jumpattempts = 0;
  size_t jumps = 0;
  while (true)
  {
    if (j == len2) break;
    docIdDifference = docIds2[j] - docIds1[i];
    while (docIdDifference > 0)
    {
      ++i;
      if (docIdDifference > 10)
      {
        jumpattempts++;
        // stepwidthis always 1 for big lists, so we ignore stepwidth :)
        i = i + docIdDifference;
        if (i >= len1)
          i = len1-1;
        /* old idea: jump conditionallyonly if safe..
        if(predictedIndex < len1 && docIds1[predictedIndex] < docIds2[j])
        {
          i = predictedIndex;
          jumps++;
        }*/
        /* new idea: always jump, then loop back if youve jumped too far*/
        while (docIds2[j] <= docIds1[i])
          i--;
        i++;
        docIdDifference = docIds2[j] - docIds1[i];
      }
      if (i == len1) break;
      docIdDifference = docIds2[j] - docIds1[i];
    }
    if (i == len1) break;

    while (docIdDifference < 0)
    {
      ++j;
      if (j == len2) break;
      docIdDifference = docIds2[j] - docIds1[i];
    }
    if (needToCheckPositions == false)
    {
      if (1 == 1)
      {
        while (j < len2 && docIds2[j] == docIds1[i])
        {
          docIds3. push_back(docIds2[j]);
          wordIds3. push_back(wordIds2[j]);
          positions3.push_back(positions2[j]);
          scores3. push_back(scores1[i] + scores2[j] +
                  (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
          ++j;
        }
      }
    }
    if (j == len2) break;
  }
  printf("jumpattempts: %zu, jumps: %zu\n\n", jumpattempts, jumps);
}


// END MY STUFF
namespace IntersectPerformanceEvaluation
{
  void simpleIntersectNx4Array(int* inputList1, size_t arrayLength1,
                               int* inputList2, size_t arrayLength2,
                               ArrayWithSize* result)
  {
      int* resultArray = result->array;
      size_t i = 0;
      size_t j = 0;
      size_t k = 0;
      size_t elementsInList1 = arrayLength1 / 4;
      size_t elementsInList2 = arrayLength2 / 4;

      while (true)
      {
        if (j == elementsInList2) break;
        while (i < elementsInList1 && inputList1[4 * i] < inputList2[4 * j])
        {
          ++i;
        }
        if (i == elementsInList1) break;
        while (j < elementsInList2 && inputList2[4 * j] < inputList1[4 * i])
        {
          ++j;
        }
        if (j == elementsInList2) break;
        if (inputList2[4 * j] == inputList1[4 * i])
        {
            resultArray[4 * k] = inputList2[4 * j];
            resultArray[4 * k + 1] = inputList2[4 * j + 1];
            resultArray[4 * k + 2] = inputList2[4 * j + 2];
            // scores are always sum form list 1 and list 2
            resultArray[4 * k + 3] = inputList2[4 * j + 3] +
                                     inputList1[4 * i + 3];
            k++;
            j++;
        }
      }
      // set result array size...
      result->size = 4 * k;
  }

  // ___________________________________________________________________________
  void simpleIntersectNx4ArrayInLimits(int* inputList1,
                                       size_t from1, size_t to1,
                                       int* inputList2,
                                       size_t from2, size_t to2,
                                       ArrayWithSize* result)
  {
      int* resultArray = result->array;
      // calculate correct startdocids and correct last doc id!
      size_t i = from1 / 4;
      size_t j = from2 / 4;
      size_t k = 0;
      // +1 to also take last element in loop
      size_t lastElementInList1 = (to1 / 4) + 1;
      size_t lastElementInList2 = (to2 / 4) + 1;

      while (true)
      {
        if (j == lastElementInList2) break;
        while (i < lastElementInList1 && inputList1[4 * i] < inputList2[4 * j])
        {
          i++;
        }
        if (i == lastElementInList1) break;
        while (j < lastElementInList2 && inputList2[4 * j] < inputList1[4 * i])
        {
          j++;
        }
        if (j == lastElementInList2) break;
        if (inputList2[4 * j] == inputList1[4 * i])
        {
          resultArray[4 * k]     = inputList2[4 * j];
          resultArray[4 * k + 1] = inputList2[4 * j + 1];
          resultArray[4 * k + 2] = inputList2[4 * j + 2];
          // scores are always sum form list 1 and list 2
          resultArray[4 * k + 3] = inputList2[4 * j + 3] +
                                   inputList1[4 * i + 3];
          k++;
          j++;
        }
      }

      // set result array size...
      result->size = 4 * k;
  }
  // ___________________________________________________________________________
  void* simpleIntersectNx4ArrayInLimitsPthread(void* par)
  {
      ThreadParam *param = reinterpret_cast<ThreadParam*>(par);
      // //cout << "vorher ="<<param->result->size << endl<< flush;
      int* resultArray = param->result->array;
      // calculate correct startdocids and correct last doc id!
      size_t i = param->from1 / 4;
      size_t j = param->from2 / 4;
      size_t k = 0;
      // +1 to also take last element in loop
      size_t lastElementInList1 = (param->to1 / 4) + 1;
      size_t lastElementInList2 = (param->to2 / 4) + 1;

      while (true)
      {
        if (j == lastElementInList2) break;
        while (i < lastElementInList1 &&
               param->inputList1[4 * i] < param->inputList2[4 * j])
        {
          i++;
        }
        if (i == lastElementInList1) break;
        while (j < lastElementInList2 &&
               param->inputList2[4 * j] < param->inputList1[4 * i])
        {
          j++;
        }
        if (j == lastElementInList2) break;
        if (param->inputList2[4 * j] == param->inputList1[4 * i])
        {
          resultArray[4 * k] = param->inputList2[4 * j];
          resultArray[4 * k + 1] = param->inputList2[4 * j + 1];
          resultArray[4 * k + 2] = param->inputList2[4 * j + 2];
          // scores are always sum form list 1 and list 2
          resultArray[4 * k + 3] = param->inputList2[4 * j + 3] +
                                   param->inputList1[4 * i + 3];
          k++;
          j++;
        }
      }
      // set result array size...
      param->result->size = 4 * k;
      // //cout << "start " << param->from1/4 << " end " << param->to1/4;
      // //cout << "danach ="<<param->result->size << endl<< flush;
  }

  // ___________________________________________________________________________
  // Patterlgorithmus: Native algo without special cases
  void patternAlgorithmIntersect(const QueryResult& list1,
                                 const QueryResult& list2,
                                       QueryResult& result)
  {
    const DocList&      docIds1    = list1._docIds;
    const ScoreList&    scores1    = list1._scores;
    const DocList&      docIds2    = list2._docIds;
    const WordList&     wordIds2   = list2._wordIds;
    const PositionList& positions2 = list2._positions;
    const ScoreList&    scores2    = list2._scores;
          DocList&      docIds3    = result._docIds;
          WordList&     wordIds3   = result._wordIds;
          PositionList& positions3 = result._positions;
          ScoreList&    scores3    = result._scores;
    unsigned int len1 = docIds1.size();
    unsigned int len2 = docIds2.size();
    WordId _lastBestMatchWordId;
    unsigned int i = 0;
    unsigned int j = 0;
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && docIds1[i] < docIds2[j])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && docIds2[j] < docIds1[i])
      {
        ++j;
      }
      if (j == len2) break;
        if (docIds2[j] == docIds1[i])
        {
          docIds3. push_back(docIds2[j]);
          wordIds3. push_back(wordIds2[j]);
          positions3.push_back(positions2[j]);
          scores3. push_back(scores1[i] + scores2[j] +
                  (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
          ++j;
        }
    }
  }

  // ___________________________________________________________________________
  // Patterlgorithmus: Naitive algo without special cases
  void patternAlgorithmIntersectMod1(const QueryResult& list1,
                                              const QueryResult& list2,
                                                    QueryResult& result)
  {
    const DocList&      docIds1    = list1._docIds;
    const ScoreList&    scores1    = list1._scores;
    const DocList&      docIds2    = list2._docIds;
    const WordList&     wordIds2   = list2._wordIds;
    const PositionList& positions2 = list2._positions;
    const ScoreList&    scores2    = list2._scores;
          DocList&      docIds3    = result._docIds;
          WordList&     wordIds3   = result._wordIds;
          PositionList& positions3 = result._positions;
          ScoreList&    scores3    = result._scores;
    unsigned int len1 = docIds1.size();
    unsigned int len2 = docIds2.size();
    WordId _lastBestMatchWordId;
    // TODO(simon): Vergleich für j = 0 i=0
    unsigned int i = 1;
    unsigned int j = 1;
    while (true)
    {
      if (j >= len2) break;
      while (i < len1 && docIds1[i] < docIds2[j])
      {
        i = i+2;
      }
      if (i >= len1) break;
      if (docIds1[i-1] >= docIds2[j])
        i--;
      while (j < len2 && docIds2[j] < docIds1[i])
      {
        j = j+2;
      }
      if (j >= len2) break;
      if (docIds2[j-1] >= docIds1[i])
        j--;
      while (j < len2 && docIds2[j] == docIds1[i])
      {
        docIds3. push_back(docIds2[j]);
        wordIds3. push_back(wordIds2[j]);
        positions3.push_back(positions2[j]);
        scores3. push_back(scores1[i] + scores2[j] +
                (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
        ++j;
      }
      if (j >= len2) break;
    }
  }
  // ___________________________________________________________________________
  // Patterlgorithmus: Naitive algo without special cases
  void patternAlgorithmIntersectMod2(const QueryResult& list1,
                                              const QueryResult& list2,
                                                    QueryResult& result)
  {
    const DocList&      docIds1    = list1._docIds;
    const ScoreList&    scores1    = list1._scores;
    const DocList&      docIds2    = list2._docIds;
    const WordList&     wordIds2   = list2._wordIds;
    const PositionList& positions2 = list2._positions;
    const ScoreList&    scores2    = list2._scores;
          DocList&      docIds3    = result._docIds;
          WordList&     wordIds3   = result._wordIds;
          PositionList& positions3 = result._positions;
          ScoreList&    scores3    = result._scores;
    unsigned int len1 = docIds1.size();
    unsigned int len2 = docIds2.size();
    WordId _lastBestMatchWordId;
    // TODO(simon): Vergleich für j = 0 i=0
    unsigned int i = 1;
    unsigned int j = 1;
    unsigned int gapI = max(1, static_cast<int>((len1/len2)/1.5));
    unsigned int gapJ = max(1, static_cast<int>((len2/len1)/1.5));
    // //cout << "gapI =" << gapI << " gapJ=" << gapJ << endl;
    while (docIds2[j] < docIds1[i])
      j++;
    while (docIds2[j] == docIds1[i])
      {
        docIds3. push_back(docIds2[j]);
        wordIds3. push_back(wordIds2[j]);
        positions3.push_back(positions2[j]);
        scores3. push_back(scores1[i] + scores2[j] +
                (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
        ++j;
      }

    while (true)
    {
      if (j >= len2) break;
      while (i < len1 && docIds1[i] < docIds2[j])
      {
        i = i+gapI;
      }
      if (i >= len1) break;
      while (docIds1[i-1] >= docIds2[j])
        i--;
      while (j < len2 && docIds2[j] < docIds1[i])
      {
        j = j+gapJ;
      }
      if (j >= len2) break;
      while (docIds2[j-1] >= docIds1[i])
        j--;
      while (j < len2 && docIds2[j] == docIds1[i])
      {
        docIds3. push_back(docIds2[j]);
        wordIds3. push_back(wordIds2[j]);
        positions3.push_back(positions2[j]);
        scores3. push_back(scores1[i] + scores2[j] +
                (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
        ++j;
      }
      if (j >= len2) break;
    }
  }

  // ___________________________________________________________________________
  // Patterlgorithmus: Naitive algo without special cases
  void patternAlgorithmUnion(const QueryResult& list1,
                                                   const QueryResult& list2,
                                                   QueryResult& result)
  {
    const DocList&      docIds1    = list1._docIds;
    const DocList&      docIds2    = list2._docIds;
    const WordList&     wordIds2   = list2._wordIds;
    const WordList&     wordIds1   = list1._wordIds;
    const PositionList& positions2 = list2._positions;
    const PositionList& positions1 = list1._positions;
    const ScoreList&    scores2    = list2._scores;
    const ScoreList&    scores1    = list1._scores;
          DocList&      docIds3    = result._docIds;
          WordList&     wordIds3   = result._wordIds;
          PositionList& positions3 = result._positions;
          ScoreList&    scores3    = result._scores;
    unsigned int len1 = docIds1.size();
    unsigned int len2 = docIds2.size();
    WordId _lastBestMatchWordId;
    unsigned int i = 0;
    unsigned int j = 0;
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && docIds1[i] <= docIds2[j])
      {
        docIds3. push_back(docIds1[i]);
        wordIds3. push_back(wordIds1[i]);
        positions3.push_back(positions1[i]);
        scores3. push_back(scores1[i] + scores2[j] +
                (wordIds1[i] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && docIds2[j] <= docIds1[i])
      {
        docIds3. push_back(docIds2[j]);
        wordIds3. push_back(wordIds2[j]);
        positions3.push_back(positions2[j]);
        scores3. push_back(scores1[i] + scores2[j] +
                (wordIds2[j] == _lastBestMatchWordId ? BEST_MATCH_BONUS : 0));
        ++j;
      }
    }
  }

  // check whether the given results of an intersection are correct!
  bool checkIntersectionCorrectness(const QueryResult& list1,
                                    const QueryResult& list2,
                                    const QueryResult& result)
  {
    // just use simple algorithm, assumes simple algorithm gives correct result
    QueryResult correctResult;
    patternAlgorithmIntersect(list1, list2, correctResult);
    bool intersectionCorrect = true;
    const DocList&      docIds    = result._docIds;
    const WordList&     wordIds   = result._wordIds;
    const PositionList& positions = result._positions;
    const ScoreList&    scores    = result._scores;
    const DocList&      docIdsCorrect    = correctResult._docIds;
    const WordList&     wordIdsCorrect   = correctResult._wordIds;
    const PositionList& positionsCorrect = correctResult._positions;
    const ScoreList&    scoresCorrect    = correctResult._scores;
    if (docIds.size()!= docIdsCorrect.size())
    {
      intersectionCorrect = false;
      printf("sizes different! correct: %zu, actual: %zu\n",
                                                           docIdsCorrect.size(),
                                                           docIds.size());
    }
    size_t length = docIds.size();
    for (size_t i = 0; i < length; i++)
    {
      if (docIdsCorrect[i] != docIds[i])
      {
        printf("Fehler an Stelle %zu, correctDocId: %d, actualDocId: %d\n", i,
                                                               docIdsCorrect[i],
                                                               docIds[i]);
        intersectionCorrect = false;
      }
      if (wordIdsCorrect[i] != wordIds[i])
      {
        printf("Fehler an Stelle %zu, correctwordId: %d, actualwordId: %d\n", i,
                                                              wordIdsCorrect[i],
                                                              wordIds[i]);
        intersectionCorrect = false;
      }
      if (positionsCorrect[i] != positions[i])
      {
        printf("Fehler an Stelle %zu, correctposition: %d,actualposition: %d\n",
                                                            i,
                                                            positionsCorrect[i],
                                                            positions[i]);
        intersectionCorrect = false;
      }
      if (scoresCorrect[i] != scores[i])
      {
        printf("Fehler an Stelle %zu, correctscores: %d, actualscores: %d\n", i,
                                                               scoresCorrect[i],
                                                               scores[i]);
        intersectionCorrect = false;
      }
    }
    return intersectionCorrect;
  }
    // check whether the given results of an intersection are correct!
  bool checkArrayIntersectionCorrectness(int* inputList1, size_t arrayLength1,
                                         int* inputList2, size_t arrayLength2,
                                         ArrayWithSize* result)
  {
    bool intersectionCorrect = true;
    int* resultArray = result->array;

    ArrayWithSize correctResult;
    correctResult.array = new int[arrayLength2];
    int* correctResultArray = correctResult.array;

    // intersect with tested algorithm
    simpleIntersectNx4Array(inputList1, arrayLength1, inputList2, arrayLength2,
                            &correctResult);

    // compare results
    size_t i = 0;
    if (correctResult.size != result->size)
    {
      printf("Sizes of Result Arrays different!: correct: %zu, actual: %zu\n",
                                                        correctResult.size,
                                                        result->size);
      intersectionCorrect = false;
    }
    while (i < result->size)
    {
      if (resultArray[i] != correctResultArray[i])
      {
        intersectionCorrect = false;
        printf("numbers different at index %zu! correct: %d, actual: %d\n",
                                                          i,
                                                          correctResultArray[i],
                                                          resultArray[i]);
      }
      i++;
    }
    delete correctResult.array;
    return intersectionCorrect;
  }
  void concatenateResults(ArrayWithSize* results, size_t nrOfResults,
                          ArrayWithSize* result)
  {
    int* resultArray = result->array;

    int* partialResultArray;
    size_t partialResultSize;

    size_t resultIndex = 0;
    for (size_t i = 0; i < nrOfResults; i++)
    {
      partialResultArray = results[i].array;
      partialResultSize = results[i].size;
      for (size_t j = 0; j < partialResultSize; j++)
      {
        resultArray[resultIndex] = partialResultArray[j];
        resultIndex++;
      }
    }
    result->size = resultIndex;
  }
}
