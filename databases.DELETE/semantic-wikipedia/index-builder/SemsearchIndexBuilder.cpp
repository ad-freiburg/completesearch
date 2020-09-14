// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <vector>
#include <iostream>

#include "./Postings.h"
#include "./IndexBuilderComparators.h"
#include "./Extractors.h"
#include "./SemsearchIndexBuilder.h"
#include "./PlainSerializer.h"
#include "./PlainOntologySerializer.h"

#include "../codebase/semantic-wikipedia-utils/Log.h"

using std::vector;
using std::string;
using std::endl;
using std::flush;

namespace ad_semsearch
{
// _____________________________________________________________________________
void SemsearchIndexBuilder::buildFulltextIndex(const string& asciiFilename,
    const string& vocabularyFilename,
    const string& fulltextBlockBoundariesFilename,
    const string& ontologyBlockBoundariesFilename,
    const string& outputFilename) const
{
  LOG(INFO) << "Semsearch Index Builder. " << endl;
  LOG(INFO) << "This will build an index consiting of postings of size: " << 2
      * sizeof(Id) + sizeof(Score) + sizeof(Position) << " Byte." << endl;
  LOG(INFO)
      << "Using an Id size of " << sizeof(Id) << " Byte, a Score size of "
      << sizeof(Score) << " Byte and a Position size of " << sizeof(Position)
      << " Byte." << endl << flush;

  doBuildFulltextIndex<DefaultExtractor, Posting, DefaultPostingComparator,
      PlainSerializer > (asciiFilename, vocabularyFilename,
      fulltextBlockBoundariesFilename, ontologyBlockBoundariesFilename,
      outputFilename);
}
// _____________________________________________________________________________
void SemsearchIndexBuilder::buildOntologyIndex(const string& asciiFilename,
    const vector<string>& relationsToBeSplitIntoBlocksByLhs,
    const string& classInstancesCountsFileName,
    const string& outputFilename) const
{
  LOG(INFO)
      << "Building Ontology Index." << endl;
  LOG(INFO)
      << "Splitting " << relationsToBeSplitIntoBlocksByLhs.size()
      << " relations into blocks for different left-hand-sides." << endl;
  doBuildOntologyIndex<OntologyTripleExtractor, RelationFact,
      PlainOntologySerializer>(asciiFilename,
      relationsToBeSplitIntoBlocksByLhs, classInstancesCountsFileName,
      outputFilename);
  LOG(INFO) << "Creation of ontology index done." << endl;
}
// _____________________________________________________________________________
SemsearchIndexBuilder::SemsearchIndexBuilder()
{
}
// _____________________________________________________________________________
SemsearchIndexBuilder::~SemsearchIndexBuilder()
{
}
}
