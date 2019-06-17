#include "Globals.h"
#include "ExcerptsDB_NEW.h"

using namespace std;

//! USAGE INFO
void printUsage()
{
  cout << endl
       << "Usage: buildDocOffsets [options] <db>.docs" << endl 
       << endl
       << "Options:" << endl
       << endl
       << " -o <output file name>: default is <db>.docs.offsets" << endl
       << endl;
}

// PARAMETERS with defaults
string docsFileName;
string docOffsetsFileName;


//! MAIN LOOP
int main(char argc, char** argv)
{
  cout << endl << EMPH_ON << "BUILD DOC OFFSETS" << EMPH_OFF << endl << endl;

  // PARSE OPTIONS
  while (true)
  {
    int c = getopt(argc, argv, "o:");
    if (c == -1) break;
    switch (c)
    {
      case 'o': docOffsetsFileName = optarg; break;
      default : printUsage(); exit(1); break;
    }
  }
  
  // PARSE REMAINING PARAMETER (docs file name)
  if (optind >= argc) { printUsage(); exit(1); }
  docsFileName = argv[optind++];
  if (docOffsetsFileName == "") docOffsetsFileName = docsFileName + ".offsets";

  cout << "building doc offsets from file \"" << docsFileName << "\"" << endl << endl;

  ExcerptsDB_NEW docs;
  docs.buildOffsets(docsFileName, docOffsetsFileName);

  cout << "written offsets file \"" << docOffsetsFileName << "\"" << endl << endl;

} // end of MAIN


