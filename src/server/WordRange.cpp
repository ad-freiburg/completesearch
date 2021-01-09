#include "WordRange.h"

//! Construct word range with given boundarie
WordRange::WordRange(WordId first, WordId second)
{
  assert(first <= second);
  _first = first;
  _second = second;
}


//! Get size of range (last - first + 1); 0 means empty range, -1 means infinite range
WordId WordRange::size() const 
{
  if (_first < 0 && _second < 0)       return 0;
  else if (_first < 0 && _second >= 0) return -1;
  else                                 return _second - _first + 1;      
}


//! Return true iff given word id is in range
bool WordRange::isInRange(const WordId& wordId) const
{
  if (isInfiniteRange()) return true;
  assert(_first >= 0);      
  assert(wordId >= 0);
  return wordId >= _first && wordId <= _second;
}

//! Print word range to stream
ostream& operator<<(ostream& os, const WordRange& wordRange)
{
  if (wordRange.isInfiniteRange()) os << "infinite";
  else                             os << wordRange.firstElement() << ".." 
                                      << wordRange.lastElement();
  return os;
}


