// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#include "./Query.h"
#include <vector>
#include <string>
#include <utility>
#include "./Globals.h"
#include "./WordRange.h"
#include "./Separator.h"
#include "./ConcurrentLog.h"
#include "../utility/WkSupport.h"

bool normalizeWords;

// _____________________________________________________________________________
void Query::normalize()
{
  unsigned int i = 0;
  while (i < _queryString.length() && _queryString[i] == ' ') ++i;
  unsigned int j = _queryString.length();
  while (j > i && _queryString[j - 1] == ' ') --j;
  if (i > 0 || j < _queryString.length())
    _queryString = _queryString.substr(i, j - i);
}


// _____________________________________________________________________________
string Query::normalizeQueryPart(string queryPart)
{
  // TODO(bast): optionally normalize the query here.
  // return(queryPart);

  // NEW Hannah+Ina 8Feb13: Alternative <alternativlos> for dealing with "..."
  // in queries. See CompleterBase.cpp:237 8Feb13.
  //
  // Before any processing, mask all non-word characters within "..." by
  // $<ascii hex code> (or any other special symbol than $). See
  // CompleterBase::processBasicQuery.
  //
  // At this point here, re-convert the $<...> to the respective character.

  if (normalizeWords == false)
  {
    queryPart  = globalStringConverter.convert(queryPart,
                                      StringConverter::ENCODING_UTF8,
                                      StringConverter::CONVERSION_FROM_MASK);
    return queryPart;
  }

  vector<string> queryPartsList;
  size_t n = queryPart.find("--");
  if (n != string::npos)
  {
    // queryPart is a range query like:
    // :filter:Kategorie:Comp*--:filter:Kategorie:Ele*"
    queryPartsList.resize(2);
    queryPartsList[0] = queryPart.substr(0, n);
    queryPartsList[1] = queryPart.substr(n + 2, string::npos);
  }
  else
  {
    queryPartsList.push_back(queryPart);
  }

  vector<string> queryParts;
  string         head;
  string         middle;
  string         tail;
  string         normalizedQueryPart;
  bool           lastPartIsOriginal = false;
  for (size_t n = 0; n < queryPartsList.size(); n++)
  {
    putTokenInVector(queryPartsList[n], wordPartSep, &queryParts);

    if (queryParts.size() == 1)
    {
      // Expl.:
      // queryPart  = "COST*";
      // --> head   = ""
      //     middle = ""
      //     tail   = "COST*"
      tail = queryParts[0];
    }
    else if (queryParts.size() == 2)
    {
      // Expl.:
      // queryPart  = "query:*"; (to get qury:query)
      // --> head   = ""
      //     middle = "query"
      //     tail   = ":*"
      if (!queryParts[0].empty())
      {
        lastPartIsOriginal = true;
        middle = queryParts[0];
        tail   = wordPartSep + queryParts[1];
      }
      // Expl.:
      // queryPart  = ":fil*";
      // --> head   = ":fil*"
      //     middle = ""
      //     tail   = ""
      else
        head = queryParts[0] + wordPartSep + queryParts[1];
    }
    else if (queryParts.size() == 3)
    {
      // Expl.:
      // queryPart  = ":filter:Nat*";
      // --> head   = ":filter"
      //     middle = ":Nat*"
      //     tail   = ""
      // OR: C:32:*
      head   = queryParts[0] + wordPartSep + queryParts[1];
      middle = wordPartSep + queryParts[2];
    }
    else if (queryParts.size() == 4)
    {
      // Expl.:
      // queryPart  = ":filter:Nation:COST*";
      // --> head   = ":filter"
      //     middle = ":Nat*" 
      //     tail   = ":COST*"
      // NEW 16Dec11 Ina: Added queryParts[0] to head. It was missing and
      // therefore facets could not be found:
      // ce:author:christoph_meinel:* was splitted normalized to :author:christoph_meinel:*
      // instead of ce:author:christoph_meinel:*
      head   = queryParts[0] + wordPartSep + queryParts[1];
      middle = wordPartSep + queryParts[2];
      tail   = wordPartSep  + queryParts[3];
      if (!queryParts[0].empty())
      {
        // Expl.:
        // queryPart  = "ce:author:christoph_meinel:*";
        // --> head   = "ce:author"
        //     middle = ":christoph_meinel 
        //     tail   = ":*"
        tail = middle + tail;
        middle = "";
        lastPartIsOriginal = true;
        // Expl.:
        // queryPart  = "query:C:123:*";
        // --> head   = ""
        //     middle = "query"  --> normalize
        //     tail   = ":C:123:**"
        if (queryParts[1] == "C" || queryParts[1] == "S")
        {
          head   = "";
          middle = queryParts[0];
          tail   = wordPartSep + queryParts[1]
                 + wordPartSep + queryParts[2]
                 + wordPartSep + queryParts[3];
        }
      }
    }
    else if (queryParts.size() == 5)
    {
      // Expl.:
      // queryPart  = ":filter:title:query:*";
      // --> head   = ":filter:title:"
      //     middle = ":query*" --> to normalize
      //     tail   = ":*"
      lastPartIsOriginal = true;
      head   = queryParts[0]
             + wordPartSep + queryParts[1]
             + wordPartSep + queryParts[2];
      middle = wordPartSep + queryParts[3];
      tail   = wordPartSep + queryParts[4];
    }
    else if (queryParts.size() == 6)
    {
      // Expl.:
      // queryPart  = ":filter:Nation:C:123:*";
      // --> head   = ":filter"
      //     middle = ":Nation:C:123"
      //     tail   = ":*"
      head   = wordPartSep + queryParts[1];
      middle = wordPartSep + queryParts[2]
             + wordPartSep + queryParts[3]
             + wordPartSep + queryParts[4];
      tail   = wordPartSep + queryParts[5];
    }
    else if (queryParts.size() == 7)
    {
      // Expl.:
      // queryPart  = ":filter:title:C:123:query:*";
      // --> head   = ":filter:Nation:C:123"
      //     middle = ":query" --> to normalize
      //     tail   = ":*"
      lastPartIsOriginal = true;
      head   = wordPartSep + queryParts[1]
             + wordPartSep + queryParts[2]
             + wordPartSep + queryParts[3];
      middle = wordPartSep + queryParts[4];
      tail   = wordPartSep + queryParts[5];
    }
    else
    {
      tail = queryPart;
    }

    // We normalize always only tail but not for facet query parts
    if (head.find(wordPartSep + string("facet")) == string::npos
	&& !lastPartIsOriginal)
    {
      tail = globalStringConverter.convert(tail,
                                           StringConverter::ENCODING_UTF8,
                                           StringConverter::CONVERSION_TO_NORM);
    }
    else if (lastPartIsOriginal)
    {
      middle = globalStringConverter.convert(middle,
                                           StringConverter::ENCODING_UTF8,
                                           StringConverter::CONVERSION_TO_NORM);
    }

    if (n == 1) normalizedQueryPart += "--";
    normalizedQueryPart += head + middle + tail;
  }  // for (size_t n = 0; n < queryPartsList.size(); n++)

  // LOG << "NEW! NORMALIZE QUERY PART: "
  //     << "\"" << queryPart << "\" -> \"" << normalizedQueryPart << "\""
  //     << endl;
  normalizedQueryPart = globalStringConverter.convert(normalizedQueryPart,
                                      StringConverter::ENCODING_UTF8,
                                      StringConverter::CONVERSION_FROM_MASK);

  LOG << IF_VERBOSITY_HIGHEST
      << "! queryPart = \"" << queryPart << "\" "
      << "; head = \""  << head << "\""
      << "; middle = \""  << middle << "\""
      << "; tail = \""  << tail << "\""
      << "; normalizedQueryPart = \"" << normalizedQueryPart << "\""
      << endl;

  return normalizedQueryPart;
}



//! Split at last separator, returning last word
string Query::splitAtLastSeparator() const
{
  Query firstPart, lastPart;
  Separator splitSeparator;
  return splitAtLastSeparator(&firstPart, &lastPart, &splitSeparator) == true ?
    lastPart.getQueryString() : string("");
}

//! Split at last separator, returning last word AND first part
bool Query::splitAtLastSeparator(Query* firstPart,
                                 Query* lastPart,
                                 Separator* separator) const
{
  LOG << AT_BEGINNING_OF_METHOD << IF_VERBOSITY_HIGHEST << "; query is \""
      << *this << "\"" << endl;

  if (_queryString.size() == 0)
  {
    (*separator) =  Separator();
    // cout << "! SPLITTING EMPTY STRING " << endl << flush;
    (*firstPart) = Query("");
    (*lastPart) = Query("");
    return false;
  }

  // GO THROUGH STRING FROM END AND LOOK FOR SEPARATOR CHARS
  string::const_iterator sepPosBegin = _queryString.begin();
  string::const_iterator sepPosEnd = _queryString.end();
  bool sepFound = false;
  string::const_iterator it = _queryString.end();
  while (it >= _queryString.begin())  // does this include the first character?
  {
    if ((it < _queryString.end()) && (SEP_CHARS.index(*it) != -1))
    {
      if (!sepFound) {sepPosEnd = it;}
      sepPosBegin = it;
      sepFound = true;
      LOG << IF_VERBOSITY_HIGHEST << "! found separator: \"" << *it << "\""
          << endl;
    }
    else
    {
      if (sepFound)
      {
        assert(sepPosBegin <= sepPosEnd);
        break;
      }
    }
    it--;
  }
  if (sepFound == false)
    LOG << IF_VERBOSITY_HIGHEST << "! no separator found" << endl;

  // Identify first part and last part; without separator, first part is
  // empty and last part is whole query
  string sepString;
  if (sepFound)
  {
    (*firstPart) = Query(
        _queryString.substr(0, sepPosBegin - _queryString.begin()));
    (*lastPart) = Query(
        _queryString.substr(1 + sepPosEnd - _queryString.begin(),
                            _queryString.end() - _queryString.begin()));
    sepString = _queryString.substr(sepPosBegin - _queryString.begin(),
                                    1 + sepPosEnd - sepPosBegin);
    assert(sepString != "");
  }
  else
  {
    (*firstPart) = Query("");
    (*lastPart) = Query(_queryString);
  }

  // Remove redundant whitespace at the beginning; TODO: what about the end?
  size_t pos = sepString.find_first_not_of(" \t");
  if (pos > 0 && pos != string::npos) sepString = sepString.substr(pos);

  LOG << IF_VERBOSITY_HIGHEST
      << "! first part = \"" << (*firstPart) << "\" "
      << "; last part = \""  << (*lastPart) << "\""
      << "; separator = \"" << sepString << "\" (after whitespace removal)"
      << endl;

  // MAP THE SEQUENCE OF SEP CHARS TO A SEPARATOR ID (unsigned char)
  if (sepFound)
  {
    unsigned char indexInFixedSeparators = (unsigned char) -1;
    for (unsigned char i = 0; i < fixed_separators._separators.size(); i++)
    {
      if (fixed_separators._separators[i]._separatorString == sepString)
      {
        indexInFixedSeparators = i;
        (*separator) = fixed_separators._separators[i];
        break;
      }
    }
    if (indexInFixedSeparators == (unsigned char) -1)
    {
      // TODO(foobar): check for special separator string
      // used as a class method, not as an object method
      if ((*separator).parseFlexiSeparatorString(sepString, (*separator)))
      {
        sepFound = true;
      }
      else
      {
        LOG << "! Invalid separator string: \"" << sepString <<"\"" << endl;
        sepFound = false;
      }
    }
  }  // end if: found some separator characters
  else
  {
    (*separator) = Separator("", pair<signed int, signed int>(-1, -1), FULL);
  }

  // NEW 06Apr08: check for NOT operator
  if ((*firstPart)._queryString.size() > 0)
  {
    LOG << IF_VERBOSITY_HIGH
        << "! checking for modifiers NOT or OPTIONAL; "
        << "first part = \"" << (*firstPart)._queryString << "\", "
        << "last part = \"" << (*lastPart)._queryString  << "\"" << endl;
    string& last = (*lastPart)._queryString;
    if (last.length() >= 2 && last[0] == NOT_QUERY_SEP)
    {
      last.erase(0, 1);
      (*separator).setOutputMode(Separator::OUTPUT_NON_MATCHES);
      LOG << IF_VERBOSITY_HIGH
          << "! modifier NOT found; last part now \"" << last << "\" "
          << "; separator now " << (*separator).infoString() << endl;
    }
    else if (last.length() >= 2 && last[0] == '(')
    {
      last = last.substr(1);
      if (last.length() >= 2 && last[last.length() - 1] == ')')
        last.erase(last.length() - 1);
      (*separator).setOutputMode(Separator::OUTPUT_ALL);
      LOG << IF_VERBOSITY_HIGH
          << "! modifier OPTIONAL found; last part now \"" << last << "\" "
          << "; separator now " << (*separator).infoString() << endl;
    }
  }

  return sepFound;
}  // end: splitAtLastSeparator

// Rewrite query word.
void Query::rewriteQuery()
{
  // NEW Hannah+Ina 8Feb13: Alternative <alternativlos> for dealing with "..."
  // in queries.
  // Idea: Sometimes we don't want special characters to be handled like
  // separators, e.g. the dot Philip_S._Yu. Such characters can be marked
  // by quoting: Philip_S"."_Yu (or another quote containing the dot).
  // To make sure that those characters are not handled as separators,
  // we just mask them as long as possible.
  //
  // At this point here, mask all non-word characters within "..." by
  // $<ascii hex code> (or any other special symbol than $). Just go over the
  // string once from left to right, and maintainin a bit holding the parity of
  // the number of " seen so far (odd or even).

  // Encode quoted parts.
  _queryString = encodeQuotedQueryParts(_queryString);

  if (!normalizeWords) {
/*    size_t pos = 0;
    while ((pos = _queryString.find('$', pos)) != string::npos)
      _queryString.erase(pos, 1);*/
    return;
  }
  string word;
  string query;
  query.reserve(_queryString.size());
  for (size_t i = 0; i < _queryString.size(); i++) {
    const char& c = _queryString[i];
    // Chars, which might occur in a query word are:
    // alphanums + any Umlaute, etc., % and the wordPartSep.
    // Separators are all ascii letters beside alphanums, % and the wordPartSep.
    int codepoint = static_cast<int>(static_cast<unsigned char>(c));
    // Iff separator.
    if (!isalnum(c) && codepoint < 128
	&& c != '%' && c != wordPartSep && c != '_' && c != '"') {
      if (c == '*') {
	if (word.empty()) continue;
	query.append(word);
	// NEW (baumgari) 29May14: Actually we do not need the $ to be able to
	// find e.g. müller (and not müller, mueller, muller and müeller). This
	// can be done by surrounding the tricky letters by quotes.
     /* } else if (c == '$') {
	string wordToLower = globalStringConverter.convert(word,
                                      StringConverter::ENCODING_UTF8,
                                      StringConverter::CONVERSION_TO_LOWER);
	string wordToNorm = globalStringConverter.convert(word,
                                      StringConverter::ENCODING_UTF8,
                                      StringConverter::CONVERSION_TO_NORM);
	if (wordToLower != wordToNorm) query.append(wordToLower + wordPartSep + wordToLower);
	else query.append(word);
        word.clear();
	// Erase $.
	continue;*/
      } else if (!word.empty()) {
        // Single word without asterix. Append both versions.
        query.append(word + wordPartSep + "*");
        query.append(1, OR_QUERY_SEP);
        query.append(word);
      }
      word.clear();
      query.append(1, c);
    } else {
      word.append(1, c);
    }
  }
  if (!word.empty()) {
    // Single word without asterix. Append both versions.
    query.append(word + wordPartSep + "*");
    query.append(1, OR_QUERY_SEP);
    query.append(word);
  }
  _queryString = query;
  if (query != _queryString)
    LOG << IF_VERBOSITY_HIGH << "* NEW: Rewrote query from \""
        << _queryString << "\" to \"" << query << "\"" << endl;
}


//! Check if only word; TODO: misnomer
bool Query::isRightmostPart() const
{
  return _queryString.rfind(" ", _queryString.length()-1) == string::npos;
}


//! Check whether _queryString matches an xml-tag.
/*! \param     _tag     An xhtml-tag
 *  \param     isCompleteWord   Indicates if _queryString is to be taken as a complete word or a prefix
 *  \return   \c true iff _queryString matches the xml-tag \a _tag
 *  \pre     \a _tag is a valid xml-tag. If not fulfilled, the return value is undefined.
 *
 *  If \a isCompleteWord == \c true, then _queryString matches \a _tag if either \<_queryString\> or
 *  \</_queryString\> is equal to \a _tag. <BR>
 *  Otherwise, _queryString matches \a _tag if _queryString is a prefix of a string \a s, such that either
 *  \<\a s\> or \</\a s\> is equal to \a _tag.
 */
bool Query::matchesTag(const string& _tag, const bool isCompleteWord) const
{
  size_t start = _tag.find_first_not_of("</");
  if (start != 1 && start != 2) return false;
  std::string tag = _tag.substr(start, _tag.length()-start-1);
  return (isCompleteWord ? _queryString == tag : tag.find(_queryString) == 0);
}

//! Split Query at two or more dots.
/*! \return A vector containing the resulting parts
 *
 *  Splits _queryString at each occurrence of two or more dots.
 *  The returned parts have the same separators and keyword-expression as _queryString.
 */
std::vector<Query> Query::splitAtDots() const
{
  std::vector<Query> parts;
  Query part;
  size_t i = 0, j = 0;
  while (i < _queryString.length() && j < _queryString.length())
  {
    j = _queryString.find("..", i);
    part = Query(_queryString.substr(i, j-i));
    if (!part.empty())
    {
      ////    part._separators = _separators;
      ////    part._keywordRegexp = _keywordRegexp;
      parts.push_back(part);
    }
    if (j != std::string::npos)
    {
      i = j + 1;
      while (i < _queryString.length() && _queryString[i] == '.') i++;
    }
  }
  return parts;
}  // end: splitAtDots

//! Check whether _queryString is a keyword.
/*! \return \c true, iff _queryString matches the regular expression for keywords.
 */
bool isKeyword()
{
  // TODO(foobar): KEYWORDS ARE NOT YET SUPPORTED
  return false;
}



//! Split Query at a character.
/*! \param     c   The character to split at
 *  \return   A vector containing the resulting parts
 *
 *  The returned parts have the same separators and keyword-expression as _queryString.
 *  This function is currently (31Jul2006) only used in ExcerptsGenerator.h
 */
//    std::vector<Query> splitAt(const char& c) const
std::vector<Query> Query::splitAt(const string& c) const
{
  std::vector<Query> parts;
  Query part;

  size_t i = 0, j = 0;
  while (i < _queryString.length() && j < _queryString.length())
  {
    i = _queryString.find_first_not_of(c, j);
    if (i != std::string::npos)
    {
      j = _queryString.find_first_of(c, i);
      part = Query(_queryString.substr(i, j-i));
      if (!part.empty())
      {
  parts.push_back(part);
      }
    }
  }
  return parts;
}  // end: splitAt


//! Remove strange characters from query, to make highlighting easier
void Query::cleanForHighlighting()
{
  // Remove parantheses and newlines
  string::iterator it = _queryString.begin();
  while (it != _queryString.end())
    if (*it == ENHANCED_QUERY_BEG ||
        *it == ENHANCED_QUERY_END ||
        *it == ENHANCED_QUERY_SEP ||
        *it == '('                ||
        *it == ')'                  ||
        *it == '\n')
      it = _queryString.erase(it);
    else
      it++;

  // Chop off trailing space characters
  unsigned int pos = _queryString.length();
  while (pos > 0 && _queryString[pos - 1] == ' ') --pos;
  if (pos < _queryString.length()) _queryString.erase(pos);

  // HACK(bast): also chop off a trailing ~, so that the highlighting also works
  // for fuzzy search queries, at least for the parts of the rewritten OR query
  // that can be highlighted (won't work for the C:... words obviously).
  if (getLastCharacter() == '~') removeLastCharacter();
}


//! Output operator; simply outputs the query string
ostream& operator<<(ostream& os, const Query& query)
{
  os << query._queryString;
  return os;
}
