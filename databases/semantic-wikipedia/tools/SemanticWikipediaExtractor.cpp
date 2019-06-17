// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Björn Buchhold <buchholb>

#include <expat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
// #include <boost/regex.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "./SemanticWikipediaExtractor.h"

// Buffer sizes. Values chosen as in codebase/parser/XmlParser
const size_t XML_PARSER_BUFFER_SIZE = 10 * 1024 * 1024;
const size_t BUF_SIZE = XML_PARSER_BUFFER_SIZE;

// ____________________________________________________________________________
template<unsigned char MODE>
void start(void *data, const char *el, const char **attr)
{
  SemanticWikipediaExtractor<MODE>* de =
      reinterpret_cast<SemanticWikipediaExtractor<MODE>*> (data);
  de->doHandleStart(el, attr);
}

// ____________________________________________________________________________
template<unsigned char MODE>
void end(void *data, const char *el)
{
  SemanticWikipediaExtractor<MODE>* de =
      reinterpret_cast<SemanticWikipediaExtractor<MODE>*> (data);
  de->doHandleEnd(el);
}

// ____________________________________________________________________________
template<unsigned char MODE>
void handleChar(void *data, const XML_Char *s, int len)
{
  SemanticWikipediaExtractor<MODE>* de =
      reinterpret_cast<SemanticWikipediaExtractor<MODE>*> (data);
  de->doHandleCharacter(s, len);
}

// ____________________________________________________________________________
template<unsigned char MODE>
SemanticWikipediaExtractor<MODE>::SemanticWikipediaExtractor(
    const std::string& inTextPattern, const std::string& regex,
    const std::string& titlesFile)
{
  _inTextPattern = inTextPattern;
  _regexS = regex;
  _buf = new char[BUF_SIZE];
  _titleBuf = new char[BUF_SIZE];
  _pos = 0;
  _titlePos = 0;
  _titleOpen = false;

  if (regex.size() > 0)
  {
    std::cerr << "Regexp-support currently disabled! ";
    exit(1);
    std::cout << "About to build a regex from: " << _regexS << std::endl;
//    try
//    {
//      // Set up the regular expression for case-insensitivity
//      _regex.assign(_regexS, boost::regex_constants::icase);
//    } catch(const boost::regex_error& e)
//    {
//      std::cout << _regexS << " is not a valid regular expression: \""
//          << e.what() << "\"" << std::endl;
//      // Set the regex string to be empty
//      // so that the regex is ignored when matching.
//      _regexS = "";
//    }
  }
  if (titlesFile.size() > 0)
  {
    std::ifstream ts(titlesFile.c_str(), std::ios::in);
    std::string line;
    // Read line-wise.
    while (std::getline(ts, line))
    {
      // Replace possible spaces by underscores. This is also done for the
      // titles found in the base xml, later.
      std::string::iterator it = line.begin();
      while (it != line.end())
      {
        if (isspace(*it))
        {
          *it = '_';
        }
        ++it;
      }
      // Add the title to the set.
      _titles.insert(line);
    }
    ts.close();
  }
}

// ____________________________________________________________________________
template<unsigned char MODE>
SemanticWikipediaExtractor<MODE>::~SemanticWikipediaExtractor()
{
  delete[] _buf;
  delete[] _titleBuf;
}

// ____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaExtractor<MODE>::doHandleStart(const char *el,
    const char **attr)
{
  if (std::string(el) == "title")
  {
    _titleOpen = true;
    _titlePos = 0;
  }
  if (std::string(el) == "page")
  {
    // Write everything from before.
    if (_pos > 0)
    {
      _buf[_pos] = 0;
      if (std::string(_buf).find_first_not_of(" \t\n") != std::string::npos)
      {
        fprintf(_outputFile, "%s", _buf);
      }
      _pos = 0;
    }
  }
  // Write the tag to the buffer.
  // Open the tag.
  std::string elementName = el;
  _buf[_pos] = '<';
  ++_pos;
  for (size_t i = 0; i < elementName.size(); ++i)
  {
    _buf[_pos] = elementName[i];
    ++_pos;
  }

  // Write attributes
  for (int i = 0; attr[i]; i += 2)
  {
    std::ostringstream os;
    os << " " << std::string(attr[i]) << "=\"" << std::string(attr[i + 1])
        << "\"";
    std::string attribute = os.str();
    for (size_t j = 0; j < attribute.size(); ++j)
    {
      _buf[_pos] = attribute[j];
      ++_pos;
    }
  }
  // Close tag.
  _buf[_pos] = '>';
  ++_pos;
}

// ____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaExtractor<MODE>::doHandleEnd(const char *el)
{
  if (std::string(el) == "title")
  {
    _titleOpen = false;
    _titleBuf[_titlePos] = 0;
  }
  // Write the tag to the buffer.
  std::string elementName = el;
  _buf[_pos] = '<';
  ++_pos;
  _buf[_pos] = '/';
  ++_pos;
  for (size_t i = 0; i < elementName.size(); ++i)
  {
    _buf[_pos] = elementName[i];
    ++_pos;
  }
  _buf[_pos] = '>';
  ++_pos;

  // If a page ends, check if we want that page
  // or if it is supposed to be filtered.
  if (std::string(el) == "page")
  {
    _buf[_pos] = '\n';
    _buf[_pos + 1] = 0;
    std::string content;
    content.assign(_buf);
    if (shouldBeKept(content))
      fprintf(_outputFile, "%s", _buf);
    _pos = 0;
  }

  // If the mediawiki tag is closed, write the buffer, too.
  if (std::string(el) == "mediawiki")
  {
    _buf[_pos] = '\n';
    _buf[_pos + 1] = 0;
    fprintf(_outputFile, "%s", _buf);
    _pos = 0;
  }
}

// ____________________________________________________________________________
template<unsigned char MODE>
bool SemanticWikipediaExtractor<MODE>::shouldBeKept(const std::string& content)
{
  // Process title list.
  if (_titles.size() > 0)
  {
    std::string title(_titleBuf);
    std::string::iterator it = title.begin();

    // Replace white-spaces by underscores
    while (it != title.end())
    {
      if (isspace(*it))
      {
        *it = '_';
      }
      ++it;
    }

    if (_titles.count(title) == 0)
    {
      if (MODE == MATCH_ALL_MODE) return false;
    } else
    {
      if (MODE == MATCH_ANY_MODE) return true;
    }
  }

  // Process in Text pattern
  if (_inTextPattern.size() > 0)
  {
    bool match = content.find(_inTextPattern) != std::string::npos;
    if (MODE == MATCH_ALL_MODE && match == false) return false;
    if (MODE == MATCH_ANY_MODE && match == true) return true;
  }

//  // Process Regular expressions
//  if (_regexS.size() > 0)
//  {
//    bool match;
//    try
//    {
//      match = boost::regex_match(content, _regex);
//    } catch(const std::runtime_error& e)
//    {
//      std::cout << "Exception thrown when trying to match with: "
//          << content.substr(0, 50) << "... " << std::endl << std::endl
//          << " Exception was:  \"" << e.what() << "\"" << std::endl
//          << std::flush;
//      // Do not keep this page.
//      match = false;
//    }
//    if (MODE == MATCH_ALL_MODE && match == false) return false;
//    if (MODE == MATCH_ANY_MODE && match == true) return true;
//  }
  // All passes done. If this section is reached it means that we used
  // MATCH_ALL_MODE and all matched -> return true
  // or that we used MATCH_ANY_MODE and neither matched -> return false
  return (MODE == MATCH_ALL_MODE);
}

// ____________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaExtractor<MODE>::doHandleCharacter(const char* s, int len)
{
  for (int i = 0; i < len; ++i)
  {
    if (s[i] == '<')
    {
      _buf[_pos] = '&';
      ++_pos;
      _buf[_pos] = 'l';
      ++_pos;
      _buf[_pos] = 't';
      ++_pos;
      _buf[_pos] = ';';
      ++_pos;
      continue;
    }
    if (s[i] == '>')
    {
      _buf[_pos] = '&';
      ++_pos;
      _buf[_pos] = 'g';
      ++_pos;
      _buf[_pos] = 't';
      ++_pos;
      _buf[_pos] = ';';
      ++_pos;
      continue;
    }
    if (s[i] == '&')
    {
      _buf[_pos] = '&';
      ++_pos;
      _buf[_pos] = 'a';
      ++_pos;
      _buf[_pos] = 'm';
      ++_pos;
      _buf[_pos] = 'p';
      ++_pos;
      _buf[_pos] = ';';
      ++_pos;
      continue;
    }
    _buf[_pos] = s[i];
    ++_pos;
  }
  if (_titleOpen)
  {
    for (int i = 0; i < len; ++i)
    {
      _titleBuf[_titlePos] = s[i];
      ++_titlePos;
    }
  }
}

// ___________________________________________________________________________
template<unsigned char MODE>
void SemanticWikipediaExtractor<MODE>::extract(const std::string & xmlFileName,
    const std::string& outputFile)
{
  std::cout << "About to extract from " << xmlFileName << ".\n";
  _outputFile = fopen(outputFile.c_str(), "w");

  // Create Parser and register handlers.
  XML_Parser xmlParser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xmlParser, start<MODE>, end<MODE>);
  XML_SetCharacterDataHandler(xmlParser, handleChar<MODE>);
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
       stSize > XML_PARSER_BUFFER_SIZE ? xml_file_stat.st_size
          / 10 : xml_file_stat.st_size / 3;
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
    if (done)
      break;
  }

  // free buffer and close files
  delete[] xml_buffer;
  fclose(xml_file);
  fclose(_outputFile);

  // progress output
  timer = time(NULL) - timer;
  std::cerr << " extraction done" << std::flush;
  if (timer > 0)
    std::cerr << " in " << timer << " seconds (" << (xml_file_stat.st_size
        / (timer * 1000 * 1000)) << " MiB/second)" << std::flush;
}

// Template instantiation.
template class SemanticWikipediaExtractor<MATCH_ALL_MODE>;
template class SemanticWikipediaExtractor<MATCH_ANY_MODE>;
