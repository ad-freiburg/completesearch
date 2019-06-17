// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "./OntologyRelation.h"
#include "./OntologyManager.h"

using std::cerr;
using std::endl;
using std::flush;
using std::vector;

namespace ad_semsearch
{
// _____________________________________________________________________________
OntologyRelation::OntologyRelation(const string& relation, bool isTransitive,
    const string& pathToOntology)
{
  // Parse the string representation and separate infos by the dots.
  string::size_type indexOfDot1 = relation.find('.');
  assert(indexOfDot1 != string::npos);
  string::size_type indexOfDot2 = relation.find('.', indexOfDot1 + 1);
  assert(indexOfDot2 != string::npos);
  string::size_type indexOfDot3 = relation.find('.', indexOfDot2 + 1);
  assert(indexOfDot3 != string::npos);
  string::size_type indexOfDot4 = relation.find('.', indexOfDot3 + 1);
  assert(indexOfDot4 != string::npos);

  // Set members
  _name = relation.substr(indexOfDot1 + 1, indexOfDot2 - (indexOfDot1 + 1));
  _sourceType = relation.substr(indexOfDot2 + 1,
      indexOfDot3 - (indexOfDot2 + 1));
  _destinationType = relation.substr(indexOfDot3 + 1,
      indexOfDot4 - (indexOfDot3 + 1));
  _pathToOntology = pathToOntology;
  _isTransitive = isTransitive;
}
// _____________________________________________________________________________
vector<OntologyRelation::SourceDestPair>
OntologyRelation::getContentSnapshot() const
{
  std::ifstream* currentData = openCurrentDataFileForReading();
  string line;
  vector<SourceDestPair> retVal;
  while (std::getline(*currentData, line))
  {
    // Split line at tab
    size_t indexOfTab = line.find('\t');
    assert(indexOfTab > 0);
    assert(indexOfTab != string::npos);
    retVal.push_back(
        OntologyRelation::SourceDestPair(line.substr(0, indexOfTab),
            line.substr(indexOfTab + 1, line.size() - (indexOfTab + 1))));
  }
  currentData->close();
  delete currentData;
  return retVal;
}
// _____________________________________________________________________________
bool OntologyRelation::addTuple(const string& srcValue, const string& destValue)
{
  string entryToAdd = srcValue + "\t" + destValue;
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;
  bool alreadyInThere = false;
  while (std::getline(*current, line))
  {
    alreadyInThere = alreadyInThere || (line == entryToAdd);
    tmp << line << endl;
  }
  current->close();
  delete current;
  if (!alreadyInThere)
  {
    tmp << entryToAdd << endl << flush;
  }
  tmp.close();
  persitsTempChanges();
  return !alreadyInThere;
}
// _____________________________________________________________________________
void OntologyRelation::addTuples(vector<SourceDestPair> tuples)
{
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;
  while (std::getline(*current, line))
  {
    tmp << line << endl;
  }
  current->close();
  delete current;
  for (size_t i = 0; i < tuples.size(); ++i)
  {
    tmp << tuples[i].first << "\t" << tuples[i].second << endl;
  }
  tmp.close();
  persitsTempChanges();
}
// _____________________________________________________________________________
void OntologyRelation::deleteTuple(const string& source, const string& dest)
{
  string entryToDelete = source + "\t" + dest;
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;

  while (std::getline(*current, line))
  {
    if (!(line == entryToDelete))
    {
      tmp << line << endl;
    }
  }
  current->close();
  delete current;
  tmp.close();
  persitsTempChanges();
}
// _____________________________________________________________________________
int OntologyRelation::deleteTuplesBySource(const string& sourceToDel)
{
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;
  int nofDeletes = 0;
  while (std::getline(*current, line))
  {
    // Get the source from line
    size_t indexOfTab = line.find('\t');
    assert(indexOfTab > 0);
    string lineSrc = line.substr(0, indexOfTab);
    if (lineSrc != sourceToDel)
    {
      tmp << line << endl;
    }
    else
    {
      nofDeletes++;
    }
  }
  current->close();
  delete current;
  tmp.close();
  persitsTempChanges();
  return nofDeletes;
}
// _____________________________________________________________________________
int OntologyRelation::deleteTuplesByDest(const string& destToDel)
{
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;
  int nofDeletes = 0;
  while (std::getline(*current, line))
  {
    // Get the dest from line
    size_t indexOfTab = line.find('\t');
    assert(indexOfTab > 0);
    string lineDest = line.substr(indexOfTab + 1,
        line.size() - (indexOfTab + 1));
    if (lineDest != destToDel)
    {
      tmp << line << endl;
    }
    else
    {
      nofDeletes++;
    }
  }
  current->close();
  delete current;
  tmp.close();
  persitsTempChanges();
  return nofDeletes;
}
// _____________________________________________________________________________
int OntologyRelation::replaceEntity(const string& oldValue,
    const string& newValue)
{
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;
  int nofRepelaces = 0;
  while (std::getline(*current, line))
  {
    // Get the source and dest from line
    size_t indexOfTab = line.find('\t');
    assert(indexOfTab > 0);
    string source = line.substr(0, indexOfTab);
    string dest = line.substr(indexOfTab + 1, line.size() - (indexOfTab + 1));

    if (source == oldValue)
    {
      source = newValue;
      nofRepelaces++;
    }
    if (dest == oldValue)
    {
      dest = newValue;
      nofRepelaces++;
    }
    tmp << source << "\t" << dest << endl;
  }
  current->close();
  delete current;
  tmp.close();
  persitsTempChanges();
  return nofRepelaces;
}
// _____________________________________________________________________________
int OntologyRelation::introduceAlias(const string& entity,
    const string& newAlias)
{
  std::ifstream* current;
  current = openCurrentDataFileForReading();
  assert(current->is_open());
  std::ofstream tmp(getTempFilePath().c_str(), std::ios::trunc);
  string line;
  int nofAdded = 0;
  while (std::getline(*current, line))
  {
    // Get the source and dest from line
    size_t indexOfTab = line.find('\t');
    assert(indexOfTab > 0);
    string source = line.substr(0, indexOfTab);
    string dest = line.substr(indexOfTab + 1, line.size() - (indexOfTab + 1));
    tmp << source << "\t" << dest << endl;
    if (source == entity)
    {
      source = newAlias;
      tmp << source << "\t" << dest << endl;
      nofAdded++;
    }
    if (dest == entity)
    {
      dest = newAlias;
      tmp << source << "\t" << dest << endl;
      nofAdded++;
    }
  }
  current->close();
  delete current;
  tmp.close();
  persitsTempChanges();
  return nofAdded;
}
// _____________________________________________________________________________
string OntologyRelation::getBaseFilePath() const
{
  return _pathToOntology + "/" + NORMAL_PREFIX + "." + _name + "."
      + _sourceType + "." + _destinationType + ".txt";
}
// _____________________________________________________________________________
string OntologyRelation::asString() const
{
  return _name + "." + _sourceType + "." + _destinationType;
}
// _____________________________________________________________________________
string OntologyRelation::getCorrectedFilePath() const
{
  return _pathToOntology + "/" + CORRECTED_PREFIX + "." + _name + "."
      + _sourceType + "." + _destinationType + ".txt";
}
// _____________________________________________________________________________
string OntologyRelation::getTempFilePath() const
{
  return _pathToOntology + "/" + "__tmp" + "." + _name + "." + _sourceType
      + "." + _destinationType + ".txt";
}
// _____________________________________________________________________________
std::ifstream* OntologyRelation::openCurrentDataFileForReading() const
{
  std::ifstream* corStream;
  corStream = new std::ifstream(getCorrectedFilePath().c_str());
  if (corStream->is_open())
  {
    return corStream;
  }
  delete corStream;
  std::ifstream* baseStream;
  baseStream = new std::ifstream(getBaseFilePath().c_str());
  return baseStream;
}
// _____________________________________________________________________________
void OntologyRelation::persitsTempChanges()
{
  std::ifstream tmpStream(getTempFilePath().c_str(), std::ios::in);
  std::ofstream corStream(getCorrectedFilePath().c_str(), std::ios::trunc);
  assert(tmpStream.is_open());
  assert(corStream.is_open());
  string line;
  while (std::getline(tmpStream, line))
  {
    corStream << line << endl;
  }
  corStream << flush;
  tmpStream.close();
  corStream.close();
}
}
