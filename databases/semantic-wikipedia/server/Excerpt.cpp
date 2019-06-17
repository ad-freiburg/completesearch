// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include "../codebase/semantic-wikipedia-utils/Log.h"
#include "./Excerpt.h"

using std::string;

namespace ad_semsearch
{
// _____________________________________________________________________________
void Excerpt::parseRawExcerpt(const string& rawExcerpt)
{
  size_t indexOfTab1 = rawExcerpt.find('\t');
  size_t indexOfTab2 = rawExcerpt.find('\t', indexOfTab1 + 1);
  size_t indexOfTab3 = rawExcerpt.find('\t', indexOfTab2 + 1);

  _contextId = rawExcerpt.substr(0, indexOfTab1);
  _url = rawExcerpt.substr(indexOfTab1 + 1, indexOfTab2 - (indexOfTab1 + 1));
  _title = rawExcerpt.substr(indexOfTab2 + 1, indexOfTab3 - (indexOfTab2 + 1));
  _originalExcerpt = rawExcerpt.substr(indexOfTab3 + 1);
  _excerptWithHighlighting = "";
}
// _____________________________________________________________________________
const string& Excerpt::getExcerptWithHighlighting()
{
  // If the version with highlighting stored as member is nonempty.
  // we do not have to do anything at all
  if (_excerptWithHighlighting.size() == 0)
  {
    // Otherwise construct the version with highlighting.
    size_t i = 0;
    Position currentPos = 0;
    Highlights::const_iterator posToHighlightIterator = _highlights.begin();

    bool currentlyHighlighting = (posToHighlightIterator != _highlights.end()
        && *posToHighlightIterator == currentPos);
    if (currentlyHighlighting)
    {
      _excerptWithHighlighting += HIGHLIGHT_START;
    }

    while (i < _originalExcerpt.size())
    {
      // Check if we're seeing a delimiter start.
      if (delimiterStartsAtPos(_originalExcerpt, i))
      {
        // If we do, update state.
        ++currentPos;
        i += strlen(DOCSFILE_POS_DELIMITER);
        if (currentlyHighlighting)
        {
          _excerptWithHighlighting += HIGHLIGHT_END;
          currentlyHighlighting = false;
        }
        if (!currentlyHighlighting && posToHighlightIterator
            != _highlights.end() && *posToHighlightIterator == currentPos)
        {
          _excerptWithHighlighting += HIGHLIGHT_START;
          currentlyHighlighting = true;
          ++posToHighlightIterator;
        }
      }
      else
      {
        // Otherwise just copy the text.
        _excerptWithHighlighting += _originalExcerpt[i];
        ++i;
      }
    }
    // Close unclosed highlighting tags.
    if (currentlyHighlighting)
    {
      _excerptWithHighlighting += HIGHLIGHT_END;
      currentlyHighlighting = false;
    }
  }
  return _excerptWithHighlighting;
}
}
