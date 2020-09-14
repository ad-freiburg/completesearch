// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <iostream>

#include "./LogTemplates.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;


// Main function.
int main(int argc, char** argv)
{
  ad_utility::Log<DEBUG> debugLog;
  debugLog.debug("Should be visible");

  ad_utility::Log<ERROR> errorLog;
  errorLog.error("Templates here!");
  errorLog.debug("Should not be visible");

  size_t sum = 0;
  for (size_t i = 0; i < 10000000000; ++i)
  {
    errorLog.debug("i: ");
    sum += i;
  }

  errorLog.error("Sum is: ");
}
