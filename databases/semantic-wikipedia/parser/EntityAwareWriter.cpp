// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <stdio.h>
#include <stdlib.h>
#include <istream>
#include <string>
#include <vector>
#include <sstream>
#include "./EntityAwareWriter.h"
#include "./SemanticWikipediaParser.h"
#include "../codebase/semantic-wikipedia-utils/StringUtils.h"
#include "../codebase/semantic-wikipedia-utils/Log.h"

using ad_utility::DOCS_W_CHAR_MAP;
using ad_utility::W_CHAR_MAP;
using ad_utility::PRECHUNK_W_CHAR_MAP;
using ad_utility::S_CHAR_MAP;

namespace ad_semsearch
{
EntityAwareWriter::EntityAwareWriter(std::string::size_type minWordLength,
    size_t maxHeaderLevel)
{
  _pos = 0;
  _sentenceNumber = 0;
  _docId = 1;
  _minWordLength = minWordLength;
  _maxHeaderLevelWrittenToNewPos = maxHeaderLevel;
  _markEntityEnds = false;
  _onlyWriteEntitiesFromOntology = false;
  _doNotWritePaths = false;
  _newsMode = false;
  _prechunkMode = false;
  _currentDefaultScore = DEFAULT_SCORE;
  _preserveCaseInOutput = false;
  _entityPrefix = ENTITY_PREFIX_OLD;
  _writeWikiDocPosting = true;
}

EntityAwareWriter::~EntityAwareWriter()
{
}
// ____________________________________________________________________________
void EntityAwareWriter::writeWord(const std::string& word)
{
  // Use the lower case version of the word for lookups.
  std::string wordLowerCase(word);
  std::string::iterator it = wordLowerCase.begin();
  while (it != wordLowerCase.end())
  {
    *it = static_cast<char> (tolower(*it));
    ++it;
  }

  std::string title = "";
  // Check for pronouns.
  if (_pronouns.count(wordLowerCase) > 0)
  {
    title = resolvePronounEntity(wordLowerCase);
  }
  else
  {
    if (wordLowerCase.size() < _minWordLength)
    {
      return;
    }
    // Check if the word is part of a title or
    // near Entity depending on settings.
    if (!_newsMode)
    {
      title = _repos.isEntity(wordLowerCase);
    }
  }
  bool wroteEntity = false;
  if (title.size() > 0)
  {
    writeEntity(title, false, false);
    wroteEntity = true;
    // Write the special string that highlights the word if something
    // concerning the entity does match.
    if (!_prechunkMode )
    {
      if (_availableEntities.size() == 0)
      {
        _docsFileOs << DOCSFILE_ENTITY_SEPARATOR;
      }
      else
      {
        _docsFileOs << ' ';
      }
    }
  }
  else
  {
    // Write the "normal" separator for the docsfile
    if (!_prechunkMode) _docsFileOs << ' ';
  }
  // Write the word to the index.
  if (_preserveCaseInOutput)
  {
    writePostingToWordsfile(word, _docId, _currentDefaultScore, _pos);
  }
  else
  {
    writePostingToWordsfile(wordLowerCase, _docId, _currentDefaultScore, _pos);
  }

  if (wroteEntity)
  {
    markEntityEndIfEnabled();
  }

  // Write the word to the docsfile.
  // Note that the separator is written above and depends on if there
  // was some entity detected or not.
  if (!_prechunkMode) _docsFileOs << word;

  // Increase position.
  incPosition();
}
// ____________________________________________________________________________
void EntityAwareWriter::writeNonEntity(const std::string& word,
    bool specialDocsFileSeparatorNeeded)
{
  // Available entities map used -> Not the completesearch index
  // -> no special docsfile separators:
  if (_availableEntities.size() > 0) specialDocsFileSeparatorNeeded = false;

  // Decide which separator to use and forward the word
  // to the docsfile.
  if (!_prechunkMode)
  {
    _docsFileOs << (specialDocsFileSeparatorNeeded ? DOCSFILE_ENTITY_SEPARATOR
        : " ") << word;
  }

  if (word.size() > 0)
  {
    if (_preserveCaseInOutput)
    {
      writePostingToWordsfile(word, _docId, _currentDefaultScore, _pos);
    }
    else
    {
      // Use the lower case version of the word for the index.
      std::ostringstream os;
      for (size_t i = 0; i < word.size(); ++i)
      {
        os << static_cast<char> (tolower(word[i]));
      }
      if (os.str().size() > 0)
      {
        writePostingToWordsfile(os.str(), _docId, _currentDefaultScore, _pos);
      }
    }
  }
}
// ____________________________________________________________________________
void EntityAwareWriter::writeEntity(const std::string& words,
    bool alsoWriteWords, bool addToRepos)
{
  assert(words.size() > 0);
  bool specialDocsFileSepartorNeeded = true;
  if (_availableEntities.size() > 0)
  {
    specialDocsFileSepartorNeeded = false;
    // Write a machting entity from the entity map.
    if (addToRepos)
    {
      // If desired inform the repos that the entity has been discovered.
      _repos.addDiscoveredEntity(words);
    }
    doWriteMatchingEntity(words);
    // Remember that we just processed this entity.
    _lastEntity = words;
  }
  else
  {
    std::string entity = normalizeEntity(words, true);
    assert(entity.size() > 0);
    if (addToRepos)
    {
      // If desired inform the repos that the entity has been discovered.
      _repos.addDiscoveredEntity(entity);
    }
    // Write the YAGO facts about that entity
    doWriteEntityWithFacts(entity);
    // Remember that we just processed this entity.
    _lastEntity = entity;
  }

  // Check if the words themselves should be written.
  if (alsoWriteWords)
  {
    // Write the words if desired. If they are not written, that
    // usually means that in fact some other word was identified as this entity,
    // for example a pronoun, and is written instead.
    std::ostringstream wordOs;
    for (std::string::size_type i = 0; i < words.length(); ++i)
    {
      if (PRECHUNK_W_CHAR_MAP[static_cast<uint8_t> (words[i])] == 'w')
      {
        wordOs << words[i];
      }
      else
      {
        if (wordOs.str().length() >= _minWordLength)
        {
          writeNonEntity(wordOs.str(), specialDocsFileSepartorNeeded);
        }
        wordOs.str("");
      }
    }
    if (wordOs.str().length() >= _minWordLength)
    {
      writeNonEntity(wordOs.str(), specialDocsFileSepartorNeeded);
      wordOs.str("");
    }
    markEntityEndIfEnabled();
  }
}
// ____________________________________________________________________________
void EntityAwareWriter::writeEntityWithHiding(const std::string& text)
{
  assert(text.size() > 2);
  // Use two streams. One for the title and one for the actual words
  std::ostringstream wordOs;
  std::ostringstream titleOs;
  bool paramOpen = false;
  std::vector<std::string> words;
  for (std::string::size_type i = 0; i < text.length(); ++i)
  {
    if (W_CHAR_MAP[static_cast<uint8_t> (text[i])] != 's')
    {
      if (text[i] == '(')
      {
        paramOpen = true;
        continue;
      }
      if (text[i] == ')')
      {
        paramOpen = false;
        continue;
      }
      if (!paramOpen && PRECHUNK_W_CHAR_MAP[static_cast<uint8_t> (text[i])]
          == 'w')
      {
        wordOs << text[i];
      }
    }
    else
    {
      if (wordOs.str().length() >= _minWordLength)
      {
        words.push_back(wordOs.str());
      }
      wordOs.str("");
    }
    titleOs << text[i];
  }
  if (wordOs.str().length() >= _minWordLength)
  {
    words.push_back(wordOs.str());
  }

  // Write title as entity and words as non entity
  writeEntity(titleOs.str(), false, true);
  for (size_t i = 0; i < words.size(); ++i)
  {
    writeNonEntity(words[i], true);
  }
  markEntityEndIfEnabled();
}
// ____________________________________________________________________________
void EntityAwareWriter::writeEntityWithRenaming(const std::string& title,
    const std::string& renaming)
{
  assert(title.size() > 0);
  writeEntity(title, false, true);
  // Write the words
  std::ostringstream wordOs;
  for (std::string::size_type i = 0; i < renaming.length(); ++i)
  {
    if (ad_utility::PRECHUNK_W_CHAR_MAP[static_cast<uint8_t> (renaming[i])]
        == 'w')
    {
      wordOs << renaming[i];
    }
    else
    {
      if (S_CHAR_MAP[static_cast<uint8_t> (renaming[i])] == 'y'
          && wordOs.str().length() >= _minWordLength)
      {
        writeNonEntity(wordOs.str(), true);
      }
      wordOs.str("");
    }
  }
  if (wordOs.str().length() >= _minWordLength)
  {
    writeNonEntity(wordOs.str(), true);
    wordOs.str("");
  }
  markEntityEndIfEnabled();
}
// ____________________________________________________________________________
void EntityAwareWriter::writeSectionHead(const std::string& words,
    int sectionLevel, const std::vector<std::string>& associatedEntities)
{
  // Clean the string, remove everything that is no word char.
  // replace all possible separators by white spaces.
  std::ostringstream os;
  for (std::string::size_type i = 0; i < words.length(); ++i)
  {
    if (W_CHAR_MAP[static_cast<uint8_t> (words[i])] == 'w')
    {
      os << words[i];
    }
    else if (S_CHAR_MAP[static_cast<uint8_t> (words[i])] == 'y')
    {
      os << ' ';
    }
  }
  std::string cleanHeader = os.str();
  // Inform the repository
  _repos.setNewSectionHeader(cleanHeader, sectionLevel, associatedEntities);

  // Write the section entities at the new position
  newSection();
}
// ____________________________________________________________________________
void EntityAwareWriter::writeCategory(const std::string& categoryString)
{
  // Build the concatenated version (only word chars)
  // and the one with underscores
  std::ostringstream concatOs;
  std::ostringstream underscoreOs;

  for (std::string::size_type i = 0; i < categoryString.length(); ++i)
  {
    if (W_CHAR_MAP[static_cast<uint8_t> (categoryString[i])] == 'w')
    {
      concatOs << static_cast<char> (tolower(categoryString[i]));
      underscoreOs << categoryString[i];
    }
    else
    {
      if (W_CHAR_MAP[static_cast<uint8_t> (categoryString[i])] == 's')
      {
        underscoreOs << '_';
      }
      else
      {
        // Keep other chars
        underscoreOs << categoryString[i];
      }
    }
  }

  std::ostringstream os;
  os << CATEGORY_PREFIX << concatOs.str() << ":" << underscoreOs.str();
  writePostingToWordsfile(os.str(), _docId, _currentDefaultScore, _pos);
  writePostingToWordsfile(":inDoc:" + _title, _docId, _currentDefaultScore,
      _pos);
  if (!_prechunkMode) _docsFileOs << " ^^" << CATEGORY_PREFIX_DOCSFILE
      << concatOs.str() << "^^" << underscoreOs.str();
}
// ____________________________________________________________________________
void EntityAwareWriter::finish(std::string* wordsFileOutput,
    std::string* docsFileOutput)
{
  *wordsFileOutput = _wordsFileOs.str();
  _wordsFileOs.str("");
  if (_prechunkMode) assert(_docsFileOs.str().size() == 0);
  *docsFileOutput = _docsFileOs.str();
  _docsFileOs.str("");
  _pos = 0;
  _sentenceNumber = 0;
}
// ____________________________________________________________________________
void EntityAwareWriter::setDocument(const std::string& origTitle)
{
  string title = origTitle.size() > 0 ? origTitle : "no title";

  _pos = 0;
  _sentenceNumber = 1;
  _docId++;
  if (_prechunkMode)
  {
    std::ostringstream os;
    os << ":title:" << _title << " [" << _sentenceNumber << "]";
    writePostingToWordsfile(os.str(), _docId, _currentDefaultScore, _pos);
    writePostingToWordsfile(std::string(":url:") + _url, _docId,
        _currentDefaultScore, _pos);
  }
  else
  {
    _docsFileOs << _docId << '\t' << "u:" << _url
        << '\t' << "t:" << _title << " [" << _sentenceNumber << "]\tH:";
  }

  writeWikiDocPostingIfEnabled(_title);

  // NEW 28June 11:
  // Use low scores for the headline because headlines
  // usually are no interesting sentences and only wanted as excerpt
  // if there is no other source.
  int currentScoreBackup = _currentDefaultScore;
  _currentDefaultScore = LOWEST_SCORE;

  if (_availableEntities.size() == 0)
  {
    std::string entity = normalizeEntity(title, true);
    _repos.setNewPageEntity(entity);
    assert(entity.size() > 0);
    writeEntity(entity, false, false);
  }
  else
  {
    _repos.setNewPageEntity(title);
    writeEntity(title, false, false);
  }
  // Tokenize the title words
  std::ostringstream wordOs;
  for (std::string::size_type i = 0; i < title.length(); ++i)
  {
    if (PRECHUNK_W_CHAR_MAP[static_cast<uint8_t> (title[i])] == 'w')
    {
      wordOs << title[i];
    }
    else
    {
      if (wordOs.str().length() >= _minWordLength)
      {
        writeNonEntity(wordOs.str(), true);
      }
      wordOs.str("");
    }
  }
  if (wordOs.str().length() >= _minWordLength)
  {
    writeNonEntity(wordOs.str(), true);
    wordOs.str("");
  }
  markEntityEndIfEnabled();

  _currentDefaultScore = currentScoreBackup;
}
// ____________________________________________________________________________
void EntityAwareWriter::newSentence()
{
  ++_docId;
  ++_sentenceNumber;
  _pos = 0;
  if (_prechunkMode)
  {
    std::ostringstream os;
    os << ":title:" << _title << " [" << _sentenceNumber << "]";
    writePostingToWordsfile(os.str(), _docId, _currentDefaultScore, _pos);
    writePostingToWordsfile(std::string(":url:") + _url, _docId,
        _currentDefaultScore, _pos);
  }
  else
  {
    _docsFileOs << "\n" << _docId << '\t' << "u:" << _url << '\t' << "t:"
        << _title << " [" << _sentenceNumber << "]\tH:";
  }

  writeWikiDocPostingIfEnabled(_title);

  std::vector<string> sectionHeaders = _repos.getSectionHeaders();
  std::vector<string> sectionEntities = _repos.getSectionEntities();
  // For each section header including document title,
  for (size_t i = 0; (i < sectionHeaders.size() && i
      < _maxHeaderLevelWrittenToNewPos); ++i)
  {
    // Write the associated entity.
    if (sectionEntities[i] != "")
    {
      writeEntity(sectionEntities[i], false, false);
      markEntityEndIfEnabled();
    }
    // Write the words of the header.
    std::string header = sectionHeaders[i];
    std::ostringstream wordOs;
    for (std::string::size_type j = 0; j < header.length(); ++j)
    {
      if (W_CHAR_MAP[static_cast<uint8_t> (header[j])] == 'w' || header[j] < 0)
      {
        wordOs << header[j];
      }
      else
      {
        if (wordOs.str().length() >= _minWordLength)
        {
          writeNonEntity(wordOs.str(), true);
        }
        wordOs.str("");
      }
    }
    if (wordOs.str().length() >= _minWordLength)
    {
      writeNonEntity(wordOs.str(), true);
      wordOs.str("");
    }
  }
  incPosition();
}
// ____________________________________________________________________________
void EntityAwareWriter::newSection()
{
  _pos = 0;
  _sentenceNumber++;
  _docId++;
  if (_prechunkMode)
  {
    std::ostringstream os;
    os << ":title:" << _title << " [" << _sentenceNumber << "]";
    writePostingToWordsfile(os.str(), _docId, _currentDefaultScore, _pos);
    writePostingToWordsfile(std::string(":url:") + _url, _docId,
            _currentDefaultScore, _pos);
  }
  else
  {
    _docsFileOs << "\n" << _docId << '\t' << "u:" << _url << '\t' << "t:"
        << _title << " [" << _sentenceNumber << "]\tH:";
  }

  writeWikiDocPostingIfEnabled(_title);

  std::vector<string> sectionHeaders = _repos.getSectionHeaders();
  std::vector<string> sectionEntities = _repos.getSectionEntities();

  // NEW 28June 11:
  // Use very low scores for section headlines because they
  // usually make very poor sentences and are not wanted as excerpts
  int currentScoreBackup = _currentDefaultScore;
  _currentDefaultScore = LOWEST_SCORE;

  // For each section header including document title,
  for (size_t i = 0; i < sectionHeaders.size(); ++i)
  {
    // Only write levels that should be written PLUS the current new header
    if ((i >= _maxHeaderLevelWrittenToNewPos) && i != (sectionHeaders.size()
        - 1))
    {
      continue;
    }

    if (i == sectionHeaders.size() - 1) incPosition();
    // Write the associated entity.
    if (sectionEntities[i] != "")
    {
      writeEntity(sectionEntities[i], false, false);
      markEntityEndIfEnabled();
    }
    // Write the words of the header.
    std::string header = sectionHeaders[i];
    std::ostringstream wordOs;
    for (std::string::size_type j = 0; j < header.length(); ++j)
    {
      if (W_CHAR_MAP[static_cast<uint8_t> (header[j])] == 'w' || header[j] < 0)
      {
        wordOs << header[j];
      }
      else
      {
        if (wordOs.str().length() >= _minWordLength)
        {
          writeNonEntity(wordOs.str(), true);
          if (i == sectionHeaders.size() - 1) incPosition();
        }
        wordOs.str("");
      }
    }
    if (wordOs.str().length() >= _minWordLength)
    {
      writeNonEntity(wordOs.str(), true);
      if (i == sectionHeaders.size() - 1) incPosition();
      wordOs.str("");
    }
  }
  _currentDefaultScore = currentScoreBackup;
}
// _____________________________________________________________________________
bool EntityAwareWriter::isPerson(const std::string& entity)
{
  HashMap<std::string, Entity>::const_iterator it = _availableEntities.find(
      entity);
  if (it != _availableEntities.end())
  {
    return it->second._isPerson;
  }

  if (_entityIsAMap.count(
      normalizeEntity(entity, false)) > 0)
  {
    std::vector<std::string>& facts = _entityIsAMap[entity];
    for (size_t i = 0; i < facts.size(); ++i)
    {
      if (facts[i].find(":person") != string::npos)
      {
        return true;
      }
    }
  }
  return false;
}
// _____________________________________________________________________________
std::string EntityAwareWriter::resolvePronounEntity(const string& pronoun)
{
  // TODO(buchholb): Refine heuristic and maybe
  // implement a possibility to switch between heuristics.
  bool pronounIsForPerson = _pronouns[pronoun] <= 2;
  std::string title = "";

  if (isPerson(_repos.getSectionEntities()[0]) == pronounIsForPerson)
  {
    title = _repos.getSectionEntities()[0];
  }
  else if (isPerson(_lastEntity) == pronounIsForPerson)
  {
    title = _lastEntity;
  }
  return title;
}
// _____________________________________________________________________________
void EntityAwareWriter::setYagoRelationsFileName(
    const string& relationsFileName)
{
  this->readRelations(relationsFileName);
}
// ____________________________________________________________________________
void EntityAwareWriter::doWriteEntityWithFacts(const std::string& entity,
    bool onlyWriteDocsPart)
{
  assert(entity.size() > 0);

  // Check if it is a value
  if (entity[0] == ':')
  {
    // The !_doNotWritePaths ensures no such things are
    // written when parsing for the new-index.
    // TODO(buchholb): Urgently refactor this when the whole parser is
    // Changed to properly match the new index chain.
    if (!onlyWriteDocsPart && !_doNotWritePaths)
    {
      // Content of "entity" is either a value or some useless fragment.
      // having them in addition does not much harm
      // and wont happen for the new index.
      writePostingToWordsfile(entity, _docId, _currentDefaultScore, _pos);
    }
    if (!_prechunkMode) _docsFileOs << "^^" << entity << "x";
    return;
  }

  // Shoulnd't be a value but a normal entity instead.
  string normalizedEntity = normalizeEntity(entity, false);
  // If the option is enabled, check if there is any entry in the ontology
  // for this entitiy. If there isn't, don't write anything and ignore
  // the entity. There is no additional information in comparison
  // to an ordinary words, anyways.
  if (_onlyWriteEntitiesFromOntology)
  {
    if (_entityIsAMap.count(normalizedEntity) == 0 && _relationMap.count(
        normalizedEntity) == 0)
    {
      return;
    }
  }

  // Ensure that all entities occur in the relations map, regardless
  // of whether or not they occur in yago. This is required because
  // only then a special entity-context will be written for them and
  // the semantic search assumes that such a context exists for each
  // and every entity.
  if (!_doNotWritePaths && _relationMap.size() > 0)
  {
    _relationMap[normalizedEntity];
  }
  int localScore = (
      _abstractEntities.count(normalizedEntity) > 0 ? 0 : _currentDefaultScore);
  if (localScore > LOWEST_SCORE && _repos.isDocumentEntity(entity))
  {
    if (_currentDefaultScore < DOCUMENT_ENTITY_SCORE)
    {
      localScore = (_sentenceNumber == 2 ? DOCUMENT_ENTITY_FIRSTCONTEXT_SCORE
          : DOCUMENT_ENTITY_SCORE);
    }
  }
  std::string normalizedLowercase = ad_utility::getNormalizedLowercase(
      normalizedEntity);

  std::vector<std::string> noFacts;
  std::vector<std::string>& facts = noFacts;
  if (_entityIsAMap.count(normalizedEntity) > 0)
  {
    facts = _entityIsAMap[normalizedEntity];
  }

  // The artificial monster words for types.
  if (!_prechunkMode) _docsFileOs << " ";
  if (!_doNotWritePaths)
  {
    for (size_t i = 0; i < facts.size(); ++i)
    {
      if (!onlyWriteDocsPart || _prechunkMode)
      {
        _wordsFileOs << FACT_PREFIX << facts[i] << ":" << normalizedLowercase
            << ":" << normalizedEntity << '\t' << _docId << '\t' << localScore
            << '\t' << _pos << "\n";
      }

      const std::string& fact = facts[i];
      std::ostringstream factOs;
      for (std::string::size_type j = 0; j < fact.size(); ++j)
      {
        if (fact[j] == ':') factOs << "x";
        else if (!(fact[j] == '_')) factOs << fact[j];
      }
      if (!_prechunkMode) _docsFileOs << "^^" << FACT_PREFIX_DOCSFILE
          << factOs.str() << "x" << normalizedLowercase << "x";
    }
  }
  // The artificial word for just the entity, for example:
  // :e:entity:alberteinstein: (and with x and ^ in the docs file).
  // _wordsFileOs << ENTITY_PREFIX << lowercase << ":" << entity << '\t'
  //    << _docId << '\t' << "0" << '\t' << _pos << "\n";
  if (!onlyWriteDocsPart || _prechunkMode)
  {
    _wordsFileOs << _entityPrefix << normalizedLowercase << ":"
        << normalizedEntity << '\t' << _docId << '\t' << localScore << '\t'
        << _pos << "\n";
  }
  if (!_prechunkMode) _docsFileOs << "^^" << ENTITY_PREFIX_DOCSFILE
      << normalizedLowercase << "x";

  // We now rely on the surrounding methods to write the necessary
  // string to be displayed preceded by the ^^^ separator.
  // So the final ^^^X is no longer written to the docsfile.
}
// _____________________________________________________________________________
void EntityAwareWriter::doWriteMatchingEntity(const string& entity)
{
  std::string key(entity);
  for (size_t i = 0; i < key.size(); ++i)
  {
    if (isspace(key[i])) key[i] = '_';
  }
  HashMap<std::string, Entity>::const_iterator it = _availableEntities.find(
      key);
  // Only write something at all, if we found it in the map.
  if (it != _availableEntities.end())
  {
    // Determine the score for this entity
    int localScore = (
        _abstractEntities.count(it->second._stringRepresentation) > 0 ? 0 :
            _currentDefaultScore);
    if (localScore > LOWEST_SCORE && (_repos.isDocumentEntity(key)
        || _repos.isDocumentEntity(entity)))
    {
      if (_currentDefaultScore < DOCUMENT_ENTITY_SCORE)
      {
        localScore = (_sentenceNumber == 2 ? DOCUMENT_ENTITY_FIRSTCONTEXT_SCORE
            : DOCUMENT_ENTITY_SCORE);
      }
    }
    writePostingToWordsfile(it->second._stringRepresentation, _docId,
        localScore, _pos);
  }
}
// _____________________________________________________________________________
void EntityAwareWriter::writeEntityContexts(FILE* wordsFile, FILE* docsFile)
{
  assert(_wordsFileOs.str() == "");
  assert(_docsFileOs.str() == "");

  // Now, in prechunk-mode start writing the docsfile, again. This means
  // stop prechunk-mode. Should be okay, because entity contexts are always
  // written in the end, only.
  _prechunkMode = false;
  _currentDefaultScore = ENTITY_CONTEXT_SCORE;
  for (RelationMap::const_iterator it = _relationMap.begin(); it
      != _relationMap.end(); ++it)
  {
    _docId++;
    _pos = 1;

    const std::string& subject = it->first;
    const std::vector<Relation>& relations = it->second;

    std::string normalizedSubject;
    std::string displayedSubject;

    assert(subject.size() > 0);
    if (subject[0] == ':')
    {
      // Value.
      normalizedSubject = subject;
      displayedSubject = formatValueForExcerpts(subject);
    }
    else
    {
      // Not a value.
      // Build the strings representing the subject in the
      // index and the docs file.
      // Build the version that is displayed
      std::ostringstream displayOs;

      bool nextCapital = true;

      for (size_t i = 0; i < subject.length(); ++i)
      {
        if (W_CHAR_MAP[static_cast<uint8_t> (subject[i])] == 'w')
        {
          if (nextCapital)
          {
            displayOs << static_cast<char> (toupper(subject[i]));
            nextCapital = false;
          }
          else
          {
            displayOs << subject[i];
          }
        }
        else
        {
          if (subject[i] == '_' || subject[i] == '-' || subject[i] == '#')
          {
            displayOs << DOCSFILE_ENTITY_SEPARATOR;
            nextCapital = true;
          }
          else
          {
            nextCapital = true;
          }
        }
      }
      normalizedSubject = ad_utility::getNormalizedLowercase(subject);
      assert(normalizedSubject.size() > 0);
      displayedSubject = displayOs.str();
    }

    writeEntityContextHeader(subject, normalizedSubject, displayedSubject);

    for (size_t j = 0; j < relations.size(); ++j)
    {
      const std::string& relation = relations[j]._relation;
      const std::string& dest = relations[j]._destination;
      const std::string& sourceType = relations[j]._sourceType;
      const std::string& destType = relations[j]._targetType;

      // Get the relation name without special chars
      std::string relationEGString = string("^^")
          + ad_utility::noSpecialChars(relation) + "^^^";

      _docsFileOs << DOCSFILE_RELATION_MARKER << "^^" << relation
          << (relations[j]._isReversed ? "_rev " : " ");
      ++_pos;

      std::string normalizedDest;
      std::string displayedDest;

      assert(dest.size() > 0);
      if (dest[0] == ':')
      {
        // Value.
        normalizedDest = dest;
        displayedDest = formatValueForExcerpts(dest);
      }
      else
      {
        // Build the strings representing the dest in the
        // index and the docs file.
        // Build the version that is displayed
        std::ostringstream displayOs;
        bool nextCapital = true;

        for (size_t i = 0; i < dest.length(); ++i)
        {
          if (W_CHAR_MAP[static_cast<uint8_t> (dest[i])] == 'w')
          {
            if (nextCapital)
            {
              displayOs << static_cast<char> (toupper(dest[i]));
              nextCapital = false;
            }
            else
            {
              displayOs << dest[i];
            }
          }
          else
          {
            if (dest[i] == '_' || dest[i] == '-' || dest[i] == '#')
            {
              displayOs << DOCSFILE_ENTITY_SEPARATOR;
              nextCapital = true;
            }
            else
            {
              nextCapital = true;
            }
          }
        }
        normalizedDest = ad_utility::getNormalizedLowercase(dest);
        assert(normalizedDest.size() > 0);
        displayedDest = displayOs.str();
      }
      // NEW 28Mar11(Björn): Do not write relations in both directions,
      // instead keep the information which one is source and
      // which one is dest and always write it in the same order.
      if (!relations[j]._isReversed)
      {
        // CASE: not reversed
        _wordsFileOs << REL_PREFIX << relation << ":" << normalizedSubject
            << ":" << normalizedDest << '\t' << _docId << '\t'
            << _currentDefaultScore << '\t' << _pos << "\n";

        _wordsFileOs << NOT_REVERSED_RELATION_MARKER << '\t' << _docId << '\t'
            << _currentDefaultScore << '\t' << _pos << "\n";

        _wordsFileOs << REL_TYPE_PREFIX << relation << ":" << sourceType
            << ":" << destType << '\t' << _docId << '\t'
            << _currentDefaultScore << '\t' << _pos << "\n";

        doWriteEntityWithFacts(subject, true);
        _docsFileOs << "^^^" << displayedSubject << " " << relationEGString
            << relation << " ";
        doWriteEntityWithFacts(dest, true);
        _docsFileOs << "^^^" << displayedDest << ". ";
      }
      else
      {
        // CASE: reversed
        _wordsFileOs << REL_PREFIX << relation << ":" << normalizedDest << ":"
            << normalizedSubject << '\t' << _docId << '\t'
            << _currentDefaultScore << '\t' << _pos << "\n";

        _wordsFileOs << REVERSED_RELATION_MARKER << '\t' << _docId << '\t'
            << _currentDefaultScore << '\t' << _pos << "\n";

        _wordsFileOs << REL_TYPE_PREFIX << REVERSED_RELATION_MARKER
            << relation << ":" << sourceType << ":" << destType << '\t'
            << _docId << '\t' << _currentDefaultScore << '\t' << _pos << "\n";

        doWriteEntityWithFacts(dest, true);
        _docsFileOs << "^^^" << displayedDest << " " << relationEGString
            << relation << " ";
        doWriteEntityWithFacts(subject, true);
        _docsFileOs << "^^^" << displayedSubject << ". ";
      }
      _docsFileOs << " " << DOCSFILE_RELATION_END_MARKER;
    }
    _docsFileOs << "\n";

    // Write the files
    fprintf(wordsFile, "%s", _wordsFileOs.str().c_str());
    fprintf(docsFile, "%s", _docsFileOs.str().c_str());
    _wordsFileOs.str("");
    _docsFileOs.str("");
  }

  _currentDefaultScore = DEFAULT_SCORE;
}
// ___________________________________________________________________________
void EntityAwareWriter::writeEntityContextHeader(const std::string& entity,
    const std::string& normalized, const std::string& displayed)
{
  _docsFileOs << _docId << '\t' << "u:"
      << "http://www.mpi-inf.mpg.de/yago-naga/yago/" << '\t' << "t:"
      << "YAGO Ontology: " << entity << "\tH:";

  _wordsFileOs << ENTITY_CONTEXT_PREFIX << normalized << '\t' << _docId
      << '\t' << _currentDefaultScore << '\t' << _pos << "\n";

  assert(normalized.size() > 0);

  // Check if we're dealing with a value
  if (normalized[0] == ':')
  {
    _wordsFileOs << normalized << '\t' << _docId
        << '\t' << _currentDefaultScore << '\t' << _pos << "\n";

    _docsFileOs << DOCSFILE_ENTITY_SEPARATOR << displayed;

    return;
  }

  // No Value but a normal entity.
  EntityIsAMap::const_iterator it = _entityIsAMap.find(
      normalizeEntity(entity, false));

  std::vector<std::string> classes;
  if (it != _entityIsAMap.end()) classes = it->second;
  // The artificial word for just the entity:
  _wordsFileOs << _entityPrefix << normalized << ":" << entity << '\t'
      << _docId << '\t' << _currentDefaultScore << '\t' << _pos << "\n";

  doWriteEntityWithFacts(entity, true);
  _docsFileOs << DOCSFILE_ENTITY_SEPARATOR << displayed;
  _docsFileOs << " is a: ";

  // The artificial monster words for types.
  for (size_t i = 0; i < classes.size(); ++i)
  {
    _wordsFileOs << FACT_PREFIX << classes[i] << ":" << normalized << ":"
        << entity << '\t' << _docId << '\t' << _currentDefaultScore << '\t'
        << _pos << "\n";

    const std::string& fact = classes[i];
    std::ostringstream factOs;
    for (std::string::size_type j = 0; j < fact.size(); ++j)
    {
      if (fact[j] == ':') factOs << "x";
      else if (!(fact[j] == '_')) factOs << fact[j];
    }
    _docsFileOs << "^^" << FACT_PREFIX_DOCSFILE << factOs.str() << "x"
        << DOCSFILE_ENTITY_SEPARATOR << classes[i].substr(
        classes[i].rfind(':') + 1) << (i < (classes.size() - 2) ? ", " : (i
        < (classes.size() - 1) ? " and " : ". "));
  }
}
// ____________________________________________________________________________
void EntityAwareWriter::readRelations(const std::string& yagoRelationsFileName)
{
  std::cout << "Reading YAGO relation data...\t" << std::flush;

  std::ifstream yaStream(yagoRelationsFileName.c_str(), std::ios::in);
  std::string line;
  int factsCount = 0;
  // Read line-wise.
  while (std::getline(yaStream, line))
  {
    // Tokenize tab separated file content.
    // The input file looks like this:
    // relation<tab>sourceType<tab>sourceEntity<tab>destType<tab>destEntity
    std::string::size_type indextofTab = line.find('\t');
    std::string::size_type indextofTab2 = line.find('\t', indextofTab + 1);
    std::string::size_type indextofTab3 = line.find('\t', indextofTab2 + 1);
    std::string::size_type indextofTab4 = line.find('\t', indextofTab3 + 1);
    assert(indextofTab != std::string::npos);
    assert(indextofTab2 != std::string::npos);
    assert(indextofTab3 != std::string::npos);
    assert(indextofTab4 != std::string::npos);

    std::string relation = line.substr(0, indextofTab);
    std::string sourceType = line.substr(indextofTab + 1,
        indextofTab2 - (indextofTab + 1));
    std::string sourceVal = line.substr(indextofTab2 + 1,
        indextofTab3 - (indextofTab2 + 1));
    std::string destType = line.substr(indextofTab3 + 1,
        indextofTab4 - (indextofTab3 + 1));
    std::string destVal = line.substr(indextofTab4 + 1);

    // Add the info to the hash-map member.
    std::vector<Relation>& relVec = _relationMap[sourceVal];
    Relation triple(relation, destVal, sourceType, destType);
    relVec.push_back(triple);

    // For relations with a destination other than value,
    // add the reversed direction, too.
    // Now always add the reversed direction, also for values.
//    if (!ad_utility::startsWith(destType, VALUE_RELATION_TYPE_PREFIX))
    {
      std::vector<Relation>& relVecDest = _relationMap[destVal];
      Relation revTriple(relation, sourceVal, destType, sourceType, true);
      relVecDest.push_back(revTriple);
    }
    factsCount++;
  }
  yaStream.close();
  std::cout << "read " << factsCount << " relations with "
      << _relationMap.size() << " different entities as subject"
      << " or object from file \"" << yagoRelationsFileName << "\""
      << std::endl;
}
// _____________________________________________________________________________
void EntityAwareWriter::readAnaphoraPronounsFile(const std::string& fileName)
{
  std::ifstream apFile(fileName.c_str(), std::ios::in);
  std::string line;
  while (std::getline(apFile, line))
  {
    std::string::size_type indexOfTab = line.find('\t');
    assert(indexOfTab != std::string::npos);
    const std::string pronoun = line.substr(0, indexOfTab);
    int typeId = atoi(line.substr(indexOfTab + 1).c_str());
    _pronouns[pronoun] = typeId;
  }
  apFile.close();
  LOG(INFO) << "Read " << _pronouns.size() << " pronouns from file \""
      << fileName << "\"" << std::endl;
}
// ____________________________________________________________________________
void EntityAwareWriter::extractYagoName(const std::string& id,
    std::string* extraction)
{
  std::string::size_type underscorePos = id.find('_');
  assert(underscorePos != std::string::npos);
  std::string prefix = id.substr(0, underscorePos);
  if (prefix == "wordnet")
  {
    // For wordnet links trim the number.
    std::string::size_type lastUnderscorePos = id.find_last_of('_');
    assert(lastUnderscorePos != std::string::npos);
    *extraction = id.substr(underscorePos + 1,
        lastUnderscorePos - (underscorePos + 1));
  }
  else
  {
    // Handle Wikicategories. They're ignored at the moment.
    // This may be the best solution, though.
  }
}
// ____________________________________________________________________________
void EntityAwareWriter::extractYagoNameFromIsA(const std::string& id,
    std::string* extraction)
{
  std::string::size_type underscorePos = id.find('_');
  assert(underscorePos != std::string::npos);

  // For wordnet links trim the number.
  std::string::size_type lastUnderscorePos = id.find_last_of('_');
  assert(lastUnderscorePos != std::string::npos);
  *extraction = FACT_PREFIX + id.substr(underscorePos + 1,
      lastUnderscorePos - (underscorePos + 1));
}
// ____________________________________________________________________________
void EntityAwareWriter::readRedirectMap(
    const std::string& redirectMapFileName)
{
  std::cout << "Reading wikipedia redirect-map from file "
      << redirectMapFileName << "...\t" << std::flush;

  // Read the title - entity mapping.
  std::ifstream fs(redirectMapFileName.c_str(), std::ios::in);
  // Read line-wise.
  std::string line;
  while (std::getline(fs, line))
  {
    // Separate titles and entity by tabs.
    std::string::size_type indextofTab = line.find('\t');
    std::string title = line.substr(0, indextofTab);
    std::string entity = line.substr(indextofTab + 1);
    _redirectMap[title] = entity;
  }
  fs.close();
  std::cout << "Read " << _redirectMap.size() << " pairs."
      << std::endl << std::flush;
}
// ____________________________________________________________________________
void EntityAwareWriter::readSynonymsMap(
    const std::string& synonymsMapFileName)
{
  std::cout << "Reading wordnet-synonyms map from file " << synonymsMapFileName
      << "...\t" << std::flush;

  // Read the title - entity mapping.
  std::ifstream fs(synonymsMapFileName.c_str(), std::ios::in);
  // Read line-wise.
  std::string line;
  while (std::getline(fs, line))
  {
    // Separate titles and entity by tabs.
    std::string::size_type indextofTab = line.find('\t');
    if (!(indextofTab > 0 && indextofTab != line.npos))
    {
      std::cout << "discovered broken line: " << line << std::endl
          << std::flush;
      continue;
    }
    // assert(indextofTab > 0 && indextofTab != line.npos);
    std::string key = ad_utility::getNormalizedLowercase(
        line.substr(0, indextofTab));
    std::string value = ad_utility::getNormalizedLowercase(
        line.substr(indextofTab + 1));

    _wordnetSynonyms[key] = value;
  }
  fs.close();
  std::cout << "Read " << _wordnetSynonyms.size() << " pairs."
      << std::endl << std::flush;
}
// ____________________________________________________________________________
void EntityAwareWriter::readYagoFacts(const std::string& yagoFactsFileName)
{
  std::cout << "Reading YAGO type data...\t" << std::flush;

  std::ifstream yaStream(yagoFactsFileName.c_str(), std::ios::in);
  std::string line;
  int factsCount = 0;
  // Read line-wise.
  while (std::getline(yaStream, line))
  {
    // Separate entities and what they are by tabs.
    // The input file look like this:
    // entity<tab>fact
    std::string::size_type indextofTab = line.find('\t');
    assert(indextofTab != std::string::npos);

    std::string entity = line.substr(0, indextofTab);
    std::string fact = line.substr(indextofTab + 1);

    // Add the info to the hash-map member.
    std::vector<std::string>& factsVec = _entityIsAMap[normalizeEntity(entity,
        false)];
    factsVec.push_back(fact);
    factsCount++;
  }
  yaStream.close();
  std::cout << "read " << factsCount << " facts about "
      << _entityIsAMap.size() << " entities from file \"" << yagoFactsFileName
      << "\"" << std::endl << "This means there are " << size_t(factsCount)
      / _entityIsAMap.size() << " facts per entity in avg. " << std::endl;
}
// _____________________________________________________________________________
void EntityAwareWriter::markEntityEndIfEnabled()
{
  if (_markEntityEnds) _wordsFileOs << ENTITY_END_MARKER << '\t' << _docId
      << '\t' << 0 << '\t' << _pos << std::endl;
}
// _____________________________________________________________________________
std::string EntityAwareWriter::formatValueForExcerpts(const std::string& value)
const
{
  std::ostringstream os;
  os << VALUE_HIGHLIGHTING_PREFIX;
  if (ad_utility::startsWith(value, ":date:"))
  {
    os << value.substr(6, 4);
    if (value.substr(10, 2) != "00")
    {
      os << "-" << value.substr(10, 2) << "-" << value.substr(12);
    }
  } else if (ad_utility::startsWith(value, ":integer:"))
  {
    os << value.substr(9);
  } else if (ad_utility::startsWith(value, ":float:"))
  {
    os << value.substr(7);
  } else
  {
    std::cerr << "Warning! Unknown value type: "
        << value << std::endl;
    os << value;
  }
  return os.str();
}
// _____________________________________________________________________________
void EntityAwareWriter::readStopWordsFile(const std::string& fileName)
{
  std::ifstream stream(fileName.c_str(), std::ios::in);
  std::string line;
  // Read line-wise.
  while (std::getline(stream, line))
  {
    _stopWords.insert(line);
  }
  stream.close();
  LOG(INFO)
      << "Read " << _stopWords.size() << " stop-words from file" << fileName
      << "." << std::endl;
}
// _____________________________________________________________________________
void EntityAwareWriter::initializeAvailableEntitiesMap(
    const std::string& entityListFile, const std::string& redirectMapFile)
{
  LOG(INFO)
  << "Reading available entities from file " << entityListFile << "."
      << std::endl;

  {
    std::ifstream stream(entityListFile.c_str(), std::ios::in);
    std::string line;

    // Read line-wise.
    while (std::getline(stream, line))
    {
      size_t indexOftab = line.find('\t');
      assert(indexOftab != 0 && indexOftab != string::npos);
      string entity = line.substr(0, indexOftab);
      assert(entity.size() > 0);
      _availableEntities[ad_utility::getLastPartOfString(entity, ':')]
          = Entity(entity, (line.substr(indexOftab + 1) == "1"));
    }
    stream.close();
    LOG(INFO)
    << "read " << _availableEntities.size() << " entities." << std::endl;
  }

  if (redirectMapFile.size() == 0)
  {
    LOG(WARN)
    << "No redirect-map specified!" << std::endl;
    LOG(WARN)
    << "The resulting index will miss many entity occurrences!" << std::endl;
    LOG(WARN)
        << "Only use this for tests that are supposed "
        << "to run quick and don't care for correct results"
        << std::endl;
    return;
  }
  // Now also read the redirect map and for each redirect with a source
  // not in available entities, check if the target has an Entity entry and
  // if yes, add a pair source->Entity.
  {
    LOG(INFO)
    << "Reading title-entity pairs from redirect-map: " << redirectMapFile
        << "." << std::endl;

    std::ifstream stream(redirectMapFile.c_str(), std::ios::in);
    std::string line;
    size_t nofLines = 0;
    // Read line-wise.
    while (std::getline(stream, line))
    {
      ++nofLines;
      size_t indexOftab = line.find('\t');
      assert(indexOftab != 0 && indexOftab != string::npos);
      string source = line.substr(0, indexOftab);
      if (_availableEntities.count(source) == 0)
      {
        string target = line.substr(indexOftab + 1);
        HashMap<string, Entity>::const_iterator it = _availableEntities.find(
            target);
        if (it != _availableEntities.end())
        {
          _availableEntities[source] = Entity(
              it->second._stringRepresentation, it->second._isPerson);
        }
      }
    }
    stream.close();
    LOG(INFO)
        << "read " << nofLines << " lines." << std::endl;
    LOG(INFO)
        << "Extended map of available entities. Now has "
        << _availableEntities.size() << " entries." << std::endl;
  }
}
// _____________________________________________________________________________
void EntityAwareWriter::readAbstractnessCounts(const string& fileName)
{
  LOG(INFO)
  << "Reading abstractness counts from" << fileName << "." << std::endl;

  std::ifstream stream(fileName.c_str(), std::ios::in);
  std::string line;
  size_t nofLines = 0;
  // Read line-wise.
  while (std::getline(stream, line))
  {
    ++nofLines;
    size_t indexOftab = line.find('\t');
    assert(indexOftab != 0 && indexOftab != string::npos);
    string eWord = line.substr(indexOftab + 1);
    std::istringstream is(line.substr(0, indexOftab));
    int count;
    is >> count;
    if (count > ABSTRACT_ENTITY_THRESHOLD)
    {
      // Add the correct string key to abstract entities.
      if (_availableEntities.size() > 0)
      {
        _abstractEntities.insert(eWord);
      }
      else
      {
        _abstractEntities.insert(
            ad_utility::getLowercase(
                ad_utility::getLastPartOfString(eWord, ':')));
      }
    }
    if (count)
    {
      string key = ad_utility::getLowercase(
          ad_utility::getLastPartOfString(eWord, ':'));
      int score = 1;
      while (count > (1 << (score - 1)))
      {
        ++score;
      }
      assert(score <= 255);
      _classScores[key] = score;
    }
  }
  stream.close();
  LOG(INFO)
  << "read " << nofLines << " lines." << std::endl;
  LOG(INFO)
  << "Identified " << _abstractEntities.size() << " abstract entities and "
      << _classScores.size() << " classes to boost." << std::endl;
}
}
