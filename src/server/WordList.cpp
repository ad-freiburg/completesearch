#include "WordList.h"

// ____________________________________________________________________________
WordList::WordList()
{
  // NEW 22Feb12 (Ina): Changed from Vector<WordId>::Vector<WordId>() to
  // Vector<WordId>(), since the newest standard of G++ (4.6) can't work with
  // it anymore and throws an error.
  // Nevertheless I am bit irritated, if that version can't lead to an
  // error, since the used class is not explicitly written anymore.
  // Officially WordList::Vector<WordID> would be correct, but doesn't work
  // with G++ (4.4). To provide backward-compatibility it's done this way.
  // More information on following sites:
  // http://www.open-std.org/jtc1ß22/wg21/docs/cwg_defects.html#147
  // http://www.open-std.org/jtc1ß22/wg21/docs/cwg_defects.html#176
  // http://gcc.gnu.org/gcc-4.5/changes.html
  Vector<WordId>();
  _size = 0;
  _isRepeated = false;
}

// ____________________________________________________________________________
WordList::WordList(const WordId& element, unsigned long nofRepetitions)
{
  assert(nofRepetitions>0);
  Vector<WordId>::resize(1);
  Vector<WordId>::operator[](0) = element;

  _isFullList = false;
  _isRepeated = true;
  _size = nofRepetitions;

  assert(getNofElements() == nofRepetitions);
}

// ____________________________________________________________________________   
void WordList::show(unsigned int nof_items_to_show) const
{
  size_t n = MIN(size(), nof_items_to_show);
  for (size_t i = 0; i < n; ++i)
  {
    cout << Vector<WordId>::operator[](i);
    if (i + 1 < n) cout << "\t";
  }
  cout << "\n";
}

// ____________________________________________________________________________
void WordList::show(ostringstream& outputStream,
                    const Vocabulary& vocabulary,
                    unsigned int nof_items_to_show) const
{
  for(unsigned int i = 0; i<MIN(size(),nof_items_to_show);i++)
  {
    assert(Vector<WordId>::operator[](i) < (WordId) vocabulary.size());
    outputStream << vocabulary[Vector<WordId>::operator[](i)];
    if ((i+1) < MIN(size(),nof_items_to_show))
    {
      outputStream << "\t";
    }
  }
  outputStream << "\n";
}
   
// ____________________________________________________________________________   
void WordList::show(const Vocabulary& vocabulary,
                    unsigned int nof_items_to_show) const
{
  for(unsigned int i = 0; i<MIN(size(),nof_items_to_show);i++)
  {
    assert(Vector<WordId>::operator[](i) < (WordId) vocabulary.size());
    cout << vocabulary[Vector<WordId>::operator[](i)];
    if ((i+1) < MIN(size(),nof_items_to_show)) cout << "\t";
  }
  cout << "\n";
}
   
// ____________________________________________________________________________
string WordList::debugString()
{
  ostringstream os;
  os << "{";
  for (size_t i = 0; i < size(); ++i)
  {
    if (i > 0) os << ",";
    os << operator[](i);
  }
  os << "}";
  return os.str();
}
