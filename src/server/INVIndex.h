#ifndef __INVINDEX_H__
#define __INVINDEX_H__

#include "Globals.h"
#include "IndexBase.h"
#include "Vector.h"
#include "File.h"
#include "MetaInfo.h"

//! Class for building an accessing an inverted index (based on a single large file)
class INVIndex : public IndexBase
{
 public:

  //! Create an inverted index with the given files in the given modes.  To
  //! load the data, call either read or build.
  INVIndex(const string&       indexFileName, 
           const string&       vocabularyFileName, 
	         unsigned char mode);

  //! Read the index from the index and vocabulary file passed to the
  //! constructor.
  void read() override;

  //! Build the index from the given words file in the given format and
  //! write it to the index and vocabulary file passed to the constructor.
  void build(const string& wordsFileName, const string& format) override;

  //! Show meta information of index: number of docs, words, etc.
  void showMetaInfo() const;

  //! The offsets of the inverted lists in the index file
  vector<off_t> _byteOffsetsForDoclists;
  
 private:

  //! Write inverted list to file; TODO: true? (was not commented before)
  void writeCurrentDataToIndexFile
        (DocList&           doclistForCurrentWord, 
         Vector<DocId>&     doclistLengthsForWordId, 
	 Vector<DiskScore>& scorelistForCurrentWord, 
	 Vector<Position>&  positionlistForCurrentWord,
         File&              indexStructureFile);

}; // end of class INVIndex


#endif
