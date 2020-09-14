// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_RELATIONSOUTPUTWRITEROLD_H_
#define SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_RELATIONSOUTPUTWRITEROLD_H_

#include <string>
#include <fstream>
#include <vector>
#include "./OutputWriter.h"
#include "./OntologyRelation.h"

using std::string;
using std::vector;
using std::fstream;

namespace ad_semsearch
{
class RelationsOutputWriterOld: public OutputWriter
{
  public:
    explicit RelationsOutputWriterOld(const string& outputFile,
        bool alsoWriteIsA = true);

    virtual ~RelationsOutputWriterOld();

    void generateOutput(vector<OntologyRelation> relations);
    void writeNonTransitiveRelation(const OntologyRelation& relation,
        fstream& fs);
    void
        writeTransitiveRelation(const OntologyRelation& relation, fstream& fs);

  private:
    string _outputFile;
    bool _alsoWriteIsA;
};
}

#endif  // SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_RELATIONSOUTPUTWRITEROLD_H_
