/*
 * executeQueries.cpp
 *
 *  Created on: Jan 12, 2010
 *      Author: celikik
 */

#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>

using namespace std;

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    cout << "Usage: executeQueries <query file> <results file path>" << endl << endl;
    return 0;
  }
  remove(argv[2]);
  fstream queryFile;
  queryFile.open(argv[1], ios::in);
  if (queryFile.fail())
  {
    cerr << "Error opening query file!" << endl << endl;
    return 1;
  }
  string query;
  int count = 0;
  while(true)
  {
    queryFile >> query;
    if (queryFile.eof())
      break;
    count ++;
    string cmdLine = "curl -s localhost:9999/?q=\"" + query + "~&h=0&c=0\" > /dev/null";
    int ret = system(cmdLine.c_str());
    if (ret == -1) { perror(cmdLine.c_str()); exit(1); }
    cout << "." << flush;
  }
  cout << endl;
  queryFile.close();
  cout << count << " queries executed." << endl << endl;
  fstream resultsFile;
  resultsFile.open(argv[2], ios::in);
  if (resultsFile.fail())
  {
    cerr << "Error opening results file!" << endl << endl;
    return 1;
  }
  int intRes;
  float floatRes;
  float avgRecall = 0;
  float minRecall = 1.0f;
  string minRecallQuery;
  float avgPrecision = 0;
  float avgSmoothPrecision = 0;
  float avgClusters = 0;
  float avgWords = 0;
  float avgExec = 0;
  int maxClusters = 0;
  string maxClustersQuery;
  int count1 = 0;
  int avgWordLen = 0;
  while(true)
  {
    resultsFile >> query;
    if (resultsFile.eof())
      break;
    avgWordLen += query.length();
    resultsFile >> intRes;
    avgWords += intRes;
    resultsFile >> intRes;
    resultsFile >> intRes;
    avgClusters += intRes;
    if (maxClusters < intRes)
    {
      maxClusters = intRes;
      maxClustersQuery = query;
    }
    resultsFile >> floatRes;
    avgPrecision += floatRes;
    resultsFile >> floatRes;
    avgSmoothPrecision += floatRes;
    resultsFile >> floatRes;
    avgRecall += floatRes;
    if (minRecall > floatRes)
    {
      minRecall = floatRes;
      minRecallQuery = query;
    }
    resultsFile >> floatRes;
    avgExec += floatRes;
    count1 ++;
  }
  cout << "Avg. nof clusters : " << avgClusters / count1 << endl;
  cout << "Max. nof clusters : " << maxClusters << " (" << maxClustersQuery << ")" << endl;
  cout << "Avg. nof words    : " << avgWords / count1 << endl;
  cout << "Avg. precision    : " << avgPrecision / count1 << endl;
  cout << "Avg. smooth prec. : " << avgSmoothPrecision / count1 << endl;
  cout << "Avg. recall       : " << avgRecall / count1 << endl;
  cout << "Min. recall       : " << minRecall << " (" << minRecallQuery << ")" << endl << endl;
  cout << "Avg query exec.   : " << avgExec / count1 << " ms." << endl;
  cout << "Avg. word length  : " << 1.0 * avgWordLen / count1 << endl << endl;
  assert(count1 = count);
  resultsFile.close();
}
