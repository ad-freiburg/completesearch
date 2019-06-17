// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_OUTPUTWRITER_H_
#define SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_OUTPUTWRITER_H_

#include <ctype.h>
#include <vector>
#include <string>
#include "./OntologyRelation.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/semantic-wikipedia-utils/Conversions.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"


using std::vector;
using std::string;

namespace ad_semsearch
{
class OutputWriter
{
  public:
    //! Method that writes the output depending on the current state
    //! of the ontology.
    virtual void generateOutput(vector<OntologyRelation> allRelations) = 0;

    //! Virtual destructor
    virtual ~OutputWriter()
    {
    }

    // Normalize an entity wrt current maps.
    virtual string normalizeEntity(const string& words) const
    {
      return ad_semsearch::normalizeEntity(words, _redirectMap,
          _wordnetSynonyms, false);
    }

    // Operator needed for hashing a string.
    class HashString
    {
      public:
        size_t operator()(const string str) const
        {
          size_t x = 0;
          const char* s = str.c_str();
          while (*s != 0)
          {
            x = 31 * x + *s++;
          }
          return x;
        }
    };

    void setOntologyNamesMapping(const string& fileName);
    void readStringStringMap(const std::string& fileName,
        ad_utility::HashMap<string, string>* targetMap);

    void readRedirectMap(const std::string& fileName)
    {
      readStringStringMap(fileName, &_redirectMap);
    }

    void readWordnetSynonyms(const std::string& fileName)
    {
      readStringStringMap(fileName, &_wordnetSynonyms);
    }

  protected:
    virtual const string& mapOntologyElementName(const string& orig) const
    {
      ad_utility::HashMap<string, string>::const_iterator it = _nameMap.find(
          orig);
      return (it != _nameMap.end() ? it->second : orig);
    }

    virtual string getOntologyElementStringWithPrefix(const string& orig,
        const string& prefix) const
    {
      string mapped = normalizeEntity(mapOntologyElementName(orig));
      return prefix + ad_utility::getNormalizedLowercase(mapped) + ":"
          + mapped;
    }

    ad_utility::HashMap<std::string, std::string> _redirectMap;
    ad_utility::HashMap<std::string, std::string> _wordnetSynonyms;
    ad_utility::HashMap<string, string> _nameMap;
};
}
#endif  // SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_OUTPUTWRITER_H_
