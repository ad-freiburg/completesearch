// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include "./Separator.h"
#include <string>
#include <utility>


// _____________________________________________________________________________
//! Default constructor (empty string, window = [0,0], index = -1).
//! @todo(foobar):: Why?
Separator::Separator()
{
  _separatorString    = "";
  _intersectionWindow = pair<signed int, signed int> (0, 0);
  _separatorIndex     = -1;
  _outputMode         = OUTPUT_MATCHES;
}


// _____________________________________________________________________________
//! Construct separator from string and range and flexi yes/no.
//! @todo(foobar): Why have to say FLEXI? Doesn't the window dimensions say it
//! all?
Separator::Separator(string separatorString,
                     pair<signed int, signed int> intersectionWindow,
                     char separatorIndex)
{
  _separatorString    = separatorString;
  _intersectionWindow = intersectionWindow;
  _separatorIndex     = separatorIndex;
  _outputMode         = OUTPUT_MATCHES;
}


// _____________________________________________________________________________
bool Separator::parseFlexiSeparatorString(const string& separatorString,
                                          Separator& flexiSeparator) const
{
  #ifndef NDEBUG
  cout << "! Checking if this string is a flexi separator string : \""
       << separatorString << "\""<< endl;
  #endif

  if ((separatorString.length() < 3) ||
      (separatorString.substr(0, 1) != ".") ||
      (separatorString.substr(separatorString.length()-1, 1) != ".") ||
      ((separatorString.substr(1, 1) != ",") &&
       (separatorString.substr(1, 1) != ";")))
  {
    #ifndef NDEBUG
    cout << "! NOT A VALID FLEXI STRING (trivial checks) !" << endl;
    #endif
    flexiSeparator = Separator();
    return false;
  }

  // iterator over the commas.
  signed int numberOfThings = 0;  // commas or semicolons
  string::const_iterator currentPositionInSeparatorString =
    separatorString.begin();
  assert(static_cast<char>(*currentPositionInSeparatorString) == '.');
  ++currentPositionInSeparatorString;
  char lastCharacter = 0;
  // While current position is a comma or semicolon.
  while ((static_cast<char>(*currentPositionInSeparatorString) == ',') ||
         (static_cast<char>(*currentPositionInSeparatorString) == ';'))
  {
    if ((lastCharacter != 0) &&
        (static_cast<char>(*currentPositionInSeparatorString) !=
         lastCharacter))
    {
        lastCharacter = 0;
        break;
    }
    lastCharacter = static_cast<char>(*currentPositionInSeparatorString);
    ++currentPositionInSeparatorString;
    numberOfThings++;
  }

  if ((lastCharacter != 0) &&
     ((currentPositionInSeparatorString + 1) == separatorString.end()) &&
      (numberOfThings > 0))
  {
    signed int signOfWindow = ((lastCharacter == ',') ? (1) : (-1));
    flexiSeparator = Separator(separatorString,
        pair<signed int, signed int>(signOfWindow*numberOfThings,
                                     signOfWindow*numberOfThings));
    #ifndef NDEBUG
    cout << "! IS A VALID FLEXI STRING !" << endl;
    #endif
    return true;
  }
  else
  {
    #ifndef NDEBUG
    cout << "! NOT A VALID FLEXI STRING (after closer look) !" << endl;
    #endif
    flexiSeparator = Separator();
    return false;
  }
}

// _____________________________________________________________________________
//! Get description of output mode as one of M, N, A, or X (invalid)
char Separator::getOutputModeAsChar(int outputMode)
{
  string codes = "MNAX";
  return codes[MIN(outputMode >= 0 ? outputMode : INT_MAX, 4)];
}


// _____________________________________________________________________________
//! Get info about this separator as string, e.g. -5..5 M (for debugging only)
string Separator::infoString() const
{
  ostringstream os;
  os  << getIntersectionWindowLeftBoundary() << ".."
      << getIntersectionWindowRightBoundary() << " "
      << getOutputModeAsChar(_outputMode);
  return os.str();
}


// _____________________________________________________________________________
//! Show information about this separator (for debugging only)
void Separator::show() const
{
  cout << "Separator string: -->" << _separatorString << "<--" << endl;
  cout << "Intersection window [" << _intersectionWindow.first << ","
       << _intersectionWindow.second << "]" << endl;
}


// _____________________________________________________________________________
//! Our separators, as a single global variable; TODO: this is sick, puke, aargh
const Separators fixed_separators;
Separators::Separators()
{
  _separators.push_back(Separator(
        " ",
        pair<signed int, signed int>(-1, -1),
        0));
  // HACK(Jens) 02May10:changed from (1,1) to (0,1) for Alex.
  // NEW(Hannah) 23May16: changed back to 1,1.
  _separators.push_back(Separator(
        ".",
        pair<signed int, signed int>(1, 1),
        1));
  _separators.push_back(Separator(
        "..",
        pair<signed int, signed int>(-NEIGHBORHOOD_SIZE, NEIGHBORHOOD_SIZE),
        2));
  _separators.push_back(Separator(
        "=",
        pair<signed int, signed int>(0, 0),
        3));
}


// _____________________________________________________________________________
//! Get inverse of this separator (NEW 06Feb08, needed for new snippet
//! generation)
void Separator::getInverseSeparator(Separator* inverseSeparator) const
{
  // Weil das alles so Scheisse ist muss ich hier so eine Scheiss
  // Fallunterscheidung machen.
  if (_intersectionWindow.first == -1 && _intersectionWindow.second == -1)
  {
    (*inverseSeparator)._intersectionWindow.first  = -1;
    (*inverseSeparator)._intersectionWindow.second = -1;
    (*inverseSeparator)._separatorIndex = SAME_DOC;
  }
  else
  {
    (*inverseSeparator)._intersectionWindow.first = -_intersectionWindow.second;
    (*inverseSeparator)._intersectionWindow.second = -_intersectionWindow.first;
    (*inverseSeparator)._separatorIndex = FLEXI;
  }
  (*inverseSeparator)._separatorString = "[computed from getInverseSeparator]";
}
