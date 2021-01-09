#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include "assert.h"
#include "Globals.h"


using namespace std;

class File
{

 private:

  string _name;
  FILE* _file;

 public:

  // Timer used to measure how much time is spent on disk seeks
  // Note: Used for fuzzysearch experiments
  Timer diskSeekTimer;

  //! DEFAULT CONSTRUCTOR 
  // Create default file object (empty name, no file).
  File() { _file = NULL; _name = ""; }
  
  // Open file with given name in given mode.
  File(const char* filename, const char* mode)
  {
    open(filename, mode);
  }

  // Copy constructor.
  File(const File& orig)
  {
    _name = orig._name;
    // crucial: Not enough to copy _file!
    open(_name.c_str(), "r");
    assert(_file);
    #ifndef NDEBUG
    cout << "! File was copied (copy constructor)" << endl;
    #endif
  }
  
  // Destructor, closes file if still open.
  ~File()
  {
    if (isOpen()) close();
  }

  // Get file name.
  string getFileName() { return _name; }

  // Return true iff file has been opened.
  bool isOpen() const
  {
    return (_file != NULL);
  }

  // Open file. Exits with error if this fails, returns true otherwise.
  bool open(const char* filename, const char* mode)
  {
    #ifdef DEBUG_UNINTERRUPTIBLE_SLEEP
    cout << "! Before fopen, in File::open" << endl;
    #endif
    _file = fopen(filename,mode);
    #ifdef DEBUG_UNINTERRUPTIBLE_SLEEP
    cout << "! After fopen, in File::open" << endl;
    #endif
    if (_file == NULL) { cout << "! ERROR opening file \"" << filename << "\" with mode \"" << mode 
			      << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
    _name = filename;
    return true;
  }

  //! CLOSE FILE (exit with error message if fails, return true otherwise)
  bool close()
  {
    if (not isOpen()) { cout << "! WARNING : closing file \"" << _name << "\" which is not open" 
                             << endl << endl; return true; }
    if (fclose(_file) != 0) { cout << "! ERROR closing file \"" << _name << "\" (" 
                                   << strerror(errno) << ")" << endl << endl; exit(1); }
    _file = NULL; /* trying to close a file twice gives a segmentation fault */
    return true;
  }

  // returns true on success
  bool seek(off_t seekOffset, int seekOrigin)
    {
      assert((seekOrigin == SEEK_SET) || (seekOrigin == SEEK_CUR) || (seekOrigin == SEEK_END));
      assert(_file);
      diskSeekTimer.cont();
      bool seekSucc = (fseeko(_file, seekOffset, seekOrigin) == 0);
      diskSeekTimer.stop();
      return seekSucc;
    }


  //returns the number of bytes from the beginning
  //is 0 on opening. Later equal the number of bytes written.
  // -1 is returned when an error occurs

  off_t tell() const
    {
      assert(_file);
      off_t returnValue = ftello(_file);
      if(returnValue == (off_t) -1)
	{
	  cerr << "\n ERROR in tell() : " << strerror(errno) << endl << endl;
	  exit(1);
	}
      return returnValue;
    }

  // read from current file pointer position
  // returns the number of bytes read
  size_t read(void* targetBuffer, size_t nofBytesToRead) 
  {
    size_t readReturn = fread(targetBuffer, (size_t) 1, nofBytesToRead, _file);
    assert(readReturn == nofBytesToRead);
    return readReturn;

  }

  // move file pointer to desired position (offset from file beginning) and start reading from there
  // returns number of bytes read
  inline size_t read(void* targetBuffer, size_t nofBytesToRead, off_t seekOffsetFromStart)
    {
      assert(_file);
      if(!seek(seekOffsetFromStart,SEEK_SET)) return 0;
      return read(targetBuffer, nofBytesToRead);
    }  

  // write to current file pointer position
  // returns number of bytes written
  size_t write(const void* sourceBuffer, size_t nofBytesToWrite)
    {
      assert(_file);
      return fwrite(sourceBuffer,(size_t) 1,nofBytesToWrite,_file);
    }

  // Flushes the file and returns true if successful.
  bool flush()
  {
    return fflush(_file) == 0;
  }

  // should not be used too often as slower than other read
  // but ok to read in index table at end
  // U must be a container for typ T
  //  template <typename U, typename T> void readVector(U& vectorToReadTo, size_t nofBytesToRead, off_t seekOffsetFromStart)
  template <typename U, typename T> void readVector(U& vectorToReadTo, size_t nofBytesToRead, off_t seekOffsetFromStart, T dummy)
    {
      assert(_file);
        // nofBytesToRead should be multiple of sizeof(T)
      assert(nofBytesToRead > 0);
      assert( (size_t) (nofBytesToRead/(sizeof(T)))*sizeof(T) == nofBytesToRead ); 
      assert(vectorToReadTo.size() == 0);

      T currentElement;

#ifdef NDEBUG
      seek(seekOffsetFromStart,SEEK_SET);
#else
      bool seekReturn = seek(seekOffsetFromStart,SEEK_SET);
      assert(seekReturn); 
#endif
      size_t readReturn;

      for(size_t i=0; i< nofBytesToRead/sizeof(T); i++)
        {
          readReturn = read(&currentElement,sizeof(T));
          assert(readReturn == sizeof(T));
          vectorToReadTo.push_back(currentElement);
        }

      assert(vectorToReadTo.size() > 0);
    }//end: readVector





  // writes each element of a vector to the current file pointer position
  template <typename U, typename T> size_t writeVector(const U& vectorToWrite, T dummy)
    {
      assert(_file);
      size_t nofBytesWritten = 0;
      for(size_t i=0; i<vectorToWrite.size(); i++)
	{
	  	  nofBytesWritten += write((void*) &vectorToWrite[i], sizeof(T) );
	}
      return nofBytesWritten;
    }
  
  bool isAtEof()
    {
      assert(_file);
      return feof(_file) ? true : false;
    }


  // returns the byte offset of the last off_t
  // the off_t itself is passed back by reference
  off_t getLastOffset(off_t &lastOffset)
    {
      assert(_file);

      // seek to end of file
      seek((off_t) 0,SEEK_END);
      off_t sizeOfFile = tell();

      // now seek to end of file - sizeof(off_t)
#ifdef NDEBUG
      seek(sizeOfFile-sizeof(off_t),SEEK_SET);
#else
      bool seekReturn = seek(sizeOfFile-sizeof(off_t),SEEK_SET);
      assert(seekReturn);
#endif
      const off_t lastOffsetOffset = ftello(_file);
      assert(lastOffsetOffset == (off_t) (sizeOfFile - sizeof(off_t)));
      assert(lastOffsetOffset > (off_t) 0);

      // now read the last off_t
#ifdef NDEBUG
      read(&lastOffset,sizeof(off_t));
#else
      size_t readReturn = read(&lastOffset,sizeof(off_t));
      assert(readReturn == (size_t) sizeof(off_t));
      assert(lastOffset > 0);
#endif

      return lastOffsetOffset;
    }

};

#endif
