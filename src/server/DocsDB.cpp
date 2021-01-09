#include "DocsDB.h"
#include <zlib.h>

#define MY_MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MY_MAX(a,b) ( (a) < (b) ? (b) : (a) )

// max size (in bytes) of uncompressed document
unsigned int MAX_IN_DOC_SIZE = 10*1000*1000;
// according to zlib, compressed document can be 0.1% large + 12 bytes
unsigned int MAX_OUT_DOC_SIZE = MAX_IN_DOC_SIZE + MAX_IN_DOC_SIZE/1000 + 13;
                    


//! CONSTRUCT FROM <db>.docs.DB
DocsDB::DocsDB(string dbFileName)
{
  const char* ERROR_MSG = "ERROR in DocsDB::DocsDB: "; 

  _file = fopen(dbFileName.c_str(), "r");
  if (_file == NULL) { perror("fopen docs.DB _file"); exit(1); }

  // READ OFFSETS AND DOC IDS (BEWARE: offsets are one more than doc ids)
  cout << "* reading offsets + doc ids from \"" << dbFileName << "\" ... " << flush;
  Timer timer;
  timer.start();
  try 
  {
    fseeko(_file, 0, SEEK_END);
    fseeko(_file, ftello(_file) - sizeof(DocId), SEEK_SET);
    size_t numItemsRead;
    numItemsRead = fread(&_nofDocs, sizeof(DocId), 1, _file);
    CS_ASSERT_EQ(1, numItemsRead);
      //cout << "[" << _nofDocs << "] ... " << flush;
    _offsets.resize(_nofDocs + 1);
    _docIds.resize(_nofDocs);
    fseeko(_file, ftello(_file) - sizeof(DocId) 
                  - sizeof(DocId) * _docIds.size() 
                  - sizeof(off_t) * _offsets.size(), SEEK_SET);
    numItemsRead = fread(&_offsets[0], sizeof(off_t), _offsets.size(), _file);
    CS_ASSERT_EQ(_offsets.size(), numItemsRead);
    numItemsRead = fread(&_docIds[0], sizeof(DocId), _docIds.size(), _file);
    CS_ASSERT_EQ(_docIds.size(), numItemsRead);
  }
  catch (exception& e)
  {
    cerr << ERROR_MSG << e.what() << endl << endl;
    exit(1);
  }
  timer.stop();
  cout << "done in " << timer << endl;

  // CHECK ORDER
  for (unsigned int i = 0; i < _nofDocs; ++i)
  {
  if (_offsets[i] > _offsets[i+1]) 
  { 
    cerr << ERROR_MSG << ": offsets not in order " << i << ":" << _offsets[i] 
       << " > " << i+1 << ":" << _offsets[i+1] << endl << endl; 
    exit(1); 
  }
  if (i < _nofDocs - 1 && _docIds[i] > _docIds[i+1]) 
  { 
    cerr << ERROR_MSG << ": doc ids not in order " << i << ":" << _docIds[i] 
       << " > " << i+1 << ":" << _docIds[i+1] << endl << endl; 
    exit(1); 
  }
  }

} // end construct from <db>.docs.db



//! GET DOCUMENT VIA ID
//
//    if doc if not found, sets document to NO_DOCUMENT (from Document.cpp)
//
void DocsDB::getDocument(const DocId docId, Document& document) const
{
  //const char* WARNING_MSG = "WARNING in DocsDB::getDocument: ";
  //const char* ERROR_MSG = "ERROR in DocsDB::getDocument: ";

  // ALLOCATE BUFFER SPACE (freed below, TODO: potential efficiency problem)
  char* in_buf = new char[MAX_IN_DOC_SIZE + 1];   // see defintion above
  char* out_buf = new char[MAX_OUT_DOC_SIZE + 1]; // dito

  // BINARY SEARCH OF DOC ID
  unsigned int l = 0;
  assert(_docIds.size() > 0);
  unsigned int r = _docIds.size() - 1;
  // maintain invariant: docIds[l] <= docId <= docIds[r]
  while (l < r)
  {
  unsigned int m = (l + r)/2;
  if (docId <= _docIds[m]) r = m; else l = m + 1;
  }
  assert(l == r);
  unsigned int i = l;
  if (_docIds[i] != docId)
  {
  ostringstream os;
  os << "document with id " << docId << " not found";
  document.setIfError(os.str());
  return;
  }

  // READ COMPRESSED LINE AND UNCOMPRESS
  off_t out_len = _offsets[i+1] - _offsets[i];
  uLongf in_len = MAX_IN_DOC_SIZE;
  if (out_len > MAX_OUT_DOC_SIZE) 
  {
  document.setIfError("document too long");
  return; 
  }
  try
  {
    fseeko(_file, _offsets[i], SEEK_SET);
    size_t numItemsRead = fread(out_buf, 1, out_len, _file);
    CS_ASSERT_LE(0, out_len);
    CS_ASSERT_EQ((size_t)out_len, numItemsRead);
    
    // CASE: was not compressed with zlib (see explanations in build method below ZZZ)
    if (*out_buf == 0)
    {
      if (out_len == 0) 
      {
      document.setIfError("zero-length document (probably input data corrupt)");
      return;
      }
      in_len = out_len - 1;
      memcpy(in_buf, out_buf + 1, in_len);
    }

      // CASE: was compressed with zlib
    else
    {
      int ret = uncompress((Bytef*)(in_buf), &in_len, 
        (Bytef*)(out_buf), out_len);
      if (ret != Z_OK)
      {
      ostringstream os;
      os << "problem uncompressing document (" << flush;
      switch (ret)
      {
        case Z_MEM_ERROR : os << "not enough memory"; break;
        case Z_BUF_ERROR : os << "output buffer too small"; break;
        case Z_DATA_ERROR : os << "input data corrupt"; break;
        default : os << "unknown error code: " << ret; break;
      }
      os << ")";
      document.setIfError(os.str());
      return;
      }
    }

    in_buf[in_len] = 0;
  }
  catch (exception& e)
  {
    document.setIfError(e.what());
    return;
  }

  // PARSE INTO DOCUMENT OBJECT
  try
  {
  document.set(in_buf);
  }
  catch (DocumentException e)
  {
  document.setIfError(e.getMessage());
  }

  delete[] in_buf;
  delete[] out_buf;
}



//! BUILD FROM <db>.docs -> <db>.docs.DB 
//    level is from 0 (no compression, fast) to 9 (high compression, slow)
//    NEW: can also specify -1 now, much faster than 0, see below ZZZ
//    docs with less than minSize bytes are not compressed
void DocsDB::build(string&      inFileName, 
                   string&      outFileName, 
                   int          compressionLevel, 
                   unsigned int minSize)
{
  //const char* ERROR_MSG = "ERROR in DocsDB::build"; 
  Timer timer;
  timer.start();

  // OPEN FILES (and get size of input _file)
  FILE* in_file = fopen(inFileName.c_str(), "r");
  if (in_file == NULL) { perror("fopen docs file for reading"); exit(1); }
  FILE* out_file = fopen(outFileName.c_str(), "w");
  if (out_file == NULL) { perror("fopen db file for writing"); exit(1); }
  struct stat buf;
  int ret = stat(inFileName.c_str(), &buf);
  if (ret != 0) { perror("stat docs file"); exit(1); }
  off_t in_size = buf.st_size;

  // ALLOCATE BUFFERS
  char* in_buf = new char[MAX_IN_DOC_SIZE + 1];   // see defintion above
  char* out_buf = new char[MAX_OUT_DOC_SIZE + 1]; // dito

  // OFFSETS AND DOCS IDS (will be appended to end of _file)
  if (sizeof(off_t) != 8) { cerr << "ERROR: sizeof(off_t) = "
                                 << sizeof(off_t) << " (want 8)" << endl
                                 << endl; exit(1); }
  vector<off_t> offsets;
  vector<DocId> docIds;

  // READ, COMPRESS, WRITE (line by line)
  cout << "compressing \"" << inFileName << "\" doc by doc " << flush; 
  off_t milestone = 0;
  off_t mile = MY_MAX(1, in_size / 10);
  while (true)
  {
    // read next line and remember doc id (exit loop if end of _file)
    char* ret1 = fgets(in_buf, MAX_IN_DOC_SIZE + 1, in_file);
    if (ret1 == NULL) { assert(feof(in_file)); break; }
    DocId docId = atoi(in_buf);
    if (docId == 0 && !(in_buf[0] == '0' && in_buf[1] == '\t')) {
      in_buf[40] = 0; 
      cerr << "ERROR: line has wrong format (first 40 characters: " 
           << in_buf << ")" << endl << endl;
      exit(1);
    }
    docIds.push_back(docId);

    uLong in_len = strlen(in_buf);
    uLongf out_len;

    // if -1 specified, write a 0-byte followed by uncompressed data ZZZ
  //
  //   NOTE: I checked that zlib does not compress either at compression level
  //   0, it is still 10 times slower than a simple memcpy, probably due to
  //   checksum computations (I wonder for how much time these account for the
  //   higher compression levels, where compression actually happens)
  //
  //   NOTE: I also checked that zlib does not write a zero as its first byte;
  //   see the zlib specification at http://www.gzip.org/zlib/rfc-zlib.html#format
  //
  if (compressionLevel == -1)
  {
    out_len = in_len + 1; // NOTE: out buffer is 12 bytes larger anyway, see above
    *out_buf = 0;
    memcpy(out_buf + 1, in_buf, in_len);
  }

    // compress with specified level (in 0..9), no compression for short docs
  else
  {
    uLong in_len = strlen(in_buf);
    out_len = MAX_OUT_DOC_SIZE;
    int ret2 = compress2((Bytef*)(out_buf), &out_len, 
               (Bytef*)(in_buf), in_len, 
               in_len >= minSize ? compressionLevel : 0);
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
  timer.stop();
  DocId nofDocs = offsets.size();
  cout << " done " << nofDocs << " docs in " << timer; 
  if (timer.usecs() > 0)
    cout << " (" << in_size / timer.usecs() << " MB/second)";
  cout << endl << endl;

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

  delete[] in_buf;
  delete[] out_buf;
}


