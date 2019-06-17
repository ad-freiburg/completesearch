// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_SEMSEARCHINDEXBUILDER_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_SEMSEARCHINDEXBUILDER_H_

#include <string>
#include <vector>
#include "./IndexBuilderBase.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
class SemsearchIndexBuilder: public ad_semsearch::IndexBuilderBase
{
  public:
    SemsearchIndexBuilder();
    virtual ~SemsearchIndexBuilder();

    //! Calls the templated version of the Baseclass
    virtual void buildFulltextIndex(const string& asciiFilename,
        const string& vocabularyFilename,
        const string& fulltextBlockBoundariesFilename,
        const string& ontologyBlockBoundariesFilename,
        const string& outputFilename) const;

    //! Calls the templated version of the Baseclass
    virtual void buildOntologyIndex(const string& asciiFilename,
        const vector<string>& relationsToBeSplitIntoBlocksByLhs,
        const string& classInstancesCountsFileName,
        const string& outputFilename) const;
};
}

#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_SEMSEARCHINDEXBUILDER_H_
