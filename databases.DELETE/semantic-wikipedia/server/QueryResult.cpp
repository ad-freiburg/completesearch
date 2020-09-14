// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <sstream>

#include "./QueryResult.h"

using std::string;

namespace ad_semsearch
{
// _____________________________________________________________________________
void QueryResult::asXml(string* xml)
{
  std::ostringstream os;
  // WORDS BOX
  os << "<words total=\"" << _totalNofWords << "\"" << " sent=\""
      << _words.size() << "\"" << " first=\"" << _firstWord << "\">\r\n";
  for (size_t i = 0; i < _words.size(); ++i)
  {
    os << "<w score=\"" << _words[i]._score << "\">" << "<![CDATA["
        << _words[i]._item << "]]>" << "</w>\r\n";
  }
  os << "</words>\r\n";

  // CLASSES BOX
  os << "<classes total=\"" << _totalNofClasses << "\"" << " sent=\""
      << _classes.size() << "\"" << " first=\"" << _firstClass << "\">\r\n";
  for (size_t i = 0; i < _classes.size(); ++i)
  {
    os << "<c score=\"" << _classes[i]._score << "\">" << "<![CDATA["
        << _classes[i]._item << "]]>" << "</c>\r\n";
  }
  os << "</classes>\r\n";

  // RELATIONS BOX
  os << "<relations total=\"" << _totalNofRelations << "\"" << " sent=\""
      << _relations.size() << "\"" << " first=\"" << _firstRelation
      << "\">\r\n";
  for (size_t i = 0; i < _relations.size(); ++i)
  {
    os << "<r score=\"" << _relations[i]._score << "\" source=\""
        << _relations[i]._lhsType << "\" target=\"" << _relations[i]._rhsType
        << "\" reversed=\"" << (_relations[i]._reversed ? "true" : "false")
        << "\">" << "<![CDATA[" << _relations[i]._item << "]]>" << "</r>\r\n";
  }
  os << "</relations>\r\n";

  // INSTANCES BOX
  os << "<instances total=\"" << _totalNofInstances << "\"" << " sent=\""
      << _instances.size() << "\"" << " first=\"" << _firstInstance
      << "\">\r\n";
  for (size_t i = 0; i < _instances.size(); ++i)
  {
    os << "<i score=\"" << _instances[i]._score << "\">" << "<![CDATA["
        << _instances[i]._item << "]]>" << "</i>\r\n";
  }
  os << "</instances>\r\n";

  // HITS
  os << "<hitgroups total=\"" << _totalNofHitGroups << "\"" << " sent=\""
      << _hitGroups.size() << "\"" << " first=\"" << _firstHitGroup
      << "\">\r\n";
  for (size_t i = 0; i < _hitGroups.size(); ++i)
  {
    os << "<group>\r\n" << "<groupname>" << "<![CDATA["
        << _hitGroups[i]._entity << "]]>" << "</groupname>\r\n";
    for (size_t j = 0; j < _hitGroups[i]._hits.size(); ++j)
    {
      os << "<hit " << "score=\"" << _hitGroups[i]._hits[j].getScore() << "\" "
          << "contextId=\"" << _hitGroups[i]._hits[j].getContextId()
          << "\">\r\n"
          << "<title>" << "<![CDATA["
          << _hitGroups[i]._hits[j].getTitle() << "]]>"
          << "</title>\r\n" << "<url>" << "<![CDATA["
          << _hitGroups[i]._hits[j].getUrl() << "]]>" << "</url>\r\n"
          << "<excerpt>" << "<![CDATA["
          << _hitGroups[i]._hits[j].getExcerptWithHighlighting() << "]]>"
          << "</excerpt>\r\n" << "</hit>\r\n";
    }
    os << "</group>\r\n";
  }
  os << "</hitgroups>\r\n";
  *xml = os.str();
}
}
