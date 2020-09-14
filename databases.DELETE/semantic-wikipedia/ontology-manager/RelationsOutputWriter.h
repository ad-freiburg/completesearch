// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_RELATIONSOUTPUTWRITER_H_
#define SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_RELATIONSOUTPUTWRITER_H_

#include <string>
#include <fstream>
#include <vector>
#include "./OutputWriter.h"
#include "./OntologyRelation.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"

using std::string;
using std::vector;
using std::fstream;

namespace ad_semsearch
{
class RelationsOutputWriter: public OutputWriter
{
  public:
    explicit RelationsOutputWriter(const string& outputFile,
        bool alsoWriteIsA = true);

    virtual ~RelationsOutputWriter();

    void generateOutput(vector<OntologyRelation> relations);
    void writeNonTransitiveRelation(const OntologyRelation& relation,
        fstream& fs) const;
    void writeTransitiveRelation(
        const OntologyRelation& relation, fstream& fs) const;

    // Overwrite normalization method.
    // Use the only that redirects if:
    //   a) source and destination only differ in case
    //   OR
    //   b) at most one out of source and destination has associated facts.
    virtual string normalizeEntity(const string& words) const
    {
      return ad_semsearch::normalizeEntity(words, _redirectMap,
          _thingsWithFacts);
    }

  private:
    string _outputFile;
    bool _alsoWriteIsA;
    ad_utility::HashSet<string> _thingsWithFacts;
};
}

#endif  // SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_RELATIONSOUTPUTWRITER_H_
