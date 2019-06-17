#include "Globals.h"
#include "DocsDB.h"

#define MY_MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MY_MAX(a,b) ( (a) < (b) ? (b) : (a) )

void printUsage() 
{ 
  cout << "Usage: buildDocsDB [-l] [-m] [-f] <db>.docs" << endl
       << endl
       << "produces file <db>.docs.DB" << endl
       << endl
       << "-l to specify compression level, from 0 (no compr., fast) to"
          "   9 (max. compr., slow); -1 is like 0 but about 10 times faster"
          "   because it doesn't use zlib (which computes checksums etc.)" << endl
       << "-m to specify the minimum size, below which a document is not compressed" << endl
       << "-f forces overwrite, if <db>.docs.DB already exists" << endl
       << endl;
}

int compressionLevel = 6; // default value (from 0..9)
unsigned int minSize = 100; // no compression for docs with less bytes
bool forceOverwrite = false;

// NOTE 07Aug07 (Holger): I checked that for lines of less than a hundred bytes,
// the compressions can cause a slight blowup. Therefore, it might be worth
// making the compression level depend on the line size, especially if most
// lines are short. (No issue for Wikipedia, however.)

// NOTE 07Aug07 (Holger): On ester-large.docs with 5.8 GB size, gzip took about
// 8 minutes and compressed to 1.9 GB, while buildDocsDB took about 11 minutes
// and compressed to 2.4 GB. That's reasonable, given the linewise compression.

int main(int argc, char** argv)
{
  // TITLE
  cout << endl << EMPH_ON << "BUILD NEW (FILE-BASED & COMPRESSED) DOCS DATABASE"
       << " (" << VERSION << ")" << EMPH_OFF << endl << endl;

  // PARSE COMMAND LINE
  while (true)
  {
    int c = getopt(argc, argv, "l:m:f");
    if (c == -1) break;
    switch (c)
    {
      case 'l': compressionLevel = atoi(optarg); break;
      case 'm': minSize = atoi(optarg); break;
      case 'f': forceOverwrite = true; break;
      default : printUsage(); exit(1); break;
    }
  }
  if (optind >= argc) { printUsage(); exit(1); }
  string inFileName = argv[optind++];
  string outFileName = inFileName + ".DB";
 
  // CHECK IF OUTPUT FILE ALREADY EXISTS
  if (access(outFileName.c_str(), F_OK) == 0 && forceOverwrite == false)
  {
    cerr << "file \"" << outFileName << "\" already exists"
         << ", run again with -f to force overwrite" << endl << endl;
    exit(1);
  }

  // CHECK PARAMETERS
  if (compressionLevel < -1 || compressionLevel >= 9)
  {
    cerr << "compression level must be in 0..9 or -1 (no compression)"
         << endl;
    exit(1);
  }

  // SHOW PARAMETERS
  cout << "compression level is " << compressionLevel;
  if (compressionLevel > 0)
    cout << ", documents of less than " << minSize << " bytes are not compressed";
  cout << endl << endl;

  // ACTUAL BUILD
  DocsDB::build(inFileName, outFileName, compressionLevel, minSize);

  return 0;
}





// REST TO BE DELETED SOON !!!
#if (0)

  // OPEN FILES (and get size of input file)
  FILE* in_file = fopen(inFileName.c_str(), "r");
  if (in_file == NULL) { perror("fopen docs file for reading"); exit(1); }
  FILE* out_file = fopen(outFileName.c_str(), "w");
  if (out_file == NULL) { perror("fopen db file for writing"); exit(1); }
  struct stat buf;
  int ret = stat(inFileName.c_str(), &buf);
  assert(ret == 0);
  off_t in_size = buf.st_size;


  // OFFSETS AND DOCS IDS (will be appended to end of file)
  if (sizeof(off_t) != 8) { cerr << "ERROR: sizeof(off_t) = "
                                 << sizeof(off_t) << " (want 8)" << endl
                                 << endl; exit(1); }
  vector<off_t> offsets;
  vector<DocId> docIds;

  // READ, COMPRESS, WRITE (line by line)
  cout << "compressing \"" << inFileName << "\" doc by doc " << flush; 
  timer.start();
  off_t milestone = 0;
  off_t mile = MY_MAX(1, in_size / 10);
  char* in_buf = new char[MAX_IN_DOC_SIZE + 1];   // see defintion above
  char* out_buf = new char[MAX_OUT_DOC_SIZE + 1]; // dito
  while (true)
  {
    // read next line and remember doc id (exit loop if end of file)
    char* ret1 = fgets(in_buf, MAX_IN_DOC_SIZE + 1, in_file);
    if (ret1 == NULL) { assert(feof(in_file)); break; }
    DocId docId = atoi(in_buf);
    docIds.push_back(docId);

    uLong in_len = strlen(in_buf);
    uLongf out_len;

    // if -1 specified, no compression (TODO: don't know yet if that works)
    cout << "*" << flush;
    if (compressionLevel == -1)
    {
      cout << "!" << flush;
      out_len = in_len;
      memcpy(out_buf, in_buf, in_len);
    }

    // else compress with specified level (in 0..9)
    else
    {
      out_len = MAX_OUT_DOC_SIZE;
      int ret2 = compress2((Bytef*)(out_buf), &out_len, 
                           (Bytef*)(in_buf), in_len, 
                           compressionLevel);
      // error handling
      if (ret2 != Z_OK)
      {
        cerr << endl << endl << "ERROR compressing:" << flush;
        switch (ret2)
        {
          case Z_MEM_ERROR : cerr << "not enough memory"; break;
          case Z_BUF_ERROR : cerr << "output buffer too small"; break;
          case Z_STREAM_ERROR : cerr << "invalid compression level"; break;
          default : cerr << "unknown error code (" << ret2 << ")"; break;
        }
        cerr << endl << endl;
        cerr << "TODO: deal with these kinds of errors more gracefully";
        cerr << endl << endl;
        exit(1);
      }
    }

    // write compressed line
    offsets.push_back(ftello(out_file));
    unsigned int ret3 = fwrite(out_buf, 1, out_len, out_file);
    if (ret3 != out_len) { cerr << endl << endl << "ERROR fwrite compressed line:"
                                << " (" << ret3 << " != " << out_len << ")" 
                                << endl << endl; exit(1); }

    // show progress (one . every 10%, see definition of mile above)
    while (ftello(in_file) > milestone) { cout << "." << flush; milestone += mile; }
    /*
    unsigned int perc = (100*out_len)/in_len;
    if (perc >= 100) cout << "[" << in_len << " -> " << out_len << "] " << flush;
    */
  }
  fclose(in_file);
  delete[] in_buf;
  delete[] out_buf;
  timer.stop();
  DocId nofDocs = offsets.size();
  cout << " done " << nofDocs << " docs in " << timer 
       << " (" << in_size / timer.usecs() << " MB/second)"
       << endl << endl;

  // write offsets + docIds + number of docs at the very end
  try
  {
    offsets.push_back(ftello(out_file));
    assert(offsets.size() == docIds.size() + 1);
    fwrite(&offsets[0], sizeof(off_t), offsets.size(), out_file);
    fwrite(&docIds[0], sizeof(DocId), docIds.size(), out_file);
    fwrite(&nofDocs, sizeof(DocId), 1, out_file);
    fclose(out_file);
    stat(outFileName.c_str(), &buf);
  }
  catch (exception& e)
  {
    cerr << "ERROR writing offsets + doc ids: " << e.what() << endl << endl;
    exit(1);
  }

  off_t out_size = buf.st_size;
  cout << "written \"" << outFileName << "\" with " 
       << commaStr(out_size) << " bytes"
       << " (" << (100*out_size) / in_size << "\% of input file)" 
       << endl << endl;
}

#endif
