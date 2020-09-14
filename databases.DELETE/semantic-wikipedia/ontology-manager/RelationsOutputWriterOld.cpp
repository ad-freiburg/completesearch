// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/semantic-wikipedia-utils/Conversions.h"
#include "./RelationsOutputWriterOld.h"
#include "./OntologyRelation.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::flush;

namespace ad_semsearch
{
// _____________________________________________________________________________
RelationsOutputWriterOld::RelationsOutputWriterOld(const string& outputFile,
    bool alsoWriteIsA) :
  _outputFile(outputFile), _alsoWriteIsA(alsoWriteIsA)
{
}
// _____________________________________________________________________________
RelationsOutputWriterOld::~RelationsOutputWriterOld()
{
}
// _____________________________________________________________________________
void RelationsOutputWriterOld::generateOutput(
    vector<OntologyRelation> relations)
{
  cout << "RelationOutputWriter: " << endl << flush;
  string relationsFileName = _outputFile;
  std::fstream relStream(relationsFileName.c_str(), std::ios::out);
  for (size_t i = 0; i < relations.size(); ++i)
  {
    OntologyRelation rel = relations[i];
    // Skip the isA relations,
    // types are currently processed by the SUSIOutputWriter.
    if (!_alsoWriteIsA && rel.getName() == "is-a")
    {
      continue;
    }
    cout << "Writing data for realtion: " << rel.getName() << "."
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
void RelationsOutputWriterOld::writeNonTransitiveRelation(
    const OntologyRelation& relation, fstream& fs)
{
  cout << " not transitive..." << flush;
  vector<OntologyRelation::SourceDestPair> content =
      relation.getContentSnapshot();
  for (size_t j = 0; j < content.size(); ++j)
  {
    //    string source = toLower(content[j].first);
    //    string dest = toLower(content[j].second);
    string source = content[j].first;
    string dest = (ad_utility::startsWith(relation.getDestinationType(),
        "value_") ? ad_semsearch::convertOntologyValueToIndexWord(
        content[j].second) : content[j].second);
    fs << relation.getName() << '\t' << relation.getSourceType() << '\t'
        << source << '\t' << relation.getDestinationType() << '\t' << dest
        << endl;
  }
  cout << " done" << flush;
}
// _____________________________________________________________________________
void RelationsOutputWriterOld::writeTransitiveRelation(
    const OntologyRelation& relation, fstream& fs)
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
    //    string source = toLower(transitiveContent[i].first);
    //    string dest = toLower(transitiveContent[i].second);
    const string& source = transitiveContent[i].first;
    string dest = (ad_utility::startsWith(relation.getDestinationType(),
        "value_") ? ad_semsearch::convertOntologyValueToIndexWord(
        transitiveContent[i].second) : transitiveContent[i].second);
    fs << relation.getName() << '\t' << relation.getSourceType() << '\t'
        << source << '\t' << relation.getDestinationType() << '\t' << dest
        << endl;
  }
  cout << " done" << flush;
}
}
