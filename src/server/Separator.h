// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#ifndef SERVER_SEPARATOR_H__
#define SERVER_SEPARATOR_H__

#include <vector>
#include <string>
#include <utility>
#include "./Globals.h"

/* A Separator between two query words.
 *
 * E.g. .. or .;;;.
 * @todo(foobar): not | right?
 * 
 * @todo(foobar): I already tidied this up a bit but it's still a terrible,
 * unintuitive mess, which costs energy each time one ist looking at it.
 */
class Separator
{
  public:

    //! The range defining what counts as an intersection, e.g. [-5,5] for ..
    pair<int, int> _intersectionWindow;

    //! The string used to denote the separator; can be fixed/known in advance,
    //! or only become known later
    string _separatorString;

    //! Output modes
    /*! 
     *    OUTPUT_MATCHES     : normal intersect, outputs a subset R of L2
     *    OUTPUT_NON_MATCHES : outputs exactly L2 - R (Ingmar's notIntersection mode)
     *    OUTPUT_ALL         : outputs L2 (but scores for elements from R
     *                         are aggregated with corr. ones from L1)
     */
    enum OutputMode
    {
      OUTPUT_MATCHES     = 0,
      OUTPUT_NON_MATCHES = 1,
      OUTPUT_ALL         = 2
    };

    //! The output mode.
    int _outputMode;

    /** The index of the separator in the Separators class below.
     *
     *  @todo(foobar): This is absolutely weird. If anything, there could be
     *  an emum of the predefined separators we have, with meaningful names,
     *  e.g. enum SeparatorEnum { SAME_DOCUMENT, PHRASE, ... };
     *
     *  @todo(foobar): Giving -2 a special meaning is evil, how is one
     *  supposed to understand this in the code where this is applied (and
     *  where is it applied?)
     *
     *  -1 for empty Separator, -2 for flexi separator.
     */
    signed char _separatorIndex;

    //! Default constructor.
    //! Empty string, window = [0,0], index = -1.
    //! @todo(foobar): Why?
    Separator();

    //! Construct separator from string and range and flexi yes/no.
    //! @todo(foobar): Why have to say FLEXI, doesn't the window dimensions
    //! say it all?
    Separator(string separatorString,
              pair<signed int,
              signed int> intersectionWindow,
              char separatorIndex = -2);

    //! Set intersection window
    void setIntersectionWindow(int left, int right)
    { _intersectionWindow.first = left;
      _intersectionWindow.second = right; }

    //! Set output mode
    void setOutputMode(OutputMode outputMode) { _outputMode = outputMode; }

    //! Get left boundary of intersection window, e.g. -5 for default proximity
    int getIntersectionWindowLeftBoundary() const
    { return _intersectionWindow.first; }

    //! Get right boundary of intersection window, e.g. 5 for default proximity
    int getIntersectionWindowRightBoundary() const
    { return _intersectionWindow.second; }

    bool hasInfiniteIntersectionWindow() const
    { return _separatorIndex == SAME_DOC; }

    //! Get output mode
    int getOutputMode() const { return _outputMode; }

    //! Get separator string, e.g. ".." for default proximity
    string getSeparatorString() const { return _separatorString; }

    //! Check if string has flexi separators; e.g. .,,,. or .;;;;;.
    //! @todo(foobar):: What exactly is given?
    //! @param separatorString ...
    //! @param flexiSeparator  ...
    //! @return                ...
    bool parseFlexiSeparatorString(const string& separatorString,
                                   Separator& flexiSeparator) const;

    //! Get inverse of this separator, e.g. the inverse of [1,1] is [-1,-1]
    /*!
     *   NEW 06Feb08, needed for the new snippet generation
     */
    void getInverseSeparator(Separator* inverseSeparator) const;

    //! Get description of output mode as one of M, N, A, or X (invalid)
    static char getOutputModeAsChar(int outputMode);

    //! Get info about this separator as string, e.g. -5..5 M (for debugging
    //! only)
    string infoString() const;

    //! Show information about this separator (for debugging only)
    void show() const;
};  // end class Separator


//! Default separator (space = match iff both words occur somewhere in the
//! same document)
//!
//! @todo(foobar):: Still uses Ingmar's weird signature with the index
//! (third argument)
const Separator sameDocSeparator(" ", pair<signed int, signed int>(-1, -1), 0);

//! All our separators.
//! @todo(foobar): Why in an own class?
class Separators
{
  public:
    vector<Separator> _separators;
    Separators();
};


//! Our separators as global variable;
extern const Separators fixed_separators;

#endif  // SERVER_SEPARATOR_H__
