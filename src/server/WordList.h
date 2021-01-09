#ifndef __WORDLIST_H__
#define __WORDLIST_H__

#include "Globals.h"
#include "Vector.h"
#include "Vocabulary.h"


// List of word ids.
// NOTE: a repeatedVector is an ordinary Vector (which if STL_VECTOR is defined
// is an ordinary STL vector), with the option to contain just one element
// repeated many times. This is used for INV, where we have on block per word,
// and all word ids in a block are the same.
class WordList : public repeatedVector<WordId>
{
 public:
	// Create empty word list.
  WordList();

	// Create word list with one element repeated the given number of times.
  WordList(const WordId& element, unsigned long nofRepetitions);

	// Print the first items from the word list on the screen / to the given
	// stream.
	void show(unsigned int nof_items_to_show = 20) const;
  void show(ostringstream& outputStream,
			      const Vocabulary& vocabulary,
						unsigned int nof_items_to_show = 20) const;
  void show(const Vocabulary& vocabulary,
			      unsigned int nof_items_to_show = 20) const;
   
  // Contents of the vector in human-readable form.
  string debugString();
};


#endif
