// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

// File for three classes related to statistics recording and output:
// Statisticspool, a pool of several
// measurements, one measurement is a measurement for the times for a specific
// input and has several runs with:
// one recorded time for each algorithm


#ifndef SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_TESTSTATISTICS_H_
#define SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_TESTSTATISTICS_H_

#include <gtest/gtest.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
namespace IntersectPerformanceEvaluation
{
  class Measurement;
  class StatisticsPool
  {
  public:
    FRIEND_TEST(TestStatisticsTest, StatisticsPoolConstructor);
    // standard constructor needing the attributes!
    StatisticsPool();
    ~StatisticsPool();
    FRIEND_TEST(TestStatisticsTest, StatisticsPoolStartMeasurement);
    // start the next measurement
    void startMeasurement();
    // add or edit attributes for this measurement
    void setAttributeForThisMeasurement(std::string attribute,
                                        std::string value);
    // add or edit attributes for this measurement
    void setNrAttributeForThisMeasurement(std::string attribute,
                                              double value,
                                              size_t precision);
    // start the next Run for current measurement
    void startRun();
    // add another time for the current run and given algorithm
    void addTime(std::string algorithmName, double time);

    // add time for a specific run of the current measurement
    void addTimeForRun(size_t runNr, std::string algorithmName, double time);
 
    // write statistics to output file in csv-format,
    // including averages for the runs etc.
    void writeStatisticsToFile(const std::string& fileName);
    // print same statistics on the console...
    void printStatistics();

  private:
    // these are the attributes necessary to
    // describe each measurement, for example listsize etc.
    // they will be used as headers of the output file
    std::vector<std::string>* attributes;
    // the algorithm names for this statistics pool measurement!
    std::vector<std::string>* algorithms;
    // the actual measurements!
    std::vector<Measurement>* measurements;
    // write statistics to actual file pointer...
    // will not close file afterwards...
    void writeStatisticsToFilePtr(FILE* statisticsFile);
  };
  class Run;
  // one measurement has a vector with runs
  class Measurement
  {
  public:
    Measurement();
    Measurement(const Measurement& otherMeasurement);
    ~Measurement();
    // start the next Run for this measurement
    void startRun();
    // add time for current run and given algorithm
    void addTime(std::string algorithmName, double time);
    // add time for a specific run of this measurement
    void addTimeForRun(size_t runNr, std::string algorithmName, double time);
    // add specific new input output value
    void setAttribute(std::string attribute, std::string value);
    // add specific new input output value, as a number, that will be converted
    // with the given precision
    void setNumberAttribute(std::string attribute,
                            double value,
                            size_t precision);
    // get specific input or output value, if input output value is not known
    // return "unknown"
    std::string getAttribute(std::string attribute);
    // get runtime of certain run for certain algorithm
    double getTime(size_t runNr, std::string algorithmName);
    // get average runtime of a certain algorithm
    double getAverageRuntime(std::string algorithmName);
    // get the vector of runs of this algorithm
    void getRuns(std::vector<Run>* allRuns);
    // get number of performed runs
    size_t getNumberOfRuns();
  private:
    // values of attributes, for example
    // input and output of this measurement (not times)
    std::map<std::string, std::string>* attributeValueMap;
    // the runs of this measurement
    std::vector<Run>* runs;
  };

  class Run
  {
  public:
    Run();
    Run(const Run&);
    ~Run();
    void addTime(std::string algorithmName, double time);
    double getTime(std::string algorithmName);
  private:
    // the values of the individual times of the algorithms
    std::map<std::string, double>* algorithmToTimeMap;
  };
}
#endif  // SERVER_COMPLETERBASE_INTERSECTPERF_COMPLETERBASE_TESTSTATISTICS_H_

