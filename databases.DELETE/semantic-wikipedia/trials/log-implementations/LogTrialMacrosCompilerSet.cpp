// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <iostream>

#include "./LogMacrosCompilerSet.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;


// Main function.
int main(int argc, char** argv)
{
  LOG(ERROR) << "Macros compiler set here!" << endl;
  LOG(DEBUG) << "Should not be visible" << endl;

  size_t sum = 0;
  for (size_t i = 0; i < 10000000000; ++i)
  {
    LOG(DEBUG) << "i: " << i << endl;
    sum += i;
  }

  LOG(ERROR) << "Sum is: " << sum << endl;
}
