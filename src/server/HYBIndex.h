#ifndef __HYBINDEX_H__
#define __HYBINDEX_H__

#include <string>
#include "Timer.h"
#include "IndexBase.h"

using namespace std;

//! HYB INDEX CLASS
 /* 
  *   Provides methods for accessing a HYB index residing on disk, essentially:
  *
  *   1. open the associated files (done in the constructor)
  *   2. get a HYB block (read from disk and decompress)
  *   3. make a HYB block (compress and write to disk, during index building)
  */
class HYBIndex : public IndexBase
{
 public:
  //! Create a HYB index using the given files.  To load the data, call either
  //! read or build.
  HYBIndex(const string& indexFile, const string& vocabularyFile, unsigned char mode);

  // DESTRUCTOR (just to output info)
  ~HYBIndex();

  void showMetaInfo() const;

  //! Read the index from the index and vocabulary file passed to the
  //! constructor.
  void read() override;

  //! Build the index from the given words file in the given format and
  //! write it to the index and vocabulary file passed to the constructor.
  void build(const string& wordsFileName, const string& format) override;

  // THESE TWO ARE HYB SPECIFIC
  //! POSITIONS OF BLOCKS ON DISK
  vector<off_t> _byteOffsetsForBlocks;
  
  //! IDS OF FIRST WORDS IN A BLOCK
  Vector<WordId> _boundaryWordIds;

  //! WRITE A BLOCK OF HYB TO THE INDEX FILE
  void writeCurrentBlockToIndexFile(DocList& doclistForCurrentBlock, 
                                    WordList& wordlistForCurrentBlock, 
                                    Vector<DiskScore>& scorelistForCurrentBlock, 
                                    Vector<Position>& positionlistForCurrentBlock, 
                                    BlockId currentBlock,
                                    File& indexStructureFile);
 private:
  void writeMetaInfo(File *file);

  void readBlockOffsets(File* file, off_t lastOffsetOffset,
      off_t indexTableOffset);
  void readBoundaryWordIds(File* file);
  void readMetaInfo(File* file, off_t indexTableOffset);

  void showBuildStatistics(off_t fileSizeInBytes);
  
}; // end of class HYBIndex

#endif
