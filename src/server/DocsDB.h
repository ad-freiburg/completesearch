#ifndef __DOCS_DB_H
#define __DOCS_DB_H

#include "Globals.h"
#include "Document.h"

class Document;

//! DOCS DB NEW 07Aug07 (Holger)
//
//   build: compresses <db>.docs doc by doc + stores offsets and doc ids
//   access: binary search on doc ids, then uncompress single doc
//
//   NOTE: buffers are per getDocument request, so thread-safe in this respect
//
class DocsDB
{

 private:

  //! NUMBER OF DOCUMENTS
  DocId _nofDocs;

  //! OFFSETS
  vector<off_t> _offsets;

  //! DOC IDS
  vector<DocId> _docIds;

  //! FILE
  FILE* _file;

 public:

  //! CONSTRUCT FROM <db>.docs.DB
  DocsDB(string dbFileName);

  //! GET DOCUMENT VIA ID
  void getDocument(const DocId docId, Document& document) const;

  //! GET NUMBER OF DOCUMENTS
  DocId getNofDocs() const { return _nofDocs; }

  //! BUILD FROM <db>.docs -> <db>.docs.DB 
  //    level is from 0 (no compression, fast) to 9 (high compression, slow)
  //    docs with less than minSize bytes are not compressed
  static void build(string&      inFileName, 
                    string&      outFileName, 
	                int          compressionLevel = 6,
					unsigned int minSize = 100);
  
  //! FOR DEBUGGING AND TESTING
  vector<off_t>& getOffsets() { return _offsets; }
  vector<DocId>& getDocIds() { return _docIds; }

};

#endif
