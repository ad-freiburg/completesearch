// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "./OntologyManager.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;

namespace ad_semsearch
{
// _____________________________________________________________________________
OntologyManager::OntologyManager(const string& ontologyDirectoryPath)
{
  _ontologyDirectoryPath = ontologyDirectoryPath;

  // 1. Read transitivity info.
  readTransitivityInfo();

  // 2. Get available relations.
  getAvailableRelations();

  // 3. Get pending corrections.
  getPendingCorrections();
}
// _____________________________________________________________________________
void OntologyManager::applyCorrections()
{
  string correctionsDoneFilePath = _ontologyDirectoryPath + "/"
      + CORRECTIONS_DONE_FILENAME;

  std::ofstream
      correctionsDone(correctionsDoneFilePath.c_str(), std::ios::app);

  for (size_t i = 0; i < _corrections.size(); ++i)
  {
    OntologyManager::Correction correction = _corrections[i];
    // Treat each type of correction differently
    switch (correction._type)
    {
    case Correction::NEW:
    {
      bool isT = (_transitiveRelations.count(correction._relation) > 0);
      OntologyRelation rel(correction._relation, isT, _ontologyDirectoryPath);
      bool ret = rel.addTuple(correction._value1, correction._value2);
      if (!ret)
      {
        cout << "Warning, correction: "
            << correction.getStringRepresentation()
            << " did not have any effect!";
      }
      break;
    }

    case Correction::DELETE:
    {
      bool isT = (_transitiveRelations.count(correction._relation) > 0);
      OntologyRelation rel(correction._relation, isT, _ontologyDirectoryPath);
      rel.deleteTuple(correction._value1, correction._value2);
      break;
    }
    case Correction::REPLACE:
    {
      bool isT = (_transitiveRelations.count(correction._relation) > 0);
      OntologyRelation rel(correction._relation, isT, _ontologyDirectoryPath);
      int count = rel.replaceEntity(correction._value1, correction._value2);
      if (count <= 0)
      {
        cout << "Warning, correction: "
            << correction.getStringRepresentation()
            << " did not have any effect!";
      }
      break;
    }
    case Correction::ALIAS:
    {
      bool isT = (_transitiveRelations.count(correction._relation) > 0);
      OntologyRelation rel(correction._relation, isT, _ontologyDirectoryPath);
      int count = rel.introduceAlias(correction._value1, correction._value2);
      if (count <= 0)
      {
        cout << "Warning, correction: "
            << correction.getStringRepresentation()
            << " did not have any effect!";
      }
      break;
    }
    case Correction::REPLACE_GLOBALLY:
    {
      vector<OntologyRelation> relations = getRelations();
      int count = 0;
      for (size_t i = 0; i < relations.size(); ++i)
      {
        count += relations[i].replaceEntity(correction._value1,
            correction._value2);
      }
      if (count <= 0)
      {
        cout << "Warning, correction: "
            << correction.getStringRepresentation()
            << " did not have any effect!";
      }
      break;
    }
    case Correction::DELETE_GLOBALLY:
    {
      // This is the most complex case. A global delete will have to take
      // transitivity rules into account.
      vector<OntologyRelation> relations = getRelations();
      for (size_t i = 0; i < relations.size(); ++i)
      {
        OntologyRelation rel = relations[i];
        if (rel.isTransitive())
        {
          // For transitive relations we need to pay special attention
          removeFromTransitiveRelation(rel, correction._value1);
        }
        else
        {
          // For non transitive relations simply
          // remove all tuples that have the deleted entity
          // source or dest.
          rel.deleteTuplesBySource(correction._value1);
          rel.deleteTuplesByDest(correction._value1);
        }
      }
      break;
    }
    case Correction::GLOBAL_ALIAS:
    {
      vector<OntologyRelation> relations = getRelations();
      int count = 0;
      for (size_t i = 0; i < relations.size(); ++i)
      {
        count += relations[i].introduceAlias(correction._value1,
            correction._value2);
      }
      if (count <= 0)
      {
        cout << "Warning, correction: "
            << correction.getStringRepresentation()
            << " did not have any effect!";
      }
      break;
    }
    default:
      cerr << "Unknown correction type: " << correction._type << "!" << endl
          << std::flush;
      break;
    }
    cout << "Done processing: " << correction.getStringRepresentation()
        << endl;
    correctionsDone << correction.getStringRepresentation() << endl << flush;
  }
  correctionsDone.close();
}
// _____________________________________________________________________________
void OntologyManager::revokeCorrections()
{
  for (size_t i = 0; i < _relations.size(); ++i)
  {
    std::remove(_relations[i].getCorrectedFilePath().c_str());
    string correctionsDoneFilePath = _ontologyDirectoryPath + "/"
        + CORRECTIONS_DONE_FILENAME;
    std::remove(correctionsDoneFilePath.c_str());
  }
}
// _____________________________________________________________________________
void OntologyManager::registerOutputWriter(OutputWriter* writer)
{
  _writers.push_back(writer);
}
// _____________________________________________________________________________
void OntologyManager::generateOutput()
{
  for (size_t i = 0; i < _writers.size(); ++i)
  {
    _writers[i]->generateOutput(_relations);
  }
}
// _____________________________________________________________________________
vector<OntologyRelation> OntologyManager::getRelations()
{
  return _relations;
}
// _____________________________________________________________________________
OntologyManager::Correction::Correction(const string& str)
{
  // Separate entries by tabs.
  string::size_type indextofTab1 = str.find('\t');
  assert(indextofTab1 != string::npos);
  string::size_type indextofTab2 = str.find('\t', indextofTab1 + 1);
  assert(indextofTab2 != string::npos);

  string target = str.substr(indextofTab1 + 1,
      indextofTab2 - (indextofTab1 + 1));
  bool global = (target == string(GLOBAL_TARGET));
  _relation = target;

  string type = str.substr(0, indextofTab1);
  if (type == "NEW")
  {
    if (global)
    {
      cerr << "Problems reading correction format! "
          << "Type NEW cannot be used globally. " << endl << "Type: " << type
          << endl << "Relation: " << target << flush;
    }
    _type = NEW;
  }
  else if (type == "DELETE")
  {
    _type = (global ? DELETE_GLOBALLY : DELETE);
  }
  else if (type == "ALIAS")
  {
    _type = (global ? GLOBAL_ALIAS : ALIAS);
  }
  else if (type == "REPLACE")
  {
    _type = (global ? REPLACE_GLOBALLY : REPLACE);
  }
  else
  {
    cerr << "Problems reading correction format! Unrecognizable type: "
        << type << endl << flush;
  }

  string::size_type indextofTab3 = str.find('\t', indextofTab2 + 1);
  if (_type == DELETE_GLOBALLY)
  {
    // This is the only case where only 1 value argument is expected.
    assert(indextofTab3 == string::npos);
    _value1 = str.substr(indextofTab2 + 1, str.size() - (indextofTab2 + 1));
  }
  else
  {
    assert(indextofTab3 != string::npos);
    _value1 = str.substr(indextofTab2 + 1, indextofTab3 - (indextofTab2 + 1));
    _value2 = str.substr(indextofTab3 + 1, str.size() - (indextofTab3 + 1));
  }
}
// _____________________________________________________________________________
string OntologyManager::Correction::getStringRepresentation()
{
  if (_type == NEW)
  {
    return "NEW\t" + _relation + "\t" + _value1 + "\t" + _value2;
  }
  if (_type == DELETE)
  {
    return "DELETE\t" + _relation + "\t" + _value1 + "\t" + _value2;
  }
  if (_type == DELETE_GLOBALLY)
  {
    return "DELETE\t" + _relation + "\t" + _value1;
  }
  if (_type == REPLACE || _type == REPLACE_GLOBALLY)
  {
    return "REPLACE\t" + _relation + "\t" + _value1 + "\t" + _value2;
  }
  if (_type == ALIAS || _type == GLOBAL_ALIAS)
  {
    return "ALIAS\t" + _relation + "\t" + _value1 + "\t" + _value2;
  }
  else
  {
    return "invalid correction";
  }
}
// _____________________________________________________________________________
void OntologyManager::readTransitivityInfo()
{
  string transitivityRulesFilePath = _ontologyDirectoryPath + "/" + string(
      TRANSITIVITY_RULES_FILENAME);

  std::ifstream transitivityRulesStream(transitivityRulesFilePath.c_str(),
      std::ios::in);
  std::string line;

  // Read line-wise.
  while (std::getline(transitivityRulesStream, line))
  {
    // Ignore comments
    if (line.substr(0, 1) != "#") _transitiveRelations.insert(line);
  }
}
// _____________________________________________________________________________
void OntologyManager::getAvailableRelations()
{
  int return_code;
  DIR* dir;
  struct dirent entry;
  struct dirent *result;
  if ((dir = opendir(_ontologyDirectoryPath.c_str())) == NULL)
  {
    cout << "Error(" << errno << ") opening " << _ontologyDirectoryPath
        << endl;
    cout << "Make sure that the ontology directory: "
        << _ontologyDirectoryPath << " is correct." << endl;
    exit(1);
  }

  for (return_code = readdir_r(dir, &entry, &result); result != NULL
      && return_code == 0; return_code = readdir_r(dir, &entry, &result))
  {
    string fileName = entry.d_name;
    if (fileName.substr(0, string(NORMAL_PREFIX).size()) == NORMAL_PREFIX)
    {
      bool isTransitive = (_transitiveRelations.count(fileName) > 0);
      _relations.push_back(
          OntologyRelation(fileName, isTransitive, _ontologyDirectoryPath));
    }
  }
  closedir(dir);
}
// _____________________________________________________________________________
void OntologyManager::getPendingCorrections()
{
  string correctionsFilePath = _ontologyDirectoryPath + "/" + string(
      CORRECTIONS_FILENAME);

  string correctionsDoneFilePath = _ontologyDirectoryPath + "/" + string(
      CORRECTIONS_DONE_FILENAME);

  std::ifstream correctionsStream(correctionsFilePath.c_str(), std::ios::in);
  std::ifstream correctionsDoneStream(correctionsDoneFilePath.c_str(),
      std::ios::in);

  string line;
  string lineDone;
  // Read line-wise.
  while (correctionsDoneStream.good() && correctionsStream.good())
  {
    std::getline(correctionsStream, line);
    // Ignore comments
    while (line.substr(0, 1) == "#")
    {
      std::getline(correctionsStream, line);
    }
    std::getline(correctionsDoneStream, lineDone);
    if (lineDone == "" && (!correctionsDoneStream.good()))
    {
      // CASE: More new corrections, add the current and further correction
      // as pending ones.
      if (line != lineDone) _corrections.push_back(Correction(line));
    }
    else if (line != lineDone)
    {
      // CASE: Mismatch between corrections and corrections done. Start fresh.
      cout << "Detected a mismatch between the corrections-done "
          << "and the current version of the corrections file!" << endl
          << "line in corrections.txt: " << line << endl
          << "line in correctionsDone.txt: " << lineDone << endl
          << "Revoking all corrections and starting fresh." << endl << flush;
      correctionsStream.close();
      revokeCorrections();
      correctionsStream.open(correctionsFilePath.c_str(), std::ios::in);
      while (std::getline(correctionsStream, line))
      {
        // Ignore comments
        if (line.substr(0, 1) != "#")
        {
          _corrections.push_back(Correction(line));
        }
      }
      break;
    }
  }
  if (std::getline(correctionsDoneStream, lineDone))
  {
    // CASE: CorrectionsDone is greater that corrections.
    // This is also a mismatch.
    cout << "Found more entries in corrections done "
        << "than in the corrections.txt! This is a mismatch!" << endl
        << "Revoking all corrections and starting fresh." << endl << flush;
    correctionsStream.close();
    revokeCorrections();
    correctionsStream.open(correctionsFilePath.c_str(), std::ios::in);
    while (std::getline(correctionsStream, line))
    {
      // Ignore comments
      if (line.substr(0, 1) != "#")
      {
        _corrections.push_back(Correction(line));
      }
    }
    correctionsStream.close();
    correctionsDoneStream.close();
    return;
  }
  correctionsDoneStream.close();
  // CASE: No mismatch.
  // Read further corrections, those are the pending ones.
  while (std::getline(correctionsStream, line))
  {
    // Ignore comments
    if (line.substr(0, 1) != "#")
    {
      _corrections.push_back(Correction(line));
    }
  }
  correctionsStream.close();
}
// _____________________________________________________________________________
void OntologyManager::removeFromTransitiveRelation(OntologyRelation rel,
    const string& toBeRemoved)
{
  // Get the content
  vector<OntologyRelation::SourceDestPair> content = rel.getContentSnapshot();
  assert(rel.isTransitive());
  // Create a hashset to quickly find out if some tuple already exists
  ad_utility::HashSet<string> existingTuples;
  // Find all sources that occur with toBeRemoves as destination (parents). +
  // Find all destinations that occur with toBeRemoves as source (children).
  vector<string> parents;
  vector<string> children;
  for (size_t i = 0; i < content.size(); ++i)
  {
    string tupleString = content[i].first + "\t" + content[i].second;
    existingTuples.insert(tupleString);
    if (content[i].second == toBeRemoved)
    {
      if (content[i].first == toBeRemoved)
      {
        cerr << "WARNING! Detected a tuple with identical source and dest: "
            << toBeRemoved << " in Relation " << rel.getName() << endl
            << "This will cause the global delete correction to malfunction."
            << endl
            << "Please check that manually and use local corrections instead!"
            << endl << flush;
        exit(1);
      }
      parents.push_back(content[i].first);
    }
    else if (content[i].first == toBeRemoved)
    {
      if (content[i].second == toBeRemoved)
      {
        cerr << "WARNING! Detected a tuple with identical source and dest: "
            << toBeRemoved << " in Relation " << rel.getName() << endl
            << "This will cause the global delete correction to malfunction."
            << endl
            << "Please check that manually and use local corrections instead!"
            << endl << flush;
        exit(1);
      }
      children.push_back(content[i].second);
    }
  }

  // Collect each combination of a parent and a child of toBeRemoved
  // add each tuple only once if and only if
  // it does not already exist as a tuple.
  vector<OntologyRelation::SourceDestPair> collection;
  for (size_t i = 0; i < parents.size(); ++i)
  {
    for (size_t j = 0; j < children.size(); ++j)
    {
      OntologyRelation::SourceDestPair pair(parents[i], children[j]);
      string tupleString = parents[i] + "\t" + children[j];
      bool alreadyInThere = (existingTuples.count(tupleString) > 0);
      if (!alreadyInThere)
      {
        collection.push_back(pair);
        existingTuples.insert(tupleString);
      }
    }
  }

  // Add all of the tuples we just collected. if they dont exist already.
  rel.addTuples(collection);

  // Remove all tuples in which toBeRemoved occurs.
  rel.deleteTuplesBySource(toBeRemoved);
  rel.deleteTuplesByDest(toBeRemoved);
}
// _____________________________________________________________________________
OntologyManager::~OntologyManager()
{
}
}
