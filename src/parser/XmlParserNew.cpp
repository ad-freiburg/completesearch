// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#include "parser/XmlParserNew.h"

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using std::cout;
using std::cerr;
using std::string;
using std::pair;
using std::vector;
using std::flush;
using std::endl;
using std::ostringstream;

bool XP_REPLACE_WHITESPACE = true;
bool XP_OUTPUT_COLONCOLON_WORDS = false;
char XP_INDEXED_WORD_MARKER = 0;
bool XP_NORMALIZE_WORDS = false;
const char* XP_ENTITY_PREFIX = NULL;

// Buffer sizes.
const size_t XML_PARSER_BUFFER_SIZE = 10*1024*1024;  // 10 MB
const size_t BUF_SIZE = XML_PARSER_BUFFER_SIZE;  // 1024*1024;

// For each ASCII code say whether the corresponding character is a word
// character (1), or not (0), or a character opening a tag (2).
// TODO(bast): is the code 2 really needed anywhere?
const char IS_WORD_CHAR[257] =
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~x   // NOLINT
  "00000000000000000000000000000000000000000000000011111111110020000111111111111111111111111110000001111111111111111111111111100000"  // NOLINT
  "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"; // NOLINT

// Expat function. TODO(bast): why are they global?
static void XMLCALL startElementGlobal(void* user_data,
                                       const char* name,
                                       const char** atts);
static void XMLCALL endElementGlobal(void* user_data,
                                     const char* name);
static void XMLCALL characterDataHandlerGlobal(void* user_data,
                                               const XML_Char* s, int len);
static int XMLCALL externalEntityRefHandlerGlobal(XML_Parser parser,
                                                  const XML_Char *context,
                                                  const XML_Char *base,
                                                  const XML_Char *systemId,
                                                  const XML_Char *publicId);

// _____________________________________________________________________________
XmlParserNew::XmlParserNew()
{
  cur = new char[XML_PARSER_BUFFER_SIZE];
}

// _____________________________________________________________________________
XmlParserNew::~XmlParserNew()
{
  delete[] cur;
}

// ____________________________________________________________________________
void XmlParserNew::printUsage() const
{
  // NOTE(bast): Anything XML parser specific should come before the parse base
  // usage info.
  ParserBase::printUsage();
}

// _____________________________________________________________________________
void XmlParserNew::parseCommandLineOptions(int argc, char** argv)
{
  // Reset opterr and optind, because there might already have been some option
  // parsing before (by a parser derived from XmlParserNew, see
  // XmlParserNewExampleMain.cpp for an example).
  opterr = 0;
  optind = 1;
  // NOTE(bast): Any XML parser specific options should be parsed here. See
  // CsvParser::parseCommandLineOptions for how it's done there.
  ParserBase::parseCommandLineOptions(argc, argv);
  // TODO(bast): this check should go in ParserBase::parseCommandLineOptions.
  // Note that we have the same check right now in
  // CsvParser::parseCommandLineOptions.
  if (ParserBase::_fileNameBase == "")
  {
    cerr << "! No base name specified, use --base-name" << endl << endl;
    printUsage();
    cout << endl;
    exit(1);
  }
  _xmlFileName = ParserBase::_fileNameBase + ".xml";
}

// _____________________________________________________________________________
void XmlParserNew::parse()
{
  // Open output files and optionally read vocabulary, synonyn groups, fuzzy
  // search clusters, etc.).
  init(_fileNameBase);

  // Init (was in Item constructor).
  pos = 0;
  _docId = 0;
  _pos = 0;
  reset(0);

  // Parse xml file.
  XML_Parser xmlParser = XML_ParserCreate(NULL);
  XML_SetElementHandler(xmlParser, startElementGlobal, endElementGlobal);
  XML_SetCharacterDataHandler(xmlParser, characterDataHandlerGlobal);
  XML_SetUserData(xmlParser, reinterpret_cast<void*>(this));
  XML_SetExternalEntityRefHandler(xmlParser, externalEntityRefHandlerGlobal);
  XML_SetParamEntityParsing(xmlParser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  parseXml(xmlParser, _xmlFileName);
  cerr << ", found " << _docId << " items" << endl;
  XML_ParserFree(xmlParser);

  addGlobalInformationToWordsFile();
  // Close output files and optionall write the vocabulary.
  ParserBase::done();
}

// Parse xml file (calls itself recursively in case of includes).
void XmlParserNew::parseXml(XML_Parser xml_parser, string xmlFileName)
{
  // Open file and allocate buffer.
  FILE* xml_file = fopen(xmlFileName.c_str(), "r");
  if (xml_file == NULL)
  {
    cerr << "fopen \"" << xmlFileName << "\": " << strerror(errno) << endl;
    exit(1);
  }
  char* xml_buffer = new char[XML_PARSER_BUFFER_SIZE];

  // Progress output.
  cerr << "parsing \"" << xmlFileName << "\" " << flush;
  struct stat xml_file_stat;
  stat(xmlFileName.c_str(), &xml_file_stat);
  off_t progress_step =
    static_cast<size_t>(xml_file_stat.st_size) > XML_PARSER_BUFFER_SIZE
    ? xml_file_stat.st_size / 10 : xml_file_stat.st_size / 3;
  off_t progress = progress_step;
  off_t timer = time(NULL);

  // Actual parsing loop.
  while (true)
  {
    size_t len = fread(xml_buffer, 1, XML_PARSER_BUFFER_SIZE, xml_file);
    while (progress <= ftello(xml_file))
    {
      cerr << "." << flush;
      progress += progress_step;
    }
    bool done = len < BUF_SIZE;
    int ret = XML_Parse(xml_parser, xml_buffer, len, done);
    if (ret == XML_STATUS_ERROR)
    {
      cerr << "ERROR in parse of \"" << xmlFileName << "\": "
           << XML_ErrorString(XML_GetErrorCode(xml_parser))
           << " at line " << XML_GetCurrentLineNumber(xml_parser)
           << ", column " << XML_GetCurrentColumnNumber(xml_parser) << endl;
      exit(1);
    }
    if (done) break;
  }

  // Free buffer and close file.
  delete[] xml_buffer;
  fclose(xml_file);

  // Progress output.
  timer = time(NULL) - timer;
  cerr << " done" << flush;
  if (timer > 0)
    cerr << " in " << timer << " seconds ("
         << (xml_file_stat.st_size/(timer*1000*1000)) << " MiB/second)"
         << flush;
}

// _____________________________________________________________________________
static void XMLCALL startElementGlobal(void* user_data,
                                       const char* name,
                                       const char** atts)
{
  #ifdef XML_PARSER_DEBUG
  cerr << "[S]" << flush;
  #endif
  XmlParserNew* xp = reinterpret_cast<XmlParserNew*>(user_data);
  xp->startElement(name, atts);
}

// _____________________________________________________________________________
static void XMLCALL endElementGlobal(void* user_data, const char* name)
{
  #ifdef XML_PARSER_DEBUG
  cerr << "[E]" << flush;
  #endif
  XmlParserNew* xp = reinterpret_cast<XmlParserNew*>(user_data);
  xp->endElement(name);
}

// _____________________________________________________________________________
static void XMLCALL characterDataHandlerGlobal(void* user_data,
                                               const XML_Char* s,
                                               int len)
{
  #ifdef XML_PARSER_DEBUG
  cerr << "[C]" << flush;
  #endif
  XmlParserNew* xp = reinterpret_cast<XmlParserNew*>(user_data);
  xp->characterDataHandler(s, len);
}

// _____________________________________________________________________________
static int XMLCALL externalEntityRefHandlerGlobal(XML_Parser parser,
                                                  const XML_Char *context,
                                                  const XML_Char *base,
                                                  const XML_Char *systemId,
                                                  const XML_Char *publicId)
{
  XmlParserNew* xp = reinterpret_cast<XmlParserNew*>(XML_GetUserData(parser));
  return xp->externalEntityRefHandler(parser, context, base,
                                      systemId, publicId);
}

// _____________________________________________________________________________
void XmlParserNew::startElement(const char* name, const char** atts)
{
  // Push tag id (and create one if tag name encountered for the first time).
  addTag(name, XP_NORMAL_TAG);
  unsigned int tagId = _tagIds[name];

  // Ignore some tags.
  if (_itemsByTagId[tagId].flag == XP_IGNORE_TAG) return;

  // Some whitespace might have been scanned until here -> ignore it.
  pos = 0;

  // Start new item.
  // TODO(bast): currently hard-coded that all items are level-1 nodes
  if (nodeIdStack.size() == XP_NEW_ITEM_LEVEL)
  {
    reset(XP_NEW_ITEM_LEVEL);
    ++_docId;
    _pos = 0;
  }

  // NEW 05Feb07 (Holger): add *every* new node.
  unsigned int nodeId = addNode(name);
  nodeIdStack.push_back(nodeId);

  // Store attributes just like tags (but no need to stack).
  for (unsigned int i = 0; atts[i] != NULL; i += 2)
  {
    addTag(atts[i], XP_IS_ATTRIBUTE);
    addNode(atts[i], atts[i+1]);
  }
}

// _____________________________________________________________________________
void XmlParserNew::endElement(const char* name)
{
  unsigned int tagId = _tagIds[name];

  if (_itemsByTagId[tagId].flag == XP_IGNORE_TAG) return;

  if (nodeIdStack.empty())
  {
    cerr << "closing tag \"</" << name << ">\" without opening tag ... EXIT!"
         << endl;
    exit(1);
  }

  unsigned int nodeId = nodeIdStack.back();
  nodeIdStack.pop_back();

  if (nodeIds[nodeId].first != tagId)
  {
    cerr << "closing tag \"</" << name << ">\" does not match opening tag \"<"
         << _itemsByTagId[tagId].tagName << ">\" ... EXIT!" << endl;
    exit(1);
  }

  cur[pos] = 0;
  _itemsByTagId[tagId].items[nodeIds[nodeId].second] += cur;
  pos = 0;

  // NEW 05Feb07 (Holger): hard-coded again that all new items are level 1
  if (nodeIdStack.size() == XP_NEW_ITEM_LEVEL)
  {
    _item_name = name;
    outputDocAndWords();
  }
}

// _____________________________________________________________________________
int XmlParserNew::externalEntityRefHandler(XML_Parser parser,
                                        const XML_Char *context,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId)
{
  cerr << " [external entity: " << flush;
  XML_Parser entity_parser = XML_ExternalEntityParserCreate(parser, context,
                                                            NULL);
  parseXml(entity_parser, systemId);
  XML_ParserFree(entity_parser);
  cerr << "] " << flush;
  return 1;
}
// _____________________________________________________________________________
void XmlParserNew::characterDataHandler(const char* s, int len)
{
  if ((size_t)(pos + len) > BUF_SIZE)
  {
    cerr << "characterDataHandler: buffer overflow" << endl;
    exit(1);
  }
  for (int i = 0; i < len; ++i)
  {
    cur[pos] = (XP_REPLACE_WHITESPACE && isspace(s[i]) ? ' ' : s[i]);
    ++pos;
  }
  // strncpy(cur + pos, s, len);
  // pos += len;
}


// _____________________________________________________________________________
void XmlParserNew::reset(unsigned int from)
{
  for (unsigned int i = from; i < _itemsByTagId.size(); ++i)
    _itemsByTagId[i].count = 0;
  parents.resize(from);
  nodeIds.resize(from);
}

// _____________________________________________________________________________
void XmlParserNew::addTag(const char* tag_name, int tag_flag)
{
  assert(_tagIds.size() == _itemsByTagId.size());
  if (_tagIds.count(tag_name) == 0)
  {
    unsigned int tagId = _tagIds.size();
    _tagIds[tag_name] = tagId;
    _itemsByTagId.resize(tagId + 1);
    _itemsByTagId[tagId].tagName = tag_name;
    _itemsByTagId[tagId].count = 0;
    _itemsByTagId[tagId].flag = tag_flag;
  }
}

// _____________________________________________________________________________
unsigned int XmlParserNew::addNode(const char* tag_name, const char* data)
{
  unsigned int tagId = _tagIds[tag_name];
  unsigned int nodeId = nodeIds.size();
  unsigned int count = ++_itemsByTagId[tagId].count;
  vector<string>& items = _itemsByTagId[tagId].items;
  // TODO(bast): do we really need the ids?
  vector<unsigned int>& ids = _itemsByTagId[tagId].ids;
  assert(items.size() == ids.size());
  if (count > items.size())
  {
    items.resize(count);
    ids.resize(count);
  }
  items[count-1] = data;
  ids[count-1] = nodeId;
  assert(nodeIds.size() == parents.size());
  nodeIds.push_back(pair<unsigned int, unsigned int>(tagId, count-1));
  parents.push_back(nodeIdStack.size() > 0 ? nodeIdStack.back() : 99);

  return nodeId;
}


// _____________________________________________________________________________
void XmlParserNew::addContents(const char* tag_name, const char* data)
{
  unsigned int tagId = _tagIds[tag_name];
  unsigned int count = ++_itemsByTagId[tagId].count;
  vector<string>& items = _itemsByTagId[tagId].items;
  if (count > items.size()) items.resize(count);
  items[count-1] = data;
}

// _____________________________________________________________________________
const char* XmlParserNew::getItem(const char* tag_name,
                                  const char* default_item)
{
  if (_tagIds.count(tag_name) == 0) return default_item;
  unsigned int tagId = _tagIds[tag_name];
  if (_itemsByTagId[tagId].count == 0) return default_item;
  return _itemsByTagId[tagId].items[0].c_str();
}

// _____________________________________________________________________________
vector<const char*> XmlParserNew::getItems(const char* tag_name)
{
  vector<const char*> items;
  if (_tagIds.count(tag_name) == 0) return items;
  unsigned int tagId = _tagIds[tag_name];
  items.resize(_itemsByTagId[tagId].count);
  for (unsigned int i = 0; i < items.size(); ++i)
    items[i] = _itemsByTagId[tagId].items[i].c_str();
  return items;
}

// _____________________________________________________________________________
const char* XmlParserNew::getContent(const char* tag_name,
                                     const char* default_content)
{
  return getItem(tag_name, default_content);
}
vector<const char*> XmlParserNew::getContents(const char* tag_name)
{
  return getItems(tag_name);
}

// _____________________________________________________________________________
vector<unsigned int> XmlParserNew::getChildren(unsigned int nodeId,
                                               const char* tag_name)
{
  vector<unsigned int> ids;
  if (_tagIds.count(tag_name) > 0)
  {
    assert(_tagIds.size() < UINT_MAX);
    unsigned int tagId = *tag_name != 0 ? _tagIds[tag_name] : UINT_MAX;
    assert(nodeIds.size() == parents.size());
    for (unsigned int i = 0; i < nodeIds.size(); ++i)
      if (nodeId == UINT_MAX || parents[i] == nodeId)
        if (tagId == UINT_MAX || nodeIds[i].first == tagId) ids.push_back(i);
  }
  return ids;
}

// _____________________________________________________________________________
vector<unsigned int> XmlParserNew::getChildren(const char* tag_name)
{
  return getChildren(XP_NEW_ITEM_LEVEL, tag_name);
}

// _____________________________________________________________________________
const char* XmlParserNew::getContent(unsigned int nodeId)
{
  assert(nodeId < nodeIds.size());
  unsigned int tagId = nodeIds[nodeId].first;
  assert(tagId < _itemsByTagId.size());
  unsigned int i = nodeIds[nodeId].second;
  assert(i < _itemsByTagId[tagId].count);
  return _itemsByTagId[tagId].items[i].c_str();
}

// _____________________________________________________________________________
const char* XmlParserNew::getContent(unsigned int nodeId,
                                     const char* tag_name,
                                     const char* default_content)
{
  vector<unsigned int> ids = getChildren(nodeId, tag_name);
  return ids.size() == 0 ? default_content : getContent(ids[0]);
}

// _____________________________________________________________________________
void XmlParserNew::outputDocAndWords()
{
  ostringstream os;
  os << endl << "Item #" << _docId << ":" << endl;
  assert(_tagIds.size() == _itemsByTagId.size());
  for (unsigned int tagId = 0; tagId < _tagIds.size(); ++tagId)
  {
    Items& items = _itemsByTagId[tagId];
    for (unsigned int i = 0; i < items.count; ++i)
    {
      os << setw(20) << items.tagName << " : \"" << items.items[i] << "\""
         << endl;
      outputWords(items.items[i]);
    }
  }
  outputDoc(os.str());
}


// _____________________________________________________________________________
void XmlParserNew::outputDoc(const string& str)
{
  if (_docs_file == NULL) return;
  fwrite(str.c_str(), 1, str.size(), _docs_file);
}

// _____________________________________________________________________________
void XmlParserNew::outputDocument(const string& url,
                                  const string& title,
                                  const string& text)
{
  if (_docs_file == NULL) return;
  ostringstream os;
  if (text.find('\n') != string::npos)
  {
    cerr << "ERROR: document text contains newline character "
         << "(url: " << url << ", " << "title: " << title << ")" << endl;
    exit(1);
  }
  os << _docId << "\tu:" << url << "\tt:" << title << "\tH:" << text << endl;
  fwrite(os.str().c_str(), 1, os.str().size(), _docs_file);
}


// _____________________________________________________________________________
void XmlParserNew::outputWords(const string& str, unsigned int docId,
                               unsigned int score, unsigned int pos)
{
  if (str.size() == 0) return;
  // NEW 23Nov11 (Ina): Initialize encoding of StringConverter.
  StringConverter::Encoding encoding = StringConverter::ENCODING_ISO8859_1;
  if (_encoding == ParserBase::UTF8)
  {
    encoding = StringConverter::ENCODING_UTF8;
  }
  // Make a copy (do *not* use the STL copy operator, it does a "copy-on-write"
  // only)
  string str_copy;
  str_copy.assign(str.c_str(), str.size());
  unsigned char* c = (unsigned char*)(str_copy.c_str());
  unsigned char* c_end = c + str_copy.size();
  unsigned char* c_0;
  string word;
  unsigned int actualPos = pos;
  while (c < c_end)
  {
    // Skip non-word characters.
    while (c < c_end && IS_WORD_CHAR[*c] != '1') ++c;
    // Parse word.
    c_0 = c;
    while (c < c_end && IS_WORD_CHAR[*c] == '1') ++c;
    if (c > c_0)
    {
      word.assign(reinterpret_cast<char*>(c_0), c - c_0);
      string wordToLower = _stringConverter.convert(word,
                                       encoding,
                                       StringConverter::CONVERSION_TO_LOWER);
      // NEW 23Nov11 (Ina):
      // Add word normalization. Else the request rene without accent won't find
      // rene with accent, same the other way around.
      if (_normalizeWords)
      {
        // Normalization includes lowercasing
        word = _stringConverter.convert(word,
                                        encoding,
                                        StringConverter::CONVERSION_TO_NORM);
        // NEW 14Mar13 (baumgari): The normalization is changed such that e.g.
        // oe -> ö -> o. We want to store the original word to be able to
        // distinguish them (e.g. query is now qury ---> store it as
        // qury:query).
      }
      // NEW 10May12 (Ina):
      // Use StringConverter to lower the words, instead if utf8_tolower, which
      // doesn't work correctly and just jumps over unknows letters =>
      // escpecially in case of upper special characters that's a problem, since
      // they stay in uppercase.
      /*else
      {
        word = wordToLower;
      } */   

      if (pos == NO_POS) actualPos = (++_pos);
      if (_normalizeWords && word != wordToLower)
      {
        writeToWordsFile(word + _wordPartSep + wordToLower,
                         docId != NO_DOC_ID ? docId : _docId,
                         score != NO_SCORE ? score : 2,
                         actualPos);
      }

      writeToWordsFile(wordToLower,
                       docId != NO_DOC_ID ? docId : _docId,
                       score != NO_SCORE ? score : 2,
                       actualPos);
    }
  }
}


// _____________________________________________________________________________
// TODO(bast): if this works out, deprecate the above function.
void XmlParserNew::outputWordsAndDoc(const string& s, unsigned int docId,
                                     unsigned int score, unsigned int pos)
{
  if (s.size() == 0) return;
  unsigned int i = 0;
  unsigned int len = s.length();
  unsigned int i_0;
  string word;
  while (i < len)
  {
    // parse non-word
    i_0 = i;
    while (i < len && IS_WORD_CHAR[static_cast<unsigned char>(s[i])] != '1') ++i; // NOLINT
    if (i > i_0)
    {
      if (_docs_file != NULL) fwrite(s.c_str() + i_0, 1, i - i_0, _docs_file);
    }
    // parse word
    i_0 = i;
    while (i < len && IS_WORD_CHAR[static_cast<unsigned char>(s[i])] == '1') ++i; // NOLINT
    if (i > i_0)
    {
      if (_docs_file != NULL)
      {
        fwrite(s.c_str() + i_0, 1, i - i_0, _docs_file);
        if (XP_INDEXED_WORD_MARKER != 0)
          fwrite(&XP_INDEXED_WORD_MARKER, 1, 1, _docs_file);
      }
      word =  "";
      while (i_0 < i)
      {
        word += tolower(s[i_0]);
        ++i_0;
      }
      writeToWordsFile(word,
                       docId != NO_DOC_ID ? docId : _docId,
                       score != NO_SCORE ? score : 2,
                       pos != NO_POS ? pos : ++_pos);
    }
  }
}


// _____________________________________________________________________________
void XmlParserNew::outputWords(const string& str, const string& cat,
                               const char* pref1, const char* pref2,
                               unsigned int docId, unsigned int score,
                               unsigned int pos, const char* alt_str)
{
  // NEW 23Nov11 (Ina):
  // Initialize encoding of StringConverter.
  StringConverter::Encoding encoding = StringConverter::ENCODING_ISO8859_1;
  if (_encoding == ParserBase::UTF8)
  {
    encoding = StringConverter::ENCODING_UTF8;
  }

  if (str.size() == 0) return;
  if (docId == NO_DOC_ID) docId = _docId;
  if (pos == NO_POS) pos = ++_pos;
  if (score == NO_SCORE) score = 2;

  // Make a copy (but do *not* use the STL copy operator, see commented out
  // snippet).
  string str_copy;
  str_copy.assign(str.c_str(), str.size());
  string str_copy_copy;
  str_copy_copy.assign(str_copy.c_str(), str_copy.size());

  // NOTE: STL does *not* copy the string before it is actually modified for
  // the first time!!!
  // cerr << str << endl;
  // cerr << str_copy << endl;
  // str_copy[0] = str_copy[0];
  // for (unsigned int i = 0; i < str_copy.size(); ++i)
  // { *c = tolower(*c); ++c; }
  // cerr << str << endl;
  // cerr << str_copy << endl;

  unsigned char* c = (unsigned char*)(str_copy.c_str());
  unsigned char* c_end = c + str_copy.size();
  unsigned char* c_0;
  string word;
  vector<string> words;
  string words_concatenated;
  words_concatenated.reserve(str_copy.size());
  vector<unsigned int> words_pos;
  // STEP 1: break given string into words (converts str_copy to lowercase)
  while (c < c_end)
  {
    // skip non-word characters
    while (c < c_end && IS_WORD_CHAR[*c] != '1') ++c;
    // parse word
    c_0 = c;
    while (c < c_end && IS_WORD_CHAR[*c] == '1') ++c;

    if (c >= c_0)
    {
      words.resize(words.size()+1);
      words.back().assign(reinterpret_cast<char*>(c_0),
                        (unsigned int)(c - c_0));
      words.back() = _stringConverter.convert(words.back(),
                                      encoding,
                                      StringConverter::CONVERSION_TO_LOWER);
      words_pos.resize(words_pos.size()+1);
      words_pos.back() = words_concatenated.size();
      // NEW(Hannah, 25Mar10): Add _ between individual word parts.
      if (words_concatenated.size() > 0) words_concatenated += "_";
      words_concatenated += words.back();
    }
  }
  // NEW 23Nov11 (Ina):
  // Add word normalization. Else the request rene without accent won't find
  // rene with accent, same the other way around.
  // NEW 09Jan12 (Ina):
  // Commented out: we don't want to normalize author:-words and venue:-words,
  // since in this case some bookmarks of the dblp users won't work anymore.
  /* if (_normalizeWords)
  {
    // Normalization includes lowercasing
    words_concatenated = _stringConverter.convert(words_concatenated,
                                          encoding,
                                          StringConverter::CONVERSION_TO_NORM);
  } */
  // STEP 2: replace all spaces of the original string by _
  // (use unmodified str_copy_copy)
  assert(str_copy_copy.size() == str_copy.size());
  for (unsigned int i = 0; i < str.size(); ++i)
    str_copy[i] = isspace(str_copy_copy[i]) ? '_' : str_copy_copy[i];
    // str_copy[i] = str_copy_copy[i] == ' ' ? '_' : str_copy_copy[i];

  // STEP 3: for each word add the artifical word
  // <middle>:<front>:<back>:<category>:<Original String>
  assert(words.size() == words_pos.size());
  if (pref2 != NULL)
  {
    for (unsigned int i = 0; i < words.size(); ++i)
    {
      word = string(pref2) + words[i].c_str() + _wordPartSep +
             words_concatenated.substr(0, words_pos[i]) + _wordPartSep +
             (i  == words.size() - 1
               ? string("")
               : words_concatenated.substr(
                 words_pos[i+1],
                 words_concatenated.size() - words_pos[i+1])) + _wordPartSep +
             cat + _wordPartSep +
             str_copy;
      writeToWordsFile(word, docId, score, pos);
    }
  }

  // New 07Sep11 (Hannah): Optionally do the new STEP 4b. STEP 4a is the old
  // STEP 4.

  // STEP 4a: Also add the artificial word
  // ct:<category>:<words concatenated>:<Original String> (with score 2)
  word = string(pref1) + cat + _wordPartSep +
         words_concatenated + _wordPartSep +
         (alt_str == NULL ? str_copy.c_str() : alt_str);
  writeToWordsFile(word, docId, score, pos);

  // STEP 4b: Optionally add the artificial word
  // ce:<category>:<words concatenated>:<Original String> (with score 2)
  if (XP_ENTITY_PREFIX != NULL)
  {
    word = string(XP_ENTITY_PREFIX) + cat + _wordPartSep +
           words_concatenated + _wordPartSep +
           (alt_str == NULL ? str_copy.c_str() : alt_str);
    writeToWordsFile(word, docId, score, pos);
  }

  // STEP 5: Optionally add the artificial word ct:::<category>
  if (XP_OUTPUT_COLONCOLON_WORDS)
  {
    word = string("ct") + _wordPartSep + _wordPartSep + _wordPartSep + cat;
    writeToWordsFile(word, docId, score, pos);
  }
}

// Replaced by the better working StringConverter class.
/*
// _____________________________________________________________________________
int XmlParserNew::utf8_tolower(char* p)
{
  // CASE: normal character
  if (*p > 0)
  {
    *p = tolower(*p);
    return 1;
  }

  // CASE: some more complex thingy
  wchar_t wc;
  // TODO(bast): mbtowc is not thread-safe, use mbrtowc instead.
  int len = mbtowc(&wc, p, 4);
  if (len == 1)
  {
    *p = tolower(*p);
    return 1;
  }
  else if (len > 1)
  {
    wc = towlower(wc);
    int len2 = wctomb(p, wc);
    if (len2 > len)
    {
      cerr << "\n\nLENGTH DIFFERENCE AFTER LOWERCASE:\n\n";
      if (strlen(p) > 50) p[50] = 0;
      cerr << "Next characters are: '" << p << "'" << endl;
      cerr << "Length before: " << len << endl;
      cerr << "Length after : " << len2 << endl;
      exit(1);
    }
    return len;
  }
  else
  {
    return 1;
  }
}*/
