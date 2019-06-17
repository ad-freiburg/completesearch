// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <expat.h>
#include <assert.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <string>
#include "../codebase/semantic-wikipedia-utils/CharMaps.h"
#include "./RedirectMapBuilder.h"

namespace semsearch
{
// Buffer sizes. Values chosen as in codebase/parser/XmlParser
const size_t XML_PARSER_BUFFER_SIZE = 10 * 1024 * 1024;
const size_t BUF_SIZE = XML_PARSER_BUFFER_SIZE;

// ____________________________________________________________________________
void start(void *data, const char *el, const char **attr)
{
  RedirectMapBuilder* rmb = reinterpret_cast<RedirectMapBuilder*> (data);
  rmb->doHandleStart(el, attr);
}

// ____________________________________________________________________________
void end(void *data, const char *el)
{
  RedirectMapBuilder* rmb = reinterpret_cast<RedirectMapBuilder*> (data);
  rmb->doHandleEnd(el);
}

// ____________________________________________________________________________
void handleChar(void *data, const XML_Char *s, int len)
{
  RedirectMapBuilder* rmb = reinterpret_cast<RedirectMapBuilder*> (data);
  rmb->doHandleCharacter(s, len);
}

// ____________________________________________________________________________
RedirectMapBuilder::RedirectMapBuilder(const std::string& outputFileName)
{
  _outputFile = fopen(outputFileName.c_str(), "w");
  _titleBuf = new char[BUF_SIZE];
  _redirectsToBuf = new char[BUF_SIZE];
  _noLinkYet = false;
  _titleOpen = false;
  _freshText = false;
  _inTheMiddeOfText = false;
  _targetPos = 0;
  _titlePos = 0;
}

// ____________________________________________________________________________
RedirectMapBuilder::~RedirectMapBuilder()
{
  delete[] _titleBuf;
  delete[] _redirectsToBuf;
  fclose(_outputFile);
}

// ____________________________________________________________________________
void RedirectMapBuilder::process(const std::string& xmlFileName)
{
  std::cout << "About to build the redirect-map from " << xmlFileName << ".\n";

  // Create Parser and register handlers.
  XML_Parser xmlParser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xmlParser, start, end);
  XML_SetCharacterDataHandler(xmlParser, handleChar);
  XML_SetUserData(xmlParser, reinterpret_cast<void*> (this));

  // Main parse loop. Mostly taken from codebase/parser/XmlParser.
  // open file and allocate buffer
  FILE* xml_file = fopen(xmlFileName.c_str(), "r");
  if (xml_file == NULL)
  {
    std::cerr << "fopen \"" << xmlFileName << "\": " << std::endl;
    exit(1);
  }
  char* xml_buffer = new char[XML_PARSER_BUFFER_SIZE];

  // progress output
  std::cerr << "parsing \"" << xmlFileName << "\" ";
  struct stat xml_file_stat;
  stat(xmlFileName.c_str(), &xml_file_stat);
  size_t stSize = xml_file_stat.st_size;
  off_t progress_step =
      stSize > XML_PARSER_BUFFER_SIZE ? xml_file_stat.st_size / 10
          : xml_file_stat.st_size / 3;
  off_t progress = progress_step;
  off_t timer = time(NULL);

  // actual parsing loop
  while (true)
  {
    size_t len = fread(xml_buffer, 1, XML_PARSER_BUFFER_SIZE, xml_file);
    while (progress <= ftello(xml_file))
    {
      std::cerr << "." << std::flush;
      progress += progress_step;
    }
    bool done = len < BUF_SIZE;
    int ret = XML_Parse(xmlParser, xml_buffer, len, done);
    if (ret == XML_STATUS_ERROR)
    {
      std::cerr << "ERROR in parse of \"" << xmlFileName << "\n";
      exit(1);
    }
    if (done) break;
  }

  // free buffer and close file
  delete[] xml_buffer;
  fclose(xml_file);

  // progress output
  timer = time(NULL) - timer;
  std::cout << std::endl << std::endl
      << " Extracting redirect information done"
      << std::endl << std::flush;
  if (timer > 0) std::cout << " in " << timer << " seconds ("
      << (xml_file_stat.st_size / (timer * 1000 * 1000)) << " MiB/second)"
      << std::flush;

  // Write the map to file
  std::cout
      << "Now resolving chained redirects and writing the map to file..."
      << std::flush;

  for (ad_utility::HashMap<std::string, std::string>::iterator it =
      _redirectMap.begin(); it != _redirectMap.end(); ++it)
  {
    // Use a reference and actually change the target in the map for future use.
    string& target = it->second;
    ad_utility::HashMap<std::string, std::string>::const_iterator itt;
    // Take care of cycles
    int depth = 1;
    itt = _redirectMap.find(target);
    while (itt != _redirectMap.end() && target != itt->second)
    {
      assert(depth < 10);
      ++depth;
      target = itt->second;
      itt = _redirectMap.find(target);
    }
    // Remove leading underscores from target
    std::ostringstream cleanTarget;
    bool seenSomething = false;
    for (size_t i = 0; i < target.size(); ++i)
    {
      if (target[i] != '_')
      {
        seenSomething = true;
        cleanTarget << target[i];
      }
      else
      {
        if (seenSomething) cleanTarget << target[i];
      }
    }
    // Remove trailing underscores
    string clean = cleanTarget.str().substr(0,
        cleanTarget.str().find_last_not_of('_') + 1);
    if (clean.size() > 0)
    fprintf(_outputFile, "%s\t%s\n", it->first.c_str(), clean.c_str());
  }
  std::cout << " done." << std::endl;
}
// ____________________________________________________________________________
void RedirectMapBuilder::doHandleStart(const char *el, const char **attr)
{
  if (std::string(el) == "title")
  {
    _titleOpen = true;
    _titlePos = 0;
  }
  if (std::string(el) == "text")
  {
    _freshText = true;
    _noTextYet = true;
  }
}

// ____________________________________________________________________________
void RedirectMapBuilder::doHandleEnd(const char *el)
{
  if (std::string(el) == "title")
  {
    _titleOpen = false;
    _titleBuf[_titlePos] = 0;
    _currentTitle = std::string(_titleBuf);
  }
  // Handle existing but empty text tags
  if (std::string(el) == "text")
  {
    _freshText = false;
    // Output an error message if the builder still expected to find the target
    if (_inTheMiddeOfText || _noLinkYet)
    {
      std::cerr << "Unable to find the redirect target for " << _currentTitle
          << std::endl << std::flush;
      _inTheMiddeOfText = false;
      _noLinkYet = false;
    }
    // Now also write a redirect from the lowercase version of
    // the title to the redirect target if there is one
    // and the the title itself if there is no target
    std::string lowercaseTitle;
    std::string lowercaseWithUnderscores;

    bool remarkableUpperCase = false;
    for (size_t i = 0; i < _currentTitle.length(); ++i)
    {
      if (isupper(_currentTitle[i]) && i > 0
          && ad_utility::W_CHAR_MAP[static_cast<int> (_currentTitle[i - 1])]
              == 'w')
      {
        remarkableUpperCase = true;
      }
      if (isspace(_currentTitle[i]))
      {
        lowercaseWithUnderscores += '_';
      }
      else
      {
        lowercaseWithUnderscores += tolower(_currentTitle[i]);
      }
      lowercaseTitle += tolower(_currentTitle[i]);
    }
    std::string target;
    if (_redirectsToBuf[0] != 0)
    {
      target = _redirectsToBuf;
    }
    else
    {
      assert(_currentTitle.size() > 0);
      target += static_cast<char> (toupper(_currentTitle[0]));
      for (size_t i = 1; i < _currentTitle.size(); ++i)
      {
        target += (isspace(_currentTitle[i]) ? '_' : _currentTitle[i]);
      }
    }
    target[0] = static_cast<char> (toupper(target[0]));

    _redirectsToBuf[0] = 0;

    if (!remarkableUpperCase)
    {
//    fprintf(_outputFile, "%s\t%s\n", lowercaseTitle.c_str(), target.c_str());
//    fprintf(_outputFile, "%s\t%s\n", lowercaseWithUnderscores.c_str(),
//        target.c_str());
      string& existingTarget1 = _redirectMap[lowercaseTitle];
      if (existingTarget1 == "")
      {
        existingTarget1 = target;
      }
      string& existingTarget2 = _redirectMap[lowercaseWithUnderscores];
      if (existingTarget2 == "")
      {
        existingTarget2 = target;
      }
    }
  }
}
// ____________________________________________________________________________
void RedirectMapBuilder::doHandleCharacter(const char* s, int len)
{
  // Collect the titles.
  if (_titleOpen)
  {
    for (int i = 0; i < len; ++i)
    {
      _titleBuf[_titlePos] = s[i];
      _titlePos++;
    }
  }

  // Start of a new text tag's content ignoring spaces.
  if (_freshText)
  {
    // Go through the buffer's content.
    for (int i = 0; i < len; ++i)
    {
      // Distinguish the three cases depending on what is expected.
      // 1. looking for #redirect, 2. looking for [[, 3. looking for ] or #

      // Case 1 - looking for #redirect.
      if (_noTextYet)
      {
        // Ignore any kind of spaces
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n')
        {
          // Check if the text starts with #REDIRECT or #redirect
          if (len < i + 8)
          {
            return;
          }
          char textStart[10];

          for (int pos = 0; pos < 9; pos++)
          {
            textStart[pos] = tolower(s[i]);
            i++;
          }
          textStart[9] = 0;

          std::string textStartString(textStart);

          if (textStartString == "#redirect")
          {
            _noTextYet = false;
            _noLinkYet = true;
          }
          else
          {
            _noTextYet = false;
            _freshText = false;
          }
        }
      }

      // Case 2 - looking for [[
      if (_noLinkYet)
      {
        // Go through the buffer and ignore anything until [[ is discoverd.
        // Ignore any kind of spaces
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n')
        {
          // Take care of buffer's bounds.
          if (len < i + 2)
          {
            return;
          }
          if (s[i] == '[' && s[i + 1] == '[')
          {
            s += 2;
            _noLinkYet = false;
            _inTheMiddeOfText = true;
            _targetPos = 0;
          }
        }
      }

      // Case 3 - looking for the end
      if (_inTheMiddeOfText)
      {
        while (s[i] != ']' && s[i] != '#')
        {
          if (isspace(s[i]))
          {
            _redirectsToBuf[_targetPos] = '_';
          }
          else
          {
            if (_targetPos == 0)
            {
              _redirectsToBuf[_targetPos] = static_cast<char>(toupper(s[i]));
            }
            else
            {
              _redirectsToBuf[_targetPos] = s[i];
            }
          }
          _targetPos++;
          i++;
          // Handle case where target title is split over many buffer calls.
          if (i >= len)
          {
            return;
          }
        }
        // Now we're at the end with the target in the buffer. Finish
        // the string in the buffer with a 0 and write the redirect tuple
        // to the output.
        _inTheMiddeOfText = false;
        _redirectsToBuf[_targetPos] = 0;
        if (_currentTitle.size() == 0)
        {
          std::cerr << "got no title for redirect " << std::string(
              _redirectsToBuf) << std::endl << "skipping entry..."
              << std::endl << std::flush;
          return;
        }
        if (std::string(_redirectsToBuf).size() == 0)
        {
          std::cerr << "got no target for title " << _currentTitle
              << std::endl << "skipping entry..." << std::endl << std::flush;
        }
        // Write both, the current title with spaces, and the current title
        // with underscores
//        fprintf(_outputFile, "%s\t%s\n", _currentTitle.c_str(),
//            _redirectsToBuf);
        string& existingTarget1 = _redirectMap[_currentTitle];
        if (existingTarget1 == "")
        {
          existingTarget1 = _redirectsToBuf;
        }

        std::string currentTitleUnderscores(_currentTitle);
        for (std::string::iterator it = currentTitleUnderscores.begin(); it
            != currentTitleUnderscores.end(); ++it)
        {
          if (*it == ' ') *it = '_';
        }
        if (currentTitleUnderscores != _currentTitle)
        {
//          fprintf(_outputFile, "%s\t%s\n", currentTitleUnderscores.c_str(),
//              _redirectsToBuf);
          string& existingTarget2 = _redirectMap[currentTitleUnderscores];
          if (existingTarget2 == "")
          {
            existingTarget2 = _redirectsToBuf;
          }
        }
      }
    }
  }
}
}
