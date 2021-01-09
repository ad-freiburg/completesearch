
// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Jens Hoffmann <hoffmaje@informatik.uni-freiburg.de>, Hannah Bast
// <bast@informatik.uni-freiburg.de>

#include "./CsvParser.h"

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[0m"

int main(int argc, char** argv)
{
  cout << endl
       << EMPH_ON << "General-purpose CSV parser, version "
                  << __DATE__ << " " << __TIME__ << EMPH_OFF << endl
       << endl;
  // TODO(bast): options should be a member of CsvParser, and CsvParser should
  // have a method parseCommandLineOptions that sets this options member
  // according to the command line arguments. That's more intuitive and also
  // more consistent with how it's done in ParserBase.
  CsvParser parser;
  parser.parseCommandLineOptions(argc, argv);
  parser.parse();
}
