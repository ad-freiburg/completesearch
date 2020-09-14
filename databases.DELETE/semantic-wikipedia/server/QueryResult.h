// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_SERVER_QUERYRESULT_H_
#define SEMANTIC_WIKIPEDIA_SERVER_QUERYRESULT_H_

#include <vector>
#include <string>

#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "./HitList.h"

using std::vector;
using std::string;
using std::endl;

namespace ad_semsearch
{
class ItemWithScore
{
  public:
    ItemWithScore(const string& item, AggregatedScore score) :
        _item(item), _score(score)
    {
    }

    string _item;
    AggregatedScore _score;

    string asString()
    {
      std::ostringstream os;
      os << '(' << _item << ", " << _score << ')';
      return os.str();
    }
};

class RelationBoxEntry : public ItemWithScore
{
  public:
    RelationBoxEntry(const string& item, AggregatedScore score,
        const string& lhsType, const string& rhsType)
    : ItemWithScore(item, score), _lhsType(lhsType), _rhsType(rhsType),
      _reversed(false)
    {
      if (ad_utility::endsWith(_item, REVERSED_RELATION_SUFFIX))
      {
        _reversed = true;
        _item = _item.substr(0,
            _item.size() - strlen(REVERSED_RELATION_SUFFIX));
      }
    }

    string _lhsType;
    string _rhsType;
    bool _reversed;
};

struct HitGroup
{
    HitGroup(const string& entityName, const HitList& hits,
        AggregatedScore score)
    :_entity(entityName), _hits(hits), _score(score)
    {
    }

    string _entity;
    HitList _hits;
    AggregatedScore _score;

    string asString()
    {
      std::ostringstream os;
      os << "(GROUP: " << _entity << '(' << _score << "): " << _hits.asString()
          << ')';
      return os.str();
    }
};

//! Result for a complete query.
//! Contains results for the boxes (entities, hits, relations, etc).
//! Entirely abstracts from query trees, intermediate lists and the likes.
//! Instead this is very close to response that are returned for queries.
class QueryResult
{
  public:

    QueryResult() :
        _instances(), _relations(), _classes(), _words(), _hitGroups(),
        _totalNofInstances(0), _firstInstance(0), _totalNofRelations(0),
        _firstRelation(0), _totalNofClasses(0), _firstClass(0),
        _totalNofWords(0), _firstWord(0), _totalNofHitGroups(0),
        _firstHitGroup(0)
    {
    }

    List<ItemWithScore> _instances;
    List<RelationBoxEntry> _relations;
    List<ItemWithScore> _classes;
    List<ItemWithScore> _words;
    List<HitGroup> _hitGroups;

    size_t _totalNofInstances;
    size_t _firstInstance;
    size_t _totalNofRelations;
    size_t _firstRelation;
    size_t _totalNofClasses;
    size_t _firstClass;
    size_t _totalNofWords;
    size_t _firstWord;
    size_t _totalNofHitGroups;
    size_t _firstHitGroup;

    void asXml(string* xml);
  };

inline std::ostream& operator<<(std::ostream& os,
    const QueryResult& qr)
{
  return
      os << endl << "Query Result: " << endl
      << "\t" << "Instances: " << qr._instances.asString() << endl
      << "\t" << "Relations: " << qr._relations.asString() << endl
      << "\t" << "Classes: " << qr._classes.asString() << endl
      << "\t" << "Words: " << qr._words.asString() << endl
      << "\t" << "Hits: " << qr._hitGroups.asString() << endl;
}
}

#endif  // SEMANTIC_WIKIPEDIA_SERVER_QUERYRESULT_H_
