// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <stdio.h>
#include <string>
#include <vector>

#include "./EntityRepository.h"

namespace ad_semsearch
{
// ____________________________________________________________________________
void EntityRepository::setNewPageEntity(const std::string& title)
{
  std::string titleLowerCase = std::string(title);
  for (std::string::iterator i = titleLowerCase.begin(); i
      != titleLowerCase.end(); ++i)
  {
    *i = tolower(*i);
  }

  std::vector<std::string> entity;
  entity.push_back(title);
  setNewSectionHeader(title, 0, entity);
}
// ____________________________________________________________________________
void EntityRepository::setNewSectionHeader(const std::string& header,
    size_t level, const std::vector<std::string>& associatedEntities)
{
  if (level > 8)
  {
    // std::cout << "Strange level: " << level << std::endl << std::flush;
    // Whenever this branch is taken, we can be use there has been
    // a problem with parsing.
    level = 5;
  }
  _entityInSectionCount = 0;
  _sectionHeaders.resize(level + 1, "");
  _sectionHeaders[level] = header;
  _sectionEntities.resize(level + 1, "");
  if (associatedEntities.size() > 0)
  {
    _sectionEntities[level] = associatedEntities[0];
  }
  if (_currentLevel >= level)
  {
    clearAndRebuildMap();
  }
}
// ____________________________________________________________________________
std::string EntityRepository::isEntity(const std::string& word)
{
  if (_nearEntities.count(word) == 0)
  {
    return "";
  }
  else
  {
    return _nearEntities[word];
  }
}
// ____________________________________________________________________________
bool EntityRepository::isDocumentEntity(const std::string& word)
{
  return (_sectionEntities.size() > 0 && _sectionEntities[0] == word);
}
// ____________________________________________________________________________
void EntityRepository::addDiscoveredEntity(const std::string& title)
{
  if (title == "")
  {
    // Add nothing
    return;
  }

  // TODO(buchholb): Externalize this constants as arguments
  std::string::size_type minTitleWordLength = 4;
  size_t maxEntitiesInSectionForHeader = 2;

  _entityInSectionCount++;

  // Add the discovered information to the map.
  // Split the words.
  // Note that while some chars are ignore for normal words and
  // foo-bar becomes foobar during the normal parsing, titles
  // should be split more strict. To enable recognition of only
  // parts of the title.
  std::string delimiters = " '-/()_";
  std::string::size_type lastPos = title.find_first_not_of(delimiters, 0);
  std::string::size_type pos = title.find_first_of(delimiters, lastPos);

  // Go through the title
  while (pos != title.npos || lastPos != title.npos)
  {
    // Found a word. Make it lower case
    std::string word = title.substr(lastPos, pos - lastPos);
    bool allUpper = true;
    for (std::string::iterator i = word.begin(); i != word.end(); ++i)
    {
      allUpper = allUpper && isupper(*i);
      *i = tolower(*i);
    }
    // Write word (that is longer than some constant) -> title into map.
    if (word.length() >= minTitleWordLength || (word.length() > 1 && allUpper))
    {
      // Only set it if it isn't already set
      if (_nearEntities.count(word) == 0)
      {
        _nearEntities[word] = title;
      }
    }
    // Skip delimiters
    lastPos = title.find_first_not_of(delimiters, pos);
    // Find next
    pos = title.find_first_of(delimiters, lastPos);
  }

  // Take Care of updating the current section entities.
  if (_currentLevel > 0)
  {
    // Get the most recent headline
    assert(_currentLevel < _sectionHeaders.size());
    std::string head = _sectionHeaders[_currentLevel];

    if (_entityInSectionCount <= maxEntitiesInSectionForHeader
        && _sectionEntities[_currentLevel] == "" && titleHeadlineMatch(title,
        head))
    {
      // Got a match. Set this
      _sectionEntities[_currentLevel] = title;
    }
  }
}
// ____________________________________________________________________________
void EntityRepository::clearAndRebuildMap()
{
  _nearEntities.clear();
  for (size_t i = 0; i < _sectionEntities.size(); ++i)
  {
    addDiscoveredEntity(_sectionEntities[i]);
  }
}
//
bool EntityRepository::titleHeadlineMatch(const std::string& title,
    const std::string& headline)
{
  return title == headline;
}
}
