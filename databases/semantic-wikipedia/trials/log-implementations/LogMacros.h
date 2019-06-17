// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGMACROS_H_
#define SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGMACROS_H_

#define LOG(x) if (x > ad_utility::Log::level()) ; else ad_utility::Log::getLog() //NOLINT

namespace ad_utility
{
enum LogLevel
{
  ERROR, DEBUG
};

//! Log
class Log
{
  public:

    static LogLevel& level()
    {
      return s_level;
    }

    static std::ostream& getLog()
    {
      return std::cout;
    }

  private:
    static LogLevel s_level;
};
LogLevel Log::s_level = DEBUG;
}

#endif  // SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGMACROS_H_
