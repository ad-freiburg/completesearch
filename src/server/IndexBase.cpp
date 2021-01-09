#include "IndexBase.h"

// _____________________________________________________________________________
IndexBase::IndexBase(const string& indexFileName,
    const string& vocabularyFileName, const unsigned char &mode)
  : _indexFileName(indexFileName)
  , _vocabularyFileName(vocabularyFileName)
  , MODE(mode)
{
}

// _____________________________________________________________________________
// used in destructor
void IndexBase::freeCompressionBuffer()
{
  if(_compressionBuffer)
  {
    #ifndef NDEBUG
    cout << "! freeing compression buffer ( "
         << _compressionBufferSize/(1024*1024) << " MB)" << endl;
    #endif
    assert(_compressionBufferSize > 0);
    delete[] _compressionBuffer;
    _compressionBuffer = NULL;
  }
  _compressionBufferSize = 0;
}


// _____________________________________________________________________________
//! WRITE VOCABULARY TO FILE (one word per line)
void IndexBase::writeVocabularyToFile() const
{
  FILE* file = fopen(_vocabularyFileName.c_str(), "w");
  assert(file != NULL);
  assert(_vocabulary.size() > 0);
  cout << "* writing all words to file \"" << _vocabularyFileName << "\" ... "
    << flush;
  for(unsigned int i=0; i<_vocabulary.size(); i++)
  {
    //does not write the null termination
    fputs(_vocabulary[i].c_str(),file);
    //append a newline char as a readable word separator
    fputs("\n",file);
  }
  cout << "done (" << commaStr(_vocabulary.size()) << " words)" << endl;
  fclose(file);
}


// _____________________________________________________________________________
void IndexBase::reserveCompressionBuffersAndResetPointersAndCounters()
{
  // sets pointers to NULL, reserves compression Buffer
  reserveCompressionBuffersAndResetPointers();
  resetCounters();
}


// _____________________________________________________________________________
void IndexBase::resetCounters()
{
  doclistVolumeCompressed = 0;
  positionlistVolumeCompressed = 0;
  wordlistVolumeCompressed = 0;
  scorelistVolumeWritten = 0;
}


// _____________________________________________________________________________
void IndexBase::reserveCompressionBuffersAndResetPointers()
{
  assert(initialCompressionBufferSize > 0);
  try {
    _compressionBuffer = new char[initialCompressionBufferSize];
  }
  catch (exception e) {
    ostringstream os;
    os << e.what() << "; new char[" << initialCompressionBufferSize << "]";
    CS_THROW(Exception::NEW_FAILED, os.str());
  }
  assert(_compressionBuffer);
  _compressionBufferSize = initialCompressionBufferSize;
}


// _____________________________________________________________________________
void IndexBase::resizeCompressionBuffer(unsigned long size)
{
#ifndef NDEBUG
  cout << MSG_BEG << "resizing compression buffer to size " << size << "... "
       << flush;
#endif
  if(_compressionBuffer)
  {
    resizeAndReserveTimer.cont();
    delete[] _compressionBuffer;
    resizeAndReserveTimer.stop();
    _compressionBuffer = NULL;
  }
  _compressionBufferSize = size;
  resizeAndReserveTimer.cont();
  _compressionBuffer = new char[size];
  memset(_compressionBuffer,0,size); /* to avoid TLB misses */
  resizeAndReserveTimer.stop();
  assert(_compressionBuffer);
#ifndef NDEBUG
  cout << "done, now size " << commaStr(size) << MSG_END << flush;
#endif
}

// _____________________________________________________________________________
string IndexBase::getIndexFileName() const
{
  return _indexFileName;
}
