#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include "Globals.h"
//#include "DocsDB.h"

class DocsDB;
class DocumentException;

//! A DOCUMENT FROM <db>.docs resp. <db>.docs.DB
//
//   NOTE: keep the interface from the old ExcerptData from Thomas, so that
//   little needs to be changed in ExcerptsGenerator
//
//   BEWARE: assignment must do deep copy (currently, the built-in one does fine)
//
class Document
{

 private:

  //! doc id
  DocId _docId;

  //! url
  string _url;

  //! title
  string _title;

  //! excerpt
  string _text;
  
  //! set doc id, url, etc. explicitly
  void set(DocId docId, string url, string title, string text);

  //! set doc id, url, etc. from line from <db>.docs
  void set(const char* doc);

  //! set in case of error
  void setIfError(string message);

 public:

   //! default constructor
  Document() { _docId = 0; }
  
  //! construct from line of <db>.docs
  Document(const char* doc) { set(doc); } 
  
  //! the obvious access methods
  DocId getDocId() const { return _docId; }
  const string& getUrl() const { return _url; }
  const string& getTitle() const { return _title; }
  const string& getText() const { return _text; }

  //! PRINT DOCUMENT (for debugging purposes only)
  friend ostream& operator<<(ostream& os, Document& document);

  // ...
  friend class DocsDB;
  //friend void DocsDB::getDocument(const DocId docId, Document& document) const;
};



//! ASSOCIATED EXCEPTION CLASS 
class DocumentException
{

 public:

  //! POSSIBLE CODES
  enum Code { NO_DOC_ID, ZERO_DOC_ID, MISSING_TAB, NO_URL, NO_TITLE, NO_TEXT };

 private:

  //! CODE
  Code _code;

  //! DETAILS (specific information beyond what the code already says)
  const char* _details;

 public:
  
  //! CONSTUCTOR
  DocumentException(Code code, const char* details = "") : _code(code), _details(details) { };

  //! GET CODE
  Code getCode() { return _code; }

  //! GET MESSAGE PERTAINING TO CODE
  const char* getMessage(Code code);

  //! GET FULL ERROR MESSAGE (message + details)
  string getMessage();
};



/*
//! PRINT DOCUMENT (for debugging purposes only)
ostream& operator<<(ostream& os, Document& document);
*/

#endif
