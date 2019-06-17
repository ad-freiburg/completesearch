// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "../utils/File.h"
#include "../utils/Timer.h"

using std::string;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

// Available options.
struct option options[] = { {"file", required_argument, NULL, 'f'}, {NULL, 0,
                                                                     NULL, 0}};

// Main function.
int main(int argc, char** argv)
{
  cout << endl << "FileReadlineSafetyCheckTrial, version " << __DATE__ << " "
      << __TIME__ << endl << endl;

  // Init variables that may or may not be
  // filled / set depending on the options.
  string filename = "";

  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "f:", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    case 'f':
      filename = optarg;
      break;
    default:
      cout << endl << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  try
  {
    semsearch::Timer t;

    {
      std::fstream fileAsStream(filename.c_str(), std::ios::in);
      size_t nofLines = 0;
      t.start();
      string line;
      while (std::getline(fileAsStream, line))
        ++nofLines;
      t.stop();

      cout << endl << endl << "Read: " << nofLines << " lines." << endl
          << flush;

      cout << endl << endl << "Reading the whole file took: " << t.secs()
          << " seconds" << endl << flush;
    }

    {
      semsearch::File file(filename.c_str(), "r");
      size_t nofLines = 0;

      string line;
      t.start();
      while (file.readLine(&line))
        ++nofLines;
      t.stop();

      cout << endl << endl << "Read: " << nofLines << " lines." << endl
          << flush;

      cout << endl << endl << "Reading the whole file took: " << t.secs()
          << " seconds" << endl << flush;
    }

    {
      semsearch::File file(filename.c_str(), "r");
      size_t nofLines = 0;

      string line;
      char buf[1000];
      t.start();
      while (file.readLine(&line, buf, 1000))
        ++nofLines;
      t.stop();

      cout << endl << endl << "Read: " << nofLines << " lines." << endl
          << flush;

      cout << endl << endl << "Reading the whole file took: " << t.secs()
          << " seconds" << endl << flush;
    }


    {
      string line;
      semsearch::File file2(filename.c_str(), "r");
      char bufTooShort[20];
      size_t nofLines2 = 0;
      t.start();
      while (file2.readLine(&line, bufTooShort, 20))
        ++nofLines2;
      t.stop();
      cout << endl << endl << "Read: " << nofLines2 << " lines." << endl
          << flush;

      cout << endl << endl << "Reading the whole file took: " << t.secs()
          << " seconds" << endl << flush;
    }
  }
  catch(semsearch::Exception e)
  {
    cout << endl << endl << e.getErrorDetails() << endl << flush;
  }

  return 0;
}
