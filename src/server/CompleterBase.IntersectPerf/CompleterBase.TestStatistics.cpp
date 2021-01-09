// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#include <map>
#include <utility>
#include <vector>
#include<string>
#include <sstream>
#include <algorithm>
#include "./CompleterBase.TestStatistics.h"
namespace IntersectPerformanceEvaluation
{
  // _______________________ STATISTICSPOOL CLASS_______________________________
  StatisticsPool::StatisticsPool()
  {
    this->attributes = new std::vector<std::string>();
    this->algorithms = new std::vector<std::string>();
    this->measurements = new std::vector<Measurement>();
  }

  StatisticsPool::~StatisticsPool()
  {
    delete this->attributes;
    delete this->algorithms;
    delete this->measurements;
  }

  void StatisticsPool::startMeasurement()
  {
    Measurement newMeasurement;
    this->measurements->push_back(newMeasurement);
  }

  void StatisticsPool::setAttributeForThisMeasurement(std::string attribute,
                                                      std::string value)
  {
    // check that there is atleast one measurement!
    assert(this->measurements->size() > 0);
    // remember attribute if not known
    if (std::find(this->attributes->begin(), this->attributes->end(), attribute)
        == this->attributes->end())
      this->attributes->push_back(attribute);
    Measurement& currentMeasurement = this->measurements->at(
                                     this->measurements->size() -1);
    currentMeasurement.setAttribute(attribute, value);
  }

  void StatisticsPool::setNrAttributeForThisMeasurement(std::string attribute,
                                                        double value,
                                                        size_t precision)
  {
    // check that there is atleast one measurement!
    assert(this->measurements->size() > 0);
        // remember attribute if not known
    if (std::find(this->attributes->begin(), this->attributes->end(), attribute)
        == this->attributes->end())
      this->attributes->push_back(attribute);
    Measurement& currentMeasurement = this->measurements->at(
                                     this->measurements->size() -1);
    currentMeasurement.setNumberAttribute(attribute, value, precision);
  }

  void StatisticsPool::startRun()
  {
    // check that there is atleast one measurement!
    assert(this->measurements->size() > 0);
    Measurement& currentMeasurement = this->measurements->at(
                                     this->measurements->size() -1);
    currentMeasurement.startRun();
  }

  void StatisticsPool::addTime(std::string algorithmName, double time)
  {
    // check that there is atleast one measurement!
    assert(this->measurements->size() > 0);
    Measurement& currentMeasurement = this->measurements->at(
                                     this->measurements->size() -1);
    currentMeasurement.addTime(algorithmName, time);
    // add algorithm name to known algorithms if its not already added!
    if (std::find(this->algorithms->begin(),
                  this->algorithms->end(),
                  algorithmName) ==
         this->algorithms->end())
      this->algorithms->push_back(algorithmName);
  }

  void StatisticsPool::addTimeForRun(size_t runNr, std::string algorithmName,
                                     double time)
  {
    // check that there is atleast one measurement!
    assert(this->measurements->size() > 0);
    Measurement& currentMeasurement = this->measurements->at(
                                     this->measurements->size() -1);
    currentMeasurement.addTimeForRun(runNr, algorithmName, time);
    // add algorithm name to known algorithms if its not already added!
    if (std::find(this->algorithms->begin(),
                  this->algorithms->end(),
                  algorithmName) ==
         this->algorithms->end())
      this->algorithms->push_back(algorithmName);
  }

  void StatisticsPool::writeStatisticsToFile(const std::string& fileName)
  {
    FILE* statisticsFile = fopen(fileName.c_str(), "w");
    if (statisticsFile == NULL)
    {
      printf("Can not create statistics file %s", fileName.c_str());
      return;
    }
    writeStatisticsToFilePtr(statisticsFile);
    fclose(statisticsFile);
  }

  void StatisticsPool::printStatistics()
  {
    this->writeStatisticsToFilePtr(stdout);
  }

  void StatisticsPool::writeStatisticsToFilePtr(FILE* statisticsFile)
  {
    // check that we do have a file pointer...
    assert(statisticsFile != NULL);

     // write headers with 12 characters
    for (size_t i = 0; i < this->attributes->size(); i++)
      fprintf(statisticsFile,
              "%12s ",
              this->attributes->at(i).substr(0, 12).c_str());
    // write algorithms with 12 characters
    for (size_t i = 0; i < this->algorithms->size(); i++)
      fprintf(statisticsFile,
              "%12s ",
              this->algorithms->at(i).substr(0, 12).c_str());
    fprintf(statisticsFile, "\n");
    // for each measurement write time for each run and average time
    for (size_t i = 0; i < this->measurements->size(); i++)
    {
      Measurement& curMeasurement = this->measurements->at(i);
      // write times for each run
      for (size_t j = 0; j < curMeasurement.getNumberOfRuns(); j++)
      {
        // write input output values

        for (size_t k = 0; k < this->attributes->size(); k++)
        {
          // only write input output lines on first line
          // on following lines write empty strings
          if (j == 0)
            fprintf(statisticsFile,
                    "%12s ",
                    curMeasurement.getAttribute(this->attributes->at(k))
                    .substr(0, 12).c_str());
          else
            fprintf(statisticsFile, "%12s ", "");
        }

        // write algorithm times
        for (size_t k = 0; k < this->algorithms->size(); k++)
        {
          fprintf(statisticsFile,
                  "%12.1f ",
                  curMeasurement.getTime(j, this->algorithms->at(k)));
        }
        fprintf(statisticsFile, "\n");
      }
      // no average times if there were no runs!
      if (curMeasurement.getNumberOfRuns() == 0) continue;
      // write average times:
      // write empty strings for input output values

      // write long line at start for seperating average times from runs
      size_t lineSize = ((this->attributes->size() + this->algorithms->size())
                         * 13) - 1;  // -1 because last character is a space
      for (size_t j = 0; j < lineSize; j++)
        fprintf(statisticsFile, "-");
      fprintf(statisticsFile, "\n");
      for (size_t j = 0; j < this->attributes->size(); j++)
      {
        fprintf(statisticsFile, "%12s ", "");
      }
      for (size_t j = 0; j < this->algorithms->size(); j++)
      {
        fprintf(statisticsFile,
                "%12.1f ",
                curMeasurement.getAverageRuntime(this->algorithms->at(j)));
      }
      fprintf(statisticsFile, "\n\n");
    }
  }
  // _____________________END STATISTICSPOOL CLASS______________________________

  // _________________________MEASUREMENT CLASS_________________________________
  Measurement::Measurement()
  {
    this->attributeValueMap = new std::map<std::string, std::string>();
    this->runs = new std::vector<Run>();
  }

  Measurement::Measurement(const Measurement& otherMeasurement)
  {
    // copy input output map
    this->attributeValueMap = new std::map<std::string, std::string>(
                                        *(otherMeasurement.attributeValueMap));
    // copy runs
    this->runs = new std::vector<Run>(*(otherMeasurement.runs));
  }
  Measurement::~Measurement()
  {
    delete this->attributeValueMap;
    delete this->runs;
  }
  void Measurement::startRun()
  {
    Run newRun;
    this->runs->push_back(newRun);
  }
  void Measurement::addTime(std::string algorithmName, double time)
  {
    // check that size is bigger 0
    assert(this->runs->size() > 0);
    Run& currentRun = this->runs->at(this->runs->size() - 1);
    currentRun.addTime(algorithmName, time);
  }

  void Measurement::addTimeForRun(size_t runNr, std::string algorithmName,
                                  double time)
  {
    // check that runNr exists!
    assert(this->runs->size() > runNr);
    Run& wantedRun = this->runs->at(runNr);
    wantedRun.addTime(algorithmName, time);
  }

  void Measurement::setAttribute(std::string attribute, std::string value)
  {
    // first erase old value, then add
    this->attributeValueMap->erase(attribute);
    std::pair<std::string, std::string> attributeValuePair(attribute, value);
    this->attributeValueMap->insert(attributeValuePair);
  }
  void Measurement::setNumberAttribute(std::string attribute,
                                       double value,
                                       size_t precision)
  {
    //  first erase old value, then convert to string, then add
    this->attributeValueMap->erase(attribute);

    // set to fixed display not scientific and precision amount of digits
    // after comma
    std::ostringstream numberAsString;
    numberAsString << std::setprecision(precision) << std::fixed;
    numberAsString << value;
    std::pair<std::string, std::string> attributeValuePair(attribute,
                                                          numberAsString.str());
    this->attributeValueMap->insert(attributeValuePair);
  }
  std::string Measurement::getAttribute(std::string attribute)
  {
    // check that value is present, if not return "unknown"
    if (this->attributeValueMap->find(attribute) ==
            this->attributeValueMap->end())
      return "unknown";
    else
      return this->attributeValueMap->at(attribute);
  }
  double Measurement::getTime(size_t runNr, std::string algorithmName)
  {
    // check that run index is not out of range
    assert(runNr < this->runs->size());
    double runtime = this->runs->at(runNr).getTime(algorithmName);
    return runtime;
  }

  double Measurement::getAverageRuntime(std::string algorithmName)
  {
    double sumOfRuntime = 0;
    size_t validRuns = 0;
    for (size_t i = 0; i < this->runs->size(); i++)
    {
      double oneRuntime = this->runs->at(i).getTime(algorithmName);
      if (oneRuntime != -1)  // -1 signals run wasnt done with this algorithm!
      {
        sumOfRuntime += oneRuntime;
        validRuns++;
      }
    }
    // if no valid runs, just return -1...
    if (validRuns == 0) return -1;
    double averageRuntime = sumOfRuntime / validRuns;
    return averageRuntime;
  }
  void Measurement::getRuns(std::vector<Run>* allRuns)
  {
    assert(allRuns != NULL);
    allRuns->clear();
    for (size_t i = 0; i < this->runs->size(); i++)
    {
      allRuns->push_back(this->runs->at(i));
    }
  }

  size_t Measurement::getNumberOfRuns()
  {
    return this->runs->size();
  }
  // ________________________END MEASUREMENT CLASS______________________________

  // ________________________RUN CLASS__________________________________________
  Run::Run()
  {
    this->algorithmToTimeMap = new std::map<std::string, double>();
  }

  Run::Run(const Run& otherRun)
  {
    // copy algorithm index map
    this->algorithmToTimeMap =
            new std::map<std::string, double>(*otherRun.algorithmToTimeMap);
  }

  Run::~Run()
  {
    delete this->algorithmToTimeMap;
  }

  void Run::addTime(std::string algorithmName, double time)
  {
    // erase old time, then add new one
    this->algorithmToTimeMap->erase(algorithmName);
    std::pair<std::string, double> algorithmTimePair(algorithmName, time);
    this->algorithmToTimeMap->insert(algorithmTimePair);
  }

  double Run::getTime(std::string algorithmName)
  {
    // check if algorithmName exists, if not return -1 else return time
    if (this->algorithmToTimeMap->find(algorithmName) ==
           this->algorithmToTimeMap->end())
      return -1;  // -1 signal for algorithm has not been assigned a time!
    double algorithmTime = this->algorithmToTimeMap->at(algorithmName);
    return algorithmTime;
  }
  // ________________________END RUN CLASS______________________________________
}
