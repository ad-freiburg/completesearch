// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/semantic-wikipedia-utils/Conversions.h"
#include "./RelationsOutputWriter.h"
#include "./OntologyRelation.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::flush;

namespace ad_semsearch
{
// _____________________________________________________________________________
RelationsOutputWriter::RelationsOutputWriter(const string& outputFile,
    bool alsoWriteIsA) :
  _outputFile(outputFile), _alsoWriteIsA(alsoWriteIsA)
{
}
// _____________________________________________________________________________
RelationsOutputWriter::~RelationsOutputWriter()
{
}
// _____________________________________________________________________________
void RelationsOutputWriter::generateOutput(vector<OntologyRelation> relations)
{
  cout << "RelationOutputWriter: " << endl << flush;
  string relationsFileName = _outputFile;
  std::fstream relStream(relationsFileName.c_str(), std::ios::out);

  // We first collect a list of all entities with facts. This information
  // is necessary in order to decide whether a redirect should be followed.
  // Later we want to follow redirects only iff only one out of source and
  // destination has facts associated to it.
  for (size_t i = 0; i < relations.size(); ++i)
  {
    const OntologyRelation& rel = relations[i];
    if (rel.getName() == "is-a")
    {
      const vector<OntologyRelation::SourceDestPair> content =
          rel.getContentSnapshot();
      for (size_t j = 0; j < content.size(); ++j)
      {
        _thingsWithFacts.insert(
            ad_utility::getLowercase(mapOntologyElementName(content[j].first)));
      }
    }
  }
  if (_thingsWithFacts.size() == 0)
  {
    std::cerr << "Couldn't extract entities from is-a relation!" << std::endl
        << "Result may be flawed!" << std::endl;
  }

  // Now write each of the relations.
  for (size_t i = 0; i < relations.size(); ++i)
  {
    const OntologyRelation& rel = relations[i];
    // Skip the isA relations,
    // types are currently processed by the SUSIOutputWriter.
    if (!_alsoWriteIsA && rel.getName() == "is-a")
    {
      continue;
    }
    cout << "Writing data for relation: " << rel.getName() << "."
        << rel.getSourceType() << "." << rel.getDestinationType() << "...";
    if (rel.isTransitive())
    {
      writeTransitiveRelation(rel, relStream);
    }
    else
    {
      writeNonTransitiveRelation(rel, relStream);
    }
    cout << endl << flush;
  }
  relStream.close();
  cout << "Done writing relations." << endl << flush;
}
// _____________________________________________________________________________
void RelationsOutputWriter::writeNonTransitiveRelation(
    const OntologyRelation& relation, fstream& fs) const
{
  cout << " not transitive..." << flush;
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  for (size_t j = 0; j < content.size(); ++j)
  {
    string source = getOntologyElementStringWithPrefix(content[j].first,
        ENTITY_PREFIX);
    string dest = (ad_utility::startsWith(relation.getDestinationType(),
        "value_") ? ad_semsearch::convertOntologyValueToIndexWord(
        content[j].second) : getOntologyElementStringWithPrefix(
        content[j].second,
                ENTITY_PREFIX));
    fs
        << RELATION_PREFIX
        << relation.getName()
        << '\t'
        << getOntologyElementStringWithPrefix(relation.getSourceType(),
            ENTITY_PREFIX)
        << '\t'
        << getOntologyElementStringWithPrefix(relation.getDestinationType(),
            ENTITY_PREFIX) << '\t' << source << '\t' << dest << endl;
  }
  cout << " done" << flush;
}
// _____________________________________________________________________________
void RelationsOutputWriter::writeTransitiveRelation(
    const OntologyRelation& relation, fstream& fs) const
{
  cout << " transitive..." << flush;
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  // Go through the data and create a map cpMap child -> parents.
  typedef ad_utility::HashMap<string, vector<string> > ChildParentsMap;
  ad_utility::HashMap<string, ad_utility::HashSet<string> > parentsAdded;
  ChildParentsMap cpMap;

  for (size_t i = 0; i < content.size(); ++i)
  {
    cpMap[content[i].first].push_back(content[i].second);
  }

  // Now construct the transitive content.
  vector<OntologyRelation::SourceDestPair> transitiveContent;
  for (size_t i = 0; i < content.size(); ++i)
  {
    string current = content[i].first;
    // Reflexive
    // NEW May,17 2011: Why reflexive? Don't need it at the moment.
    // transitiveContent.push_back(Relation::SourceDestPair(current, current));
    // Get parents
    vector<string>& parents = cpMap[current];

    while (parents.size() > 0)
    {
      const string& currentP = parents[parents.size() - 1];
      parents.pop_back();
      if (parentsAdded[current].count(currentP) == 0)
      {
        parentsAdded[current].insert(currentP);
        transitiveContent.push_back(
            OntologyRelation::SourceDestPair(current, currentP));
        const vector<string>& nextParents = cpMap[currentP];
        for (size_t j = 0; j < nextParents.size(); ++j)
        {
          parents.push_back(nextParents[j]);
        }
      }
    }
  }
  for (size_t i = 0; i < transitiveContent.size(); ++i)
  {
    string source = getOntologyElementStringWithPrefix(
        transitiveContent[i].first, ENTITY_PREFIX);
    string dest = (ad_utility::startsWith(relation.getDestinationType(),
        "value_") ? ad_semsearch::convertOntologyValueToIndexWord(
        transitiveContent[i].second) : getOntologyElementStringWithPrefix(
        transitiveContent[i].second, ENTITY_PREFIX));

    fs << RELATION_PREFIX << relation.getName() << '\t'
        << getOntologyElementStringWithPrefix(relation.getSourceType(),
            ENTITY_PREFIX)
        << '\t'
        << getOntologyElementStringWithPrefix(relation.getDestinationType(),
            ENTITY_PREFIX) << '\t' << source << '\t' << dest << endl;
  }
  cout << " done" << flush;
}
}
