// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_ONTOLOGYRELATION_H_
#define SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_ONTOLOGYRELATION_H_

#include <string>
#include <utility>
#include <vector>

using std::string;
using std::vector;

namespace ad_semsearch
{
//! Class representing relations. Instances are wrappers for the
//! files that actually carry the data.
//! A relation always has two file representations. An original
//! representation and a corrected, currently valid version.
//! If there is nofile for teh corrected version,
//! a copy is made.
class OntologyRelation
{
  public:

    // --- CONSTRUCTORS ---
    //! Constructor that constructs a relation wrapper
    //! object from the string representation of the object.
    //! If there is no suitable base file, an assertion error
    //! should be thrown. If there is no file with corrected
    //! content, the base file is copied and this copy is used.
    OntologyRelation(const string& relation, bool isTransitive = false,
        const string& pathToOntology = "ontology");

    // --- METHODS ---
    typedef std::pair<string, string> SourceDestPair;

    //! Returns the content of the relation as vector
    //! of source-dest pairs. Source and dest are
    //! strings that represent entities in the ontology.
    //! This is a snapshot and hence does not change
    //! when the relation is changed.
    vector<SourceDestPair> getContentSnapshot() const;

    //! Adds a tuple with the specified values to the relation.
    //! Only adds the tuple if there isn't an identical
    //! tuple already.
    //! Returns true if and only if something has been added.
    bool addTuple(const string& srcValue, const string& destValue);

    //! Adds tuples with the specified values to the relation.
    //! Always adds all tuples. Caller has to ensure that
    //! there isn't an identical tuple already.
    void addTuples(vector<SourceDestPair> tuples);

    //! Deletes a specific tuple from the relation. NOTE: tuples
    //! have to be unique. Identical tuples make a broken
    //! ontology.
    void deleteTuple(const string& source, const string& dest);

    //! Deletes tuples with the given source from the relation.
    //! Returns the number of tuples deleted.
    int deleteTuplesBySource(const string& source);

    //! Deletes tuples with the given destination from the relation.
    //! Returns the number of tuples deleted.
    int deleteTuplesByDest(const string& dest);

    //! Replaces all occurrences of an oldValue by another value.
    //! Returns the number of changes made.
    int replaceEntity(const string& oldValue, const string& newValue);

    //! Introduces an additional occurrences of an entity by alias by alias.
    //! for each existing occurrence.
    //! Returns the number of changes made.
    int introduceAlias(const string& entity, const string& newAlias);

    //! Returns the file system path to the base data for this relation.
    string getBaseFilePath() const;

    string asString() const;

    //! Returns the file system path to the corrected data for this relation.
    string getCorrectedFilePath() const;

    // --- GETTERS ---
    //! Getter for member _name
    string getName() const
    {
      return _name;
    }
    //! Getter for member _sourceType
    string getSourceType() const
    {
      return _sourceType;
    }
    //! Getter for member _destinationType
    string getDestinationType() const
    {
      return _destinationType;
    }
    //! Getter for member _isTransitive
    bool isTransitive() const
    {
      return _isTransitive;
    }

  private:
    // --- MEMBER ATTRIBUTES ---
    //! Member holding the name of the relations.
    string _name;
    //! Member holding the source type of the relations.
    string _sourceType;
    //! Member holding the destination type of the relations.
    string _destinationType;
    //! Member indication whether or not the relation is transitive.
    bool _isTransitive;
    string _pathToOntology;

    // --- PRIVATE METHODS ---
    std::ifstream* openCurrentDataFileForReading() const;
    void persitsTempChanges();

    string getTempFilePath() const;
};
}
#endif  // SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_ONTOLOGYRELATION_H_
