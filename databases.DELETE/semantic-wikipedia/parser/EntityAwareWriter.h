// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#ifndef SEMANTIC_WIKIPEDIA_PARSER_ENTITYAWAREWRITER_H_
#define SEMANTIC_WIKIPEDIA_PARSER_ENTITYAWAREWRITER_H_

#include <stdio.h>
#include <stdint.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "../codebase/semantic-wikipedia-utils/Globals.h"
#include "../codebase/semantic-wikipedia-utils/HashMap.h"
#include "../codebase/semantic-wikipedia-utils/HashSet.h"
#include "../codebase/semantic-wikipedia-utils/Conversions.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "./EntityRepository.h"

using ad_utility::HashMap;
using ad_utility::HashSet;

namespace ad_semsearch
{
class EntityAwareWriter
{
  public:

    EntityAwareWriter(std::string::size_type minWordLength,
        size_t maxHeaderLevel);
    virtual ~EntityAwareWriter();

    void setYagoRelationsFileName(const std::string& yagoRelationsFileName);

    // Writes a word. Takes care of proper formatting
    // and entity recognition.
    // Note: The minWordLength comparison is not done here in order to
    // save comparisons. Make sure the caller takes care if this.
    void writeWord(const std::string& word);

    // Explicitly writes an entity. One can specify if the actual words
    // should be written to the output as well and if this
    // entity is supposed to be added EntityRepository.
    void writeEntity(const std::string& words, bool alsoWriteWords,
        bool addToRepos);

    // Explicitly writes an entity with enabled hiding
    // Writes the words that are displayed on the wikipedia page
    // prefix by the special string for the entity occurrence.
    void writeEntityWithHiding(const std::string& text);

    // Explicitly writes an entity with renaming
    // Writes the words that are displayed on the wikipedia page
    // prefix by the special string for the entity occurrence.
    void writeEntityWithRenaming(const std::string& title,
        const std::string& renaming);

    // Write and handle (w.r.t. entity recognition) a section headline.
    void writeSectionHead(const std::string& words, int sectionLevel,
        const std::vector<std::string>& associatedEntities);

    // Writes a category. Currently supported by the parser and this writer
    // but not used for queries.
    void writeCategory(const std::string& categoryString);

    // Makes the writer finish it's output and flush the buffer / stream.
    void finish(std::string* wordsFileOutput, std::string* docsFileOutput);

    // Set the current document. This indicates a start of a
    // new document and triggers writing of the title.
    void setDocument(const std::string& title);

    void setMarkEntityEnds(bool markEntityEnds)
    {
      _markEntityEnds = markEntityEnds;
    }

    void setNewsMode(bool newsMode)
    {
      _newsMode = newsMode;
    }

    void setPrechunkMode(bool mode)
    {
      _prechunkMode = mode;
    }

    void setEntityPrefix(const std::string& entityPrefix)
    {
      _entityPrefix = entityPrefix;
    }

    void setWriteWikiDocIdPosting(bool value)
    {
      _writeWikiDocPosting = value;
    }


    // Writes a word an ignores all kinds of entity-checks.
    void writeNonEntity(const std::string& word,
        bool specialDocsFileSeparatorNeeded);

    // Called to indicate that a new sentence starts.
    void newSentence();

    // Like new position but does not check for max headline level.
    void newSection();

    void incPosition()
    {
      ++_pos;
      // If we do not parse for a CompleteSearchIndex,
      // Write a pos delimiter.
      if (_availableEntities.size() > 0) _docsFileOs << DOCSFILE_POS_DELIMITER;
    }

    // Pass additional characters to docs file. For example punctuation marks
    void writeAdditionalCharsToDocsFile(const std::string& chars)
    {
      _docsFileOs << chars;
    }
    // Pass additional characters to docs file. For example punctuation marks
    void writeAdditionalCharsToDocsFile(char character)
    {
      _docsFileOs << character;
    }

    void setPreserveCaseInOutput(bool value)
    {
      _preserveCaseInOutput = value;
    }

    void setOnlyWriteEntitiesFromOntology(bool value)
    {
      _onlyWriteEntitiesFromOntology = value;
    }

    void setDoNotWritePaths(bool value)
    {
      _doNotWritePaths = value;
    }

    void setTitle(const string& title)
    {
      _title = title;
    }

    void setUrl(const string& url)
    {
      _url = url;
    }

    // Checks if some entity is a person
    bool isPerson(const std::string& entity);

    // Resolves the entity that matches the pronoun.
    std::string resolvePronounEntity(const std::string& pronoun);

    // Writes special documents that carry information
    // on the relations between entities.
    void writeEntityContexts(FILE* worsFile, FILE* docsFile);

    void readRedirectMap(const std::string& fileName);
    void readSynonymsMap(const std::string& fileName);
    void readYagoFacts(const std::string& fileName);
    void readRelations(const std::string& fileName);
    void readAnaphoraPronounsFile(const std::string& fileName);
    void readStopWordsFile(const std::string& fileName);
    void readAbstractnessCounts(const string& fileName);

    void initializeAvailableEntitiesMap(const std::string& entityListFile,
        const std::string& redirectMapFile);

    ad_utility::HashMap<std::string, unsigned char>& getClassScoresNonConst()
    {
      return _classScores;
    }

  private:

    std::string _entityPrefix;
    int _currentDefaultScore;
    std::ostringstream _wordsFileOs;
    std::ostringstream _docsFileOs;
    std::string::size_type _minWordLength;
    int _pos;
    int _sentenceNumber;
    size_t _docId;
    size_t _maxHeaderLevelWrittenToNewPos;
    bool _markEntityEnds;
    bool _preserveCaseInOutput;
    bool _onlyWriteEntitiesFromOntology;
    bool _doNotWritePaths;
    bool _newsMode;
    bool _prechunkMode;
    bool _writeWikiDocPosting;

    typedef HashMap<std::string, std::string> RedirectMap;
    typedef HashMap<std::string, int> Pronouns;
    typedef HashMap<std::string, std::vector<std::string> > EntityIsAMap;

    class Relation
    {
      public:
        Relation(const std::string& relation, const std::string& destination,
            const std::string sourceType, const std::string& targetType,
            bool isReversed = false) :
          _relation(relation), _destination(destination),
              _sourceType(sourceType), _targetType(targetType),
              _isReversed(isReversed)
        {
        }
        std::string _relation;
        std::string _destination;
        std::string _sourceType;
        std::string _targetType;
        bool _isReversed;
    };

    class Entity
    {
      public:
        Entity()
        {
        }

        Entity(const std::string& stringRepresentation, bool isPerson)
        : _stringRepresentation(stringRepresentation), _isPerson(isPerson)
        {
        }

        Entity& operator=(const Entity& other)
        {
          _stringRepresentation = other._stringRepresentation;
          _isPerson = other._isPerson;
          return *this;
        }

        std::string _stringRepresentation;
        bool _isPerson;
    };
    typedef HashMap<std::string, std::vector<Relation> > RelationMap;
    typedef HashMap<std::string, std::string> RelationTypeMap;

    // Redirect-map
    RedirectMap _redirectMap;
    RedirectMap _wordnetSynonyms;

    Pronouns _pronouns;
    EntityRepository _repos;
    std::string _lastEntity;

    // Map from entities to YAGO fact. Corresponds to the content
    // of the YAGO isAExtractor.txt. This map should be initialized
    // from the file during instantiation of an EntityRepository
    // and remain unchanged during parsing.
    EntityIsAMap _entityIsAMap;

    ad_utility::HashSet<std::string> _stopWords;

    // Entities identified as abstract entity. Should contain
    // elements that exactly match items from the availableEntities map
    // when that style is enabled and otherwise it should match keys from
    // the _entityIsAMap.
    ad_utility::HashSet<std::string> _abstractEntities;

    // Conatins classes that are supposed to be boosted when writing
    // the :t: words.
    ad_utility::HashMap<std::string, unsigned char> _classScores;

    // Set of all available entities.
    // Represented as a map that also tracks if the given entity is a person.
    // This information is used for anaphora pronoun resolution.
    ad_utility::HashMap<std::string, Entity> _availableEntities;

    std::string _title;
    std::string _url;

    RelationMap _relationMap;

    // Gets the value representation used in excerpts. This includes
    // hidden special words for highlighting.
    std::string formatValueForExcerpts(const std::string& value) const;

    // Extracts the meaningful part of a YAGO fact. At the moment wikicategories
    // are ignores.
    void extractYagoName(const std::string& id, std::string* extraction);

    // Extracts the meaningful part of a YAGO fact.
    // More efficient than extractYagoName() but only works
    // with IsAExtractor. Does not have to check for wikicategories
    // because IsAExtractor only contains wordnet entries.
    void
    extractYagoNameFromIsA(const std::string& id, std::string* extraction);

    // Worker method for writing an entity.
    // Writes all facts obtained for that entity.
    // If there are no facts, the entity prefix itself is written.
    void doWriteEntityWithFacts(const std::string& entity,
        bool onlyWriteDocsPart = false);

    // Worker method for writing an entity.
    // Matches entity against the available entities map and writes
    // the the value if there is one. No extra facts are written.
    void doWriteMatchingEntity(const string& entity);

    // Write the information of the entity itself. No relations,
    // only the 'isA' facts from the taxonomy with adequate
    // presentation.
    void writeEntityContextHeader(const std::string& entity,
        const std::string& normalized, const std::string& displayed);

    void writePostingToWordsfile(const std::string& word, size_t contextId,
        int score, int pos)
    {
      if (_prechunkMode || _stopWords.count(word) == 0)
      {
        _wordsFileOs << word << '\t' << contextId << '\t' << score << '\t'
            << pos << std::endl;
      }
    }

    void writeWikiDocPostingIfEnabled(const string& title, int writeForNthDoc =
        5)
    {
      if (_writeWikiDocPosting && _docId % writeForNthDoc == 0)
      {
        _wordsFileOs << WIKI_DOC_WORD_PREFIX << title << '\t'
            << _docId << '\t' << 0 << '\t' << 0 << std::endl;
      }
    }

    void markEntityEndIfEnabled();


    // Normalized an entity according to the current settings.
    // Normalizing an entity means adjustment of case
    // according to the redirect map (if there is an entry), replacement
    // of white-spaces by underscores and probably following
    // redirects to entirely different entity names:
    // (soccer -> Association_football).
    // Whether or not a redirect is followed depends on the current
    // settings of the EntityAwareWriter.
    string normalizeEntity(const string& words, bool alwaysUseRedirects)
    {
      string key(words);
      for (size_t i = 0; i < key.size(); ++i)
      {
        if (isspace(key[i])) key[i] = '_';
      }
      if (_availableEntities.size() > 0)
      {
        HashMap<std::string, Entity>::const_iterator it =
            _availableEntities.find(key);
        if (it != _availableEntities.end())
        {
          return it->second._stringRepresentation;
        }
        else
        {
          return words;
        }
      }
      // Case: Old settings, use synonyms map to decide
      // if we follow a redirect.
      return ad_semsearch::normalizeEntity(words, _redirectMap,
          _wordnetSynonyms, alwaysUseRedirects);
    }
};
}
#endif  // SEMANTIC_WIKIPEDIA_PARSER_ENTITYAWAREWRITER_H_
