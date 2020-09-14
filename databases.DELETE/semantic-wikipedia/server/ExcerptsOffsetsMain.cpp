// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>

#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./ExcerptOffsetCollection.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::flush;
using std::cerr;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

// Available options.
struct option options[] =
{
  {NULL, 0, NULL, 0}
};

// Main function.
int main(int argc, char** argv)
{
  std::cout << std::endl << EMPH_ON
      << "ExcerptsOffsetsMain (rudimentary excerpts DB), version " << __DATE__
      << " " << __TIME__ << EMPH_OFF << std::endl << std::endl;


  optind = 1;
  // Process command line arguments.
  while (true)
  {
    int c = getopt_long(argc, argv, "", options, NULL);
    if (c == -1) break;
    switch (c)
    {
    default:
      cout << endl
          << "! ERROR in processing options (getopt returned '" << c
          << "' = 0x" << std::setbase(16) << static_cast<int> (c) << ")"
          << endl << endl;
      exit(1);
    }
  }

  vector<string> fulltextBases;
  while (optind < argc)
  {
    fulltextBases.push_back(argv[optind++]);
  }

  // Start the Program
  for (size_t i = 0; i < fulltextBases.size(); ++i)
  {
    LOG(INFO) << "Getting byte offsets for excerpts from file: "
        << fulltextBases[i] << ".docs-by-contexts.ascii." << endl;
    ad_semsearch::ExcerptOffsetCollection offsets;
    offsets.createFromDocsfile(fulltextBases[i] + ".docs-by-contexts.ascii");
    offsets.writeToFile(fulltextBases[i] + ".docs-offsets");
    LOG(INFO) << "Done. Written file: " << fulltextBases[i]
        << ".docs-offsets." << endl;
  }
  return 0;
}
