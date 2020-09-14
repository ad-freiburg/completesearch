// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_INDEX_BUILDER_INDEXBUILDERBASE_H_
#define SEMANTIC_WIKIPEDIA_INDEX_BUILDER_INDEXBUILDERBASE_H_

#include <gtest/gtest.h>
#include <stxxl/vector>
#include <string>
#include <vector>
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/Timer.h"
#include "../server/Identifiers.h"
#include "./Facts.h"

using std::string;
using std::vector;

using ad_utility::HashMap;
using ad_utility::Timer;

namespace ad_semsearch
{
//! Index Builder for the semantic-search index.
//! Constructs an actual index for either the ontology part
//! or an arbitrary size portion of the full-text that
//! are supposed to be searched later on.
//! Currently the required input is:
//! 1) The ontology-vocabulary, plus:
//!   2.a) The ASCII version of the ontology for building the ontology index
//!   2.b) The ASCII version of the postings plus the associated vocabulary
//!        when building an index for a full-text part.
class IndexBuilderBase
{
  public:
    IndexBuilderBase()
    : _ontologyVocabulary()
    {
    }
    virtual ~IndexBuilderBase()
    {
    }

    //! Builds an index from the specified ASCII full-text file.
    //! Default version that calls the templated versions of the worker
    //! methods with a fixed types. Note that we do not expose the
    //! choice of templates to calling clients.
    //! In order to keep interfaces simple, this method
    //! is instead supposed to be overwritten by subclasses and
    //! each subclass is supposed call the appropriate templated versions.
    virtual void
    buildFulltextIndex(const string& asciiFilename,
        const string& vocabularyFilename,
        const string& fulltextBlockBoundariesFilename,
        const string& ontologyBlockBoundariesFilename,
        const string& outputFilename) const = 0;

    //! Builds an index from the specified ASCII ontology file.
    //! Default version that calls the templated versions of the worker
    //! methods with a fixed types. Note that we do not expose the
    //! choice of templates to calling clients.
    //! In order to keep interfaces simple, this method
    //! is instead supposed to be overwritten by subclasses and
    //! each subclass is supposed call the appropriate templated versions.
    virtual void buildOntologyIndex(const string& asciiFilename,
        const vector<string>& relationsToBeSplitIntoBlocksByLhs,
        const string& classInstancesCountsFile,
        const string& outputFilename) const = 0;

    //! Sets the ontology vocabulary. An ontology vocabulary also provides
    //! IDs for the entities and necessarily has to be set whenever
    //! any kind of index is build, since entities should occur in any
    //! meaningful full-text to be indexed an in the onology anyway.
    void setOntologyVocabulary(const string& ontologyVocabularyFilename);

  protected:
    //! Builds an index from the specified ASCII full-text file.
    //! Templated version to be called by the non-templated public method.
    //! Manages the main control flow and delegated to other, templated
    //! methods for the steps.
    template<class Extractor, class Posting, class Comparator,
        class Serializer>
    void doBuildFulltextIndex(const string& asciiFilename,
        const string& vocabularyFilename,
        const string& fulltextBlockBoundariesFilename,
        const string& ontologyBlockBoundariesFilename,
        const string& outputFilename) const;

    //! Builds an index from the specified ASCII ontology file.
    //! Templated version to be called by the non-templated public method.
    //! Manages the main control flow and delegated to other, templated
    //! methods for the steps.
    template<class Extractor, class Fact, class Serializer>
    void doBuildOntologyIndex(const string& asciiFilename,
        const vector<string>& relationsToBeSplitIntoBlocksByLhs,
        const string& classInstancesCountsFile,
        const string& outputFilename) const;

    virtual Id getOntologyElementId(const string& element) const;

  private:
    //! The ontology vocabulary. Has to be set in the beginning (via
    //! the corresponding method that reads it from a file) for both,
    //! full-text and ontology index construction.
    HashMap<string, Id> _ontologyVocabulary;

    //! Parses the ASCII fulltext file into an stxxl::vector of postings.
    //! Depending on template parameters, any kind of ASCII format and
    //! posting layout are supported.
    //! Does the "multiplication", i.e. every entity posting is sent to
    //! all blocks in which a word from the same context occurs.
    template<class Extractor, class Posting, class PostingComparator>
    void extractPostingsFromAsciiFulltextFile(const string& asciiFilename,
        const Extractor& extractor, stxxl::vector<Posting>* postings,
        const PostingComparator& comp) const;


    //! Reads a vocabulary from a file into a map.
    void readFulltextVocabulary(const string& fileName,
        HashMap<string, Id>* vocabulary) const;

    //! Performs the "multiplication", i.e. writes every entity posting
    //! in all blocks of words from the same context.
    template<class Posting, class PostingComparator>
    void multiply(const vector<Posting>& wordPostings,
        const vector<Posting>& entityPostingsPtr,
        stxxl::vector<Posting>* postings,
        const PostingComparator& comp) const;

    //! Gets the meta Data for the special relation has-relations
    //! which does not directly occur in the ASCII ontology.
    //! This method simply resolves the correct Id for the constant
    //! attributes (name, target type, source type)
    //! of the has-relations relation.
    RelationMetaData resolveMetaDataForHasRelations() const;

    //! Creates the special relation has-relations.
    //! Sorts the contents of the facts vector by source/lhs.
    //! Afterwards collects all relations for each of those entities
    //! and appends them to the vector.
    template<class Fact>
    void createHasRelations(stxxl::vector<Fact>* allFactsSoFar) const;

    //! Gets the meta Data for the special Rrlation has-instances
    //! which does not directly occur in the ASCII ontology.
    //! This method simply resolves the correct Id for the constant
    //! attributes (name, target type, source type)
    //! of the has-instances relation.
    RelationMetaData resolveMetaDataForHasInstances() const;

    //! Creates the special relation has-instances.
    //! This implementation takes the information directly from a file.
    template<class Fact>
    void createHasInstances(File* classInstanceCounts,
        stxxl::vector<Fact>* allFactsSoFar) const;

    //! Timers
    mutable Timer _extractionTimer;
    mutable Timer _hasRelationsTimer;
    mutable Timer _hasInstancesTimer;
    mutable Timer _multiplyTimer;
    mutable Timer _sortTimer;
    mutable Timer _serializationTimer;

    //! Friend tests.
    FRIEND_TEST(IndexBuilderBaseTest, multiplyTest);
    FRIEND_TEST(IndexBuilderBaseTest, extractPostingsFromAsciiFulltextFileTest);
    FRIEND_TEST(IndexBuilderBaseTest, createHasRelationsTest);
};
}
#endif  // SEMANTIC_WIKIPEDIA_INDEX_BUILDER_INDEXBUILDERBASE_H_
