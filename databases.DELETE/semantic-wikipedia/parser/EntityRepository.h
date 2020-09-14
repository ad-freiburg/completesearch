// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_PARSER_ENTITYREPOSITORY_H_
#define SEMANTIC_WIKIPEDIA_PARSER_ENTITYREPOSITORY_H_

#include <stdio.h>
#include <string>
#include <vector>

#include "../codebase/semantic-wikipedia-utils/HashMap.h"

namespace ad_semsearch
{
/**
 * Repository that keeps track of currently interesting entities
 * Such entities are entities who's page is being processed,
 * entities that occurred in a section headline
 * and entities that occurred recently in the text.
 */
class EntityRepository
{
  public:

  EntityRepository()
  {
    _currentLevel = 0;
    _entityInSectionCount = 0;
  }

  // Sets a new page entity. This means that all section and near
  // informations are discarded.
  void setNewPageEntity(const std::string& title);

  // Sets a new section header. Discards previous section headers of
  // the same or higher levels.
  void setNewSectionHeader(const std::string& header, size_t level,
      const std::vector<std::string>& entities);

  // Checks if a word is identified as entity in the current context.
  // Returns the corresponding title or NULL if there is no match.
  // This method is called for basically every single word occurrence
  // in the whole data set and should be implemented efficiently.
  // The word has to be in lower case!
  std::string isEntity(const std::string& word);

  // Checks if a word is identified as document entity in the current context.
  // The document entity is the entity described by the current wikipedia
  // page.
  bool isDocumentEntity(const std::string& word);

  // Informs the repository that an entity has been discovered
  // at the current position. The repository takes care of
  // associating the occurrence with the correct scope.
  void addDiscoveredEntity(const std::string& title);

  // Getter for section headers member.
  std::vector<std::string> getSectionHeaders() const
  {
    return _sectionHeaders;
  }

  // Getter for section entities member.
  std::vector<std::string> getSectionEntities() const
  {
    return _sectionEntities;
  }

  private:

  // Choice of data structures. Lookup has to be efficient.
  // A has map from words to titles is used. Problem:
  // double occurrences. Possibly easy and efficient solution:
  // Ignore words that occur in two current entities and minimize
  // the amount by removing meaningless words anyway. The maps mixed
  // Recent and header/section entities. While it is cleared for
  // each new section, the up-to-date section and page entities
  // are simply added again. This should be more useful than looking up
  // different maps / data structures for each word.
  ad_utility::HashMap<std::string, std::string> _nearEntities;

  // Contains the section headers for each relevant section. If an entity
  // could be identified, an entry in sectionEntities is produced. The index
  // is used to indicate the current section  level. Level 0 corresponds to
  // the page title.
  std::vector<std::string> _sectionHeaders;

  // Contains the section entities for each relevant section. May contain
  // empty string if no entity could have been defined. The length of the
  // vector depends on the current section level in the parsed document.
  std::vector<std::string> _sectionEntities;

  // Clears the map if a new section started and rebuilds it with the
  // information that is supposed to persist through this section change.
  void clearAndRebuildMap();

  // Checks if some entity title and section header do match
  bool titleHeadlineMatch(const std::string& title,
      const std::string& headline);

  size_t _currentLevel;
  size_t _entityInSectionCount;
};
}

#endif  // SEMANTIC_WIKIPEDIA_PARSER_ENTITYREPOSITORY_H_
