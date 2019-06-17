// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGTEMPLATES_H_
#define SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGTEMPLATES_H_

#include <iostream>
#include <string>

using std::string;

namespace ad_utility
{
#define DEBUG 0
#define INFO 1
#define WARN 2
#define ERROR 3
#define FATAL 4

//! Log
template<unsigned char LEVEL>
class Log
{
  public:
    Log()
    {
    }

    ~Log() {}

    inline void debug(const string& msg) const
    {
      if (LEVEL <= DEBUG)
      {
        std::cout << msg << std::endl;
      }
    }

    inline void error(const string& msg) const
    {
      if (LEVEL <= ERROR)
      {
        std::cout << msg << std::endl;
      }
    }

  private:
};
}

#endif  // SEMANTIC_WIKIPEDIA_TRIALS_LOG_IMPLEMENTATIONS_LOGTEMPLATES_H_
