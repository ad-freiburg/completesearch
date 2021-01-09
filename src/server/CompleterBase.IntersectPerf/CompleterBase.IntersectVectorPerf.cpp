// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#include <gtest/gtest.h>
#include <getopt.h>
#include <math.h>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>
#include "../Globals.h"

unsigned seed;
class Quatro
{
 public:
  Quatro(DocId docId, WordId wordId, Score score, Position posId);
  Quatro();
  DocId _docId;
  WordId _wordId;
  Score _score;
  Position _posId;
};
struct QuatroStruct
{
  DocId _docId;
  WordId _wordId;
  Score _score;
  Position _posId;
};
// _____________________________________________________________________________
Quatro::Quatro() {}
// _____________________________________________________________________________
Quatro::Quatro(DocId docId, WordId wordId, Score score, Position posId)
{
  _docId = docId;
  _wordId = wordId;
  _score = score;
  _posId = posId;
}
// _____________________________________________________________________________
// Print Usage
void printUsage()
{
  cout << endl
       << "USAGE : ./CompleterBase.IntersectVectorPerf [options]"
       << endl
       << endl
       << "Options are:"
       << endl
       << endl
       << " --line-(n)umber:               Number of Entries"
          " (default: 1000000)"
       << endl
       << endl
       << " --(r)epeat:                   Level of repeating."
       << " How often each query will be tested"
       << " (default: 1)."
       << endl
       << endl;
}
// _____________________________________________________________________________
void SimpleIntersect(vector<pair <pair <DocId, WordId>,
                     pair<Score, Position> > >* globalDataPair,
                     vector<Quatro>* globalDataClass,
                     vector<QuatroStruct>* globalDataStruct,
                     vector<DocId>* docIds,
                     vector<WordId>* wordIds,
                     vector<Score>* scoreIds,
                     vector<Position>* posId,
                     int* array_4xN,
                     int* array_Nx4,
                     size_t num,
                     bool save,
                     int loop)
{
  vector<pair <pair <DocId, WordId>, pair<Score, Position> > > globalDataPair1;
  vector<Quatro> globalDataClass1;
  vector<QuatroStruct> globalDataStruct1;
  vector<DocId> docIds1;
  vector<WordId> wordIds1;
  vector<Score> scoreIds1;
  vector<Position> posId1;
  vector<pair <pair <DocId, WordId>, pair<Score, Position> > > globalDataPair2;
  vector<Quatro> globalDataClass2;
  vector<QuatroStruct> globalDataStruct2;
  vector<DocId> docIds2;
  vector<WordId> wordIds2;
  vector<Score> scoreIds2;
  vector<Position> posId2;
  int* array_4xN1 = new int[4*num];
  int* array_4xN2 = new int[4*num];
  int* array_Nx41 = new int[4*num];
  int* array_Nx42 = new int[4*num];

  // reserve place for result lists
  globalDataClass2.reserve(num);
  globalDataStruct2.reserve(num);
  globalDataPair2.reserve(num);
  docIds2.reserve(num);
  wordIds2.reserve(num);
  scoreIds2.reserve(num);
  posId2.reserve(num);
  // Create second copy
  for (size_t i; i < num; i++)
  {
    docIds1.push_back(docIds->at(i));
    wordIds1.push_back(wordIds->at(i));
    scoreIds1.push_back(scoreIds->at(i));
    posId1.push_back(posId->at(i));
    globalDataPair1.push_back(globalDataPair->at(i));
    globalDataClass1.push_back(globalDataClass->at(i));
    globalDataStruct1.push_back(globalDataStruct->at(i));
    array_4xN1[i] = array_4xN[i];
    array_4xN1[i + num] = array_4xN[i + num];
    array_4xN1[i + 2*num] = array_4xN[i + 2*num];
    array_4xN1[i + 3*num] = array_4xN[i + 3*num];
    array_Nx41[4*i] = array_Nx4[4*i];
    array_Nx41[4*i+1] = array_Nx4[4*i+1];
    array_Nx41[4*i+2] = array_Nx4[4*i+2];
    array_Nx41[4*i+3] = array_Nx4[4*i+3];
  }
  unsigned int len1 = docIds->size();
  unsigned int len2 = docIds1.size();
  // Test separate
  cout << "Intersect separate based Vector... ";
  off_t uSecTime = 0;
  unsigned int i = 0;
  unsigned int j = 0;
  vector<DocId>& docIdsC = *docIds;
  vector<WordId>& wordIdsC = *wordIds;
  vector<Score>& scoreIdsC = *scoreIds;
  vector<Position>& posIdC = *posId;
  Timer timer;
  posId2.resize(len2, 0);
  docIds2.resize(len2, 0);
  wordIds2.resize(len2, 0);
  scoreIds2.resize(len2, 0);
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    size_t k = 0;
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && docIdsC[i] < docIds1[j])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && docIds1[j] < docIdsC[i])
      {
        ++j;
      }
      if (docIds1[j] == docIdsC[i])
      {
        if (save)
        {
         docIds2[k] = docIds1[j];
         wordIds2[k] = wordIds1[j];
         posId2[k] = posId1[j];
         scoreIds2[k] = scoreIds1[j];
        }
        ++k;
        ++j;
      }
    }
    timer.stop();
    if (l>= 2)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * (loop - 2));
  cout << mSecTime << endl;

  // Test Array[4xN]
  cout << "Intersect Array[4xN]...            ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    size_t k = 0;
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && array_4xN[i] < array_4xN1[j])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && array_4xN1[j] < array_4xN[i])
      {
        ++j;
      }
      if (array_4xN1[j] == array_4xN[i])
      {
        if (save)
        {
          array_4xN2[k] = array_4xN1[j];
          array_4xN2[k + num] = array_4xN1[j + num];
          array_4xN2[k + 2*num] = array_4xN1[j + 2*num];
          array_4xN2[k + 3*num] = array_4xN1[j + 3*num];
        }
        ++k;
        ++j;
      }
    }
    timer.stop();
    if (l>= 2)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 2));
  cout << mSecTime << endl;

  // Test Array[Nx4]
  cout << "Intersect Array[Nx4]...            ";
  uSecTime = 0;
  size_t k;
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    k = 0;
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && array_Nx4[4*i] < array_Nx41[4*j])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && array_Nx41[4*j] < array_Nx4[4*i])
      {
        ++j;
      }
      if (array_Nx41[4*j] == array_Nx4[4*i])
      {
        if (save)
        {
          array_Nx42[4*k] = array_Nx41[4*j];
          array_Nx42[4*k + 1] = array_Nx41[4*j + 1];
          array_Nx42[4*k + 2] = array_Nx41[4*j + 2];
          array_Nx42[4*k + 3] = array_Nx41[4*j + 3];
        }
        ++j;
        k++;
      }
    }
    timer.stop();
    if (l >= 2)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 2));
  cout << mSecTime << endl;

  // Test SEPARATE
  vector<QuatroStruct> &globalDataStructC = *globalDataStruct;
  cout << "Intersect struct based Vector...   ";
  uSecTime = 0;
  globalDataStruct2.resize(globalDataStruct1.size(), {0, 0, 0, 0});
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    size_t k = 0;
    // globalDataStruct2.clear();
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1
          && globalDataStructC[i]._docId < globalDataStruct1[j]._docId)
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2
          && globalDataStruct1[j]._docId < globalDataStructC[i]._docId)
      {
        ++j;
      }
      if (globalDataStruct1[j]._docId == globalDataStructC[i]._docId)
      {
        if (save)
        {
          globalDataStruct2[k]._docId = globalDataStruct1[j]._docId;
          globalDataStruct2[k]._posId = globalDataStruct1[j]._posId;
          globalDataStruct2[k]._score = globalDataStruct1[j]._score;
          globalDataStruct2[k]._wordId = globalDataStruct1[j]._wordId;
        }
        ++k;
        ++j;
      }
    }
    timer.stop();
    if (l >= 2)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 2));
  cout << mSecTime << endl;

  printf("k is: %zu\n", k);
  size_t sum = globalDataStruct2.size()
             + docIds2.size()
             + wordIds2.size()
             + scoreIds2.size()
             + posId2.size()+ array_Nx42[4 * (k - 1)];
  cout << "CheckSum :" << sum << endl;
}
// _____________________________________________________________________________
void SimpleIntersectFull(vector<pair <pair <DocId, WordId>,
                                      pair<Score, Position> > >* globalDataPair,
              vector<Quatro>* globalDataClass,
              vector<QuatroStruct>* globalDataStruct,
              vector<DocId>* docIds,
              vector<WordId>* wordIds,
              vector<Score>* scoreIds,
              vector<Position>* posId,
              int* array_4xN,
              int* array_Nx4,
              size_t num,
              int loop)
{
  vector<pair <pair <DocId, WordId>, pair<Score, Position> > > globalDataPair1;
  vector<Quatro> globalDataClass1;
  vector<QuatroStruct> globalDataStruct1;
  vector<DocId> docIds1;
  vector<WordId> wordIds1;
  vector<Score> scoreIds1;
  vector<Position> posId1;
  vector<pair <pair <DocId, WordId>, pair<Score, Position> > > globalDataPair2;
  vector<Quatro> globalDataClass2;
  vector<QuatroStruct> globalDataStruct2;
  vector<DocId> docIds2;
  vector<WordId> wordIds2;
  vector<Score> scoreIds2;
  vector<Position> posId2;
  int* array_4xN1 = new int[4*num];
  int* array_4xN2 = new int[4*num];
  int* array_Nx41 = new int[4*num];
  int* array_Nx42 = new int[4*num];

  // reserve place for result lists
  globalDataClass2.reserve(num);
  globalDataStruct2.reserve(num);
  globalDataPair2.reserve(num);
  docIds2.reserve(num);
  wordIds2.reserve(num);
  scoreIds2.reserve(num);
  posId2.reserve(num);

  // Create second copy
  for (size_t i; i < num; i++)
  {
    globalDataPair1.push_back(globalDataPair->at(i));
    globalDataClass1.push_back(globalDataClass->at(i));
    globalDataStruct1.push_back(globalDataStruct->at(i));
    docIds1.push_back(docIds->at(i));
    wordIds1.push_back(wordIds->at(i));
    scoreIds1.push_back(scoreIds->at(i));
    posId1.push_back(posId->at(i));
    array_4xN1[i] = array_4xN[i];
    array_4xN1[i + num] = array_4xN[i + num];
    array_4xN1[i + 2*num] = array_4xN[i + 2*num];
    array_4xN1[i + 3*num] = array_4xN[i + 3*num];
    array_Nx41[4*i] = array_Nx4[4*i];
    array_Nx41[4*i+1] = array_Nx4[4*i+1];
    array_Nx41[4*i+2] = array_Nx4[4*i+2];
    array_Nx41[4*i+3] = array_Nx4[4*i+3];
  }
  unsigned int len1 = docIds->size();
  unsigned int len2 = docIds1.size();
  // Test separate
  cout << "Intersect separate based Vector... ";
  off_t uSecTime = 0;
  unsigned int i = 0;
  unsigned int j = 0;
  Timer timer;
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    docIds2.clear();
    wordIds2.clear();
    posId2.clear();
    scoreIds2.clear();
    size_t k = 0;
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && docIds->at(i) < docIds1[j]
              && wordIds->at(i) < wordIds1[j]
              && scoreIds->at(i) < scoreIds1[j]
              && posId->at(i) < posId1[j])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && docIds1[j] < docIds->at(i)
              && wordIds1[j] < wordIds->at(i)
              && scoreIds1[j] < scoreIds->at(i)
              && posId1[j] < posId->at(i))
      {
        ++j;
      }
      if (docIds1[j] == docIds->at(i)
              && wordIds1[j] == wordIds->at(i)
              && scoreIds1[j] == scoreIds->at(i)
              && posId1[j] == posId->at(i))
      {
        docIds2.push_back(docIds1[j]);
        wordIds2.push_back(wordIds1[j]);
        posId2.push_back(posId1[j]);
        scoreIds2.push_back(scoreIds1[j]);
        ++j;
        ++i;
      }
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  // Test class
  cout << "Intersect Struct based Vector...   ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    size_t k = 0;
    globalDataStruct2.clear();
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && globalDataStruct->at(i)._docId
                       < globalDataStruct1[j]._docId
              && globalDataStruct->at(i)._posId < globalDataStruct1[j]._posId
              && globalDataStruct->at(i)._score < globalDataStruct1[j]._score
              && globalDataStruct->at(i)._wordId < globalDataStruct1[j]._wordId)
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && globalDataStruct1[j]._docId
                       < globalDataStruct->at(i)._docId
              && globalDataStruct1[j]._posId < globalDataStruct->at(i)._posId
              && globalDataStruct1[j]._score < globalDataStruct->at(i)._score
              && globalDataStruct1[j]._wordId < globalDataStruct->at(i)._wordId)
      {
        ++j;
      }
      if (globalDataStruct1[j]._docId == globalDataStruct->at(i)._docId
             && globalDataStruct1[j]._posId == globalDataStruct->at(i)._posId
             && globalDataStruct1[j]._score == globalDataStruct->at(i)._score
             && globalDataStruct1[j]._wordId == globalDataStruct->at(i)._wordId)
      {
        globalDataStruct2.push_back({globalDataStruct1[j]._docId,
                                     globalDataStruct1[j]._posId,
                                     globalDataStruct1[j]._score,
                                     globalDataStruct1[j]._wordId});
        ++j;
        ++i;
      }
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;


  // Test Array[4xN]
  cout << "Intersect Array[4xN]...            ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    size_t k = 0;
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && array_4xN[i] < array_4xN1[j]
                      && array_4xN[i + num] < array_4xN1[j + num]
                      && array_4xN[i + 2*num] < array_4xN1[j + 2*num]
                      && array_4xN[i + 3*num] < array_4xN1[j + 3*num])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && array_4xN1[j] < array_4xN[i]
                      && array_4xN1[j + num] < array_4xN[i + num]
                      && array_4xN1[j + 2*num] < array_4xN[i + 2*num]
                      && array_4xN1[j + 3*num] < array_4xN[i + 3*num])
      {
        ++j;
      }
      if (array_4xN1[j] == array_4xN[i]
       && array_4xN1[j + num] == array_4xN[i + num]
       && array_4xN1[j + 2*num] == array_4xN[i + 2*num]
       && array_4xN1[j + 3*num] == array_4xN[i + 3*num])
      {
        array_4xN2[k] = array_4xN1[k];
        array_4xN2[k + num] = array_4xN1[k + num];
        array_4xN2[k + 2 * num] = array_4xN1[k + 2 * num];
        array_4xN2[k + 3 * num] = array_4xN1[k + 3 * num];
        ++j;
        ++i;
        ++k;
      }
    }
    timer.stop();
    if (l>= 2)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 2));
  cout << mSecTime << endl;

  // Test Array[Nx4]
  cout << "Intersect Array[Nx4]...            ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    i = 0;
    j = 0;
    size_t k = 0;
    timer.start();
    while (true)
    {
      if (j == len2) break;
      while (i < len1 && array_Nx4[4*i] < array_Nx41[4*j]
                      && array_Nx4[4*i+1] < array_Nx41[4*j+1]
                      && array_Nx4[4*i+2] < array_Nx41[4*j+2]
                      && array_Nx4[4*i+3] < array_Nx41[4*j+3])
      {
        ++i;
      }
      if (i == len1) break;
      while (j < len2 && array_Nx41[4*j] < array_Nx4[4*i]
                      && array_Nx41[4*j+1] < array_Nx4[4*i+1]
                      && array_Nx41[4*j+2] < array_Nx4[4*i+2]
                      && array_Nx41[4*j+3] < array_Nx4[4*i+3])
      {
        ++j;
      }
      if (array_Nx41[4*j] == array_Nx4[4*i]
       && array_Nx41[4*j+1] == array_Nx4[4*i+1]
       && array_Nx41[4*j+2] == array_Nx4[4*i+2]
       && array_Nx41[4*j+3] == array_Nx4[4*i+3])
      {
        array_Nx42[4 * k] = array_Nx41[k];
        array_Nx42[4 * k + 1] = array_Nx41[4 * k + 1];
        array_Nx42[4 * k + 2] = array_Nx41[4 * k + 2];
        array_Nx42[4 * k + 3] = array_Nx41[4 * k + 3];
        ++k;
        ++j;
        ++i;
      }
    }
    timer.stop();
    if (l>= 2)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 2));
  cout << mSecTime << endl;
}
// _____________________________________________________________________________
void readDataParallel(vector<pair <pair <DocId, WordId>,
                             pair<Score, Position> > >* globalDataPairA,
                      vector<Quatro>* globalDataClassA,
                      vector<QuatroStruct>* globalDataStructA,
                      vector<DocId>* docIdsA,
                      vector<WordId>* wordIdsA,
                      vector<Score>* scoreIdsA,
                      vector<Position>* posIdA,
                      int* array_4xN,
                      int* array_Nx4,
                      size_t num,
                      int loop)
{
  cout << "Read Pair based Vector...          ";
  Timer timer;
  size_t sum = 0;
  size_t sum1 = 0;
  off_t uSecTime = 0;
  vector<pair <pair <DocId, WordId>,
         pair<Score, Position> > >& globalDataPair = *globalDataPairA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum1 += globalDataPair[i - 1].first.first
            + globalDataPair[i].first.first;
      sum1 += globalDataPair[i - 1].first.second
            + globalDataPair[i].first.second;
      sum1 += globalDataPair[i - 1].second.first
            + globalDataPair[i].second.first;
      sum1 += globalDataPair[i - 1].second.second
            + globalDataPair[i].second.second;
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read Class based Vector...         ";
  size_t sum2 = 0;
  uSecTime = 0;
  vector<Quatro>& globalDataClass = *globalDataClassA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum2 += globalDataClass[i - 1]._docId + globalDataClass[i]._docId;
      sum2 += globalDataClass[i - 1]._posId + globalDataClass[i]._posId;
      sum2 += globalDataClass[i - 1]._score + globalDataClass[i]._score;
      sum2 += globalDataClass[i - 1]._wordId + globalDataClass[i]._wordId;
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;
  cout << "Read Struct based Vector...        ";
  size_t sum3 = 0;
  uSecTime = 0;
  vector<QuatroStruct>& globalDataStruct = *globalDataStructA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum3 += globalDataStruct[i - 1]._docId + globalDataStruct[i]._docId;
      sum3 += globalDataStruct[i - 1]._posId + globalDataStruct[i]._posId;
      sum3 += globalDataStruct[i - 1]._score + globalDataStruct[i]._score;
      sum3 += globalDataStruct[i - 1]._wordId + globalDataStruct[i]._wordId;
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read separate based Vector...      ";
  size_t sum4 = 0;
  uSecTime = 0;
  vector<DocId>& docIds = *docIdsA;
  vector<WordId>& wordIds = *wordIdsA;
  vector<Score>& scoreIds = *scoreIdsA;
  vector<Position>& posId = *posIdA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 1; i < num; i++)
    {
      sum4 += docIds[i - 1] + docIds[i];
      sum4 += posId[i - 1] + posId[i];
      sum4 += scoreIds[i - 1] + scoreIds[i];
      sum4 += wordIds[i - 1] + wordIds[i];
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read ArrayDs[4,N] based Vector...  ";
  size_t sum5 = 0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum5 += array_4xN[i - 1] + array_4xN[i];
      sum5 += array_4xN[(i + num) - 1] + array_4xN[(i + num)];
      sum5 += array_4xN[(i + 2 * num) - 1] + array_4xN[(i + 2 * num)];
      sum5 += array_4xN[(i + 3 * num) - 1] + array_4xN[(i + 3 * num)];
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read ArrayDs[N,4] based Vector...  ";
  size_t sum6 = 0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum6 += array_4xN[4 * i - 1] + array_4xN[4 * i];
      sum6 += array_4xN[(4 *  i+ 1) - 1] + array_4xN[(4 * i + 1)];
      sum6 += array_4xN[(4 * i + 2) -1] + array_4xN[(4 * i + 2)];
      sum6 += array_4xN[(4 * i + 3) - 1] + array_4xN[(4 * i + 3)];
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;
  sum = sum1 + sum2 + sum3 + sum4 + sum5 + sum6;
  cout << "CheckSum :" << sum << endl;
}
// _____________________________________________________________________________
void readDocId(vector<pair <pair <DocId, WordId>,
                      pair<Score, Position> > >* globalDataPairA,
                      vector<Quatro>* globalDataClassA,
                      vector<QuatroStruct>* globalDataStructA,
                      vector<DocId>* docIdsA,
                      vector<WordId>* wordIdsA,
                      vector<Score>* scoreIdsA,
                      vector<Position>* posIdA,
                      int* array_4xN,
                      int* array_Nx4,
                      size_t num,
                      int loop)
{
  cout << "Read Pair based Vector...          ";
  Timer timer;
  size_t sum = 0;
  size_t sum1 = 0;
  off_t uSecTime = 0;
  vector<pair <pair <DocId, WordId>,
         pair<Score, Position> > >& globalDataPair = *globalDataPairA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum1 += globalDataPair[i-1].first.first + globalDataPair[i].first.first;
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read Class based Vector...         ";
  size_t sum2 = 0;
  uSecTime = 0;
  vector<Quatro>& globalDataClass = *globalDataClassA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum2 += globalDataClass[i-1]._docId + globalDataClass[i]._docId;
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;
  cout << "Read Struct based Vector...        ";
  size_t sum3 = 0;
  uSecTime = 0;
  vector<QuatroStruct>& globalDataStruct = *globalDataStructA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum3 += globalDataStruct[i-1]._docId + globalDataStruct[i]._docId;
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read ArrayDs[4,N] based Vector...  ";
  size_t sum5 = 0;
  uSecTime = 0;

  int*  buf = new int[num];
  for (int i = 0; i < num; i++)
    buf[i] = array_4xN[i];

  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum5 += buf[i-1] + buf[i];
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read separate based Vector...      ";
  size_t sum4 = 0;
  uSecTime = 0;
  vector<DocId>& docIds = *docIdsA;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum4 += docIds[i-1] + docIds[i];
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read ArrayDs[N,4] based Vector...  ";
  size_t sum6 = 0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 1; i < num; i++)
    {
      sum6 += array_4xN[4*i-1] + array_4xN[4*i];
    }

    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;
  sum = sum1 + sum2 + sum3 + sum4 + sum5 + sum6;
  cout << "CheckSum :" << sum << endl;
}
// _____________________________________________________________________________
void readDataRandomly(vector<pair <pair <DocId, WordId>,
                             pair<Score, Position> > >* globalDataPair,
                      vector<Quatro>* globalDataClass,
                      vector<QuatroStruct>* globalDataStruct,
                      vector<DocId>* docIds,
                      vector<WordId>* wordIds,
                      vector<Score>* scoreIds,
                      vector<Position>* posId,
                      size_t num,
                      int loop)
{
  cout << "Read Pair based Vector...          ";
  Timer timer;
  size_t sum = 0;
  size_t j = 0;
  int k = 0;
  off_t uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 1; i < num; i++)
    {
      j = rand_r(&seed) % (num - 1) + 1;
      k = rand_r(&seed) % 4;
      switch (k)
      {
        case 0: sum += globalDataPair->at(j).first.first;
          break;
        case 1: sum += globalDataPair->at(j).first.second;
          break;
        case 2: sum += globalDataPair->at(j).second.first;
          break;
        default: sum += globalDataPair->at(j).second.second;
          break;
      }
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;


  cout << "Read Class based Vector...         ";
  sum = 0;
  j = 0;
  k = 0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 1; i < num; i++)
    {
      j = rand_r(&seed) % (num - 1) + 1;
      k = rand_r(&seed) % 4;
      switch (k)
      {
        case 0: sum += globalDataClass->at(j)._docId;
          break;
        case 1: sum += globalDataClass->at(j)._posId;
          break;
        case 2: sum += globalDataClass->at(j)._score;
          break;
        default: sum += globalDataClass->at(j)._wordId;
          break;
      }
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read Struct based Vector...        ";
  sum = 0;
  j = 0;
  k = 0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 1; i < num; i++)
    {
      j = rand_r(&seed) % (num - 1) + 1;
      k = rand_r(&seed) % 4;
      switch (k)
      {
        case 0: sum += globalDataStruct->at(j)._docId;
          break;
        case 1: sum += globalDataStruct->at(j)._posId;
          break;
        case 2: sum += globalDataStruct->at(j)._score;
          break;
        default: sum += globalDataStruct->at(j)._wordId;
          break;
      }
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read separate based Vector...      ";
  sum = 0;
  j = 0;
  k = 0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 1; i < num; i++)
    {
      j = rand_r(&seed) % (num - 1) + 1;
      k = rand_r(&seed) % 4;
      switch (k)
      {
        case 0: sum += docIds->at(j);
          break;
        case 1: sum += posId->at(j);
          break;
        case 2: sum += scoreIds->at(j);
          break;
        default: sum += wordIds->at(j);
          break;
      }
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;
}
// _____________________________________________________________________________
void readDataSequentiel(vector<pair <pair <DocId, WordId>,
                               pair<Score, Position> > >* globalDataPairA,
                        vector<Quatro>* globalDataClassA,
                        vector<QuatroStruct>* globalDataStructA,
                        vector<DocId>* docIdsA,
                        vector<WordId>* wordIdsA,
                        vector<Score>* scoreIdsA,
                        vector<Position>* posIdA,
                        int* array_4xN,
                        int* array_Nx4,
                        size_t num,
                        int loop)
{
  vector<pair <pair <DocId, WordId>,
         pair<Score, Position> > >& globalDataPair = *globalDataPairA;
  vector<Quatro>& globalDataClass = *globalDataClassA;
  vector<QuatroStruct>& globalDataStruct = *globalDataStructA;
  vector<DocId>& docIds = *docIdsA;
  vector<WordId>& wordIds = *wordIdsA;
  vector<Score>& scoreIds = *scoreIdsA;
  vector<Position>& posId = *posIdA;
  cout << "Read Pair based Vector...          ";
  Timer timer;
  size_t sum = 0;
  size_t sum1 =0;
  off_t uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 0; i < num; i++)
    {
      sum1 += globalDataPair[i].first.first;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum1 += globalDataPair[i].first.second;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum1 += globalDataPair[i].second.first;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum1 += globalDataPair[i].second.second;
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read Class based Vector...         ";
  size_t sum2 =0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 0; i < num; i++)
    {
      sum2 += globalDataClass[i]._docId;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum2 += globalDataClass[i]._posId;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum2 += globalDataClass[i]._score;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum2 += globalDataClass[i]._wordId;
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read Struct based Vector...        ";
  size_t sum3 =0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 0; i < num; i++)
    {
      sum3 += globalDataStruct[i]._docId;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum3 += globalDataStruct[i]._posId;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum3 += globalDataStruct[i]._score;
    }
    for (size_t i = 0; i < num; i++)
    {
      sum3 += globalDataStruct[i]._wordId;
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read separate based Vector...      ";
  size_t sum4 =0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 0; i < num; i++)
    {
      sum4 += docIds[i];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum4 += posId[i];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum4 += scoreIds[i];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum4 += wordIds[i];
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read ArrayDS[4,N] based Vector...  ";
  size_t sum5 =0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 0; i < num; i++)
    {
      sum5 += array_4xN[i];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum5 += array_4xN[i+num];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum5 += array_4xN[i+2*num];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum5 += array_4xN[i+3*num];
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  cout << "Read ArrayDS[N,4] based Vector...  ";
  size_t sum6 =0;
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();

    for (size_t i = 0; i < num; i++)
    {
      sum6 += array_4xN[4*i];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum6 += array_4xN[4*i+1];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum6 += array_4xN[4*i+2];
    }
    for (size_t i = 0; i < num; i++)
    {
      sum6 += array_4xN[4*i+3];
    }
    timer.stop();
    uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * loop);
  cout << mSecTime << endl;

  sum = sum1 + sum2 + sum3 + sum4 + sum5 + sum6;
  cout << "CheckSum :" << sum << endl;
}
// _____________________________________________________________________________
void fillData(vector<pair <pair <DocId, WordId>,
                     pair<Score, Position> > >& globalDataPair,
              vector<Quatro>& globalDataClass,
              vector<QuatroStruct>& globalDataStruct,
              vector<DocId>& docIds,
              vector<WordId>& wordIds,
              vector<Score>& scoreIds,
              vector<Position>& posId,
              int* arrayDS_4xN,
              int* arrayDS_Nx4,
              size_t num,
              int loop)
{
  double index = 1;
  Timer timer;
  off_t uSecTime = 0;
  cout << "Fill Pair based Vector...          ";
  for (int l = 0; l < loop; l++)
  {
    // globalDataPair.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      index += 0.2;
      // fill pair Vector
      pair <pair <DocId, WordId>, pair<Score, Position> > pr;
      globalDataPair[i].first.first = static_cast<int> (index);
      globalDataPair[i].first.second = static_cast<int> (index);
      globalDataPair[i].second.first = static_cast<int> (index);
      globalDataPair[i].second.first = static_cast<int> (index);
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;

  index = 1;
  cout << "Fill Class based Vector...         ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    // globalDataClass.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      index += 0.2;
      // fill pair Vector
      Quatro cl;
      globalDataClass[i]._docId = static_cast<int>(index);
      globalDataClass[i]._posId = static_cast<int>(index);
      globalDataClass[i]._score = static_cast<int>(index);
      globalDataClass[i]._wordId = static_cast<int>(index);
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;
  index = 1;
  cout << "Fill Separate based Vector...      ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    // docIds.clear();
    // wordIds.clear();
    // scoreIds.clear();
    // posId.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      index += 0.2;
      // fill pair Vector
      docIds[i] = static_cast<int>(index);
      wordIds[i] = static_cast<int>(index);
      scoreIds[i] = static_cast<int>(index);
      posId[i] = static_cast<int>(index);
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;

  index = 1;
  cout << "Fill Struct based Vector...        ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    // globalDataStruct.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      index += 0.2;
      // fill pair Vector
      globalDataStruct[i]._docId = index;
      globalDataStruct[i]._posId = index;
      globalDataStruct[i]._score = index;
      globalDataStruct[i]._wordId = index;
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;
  index = 1;
  cout << "Fill ArrayDs[4,N]...               ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      index += 0.2;
      // fill pair Vector
      arrayDS_4xN[i] = index;
      arrayDS_4xN[i + num] = index;
      arrayDS_4xN[i + 2 * num] = index;
      arrayDS_4xN[i + 3 * num] = index;
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 1));
  cout << mSecTime << endl;

  index = 1;
  cout << "Fill ArrayDs[N,4]...               ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      index += 0.2;
      // fill pair Vector
      arrayDS_Nx4[4 * i] = index;
      arrayDS_Nx4[4 * i + 1] = index;
      arrayDS_Nx4[4 * i + 2] = index;
      arrayDS_Nx4[4 * i + 3] = index;
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop - 1));
  cout << mSecTime << endl;
}
// _____________________________________________________________________________
void fillDataFromVector(vector<pair <pair <DocId, WordId>,
                               pair<Score, Position> > >& globalDataPair,
              vector<Quatro>& globalDataClass,
              vector<QuatroStruct>& globalDataStruct,
              vector<DocId>& docIds,
              vector<WordId>& wordIds,
              vector<Score>& scoreIds,
              vector<Position>& posId,
              int* arrayDS_4xN,
              int* arrayDS_Nx4,
              size_t num,
              int loop)
{
  vector<pair <pair <DocId, WordId>, pair<Score, Position> > > globalDataPair1;
  vector<Quatro> globalDataClass1;
  vector<QuatroStruct> globalDataStruct1;


  vector<DocId> docIds1;
  vector<WordId> wordIds1;
  vector<Score> scoreIds1;
  vector<Position> posId1;
  Quatro cl(0, 0, 0, 0);
  globalDataClass1.resize(num, cl);
  globalDataStruct1.resize(num, {0, 0, 0, 0});
  pair <pair <DocId, WordId>, pair<Score, Position> > pr;
  pr.first.first = static_cast<int> (0);
  pr.first.second = static_cast<int> (0);
  pr.second.first = static_cast<int> (0);
  pr.second.first = static_cast<int> (0);
  globalDataPair1.resize(num, pr);
  docIds1.resize(num, 0);
  wordIds1.resize(num, 0);
  scoreIds1.resize(num, 0);
  posId1.resize(num, 0);

  int *arrayDS_4xN1 = new int[num * 4];
  int *arrayDS_Nx41 = new int[num * 4];
  double index = 1;
  Timer timer;
  off_t uSecTime = 0;
  cout << "Fill Pair based Vector...          ";
  for (int l = 0; l < loop; l++)
  {
    // globalDataPair1.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      globalDataPair1[i] = globalDataPair[i];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / (1000.0 * (loop - 1));
  cout << mSecTime << endl;

  index = 1;
  cout << "Fill Class based Vector...         ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    // globalDataClass1.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      globalDataClass1[i] = globalDataClass[i];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;
  index = 1;
  cout << "Fill Separate based Vector...      ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    // docIds1.clear();
    // wordIds1.clear();
    // scoreIds1.clear();
    // posId1.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      // fill pair Vector
      docIds1[i] = docIds[i];
      wordIds1[i] = wordIds[i];
      scoreIds1[i] = scoreIds[i];
      posId1[i] = posId[i];
    }
    timer.stop();
    if (l > 0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;

  index = 1;
  cout << "Fill Struct based Vector...        ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    // globalDataStruct1.clear();
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      // fill pair Vector
      globalDataStruct1[i] = globalDataStruct[i];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;
  index = 1;
  cout << "Fill ArrayDs[4,N]...               ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      // index += 0.2;
      // fill pair Vector
      arrayDS_4xN1[i] = arrayDS_4xN[i];
      arrayDS_4xN1[i + num] = arrayDS_4xN[i + num];
      arrayDS_4xN1[i + 2 * num] = arrayDS_4xN[i + 2 * num];
      arrayDS_4xN1[i + 3 * num] = arrayDS_4xN[i + 3 * num];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;

  index = 1;
  cout << "Fill ArrayDs[N,4]...               ";
  uSecTime = 0;
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < num; i++)
    {
      arrayDS_Nx41[ 4 * i]= arrayDS_Nx4[4 * i];
      arrayDS_Nx41[4 * i + 1]= arrayDS_Nx4[4 * i + 1];
      arrayDS_Nx41[4 * i + 2]= arrayDS_Nx4[4 * i + 2];
      arrayDS_Nx41[4 * i + 3]= arrayDS_Nx4[4 * i + 3];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / (1000.0 * (loop-1));
  cout << mSecTime << endl;
  size_t sum = globalDataClass1.size()
             + globalDataPair1.size()
             + globalDataStruct1.size()
             + docIds1.size()
             + wordIds1.size()
             + scoreIds1.size()
             + posId1.size()
             + arrayDS_4xN1[num*2]
             + arrayDS_Nx41[num*2];
  cout << "CheckSum :" << sum << endl;
}
// _____________________________________________________________________________
void staticVSnewTest(int loop)
{
  int n = 1000000;
  int arraySs[1000000];
  int *arrayDs = new int[n];
  Timer timer;
  off_t uSecTime = 0;
  cout << "Fill new array[1000000]...         ";
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < n; i++)
    {
      arrayDs[i] = 1;
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  double mSecTime = uSecTime / ((loop-1));
  cout << mSecTime << endl;

  uSecTime = 0;
  cout << "Fill static array[1000000]...      ";
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < n; i++)
    {
      arraySs[i] = 1;
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / ((loop-1));
  cout << mSecTime << endl;

  uSecTime = 0;
  size_t sum1 =0;
  cout << "Read new array[1000000]...         ";
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < n; i++)
    {
      sum1+= arrayDs[i];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / ((loop-1));
  cout << mSecTime << endl;

  uSecTime = 0;
  size_t sum2 =0;
  cout << "Read static array[1000000]...      ";
  for (int l = 0; l < loop; l++)
  {
    timer.start();
    for (size_t i = 0; i < n; i++)
    {
      sum2+= arraySs[i];
    }
    timer.stop();
    if (l>0)
      uSecTime += timer.usecs();
  }

  // set the time of the queryStat object
  mSecTime = uSecTime / ((loop-1));
  cout << mSecTime << endl;
  size_t sum = sum1 + sum2;
  cout << "CheckSum :" << sum << endl;
}
// _____________________________________________________________________________
void makeTest(int argc, char **argv)
{
  size_t lineNumber;
  int loopNumber;
  vector<pair <pair <DocId, WordId>, pair<Score, Position> > > globalDataPair;
  vector<Quatro> globalDataClass;
  vector<QuatroStruct> globalDataStruct;

  vector<DocId> docIds;
  vector<WordId> wordIds;
  vector<Score> scoreIds;
  vector<Position> posId;

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
      {"line-number", required_argument, 0, 'n'},
      {"repeat", required_argument, 0, 'r'},
      {0, 0, 0, 0}
    };

    optChr = getopt_long(argc, argv, "hn:r:", longOptions, &optionIndex);

    if (optChr == -1) break;

    switch (optChr)
    {
      case 0:
        break;
      case 'h':
        printUsage();
        exit(1);
        break;
      case 'n':
        lineNumber = atoi(optarg);
        break;
      case 'r':
        loopNumber = atoi(optarg);
        break;
      default:
        cerr << "unknown option: " << optChr << " ??" << endl;
        printUsage();
        exit(1);
        break;
    }
  }
  if (lineNumber == 0) lineNumber = 1000000;
  if (loopNumber == 0) loopNumber = 3;
  int *arrayDS_4xN = new int[lineNumber*4];
  int *arrayDS_Nx4 = new int[lineNumber*4];
  globalDataClass.reserve(lineNumber);
  globalDataStruct.reserve(lineNumber);
  globalDataPair.reserve(lineNumber);
  docIds.reserve(lineNumber);
  wordIds.reserve(lineNumber);
  scoreIds.reserve(lineNumber);
  posId.reserve(lineNumber);

  Quatro cl(0, 0, 0, 0);
  globalDataClass.resize(lineNumber, cl);
  globalDataStruct.resize(lineNumber, {0, 0, 0, 0});
  pair <pair <DocId, WordId>, pair<Score, Position> > pr;
  pr.first.first = static_cast<int> (0);
  pr.first.second = static_cast<int> (0);
  pr.second.first = static_cast<int> (0);
  pr.second.first = static_cast<int> (0);
  globalDataPair.resize(lineNumber, pr);
  docIds.resize(lineNumber, 0);
  wordIds.resize(lineNumber, 0);
  scoreIds.resize(lineNumber, 0);
  posId.resize(lineNumber, 0);

  cout << "-----------------------------------------";
  cout << " \nFILL DATA FROM CONSTANT:" << endl;
  fillData(globalDataPair,
           globalDataClass,
           globalDataStruct,
           docIds,
           wordIds,
           scoreIds,
           posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nFILL DATA FROM VECTOR:" << endl;
  fillDataFromVector(globalDataPair,
           globalDataClass,
           globalDataStruct,
           docIds,
           wordIds,
           scoreIds,
           posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nREAD DATA PARALLEL:" << endl;
  readDataParallel(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nREAD DATA SEQUENTIEL:" << endl;
  readDataSequentiel(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           loopNumber);
  cout << "-----------------------------------------";
  /*cout << " \nREAD DATA RANDOMLY:" << endl;
  readDataRandomly(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           lineNumber,
           loopNumber);*/
  cout << "-----------------------------------------";
  cout << " \nREAD JUST DOCIDS:" << endl;
  readDocId(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nSIMPLE INTERESECT ALGORITHM:" << endl;
  SimpleIntersect(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           true,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nSIMPLE INTERESECT WITH FULL COMPARISON:" << endl;
  SimpleIntersectFull(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nSIMPLE INTERESECT ALGORITHM WITHOUT MAKING OF RESULTLIST:"
       << endl;
  SimpleIntersect(&globalDataPair,
           &globalDataClass,
           &globalDataStruct,
           &docIds,
           &wordIds,
           &scoreIds,
           &posId,
           arrayDS_4xN,
           arrayDS_Nx4,
           lineNumber,
           false,
           loopNumber);
  cout << "-----------------------------------------";
  cout << " \nTEST STATIC VS NEW:" << endl;
  staticVSnewTest(loopNumber);
}
// _____________________________________________________________________________
int main(int argc, char **argv)
{
  cout.setf(ios::fixed);
  cout.precision(2);
  makeTest(argc, argv);
}
