// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright Hannah Bast 2010

#ifndef CODEBASE_SERVER_WORDRANGE_H_
#define CODEBASE_SERVER_WORDRANGE_H_

#include <gtest/gtest.h>
#include <iostream>
#include "./Globals.h"
#include "./assert.h"


/** A range of word ids.
 *
 *     If (x, y) is a range, then
 *       - it is empty range when x < 0 && y < 0.
 *       - it is infinite range when x < 0 && y >= 0.
 *       - it has range size y - x + 1.
 *       - it is infinite range (too) when the last expression evaluates to -1.
 *       - it is empty range (too) when the last expressioin evaluates to 0.
 *
 *    Note: anything with a negative first and non-negative second component is
 *    taken as infinite range.
 *
 *    @todo: this would be fine, if one need not know that outside of the class,
 *    but that is not the case it seem.
 *
 *    @todo: it's not nice to have WordId of type int just because of this hacky
 *    way of doing things. (Especially given that all other types, like DocId,
 *    Position, etc. are unsigned int.)
 */
class WordRange
{
 private:

    //! Left boundary of word range (still belongs to range)
    WordId _first;

    //! Right boundary of word range (still belongs to range)
    WordId _second;

  public:

    //! Construct empty word range
    FRIEND_TEST(WordRangeTest, constructor);
    WordRange() { _first = _second = -1; }

    //! Construct word range with given boundarie
    WordRange(WordId first, WordId second);

    //! Get first element from range
    WordId firstElement() const { return _first; }

    //! Get last element from range
    WordId lastElement() const { return _second; }

    //! Get size of range.
    //! size = (last - first + 1). 0 means empty range, -1 means infinite range.
    WordId size() const;

    //! Set WordRange infinite.
    void setToInfiniteRange() { _first = -1; _second = 0; }  // NOLINT

    //! Set WordRange empty.
    void setToEmptyRange() { _first = -1; _second = -1; }  // NOLINT

    //! Return true iff given word id is in range.
    //! @todo(hoffmaje): WordId is a signed integer. So with a empty range
    //! (-1,-1) this method returns true for a given WordId = -1 though.
    bool isInRange(const WordId& wordId) const;

    //! Return true iff this is empty word range.
    bool isEmptyRange() const { return _first < 0 && _second < 0; }

    //! Return true iff this is infinite word range.
    bool isInfiniteRange() const { return _first < 0 && _second >= 0; }

    //! Get i-th id from range. 0-th id = first
    const WordId operator[](unsigned long i) const { return (_first + i); }
};

const WordRange infiniteWordIdRange(-1, 0);

//! Print word range to stream
ostream& operator<<(ostream& os, const WordRange& wordRange);

#endif  // CODEBASE_SERVER_WORDRANGE_H_
