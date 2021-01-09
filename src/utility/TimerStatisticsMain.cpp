// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Ina Baumgarten <baumgari>.

#include <stdio.h>
#include <string>
#include <vector>
#include "../server/Timer.h"
#include "./TimerStatistics.h"
// Documentation about using the TimeStatistics.h.
// This class is used for printing a performance overview
// about the time and volume your programs or methods need.
int main(int argc, char* argv[])
{
  // For using this class you need to use /codebase/server/Timer.h
  // to compute how much time your programs need.

  // Specifiy the timer you are going the use.
  Timer t1, t1_1, t1_2, t2, t2_1, t2_1_1, t3, t4, t5, total;

  // There are 2 possibilities using this class:

  // 1. - You can define and declare the TimerStatistics
  //      after the timer have been running.
  // 2. - You can define and declare the TimerStatistics
  //      before the timer habe been running.
  // I am going to show the first possibilty.

  // Let your timers run. For simplifying I just set them here
  // directly to a given value (in microsecs).

  // SubTimers
  t1.setSecs(160);
  // SubTimers of subTimer t1. Just in case you want to split
  // one of the subTimers in some smaller pieces.
  t1_1.setSecs(80);
  t1_2.setSecs(40);

  t2.setSecs(10);
  // SubTimer of t2.
  t2_1.setSecs(9);
  // Subtimer of t2_1.
  t2_1_1.setSecs(5);

  // Some typical timers.
  t3.setSecs(20);
  t4.setSecs(9);
  t5.setSecs(5);

  // (Optional) TotalTimer: Measures the time of all the subTimers + maybe
  // forgotten time. You would start this timer before anyone of
  // the subTimers has been started and stop it, after all of
  // the subTimers have been running.
  total.setSecs(600);


  // Define and declare your TimerStatistics.
  // For subsubTimers you have to create an own TimerStatisticIntstance.
  TimerStatistics statisticTotal;
  TimerStatistics statisticT1;
  TimerStatistics statisticT2;
  TimerStatistics statisticT2_1;

  // Now you can add your subTimers to your TimerStatisticsInstances.
  // For this you need among other parameters a TimerUnit for each timer:
  //   AUTO, NSECS, USECS, MSECS, SECS, MINS, HOURS
  //   -- DAYS are based on an overflow not possible right now.

  // Add subTimers without own subTimers.
  //   void addSubTimer(const Timer& timer,
  //                   TimeUnit timerUnit,
  //                   const std::string& timerDescription,
  //                   bool showAlways);
  //   or
  //   void addSubTimerWithVolume(const Timer& timer,
  //                              TimeUnit timerUnit,
  //                              const std::string& timerDescription,
  //                              float rate,
  //                              std::string rateUnit,
  //                              std::string rateDescription,
  //                              bool showAlways);
  // You can specifiy your rate information. By inserting $RATE in your
  // rateDescription, you can specify where rate and rateUnit have to been
  // placed. If the string does not contain $RATE, rate and rateUnit are
  // just appended.

  std::string t1s = "SubTimer t1",
              t1_1s = "SubsubTimer t1_1",
              t1_2s = "SubsubTimer t1_2 (a)",
              t2s = "SubTimer t2 (a)",
              t2_1s = "SubsubTimer t2_1",
              t2_1_1s = "SubsubsubTimer t2_1_1",
              t3s = "SubTimer t3",
              t4s = "SubTimer t4",
              t5s = "SubTimer t5 (a)",
              totals = "Total timer";

  statisticTotal.addSubTimerWithComment(t3,  // timer
                                        statisticTotal.SECS,  // timerUnit
                                        t3s,  // timerDescription
                                        "Adding some comment here.",
                                        false);
                                        // just shown if it fits to the params
  statisticTotal.addSubTimerWithVolume(t4,  // timer
                      statisticTotal.SECS,  // timerUnit
                      t4s,  // timerDescription
                      1212.543245,  // rate
                      std::string("TB/s"),  // rateUnit as normal string
                      std::string("This timers processes $RATE data."
                                  "That's awesome!"),  // rateDescription
                      false);  // showAlways);
  // To regconize the alwaysShown subTimers
  // i'm going to append an (a) like always.
  statisticTotal.addSubTimer(t5, statisticTotal.AUTO,
                             t5s, true);
  statisticT1.addSubTimer(t1_1, statisticTotal.AUTO,
                          t1_1s, false);
  statisticT1.addSubTimer(t1_2, statisticTotal.AUTO,
                          t1_2s, true);
  statisticT2_1.addSubTimer(t2_1_1, statisticTotal.SECS,
                            t2_1_1s, true);

  // ... subTimers with own subTimer. The same like above + extra
  // TimerStatistics.
  statisticTotal.addSubTimer(t1, statisticTotal.AUTO,
                             t1s,
                             false, statisticT1);  // TimeStatistics
  statisticTotal.addSubTimerWithVolume(t2, statisticT2.AUTO,
                                       t2s,
                                       1.0, "KB/h",
                                       "Just processes $RATE data.",
                                       true, statisticT2);
  statisticT2.addSubTimerWithVolume(t2_1, statisticT2.AUTO,
                                    t2_1s, 1.0, "KB/h",
                                    "Ahh, found the bottleneck:", false,
                                    statisticT2_1);
  // Adding the total timer.
  statisticTotal.addTotalTimer(total, statisticTotal.AUTO,
                               totals);

  // So now you have some possibilties to print your statistic:

  // Default:
  // timePrecision = 1, => i.e, 19.3 secs
  // percPrecision = 0, => i.e. 45%
  // ratePrecision = 1, => i.e. 123.1 MB/s
  // percentageThreshold = 0, => all items are shown, if you specify
  //                             this parameter, items with less then
  //                             "percentageThreshold" percent
  //                             won't be printed.
  // sumOfPercentageThreshold = 100, => all items are shown, if you
  //                                    specify this parameter,
  //                                    TimerStatistics will just show
  //                                    the first "sumOfPercentageThreshold"
  //                                    percent.
  // numItemsThreshold = -1, => all items are shown, if you specify this
  //                            parameter, just the first "numItemsThreshold"
  //                            subTimers are going to be printed.
  //                            (not included items of subsubTimers).

  std::vector<std::string> stat;
  printf("Default:\n");
  stat = statisticTotal.getStatistics();
  for (size_t i = 0; i < stat.size(); i++)
    std::cout << stat[i] << std::endl;

  // Not default
  statisticTotal.set(3, 0, 1, 0, 100, -1);
  printf("\nPrecision of time:\n");
  stat = statisticTotal.getStatistics();
  for (size_t i = 0; i < stat.size(); i++)
    std::cout << stat[i] << std::endl;


  statisticTotal.set(1, 3, 0, 0, 100, -1);
  statisticTotal.setPrintOptions(false, false, true);
  printf("\nPrecsion of all:\n");
  stat = statisticTotal.getStatistics();
  for (size_t i = 0; i < stat.size(); i++)
    std::cout << stat[i] << std::endl;


  statisticTotal.set(1, 0, 1, 4, 100, -1);
  statisticTotal.setPrintOptions(true, false, true);
  printf("\nNo items with less than 4 percent:\n");
  stat = statisticTotal.getStatistics();
  for (size_t i = 0; i < stat.size(); i++)
    std::cout << stat[i] << std::endl;


  statisticTotal.set(1, 0, 1, 0, 100, 3);
  statisticTotal.setPrintOptions(true, true, true);
  printf("\nThe first three items:\n");
  stat = statisticTotal.getStatistics();
  for (size_t i = 0; i < stat.size(); i++)
    std::cout << stat[i] << std::endl;


  statisticTotal.set(1, 0, 1, 0, 50, 9);
  printf("\nThe sum of the item's percentage should be max 50\n");
  stat = statisticTotal.getStatistics();
  for (size_t i = 0; i < stat.size(); i++)
    std::cout << stat[i] << std::endl;
}
