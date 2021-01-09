// Copyright 2009, University of Freiburg
// Chair of Algorithms and Data Structures.
// Authors: Hannah Bast <bast>.

#ifndef PARSER_XMLPARSERNEW_H_
#define PARSER_XMLPARSERNEW_H_

#include <expat.h>
#include <gtest/gtest.h>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

#include "parser/ParserBase.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

#define NO_DOC_ID UINT_MAX
#define NO_POS UINT_MAX
#define NO_SCORE UINT_MAX

#define XP_NEW_ITEM_LEVEL 1
#define XP_NORMAL_TAG   0
#define XP_IS_ATTRIBUTE 1
#define XP_NEW_ITEM     2
#define XP_IGNORE_TAG   3

// New XML parser with basically the same functionality of the old XML parser,
// but derived from ParserBase. That way all the more recent goodies like
// synonyms, fuzzy search, etc. are now also easy to have when the data is given
// as XML.
class XmlParserNew : public ParserBase
{
 public:
  // Default constructor.
  XmlParserNew();
  // Destructor. Frees space allocated for parse buffer.
  virtual ~XmlParserNew();
  // Print usage info. First prints usage info specific to the XML parser, then
  // usage info from parser/ParserBase.
  void printUsage() const;
  // Parse options from command line. First calls
  // ParserBase::parseCommandLineOptions and then parses options specific to
  // XmlParserNew (currently none).
  void parseCommandLineOptions(int argc, char** argv);
  // Parse.
  void parse();
  FRIEND_TEST(XmlParserNewTest, exampleParser);

  // Create new tag id if encountered for the first time (and if flag is
  // one of XP_NORMAL_TAG, XP_IS_ATTRIBUTE, XP_IGNORE_TAG).
  void addTag(const char* tag_name, int tag_flag);
  // Add contents. If no valid tag id for tag name exists do nothing.
  // TODO(bast): deal with nested tags.
  void addContents(const char* tag_name, const char* data);
  // Call at each opening tag or attribute (should replace addContents), returns
  // new node id.
  unsigned int addNode(const char* tag_name, const char* data = "");
  // Get content of given tag. Returns content of first tag with that name in
  // list, or default if no tags with that name exist.
  const char* getItem(const char* tag_name, const char* default_item = "");
  // Get contents of all tags with the given name. Returns the empty vector if
  // no tags with that name exist.
  vector<const char*> getItems(const char* tag_name);

  // More sophisticated access methods (to get content of particular tag at a
  // particular nesting level).
  vector<unsigned int> getChildren(unsigned int nodeId,
                                   const char* tag_name = "");
  vector<unsigned int> getChildren(const char* tag_name = "");
  const char* getContent(unsigned int nodeId);
  const char* getContent(unsigned int nodeId,
                         const char* tag_name,
                         const char* default_content = "");
  const char* getContent(const char* tag_name, const char* default_item = "");
  vector<const char*> getContents(const char* tag_name);
  // Get name of current top-level item.
  const char* getItemName() { return _item_name; }
  // Output doc and words (virtual so that it can be overloaded by
  // application
  virtual void outputDocAndWords();
  // Output doc (from given string).
  void outputDoc(const string& str);
  // Output doc with url, title, text.
  void outputDocument(const string& url,
                      const string& title,
                      const string& text);
  // Tokenize string and output as words.
  void outputWords(const string& str,
                   unsigned int docId = NO_DOC_ID,
                   unsigned int score = NO_SCORE,
                   unsigned int pos = NO_POS);
  // Same as above, but simultaneously output words to doc file.
  void outputWordsAndDoc(const string& s,
                         unsigned int docId = NO_DOC_ID,
                         unsigned int score = NO_SCORE,
                         unsigned int pos = NO_POS);

  // Output special word.
  // The last argument specifies an alternative display string, e.g.
  // a call with ("Kurt Mehlhorn", "author", "ct:", ..., "König Kurt") would
  // output ct:author:kurtmehlhorn:König_Kurt.
  void outputWords(const string& str, const string& cat,
                   const char* pref1,
                   const char* pref2 = "",
                   unsigned int docId = NO_DOC_ID,
                   unsigned int score = NO_SCORE,
                   unsigned int pos = NO_POS,
                   const char* alt_str = NULL);
  // Reset current item. TODO(bast): what does this do?
  void reset(unsigned int);

  // Convert UTF-8 string to lower case.
  // TODO(bast): we now have a function for this in parser/StringConversion,
  // this should be used.
  int utf8_tolower(char* p);

  // Functions called by expat (for dealing with an opening tag, a closing tag,
  // character data, and external entities).
  // NOTE(bast): must be public, because the expat functions callings these are
  // (by expat design?) global functions.
  void startElement(const char* name, const char** atts);
  void endElement(const char* name);
  void characterDataHandler(const char* s, int len);
  int externalEntityRefHandler(XML_Parser parser,
                               const XML_Char *context,
                               const XML_Char *base,
                               const XML_Char *systemId,
                               const XML_Char *publicId);

 protected:
  // Doc id and position.
  unsigned int _docId;
  unsigned int _pos;

 private:
  // Function doing the actual XML parsing (calls itself recursively in case of
  // includes).
  void parseXml(XML_Parser xml_parser, string xmlFileName);

  // XML file name.
  string _xmlFileName;
  // Parse buffer.
  char* cur;
  // Current position in parse buffer.
  int pos;
  // Stack of nodes in XML tree at current parse position.
  vector<unsigned int> nodeIdStack;
  // Ids for all tags encountered so far (nested tags treated as single tag,
  // e.g. "author name")
  unordered_map<string, unsigned int, HashString> _tagIds;
  // TODO(bast): what is this?
  typedef struct
  {
   public:
    string tagName;
    int flag;
    unsigned int count;
    vector<string> items;
    vector<unsigned int> ids;
  } Items;
  // Items by tag id.
  vector<Items> _itemsByTagId;

  // Nodes and their parents.
  vector< pair<unsigned int, unsigned int> > nodeIds;
  vector<unsigned int> parents;
  const char* _item_name;
};

#endif  // PARSER_XMLPARSERNEW_H_
