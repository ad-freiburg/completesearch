// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_HITLIST_H_
#define SEMANTIC_WIKIPEDIA_SERVER_HITLIST_H_

#include <ostream>
#include <string>
#include <vector>
#include "./Excerpt.h"
#include "./List.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
//! Represent a hit either from the full-text or from the ontology.
//! Purely associated with outputting a query result,
//! Contains pretty excerpts and about everything interesting
//! to output for a single hit.
class Hit
{
  public:
    //! Default ctor.
    Hit()
    {
    }

    //! Typical ctor without matching entities, calculates excerpt
    Hit(Id contextId, const string& rawExcerpt,
        const Excerpt::Highlights highlights, AggregatedScore score)
    : _contextId(contextId), _excerpt(rawExcerpt, highlights),
      _score(score), _matchedEntities()
    {
    }

    //! Typical ctor with matching entities, calculates excerpt
    Hit(Id contextId, const string& rawExcerpt,
        const Excerpt::Highlights highlights, AggregatedScore score,
        const vector<Id>& entities)
    : _contextId(contextId), _excerpt(rawExcerpt, highlights), _score(score),
      _matchedEntities(entities)
    {
    }

    //! Typical ctor without matching entities, copies excerpt
    Hit(Id contextId, const Excerpt& excerpt, AggregatedScore score)
    : _contextId(contextId), _excerpt(excerpt),
      _score(score), _matchedEntities()
    {
    }

    //! Typical ctor with matching entities, copies excerpt
    Hit(Id contextId, const Excerpt& excerpt, AggregatedScore score,
        vector<Id> entities)
    : _contextId(contextId), _excerpt(excerpt), _score(score),
      _matchedEntities(entities)
    {
    }

    Id getContextId()  const
    {
      return _contextId;
    }

    //! Getter for lazy field.
    const string& getExcerptWithHighlighting()
    {
      return _excerpt.getExcerptWithHighlighting();
    }

    //! Getter.
    AggregatedScore getScore() const
    {
      return _score;
    }

    //! Setter that triggers computation.
    void setRawExcerptString(const string& excerptFromFile)
    {
      _excerpt.parseRawExcerpt(excerptFromFile);
    }

    //! Setter. Will cause lazy computation of future gets on the excerpts.
    void setHightlights(const Excerpt::Highlights& highlights)
    {
      _excerpt.setHighlights(highlights);
    }

    //! Setter.
    void setContextId(Id contextId)
    {
      _contextId = contextId;
    }

    //! Setter.
    void setScore(Score score)
    {
      _score = score;
    }

    //! Getter
    const vector<Id>& getMatchedEntities() const
    {
      return _matchedEntities;
    }

    //! Setter
    void setMatchedEntities(const vector<Id> entities)
    {
      _matchedEntities = entities;
    }

    //! Get a human readable string.
    string asString() const
    {
      std::ostringstream os;
      os << "(ContextId: " << _contextId << ", Score: "
          << static_cast<int> (_score) << ")";
      return os.str();
    }

    //! Full string representation for this hit. Not const since it may
    //! cause computation of the highlighted excerpt string.
    string fullString()
    {
      std::ostringstream os;
      os << "(ContextId:" << _excerpt.getContextId() << ", URL:"
          << _excerpt.getUrl() << ", Title:" << _excerpt.getTitle()
          << ", Excerpt:" << _excerpt.getExcerptWithHighlighting()
          << ", Score: " << static_cast<int>(_score) << ")";
      return os.str();
    }

    //! Pretty string representation for this hit. Not const since it may
    //! cause computation of the highlighted excerpt string.
    string prettyString()
    {
      std::ostringstream os;
      os <<  _excerpt.getExcerptWithHighlighting() << " ("
          << static_cast<int>(_score) << ")";
      return os.str();
    }

    const string& getUrl() const
    {
      return _excerpt.getUrl();
    }

    const string& getTitle() const
    {
      return _excerpt.getTitle();
    }

  private:
    Id _contextId;
    Excerpt _excerpt;
    AggregatedScore _score;
    vector<Id> _matchedEntities;
};

typedef List<Hit> HitList;

//! Stream operator for convenience.
inline std::ostream& operator<<(std::ostream& stream, Hit& hit)
{
  return stream << hit.prettyString();
}
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_HITLIST_H_
