// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#ifndef CODEBASE_WORDSFILE_H__
#define CODEBASE_WORDSFILE_H__

#include <google/dense_hash_map>
#include <iostream>
#include <string>
#include "./Globals.h"

// #include <ext/hash_set>  // Used to maintain distinct doc ids.
// #include <hash_set>
// using __gnu_cxx::hash_set;

class WordsFile
{
 public:
  // Constructor opens file with given name. Default format is HTDIG (word, doc
  // id, score, position). By default will not skip lines with same word and doc
  // id, and will not maintain max doc id. Allocate space for a number of
  // buffers for reading.
  explicit WordsFile(string fileName);

  // Destructor closes file and frees the space that was allocated for the
  // various read buffers in the constructor.
  ~WordsFile();

  // File formats.
  enum Format
  {
    FORMAT_INVALID_LOW = 0,
    FORMAT_HTDIG       = 1,
    FORMAT_SHORT       = 2,
    FORMAT_BINARY      = 3,
    FORMAT_INVALID_UPP = 4
  };

  // Get next line from a words file. Returns false iff (1) end of file if
  // reached; isEof will return true then; (2) a parsing error occurred for that
  // linel; (3) a line with the same word and doc id as the previous line was
  // skipped.
  bool getNextLine(string& word, WordId& wordId, DocId& docId,
                   DiskScore& score, Position& pos);

  // Getters and setters.
  void setFormat(const Format& format);
  void setSkipLineWithSameWordAndDoc(bool x)
  { _skipLineWithSameWordAndDoc = x; }
  void setMaintainSetOfDistinctDocIds(bool x)
  { _maintainSetOfDistinctDocIds = x; }
  off_t getFileSizeInBytes() { return _fileSizeInBytes; }
  bool isEof() const { return _isEof; }
  off_t getFileSizeInBytes() const;
  off_t getLineNumber() const { return _lineNumber; }
  DocId maxDocId() const { return _maxDocId; }
  off_t totalNumBytesRead() const { return _totalNumBytesRead; }
  bool formatIsBinary() const { return _format == FORMAT_BINARY; }
  // Returns number of distinct doc ids when _maintainSetOfDistinctDocIds ==
  // true, otherwise returns max doc id.
  DocId numDocs() const;

 private:
  // Words file name.
  string _fileName;
  // Words file.
  FILE* _file;
  // Explicit buffer for reading from file.
  char* _fileBuffer;
  // Format (see enum above).
  Format _format;
  // Whether to skip lines with same word and doc as previous line.
  bool _skipLineWithSameWordAndDoc;
  // Whether to maintain the set of distinct doc ids.
  bool _maintainSetOfDistinctDocIds;

  // Buffer for reading a line / record from file.
  char* _line;
  // Buffer for reading word via sscanf.
  char* _word;
  // Set of distinct docs id (only maintained when _maintainSetOfDistinctDocIds
  // == true).
  google::dense_hash_map<DocId, bool> _distinctDocIds;
  // hash_set<DocId> _distinctDocIds;
  // Keep track of these while scanning the file.
  WordId _lastWordId;
  DocId _lastDocId;
  DocId _maxDocId;
  string _lastWord;
  off_t _lineNumber;
  off_t _fileSizeInBytes;
  off_t _totalNumBytesRead;
  bool _isEof;
};

#endif  // CODEBASE_WORDSFILE_H__
