// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGMACROSCOMPILERSET_H_
#define SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGMACROSCOMPILERSET_H_

#include <time.h>
#include <sys/timeb.h>
#include <string>
#include <sstream>
#include <iostream>

#define LOG(x) if (x > LOGLEVEL) ; else ad_utility::Log::getLog<x>() //NOLINT

#define DEBUG 4
#define INFO 3
#define WARN 2
#define ERROR 1
#define FATAL 0

#ifndef LOGLEVEL
#define LOGLEVEL 2
#endif

using std::string;

namespace ad_utility
{
//! Log
class Log
{
    public:
    template<unsigned char LEVEL>
    static std::ostream& getLog()
    {
      return std::cout << ad_utility::Log::getTimeStamp() << "\t- "
          << ad_utility::Log::getLevel<LEVEL>() << '\t';
    }

    static string getTimeStamp()
    {
      struct timeb timebuffer;
      char timeline[26];

      ftime(&timebuffer);
      ctime_r(&timebuffer.time, timeline);
      timeline[19] = '.';
      timeline[20] = 0;

      std::ostringstream os;
      os << timeline << timebuffer.millitm;
      return os.str();
    }

    template<unsigned char LEVEL>
    static string getLevel()
    {
      if (LEVEL == DEBUG)
      {
        return "DEBUG:";
      }
      if (LEVEL == INFO)
      {
        return "INFO:";
      }
      if (LEVEL == WARN)
      {
        return "WARN:";
      }
      if (LEVEL == ERROR)
      {
        return "ERROR:";
      }
      if (LEVEL == FATAL)
      {
        return "FATAL:";
      }
    }
};
}

#endif  // SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGMACROSCOMPILERSET_H_
