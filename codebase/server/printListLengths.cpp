#include <iostream>
#include <getopt.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "CompleterBase.h"
#include "HYBCompleter.h"
#include "INVCompleter.h"

using namespace std;

void printUsage() 
{
  cout << "Usage: printListLengths [INV|HYB] index-file" << endl
       << endl
       << "-l" << endl
       << "     Use location information from specified index file (which must have been built accordingly)." << endl
       << endl
       << "-s" << endl
       << "     Use score information from specified index file (which must have been built accordingly)." << endl
       << endl
       << endl;
}

//
// GLOBAL VARIABLES (all set in main, many used in answerQueries)
//
string method;
string indexFileName;
string vocFileName;


//
// PRINT LIST LENGTHS
//
template<class Completer>
void printListLengths()
{
  Completer completer(indexFileName, vocFileName);
  completer.printAllListLengths();

} /* end of method printListLengths */


//
// MAIN : parse command line argument and call answerQueries
//
int main(int argc, char *argv[])
{
  cout << endl
       << EMPH_ON << "PRINT LIST LENGTHS" << EMPH_OFF << endl
       << endl;
 
  bool useLocations = false;
  bool useScores = false;


//
  // PROCESS COMMAND LINE ARGUMENTS (gloabal variables declared after printUsage() above)
  //
  while (true)
  {
    char c = getopt(argc, argv, "ls");
    if (c == -1) break;
    switch (c)
    {
      case 'l': // "ell" not 1
        useLocations = true;
        break;
      case 's': 
        useScores = true;
        break;
      default:
        cout << endl << "! ERROR in processing options (getopt returned '" << c << "' = 0x" << setbase(16) << int(c) << ")" << endl << endl;
        exit(1);
    }
  }

















  //
  // PROCESS COMMAND LINE ARGUMENTS (gloabal variables declared after printUsage() above)
  //
  /*
  while (true)
  {
    char c = getopt(argc, argv, "x");
    break;
  }
  */

  // first remaining argument must be one of INV or HYB
  if (optind >= argc) { printUsage(); exit(1); }
  method = argv[optind++];
  if (method != "INV" && method != "HYB") { printUsage(); exit(1); }

  // next argument must be name of index file (was: db name)
  if (optind >= argc) { printUsage(); exit(1); }
  indexFileName = argv[optind++];
  // vocabulary file is basename of index file + suffix .vocabulary
  vocFileName = indexFileName;
  vocFileName = vocFileName.erase(vocFileName.rfind('.')) + ".vocabulary";

  //
  // PRINT LIST LENGTHS according to the specified method
  //
  unsigned char MODE = 0;
  MODE += ((useLocations) ? (2) : (0)) + ((useScores) ? (4) : (0)); 
       if (method == "INV" && MODE == 0)
    printListLengths< INVCompleter< 0> >();
  else if (method == "INV" && MODE == WITH_SCORES + WITH_POS)
    printListLengths< INVCompleter<WITH_SCORES+WITH_POS+WITH_DUPS> >();
  else if (method == "INV" && MODE == WITH_POS)   
    //    { cout << endl << "! ERROR invalid mode (deactivated to reduce compile times)" << endl << endl; exit(1); }
       printListLengths< INVCompleter<WITH_POS+WITH_DUPS> >();
  else if (method == "INV" && MODE == WITH_SCORES)
    //    { cout << endl << "! ERROR invalid mode (deactivated to reduce compile times)" << endl << endl; exit(1); }
       printListLengths< INVCompleter<WITH_SCORES> >();
  else if (method == "HYB" && MODE == 0)
    printListLengths< HYBCompleter<WITH_DUPS> >();
  else if (method == "HYB" && MODE == WITH_POS + WITH_SCORES)
    printListLengths< HYBCompleter<WITH_DUPS + WITH_POS + WITH_SCORES> >();
  else if (method == "HYB" && MODE == WITH_POS)
    //    { cout << endl << "! ERROR invalid mode (deactivated to reduce compile times)" << endl << endl; exit(1); }
    printListLengths< HYBCompleter<WITH_DUPS + WITH_POS> >();
  else if (method == "HYB" && MODE == WITH_SCORES)
    //    { cout << endl << "! ERROR invalid mode (deactivated to reduce compile times)" << endl << endl; exit(1); }
    printListLengths< HYBCompleter<WITH_DUPS + WITH_SCORES> >();
  else {
    cout << endl << "! ERROR in main : invalid value for MODE" 
                 << " (" << (int) MODE << ")" << endl << endl; exit(1); }

}
