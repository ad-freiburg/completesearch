// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Ina Baumgarten <baumgari>.

#include <assert.h>
#include <algorithm>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include "../server/Timer.h"
#include "./TimerStatistics.h"

size_t TimerStatistics::maxString = 0;
bool TimerStatistics::hierarchy = true;
bool TimerStatistics::printTotalTimer = true;
bool TimerStatistics::sortAfterPercentage = false;

// ____________________________________________________________________________
TimerStatisticsSubTimer::TimerStatisticsSubTimer(const Timer* timer,
                                TimerStatistics* subsubTimer)
  : timer(timer), subsubTimer(subsubTimer)
{
}

// ____________________________________________________________________________
TimerStatistics::TimerStatistics(int indentLevel, float percent)
    : indentLevel(indentLevel), percent(percent)
{
  _totalTimer = new TimerStatisticsSubTimer(new Timer());
  parametersSet = false;
  number = 0;
  timePrecision = 1;
  ratePrecision = 1;
  percPrecision = 0;
  percentageThreshold = 0;
  sumOfPercentageThreshold = 100;
  numItemsThreshold = -1;
  printTotalTimer = true;
  hierarchy = true;
  sortAfterPercentage = false;
}

// ____________________________________________________________________________
void TimerStatistics::addSubTimer(const Timer& timer,
                                  TimeUnit timerUnit,
                                  std::string timerDescription,
                                  bool showAlways)
{
  assert(timerUnit <= 7);
  TimerStatisticsSubTimer subTimer(&timer, NULL);
  subTimer.set = false;
  subTimer.timerDescription = timerDescription;
  subTimer.comment = "";
  subTimer.timerUnit = timerUnit;
  subTimer.showAlways = showAlways;
  subTimer.rate = 0;
  subTimer.number = this->number;
  number++;
  _subTimers.push_back(subTimer);
}

// ____________________________________________________________________________
void TimerStatistics::addSubTimerWithComment(const Timer& timer,
                                             TimeUnit timerUnit,
                                             std::string timerDescription,
                                             std::string comment,
                                             bool showAlways)
{
  assert(timerUnit <= 7);
  TimerStatisticsSubTimer subTimer(&timer, NULL);
  subTimer.set = false;
  subTimer.timerDescription = timerDescription;
  subTimer.comment = comment;
  subTimer.timerUnit = timerUnit;
  subTimer.showAlways = showAlways;
  subTimer.rate = 0;
  subTimer.number = this->number;
  number++;
  _subTimers.push_back(subTimer);
}

// ____________________________________________________________________________
void TimerStatistics::addSubTimer(const Timer& timer,
                                  TimeUnit timerUnit,
                                  std::string timerDescription,
                                  bool showAlways,
                                  TimerStatistics& subsubTimer)

{
  assert(timerUnit <= 7);
  TimerStatisticsSubTimer subTimer(&timer, NULL);
  subTimer.timerDescription = timerDescription;
  subTimer.timerUnit = timerUnit;
  subTimer.comment = "";
  subTimer.showAlways = showAlways;
  subTimer.rate = 0;
  subTimer.number = this->number;
  number++;
  setSubsubTimer(&subTimer, &subsubTimer);
  _subTimers.push_back(subTimer);
}

// ____________________________________________________________________________
void TimerStatistics::addSubTimerWithComment(const Timer& timer,
                                             TimeUnit timerUnit,
                                             std::string timerDescription,
                                             std::string comment,
                                             bool showAlways,
                                             TimerStatistics& subsubTimer)

{
  assert(timerUnit <= 7);
  TimerStatisticsSubTimer subTimer(&timer, NULL);
  subTimer.timerDescription = timerDescription;
  subTimer.timerUnit = timerUnit;
  subTimer.comment = comment;
  subTimer.showAlways = showAlways;
  subTimer.rate = 0;
  subTimer.number = this->number;
  number++;
  setSubsubTimer(&subTimer, &subsubTimer);
  _subTimers.push_back(subTimer);
}

// ____________________________________________________________________________
void TimerStatistics::addSubTimerWithVolume(const Timer& timer,
                                            TimeUnit timerUnit,
                                            std::string timerDescription,
                                            float rate,
                                            std::string rateUnit,
                                            std::string rateDescription,
                                            bool showAlways)
{
  assert(timerUnit <= 7);
  TimerStatisticsSubTimer subTimer(&timer,  NULL);
  subTimer.set = false;
  subTimer.timerDescription = timerDescription;
  subTimer.comment = "";
  subTimer.timerUnit = timerUnit;
  subTimer.rateUnit = rateUnit;
  subTimer.rateDescription = rateDescription;
  subTimer.number = this->number;
  number++;
  subTimer.rate = rate;
  subTimer.showAlways = showAlways;
  _subTimers.push_back(subTimer);
}

// ____________________________________________________________________________
void TimerStatistics::addSubTimerWithVolume(const Timer& timer,
                                            TimeUnit timerUnit,
                                            std::string timerDescription,
                                            float rate,
                                            std::string rateUnit,
                                            std::string rateDescription,
                                            bool showAlways,
                                            TimerStatistics &subsubTimer)
{
  assert(timerUnit <= 7);
  TimerStatisticsSubTimer subTimer(&timer, NULL);
  subTimer.timerDescription = timerDescription;
  subTimer.comment = "";
  subTimer.timerUnit = timerUnit;
  subTimer.rateUnit = rateUnit;
  subTimer.rateDescription = rateDescription;
  subTimer.rate = rate;
  subTimer.number = this->number;
  number++;
  subTimer.showAlways = showAlways;
  setSubsubTimer(&subTimer, &subsubTimer);
  _subTimers.push_back(subTimer);
}
// ____________________________________________________________________________
void TimerStatistics::addTotalTimer(const Timer& timer,
                                    TimeUnit timerUnit,
                                    std::string timerDescription)
{
  _totalTimer = new TimerStatisticsSubTimer(&timer);
  _totalTimer->set = false;
  _totalTimer->timerDescription = timerDescription;
  _totalTimer->timerUnit = timerUnit;
  _totalTimer->rate = 0;
  _totalTimer->showAlways = true;
}

// ____________________________________________________________________________
void TimerStatistics::setSubsubTimer(TimerStatisticsSubTimer* subTimer,
                                     TimerStatistics* subsubTimer)
{
  // Increment indentLevel of the subsubtimer-
  subsubTimer->indentLevel = indentLevel + 1;
  // Sets totalTimer.
  subsubTimer->addTotalTimer(*subTimer->timer, subTimer->timerUnit, "");
  // Set the subsubtimer of the subtimer.
  subTimer->subsubTimer = subsubTimer;
  // Confirm it is set.
  subTimer->set = true;
}

// Sort after old order.
bool sortAfterNumber(const TimerStatisticsSubTimer& sub1,
                                      const TimerStatisticsSubTimer& sub2)
{
  return sub1.number < sub2.number;
}

// ____________________________________________________________________________
std::vector<std::string> TimerStatistics::getStatistics()
{
  std::vector<TimerStatisticsSubTimer> _subTimersToPrint;
  std::vector<std::string> statistics;
  std::string indentString;
  double actualPerc = 0,
         timeAtAll = 0;

  // Inserts spaces if subsubtimer.
  if (!hierarchy) indentLevel = 0;
  for (int i = 0; i < indentLevel - 1; i++) indentString.append("  ");
  if (indentLevel > 0) indentString.append(" -");

  sort();

  // Sets number of items which should be printed.
  if (numItemsThreshold == -1) numItemsThreshold = _subTimers.size();
  if (indentLevel == 0) checkMaxLength();

  // Find subTimers which should be printed.
  for (size_t i = 0; i <_subTimers.size(); i++)
  {
    if (((_subTimers[i].percentage >= static_cast<float>(percentageThreshold))
         &&(numItemsThreshold > static_cast<int>(i))
         && (static_cast<int>(actualPerc) <= sumOfPercentageThreshold))
         || _subTimers[i].showAlways)
    {
      _subTimersToPrint.push_back(_subTimers[i]);
      actualPerc += _subTimers[i].percentage;
    }
    timeAtAll += static_cast<double>(_subTimers[i].timer->usecs());
  }

  if (!sortAfterPercentage)
  {
    std::sort(_subTimersToPrint.begin(), _subTimersToPrint.end(),
            sortAfterNumber);
  }

  // Print subTimers
  for (size_t i = 0; i < _subTimersToPrint.size(); i++)
  {
    if (hierarchy || !_subTimersToPrint[i].set)
    {
      std::string str = getSubTimerString(&_subTimersToPrint[i], indentString);
      statistics.push_back(str);
    }
    if (_subTimersToPrint[i].set)
    {
      if (!_subTimersToPrint[i].subsubTimer->parametersSet)
      {
       // Set parameters.
        _subTimersToPrint[i].subsubTimer->set(timePrecision,
                                       percPrecision,
                                       ratePrecision,
                                       percentageThreshold,
                                       sumOfPercentageThreshold,
                                       numItemsThreshold);
        _subTimersToPrint[i].subsubTimer->parametersSet = false;
      }
      // Create temp TimerStatistics.
      TimerStatistics tmp = *_subTimersToPrint[i].subsubTimer;
      // If not hierachy change timerDescription
      for (size_t j = 0; j < _subTimersToPrint[i].subsubTimer->_subTimers.size()
                      && !hierarchy; j++)
      {
        _subTimersToPrint[i].subsubTimer->_subTimers[j].timerDescription
          = _subTimersToPrint[i].timerDescription + " - "
          + _subTimersToPrint[i].subsubTimer->_subTimers[j].timerDescription;
      }
      std::vector<std::string> subsub;
      if (!hierarchy)
        subsub = _subTimersToPrint[i].subsubTimer-> getStatistics();
      else subsub = tmp.getStatistics(); // NOLINT
      statistics.insert(statistics.end(), subsub.begin(), subsub.end());
      *_subTimersToPrint[i].subsubTimer = tmp;
    }
  }

  // Print missing time.
  double missingTime = totalSum - timeAtAll;
  float missingPerc = missingTime / totalSum * percent;
  if (missingTime != 0 && _subTimersToPrint.size() != 0 && hierarchy)
  {
    // Convert missing time from usecs to msecs.
    missingTime = missingTime / 1000000.0;
    if (std::abs(missingTime) >= 1)
    {
      TimeUnit timerUnit = AUTO;
      std::string unit = checkTimeUnit(&missingTime, &timerUnit);
      std::string missingTimestring = "Missing time";
      std::string str = getStringRest(missingTimestring, missingTime, unit,
                                      missingPerc, indentString);
      statistics.push_back(str);
    }
  }

  // Print totalTimer.
  if (_totalTimer != NULL && indentLevel == 0 && printTotalTimer
      && _totalTimer->timerDescription != "")
  {
    std::string str;
    // Calculate time.
    double time = _totalTimer->timer->secs();
    assert(time >= 0);
    std::string timerUnit = checkTimeUnit(&time, &(_totalTimer->timerUnit));
    str = getStringTotal(_totalTimer->timerDescription,
                         time, timerUnit);
    statistics.push_back(str);
  }
  return statistics;
}

// _____________________________________________________________________________
std::string TimerStatistics::getStringTotal(const std::string& description,
                                            float time, std::string unit)
{
  // Define spaces.
  int timerSpacesL = -(this->maxString + 3) + 2*indentLevel;
  int timerSpacesR = -timerSpacesL,
      timePrecSpacesL = timePrecision + 5,
      timeUnitSpacesL = -12,
      timeUnitSpacesR = -timeUnitSpacesL;
  // getString
  char str[200];
  snprintf(str, sizeof(str), "%*.*s%*.*f %*.*s",
           timerSpacesL, timerSpacesR, description.c_str(),
           timePrecSpacesL, timePrecision, time,
           timeUnitSpacesL, timeUnitSpacesR, unit.c_str());

  std::string bold(str);
  // NEW(Hannah, 18Aug11): no bold for total timer.
  snprintf(str, sizeof(str), "%c[0m%s%c[0m", 0x1B,
                              bold.c_str(), 0x1B);
  // snprintf(str, sizeof(str), "%c[1m%s%c[0m", 0x1B,
  //                            bold.c_str(), 0x1B);

  return str;
}

// _____________________________________________________________________________
std::string TimerStatistics::getStringRest(const std::string& description,
                                           float time, std::string unit,
                                           float percentage,
                                           std::string indentString)
{
  // Define spaces.
  int timerSpacesL = -(this->maxString + 3) + 2*indentLevel;
  int timerSpacesR = -timerSpacesL,
      timePrecSpacesL = timePrecision + 5,
      timeUnitSpacesL = -12,
      timeUnitSpacesR = -timeUnitSpacesL,
      percPrecSpacesL = percPrecision + 3;
  // getString
  char str[200];
  snprintf(str, sizeof(str), "%s%*.*s%*.*f %*.*s%*.*f%%",
           indentString.c_str(), timerSpacesL, timerSpacesR,
           description.c_str(), timePrecSpacesL, timePrecision, time,
           timeUnitSpacesL, timeUnitSpacesR,
           unit.c_str(), percPrecSpacesL, percPrecision,
           percentage);

  if (indentLevel % 3 == 1)
  {
    std::string darkgrey(str);
    snprintf(str, sizeof(str), "%c[1;30m%s%c[0m", 0x1B,
                               darkgrey.c_str(), 0x1B);
  }
  else if (indentLevel % 3 == 2)
  {
    std::string grey(str);
    snprintf(str, sizeof(str), "%c[37m%s%c[0m", 0x1B, grey.c_str(),
                                                0x1B);
  }

  return str;
}

// _____________________________________________________________________________
std::string TimerStatistics::
  getSubTimerString(TimerStatisticsSubTimer * subTimer,
                    std::string indentString)
{
  // Define spaces.
  int timerSpacesL = -(this->maxString + 3) + 2*indentLevel;
  int timerSpacesR = -timerSpacesL,
      timePrecSpacesL = timePrecision + 5,
      timeUnitSpacesL = -12,
      timeUnitSpacesR = -timeUnitSpacesL,
      percPrecSpacesL = percPrecision + 3;

  // Specify additional string (the last column)
  std::string additionalString;
  if (subTimer->rate == 0) additionalString = subTimer->comment;
  else
  {
    additionalString = subTimer->rateDescription;
    char ratebuffer[20];
    // Create rate + Unit (120.2 MB/s)
    snprintf(ratebuffer, sizeof(ratebuffer), "%.*f %s", ratePrecision,
             subTimer->rate, subTimer->rateUnit.c_str());
    // If the rateDescription contains the placeholder,
    // substitute with the rate and its unit, else just append it.
    std::string placeholder = "$RATE";
    if (std::string::npos != additionalString.find(placeholder, 0))
    {
      additionalString.replace(additionalString.find(placeholder),
                               placeholder.length(), ratebuffer);
    }
    else
    {
      additionalString.append(" ");
      additionalString.append(ratebuffer);
      additionalString.append(".");
    }
  }
  // Calculate time.
  double time = subTimer->timer->secs();
  assert(time >= 0);
  std::string timerUnit = checkTimeUnit(&time, &subTimer->timerUnit);

  // getString
  char str[512];
  snprintf(str, sizeof(str), "%s%*.*s%*.*f %*.*s%*.*f%%      %*.*s",
           indentString.c_str(), timerSpacesL, timerSpacesR,
           subTimer->timerDescription.c_str(),
           timePrecSpacesL, timePrecision, time,
           timeUnitSpacesL, timeUnitSpacesR, timerUnit.c_str(),
           percPrecSpacesL, percPrecision, subTimer->percentage,
           -20, static_cast<int>(additionalString.length()) + 20,
           additionalString.c_str());
  if (indentLevel % 3 == 1)
  {
    std::string darkgrey(str);
    snprintf(str, sizeof(str), "%c[1;30m%s%c[0m", 0x1B,
                               darkgrey.c_str(), 0x1B);
  }
  else if (indentLevel % 3 == 2)
  {
    std::string grey(str);
    snprintf(str, sizeof(str), "%c[37m%s%c[0m", 0x1B, grey.c_str(),
                                                0x1B);
  }
  return str;
}

// _____________________________________________________________________________
std::string TimerStatistics::checkTimeUnit(double* time, TimeUnit* timerUnit)
{
  std::string unitstr;
  double factors[7];
  factors[0] = 0.000000001;
  factors[1] = 0.000001;
  factors[2] = 0.001;
  factors[3] = 1.0;
  factors[4] = 60.0;
  factors[5] = 3600.0;
  factors[6] = 24 * 3600.0;

  if (*timerUnit == 7)
  {
    if (*time < factors[2]) *timerUnit = USECS;
    if (*time >= factors[2]) *timerUnit = MSECS;
    if (*time >= factors[3]) *timerUnit = SECS;
    if (*time >= factors[4]) *timerUnit = MINS;
    if (*time >= factors[5]) *timerUnit = HOURS;
    if (*time >= factors[6]) *timerUnit = DAYS;
  }
  switch (*timerUnit)
  {
    case 0: unitstr = "nanosecs";
            break;
    case 1: unitstr = "microsecs";
            break;
    case 2: unitstr = "millisecs";
            break;
    case 3: unitstr = "secs";
            break;
    case 4: unitstr = "mins";
            break;
    case 5: unitstr = "hours";
            break;
    case 6: unitstr = "days";
            break;
    default: *time = -1.0; unitstr = "failure"; break;
  }
  *time = *time / factors[*timerUnit];
  return unitstr;
}

// _____________________________________________________________________________
void TimerStatistics::sort()
{
  totalSum = 0;
  if (_totalTimer != NULL)
    totalSum = static_cast<double>(_totalTimer->timer->usecs());
  else
  {
    for (size_t i = 0; i < _subTimers.size(); i++)
    {
      // Sum the time of all timers.
      totalSum += static_cast<double>(_subTimers[i].timer->usecs());
    }
  }

  // Computes how much percent every timer needs.
  for (size_t i = 0; i < _subTimers.size(); i++)
  {
    _subTimers[i].percentage =
      (static_cast<double>(_subTimers[i].timer->usecs()) / totalSum)*percent;

    if (_subTimers[i].set)
    {
      // If a subsubTimer exists, it's actual totalSum is the time of its
      // subtimer. Same with percent.
      _subTimers[i].subsubTimer->percent = _subTimers[i].percentage;
    }
  }
  // Calculate and sort with microseconds;
  int i = _subTimers.size();
  while (i > 1)
  {
    for (int j = 0; j < i - 1; j++)
    {
      if (_subTimers[j].timer->usecs() < _subTimers[j + 1].timer->usecs())
      {
        TimerStatisticsSubTimer tmp = _subTimers[j];
        _subTimers[j] = _subTimers[j + 1];
        _subTimers[j + 1] = tmp;
      }
    }
    i--;
  }
}

// ____________________________________________________________________________
int TimerStatistics::checkMaxLength()
{
  // If totalTimer is set, this->maxString is at least "Missing time:".length().
  size_t maxStringTot = 12 + indentLevel * 2;
  if (_totalTimer != NULL && this->maxString < maxStringTot)
    this->maxString = maxStringTot;

  size_t maxWithinIndent = 0;
  size_t maxOfSubTimers = 0;
  for (size_t i = 0; i < _subTimers.size(); i++)
  {
    // Computes how much space the first column in the table needs.
    if ((indentLevel * 2 + _subTimers[i].timerDescription.length()
        > this->maxString) && hierarchy)
       this->maxString = _subTimers[i].timerDescription.length() +
                         indentLevel*2;

    size_t max = 0;
    if (_subTimers[i].subsubTimer != NULL)
      max = _subTimers[i].subsubTimer->checkMaxLength() + 3;
    // Computes how much space the first column at max need if it is
    // nonhierarchic.
    if (!hierarchy)
    {
      if (_subTimers[i].timerDescription.length() > maxWithinIndent)
        maxWithinIndent = _subTimers[i].timerDescription.length();
      if (max > maxOfSubTimers) maxOfSubTimers = max;
    }
  }
  if (maxOfSubTimers > maxWithinIndent)
    maxWithinIndent += maxOfSubTimers;
  if (maxWithinIndent > this->maxString) this->maxString = maxWithinIndent;
  return maxWithinIndent;
}

// ____________________________________________________________________________
void TimerStatistics::set(int timePrecision,
                          int percPrecision,
                          int ratePrecision,
                          int percentageThreshold,
                          int sumOfPercentageThreshold,
                          int numItemsThreshold)
{
  this->timePrecision = timePrecision;
  this->percPrecision = percPrecision;
  this->ratePrecision = ratePrecision;
  this->percentageThreshold = percentageThreshold;
  this->sumOfPercentageThreshold = sumOfPercentageThreshold;
  this->numItemsThreshold = numItemsThreshold;
  parametersSet = true;
}

// ____________________________________________________________________________
void TimerStatistics::setPrecisions(int timePrecision,
                                    int percPrecision,
                                    int ratePrecision)
{
  this->timePrecision = timePrecision;
  this->percPrecision = percPrecision;
  this->ratePrecision = ratePrecision;
  parametersSet = true;
}

// ____________________________________________________________________________
void TimerStatistics::setThresholds(int percentageThreshold,
                                    int sumOfPercentageThreshold,
                                    int numItemsThreshold)
{
  this->percentageThreshold = percentageThreshold;
  this->sumOfPercentageThreshold = sumOfPercentageThreshold;
  this->numItemsThreshold = numItemsThreshold;
  parametersSet = true;
}
