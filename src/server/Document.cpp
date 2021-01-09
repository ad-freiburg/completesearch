#include "Document.h"


//! SET DOC ID, URL, ETC. EXPLICITLY
void Document::set(DocId docId, string url, string title, string text)
{
  _docId = docId;
  _url = url;
  _title = title;
  _text = text;
}



//! SET TO LINE FROM DOCS FILE
//
//    the following format is expected (spaces are for readability only!)
//
//    <doc id> <TAB> u:<url> <TAB> t:<title> <TAB> H:<raw text without newlines> <CR>
//
//    doc is expected to be zero-terminated
//
//    an optional newline at the end will be removed 
//    
void Document::set(const char* doc)
{
  const char* p;
  const char* p0;

  // parse doc id
  _docId = 0;
  p = doc;
  while (*p != 0 && *p != '\t') 
  { 
	if (*p < '0' || *p > '9') throw DocumentException(DocumentException::NO_DOC_ID);
	_docId = 10 * _docId + (*p - '0');
	++p;
  }
  if (_docId == 0) throw DocumentException(DocumentException::ZERO_DOC_ID);
  if (*p != '\t') throw DocumentException(DocumentException::MISSING_TAB);
  ++p;

  // parse url
  if (*p != 'u' || *(p+1) != ':') throw DocumentException(DocumentException::NO_URL);
  p += 2;
  p0 = p;
  while (*p != 0 && *p != '\t') ++p;
  if (*p != '\t') throw DocumentException(DocumentException::MISSING_TAB);
  _url.assign(p0, p - p0);
  ++p;

  // parse url
  if (*p != 't' || *(p+1) != ':') throw DocumentException(DocumentException::NO_TITLE);
  p += 2;
  p0 = p;
  while (*p != 0 && *p != '\t') ++p;
  if (*p != '\t') throw DocumentException(DocumentException::MISSING_TAB);
  _title.assign(p0, p - p0);
  ++p;

  // parse text
  if (*p != 'H' || *(p+1) != ':') throw DocumentException(DocumentException::NO_TEXT);
  p += 2;
  p0 = p;
  while (*p != 0) ++p;
  assert(p > doc);
  if (*(p-1) == '\n') --p;
  _text.assign(p0, p - p0);
}



//! SET IN CASE OF EXCEPTION (essentially: set title to error message)
void Document::setIfError(string message)
{
  ostringstream os;
  os << "[ERROR RETRIEVING DOCUMENT: " << message << "]";
  set(0, "", os.str(), "");
}



//! PRINT DOCUMENT (for debugging purposes only)
ostream& operator<<(ostream& os, Document& document)
{
  ostringstream os_tmp;
  os_tmp << document._docId << "\t" 
	     << "u:" << document._url << "\t"
	     << "t:" << document._title << "\t"
	     << "H:" << document._text;
  string s = os_tmp.str();
  for (unsigned int i = 0; i < s.size(); ++i)
	if (s[i] < 9) s[i] = 'X';
  if (s.size() < 60) {
	os << s;
  }
  else { 
	s[30] = 0; 
	os << s.c_str() << "..." << s.c_str() + s.size() - 25;
  }
  return os;
}



//! GET MESSAGE PERTAINING TO CODE
const char* DocumentException::getMessage(Code code)
{
  switch (code)
  {
	case NO_DOC_ID   : return "no document id";
    case ZERO_DOC_ID : return "zero document id";      
    case MISSING_TAB : return "missing tab";           
    case NO_URL      : return "no url in document";    
    case NO_TITLE    : return "no title in document";  
    case NO_TEXT     : return "no text in document";   
	default          : return "unknown error code";    
  }
}



//! GET FULL ERROR MESSAGE (message + details)
string DocumentException::getMessage()
{
  ostringstream os;
  os << getMessage(_code);
  if (*_details != 0)
	os << " (" << _details << ")";
  return os.str();
}
