#ifndef __INDEXBASE_H__
#define __INDEXBASE_H__

#include "Globals.h"
#include "WordsFile.h"
#include "File.h"
#include "Vector.h"
#include "Vocabulary.h"
#include "MetaInfo.h"
#include "WordList.h"
#include "DocList.h"
//#include "CompressionAlgorithm.h"
#include "Simple9CompressionAlgorithm.h"
#include "ZipfCompressionAlgorithm.h"
//#include "TrivialCompressionAlgorithm.h"

using namespace std;

//! INDEX BASE CLASS
 /*
  *   Provides basic methods for accessing an index data structure that resides
  *   on disk. 
  *
  *   Derived classes are INVIndex and HYBIndex. Both of them consist of 
  *   compressed inverted lists, which are decompressed when read into memory,
  *   and a vocabulary, necessary to translate word ids into actual words
  */
class IndexBase
{

  protected:
    char* _compressionBuffer;
    unsigned long _compressionBufferSize;
    string _indexFileName;
    string _vocabularyFileName;

    void freeCompressionBuffer();
    
    unsigned long compressionBufferSize() const {return _compressionBufferSize;}
    
    void resizeCompressionBuffer(unsigned long size);

    void writeVocabularyToFile() const;

    off_t doclistVolumeCompressed;  /* total size in bytes */
    off_t positionlistVolumeCompressed; /* total size in bytes */
    off_t wordlistVolumeCompressed; /* total size in bytes */
    off_t scorelistVolumeWritten; /* total size in bytes (sores are currently uncompressed */

    //! COMPRESSION SCHEMES
    Simple9CompressionAlgorithm _doclistCompressionAlgorithm;
    Simple9CompressionAlgorithm _positionlistCompressionAlgorithm;
    ZipfCompressionAlgorithm<WordId> _wordlistCompressionAlgorithm;

    Timer resizeAndReserveTimer, buildIndexTimer, readFromDiskTimer, writeToDiskTimer, sortTimer, doclistCompressionTimer, positionlistCompressionTimer, wordlistCompressionTimer;

  public:

    IndexBase(const string& indexFileName, const string& vocabularyFileName,
        const unsigned char& mode);
    virtual ~IndexBase() {}

    //! Read the index from the index and vocabulary file passed to the
    //! constructor.
    virtual void read() = 0;

    //! Build the index from the given words file in the given format and
    //! write it to the index and vocabulary file passed to the constructor.
    virtual void build(const string& wordsFileName, const string& format) = 0;
    
    // IN BYTES (of the compressed lists)
    unsigned long int initialCompressionBufferSize = 100 * 1024 * 1024;

    unsigned char MODE;

    void reserveCompressionBuffersAndResetPointersAndCounters();

    void reserveCompressionBuffersAndResetPointers();
    void resetCounters();

    // words are assumed to be lexicographically sorted
    void addWordToVocabulary(const string& word)      
      {
        assert( (_vocabulary.size() == 0) || (word > _vocabulary.getLastWord()) );
        _vocabulary.push_back(word);
      }

    string getIndexFileName() const;
    
    // THESE THREE ARE COMMON TO HYB/INV
    // Ingmar 27Jan07 was protected
    Vocabulary _vocabulary;
      //vector<string> _vocabulary;
    
    //! META INFO (number of words, docs, etc.)
    MetaInfo _metaInfo;

}; // end class declaration

#endif
