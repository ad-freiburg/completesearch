#include <iostream>
#include <string>
#include "CompleterBase.h"
#include "INVCompleter.h"
#include "HYBCompleter.h"

using namespace std;

void printUsage() 
{
  cout << EMPH_ON << "Usage: buildIndex [INV|HYB] words-file [-o db_name] [-b block_volume]" << EMPH_OFF << endl
       << endl
       << "Builds an index (INV or HYB, as specified) from the given words file" << endl
       << endl
       << "-o index_file_name" << endl
       << "     specifies the name of the index file name. Vocabulary file will be named by the base name of" << endl
       << "     the index file + the suffix .vocabulary (will currently be be overwritten if it exists)." << endl
       << endl
       << "-b block_volume|prefix_file" << endl
       << "     if argument parses to a number: put (approximately) this many word-in-doc pairs into a single block" << endl
       << "     otherwise, take as name of file containing block boundaries (one word per line)" << endl
       << "     *** NOTE: if block_volume < 10, make one block for each prefix of this size ***" << endl
       << endl
       << "-C" << endl
       << "     show details of Huffman decompression (whether better or worse than trivial encoding)" << endl
       << endl
       << "-L" << endl
       << "     do *not* include locations in build" << endl
       << "     *** NOTE: the old -l option did the opposite, locations are now default ***" << endl
       << endl
       << "-S" << endl
       << "     do *not* include scores in build" << endl
       << "     *** NOTE: the old -s option did the opposite, scores are now default ***" << endl
       << endl
       << "-f [ASCII|BINARY|SHORT]" << endl
       << "     format of words file. Default is ASCII: four columns separated by tabs (1 = word, 2 = doc id, 3 = score,"
       << "     4 = position), all written in ASCII. BINARY is the same thing with each of the four writte as a 4-byte"
       << "     integer. SHORT is like ASCII but without position." << endl
       << endl
       << "-M max_block_volume" << endl
       << "     ignore blocks with more than the specified number of items. To avoid program crash for blocks > 2GB," << endl
       << "     e.g., 'the' for terabyte. Default value is UINT_MAX." << endl
       << endl;
}

//
// GLOBAL VARIABLES (all set in main, many used in buildIndex)
//
string wordsFileName;
string vocFileName;
string indexFileName;
string format;

//
// BUILD AN INDEX (index file names, etc. from global variables)
//
template<class Completer, class Index>
void buildIndex()
{
  if (maxBlockVolume < UINT_MAX) 
  {
    cout << "* NOTE: -M option used, blocks with more than " << commaStr(maxBlockVolume)
         << " items will be truncated (with a warning)" << endl << endl;
  }
  Index index(indexFileName, vocFileName, Completer::mode());
  index.build(wordsFileName, format);
  index.showMetaInfo();
  //  Completer completer();
  //  completer.buildIndex(wordsFileName, indexFileName, vocFileName, format);
  //  completer.showMetaInfo();
}


//
// MAIN : parse command line argument and call buildIndex
//
int main(int argc, char *argv[])
{
  cout << endl
       << EMPH_ON << "BUILD INDEX (" << VERSION << ")" << EMPH_OFF << endl 
       << endl;
 
  //
  // PROCESS COMMAND LINE ARGUMENTS
  //

  // NEW 08Oct06 (Holger): locations and scores now default!
  bool useLocations = true;
  bool useScores = true;
  format = "ASCII";
  while (true)
  {
    char c = getopt(argc, argv, "Cb:f:o:LSM:");
    if (c == -1) break;
    switch (c)
    {
      case 'C':
        SHOW_HUFFMAN_STAT = true;
        break;
      case 'L':
        useLocations = false;
        break;
      case 'S':
        useScores = false;
        break;
      case 'b':
        // HYB_... variables defined in Globals.h 
        if (atoi(optarg)) HYB_BLOCK_VOLUME = atoi(optarg); 
        else HYB_BOUNDARY_WORDS_FILE_NAME = string(optarg);
        break;
      case 'f':
        format = optarg;
        break;
      case 'o':
        indexFileName = optarg;
        break;
      case 'M':
        maxBlockVolume = atoi(optarg);
        break;
      default:
        cout << endl << "! ERROR in processing options (getopt returned '" << c << "')" << endl << endl;
        exit(1);
    }
  }

  // first remaining argument must be one of INV or HYB
  if (optind >= argc) { printUsage(); exit(1); }
  string method = argv[optind++];
  if (method != "INV" && method != "HYB") { printUsage(); exit(1); }

  // next argument must be name of words file
  if (optind >= argc) { printUsage(); exit(1); }
  wordsFileName = argv[optind++];

  // if no index file name specified with -o option, take the basename
  // of the words file, and add .inverted or .hybrid (e.g. homeopathy.hybrid
  // for /KM/ir/what/ever/path/here/homeopathy.words). For the vocabulary
  // file, take the basename + suffix .vocabulary
  string dbName = indexFileName.empty() ? wordsFileName : indexFileName;
  dbName.erase(dbName.rfind('.'));
  if (indexFileName.empty())
  {
    if (method == "INV") indexFileName = dbName + ".inverted";
    else if (method == "HYB") indexFileName = dbName + ".hybrid";
    else { cout << endl << "! YOU SHOULD NEVER SEE THIS" << endl << endl;  exit(1); }
  }
  vocFileName = dbName + ".vocabulary";
  if (format == "ASCII") wordsFileName = dbName + ".words-sorted.ascii";
  if (format == "BINARY") wordsFileName = dbName + ".words-sorted.binary";

  //
  // BUILD AN INDEX of the specified type
  //
  unsigned char MODE = 0;
  MODE += ((useLocations) ? (2) : (0)) + ((useScores) ? (4) : (0));
  try 
  {
    if (method == "HYB" && MODE == WITH_POS + WITH_SCORES)
        buildIndex< HybCompleter<WITH_DUPS + WITH_POS + WITH_SCORES>, HYBIndex >();
    #ifdef COMPILE_INV
    else if (method == "INV" && MODE == WITH_POS + WITH_SCORES)
      buildIndex< INVCompleter<WITH_DUPS + WITH_POS + WITH_SCORES>, INVIndex  >();
    #endif
    #ifdef ALL_INDEX_VARIANTS
    else if (method == "INV" && MODE == WITH_POS)    
      buildIndex< INVCompleter<WITH_POS+WITH_DUPS>, INVIndex >();
    else if (method == "INV" && MODE == WITH_SCORES)
      buildIndex< INVCompleter<WITH_SCORES>, INVIndex  >();
    else if (method == "INV" && MODE == WITH_SCORES + WITH_POS)
      buildIndex< INVCompleter<WITH_SCORES+WITH_POS+WITH_DUPS>, INVIndex  >();
    else if (method == "INV" && MODE == 0)
      buildIndex< INVCompleter< 0>, INVIndex  >();
    else if (method == "HYB" && MODE == 0)
      buildIndex< HybCompleter<WITH_DUPS>, HYBIndex >();
    else if (method == "HYB" && MODE == WITH_POS + WITH_SCORES)
      buildIndex< HybCompleter<WITH_DUPS + WITH_POS + WITH_SCORES>, HYBIndex >();
    else if (method == "HYB" && MODE == WITH_POS)
      buildIndex< HybCompleter<WITH_DUPS + WITH_POS>, HYBIndex >();
    else if (method == "HYB" && MODE == WITH_SCORES)
      buildIndex< HybCompleter<WITH_DUPS + WITH_SCORES>, HYBIndex >();
    #endif
    else {
      cout << endl 
           << "! ERROR in main : invalid method / mode" 
           << " (" << method << " / " << (int) MODE << ")" 
           << endl << endl; exit(1); }
  }
  catch (Exception e)
  {
    cout << "! " << e.getFullErrorMessage() << endl << endl;
  }
  catch (exception e)
  {
    cout << "! std exception caught: " << e.what() << endl << endl;
  }
}// end: main()
