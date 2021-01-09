// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include "./WordsFile.h"
#include <math.h>
#include <limits>
#include <string>

// _____________________________________________________________________________
WordsFile::WordsFile(string fileName)
{
  _format = FORMAT_HTDIG;
  _fileName = fileName;
  _skipLineWithSameWordAndDoc = false;
  _maintainSetOfDistinctDocIds = false;
  _distinctDocIds.set_empty_key(std::numeric_limits<DocId>::max());

  // Open file.
  _file = fopen(_fileName.c_str(), "r");
  if (_file == NULL)
  {
    cout << endl << "! ERROR opening file \"" << _fileName
         << "\" (" << strerror(errno) << ")" << endl << endl;
    exit(1);
  }
  _fileBuffer = new char[FILE_BUFFER_SIZE];
  setbuffer(_file, _fileBuffer, FILE_BUFFER_SIZE);

  // Stat file size.
  struct stat buf;
  int ret = stat(_fileName.c_str(), &buf);
  CS_ASSERT(ret == 0);
  _fileSizeInBytes = buf.st_size;

  _lastDocId = INFTY_DOCID;
  _maxDocId = 0;
  _lastWord = "";
  _lineNumber = 0;
  _totalNumBytesRead = 0;
  _isEof = false;
  _line = new char[MAX_LINE_LENGTH+2]();
  _word = new char[MAX_LINE_LENGTH+2]();
}


// _____________________________________________________________________________
WordsFile::~WordsFile()
{
  if (_file != NULL) fclose(_file);
  if (_fileBuffer != NULL) delete[] _fileBuffer;
  if (_line != NULL) delete[] _line;
  if (_word != NULL) delete[] _word;
}


// _____________________________________________________________________________
void WordsFile::setFormat(const WordsFile::Format& format)
{
  // Check that format is valid.
  if (format <= FORMAT_INVALID_LOW || format >= FORMAT_INVALID_UPP)
  {
    cerr << "! Invalid words file format (" << format << ")" << endl;
    exit(1);
  }
  // Check the for FORMAT_BINARY sizeof(unsigned int) == 0.
  if (format == FORMAT_BINARY && sizeof(unsigned int) != 4)
  {
    cerr << "! sizeof(unsigned int) must be 4 for FORMAT_BINARY (is "
         << sizeof(unsigned int) << ")" << endl;
    exit(1);
  }
  _format = format;
}


// _____________________________________________________________________________
DocId WordsFile::numDocs() const
{
  if (_maintainSetOfDistinctDocIds)
    return _distinctDocIds.size();
  else
    return _maxDocId;
}


// _____________________________________________________________________________
bool WordsFile::getNextLine(string& word, WordId& wordId, DocId& docId,
                            DiskScore& score, Position& pos)
{
  double rawScore;
  bool allTokensParsed;
  unsigned int nofBytesRead;

  // 1. Read next line from line. Return with false if there was nothing left to
  // read.
  if (_format == FORMAT_HTDIG || _format == FORMAT_SHORT)
  {
    _isEof = (fgets(_line, MAX_LINE_LENGTH+2, _file) == NULL);
    nofBytesRead = strlen(_line) + 1;
  }
  else if (_format == FORMAT_BINARY)
  {
    assert(sizeof(unsigned int) == 4);
    nofBytesRead = 4 * fread(_line, 4, 4, _file);
    _isEof = (nofBytesRead == 0);
    if (nofBytesRead > 0 && nofBytesRead != 16)
    {
      cerr << MSG_BEG << "! Less then 16 bytes left at end of file ("
           << nofBytesRead << ")" << endl;
      exit(1);
    }
  }
  else
  {
    cerr << MSG_BEG << "! Invalid words file format (" << _format << ")"
         << endl;
    exit(1);
  }
  _lineNumber++;
  _totalNumBytesRead += nofBytesRead;
  if (_isEof) return false;

  // 2a. If line length exceeds buffer size (defined in Globals.h) ignore the
  // whole line and return false.
  if (nofBytesRead > MAX_LINE_LENGTH + 1)
  {
    assert(_format != FORMAT_BINARY);
    cerr << MSG_BEG << "WARNING while reading \"" << _fileName << "\""
         << " (line " << _lineNumber << " longer than " << MAX_LINE_LENGTH
         << " characters)" << " *ignoring this line*" << MSG_END << flush;
    // Make sure to read the rest of the line, otherwise the next call to
    // getNextLine will return nonsense.
    assert(_format != FORMAT_BINARY);
    while (!_isEof && nofBytesRead > 2 && _line[nofBytesRead-2] != '\n')
    {
      _isEof = (fgets(_line, MAX_LINE_LENGTH+2, _file) == NULL);
      nofBytesRead = strlen(_line) + 1;
      cout << MSG_BEG << "." << MSG_END << flush;
    }
    return false;
  }

  // 2b. If this is the first line, and it starts with a #, ignore it.
  if (_format != FORMAT_BINARY && _lineNumber == 1 && _line[0] == '#')
  {
    return false;
  }

  // 3. Parse the line, depending on the format.
  switch (_format)
  {
    case FORMAT_HTDIG:
      allTokensParsed = (sscanf(_line, "%s %u %lf %u",
                         _word, &docId, &rawScore, &pos) == 4);
      word = _word;
      wordId = -1;
      break;
    case FORMAT_SHORT:
      allTokensParsed = (sscanf(_line, "%s %u %u", _word, &docId, &pos) == 3);
      word = _word;
      wordId = -1;
      rawScore = 0;
      break;
    case FORMAT_BINARY:
      allTokensParsed = (nofBytesRead == 16);
      assert(allTokensParsed);
      word = "";
      wordId = *(unsigned int*)(_line);
      docId = *(unsigned int*)(_line + 4);
      rawScore = *(unsigned int*)(_line + 8);
      pos = *(unsigned int*)(_line + 12);
      break;
    default:
      cout << endl << "! Invalid format in words file (" << _format << ")"
           << endl;
      exit(1);
  }

  // Check range of score.
  if (rawScore < 0)
  {
    cout << "! WARNING, negative score in line #" << _lineNumber
         << ", set it to zero" << endl;
    rawScore = 0;
  }
  else if (rawScore > 255)
  {
    cout << "! WARNING, too large score in line #" << _lineNumber
         << ", set it to 255" << endl;
    rawScore = 255;
  }
  score = (DiskScore)(rawScore);
  // htdig scores: 2 for title (becomes 65), 0 for reguar (becomes 1)
  // rawScore = (DiskScore) 32*rawScore + 1;
  // Now assumes that rawScore == 0 indicates a title match
  // rawScore = (DiskScore) ((rawScore == (double) 2) ?
  // ((DiskScore) 65): ((rawScore == (double) 3) ? 16 : 1));
  // assert((rawScore==65) || (rawScore==1));
  // An indication how the htdig scores work
  // assert((double) sizeof(DiskScore)*8 >=
  //        (double) log((double) MAX_SCORE)/log((double) 2.0));
  // to ensure that really every score is positive
  // score = MAX((DiskScore) rawScore,(DiskScore) 1);
  // assert(score>0);
  score = static_cast<DiskScore>(rawScore);

  // Parsing of line failed.
  if (allTokensParsed == false)
  {
    cerr << MSG_BEG << "! WARNING while reading \"" << _fileName << "\""
      << " (sscanf failed on line " << _lineNumber << ")"
      << " *ignoring this line*" << MSG_END;
    return false;
  }

  // Optionally maintain the set of distinct doc ids.
  if (_maintainSetOfDistinctDocIds &&
      _distinctDocIds.find(docId) == _distinctDocIds.end())
  {
      _distinctDocIds[docId] = true;
      // _distinctDocIds.insert(docId);
  }

  // _skipLineWithSameWordAndDoc = true;
  // Optionally skip lines with same word and doc id as previous line.
  word = _word;
  if (_skipLineWithSameWordAndDoc && docId == _lastDocId &&
      ((_format != FORMAT_BINARY && word == _lastWord) ||
       (_format == FORMAT_BINARY && wordId == _lastWordId)))
  {
    return false;
  }

  // Remember word and doc id from this line.
  _lastWord = word;
  _lastWordId = wordId;
  _lastDocId = docId;
  if (docId > _maxDocId) _maxDocId = docId;
  return true;
}

