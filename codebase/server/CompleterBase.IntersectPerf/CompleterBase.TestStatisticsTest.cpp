// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Simon Skilevic and Robin Schirrmeister...

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "./CompleterBase.TestStatistics.h"
/*TEST(TestStatisticsTest, StatisticsPoolConstructor)
{
  std::string headers [] = {"bla","header2","blub"};
  std::string algorithmNames[] = {"boost", "pthread", "simple"};
  std::vector<std::string> algos(algorithmNames,
                  algorithmNames + sizeof(algorithmNames)/sizeof(std::string));
  IntersectPerformanceEvaluation::StatisticsPool statPool(headers, 3, algos);
  /* TODO(ROBIN): FUNKTIONIERT NICHT, WIESO?
  ASSERT_EQ(statPool.headers[0], "bla");
  ASSERT_EQ(statPool.headers[1], "header2");
  ASSERT_EQ(statPool.headers[2], "blub");*//*
}
TEST(TestStatisticsTest, StatisticsPoolStartMeasurement)
{
  // just call all function to test for segfault, atm nothing more possible
  // due to friend-test not working as i thought?!
  std::string headers [] = {"bla","header2","blub"};
  std::string algorithmNames[] = {"boost", "pthread", "simple"};
  std::vector<std::string> algos(algorithmNames,
                  algorithmNames + sizeof(algorithmNames)/sizeof(std::string));
  IntersectPerformanceEvaluation::StatisticsPool statPool(headers, 3, algos);
  statPool.startMeasurement();
  statPool.addValueForThisMeasurement("bla", 3);
  statPool.addValueForThisMeasurement("header2", 4.5);
  statPool.addValueForThisMeasurement("blub", -1.0);

  statPool.addValueForThisMeasurement("bla", 3);
  statPool.addValueForThisMeasurement("header2", 4.5);
  statPool.addValueForThisMeasurement("blub", -1.0);
  statPool.startRun();
  statPool.addTime("boost", 1.5);
  statPool.addTime("simple", 2.5);

  statPool.startRun();
  statPool.addTime("boost", 3.5);
  statPool.addTime("pthread", 5.8);
    statPool.startMeasurement();
  statPool.addValueForThisMeasurement("bla", 3);
  statPool.addValueForThisMeasurement("header2", 4.5);
  statPool.addValueForThisMeasurement("blub", -1.0);

  /* TODO(ROBIN): FUNKTIONIERT NICHT, WIESO?
  ASSERT_EQ(statPool.headers[0], "bla");
  ASSERT_EQ(statPool.headers[1], "header2");
  ASSERT_EQ(statPool.headers[2], "blub");*/
/*}*/

TEST(TestStatisticsTest, StatisticsPoolwriteStatisticsToOutputFile)
{
    // just call all function to test for segfault, atm nothing more possible
  // due to friend-test not working as i thought?!

  IntersectPerformanceEvaluation::StatisticsPool statPool;
  statPool.startMeasurement();
  statPool.setNrAttributeForThisMeasurement("bla", 3, 1);
  statPool.setAttributeForThisMeasurement("header2", "header2wert");
  statPool.setNrAttributeForThisMeasurement("blub", -1.0, 3);
  statPool.startMeasurement();
  statPool.setNrAttributeForThisMeasurement("bla", 3, 1);
  statPool.setAttributeForThisMeasurement("header2", "header2wert");
  statPool.setNrAttributeForThisMeasurement("blub", -1.0, 3);
  statPool.startRun();
  statPool.addTime("boost", 1.5);
  statPool.addTime("simple", 2.5);

  statPool.startRun();
  statPool.addTime("boost", 3.5);
  statPool.addTime("pthread", 5.8);
  statPool.startMeasurement();
  statPool.setAttributeForThisMeasurement("bla", "measurement3!");
  statPool.setNrAttributeForThisMeasurement("header2", 4.5, 3);
  statPool.setNrAttributeForThisMeasurement("blub", -3.0, 3);
  statPool.startRun();
  statPool.startMeasurement();
  statPool.startRun();
  statPool.addTime("boost", 4.5);
  statPool.addTime("pthread", 100.8);
  statPool.writeStatisticsToFile("ztestfilestatpool.txt");
}
TEST(TestStatisticsTest, MeasurementGetValue)
{
  IntersectPerformanceEvaluation::Measurement measurement;

  ASSERT_EQ(measurement.getAttribute("list1"), "unknown");
  measurement.setAttribute("list1", "-3.5");
  ASSERT_EQ(measurement.getAttribute("list1"), "-3.5");
  ASSERT_EQ(measurement.getAttribute("list2"), "unknown");
  ASSERT_EQ(measurement.getAttribute("result"), "unknown");

  measurement.setAttribute("result", "1000");

  ASSERT_EQ(measurement.getAttribute("list1"), "-3.5");
  ASSERT_EQ(measurement.getAttribute("list2"), "unknown");
  ASSERT_EQ(measurement.getAttribute("result"), "1000");

  measurement.setAttribute("list1", "zwanzig");

  ASSERT_EQ(measurement.getAttribute("list1"), "zwanzig");
  ASSERT_EQ(measurement.getAttribute("list2"), "unknown");
  ASSERT_EQ(measurement.getAttribute("result"), "1000");

  measurement.setNumberAttribute("list1", 3.23, 3);
  ASSERT_EQ(measurement.getAttribute("list1"), "3.230");
  ASSERT_EQ(measurement.getAttribute("list2"), "unknown");
  ASSERT_EQ(measurement.getAttribute("result"), "1000");

  measurement.setNumberAttribute("result", 5.55, 1);
  ASSERT_EQ(measurement.getAttribute("list1"), "3.230");
  ASSERT_EQ(measurement.getAttribute("list2"), "unknown");
  ASSERT_EQ(measurement.getAttribute("result"), "5.5");

  measurement.setNumberAttribute("list1", 2.0, 1);
  ASSERT_EQ(measurement.getAttribute("list1"), "2.0");
  ASSERT_EQ(measurement.getAttribute("list2"), "unknown");
  ASSERT_EQ(measurement.getAttribute("result"), "5.5");
}
TEST(TestStatisticsTest, MeasurementGetRuns)
{
  IntersectPerformanceEvaluation::Measurement measurement;
  measurement.startRun();

  measurement.addTime("boost", 3.4);
  measurement.addTime("pattern", 2.5);

  measurement.startRun();

  measurement.addTime("simple", 9.0);
  std::vector<IntersectPerformanceEvaluation::Run> allRuns;
  measurement.getRuns(&allRuns);

  ASSERT_EQ(allRuns[0].getTime("boost"), 3.4);
  ASSERT_EQ(allRuns[0].getTime("pattern"), 2.5);
  ASSERT_EQ(allRuns[0].getTime("simple"), -1.0);
  ASSERT_EQ(allRuns[1].getTime("boost"), -1.0);
  ASSERT_EQ(allRuns[1].getTime("pattern"), -1.0);
  ASSERT_EQ(allRuns[1].getTime("simple"), 9.0);
  measurement.addTime("simple", 2.0);
  measurement.getRuns(&allRuns);
  ASSERT_EQ(allRuns[1].getTime("boost"), -1.0);
  ASSERT_EQ(allRuns[1].getTime("pattern"), -1.0);
  ASSERT_EQ(allRuns[1].getTime("simple"), 2.0);
}
TEST(TestStatisticsTest, MeasurementGetTime)
{
  IntersectPerformanceEvaluation::Measurement measurement;
  measurement.startRun();

  measurement.addTime("boost", 3.4);
  measurement.addTime("pattern", 2.5);

  measurement.startRun();

  measurement.addTime("simple", 9.0);

  ASSERT_EQ(measurement.getTime(0, "boost"), 3.4);
  ASSERT_EQ(measurement.getTime(0, "pattern"), 2.5);
  ASSERT_EQ(measurement.getTime(0, "simple"), -1.0);
  ASSERT_EQ(measurement.getTime(1, "boost"), -1.0);
  ASSERT_EQ(measurement.getTime(1, "pattern"), -1.0);
  ASSERT_EQ(measurement.getTime(1, "simple"), 9.0);

  measurement.addTime("simple", 2.0);
  ASSERT_EQ(measurement.getTime(1, "boost"), -1.0);
  ASSERT_EQ(measurement.getTime(1, "pattern"), -1.0);
  ASSERT_EQ(measurement.getTime(1, "simple"), 2.0);
}
TEST(TestStatisticsTest, MeasurementGetAverageRuntime)
{
  IntersectPerformanceEvaluation::Measurement measurement;
  measurement.startRun();

  measurement.addTime("boost", 3.4);
  measurement.addTime("pattern", 2.5);

  ASSERT_EQ(measurement.getAverageRuntime("boost"), 3.4);
  ASSERT_EQ(measurement.getAverageRuntime("pattern"), 2.5);
  ASSERT_EQ(measurement.getAverageRuntime("simple"), -1.0);
  measurement.startRun();
  // should stay the same...
  ASSERT_EQ(measurement.getAverageRuntime("boost"), 3.4);
  ASSERT_EQ(measurement.getAverageRuntime("pattern"), 2.5);
  ASSERT_EQ(measurement.getAverageRuntime("simple"), -1.0);
  measurement.addTime("simple", 9.0);

  ASSERT_EQ(measurement.getAverageRuntime("boost"), 3.4);
  ASSERT_EQ(measurement.getAverageRuntime("pattern"), 2.5);
  ASSERT_EQ(measurement.getAverageRuntime("simple"), 9.0);

  measurement.addTime("simple", 2.0);

  ASSERT_EQ(measurement.getAverageRuntime("boost"), 3.4);
  ASSERT_EQ(measurement.getAverageRuntime("pattern"), 2.5);
  ASSERT_EQ(measurement.getAverageRuntime("simple"), 2.0);
  measurement.addTime("boost", 3.4);

  ASSERT_EQ(measurement.getAverageRuntime("boost"), 3.4);
  ASSERT_EQ(measurement.getAverageRuntime("pattern"), 2.5);
  ASSERT_EQ(measurement.getAverageRuntime("simple"), 2.0);
  measurement.startRun();

  measurement.addTime("boost", 3.4);
  measurement.addTime("pattern", 7.6);
  ASSERT_EQ(measurement.getAverageRuntime("boost"), 3.4);
  ASSERT_EQ(measurement.getAverageRuntime("pattern"), 5.05);
  ASSERT_EQ(measurement.getAverageRuntime("simple"), 2.0);
}
TEST(TestStatisticsTest, RunClass)
{
  IntersectPerformanceEvaluation::Run testRun;
  testRun.addTime("boost", 1.1);
  testRun.addTime("pthread", 3);
  ASSERT_EQ(testRun.getTime("boost"), 1.1);
  ASSERT_EQ(testRun.getTime("pthread"), 3);
  ASSERT_EQ(testRun.getTime("simple"), -1);

  testRun.addTime("boost", 0.2);
  ASSERT_EQ(testRun.getTime("simple"), -1);
  ASSERT_EQ(testRun.getTime("boost"), 0.2);
  ASSERT_EQ(testRun.getTime("pthread"), 3);

  testRun.addTime("simple", 0.9);

  ASSERT_EQ(testRun.getTime("simple"), 0.9);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
