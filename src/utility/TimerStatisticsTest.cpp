// Copyright 2010 <Ina Baumgarten>
// // E-Mail: baumgari@informatik.uni-freiburg.de

#include <string>
#include <vector>
#include <stdio.h> //NOLINT
#include <gtest/gtest.h> //NOLINT
#include "../server/Timer.h"
#include "./TimerStatistics.h"
TEST(constructorTests, TimerStatistics)
{
  TimerStatistics ts;
  // Check if default values are set correctly.
  ASSERT_FALSE(ts.parametersSet);
  ASSERT_EQ(0, ts.indentLevel);
  ASSERT_EQ(100, ts.percent);
  ASSERT_EQ(0, ts.number);
  ASSERT_EQ(1, ts.timePrecision);
  ASSERT_EQ(1, ts.ratePrecision);
  ASSERT_EQ(0, ts.percPrecision);
  ASSERT_EQ(0, ts.percentageThreshold);
  ASSERT_EQ(100, ts.sumOfPercentageThreshold);
  ASSERT_EQ(-1, ts.numItemsThreshold);

  // Check if subTimerStatistics values are set correctly.
  TimerStatistics ts2(5, 2);
  ASSERT_FALSE(ts2.parametersSet);
  ASSERT_EQ(5, ts2.indentLevel);
  ASSERT_EQ(2, ts2.percent);
  ASSERT_EQ(0, ts2.number);
  ASSERT_EQ(1, ts2.timePrecision);
  ASSERT_EQ(1, ts2.ratePrecision);
  ASSERT_EQ(0, ts2.percPrecision);
  ASSERT_EQ(0, ts2.percentageThreshold);
  ASSERT_EQ(100, ts2.sumOfPercentageThreshold);
  ASSERT_EQ(-1, ts2.numItemsThreshold);
}

TEST(publicTimerTests, addTimer)
{
  TimerStatistics ts, sub;
  Timer t1, tsub1, tsub2;
  t1.setSecs(160);
  tsub1.setSecs(20);
  tsub2.setSecs(40);
  // Although it is true, it will just be shown, if its subTimer is printed.
  sub.addSubTimer(tsub2, sub.AUTO, "subT", true);
  sub.addSubTimerWithComment(tsub1, sub.MSECS, "subT2", "comment", false);
  {
    // Test addSubTimer without additional subTimer
    ts.addSubTimer(t1, ts.AUTO, "dumdidum", false);
    ASSERT_EQ("dumdidum", ts._subTimers[0].timerDescription);
    ASSERT_EQ(ts.AUTO, ts._subTimers[0].timerUnit);
    ASSERT_EQ("", ts._subTimers[0].comment);
    ASSERT_FALSE(ts._subTimers[0].showAlways);
    ASSERT_EQ(0, ts._subTimers[0].rate);
    ASSERT_EQ("", ts._subTimers[0].rateDescription);
    ASSERT_EQ(0, ts._subTimers[0].number);
    ASSERT_EQ(1, ts.number);
    ASSERT_EQ(NULL, ts._subTimers[0].subsubTimer);
  }
  {
    // Test addSubTimerWithVolume without additional subTimer
    ts.addSubTimerWithVolume(t1, ts.AUTO, "dumdidum", 1.5, "MB/s",
                             "blub", false);
    ASSERT_EQ("dumdidum", ts._subTimers[1].timerDescription);
    ASSERT_EQ(ts.AUTO, ts._subTimers[1].timerUnit);
    ASSERT_EQ("", ts._subTimers[1].comment);
    ASSERT_FALSE(ts._subTimers[1].showAlways);
    ASSERT_FLOAT_EQ(1.5, ts._subTimers[1].rate);
    ASSERT_EQ("blub", ts._subTimers[1].rateDescription);
    ASSERT_EQ(1, ts._subTimers[1].number);
    ASSERT_EQ(2, ts.number);
    ASSERT_EQ(NULL, ts._subTimers[1].subsubTimer);
  }
  {
    // Test addSubTimerWithComment with additional subTimer
    ts.addSubTimerWithComment(t1, ts.SECS, "dumdidum", "blub", true, sub);
    ASSERT_EQ("dumdidum", ts._subTimers[2].timerDescription);
    ASSERT_EQ(ts.SECS, ts._subTimers[2].timerUnit);
    ASSERT_EQ("blub", ts._subTimers[2].comment);
    ASSERT_EQ(true, ts._subTimers[2].showAlways);
    ASSERT_EQ(0, ts._subTimers[2].rate);
    ASSERT_EQ("", ts._subTimers[2].rateDescription);
    ASSERT_EQ(2, ts._subTimers[2].number);
    ASSERT_EQ(3, ts.number);
    ASSERT_EQ(&sub, ts._subTimers[2].subsubTimer);
  }
}

TEST(publicTimerTests, setPrecisions)
{
}

TEST(publicTimerTests, setThresholds)
{
}

TEST(publicTimerTests, getStatistics)
{
}

TEST(privateTimerTests, sort)
{
}

TEST(privateTimerTests, checkTimeUnit)
{
}

TEST(privateTimerTests, checkMaxLength)
{
}

TEST(privateTimerTests, setSubsubTimer)
{
}
