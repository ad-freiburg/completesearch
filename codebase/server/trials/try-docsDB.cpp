#include "Globals.h"
#include "DocsDB.h"

void printUsage() 
{
  cout << "Usage: test-docsDB <db>.docs.DB" << endl << endl; 
}

int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "TEST NEW DOCS DB (" << VERSION << ")"
	   << EMPH_OFF << endl << endl;
  Timer timer;

  if (argc <= 1) { printUsage(); exit(1); }
  char* filename = argv[1];
  unsigned int R = argc > 2 ? atoi(argv[2]) : 10;

  // OPEN DOCS DB
  DocsDB docsDB(filename);

  // GET RANDOM DOCUMENTS
  cout << endl;
  for (unsigned int r = 1; r <= R; r++)
  {
	unsigned int nofDocs = docsDB.getNofDocs();
	vector<DocId>& docIds = docsDB.getDocIds();
    unsigned int i0 = (unsigned int)(drand48()*nofDocs);
    DocId docId = docIds[i0];
	if (drand48() < 0.1) { docId = (DocId)(drand48()*2*docId); i0 = UINT_MAX; }
	cout << "reading document with id " << setw(7) << docId << " ... " << flush;
	timer.start();
    Document document;
	docsDB.getDocument(docId, document);
	timer.stop();
	cout << "done in " << setw(6) << commaStr(timer.usecs()) << " microseconds"
		 //<< ", uncompressed doc has " << setw(6) << commaStr(document.size()) << " bytes"
		 << "  [" << document << "]" << endl;
  }
  cout << endl;
}

	
     


// TO BE DELETED SOON !!!
#if (0)

  FILE* file = fopen(filename, "r");
  assert(file != NULL);
  assert(sizeof(off_t) == 8);

  char* in_buf = new char[MAX_IN_DOC_SIZE + 1];
  char* out_buf = new char[MAX_OUT_DOC_SIZE + 1];

  // READ OFFSETS AND DOC IDS (BEWARE: offsets are one more than doc ids)
  cout << "reading offsets + doc ids from \"" << filename << "\" ... " << flush;
  timer.start();
  off_t nofDocs;
  vector<off_t> offsets;
  vector<DocId> docIds;
  try 
  {
	fseeko(file, 0, SEEK_END);
	fseeko(file, ftello(file) - sizeof(off_t), SEEK_SET);
	fread(&nofDocs, sizeof(off_t), 1, file);
	cout << "[" << nofDocs << "] ... " << flush;
    offsets.resize(nofDocs + 1);
    docIds.resize(nofDocs);
	fseeko(file, ftello(file) - sizeof(off_t) 
		                      - sizeof(DocId) * docIds.size() 
		                      - sizeof(off_t) * offsets.size(), SEEK_SET);
	fread(&offsets[0], sizeof(off_t), nofDocs + 1, file);
	fread(&docIds[0], sizeof(DocId), nofDocs, file);
  }
  catch (exception& e)
  {
	cerr << "ERROR reading offsets: " << e.what() << endl << endl;
	exit(1);
  }
  timer.stop();
  cout << "done in " << timer << endl << endl;


  // CHECK ORDER
  for (unsigned int i = 0; i < nofDocs; ++i)
  {
	if (offsets[i] > offsets[i+1]) 
	{ 
	  cerr << "  [ERROR: offsets not in order " << i << ":" << offsets[i] 
		   << " > " << i+1 << ":" << offsets[i+1] << "]" << endl << endl; 
	  exit(1); 
	}
	if (i < nofDocs - 1 && docIds[i] > docIds[i+1]) 
	{ 
	  cerr << "  [ERROR: doc ids not in order " << i << ":" << docIds[i] 
		   << " > " << i+1 << ":" << docIds[i+1] << "]" << endl << endl; 
	  exit(1); 
	}
  }


  // READ + UNCOMPRESS RANDOM DOCS
  for (unsigned int r = 1; r <= R; r++)
  {
    unsigned int i0 = (unsigned int)(drand48()*nofDocs);
    DocId docId = docIds[i0];
	if (drand48() < 0.1) { docId = (DocId)(drand48()*2*docId); i0 = UINT_MAX; }
	cout << "reading document with id " << setw(7) << docId << " ... " << flush;
	timer.start();

	// binary search of doc id
	unsigned int l = 0;
	assert(docIds.size() > 0);
	unsigned int r = docIds.size() - 1;
	// maintain invariant: docIds[l] <= docId <= docIds[r]
	while (l < r)
	{
	  unsigned int m = (l + r)/2;
	  if (docId <= docIds[m]) r = m; else l = m + 1;
	}
	assert(l == r);
	unsigned int i = l;
	if (docIds[i] != docId)
	{
	  timer.stop();
	  cout << "done in " << setw(6) << commaStr(timer.usecs()) << " microseconds"
	       << ", DOCUMENT NOT FOUND" << endl;
	  continue;
	}
	assert(i == i0 || i0 == UINT_MAX);

	off_t out_len = offsets[i+1] - offsets[i];
	uLongf in_len = MAX_IN_DOC_SIZE;
	if (out_len > MAX_OUT_DOC_SIZE) 
	{
	  cerr << "WARNING: document too long, I skip it" << endl; 
	  continue; 
	}
	try
	{
	  fseeko(file, offsets[i], SEEK_SET);
	  fread(out_buf, 1, out_len, file);
	  int ret = uncompress((Bytef*)(in_buf), &in_len, 
		                   (Bytef*)(out_buf), out_len);
	  if (ret != Z_OK)
	  {
		cerr << endl << endl << "ERROR compressing: " << flush;
		switch (ret)
		{
		  case Z_MEM_ERROR : cerr << "not enough memory"; break;
		  case Z_BUF_ERROR : cerr << "output buffer too small"; break;
		  case Z_DATA_ERROR : cerr << "input data corrupt"; break;
		  default : cerr << "unknown error code (" << ret << ")"; break;
		}
		cerr << endl << endl;
		cerr << "TODO: deal with these kinds of errors more gracefully";
		cerr << endl << endl;
		exit(1);
	  }

      in_buf[in_len] = 0;
	}
	catch (exception& e)
	{
	  cerr << "ERROR reading document: " << e.what() << endl << endl;
	  continue;
	}
	timer.stop();
	cout << "done in " << setw(6) << commaStr(timer.usecs()) << " microseconds"
	     << ", uncompressed doc has " << setw(6) << commaStr(in_len) << " bytes";
	char* p = in_buf;
	while (*p != 0) { if (*p < 9) { *p = 'X'; } p++; }
	if (in_len < 50) {
	  cout << " (" << in_buf << ")" << endl;
	}
	else { 
	  in_buf[20] = 0; 
	  assert(in_len > 0);
	  assert(in_buf[in_len-1] == '\n');
	  in_buf[in_len-1] = 0;
	  cout << " [" << in_buf << "..." << in_buf + in_len - 25 << "]" << endl;
	}
  }
  cout << endl;

}

#endif
