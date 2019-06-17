// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <fstream>
#include "./CsvParser.h"
#include "./CsvParserOptions.h"


using std::string;
using std::ofstream;
using std::cout;
using std::endl;

// _____________________________________________________________________________
int main(int argc, char** argv)
{
  cout << "~~~~~~~~~~~~~~" << endl
       << "CsvParserPerf" << endl
       << "~~~~~~~~~~~~~~" << endl
       << "Performance test for CsvParserMain" << endl << endl;

  const string basename = string("perfbase");
  const string filename = basename + (".csv");
  const string firstline =
    string("field1\tfield2\tfield3\tfield4\tfield5\tfield6");
  const string record =
    string("Ich\tIch bin\tIch bin auch\tIch bin auch nur\t"
           "Ich bin auch nur ein\tIch bin auch nur ein Mensch");
  const int LINES_IN_CSV = 1000000;

  cout << "Prepare needed filestreams... " << flush;
  // Create test file.
  ofstream file(filename.c_str());
  if (!file.is_open())
  {
    cout << "Could not open file for writing." << endl
         << endl;
    exit(1);
  }
  file << firstline << endl;
  for (int i = 0; i < LINES_IN_CSV; ++i)
  {
    file << record << endl;
  }
  cout << "done." << endl;

  std::stringstream cmd;
  cmd << "./CsvParserMain --base-name=" << basename
      << " --full-text=field1,field2,field3,field4,field5,field6 > /dev/null";

  cout << "Calling CsvParserMain... " << flush;
  // Start runtime test.
  timeval start, end;
  double t1, t2, elapsed, linespeed;
  gettimeofday(&start, NULL);
  FILE* fp = popen(cmd.str().c_str(), "w");
  pclose(fp);
  gettimeofday(&end, NULL);
  cout << "done." << endl << endl << "Results:" << endl;
  // Evaluate result.
  t1 = start.tv_sec + (start.tv_usec/1000000.0);
  t2 = end.tv_sec + (end.tv_usec/1000000.0);
  elapsed = t2 - t1;
  linespeed = LINES_IN_CSV/elapsed;

  cout << "Time needed   : " << elapsed << " seconds" << endl;
  cout << "Record size   : " << record.size() << " Byte" << endl;
  cout << "Records total : " << LINES_IN_CSV << endl;
  double speedMb = (LINES_IN_CSV * record.size()) /
                   static_cast<double>(1024 * 1024) / elapsed;
  cout << "Line speed    : " << linespeed << " lines per second" << endl
       << "Byte speed    : " << speedMb << " MB per second" << endl
       << endl;

  // rm generated files.
  cout << "Cleanup... " << flush;
  char command[256];
  snprintf(command, sizeof(command), "rm -f %s.*", basename.c_str());
  int retval = system(command);
  if (retval == -1)
  {
    perror("");
    exit(1);
  }
  cout << "done." << endl;
  remove(filename.c_str());
  return 0;
}
