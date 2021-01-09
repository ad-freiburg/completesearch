// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Hannah Bast <bast>, Ina Baumgarten <baumgari>.

#ifndef UTILITY_TIMERSTATISTICS_H_
#define UTILITY_TIMERSTATISTICS_H_

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "../server/Timer.h"

class TimerStatisticsSubTimer;

class TimerStatistics
{
  FRIEND_TEST(constructorTests, TimerStatistics);
  FRIEND_TEST(publicTimerTests, addTimer);
  FRIEND_TEST(publicTimerTests, set);
  FRIEND_TEST(publicTimerTests, setThresholds);
  FRIEND_TEST(publicTimerTests, setPrecisions);
  FRIEND_TEST(privateTimerTests, sort);
  FRIEND_TEST(privateTimerTests, checkTimeUnit);
  FRIEND_TEST(privateTimerTests, checkMaxLength);
  FRIEND_TEST(privateTimerTests, setSubsubTimer);

  public:
  // Constructor
  TimerStatistics(int indentLevel = 0, float percent = 100);

  // Possible time units.
  enum TimeUnit
  {
    NSECS = 0,
    USECS = 1,
    MSECS = 2,
    SECS = 3,
    MINS = 4,
    HOURS = 5,
    DAYS = 6,
    AUTO = 7
  };

  // TODO(bast) for Ina: maybe best to have the timer and volume descriptions
  // contain a placeholder which says where the time / volume should go, for
  // example: receive query $TIME. Maybe have this only for the volume
  // descriptions.
  // (baumgari): done for volume description

  // Add a subtimer with or without subsubtimers/comment.
  void addSubTimerWithComment(const Timer& timer,
                              TimeUnit timerUnit,
                              std::string timerDescription,
                              std::string comment,
                              bool showAlways,
                              TimerStatistics& subsubTimer);

  void addSubTimer(const Timer& timer,
                   TimeUnit timerUnit,
                   std::string timerDescription,
                   bool showAlways,
                   TimerStatistics& subsubTimer);

  void addSubTimerWithComment(const Timer& timer,
                              TimeUnit timerUnit,
                              std::string timerDescription,
                              std::string comment,
                              bool showAlways);

  void addSubTimer(const Timer& timer,
                   TimeUnit timerUnit,
                   std::string timerDescription,
                   bool showAlways);

  // Add a subtimer with additional volume measurement and optional
  // some subsubtimers.
  void addSubTimerWithVolume(const Timer& timer,
                             TimeUnit timerUnit,
                             std::string timerDescription,
                             float rate,
                             std::string rateUnit,
                             std::string rateDescription,
                             bool showAlways,
                             TimerStatistics& subsubTimer);

  void addSubTimerWithVolume(const Timer& timer,
                             TimeUnit timerUnit,
                             std::string timerDescription,
                             float rate,
                             std::string rateUnit,
                             std::string rateDescription,
                             bool showAlways);

  // Add timer which measured the total time (optional).
  void addTotalTimer(const Timer& timer,
                     TimeUnit timerUnit,
                     std::string timerDescription);

  // Get statistics as string in human-readable form. Show only items were the
  // percentage of the running time is above percentageThreshold. Show at most
  // numItemsThreshold (-1 = show all). Items with showAlways == true are always
  // shown, independent of the threshold parameters. Precision is the number of
  // digits that should be shown after the dot (should be the same number for
  // all items so that it looks nice).
  std::vector<std::string> getStatistics();

  // Set if totalTimer should be printed.
  void setPrintOptions(bool printTotalTimer,
                       bool printAsHierarchy,
                       bool sortAfterPercentage)
  {
    this->printTotalTimer = printTotalTimer;
    this->hierarchy = printAsHierarchy;
    this->sortAfterPercentage = sortAfterPercentage;
  };

  // Set the parameters of get Statistics.
  void set(int timePrecision,
           int percPrecision,
           int ratePrecision,
           int percentageThreshold,
           int sumOfPercentageThreshold,
           int numItemsThreshold);

  // Set precision parameters of get Statistics.
  void setPrecisions(int timePrecision,
                     int percPrecision,
                     int ratePrecision);

  // Set threshold parameters of get Statistics.
  void setThresholds(int percentageThreshold,
                     int sumOfPercentageThreshold,
                     int numItemsThreshold);

  // Indent_Level of TimerStatistics (for subsubtimer).
  int indentLevel;

 private:
  // SortFunction
  void sort();
  // Returns string for a given subTimer.
  std::string getSubTimerString(TimerStatisticsSubTimer * subTimer,
                                std::string indentString);
  std::string getStringRest(const std::string& description, float time,
                            std::string unit, float percentage,
                            std::string indentString);
  std::string getStringTotal(const std::string& description, float time,
                             std::string unit);


  // Decide which unit should be used.
  std::string checkTimeUnit(double* time, TimeUnit* timerUnit);
  // Checks the length of the longest Timer Description.
  int checkMaxLength();
  // Sets subsubTimer to a _subtimer.
  void setSubsubTimer(TimerStatisticsSubTimer* subTimer,
                      TimerStatistics* subsubTimer);
  // All the subtimers and their associated information.
  std::vector<TimerStatisticsSubTimer> _subTimers;
  // Total time (optional).
  TimerStatisticsSubTimer* _totalTimer;
  // Stores the length of the longest timer description.
  static size_t maxString;
  // Total sum of time.
  double totalSum;
  // Total time;
  double totalTime;
  // Stores how many percent the whole timer represents.
  float percent;
  // Parameters for getStatistics().
  int number;
  int timePrecision;
  int percPrecision;
  int ratePrecision;
  int percentageThreshold;
  int sumOfPercentageThreshold;
  int numItemsThreshold;
  // Stores if the parameters are not default.
  // If they are default, the subsubtimers are going
  // to use the "old" precisions of the subtimer, but
  // showing all items.
  bool parametersSet;
  // Stores if totalTimer should be printed.
  static bool printTotalTimer;
  // Stores if an hierarchy should be printed (Timer -> Subtimers)
  static bool hierarchy;
  // Stores whether the output should be printed by percentage or by input
  // order.
  static bool sortAfterPercentage;
};

// A subtimer and all the information associated with it.
class TimerStatisticsSubTimer
{
 public:
  const Timer* timer;
  TimerStatistics::TimeUnit timerUnit;
  std::string timerDescription;
  std::string comment;
  float rate;
  std::string rateUnit;
  std::string rateDescription;
  bool showAlways;
  float percentage;
  int number;
  TimerStatistics* subsubTimer;
  // Sets if subsubTimer is set.
  TimerStatisticsSubTimer(const Timer* timer,
                          TimerStatistics* subsubTimer = NULL);
  bool set;
};
#endif  // UTILITY_TIMERSTATISTICS_H_
