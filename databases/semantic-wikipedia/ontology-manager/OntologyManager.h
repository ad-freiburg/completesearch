// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_ONTOLOGYMANAGER_H_
#define SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_ONTOLOGYMANAGER_H_

#include <assert.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "./OntologyRelation.h"
#include "./OutputWriter.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"

using std::string;
using std::vector;

namespace ad_semsearch
{
const char* const TRANSITIVITY_RULES_FILENAME = "transitivity-rules.txt";
const char* const CORRECTIONS_FILENAME = "corrections.txt";
const char* const CORRECTIONS_DONE_FILENAME = "corrections-done.txt";
const char* const GLOBAL_TARGET = "__global";
const char* const NORMAL_PREFIX = "facts";
const char* const CORRECTED_PREFIX = "corrected";

class OntologyManager
{
  public:

    //! Constructor for the OntologyManger. Using the base directory
    //! of the ontology, the manager will be able to know all
    //! relations in the ontology and will instantly be able to
    //! fill members for transitivity rules and corrections.
    explicit OntologyManager(const string& ontologyDirectoryPath);
    virtual ~OntologyManager();

    //! Apply corrections from the corrections file to the relations.
    //! IMPORTANT NOTE: Does not start from a fresh base.
    //! Use a call revokeCorrections() first to apply the corrections
    //! to the base files.
    void applyCorrections();

    //! Revokes all changes made by the corrections.
    //! After calls to this method, corrected relations are identical to
    //! The original files.
    void revokeCorrections();

    //! Register an OutputWriter.
    //! When the generateOutput() method is called in this OntologyManager,
    //! all registered writers produce their corresponding output
    //! according the the state of all relations.
    void registerOutputWriter(OutputWriter* writer);

    //! Generate ontology output files. The generated output depends
    //! on registered OutputWriters.
    void generateOutput();

    //! Get all relations in the ontology. Note that relation objects
    //! are wrappers for the files (normal and corrected version)
    //! that actually contain the relation data.
    //! This is supposed to allow HUGE relations without restirctions
    //! on memory size.
    vector<OntologyRelation> getRelations();

    size_t getNumberOfWriters() const
    {
      return _writers.size();
    }

  private:

    // --- HELPER CLASSES ---
    // Class representing Corrections
    class Correction
    {
      public:
        explicit Correction(const string& str);

        enum CorrectionType
        {
          NEW,
          DELETE,
          REPLACE,
          ALIAS,
          DELETE_GLOBALLY,
          REPLACE_GLOBALLY,
          GLOBAL_ALIAS
        };

        CorrectionType _type;
        string _relation;
        string _value1;
        string _value2;

        string getStringRepresentation();
    };

    // Operator needed for hashing a string.
    // Required since a hash set is used once in the code below.
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

    // --- MEMBERS ---
    string _ontologyDirectoryPath;
    vector<OutputWriter*> _writers;
    vector<OntologyRelation> _relations;
    vector<Correction> _corrections;
    ad_utility::HashSet<string> _transitiveRelations;

    // --- PRIVATE METHODS ---
    void readTransitivityInfo();
    void getAvailableRelations();
    void getPendingCorrections();
    FRIEND_TEST(OntologyManagerTest, testRemoveFromTransitiveRelation);
    void removeFromTransitiveRelation(OntologyRelation rel,
        const string& value);
};
}
#endif  // SEMANTIC_WIKIPEDIA_ONTOLOGY_MANAGER_ONTOLOGYMANAGER_H_
