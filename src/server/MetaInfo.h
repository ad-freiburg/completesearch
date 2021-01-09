#ifndef __METAINFO_H__
#define __METAINFO_H__

#include "Globals.h"
#include "assert.h"

// could also hold other meta info (description of collection etc)
class MetaInfo
{
  // TODO: could use DOCID, WORDID here as well (and wherever it is set)

 public:
  // Dump object to string.
  std::string asString(void)
  {
    std::stringstream ss;
    ss << "Max doc id             : " << _maxDocID << std::endl
       << "Nof words              : " << _nofWords << std::endl
       << "Nof words in doc pairs : " << _nofWordInDocPairs << std::endl
       << "Nof blocks             : " << _nofBlocks;
    return ss.str();
  }

 private:
  DocId _maxDocID;
  mutable WordId _nofWords;
  mutable DocId _nofDocs;
  unsigned long _nofWordInDocPairs;
  BlockId _nofBlocks;

 public:
  MetaInfo()
    {
      _nofBlocks = _maxDocID = _nofWords = _nofDocs = _nofWordInDocPairs = 0;
    }

  // copy constructor
  MetaInfo(const MetaInfo& orig)
    {
      _nofBlocks = orig._nofBlocks;
      _maxDocID = orig._maxDocID;
      _nofWords = orig._nofWords;
      _nofDocs = orig._nofDocs;
      _nofWordInDocPairs = orig._nofWordInDocPairs;
    }

  DocId getMaxDocID() const {return _maxDocID;}
  WordId getNofWords() const {return _nofWords;}
  DocId getNofDocs() const {return _nofDocs;}
  unsigned long getNofWordInDocPairs() const {return _nofWordInDocPairs;}
  BlockId getNofBlocks() const {return _nofBlocks;}

  void setMaxDocID(DocId maxDocID) {assert(maxDocID > 0); _maxDocID = maxDocID;}
  void setNofWords(WORDID nofWords) const {assert(nofWords>0); _nofWords = nofWords;}
  void setNofDocs(DOCID nofDocs) const {assert(nofDocs>0);_nofDocs = nofDocs;}
  void setNofBlocks(BLOCKID nofBlocks) {assert(nofBlocks>0);_nofBlocks = nofBlocks;}
  void setNofWordInDocPairs(unsigned long nofWordInDocPairs) {assert(nofWordInDocPairs>0);_nofWordInDocPairs = nofWordInDocPairs;}

  unsigned long getSizeInBytes() const
    {
      return sizeof(_maxDocID) + sizeof(_nofWords) + sizeof(_nofDocs) + sizeof(_nofWordInDocPairs) + sizeof(_nofBlocks);
    }

  void writeToBuffer(void* writeBuffer) const
    {
      *((DOCID*)(((char*) writeBuffer) + 0)) = _maxDocID;
      *((WORDID*)(((char*) writeBuffer) + sizeof(DOCID))) = _nofWords;
      *((DOCID*)(((char*) writeBuffer) + sizeof(DOCID) + sizeof(WORDID))) = _nofDocs;
      *((unsigned long*)(((char*) writeBuffer) + sizeof(DOCID) + sizeof(WORDID) + sizeof(DOCID))) = _nofWordInDocPairs;
      *((BLOCKID*)(((char*) writeBuffer) + sizeof(DOCID) + sizeof(WORDID) + sizeof(DOCID) + sizeof(unsigned long))) = _nofBlocks;
    }
  
  // copies!
  void readFromBuffer(const void* readBuffer)
    {
      _maxDocID = *((DOCID*)(((char*) readBuffer) + 0));
      assert(_maxDocID > 0);
      
      _nofWords = *((WORDID*)(((char*) readBuffer) + sizeof(DOCID)));
      assert(_nofWords > 0);

      _nofDocs = *((DOCID*)(((char*) readBuffer) + sizeof(DOCID) + sizeof(WORDID)));
      assert(_nofDocs > 0);

      _nofWordInDocPairs = *((unsigned long*)(((char*) readBuffer) + sizeof(DOCID) + sizeof(WORDID) + sizeof(DOCID)));
      assert(_nofWordInDocPairs > 0);

      _nofBlocks =  *((BLOCKID*)(((char*) readBuffer) + sizeof(DOCID) + sizeof(WORDID) + sizeof(DOCID) + sizeof(unsigned long)));
    }

  void show() const
    {
      if (getNofWords() == 0) { cout << "! WARNING : number of words is zero, setting it to 1" << endl << endl; setNofWords(1); }
      if (getNofDocs() == 0) { cout << "! WARNING : number of documents is zero, setting it to 1" << endl << endl; setNofDocs(1); }
      cout << setw(15) << commaStr(getNofDocs()) << " documents" << endl
           << setw(15) << commaStr(getNofWords()) << " words" << endl
           << setw(15) << commaStr(getNofWordInDocPairs()) << " word-in-document pairs" << endl
           << setw(15) << commaStr(getMaxDocID()) << " is the max document id" << endl
           << setw(15) << commaStr(getNofBlocks()) << " blocks" << endl
           << setw(15) << getNofWordInDocPairs()/getNofDocs() << " is the average number of distinct words per document (L)" << endl
           << setw(15) << getNofWordInDocPairs()/getNofWords() << " is the average frequency of a word" << endl;
    }

};

#endif
