// Copyright 2010, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Ina Baumgarten <baumgari>

#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <utility>
#include "codebase/server/Timer.h"

using namespace std; // NOLINT

extern string decodeEntitiesToUtf8(const string& src);
extern string convertEntitiesToHtmlEntities(const std::string& src);

class Xml2Csv
{
 public:
  // Constructor.
  Xml2Csv();
  // Parse the xml and convert it to csv.
  void parse(const string& basename);
  // We don't want some words to be stored as facet. This is done especially
  // with synonyms. We store the canonical name as facet and the synonym words
  // just as filter words to be able to find the author, but to not be
  // confused by the synonyms - the user cannot know which name is a synonym
  // and which not.
  void setNoFacetCharacter(char c);
  // Sets the withinFieldDelimiter, which is used to distinguish e.g.
  // different authors within one field.
  void setWithinFieldDelimiter(char c);
  // Fill fields with json instead of xml.
  void setFieldFormatToJson();

 private:

  struct PersonRecord
  {
    vector<string> aliases;
    vector<string> urls;
    string key;

    void clear()
    {
      aliases.clear();
      urls.clear();
      key.clear();
    }
  };

  // Struct representing the fields of the csv (besides "type" and
  // "authorname").
  struct PublicationRecord
  {
    vector<string> authors;
    string dblpRecord;
    string year;
    string month;
    string title;
    string venue;
    string journal;
    string conference;
    string pages;
    string volume;
    string ee;
    string url;
    string publisher;
    string isbn;
    string type;
    string key;
    string series;
    string number;
    string crossref;
    string note;
    string cite;
    string cdrom;

    void clear()
    {
      authors.clear();
      dblpRecord.clear();
      year.clear();
      month.clear();
      title.clear();
      venue.clear();
      journal.clear();
      conference.clear();
      pages.clear();
      volume.clear();
      ee.clear();
      url.clear();
      publisher.clear();
      isbn.clear();
      type.clear();
      key.clear();
      series.clear();
      number.clear();
      crossref.clear();
      note.clear();
      cite.clear();
      cdrom.clear();
    }
  };

  // Some lines are not well formed and containg more than one tag or end with
  // whitespaces. Clean the lines and store them in linesOut (one line - one
  // tag).
  void preparseAndSplitLine(string* const text,
                            vector<string>* const linesOut);
  // Get the text from a tag like <author ...>text<author> and unescape html
  // entities.
  // For a correct usage it's necessary that end indicates the end of the
  // tag ">".
  inline string getText(size_t& start, size_t& end, const string& xmlLine);
  // Get tag name within a given tag like <author ...>text<author>.
  // It's necessary that start indicated the beginning of the tag "<".
  inline string getTagName(size_t& start, size_t& end, const string& xmlLine);
  // Extract attributes from a tag.
  inline vector<pair<string, string> >
    getAttributes(const string& xmlLine) const;
  // Do something with the information from each person of the persons area.
  // In our case we add all aliases to a map for a fast accessing later.
  void handlePersonInformation(const size_t& id,
                               const PersonRecord& personInfo);
  // For each author add main author as facet and all synonyms as nonfacet.
  string getAuthorsWithSynonyms(const vector<string>& authors) const;
  // Transforms an author the a dblp url like: Georg von Blaumann
  // www.dblp.org/indices/a-tree/b/Blaumann:Georg_von
  string transformAuthorToDblpUrl(const string& author) const;
  // Create dblp record from record member. This should be the unmodified xml
  // surrounded by cdata tags.
  string createDblpRecord(const PublicationRecord& publication) const;
  // Create html record from record member. This should look more or less like
  // actual html record created by the UI.
  string createHtmlRecord(const PublicationRecord& publication) const;
  // Create xml record from record member. This should be returned using the
  // standard api.
  string createXmlRecord(const PublicationRecord& publication) const;
  // Function for escaping nonproper xml. This is done to
  // achieve proper xml, which can be used without cdata tags. Obviously it's
  // necessary to unescape it later again.
  string escapeXml(const string& text) const;
  // Function for escaping nonjson text. This is done to achieve proper json.
  string escapeJson(const string& text) const;
  // There are some nonstandard rewriting, which should be in here, if
  // possible.
  void rewriteRecordEntries(PublicationRecord * const publication);
  // Storing the type of the record.
  string _startTagName;
  // Holds the base url of the website.
  string _baseUrl;
  // Char indicating that a string should not be stores as facet.
  char _noFacetChar;
  // Char to delimit authors.
  char _withinFieldDelimiter;
  // Fill csv fields with json instead of xml.
  bool _fillWithJson;
  // Static variables.
  static const char kCollectionNameTag[];
  static const char kPersonsTag[];
  static const char kPersonTag[];
  static const char kLegendTag[];
  static const char kPublicationsTag[];
  static const char kSectionTag[];
  static const char kBaseUrl[];

  // Store the id of the authorsWithSeveralNamesIdToNames for each name, which
  // can be found in the authorLists.
  boost::unordered_map<string, size_t> authorsWithSeveralNamesNameToId;
  // Get a person record with a list of synonyms for a given id. The first name
  // should be used as canonical name.
  vector<PersonRecord> authorsWithSeveralNamesIdToPersonRecord;
  // Stores the section name for each type, e.g. "A" -> "Book and Theses"
  boost::unordered_map<string, string> sectionToType;
};

const char Xml2Csv::kCollectionNameTag[] = "sorteddblp";
const char Xml2Csv::kPersonsTag[]        = "persons";
const char Xml2Csv::kPersonTag[]         = "person";
const char Xml2Csv::kLegendTag[]         = "legend";
const char Xml2Csv::kPublicationsTag[]   = "publications";
const char Xml2Csv::kSectionTag[]        = "section";
const char Xml2Csv::kBaseUrl[]           = "http://www.dblp.org/";

// ____________________________________________________________________________
Xml2Csv::Xml2Csv()
{
  _fillWithJson = false;
  _noFacetChar = '*';
  _withinFieldDelimiter = '#';
}

// ____________________________________________________________________________
void Xml2Csv::setNoFacetCharacter(char c)
{
  _noFacetChar = c;
}

// ____________________________________________________________________________
void Xml2Csv::setWithinFieldDelimiter(char c)
{
  _withinFieldDelimiter = c;
}

// ____________________________________________________________________________
void Xml2Csv::setFieldFormatToJson()
{
  _fillWithJson = true;
}

// ____________________________________________________________________________
string Xml2Csv::escapeXml(const string& textToEscape) const
{
  string text;
  for (size_t i = 0; i < textToEscape.length(); i++)
  {
    if (textToEscape[i] == '&') text += "&amp;";
    else if (textToEscape[i] == '\'') text += "&apos;";
    else if (textToEscape[i] == '\"') text += "&quot;";
    else if (textToEscape[i] == '<') text += "&lt;";
    else if (textToEscape[i] == '>') text += "&gt;";
    else
    {
      text += textToEscape[i];
    }
  }
  return text;
}

// ____________________________________________________________________________
string Xml2Csv::escapeJson(const string& textToEscape) const
{
  string text;
  for (size_t i = 0; i < textToEscape.length(); i++)
  {
    if (textToEscape[i] == '\'') text += "\\";
    else if (textToEscape[i] == '\"') text += "\\";
    else if (textToEscape[i] == '\\') text += "\\";
    text += textToEscape[i];
  }
  return text;
}

// ____________________________________________________________________________
string Xml2Csv::getTagName(size_t& start, size_t& end, const string& xmlLine)
{
  start = xmlLine.find('<');
  end = xmlLine.find_first_of("> ", start);
  assert(end != string::npos && start != string::npos);
  if (xmlLine[start + 1] == '/') start++;
  string tag = xmlLine.substr(start + 1, end - start - 1);
  start = end;
  if (xmlLine[end] == ' ') end = xmlLine.find_first_of(">", end);
  return tag;
}

// ____________________________________________________________________________
string Xml2Csv::getText(size_t& start, size_t& end, const string& xmlLine)
{
  // This is neither a new record nor the end of the record. We found some
  // line inbetween like <year>...</year>. We assume that none of the other
  // tags (author, year, ...) have attributes. Therefore we don't look for
  // them.
  string text = "";
  if (xmlLine[end] == '>' && end < xmlLine.size() - 1)
  {
    // Search for the end of the tag (</year>). We already know the
    // beginning.
    size_t textend = xmlLine.find("</", end + 1);
    assert(textend != string::npos);
    text = xmlLine.substr(end + 1, textend - 1 - end);
  }
  return text;
}

// ____________________________________________________________________________
void Xml2Csv::preparseAndSplitLine(string* const text,
                                   vector<string>* const linesOut)
{
  // In some cases we have more than one attribute in one line, which can lead
  // to some errors (</article><proceedings>). The simplest solution to this
  // is splitting the lines between "><", but first it's necessary to delete
  // possible whitespaces between > and <.
  text->erase(text->find_last_not_of(" \r\t")+1);
  *text = boost::regex_replace(*text, boost::regex(">\\s*<"), ">\n<");
  boost::split(*linesOut, *text, boost::is_any_of("\n"));
}

// ____________________________________________________________________________
string Xml2Csv::getAuthorsWithSynonyms(const vector<string>& authors) const
{
  string authorsWithSynonyms;
  for (size_t i = 0; i < authors.size(); i++)
  {
    string author = authors[i];
    vector<string> names;
    // No existing several names.
    if (authorsWithSeveralNamesNameToId.count(author) == 0)
      names.push_back(author);
    // Get several names.
    else
    {
      size_t id = authorsWithSeveralNamesNameToId.at(author);
      assert(id >= 0);
      assert(id < authorsWithSeveralNamesIdToPersonRecord.size());
      names = authorsWithSeveralNamesIdToPersonRecord[id].aliases;
    }
    if (i != 0) authorsWithSynonyms += _withinFieldDelimiter;
    assert(!names.empty());
    authorsWithSynonyms += decodeEntitiesToUtf8(names[0]);
    for (size_t i = 1; i < names.size(); i++)
    {
      authorsWithSynonyms += _withinFieldDelimiter;
      // Store the words, but don't store the name as facet.
      authorsWithSynonyms += _noFacetChar;
      authorsWithSynonyms += decodeEntitiesToUtf8(names[i]);
    }
  }
  return authorsWithSynonyms;
}

// ____________________________________________________________________________
void Xml2Csv::handlePersonInformation(const size_t& id,
                                      const PersonRecord& personInfo)
{
  // Add a vector at the end and fill it with the given authors.
  authorsWithSeveralNamesIdToPersonRecord.resize(id + 1);
  authorsWithSeveralNamesIdToPersonRecord[id] = personInfo;
  // Add all aliases to authorsWithSeveralNamesNameToId using the same id.
  for (size_t i = 0; i < personInfo.aliases.size(); i++)
  {
    string name = personInfo.aliases[i];
    if (authorsWithSeveralNamesNameToId.count(name) > 0)
    {
      cerr << "{!!! WARNING: repeated occurrence of \""
           << name << "\" in person record, will be ignored}" << endl;
      continue;
    }

    // Store the fitting id to each name, so it's easy to get the
    // position within the authorsWithSeveralNamesIdToNames vector.
    authorsWithSeveralNamesNameToId[name] = id;
  }
}

// ____________________________________________________________________________
vector<pair<string, string> > Xml2Csv::getAttributes(const string& buffer)
  const
{
  vector<pair<string, string> > attributes;
  // Get attributes.
  string::const_iterator startit = buffer.begin();
  string::const_iterator endit = buffer.end();

  boost::regex expression("(\\w+)=\\\"(.+?)\\\"");
  boost::match_results<std::string::const_iterator> what;
  boost::match_flag_type flags = boost::match_default;
  while (regex_search(startit, endit, what, expression, flags))
  {
    if (what.size() >= 3)
    {
      pair<string, string> tmp;
      tmp.first =
        buffer.substr(what[1].first - buffer.begin(),
                      what[1].second - buffer.begin() -
                     (what[1].first - buffer.begin()));
      tmp.second =
        buffer.substr(what[2].first - buffer.begin(),
                      what[2].second - buffer.begin() -
                     (what[2].first - buffer.begin()));
      attributes.push_back(tmp);
    }
    startit = what[0].second;
  }
  return attributes;
}

// ____________________________________________________________________________
string Xml2Csv::transformAuthorToDblpUrl(const string& author) const
{
  string link = convertEntitiesToHtmlEntities(author);
  // Replace the space before suffxes like 00002 by an underscore _.
  link = boost::regex_replace(link, boost::regex(" (\\d{4}$)"), "_$1");
  // The last space indicates the surname.
  size_t lastSpace = link.find_last_of(" ");
  if (lastSpace != string::npos)
    link = link.substr(lastSpace + 1) + ":" + link.substr(0, lastSpace);
  // Replace spaces by an underscore.
  link = boost::regex_replace(link, boost::regex(" "), "_");
  // Replace nonalphanums by =.
  link = boost::regex_replace(link, boost::regex("[^\\w:]"), "=");
  // Add base url.
  string base = _baseUrl + "pers/hc/";
  base += tolower(link[0]);
  link = base + "/" + link;
  return link;
}

// ____________________________________________________________________________
void Xml2Csv::rewriteRecordEntries(PublicationRecord * const rec)
{
  // EE
  if (rec->ee.size() > 0 &&
      rec->ee.compare(0, 7, "http://") != 0 &&
      rec->ee.compare(0, 6, "ftp://") != 0)
  {
    rec->ee = kBaseUrl + rec->ee;
  }

  // KEY
  rec->key = kBaseUrl + string("rec/bibtex/") + rec->key;

  // VENUE URL
  if (rec->url.size() > 0 &&
      rec->url.compare(0, 7, "http://") != 0 &&
      rec->url.compare(0, 6, "ftp://") != 0)
  {
    rec->url = kBaseUrl + rec->url;
  }

  // VENUE
  if (rec->venue == "Fast Software Encryption")
    rec->venue = "FSE";
  else if (rec->venue == "Symposium on Computational Geometry")
    rec->venue = "SoCG";
  else if (rec->venue == "SIGMOD Conference")
    rec->venue = "SIGMOD";
  else if (rec->venue == "ICPC" || rec->venue == "IWPC" || rec->venue == "WPC")
    rec->venue = "ICPC/IWPC";
  else if (rec->venue == "FoSSaCS")
    rec->venue = "FOSSACS";
}

// ____________________________________________________________________________
string Xml2Csv::createDblpRecord(const PublicationRecord& rec) const
{
  // return "<![CDATA[" + rec.dblpRecord + "]]>";
  // if (_fillWithJson) return escapeJson(rec.dblpRecord);
  return rec.dblpRecord;
}

// ____________________________________________________________________________
string Xml2Csv::createHtmlRecord(const PublicationRecord& rec) const
{
  // Color depending in the TYPE
  string type2color = "#ccccff";
  if (rec.type == "inproceedings") type2color = "#ccccff";
  else if (rec.type == "article") type2color = "#ffcccc";
  else if (rec.type == "book") type2color = "#ccffff";
  else if (rec.type == "proceedings") type2color = "#ccffff";
  else if (rec.url.find("journals/corr") != string::npos)
    type2color = "#cccccc";

  string ee = (rec.ee != "")
    ? "<td bgcolor=\"#ccffcc\" valign=\"top\">"
      "<a href=\"" + rec.ee + "\">EE</a></td>"
    : "<td>&nbsp;&nbsp;&nbsp;</td>";
  string venue = "";
  // VENUE article
  // VENUE informal
  // VENUE inproceedings
  // VENUE editor
  // VENUE incollection
  // VENUE book
  venue = (rec.url != "")
    ? "<a href=\"" + rec.url + "\">" + venue + "</a>"
    : venue;

  // AUTHORS
  string authorList = "";
  for (size_t i = 0; i < rec.authors.size(); i++)
  {
    if (i != 0) authorList += ", ";
    authorList += "<a href=\"" + transformAuthorToDblpUrl(rec.authors[i])
               + "\">" + decodeEntitiesToUtf8(rec.authors[i])
               + "</a>";
    if (i == rec.authors.size() - 1) authorList += ": ";
  }

  string html = "<tr>";
  html += "<td align=\"right\" valign=\"top\" bgcolor=\"" + type2color + "\""
        + "<a href=\"" + rec.key + "\">%INDEX</a><td>" + ee
        + "<td>" + authorList + rec.title + " " + venue + "</td>"
        + "</tr>";
  // if (_fillWithJson) return escapeJson(html);
  return html;
}

// ____________________________________________________________________________
string Xml2Csv::createXmlRecord(const PublicationRecord& rec) const
{
  return rec.dblpRecord;
}

// ____________________________________________________________________________
void Xml2Csv::parse(const string& basename)
{
  // Open xml file.
  string xmlpath = basename + ".xml";
  ifstream xml;
  xml.open(xmlpath.c_str(), ios::in);
  if (!xml.is_open())
  {
    cout << "Could not open " << xmlpath << "." << endl;
    exit(1);
  }

  // Create / override csv file.
  string csvpath = basename + ".csv";
  ofstream csv;
  csv.open(csvpath.c_str(), ios::out);
  if (!csv.is_open())
  {
    cout << "Could not open " << csvpath << "." << endl;
    exit(1);
  }

  csv << "authorname\tauthor\tfirstauthor\tna\ttitle\tvenue\tyear\ttype"
      << "\tdblp-record\thtml-record\txml-record\n";

  // Start parsing.
  unsigned int lineNumber = 0;
  unsigned int id = 0;
  string startTagName;
  string buffer;
  Timer timer;

  // Structs holding the different information.
  PersonRecord person;
  PublicationRecord publication;

  // Booleans to mark which area we are parsing:
  bool isPersonsArea = false;
  bool isLegendArea = false;
  bool isPublicationsArea = false;
  bool isRecord = false;

  cout << "Start parsing of " << xmlpath << "." << endl;
  timer.start();

  // Jump over the xml version line.
  if (!xml.eof()) getline(xml, buffer);

  while (!xml.eof())
  {
    lineNumber++;
    vector<string> lines;

    getline(xml, buffer);

    preparseAndSplitLine(&buffer, &lines);
    if (buffer.size() == 0) continue;

    for (size_t k = 0; k < lines.size(); k++)
    {
      buffer = lines[k];

      // Get tag name, attributes and text.
      size_t start = 0, end = 0;
      string tagName = getTagName(start, end, buffer);
      vector<pair<string, string> > attributes =
        getAttributes(buffer.substr(start + 1, end - (start + 1)));
      string text = getText(start, end, buffer);

      // Mark the start and end of each area.
      if (tagName == kCollectionNameTag) continue;
      else if (tagName == kPersonsTag)
      {
        if (isPersonsArea == false)
        {
          isPersonsArea = true;
          timer.mark();
          cout << "Parsing persons area ... " << flush;
        }
        else
        {
          isPersonsArea = false;
          timer.stop();
          cout << "in " << timer.usecs_since_mark()/1000000.0
               << " seconds." << endl;
          timer.cont();
        }
        continue;
      }
      else if (tagName == kLegendTag)
      {
        if (isLegendArea == false)
        {
          isLegendArea = true;
          cout << "Parsing legend area ... " << endl;
        }
        else
        {
          isLegendArea = false;
        }
        continue;
      }
      else if (tagName == kPublicationsTag)
      {
        if (isPublicationsArea == false)
        {
          isPublicationsArea = true;
          timer.mark();
          cout << "Parsing publications area ... " << flush;
        }
        else
        {
          isPublicationsArea = false;
          timer.stop();
          cout << "in " << timer.usecs_since_mark()/1000000.0
               << " seconds." << endl;
          timer.cont();
        }
        continue;
      }

      // PERSONS
      if (isPersonsArea)
      {
        // Find start and end of a person record.
        if (tagName == kPersonTag)
        {
          // Start of PERSON.
          if (isRecord == false)
          {
            isRecord = true;
            // Parse attributes.
            for (unsigned int i = 0; i < attributes.size(); i++)
            {
              pair<string, string>& att = attributes[i];
              if (att.first == "key") person.key = att.second;
              // Do nothing right now.
              else if (att.first == "mdate") { }
              else
                cerr << "Unknown attribute in persons area: "
                     << att.first << "." << endl;
            }
          }
          // End of PERSON.
          else
          {
            handlePersonInformation(id, person);
            id++;
            person.clear();
            isRecord = false;
          }
        }
        else if (tagName == "author") person.aliases.push_back(text);
        else if (tagName == "url")    person.urls.push_back(text);
        else if (tagName == "note" || tagName == "cite") { }
        else if (tagName == "r") { }
        else
          cerr << "Unknown tag in persons area: " << tagName << "." << endl;
        continue;
      }

      // LEGEND
      if (isLegendArea)
      {
        if (tagName == kSectionTag)
        {
          assert(attributes.size() == 1);
          sectionToType[attributes[0].second] = text;
        }
        else
          cerr << "Unknown tag in legend area: " << tagName << "." << endl;
        continue;
      }

      // PUBLICATIONS
      if (isPublicationsArea)
      {
        // Find start and end of record.
        if (tagName == "r")
        {
          // Start of Record r.
          if (isRecord == false)
          {
            isRecord = true;
            // Parse attributes.
            for (unsigned int i = 0; i < attributes.size(); i++)
            {
              pair<string, string>& att = attributes[i];
              // Set type.
              if (att.first == "section")
              {
                // publication.typeAsKey = att.second;
                publication.type = sectionToType[att.second];
              }
              // Do nothing right now.
              else if (att.first == "mdate") { }
              else
                cerr << "Unknown attribute in publications area: "
                     << att.first << "." << endl;
            }
          }
          // End of Record r.
          else
          {
            rewriteRecordEntries(&publication);
            string authorsWithSynonyms
              = getAuthorsWithSynonyms(publication.authors);
            string firstAuthor = "";
            if (publication.authors.size() > 0)
            {
              vector<string> tmpVec;
              tmpVec.push_back(publication.authors.front());
              firstAuthor = getAuthorsWithSynonyms(tmpVec);
            }
            // Write to CSV.
            csv << "\t" << authorsWithSynonyms            // author
                << "\t" << firstAuthor                    // firstAuthor
                << "\t" << publication.authors.size()    // number of authors
                << "\t" << publication.title              // title
                << "\t" << publication.venue          // venue
                << "\t" << publication.year           // year
                << "\t" << publication.type           // type
                << "\t" << createDblpRecord(publication)  // dblp-record
                << "\t" << createHtmlRecord(publication)  // html-record
                << "\t" << createXmlRecord(publication)   // xml-record
                << endl;
            publication.clear();
            isRecord = false;
          }
        }
        // Start of standard dblp record.
        else if (startTagName.empty())
        {
          startTagName = tagName;
          // Parse attributes.
          for (unsigned int i = 0; i < attributes.size(); i++)
          {
            pair<string, string>& att = attributes[i];
            if (att.first == "key") publication.key = att.second;
            // Do nothing right now.
            else if (att.first == "publtype" || att.first == "mdate"
                    || att.first == "rating" || att.first == "reviewid") { }
            else
              cerr << "Unknown attribute in publications area: "
                   << att.first << "." << endl;
          }
        }
        // End of standard dblp record.
        else if (startTagName == tagName) startTagName.clear();
        // Fill buffers.
        else if (tagName == "author" || tagName == "editor")
        {
          publication.authors.push_back(text);

          // If there is no person record entry for this person, add one.
          if (authorsWithSeveralNamesNameToId.count(text) == 0)
          {
            PersonRecord tmp;
            tmp.aliases.push_back(text);
            authorsWithSeveralNamesNameToId[text] = id;
            authorsWithSeveralNamesIdToPersonRecord.push_back(tmp);
            id++;
          }
        }
        else if (tagName == "title")
          publication.title = decodeEntitiesToUtf8(text);
        else if (tagName == "journal")
          publication.venue = decodeEntitiesToUtf8(text);
        else if (tagName == "booktitle")
          publication.venue = decodeEntitiesToUtf8(text);
        else if (tagName == "year")
          publication.year = decodeEntitiesToUtf8(text);
        else if (tagName == "ee")
          publication.ee = decodeEntitiesToUtf8(text);
        else if (tagName == "series")
          publication.series = decodeEntitiesToUtf8(text);
        else if (tagName == "number")
          publication.number = decodeEntitiesToUtf8(text);
        else if (tagName == "volume")
          publication.volume = decodeEntitiesToUtf8(text);
        else if (tagName == "pages")
          publication.pages = decodeEntitiesToUtf8(text);
        else if (tagName == "publisher")
          publication.publisher = decodeEntitiesToUtf8(text);
        else if (tagName == "school")
          publication.publisher = decodeEntitiesToUtf8(text);
        else if (tagName == "url")
          publication.url = decodeEntitiesToUtf8(text);
        else if (tagName == "month")
          publication.month = decodeEntitiesToUtf8(text);
        else if (tagName == "isbn")
          publication.isbn = decodeEntitiesToUtf8(text);
        else if (tagName == "crossref")
          publication.crossref = decodeEntitiesToUtf8(text);
        else if (tagName == "note")
          publication.note = decodeEntitiesToUtf8(text);
        else if (tagName == "cite")
          publication.cite = decodeEntitiesToUtf8(text);
        else if (tagName == "cdrom")
          publication.cdrom = decodeEntitiesToUtf8(text);
        else
          cerr << "Unknown tag in publications area: " << tagName << "." << endl;
        if (tagName != "r") publication.dblpRecord += buffer;
        continue;
      }
      cerr << "Unknown tag: " << tagName << "." << endl;
    }
  }
  // Add each author with aliases as authorname to csv.
  cout << "Adding authornames to csv ... " << flush;
  timer.mark();
  for (unsigned int i = 0; i < authorsWithSeveralNamesIdToPersonRecord.size();
       i++)
  {
    PersonRecord& person = authorsWithSeveralNamesIdToPersonRecord[i];
    string authornames;
    for (unsigned int j = 0; j < person.aliases.size(); j++)
    {
      if (j > 0)
      {
        authornames += _withinFieldDelimiter;
        authornames += _noFacetChar;
      }
      authornames += decodeEntitiesToUtf8(person.aliases[j]);
    }
    csv << authornames << "\t\t\t\t\t\t\t\t\t\t\n";
  }
  timer.stop();
  cout << "in " << timer.usecs_since_mark()/1000000.0
                << " seconds." << endl;
  cout << "Time at all: " << timer.secs() << " seconds." << endl;
  csv.close();
  xml.close();
}

void usageInfo(string nameOfProg)
{
  cout << nameOfProg << " " << "<basename>" << endl;
  exit(1);
}

int main(int argc, char** argv)
{
  Xml2Csv xml2csv;
  string basename = "";
  optind = 1;
  static struct option longOptions[] =
  {
    {"base-name", 1, NULL, 'b'},
    {"within-field-separator", 1, NULL, 's'},
    {"no-show-facet-prefix"  , 1, NULL, 'p'},
    {"fill-fields-with-json" , 0, NULL, 'j'},
    {NULL      , 0, NULL,   0}
  };
  while (true)
  {
    int c = getopt_long(argc, argv, "b:s:p:j", longOptions, NULL);
    if (c == -1) break;
    switch (c)
    {
      case 'b': basename = optarg; break;
      case 's': xml2csv.setWithinFieldDelimiter(optarg[0]); break;
      case 'p': xml2csv.setNoFacetCharacter(optarg[0]); break;
      case 'j': xml2csv.setFieldFormatToJson(); break;
      default: usageInfo(argv[0]);
    }
  }
  if (basename.empty()) usageInfo(argv[0]);

  xml2csv.parse(basename);
}
