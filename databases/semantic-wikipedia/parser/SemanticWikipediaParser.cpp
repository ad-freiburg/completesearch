// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Hannah Bast <bast>, Björn Buchhold <buchholb>

#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include "./SemanticWikipediaParser.h"
#include "./EntityAwareWriter.h"

using ad_utility::S_CHAR_MAP;
using ad_utility::W_CHAR_MAP;
using ad_utility::DOCS_W_CHAR_MAP;

namespace ad_semsearch
{
// ____________________________________________________________________________
template<unsigned char MODE>
SemanticWikipediaParser<MODE>::SemanticWikipediaParser(
    std::string::size_type minWordLength, size_t maxHeaderLevel) :
    XmlParser(), _writer(minWordLength, maxHeaderLevel)
{
  _usesRelations = false;
  _outputCategories = false;
  if (MODE == OUTPUT_SPECIAL_CHARS)
  {
    _writer.setPrechunkMode(true);
  }
}
// ____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaParser<MODE>::setSectionHeadersToSkip(
    const string& sectionHeadersToSkipFileName)
{
  // Read section headers to skip from the file.
  if (sectionHeadersToSkipFileName.size() > 0)
  {
    std::ifstream secFile(sectionHeadersToSkipFileName.c_str(), std::ios::in);
    std::string section;
    while (std::getline(secFile, section))
    {
      _sectionHeadersToSkip.insert(ad_utility::getNormalizedLowercase(section));
    }
    secFile.close();
    std::cout << "read " << _sectionHeadersToSkip.size()
        << " section headers from file \"" << sectionHeadersToSkipFileName
        << "\"" << std::endl;
  }
}
// ____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaParser<MODE>::outputDocAndWords()
{
  // Skip anything that is no page item.
  if (strcmp(getItemName(), "page") != 0
      && strcmp(getItemName(), "document") != 0)
  {
    fprintf(_log_file, "SKIPPED ITEM with name \"%s\"\n", getItemName());
    return;
  }
  // Get title text from the XML.
  std::string text = getItem("text");
  std::string title = getItem("title");
  std::string url = getItem("link", "");
  std::string path = getItem("path", "");
  if (url == "" && path != "") url = string(
      "http://etna.informatik.uni-freiburg.de/zbmed1622/") + path;

  // Skip pages titled Wikipedia.
  if (title.substr(0, 10) == "Wikipedia:")
  {
    fprintf(_log_file, "SKIPPED SPECIAL PAGE with title \"%s\"\n",
        title.c_str());
    return;
  }

  // Skip Temple pages.
  if (title.substr(0, 9) == "Template:")
  {
    fprintf(_log_file, "SKIPPED SPECIAL PAGE with title \"%s\"\n",
        title.c_str());
    return;
  }

  // Skip redirect pages
  if (text.substr(0, 10) == "#REDIRECT ")
  {
    fprintf(_log_file, "SKIPPED REDIRECT PAGE with text \"%s\"\n",
        text.c_str());
    return;
  }

  // Skip empty pages
  if (text.length() == 0)
  {
    fprintf(_log_file, "SKIPPED EMPTY PAGE with title \"%s\"\n", title.c_str());
    return;
  }

  // The page should be meaningful. Parse it now.
  std::string wordsFileOutput;
  std::string docsFileOutput;

  // Get the doc Id.
  //  int docId = atoi(getItem("id"));

  // Parse the text part of the page.
  parseText(text, title, url, &wordsFileOutput, &docsFileOutput);

  // Write output to the files.
  fprintf(_words_file, "%s", wordsFileOutput.c_str());
  fprintf(_docs_file, "%s", docsFileOutput.c_str());
}

// ____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaParser<MODE>::parseText(const string& text,
    const string& title, const string& url, string* wordsFileOutput,
    string* docsFileOutput)
{
  assert(wordsFileOutput != NULL);
  assert(docsFileOutput != NULL);

  std::string titleNoSpace = title.substr(0, title.length());
  for (std::string::size_type i = 0; i < titleNoSpace.length(); ++i)
  {
    if (isspace(titleNoSpace[i])) titleNoSpace[i] = '_';
  }

  _writer.setUrl(url == "" ?
      std::string("http://en.wikipedia.org/wiki/") + titleNoSpace : url);
  _writer.setTitle(titleNoSpace);
  _writer.setDocument(title);
  _writer.newSentence();

  // The following assertions ensure that the outputDocAndWords method
  // passes the desired parts as text
  assert(text.length() > 0);

  bool inListItem = false;
  bool inTable = false;

  // Iterate over the whole text and look at every character only once.
  std::ostringstream wordOs;
  for (std::string::size_type i = 0; i < text.length(); ++i)
  {
    // A "normal" word character is appended to the current word.
    // Uses continue because further cases do not have to be checked.
    if (isWordChar(text[i]))
    {
      // Handle Apostroph's as formatting chars
      if (text[i] == '\'' && i+1 < text.size() && text[i+1] == '\'')
      {
        i = text.find_first_not_of('\'', i) - 1;
        continue;
      }
      wordOs << text[i];
      continue;
    }
    if (isWordSeparator(text[i]))
    {
      // A separator character indicates the last word ended.
      // It can be written to the output.

      // Check if there already is something in the word buffer.
      // If so, write the last word to the output.
      std::string word = wordOs.str();
      _writer.writeWord(word);
      wordOs.str("");
      if (MODE != OUTPUT_SPECIAL_CHARS
          && ad_utility::SPEC_CHAR_MAP[static_cast<uint8_t> (text[i])] == 'y'
          && !inTable)
      {
        _writer.writeAdditionalCharsToDocsFile(text[i]);
      }

      // Output COMMATA etc if wanted
      if (MODE == OUTPUT_SPECIAL_CHARS)
      {
        if (ad_utility::SPEC_CHAR_MAP[static_cast<uint8_t>(text[i])] == 'y')
        {
          char asString[2] = {text[i], 0};
          _writer.writeWord(asString);
        }
      }

      // Check if a sentence ends, a list item starts or
      // a list item is ended by a newline.
      if (text[i] == '.' || text[i] == '?' || (!inTable && text[i] == '!'))
      {
        _writer.newSentence();
      }
      if (W_CHAR_MAP[static_cast<uint8_t> (text[i])] == 'l'
          && W_CHAR_MAP[static_cast<uint8_t> (text[i + 1])] != 'l' && text[i
          + 1] != ':')
      {
        inListItem = true;
        _writer.newSentence();
      }

      // Identify the end of a list.
      if (inListItem && text[i] == '\n'
          && W_CHAR_MAP[static_cast<uint8_t> (text[i + 1])] != 'l'
          && W_CHAR_MAP[static_cast<uint8_t> (text[i + 2])] != 'l')
      {
        inListItem = false;
        _writer.newSentence();
      }
    }

    // NEW 18Oct2010: also check if a table row starts or a table row
    // is ended.

    // A table starts.
    if (text[i] == '{' && text[i + 1] == '|')
    {
      inTable = true;
      size_t newpos = text.find('\n', i);
      if (newpos > i && newpos != string::npos)
      {
        i = newpos;
      }
    }
    // Inside a table, a new row is recognized
    if (inTable && text[i] == '|' && text[i + 1] == '-')
    {
      _writer.newSentence();
      i++;
    }
    // A table ends.
    if (inTable && text[i] == '|' && text[i + 1] == '}')
    {
      inTable = false;
      i++;
      _writer.newSentence();
    }
    std::string::size_type newPos;

    // Handle links.
    if (text[i] == '[')
    {
      // Take care that no overflow happens
      newPos = handleLink(text, i);
      if (newPos >= text.length())
      {
        newPos = text.length() - 1;
      }
      i = newPos;
    }

    // Handle beginnings of sections
    if (sectionStarts(text, i))
    {
      newPos = handleSection(text, i);
      if (newPos >= text.length())
      {
        newPos = text.length() - 1;
      }
      i = newPos;
    }

    // Handle everything starting with a '<' character.
    if (text[i] == '<')
    {
      // Skip comments.
      if (commentStarts(text, i))
      {
        newPos = i + (commentSize(text, i) - 1);
        if (newPos > i && newPos < text.length())
        {
          i = newPos;
        }
      }

      // Skip reference-tags and their content.
      if (refStarts(text, i))
      {
        newPos = i + (refSize(text, i) - 1);
        if (newPos > i && newPos < text.length())
        {
          i = newPos;
        }
      }

      // Skip math.
      if (mathStarts(text, i))
      {
        newPos = i + (mathSize(text, i) - 1);
        if (newPos > i && newPos < text.length())
        {
          i = newPos;
        }
      }

      // Skip formatting tags themselves.
      // that means skip <blockquote> e.g. but keep the content.
      if (W_CHAR_MAP[static_cast<uint8_t> (text[i])] == '<')
      {
        newPos = i + (tagSize(text, i) - 1);
        if (newPos > i && newPos < text.length())
        {
          i = newPos;
        }
      }
    }

    // Ignore wikipedia templates.
    if (templateStarts(text, i))
    {
      newPos = i + (templateSize(text, i) - 1);
      if (newPos > i && newPos < text.length())
      {
        i = newPos;
      }
    }

    // Ignore wikipedia templates - even if they use strange markup.
    if (text[i] == '{' && !templateStarts(text, i))
    {
      newPos = text.find('}', i);
      if (newPos > i && newPos < text.length())
      {
        i = newPos;
      }
    }

    // Ignore html entities
    if (text[i] == '&')
    {
      if (text.substr(i, 5) == "&nbsp" || text.substr(i, 6) == "&ndash")
      {
        while (i < text.size() && !isWordSeparator(text[i]))
          ++i;
      }
    }
  }

  // Finish with writing the very last word if there is still something
  std::string word = wordOs.str();
  _writer.writeWord(word);

  if (MODE != OUTPUT_SPECIAL_CHARS) _writer.writeAdditionalCharsToDocsFile(
      '\n');
  // Fill output string.
  _writer.finish(wordsFileOutput, docsFileOutput);
}

// ____________________________________________________________________________
template<unsigned char MODE>
bool SemanticWikipediaParser<MODE>::commentStarts(const string& text,
    const std::string::size_type& pos)
{
  return text[pos] == '<' && text.substr(pos + 1, 3) == "!--";
}

// ____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::commentSize(const string& text,
    const std::string::size_type& pos)
{
  std::string::size_type found = text.find("-->", pos);
  if (found == text.npos)
  {
    return 1;
  }
  return found - pos + 3;
}
// _____________________________________________________________________________
template<unsigned char MODE>
bool SemanticWikipediaParser<MODE>::templateStarts(const string& text,
    const std::string::size_type& pos)
{
  return text[pos] == '{' && text[pos + 1] == '{';
}

// _____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::templateSize(const string& text,
    const std::string::size_type& pos)
{
  int templateNestingLevel = 1;
  size_t i = pos + 2;
  while (templateNestingLevel > 0 && i < text.size() - 1)
  {
    if (text[i] == '{' && text[i + 1] == '{') templateNestingLevel++;
    if (text[i] == '}' && text[i + 1] == '}') templateNestingLevel--;
    ++i;
  }
  if (templateNestingLevel > 0)
  {
    // CASE: couldn't find a proper end of the template
    return 1;
  }
  // CASE: found the end. Nesting level is 0 now, i is at the position of the
  // second closing '}', because it has been incremented once more.
  return (i - pos) + 1;
}
// _____________________________________________________________________________
template<unsigned char MODE>
bool SemanticWikipediaParser<MODE>::mathStarts(const string& text,
    const std::string::size_type& pos)
{
  return text[pos] == '<' && text.substr(pos + 1, 5) == "math>";
}

// _____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::mathSize(const string& text,
    const std::string::size_type& pos)
{
  // Assertion removed due to efficiency.
  // assert(mathStarts(text, pos));
  std::string::size_type found = text.find("</math>", pos);
  if (found == text.npos)
  {
    return 1;
  }
  return found - pos + 7;
}
// _____________________________________________________________________________
template<unsigned char MODE>
bool SemanticWikipediaParser<MODE>::refStarts(const string& text,
    const std::string::size_type& pos)
{
  return text[pos] == '<' && text.substr(pos + 1, 3) == "ref";
}

// _____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::refSize(const string& text,
    const std::string::size_type& pos)
{
  // reference can be the following: <ref ...> xyz </ref> or <ref .../>
  std::string::size_type i = pos + 4;
  while (i < text.length())
  {
    // Go over the text and look for the first appropriate end
    if (text[i] == '>' && (text[i - 1] == '/' || (text[i - 1] == 'f' && text[i
        - 2] == 'e' && text[i - 3] == 'r' && text[i - 4] == '/')))
    {
      return i - pos + 1;
    }
    i++;
  }
  // no proper end found. ignoring ref tag so far
  return 4;
}
// ____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::tagSize(const string& text,
    const std::string::size_type& pos)
{
  std::string::size_type found = text.find('>', pos);
  if (found == text.npos)
  {
    return 1;
  }
  return found - pos + 1;
}
// ____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::handleLink(const string& text,
    const std::string::size_type& pos)
{
  // Check if it is some kind of internal link.
  if (text[pos + 1] == '[')
  {
    // Deal with internal links
    std::string::size_type closingPos = text.find("]]", pos);
    std::string::size_type dashPos = text.find('|', pos);
    std::string::size_type colonPos = text.find(':', pos);

    // Ignore the link opening if it is not ended properly.
    if (closingPos == text.npos)
    {
      return pos;
    }

    // Check if it is a file and if so, skip it
    if (text.substr(pos + 2, 5) == "file:")
    {
      return closingPos + 1;
    }

    // Case: normal internal link to an entity's page.
    if ((dashPos == text.npos || dashPos > closingPos) && (colonPos
        == text.npos || colonPos > closingPos))
    {
      std::string entity = text.substr(pos + 2, closingPos - (pos + 2));
      size_t end = entity.find_last_not_of(" ,");
      entity = entity.substr(0, end + 1);
      if (entity.size() < MAX_ENTITY_LENGTH && entity.size() > 0)
      {
        _writer.writeEntity(entity, true, true);
        _writer.incPosition();
      }
      return closingPos + 1;
    }

    // Case: internal link with some kind of renaming.
    if ((dashPos != text.npos && dashPos < closingPos) && (colonPos
        == text.npos || colonPos > closingPos))
    {
      // Case: normal renaming
      if (dashPos + 1 != closingPos)
      {
        // Write entity but no words.
        std::string entity = text.substr(pos + 2, dashPos - (pos + 2));
        size_t end = entity.find_last_not_of(" ,");
        entity = entity.substr(0, end + 1);
        if (entity.size() < MAX_ENTITY_LENGTH && entity.size() > 0)
        {
          _writer.writeEntityWithRenaming(entity,
              text.substr(dashPos + 1, closingPos - (dashPos + 1)));
          _writer.incPosition();
        }
        return closingPos;
      }
      // Case: Hide parts of entity name
      else
      {
        // Ignore formatting chars and treat like normal internal link
        std::string entity = text.substr(pos + 2, dashPos - (pos + 2));
        size_t end = entity.find_last_not_of(" ,");
        entity = entity.substr(0, end + 1);
        if (entity.size() < MAX_ENTITY_LENGTH && entity.size() > 2)
        {
          _writer.writeEntityWithHiding(entity);
          _writer.incPosition();
        }
        return closingPos;
      }
    }

    // Case: internal link to some special construct
    if (colonPos != text.npos && colonPos < closingPos)
    {
      // Determine proper ending pos for category links.
      int endPos;
      (dashPos != text.npos && dashPos < closingPos) ? endPos = dashPos
          : endPos = closingPos;

      // Case: Category definition
      if (text.substr(pos + 2, 9) == "Category:")
      {
        if (_outputCategories)
        {
          _writer.writeCategory(text.substr(pos + 11, endPos - (pos + 11)));
        }
        return endPos;
      }
      // Case: Category link
      if (text.substr(pos + 2, 10) == ":Category:")
      {
        if (_outputCategories)
        {
          //  _writer.writeCategory(text.substr(pos + 12, endPos - (pos + 12)));
        }
        // Write the displayed link text
        return pos + 2;
      }
      // otherwise simply ignore all those special constructs like
      // Wikipedia:, media:, Image: etc
      return closingPos + 1;
    }
  }
  else
  {
    // Deal with external links. I.e. ignore anything and just use
    // the displayed text if there is any.

    // Get positions of the space indicating the start of displayed text
    // and closing bracket indicating the end of the link.
    std::string::size_type spacePos = text.find(' ', pos);
    std::string::size_type closePos = text.find(']', pos);

    // Ignore the link if it is not ended properly
    if (closePos == text.npos)
    {
      return pos;
    }

    // Check if there is a displayed text
    if (spacePos != text.npos && spacePos < closePos)
    {
      // Skip the link itself.
      // The closing bracket will be ignored later on
      return spacePos;
    }
    else
    {
      // Otherwise ignore the whole link
      return closePos;
    }
  }
  return pos;
}
// ____________________________________________________________________________
template<unsigned char MODE>
bool SemanticWikipediaParser<MODE>::sectionStarts(const string& text,
    const std::string::size_type& pos)
{
  return text[pos] == '=' && text[pos + 1] == '=';
}
// ____________________________________________________________________________
template<unsigned char MODE>
int SemanticWikipediaParser<MODE>::handleSection(const string& text,
    const std::string::size_type& pos)
{
  // Determine Section Level
  int level = 1;
  while (text[pos + level] == '=')
  {
    level++;
  }

  // Find end of Headline
  // Generate the string to look for
  std::ostringstream oss;
  for (int i = 0; i < level; i++)
  {
    oss << '=';
  }
  std::string sectionEndStr = oss.str();
  std::string::size_type endIndex = text.find(sectionEndStr, pos + 1);
  std::string::size_type newLine = text.find("\n", pos + 1);
  // If not meaningful, discard this start-point
  if (endIndex == text.npos || (newLine != text.npos && endIndex > newLine))
  {
    // Do not handle this section. Maybe the function is called again and
    // will then check if the section heading is meaningful if it starts
    // at the next pos.
    return pos;
  }

  // Get the headline String
  std::string headlineUnclean = text.substr(pos + level,
      endIndex - (pos + level));
  std::string headline = ad_utility::getNormalizedLowercase(headlineUnclean);

  // Check if the Section should be skipped
  if (_sectionHeadersToSkip.count(headline) > 0)
  {
    // Find point of return after the section. This should be a new start
    // of another section of the same or lower level, or the end of this page.

    // Go through the text.
    std::string::size_type i = endIndex + level;
    while (i < text.length())
    {
      if (text[i] == '=' && text[i - 1] != '=' && (i + 1) < text.size()
          && text[i + 1] == '=')
      {
        int l = 1;
        while (text[i + l] == '=')
        {
          l++;
        }
        if (l <= level)
        {
          return i - (level - 1);
        }
      }
      i++;
    }
    return text.length() - 1;
  }

  // Do the following if the section is not skipped.
  // Always empty, useless anyway and error-prone on top!
  // Removed code to fill it.
  std::vector<std::string> associatedEntities;
  int i = endIndex + level;
  int posToReturn = i - 1;

  // Write the headline as words and)
  // inform the writer of the discovered section and its level
  // as well as discovered entity links.
  _writer.writeSectionHead(headlineUnclean, level, associatedEntities);
  _writer.newSentence();

  return posToReturn;
}
// _____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaParser<MODE>::writeFinish(
    const string& wordsFileAppendixName, const string& docsFileAppendixName,
    const string& yagoPathsFileName)
{
  FILE* wordsFile = fopen(wordsFileAppendixName.c_str(), "w");
  FILE* docsFile = fopen(docsFileAppendixName.c_str(), "w");

  // Write the relations
  if (_usesRelations)
  {
    _writer.writeEntityContexts(wordsFile, docsFile);
  }

  // Write the translation words to the index.
  std::ifstream yaStream(yagoPathsFileName.c_str(), std::ios::in);
  std::string line;
  // Read line-wise.
  std::string lastEntity;
  std::string lastId;
  while (std::getline(yaStream, line))
  {
    // Separate entities, ID and path by tabs.
    // The input file look like this:
    // entity<tab>id<tab>path
    std::string::size_type indextofTab = line.find('\t');
    assert(indextofTab != std::string::npos);
    std::string::size_type indextofTab2 = line.find('\t', indextofTab + 1);
    assert(indextofTab2 != std::string::npos);
    std::string entity = line.substr(0, indextofTab);
    std::string id = line.substr(indextofTab + 1,
        indextofTab2 - (indextofTab + 1));
    std::string path = line.substr(indextofTab2 + 1);
    if (entity == lastEntity && id == lastId) continue;
    lastEntity = entity;
    lastId = id;
    // Write the translate posting to the index
    fprintf(wordsFile, ":t:%s::e:%s:\t1\t%d\t1\n", entity.c_str(),
        path.c_str(), _writer.getClassScoresNonConst()[entity]);
  }
  yaStream.close();
  fclose(wordsFile);
}
// _____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaParser<MODE>::setYagoRelationsFileName(
    const string& yagoRelationsFileName)
{
  _usesRelations = true;
  _writer.setYagoRelationsFileName(yagoRelationsFileName);
}

// Instantiate templates
template class SemanticWikipediaParser<OUTPUT_SPECIAL_CHARS>;
template class SemanticWikipediaParser<STANDARD>;
}
