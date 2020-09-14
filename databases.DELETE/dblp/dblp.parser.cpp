// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Hannah Bast <bast>.

#include <errno.h>
#include <getopt.h>
#include <unordered_map>
#include <string>
#include <vector>
#include "codebase/parser/XmlParserNew.h"

// Note: without the static, g++ 4.x issues a warning
static enum
{
  FORMAT_TEXT,
  FORMAT_MLEY,
  FORMAT_GENERIC
} format = FORMAT_GENERIC;

string CT_PREFIX;  // NOLINT
string CN_PREFIX;  // NOLINT
string CE_PREFIX;  // NOLINT
extern const char* XP_ENTITY_PREFIX;
extern char XP_INDEXED_WORD_MARKER;
extern bool XP_NORMALIZE_WORDS;

// string urlBase = "http://search.mpi-inf.mpg.de/dblp/show.php?key=";
const char* urlBase = "http://www.dblp.org/rec/bibtex/";
// const char* urlBase = "http://dblp.uni-trier.de/rec/bibtex/";
bool useLocalUrl = false;
bool markIndexedWords = false;
// Extended DTD for ICE project.
bool parseExtendedDtd = false;

unordered_map<string, unsigned int, HashString> authorsWithSeveralNamesNameToId;
vector< vector<string> > authorsWithSeveralNamesIdToNames;
// Map of DBLP keys to tags.
unordered_map<string, string, HashString> keyToTagMap;

// PARSE 1: build keys for sorting and then sort

vector<unsigned int> ranks;

class DblpSortKeyParser : public XmlParserNew
{
  void outputDocAndWords();
  string _keyFileName;
  FILE* _key_file;
 public:
  void parse();
  void sort();
};

void DblpSortKeyParser::parse()
{
  _keyFileName = _fileNameBase + ".keys";
  _key_file = fopen((_keyFileName + ".unsorted").c_str(), "w");
  if (_key_file == NULL)
  {
    perror(_keyFileName.c_str());
    exit(1);
  }
  XmlParserNew::parse();
  fclose(_key_file);
}

void DblpSortKeyParser::outputDocAndWords()
{
  static string page;
  // const char* key = getItem("key");
  // if (record.key == "")
  // {
  //   fprintf(_log_file, "MISSING KEY in record #%d\n", _docId);
  //   return;
  // }
  const char* key1 = getItem("year", "9999");
  const char* key2 = getItem("booktitle");
  const char* key3 = getItem("pages");
  const char* key4 = getItem("title");
  if (*key2 == 0) key2 = getItem("journal");
  if (*key2 == 0) key2 = key4;
  if (*key3 != 0) key4 = "";
  fprintf(_key_file, "%d\t%s\t%s\t%d\t%s\n",
          _docId, key1, key2, atoi(key3), key4);

  // Remember info on authors with several names.
  string itemName = getItemName();
  string title = getItem("title");
  if (itemName == "www" && title == "Home Page")
  {
    vector<const char*> authors = getItems("author");
    if (authors.size() > 1)
    {
      unsigned int id = authorsWithSeveralNamesIdToNames.size();
      authorsWithSeveralNamesIdToNames.resize(id + 1);
      vector<string>& names = authorsWithSeveralNamesIdToNames.back();
      for (unsigned int i = 0; i < authors.size(); ++i)
      {
        string name = authors[i];
        if (authorsWithSeveralNamesNameToId.count(name) > 0)
        {
          cerr << "{!!! WARNING: repeated occurrence of \""
               << name << "\" in homepage record, will be ignored}" << flush;
          continue;
        }
        authorsWithSeveralNamesNameToId[name] = id;
        names.push_back(name);
      }
    }
  }
}

void DblpSortKeyParser::sort()
{
  unsigned int nofRecords = _docId;
  cout << "SORTING " << nofRecords << " records" << endl << endl;
  string cmd = "sort -t \"\t\" -k2,2nr -k3,3 -k4,4n -k5,5 " +
               _keyFileName + ".unsorted | cut -f1 > " +
               _keyFileName + ".sorted";
  cout << "executing \"" << cmd << "\" ... " << flush;
  int ret = system(cmd.c_str());
  if (WEXITSTATUS(ret) != 0)
  {
    fprintf(stderr, "sort exited with status %d (%s)\n",
            WEXITSTATUS(ret), strerror(errno));
    exit(1);
  }
  _key_file = fopen((_keyFileName + ".sorted").c_str(), "r");
  if (_key_file == NULL)
  {
    perror(_keyFileName.c_str());
    exit(1);
  }
  unsigned int MAX_DOCID_LEN = 1000;
  char docid_str[MAX_DOCID_LEN+1];
  unsigned int rank = 0;
  ranks.resize(nofRecords + 1);
  while (!feof(_key_file))
  {
    char* ret = fgets(docid_str, MAX_DOCID_LEN+1, _key_file);
    if (ret == NULL) continue;
    unsigned int docid = atoi(docid_str);
    assert(docid > 0);
    ranks[docid] = ++rank;
  }
  cout << "done (ranked " << rank << " records)" << endl << endl;
  fclose(_key_file);
}


// PARSE 2: write out words + docs file, with docid = rank

class DblpParser : public XmlParserNew
{
  void outputDocAndWords();
  void addGlobalInformationToWordsFile();
 public:
  void initializeKeyToTagMap();
  string escapeXml(const string& text);
};

void DblpParser::initializeKeyToTagMap()
{
  keyToTagMap["conf/spire/BastM06"] = "completesearch";
  keyToTagMap["conf/sigir/BastW06"] = "completesearch";
  keyToTagMap["conf/sigir/BastCSW06"] = "completesearch";
  keyToTagMap["conf/cikm/BastMW07"] = "completesearch";
  keyToTagMap["conf/cidr/BastW07"] = "completesearch";
  keyToTagMap["journals/ir/BastMW08"] = "completesearch";
  keyToTagMap["conf/icdm/BastSW08"] = "completesearch";
  keyToTagMap["conf/spire/CelikikB09"] = "completesearch";
  keyToTagMap["conf/sac/CelikikB09"] = "completesearch";
}

// NEW 01Nov12 (baumgari): Function for escaping nonproper xml. This is done to
// achieve proper xml, which can be used without cdata tags. Obviously it's
// necessary to unescape it later again.
string DblpParser::escapeXml(const string& textToEscape)
{
  string text = textToEscape;
  for (size_t i = 0; i < text.length(); i++)
  {
    if (text[i] == '&') text.replace(i, 1, "&amp;");
    else if (text[i] == '\'') text.replace(i, 1, "&apos;");
    else if (text[i] == '\"') text.replace(i, 1, "&quot;");
    else if (text[i] == '<') text.replace(i, 1, "&lt;");
    else if (text[i] == '>') text.replace(i, 1, "&gt;");
  }
  return text;
}

// NEW 15Nov12 (baumgari): Output predefined words for holding information
// about the index.
void DblpParser::addGlobalInformationToWordsFile()
{
  XmlParserNew::addGlobalInformationToWordsFile();
  // encoding
  string encoding = "iso-8859-1";
  if (_encoding == ParserBase::UTF8) encoding = "utf-8";
  ParserBase::writeToWordsFile(string("!!encoding") + _wordPartSep + encoding,
                               0, 0, 0);
  // name
  ParserBase::writeToWordsFile(string("!!name") + _wordPartSep
                               + getFileNameBase(), 0, 0, 0);
  // attributes with multiple elements
  string multiple = "author";
  ParserBase::writeToWordsFile(string("!!hitinfo-multiple") + _wordPartSep
                               + multiple, 0, 0, 0);
  multiple = "hits";
  ParserBase::writeToWordsFile(string("!!hitinfo-multiple") + _wordPartSep
                                      + multiple, 0, 0, 0);
}

void DblpParser::outputDocAndWords()
{
  // fprintf(_docs_file,
  //         "%d\tu:http://search.mpi-inf.mpg.de/dblp/show.php?key=\n",
  //         _docId);
  // fprintf(_docs_file,
  //         "%d\tu:http://search.mpi-inf.mpg.de/dblp/show.php?key=%s\n",
  //         _docId, getItem("key", "NO_KEY"));
  // XmlParserNew::outputDoc(os.str());
  // return;
  ostringstream os;

  // skip if neither conference nor journal
  string itemType = getItemName();
  bool isConference = itemType == "inproceedings" ||
                      itemType == "proceedings" ||
                      itemType == "incollection";
  bool isJournal = itemType == "article";
  // NEW 11May12 (Ina): Added a new type of items: phdthesis, which should be
  // handled like a book.
  bool isBook = itemType == "book" ||
                itemType == "phdthesis";
  if (!isConference && !isJournal && !isBook) return;

  // set docid to rank
  if (_docId >= ranks.size())
  {
    cout << "!!! _docId = " << _docId
         << ", ranks.size() = " << ranks.size() << endl;
  }
  assert(_docId < ranks.size());
  unsigned int docid = ranks[_docId];

  // doc id
  os << docid;

  // key
  const char* key = getItem("key", "NO_KEY");

  // MEW 14Mar09 (Hannah): some new types
  bool isCorr = strncmp(key, "journals/corr", 13) == 0;
  bool isEditor = itemType == "proceedings";

  // DBLP url and ee and year
  string      url = getItem("url");
  const char* src = getItem("src");
  string      ee = getItem("ee");
  const char* year = getItem("year", "0000_NO_YEAR");

  // NEW 31Jan08 (Hannah): Correct ee if it doesn't start with http:// or ftp://
  if (ee.size() > 0 &&
      ee.compare(0, 8, "https://") != 0 &&
      ee.compare(0, 7, "http://") != 0 &&
      ee.compare(0, 6, "ftp://") != 0)
  {
    ee = string("http://dblp.uni-trier.de/") + ee;
  }

  // NEW 27Oct09 (Hannah): add tag:completesearch to every paper on
  // completesearch.
  if (keyToTagMap.count(key) > 0)
    XmlParserNew::outputWords(keyToTagMap[key], "tag",
                              CT_PREFIX.c_str(), "", docid, 2);

  // URL
  os << "\tu:";
  if (useLocalUrl == true) os << escapeXml(src);
  else                     os << escapeXml(urlBase) << key;
  // os << "\tu:http://dblp.uni-trier.de/rec/bibtex/"
  //    << getItem("key", "NO_KEY");

  // TITLE
  os << "\tt:";

  // HTML format from DBLP
  if (format == FORMAT_MLEY)
  {
    os << "<table border=\"1\" width=\"100%\">";
    os << "<tr key=\"" << key << "\" year=\"" << year << "\">";
    //  os << "<td align=\"right\" valign=\"top\" bgcolor=\"#CCCCFF\">";
    //  os << "<a class=\"hits_title\" class=\"rec\""
    //        " href=\"http://dblp.uni-trier.de/rec/bibtex/conf/cidr/BastW07\""
    //        " name=\"p18\">NN</a>";
    //  os << "</td>";
    if (ee.size() > 0)
      os << "<td bgcolor=\"CCFFCC\" valign=\"top\">"
         << "<a class=\"ee\" href=\"" << ee << "\">EE</a>";
    //   << "<a class=\"hits_title\" class=\"ee\" href=\"" << ee << "\">EE</a>";
    else
      os << "<td bgcolor=\"#FFFFFF\" valign=\"top\">"
         << "<font color=\"#FFFFFF\">EE</font>";
    os << "</td>";
    os << "<td width=\"100%\">";
  }

  // NEW 26Sep12 (baumgari): Removed dblp: from dblp:author, dblp:venue,
  // dblp:year and dblp:title, since we actually don't need it and we want to
  // change the whole cdata-xml part, which contains the result (created in
  // here), to proper xml. In that case we wouldn't need the cdata tag anymore.

  // Authors.
  vector<const char*> authors = getItems("author");
  // NEW 21Dec07 (Holger): if no authors, try editors.
  bool authorsAreEditors = false;
  if (authors.size() == 0)
  {
    authors = getItems("editor");
    if (authors.size() > 0) authorsAreEditors = true;
  }
  if (format == FORMAT_GENERIC && !authors.empty()) os << "<authors>";
  // NEW 23Oct09 (Hannah): also output a special word for the number of authors.
  // Like this can restrict hit set to set of exactly the given authors.
  ostringstream os_na;
  os_na << authors.size();
  XmlParserNew::outputWords(os_na.str(), "na", CT_PREFIX.c_str(),
                            CN_PREFIX.c_str(), docid, 2, ++_pos);
  for (unsigned int i = 0; i < authors.size(); ++i)
  {
    if (format == FORMAT_GENERIC) os << "<author>";
    if (format == FORMAT_MLEY)
    {
      string authorQuery = "author:";
      for (const char* p = authors[i]; *p != 0; p++)
      {
        if (isalpha(*p)) authorQuery += tolower(*p);
        else if (*p < 0) authorQuery += *p;
      }
      os << "<a href=\"javascript:AC.completion_link('"
         << authorQuery << "')\">";
      // os << "<a class=\"hits_title\" href=\"javascript:AC.completion_link('"
      //    << authorQuery << "')\">";
      // os << "<a class=\"hits_title\" href=\"index.php?autocomplete_query="
      //    << authorQuery << "\">";
    }
    os << escapeXml(authors[i]);
    if (format == FORMAT_MLEY) os << "</a>";
    if (format != FORMAT_GENERIC) os << (i < authors.size() - 1 ? ", " : ": ");
    if (format == FORMAT_GENERIC) os << "</author>";

    // NEW 20Dec07 (Holger): for authors with several names, add all
    // corresponding ct:author:...
    // NEW 21Dec07 (Holger): only add canonical name (first in group), to avoid
    // display of variants of the same name in facet boxes, e.g. Alon Y. Ley
    // (22) and Alon Y. Halevy (22)
    // TODO(bast): maybe put alternative names in paranthesis, e.g. Alon Y.
    // Halevy (Alon Y. Levy), like in Michael's co-author list. Names become a
    // bit long then though.
    string name = authors[i];
    vector<string> names;
    if (authorsWithSeveralNamesNameToId.count(name) == 0)
    {
      names.push_back(name);
    }
    else
    {
      unsigned int id = authorsWithSeveralNamesNameToId[name];
      names = authorsWithSeveralNamesIdToNames[id];
    }
    for (unsigned int j = 0; j < names.size(); ++j)
    {
      string& name = names[j];
      // ct:author:... only for canonical name (even if paper written under
      // older name)
      if (j == 0)
      {
        // For ambiguous names with suffixe like 0002, don't display the suffix
        // in the Refine by boxes (using new alt_str feature of outputWords).
        string altName;
        if (name.length() > 5 &&
            name.find_last_not_of("0123456789") == name.length() - 5)
        {
          altName = name.substr(0, name.length() - 5);
          for (unsigned int i = 0; i < altName.length(); ++i)
          {
            if (isspace(altName[i])) altName[i] = '_';
          }
        }
        const char* alt_name_str = altName == "" ? NULL : altName.c_str();
        XmlParserNew::outputWords(name, "author", CT_PREFIX.c_str(),
                                  CN_PREFIX.c_str(), docid, 2, ++_pos,
                                  alt_name_str);
        // NEW 20Nov09 (Hannah): output special word for first author.
        if (i == 0 && !authorsAreEditors)
        {
          XmlParserNew::outputWords(name, "firstauthor", CT_PREFIX.c_str(),
                                    CN_PREFIX.c_str(), docid, 2, ++_pos,
                                    alt_name_str);
        }
      }
      // If no cn:... words are output or cn: prefix is non-empty, add plain
      // words for all names with score 100.
      if (CN_PREFIX.c_str() == NULL || strlen(CN_PREFIX.c_str()) > 0)
      {
        // NEW 03Jan08: Put first letter of first name as word (e.g. "k" for
        // "Kurt Mehlhorn")
        if (name.length() > 2 && isalpha(name[0]) && isalpha(name[1]))
        {
          XmlParserNew::outputWords(name.substr(0, 1), docid, 100);
          // XmlParserNew::outputWords(name.substr(0, 1), docid, 2);
          --_pos;
        }
        XmlParserNew::outputWords(name, docid, 100);
      }
    }
  }
  if (format == FORMAT_GENERIC && !authors.empty()) os << "</authors>";

  // title
  if (format == FORMAT_GENERIC) os << "<title ee=\"" << escapeXml(ee) << "\">";
  string title = getItem("title", "NO_TITLE");
  // TODO(bast): Replace all < by &lt; and all > by &gt; again in the title.
  // ...
  // if (title.size() > 0 && title[title.size()-1] == '.')
  //   title.erase(title.size()-1);
  os << escapeXml(title) << " ";
  if (format == FORMAT_GENERIC) os << "</title>";
  XmlParserNew::outputWords(title, docid, 2);

  // NEW 05Sep12 (baumgari): Modify venue tag by adding year, pages etc. as
  // attributes. (before: <dblp:venue url="...">some title with year, pages.
  // etc.</dblp:venue>, now: <dblp:venue url="..." year=... pages=... ...>some
  // title with year, pages, etc.</dblp:venue>). This is done to simplify
  // reading out the details of the result (without splitting the whole text
  // string.
  // NEW 26Sep12 (baumgari): Removed year from <dblp:venue year=xyz...>...
  // </dblp:venue> since we don't need it twice (already represented as
  // <dblp:year>

  // venue
  if (format == FORMAT_GENERIC) os << "<venue url=\"" << escapeXml(url) << "\"";
  string venue;
  // const char* year = getItem("year", "NO_YEAR");
  XmlParserNew::outputWords(year, "year", CT_PREFIX.c_str(),
                            CN_PREFIX.c_str(), docid, 0);
  if (CN_PREFIX.c_str() != NULL && strlen(CN_PREFIX.c_str()) > 0)
    XmlParserNew::outputWords(year, docid, 2);
  string pages = getItem("pages", "");
  // case 1: conference
  if (isConference)
  {
    string conference = getItem("booktitle", "NO_BOOKTITLE");
    // strip number from titles like "GECCO (1)"
    unsigned int i = conference.length();
    if (i > 0 && conference[i-1] == ')')
    {
      i--;
      while (i > 0 && isdigit(conference[i-1])) i--;
      if (i > 0 && conference[i-1] == '(')
      {
        i--;
        while (i > 0 && isspace(conference[i-1])) i--;
        conference.resize(i);
      }
    }
    // NEW 01Jan08: clean up some conference names, e.g. "Fast Software
    // Encryption" -> "FSE", "Symposium on Computational Geometry" -> "SoCG"
    string display = conference;
    if      (conference == "Fast Software Encryption")
      display = conference = "FSE";
    else if (conference == "Symposium on Computational Geometry")
      display = conference = "SoCG";
    else if (conference == "SIGMOD Conference")
      display = conference = "SIGMOD";
    else if (conference == "ICPC" ||
             conference == "IWPC" ||
             conference == "WPC")
      conference = "ICPC/IWPC";
    else if (conference == "FoSSaCS")
      conference = "FOSSACS";

    XmlParserNew::outputWords(conference, "venue", CT_PREFIX.c_str(),
                              CN_PREFIX.c_str(), docid, 2);
    if (CN_PREFIX.c_str() != NULL && strlen(CN_PREFIX.c_str()) > 0)
    {
      XmlParserNew::outputWords(conference, docid, 2);
      if (display != conference) XmlParserNew::outputWords(display, docid, 2);
    }

    // NEW 05Sep12 (baumgari): Adding attributes (see above).
    os << " conference=" << "\"" << escapeXml(conference) << "\"";
    if (!pages.empty()) os << " pages=" << "\"" << escapeXml(pages) << "\"";
    os << ">";

    if (format == FORMAT_MLEY && !url.empty())
      os << "<a href=\"http://www.informatik.uni-trier.de/~ley/"
         << url << "\">";
    os << escapeXml(display) << " " << escapeXml(year);
    if (format == FORMAT_MLEY && !url.empty())
      os << "</a>";
    if (!pages.empty()) os << ":" << escapeXml(pages);
  }
  // case 2: journal
  else if (isJournal)
  {
    string journal = getItem("journal", "NO_JOURNAL");
    const char* p =  key;
    while (*p != 0  && *p != '/') ++p;
    if (*p != 0) ++p;
    string j;
    while (*p != 0  && *p != '/') j += toupper(*p++);
    if (j.size() > 0 && j.size() < journal.size()) journal += " (" + j + ")";
    XmlParserNew::outputWords(journal, "venue", CT_PREFIX.c_str(),
                              CN_PREFIX.c_str(), docid, 2);
    if (CN_PREFIX.c_str() != NULL && strlen(CN_PREFIX.c_str()) > 0)
      XmlParserNew::outputWords(journal, docid, 2);
    // XmlParserNew::outputWords(journal, "journal", "ct:", "", docid, 2);
    string volume = getItem("volume", "");
    string number = getItem("number", "");

    // NEW 05Sep12 (baumgari): Adding attributes (see above).
    os << " journal=" << "\"" << escapeXml(journal) << "\"";
    if (!pages.empty()) os << " pages=" << "\"" << escapeXml(pages) << "\"";
    if (!number.empty()) os << " number=" << "\"" << escapeXml(number) << "\"";
    if (!volume.empty()) os << " volume=" << "\"" << escapeXml(volume) << "\"";
    os << ">";

    if (format == FORMAT_MLEY && !url.empty())
      os << "<a href=\"http://www.informatik.uni-trier.de/~ley/"
         << url << "\">";
    os << escapeXml(journal);
    if (!volume.empty()) os << " " << escapeXml(volume);
    if (format == FORMAT_MLEY && !url.empty()) os << "</a>";
    if (!number.empty()) os << "(" << escapeXml(number) << ")";
    if (!pages.empty()) os << ":" << escapeXml(pages);
    os << " (" << escapeXml(year) << ")";
  }
  // case 3: book
  else if (isBook)
  {
    string publisher = getItem("publisher", "NO_PUBLISHER");
    // NEW 05Sep12 (baumgari): Adding attributes (see above).
    // NEW 11May12 (Ina): Since book now also includes phdthesis, which doesn't
    // have a publisher, but therefore a school, where it was written, we need
    // to add this case here.
    if (strcmp(publisher.c_str(), "NO_PUBLISHER") == 0)
    {
      publisher = getItem("school", "NO_PUBLISHER");
      os << " school=" << "\"" << escapeXml(publisher) << "\">";
    }
    else
    {
      os << " publisher=" << "\"" << escapeXml(publisher) << "\">";
    }

    XmlParserNew::outputWords(publisher, "publisher", CT_PREFIX.c_str(),
                              CN_PREFIX.c_str(), docid, 2);
    if (CN_PREFIX.c_str() != NULL && strlen(CN_PREFIX.c_str()) > 0)
      XmlParserNew::outputWords(publisher, docid, 2);

    os << escapeXml(publisher) << " " << escapeXml(year);
  }
  // case 4: no venue
  else
  {
    os << "NO_VENUE";
  }
  if (format == FORMAT_GENERIC) os << "</venue>";

  // Add year explicitly
  if (format == FORMAT_GENERIC) os << "<year>" << escapeXml(year) << "</year>";

  // NEW 21Dec07 (Holger): add type of entry (inproceedings, article, book,
  // etc.)
  if (format == FORMAT_GENERIC) os << "<type>" << getItemName() <<
    "</type>";
  // NEW 13Mar09 (Hannah): and also artifical words (type:conference,
  // type:journal, type:book)
  if (isConference && !isEditor)\
    XmlParserNew::outputWords("Conference", "type", CT_PREFIX.c_str(), CN_PREFIX.c_str(),
                              docid, 2);
  if (isEditor)
    XmlParserNew::outputWords("Editor", "type", CT_PREFIX.c_str(), CN_PREFIX.c_str(),
                              docid, 2);
  if (isJournal && !isCorr)
    XmlParserNew::outputWords("Journal", "type", CT_PREFIX.c_str(), CN_PREFIX.c_str(),
                              docid, 2);
  if (isCorr)
    XmlParserNew::outputWords("CoRR", "type", CT_PREFIX.c_str(), CN_PREFIX.c_str(),
                              docid, 2);
  if (isBook)
    XmlParserNew::outputWords("Book", "type", CT_PREFIX.c_str(), CN_PREFIX.c_str(),
                              docid, 2);

  // HTML format from DBLP
  if (format == FORMAT_MLEY)
  {
    os << "</td>";
    os << "</tr>";
    os << "</table>";
  }

  // NEW 19Sep07 (Holger): add category for decade
  if (strlen(year) == 4)
  {
    string decadeBegin = year;
    string decadeEnd = year;
    decadeBegin[3] = '0';
    decadeEnd[3] = '9';
    XmlParserNew::outputWords(decadeBegin + " - " + decadeEnd, "decade",
                              CT_PREFIX.c_str(), CN_PREFIX.c_str(), docid, 2);
  }

  // NEW 17Oct07 (Holger): deal with optional text element (dor DBLP PLUS)
  os << "\tH:";
  outputDoc(os.str());
  string text = getItem("text", "");  // [FULL TEXT MISSING]");
  if (text.size() > 0)
  {
    // BEWARE: pos must start from beginning again, otherwise docs enhancemene
    // doesn't work. This might (does) affect the ranking though, because words
    // in the title now form "phrases" with words from the beginning of the
    // document
    // TODO(bast): fix this!!!
    if (XP_INDEXED_WORD_MARKER != 0)
      for (unsigned int i = 0; i < _pos; ++i)
        fprintf(_docs_file, "%c ", XP_INDEXED_WORD_MARKER);
    outputWordsAndDoc(text, docid, 0);
  }

  // Parse extended DTD (for ICE project).
  if (parseExtendedDtd)
  {
    // Full text.
    string fulltext = getItem("fulltext");
    outputWordsAndDoc(fulltext, docid, 0);
    // Doc id.
    ostringstream docIdOs;
    docIdOs << docid;
    XmlParserNew::outputWords(docIdOs.str(), "docid", CT_PREFIX.c_str(),
                              CN_PREFIX.c_str(), docid, 2);
    // Key.
    string key = getItem("key");
    XmlParserNew::outputWords(key, "key", CT_PREFIX.c_str(),
                              CN_PREFIX.c_str(), docid, 2);
  }

  // done
  outputDoc("\n");
}

// Show some (n) authors with several names. Just for checking.
void showAuthorsWithSeveralNames(size_t numAuthorsShow)
{
  cout << "Number of authors with several names: "
       << authorsWithSeveralNamesIdToNames.size() << endl << endl;
  if (authorsWithSeveralNamesIdToNames.size() > numAuthorsShow)
  {
    for (unsigned int i = 0; i < numAuthorsShow; ++i)
    {
      cout << "#" << (i+1) << " : " << flush;
      unsigned int n = authorsWithSeveralNamesIdToNames[i].size();
      for (unsigned int j = 0; j < n; ++j)
        cout << authorsWithSeveralNamesIdToNames[i][j]
             << (j < n - 1 ? ", " : "") << flush;
      cout << endl;
    }
    cout << "..." << endl;
    {
      string name = "Alon Y. Levy";
      unsigned int i = authorsWithSeveralNamesNameToId[name];
      unsigned int n = authorsWithSeveralNamesIdToNames[i].size();
      cout << "#" << setw(5) << setfill('0') << i
           << " " << setw(20) << setfill(' ') << name << " : " << flush;
      for (unsigned int j = 0; j < n; ++j)
        cout << authorsWithSeveralNamesIdToNames[i][j]
             << (j < n - 1 ? ", " : "") << flush;
      cout << endl;
    }
    {
      string name = "Alon Y. Halevy";
      unsigned int i = authorsWithSeveralNamesNameToId[name];
      unsigned int n = authorsWithSeveralNamesIdToNames[i].size();
      cout << "#" << setw(5) << setfill('0') << i
           << " " << setw(20) << setfill(' ') << name << " : " << flush;
      for (unsigned int j = 0; j < n; ++j)
        cout << authorsWithSeveralNamesIdToNames[i][j]
             << (j < n - 1 ? ", " : "") << flush;
      cout << endl;
    }
    cout << "..." << endl << endl;
  }
}



// The text element has an attribute url which specifies the URL
// of the pdf/ps from which the text was extracted, e.g.
// http://search.mpi-inf.mpg.de/papers/BastM05.pdf

// Main function. Does two parses, one to sort the doc ids by keys, than the
// actual parsing.
int main(int argc, char** argv)
{
  // Create instances of the two parsers needed, and initialize them both via
  // the command line arguments.
  DblpSortKeyParser xp1;
  // xp1.parseCommandLineOptions(argc, argv);
  DblpParser xp2;
  xp2.parseCommandLineOptions(argc, argv);
  xp1.setFileNameBase(xp2.getFileNameBase());
  xp1.parseCommandLineOptions(0, NULL);

  CT_PREFIX = string("ct") + xp2.getWordPartSep();
  CN_PREFIX = string("cn") + xp2.getWordPartSep();
  CE_PREFIX = string("ce") + xp2.getWordPartSep();

  XP_INDEXED_WORD_MARKER = markIndexedWords == true ? '' : 0;
  XP_NORMALIZE_WORDS = true;
  XP_ENTITY_PREFIX = CE_PREFIX.c_str();

  // Now deal with options specific to this DBLP parser.
  opterr = 0;
  optind = 1;
  static struct option longOptions[] =
  {
    {"mark-indexed-words" , 0, NULL, 'i'},
    {"format-mley"        , 0, NULL, 'm'},
    {"use-local-url"      , 0, NULL, 'l'},
    {"parse-extended-dtd" , 0, NULL, 'e'},
    {NULL      , 0, NULL,   0}
  };
  while (true)
  {
    int c = getopt_long(argc, argv, "imle", longOptions, NULL);
    if (c == -1) break;
    switch (c)
    {
      case 'i': markIndexedWords = true; break;
      case 'm': format = FORMAT_MLEY; break;
      case 'l': useLocalUrl = true; break;
      case 'e': parseExtendedDtd = true; break;
    }
  }

  // First parse.
  xp1.parse();

  // xp1.showStatistics();
  // xp1.showLog();
  xp1.sort();

  // Show some (up to 5) authors with several names. NOTE: just for checking.
  showAuthorsWithSeveralNames(5);

  // Second parse.
  xp2.addTag("sub", XP_IGNORE_TAG);
  xp2.addTag("sup", XP_IGNORE_TAG);
  xp2.initializeKeyToTagMap();
  xp2.parse();

  // xp2.showStatistics();
  // xp2.showLog();
}
