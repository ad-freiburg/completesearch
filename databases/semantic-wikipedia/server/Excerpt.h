// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_EXCERPT_H_
#define SEMANTIC_WIKIPEDIA_SERVER_EXCERPT_H_

#include <gtest/gtest.h>
#include <ostream>
#include <string>
#include <vector>

#include "./Identifiers.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! An excerpt is an excerpt form the original full-text.
//! Excerpts are read from file and contain special position delimiters.
//! For given results, the delimiters are removed and instead words
//! that are supposed to be highlighted can be marked for highlighting.
class Excerpt
{
  public:

    // Positions to highlight. Currently it is a mere list of positions,
    // but may be changed to "Highlight" objects that allow distinguishing
    // different words that are supposed to get some positions highlighted
    // differently.
    typedef vector<Position> Highlights;

    Excerpt()
    : _contextId(), _url(), _title(), _originalExcerpt(),
      _highlights(), _excerptWithHighlighting()
    {
    }

    explicit Excerpt(const string& rawExcerpt)
    : _contextId(), _url(), _title(), _originalExcerpt(), _highlights(),
          _excerptWithHighlighting()
    {
      parseRawExcerpt(rawExcerpt);
    }

    Excerpt(const string& rawExcerpt, const Highlights& highlights)
    : _contextId(), _url(), _title(), _originalExcerpt(),
      _highlights(highlights), _excerptWithHighlighting()
    {
      parseRawExcerpt(rawExcerpt);
    }

    Excerpt(const string& lhs, const string& relation, const string& rhs)
    : _contextId("YAGO"), _url(YAGO_URL), _title("YAGO Ontology"),
      _originalExcerpt(), _highlights(),
      _excerptWithHighlighting(lhs + " " + relation + " " + rhs + ".")
    {
    }

    Excerpt(const Excerpt& other)
    :
      _contextId(other._contextId), _url(other._url), _title(other._title),
      _originalExcerpt(other._originalExcerpt),
      _highlights(other._highlights),
      _excerptWithHighlighting(other._excerptWithHighlighting)
    {
    }

    Excerpt& operator=(const Excerpt& rhs)
    {
      _contextId = rhs._contextId;
      _url = rhs._url;
      _title = rhs._title;
      _originalExcerpt = rhs._originalExcerpt;
      _highlights = rhs._highlights;
      _excerptWithHighlighting = rhs._excerptWithHighlighting;
      return *this;
    }

    virtual ~Excerpt()
    {
    }

    //! Gets the version with highlighting. Lazily constructs the highlited
    //! version if necessary.
    const string& getExcerptWithHighlighting();

    //! Set the original string, title, url and id
    //! Marks the highlighted version of the excerpt for recomputation.
    void parseRawExcerpt(const string& rawExcerpt);

    //! Set which positions should be hihglighted.
    //! Marks the highlighted version of the excerpt for recomputation.
    void setHighlights(const vector<Position>& highlights)
    {
      _highlights = highlights;
      _excerptWithHighlighting = "";
    }

    // Pure Getters:

    const string& getContextId() const { return _contextId; }
    const string& getUrl() const { return _url; }
    const string& getTitle() const { return _title; }

    // Friend tests:
    FRIEND_TEST(ExcerptTest, delimiterStartsAtPosTest);

  private:
    string _contextId;
    string _url;
    string _title;
    string _originalExcerpt;
    Highlights _highlights;
    string _excerptWithHighlighting;

    //! Checks if the docsfile delimiter starts at a given pos in text.
    static bool delimiterStartsAtPos(const string& text, size_t pos)
    {
      if (pos + strlen(DOCSFILE_POS_DELIMITER) <= text.size())
      {
        for (size_t i = 0; i < strlen(DOCSFILE_POS_DELIMITER); ++i)
        {
          if (text[pos + i] != DOCSFILE_POS_DELIMITER[i]) return false;
        }
        return true;
      }
      return false;
    }
};
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_EXCERPT_H_
