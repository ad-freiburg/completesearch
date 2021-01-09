#ifndef __DOCLIST_H__
#define __DOCLIST_H__

#include "Vector.h"
//#include "assert.h"
//#include <iostream> //for cout

#include "Globals.h"

class DocList : public Vector<DocId>
{
 private:
  // Leave this private !
  bool _isMarkedSorted;

 public:
  void markAsSorted(bool isMarkedSorted) {_isMarkedSorted = isMarkedSorted;}
  bool isMarkedSorted() const {return _isMarkedSorted;}

  DocList()
    {
      _isMarkedSorted = false;
      // NEW 22Feb12 (Ina): Changed from Vector<DocId>::Vector<DocId>() to
      // Vector<DocId>(), since the newest standard of G++ (4.6) can't work
      // with it anymore and throws an error.
      // Nevertheless I am bit irritated, if that version can't lead to an
      // error, since the used class is not explicitly written anymore.
      // Officially DocList::Vector<DocId> would be correct, but doesn't work
      // with G++ (4.4). To provide backward-compatibility it's done this way.
      // More information on following sites:
      // http://www.open-std.org/jtc1ß22/wg21/docs/cwg_defects.html#147
      // http://www.open-std.org/jtc1ß22/wg21/docs/cwg_defects.html#176
      // http://gcc.gnu.org/gcc-4.5/changes.html
      Vector<DocId>();
    }
  
  void copy(const DocList& source)
    {
      Vector<DocId>::copy(source);
      markAsSorted(source.isMarkedSorted());
    }

  // NEW 22Feb12 (Ina): Changed from Vector<DocId>::Vector<DocId>(isFullList) to Vector<DocId>, since the
  // newest standard of G++ (4.6) can't work with it anymore and throws an error.
  DocList(bool isFullList) : Vector<DocId>::Vector(isFullList)
    {
      _isMarkedSorted = false;
    }
  
  bool isEmpty() const
    {
      return Vector<DocId>::isEmpty();
    }

};


#endif
