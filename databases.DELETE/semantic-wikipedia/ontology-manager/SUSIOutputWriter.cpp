// Copyright 2011, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Björn Buchhold <buchholb>

#include <stdio.h>
#include <assert.h>
#include <boost/algorithm/string/predicate.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "../codebase/semantic-wikipedia-utils/Comparators.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/File.h"
#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "./OntologyRelation.h"
#include "./SUSIOutputWriter.h"

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

namespace ad_semsearch
{
const char* WORDNET_ENTITY = "wordnet_entity_100001740";

// _____________________________________________________________________________
SUSIOutputWriter::SUSIOutputWriter(const string& outputDir,
    bool pathWithSpecialCharsMode) :
  _outputDir(outputDir), _pathWithSpecialCharsMode(pathWithSpecialCharsMode)
{
}
// _____________________________________________________________________________
void SUSIOutputWriter::generateOutput(vector<OntologyRelation> relations)
{
  cout << "SUSIOutputWriter: " << endl << flush;
  string pathsFileName = _outputDir + "/semantic-wikipedia.yago-paths";
  if (_pathWithSpecialCharsMode) pathsFileName += "-with-underscores";
  string factsFileName = _outputDir + "/semantic-wikipedia.yago-facts";
  ad_utility::File pathsFile(pathsFileName.c_str(), "w");

  // Find the single relation that is interesting for SUSI.
  OntologyRelation* rel = 0;
  for (size_t i = 0; i < relations.size(); ++i)
  {
    if (relations[i].asString() == string(IS_A_RELATION_FROM_YAGO))
    {
      rel = &relations[i];
    }
  }
  assert(rel != 0);
  assert(rel->getName() == "is-a");
  vector<OntologyRelation::SourceDestPair> content = rel->getContentSnapshot();

  // Go through the data and create a map cpMap child -> parents.
  typedef ad_utility::HashMap<string, vector<string> > ChildParentsMap;
  ChildParentsMap cpMap;

  for (size_t i = 0; i < content.size(); ++i)
  {
    cpMap[content[i].first].push_back(content[i].second);
  }

  cout << "Building the paths now ..." << endl << flush;
  // Go through the content again and only look at keys that are classes.
  // Complete the path until entity is reached and
  // put it in another map pathsMap: class -> path.
  typedef ad_utility::HashMap<string, vector<Path> > PathsMap;
  PathsMap pathsMap;

  Path entityPath;
  entityPath.push_back(stripWordnetAndId(WORDNET_ENTITY));
  pathsMap[WORDNET_ENTITY].push_back(entityPath);
  for (size_t i = 0; i < content.size(); ++i)
  {
    string current = content[i].first;
    if (current.substr(0, 8) == "wordnet_")
    {
      // vector<string> parents = cpMap[current];
      vector<string> lastElements;
      lastElements.push_back(current);

      vector<Path> paths;
      vector<string> path;
      path.push_back(stripWordnetAndId(current));
      paths.push_back(path);

      bool didSomething = true;
      while (didSomething)
      {
        didSomething = false;
        // Go through all open paths and for every one where the last
        // element isn't entity, continue the path
        // and add more paths whenever necessary.
        for (size_t j = 0; j < lastElements.size(); ++j)
        {
          if (lastElements[j] != string(WORDNET_ENTITY))
          {
            const vector<string>& nextSteps = cpMap[lastElements[j]];
            if (nextSteps.size() == 0)
            {
              // CASE NO PARENT FOUND.
              // Output a warning and commence to entity.
              cerr << "No parent found for: " << lastElements[j] << endl
                  << "continuing with entity. Consider fixing the ontology!"
                  << endl << flush;
              paths[j].push_back("entity");
              lastElements[j] = string(WORDNET_ENTITY);
            }
            else if (nextSteps.size() == 1)
            {
              // CASE EXACTLY ONE PARENT
              // Just continue the path.
              paths[j].push_back(stripWordnetAndId(nextSteps[0]));
              lastElements[j] = nextSteps[0];
            }
            else
            {
              // CASE MULTIPLE PARENTS.
              // Continue one path and add others.
              const Path currentPath(paths[j]);
              paths[j].push_back(stripWordnetAndId(nextSteps[0]));
              lastElements[j] = nextSteps[0];
              for (size_t k = 1; k < nextSteps.size(); ++k)
              {
                Path thisPath(currentPath);
                thisPath.push_back(stripWordnetAndId(nextSteps[k]));
                paths.push_back(thisPath);
                lastElements.push_back(nextSteps[k]);
              }
            }
            didSomething = true;
          }
        }
      }
      pathsMap[current] = paths;
    }
  }
  // Do post-processing for optimizations:
  cout << "Post processing the paths... " << endl << flush;
  for (PathsMap::const_iterator it = pathsMap.begin();
      it != pathsMap.end(); ++it)
  {
    string key = it->first;
    vector<Path> paths = it->second;
    // Sort by: short paths first, otherwise lexicographically
    SUSIOutputWriter::PathCompare comp;
    std::sort(paths.begin(), paths.end(), comp);
    // Only keep paths that add new information.
    vector<Path> newPaths;
    ad_utility::HashSet<string> alreadySeen;
    for (size_t i = 0; i < paths.size(); ++i)
    {
      bool somethingNew = false;
      Path path = paths[i];
      for (size_t j = 0; j < path.size(); ++j)
      {
        if (alreadySeen.count(path[j]) == 0)
        {
          alreadySeen.insert(path[j]);
          somethingNew = true;
        }
      }
      if (somethingNew)
      {
        newPaths.push_back(path);
      }
    }
    pathsMap[key] = newPaths;
  }
  // Write the paths to the file.
  cout << "Construction of paths complete, sorting and "
      << "writing them to the file now... " << flush;

  typedef vector<PathPair> PathsList;
  PathsList pathsAsList;
  for (PathsMap::const_iterator it = pathsMap.begin();
      it != pathsMap.end(); ++it)
  {
    for (size_t i = 0; i < it->second.size(); ++i)
    {
      PathPair pair(it->first, it->second[i]);
      pathsAsList.push_back(pair);
    }
  }
  PathPairCompare ppComp;
  std::sort(pathsAsList.begin(), pathsAsList.end(), ppComp);
  for (PathsList::iterator it = pathsAsList.begin();
      it != pathsAsList.end(); ++it)
  {
    // fprintf(pathsFile, "%s\t%s\t%s\n", stripWordnetAndId(it->first).c_str(),
    // getIdFromWN(it->first).c_str(), getPathString(it->second).c_str());
    string line = stripWordnetAndIdAccordingToSettings(it->first);
    line += "\t";
    line += (_ontologyCategoryNamesMap.size() > 0 ? "0\t" : getIdFromWN(
        it->first) + "\t");
    line += getPathString(it->second);
    pathsFile.writeLine(line);
  }
  cout << " done" << endl << flush;
  //  fclose(pathsFile);

  if (_pathWithSpecialCharsMode) return;
  // Create the yago-facts.
  ad_utility::File factsFile(factsFileName.c_str(), "w");
  cout << "Now writing the yago-facts file... " << endl << flush;
  // Go through the content again and this time write for
  // each child all of its parent's paths.
  PathsMap facts;
  for (size_t i = 0; i < content.size(); ++i)
  {
    string key;
    if (content[i].first.substr(0, 8) == "wordnet_")
    {
      key = stripWordnetAndIdKeepSpecialChars(content[i].first);
    }
    else
    {
      //      for (size_t j = 0; j < content[i].first.size(); ++j)
      //      {
      //        key.push_back(tolower(content[i].first[j]));
      //      }
      key = content[i].first;
    }
    string parent = content[i].second;
    vector<Path> paths = pathsMap[parent];
    for (size_t j = 0; j < paths.size(); ++j)
    {
      facts[key].push_back(paths[j]);
    }
  }
  cout << "Post-processing the facts... " << flush;
  // Do the post-processing.
  PathsList output;
  // Go through  the facts, process entity by entity.
  for (PathsMap::const_iterator it = facts.begin(); it != facts.end(); ++it)
  {
    // Look at all paths, sort them according to their length, sort paths first.
    string entity = it->first;
    vector<Path> paths = it->second;
    ad_utility::VectorSizeIsGreaterComparator comp;
    std::sort(paths.begin(), paths.end(), comp);
    vector<string> pathsSeen;
    for (size_t i = 0; i < paths.size(); ++i)
    {
      bool isPrefix = false;
      string pathString = getPathString(paths[i]);
      // Only write paths that are no prefix of another one.
      for (size_t j = 0; j < pathsSeen.size(); ++j)
      {
        if (pathsSeen[j].size() > pathString.size() && pathsSeen[j].substr(0,
            pathString.size()) == pathString)
        {
          isPrefix = true;
        }
      }
      if (!isPrefix)
      {
        pathsSeen.push_back(pathString);
        PathPair pair(entity, paths[i]);
        output.push_back(pair);
      }
    }
  }
  // Sort the output by key
  //  cout << "sorting... " << flush;
  //  FactPairCompare fpComp;
  //  std::sort(output.begin(), output.end(), fpComp);
  // Write the information into the semantic-wikipedia.yago-facts
  cout << "file IO... " << flush;
  for (size_t i = 0; i < output.size(); ++i)
  {
    factsFile.writeLine(
        normalizeEntity(output[i].first) + "\t" + getPathString(
            output[i].second));
  }
  cout << "done." << endl << flush;
}
// _____________________________________________________________________________
string SUSIOutputWriter::stripWordnetAndId(const string& classString)
{
  // Find the first and the last underscore
  size_t indexOfFirstUnderscore = classString.find('_');
  size_t indexOfLastUnderscore = classString.rfind('_');
  assert(indexOfFirstUnderscore != string::npos);
  assert(indexOfLastUnderscore != string::npos);
  assert(indexOfFirstUnderscore != indexOfLastUnderscore);

  string stripped;
  // Go through the string between the first and last underscore and
  // append everything but underscores to the returned string
  for (size_t i = indexOfFirstUnderscore + 1; i < indexOfLastUnderscore; ++i)
  {
    if ((classString[i] != '_') && (classString[i] != '\'') && (classString[i]
        != '-'))
    {
      stripped.push_back(tolower(classString[i]));
    }
  }
  return stripped;
}
// _____________________________________________________________________________
string SUSIOutputWriter::stripWordnetAndIdKeepSpecialChars(
    const string& classString)
{
  // Find the first and the last underscore
  size_t indexOfFirstUnderscore = classString.find('_');
  size_t indexOfLastUnderscore = classString.rfind('_');
  assert(indexOfFirstUnderscore != string::npos);
  assert(indexOfLastUnderscore != string::npos);
  assert(indexOfFirstUnderscore != indexOfLastUnderscore);

  //  string stripped;
  //  // Go through the string between the first and last underscore and
  //  // append everything but underscores to the returned string
  //  for (size_t i = indexOfFirstUnderscore + 1;
  //      i < indexOfLastUnderscore; ++i)
  //  {
  //    stripped.push_back(tolower(classString[i]));
  //  }
  //  return stripped;
  return classString.substr(indexOfFirstUnderscore + 1,
      indexOfLastUnderscore - (indexOfFirstUnderscore + 1));
}
// _____________________________________________________________________________
string SUSIOutputWriter::stripWordnetAndIdLeaveUntouched(
    const string& classString)
{
  // Find the first and the last underscore
  size_t indexOfFirstUnderscore = classString.find('_');
  size_t indexOfLastUnderscore = classString.rfind('_');
  assert(indexOfFirstUnderscore != string::npos);
  assert(indexOfLastUnderscore != string::npos);
  assert(indexOfFirstUnderscore != indexOfLastUnderscore);

  return classString.substr(indexOfFirstUnderscore + 1,
      indexOfLastUnderscore - (indexOfFirstUnderscore + 1));
}
// _____________________________________________________________________________
string SUSIOutputWriter::getIdFromWN(const string& classString) const
{
  // Find the first and the last underscore
  size_t indexOfFirstUnderscore = classString.find('_');
  size_t indexOfLastUnderscore = classString.rfind('_');
  assert(indexOfFirstUnderscore != string::npos);
  assert(indexOfLastUnderscore != string::npos);
  assert(indexOfFirstUnderscore != indexOfLastUnderscore);

  return classString.substr(indexOfLastUnderscore + 1,
      classString.size() - (indexOfLastUnderscore + 1));
}
// _____________________________________________________________________________
string SUSIOutputWriter::getPathString(const Path& path) const
{
  string pathString;
  if (path.size() > 0)
  {
    for (size_t i = (path.size() - 1); i > 0; --i)
    {
      pathString.append(path[i]);
      pathString.append(":");
    }
    pathString.append(path[0]);
  }
  return pathString;
}
// _____________________________________________________________________________
bool SUSIOutputWriter::PathCompare::operator()
    (const Path& x, const Path& y) const
{
  size_t sizeX = x.size();
  size_t sizeY = y.size();
  if (sizeX == sizeY)
  {
    assert(sizeX > 0);
    string stringX;
    string stringY;
    for (size_t i = (sizeX - 1); i > 0; --i)
    {
      stringX.append(x[i]);
      stringX.append(":");
      stringY.append(y[i]);
      stringY.append(":");
    }
    stringX.append(x[0]);
    stringY.append(y[0]);
    return (stringX.compare(stringY) < 0);
  }
  else
  {
    return (sizeX > sizeY);
  }
}
// _____________________________________________________________________________
SUSIOutputWriter::~SUSIOutputWriter()
{
}
// _____________________________________________________________________________
bool SUSIOutputWriter::PathPairCompare::operator()(const PathPair& x,
    const PathPair& y) const
{
  string name1 = stripWordnetAndId(x.first);
  string name2 = stripWordnetAndId(y.first);
  string number1 = x.first.substr(x.first.rfind('_') + 1);
  string number2 = y.first.substr(y.first.rfind('_') + 1);
  if (name1 == name2)
  {
    return number1 < number2;
  }
  else
  {
    return name1 < name2;
  }
}
// _____________________________________________________________________________
bool SUSIOutputWriter::FactPairCompare::operator()(const PathPair& x,
    const PathPair& y) const
{
  return (x.first.compare(y.first) < 0);
}
// _____________________________________________________________________________
void SUSIOutputWriter::setOntologyCategoryNamesMap(const string& fileName)
{
  ad_utility::File mapFile(fileName.c_str(), "r");
  char buf[BUFFER_SIZE_WORDNET_CATEGORIES];
  string line;
  while (mapFile.readLine(&line, buf, BUFFER_SIZE_WORDNET_CATEGORIES))
  {
    size_t indexOfTab = line.find('\t');
    _ontologyCategoryNamesMap[line.substr(0, indexOfTab)] = line.substr(
        indexOfTab + 1, line.size() - (indexOfTab + 1));
  }
}
}
