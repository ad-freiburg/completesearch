// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright 2010

#ifndef SERVER_QUERY_H__
#define SERVER_QUERY_H__

#include <gtest/gtest.h>
#include <fstream>
#include <vector>
#include <string>
#include "./Globals.h"
#include "./ConcurrentLog.h"

class Separator;

/** Class that holds a query string and provides parsing functionality like
 *  split into words etc.
 *
 *  Derived from std::string.
 *  @todo: Better have a member of type \c std::string.
 *
 *  Search strings may contain so-called separators and are able to split
 *  themselves at these separarors. They may also represent keywords. Keywords
 *  are defined by regular expressions.
 *  [This comment is still from Thomas, and therefore probably outdated.]
 */
class Query
{
 private:

  //! The query string
  string _queryString;

 public:
  
  // Dump object to string. 
  std::string asString(void)  const { return _queryString; }

  //! The default constructor.
  FRIEND_TEST(QueryTest, constructor);
  Query() {}

  //! Construct a query from a string.
  Query(const string& s) { _queryString = s; }
  // Query(const string& s) { *((string*)this) = s; }

  //! Destructor; does nothing so far
  FRIEND_TEST(QueryTest, destructor);
  ~Query() {}

  //! Set query string
  FRIEND_TEST(QueryTest, setQueryString);
  void setQueryString(const string& s) { _queryString = s; }

  //! Get (const) query string
  const string& getQueryString() const { return _queryString; }

  //! Get length of query string
  unsigned int length() const { return _queryString.length(); }

  // Size of object in bytes.
  size_t sizeInBytes() const { return _queryString.length(); }

  //! Return true iff empty query
  bool empty() const { return _queryString.empty(); }

  //! Get last character of query string, 0 if empty
  char getLastCharacter() const
  {
    return _queryString.length() > 0 ? _queryString[_queryString.length() - 1]
                                     : 0;
  }

  //! Chop off last character of query string (if empty, do nothing)
  void removeLastCharacter()
  {
    if (_queryString.length() > 0)
      _queryString.erase(_queryString.length() - 1);
  }

  //! Append given suffix.
  void append(const string& suffix) { _queryString += suffix; }

  //! Normalize query; just strips off leading and trailing whitespaces.
  void normalize();

  /** Normalize part of a query pertaining to a word range (either a single
   *  word, a prefix with a *, or a range query of the form q1--q2). This is
   *  called in CompleterBase::prefixToRange.
   *  @todo(bast): Currently this does nothing, that is, it just returns the
   *  string itself. This is the place for Wolfgang Bartsch to do his
   *  normalization.
   */
  static string normalizeQueryPart(string queryPart);

  //! Split at last separator, returning last word.
  //! @todo misnomer, since it does not affect query
  string splitAtLastSeparator() const;

  //! Split at last separator, returning last word AND first part.
  bool splitAtLastSeparator(Query* firstPart,
                            Query* lastPart,
                            Separator* splitSeparator) const;

  // NEW (baumgari) 24Apr14
  //! Rewrite query.
  //
  /* Right now it's used for the following cases:
   * - Mask special chars in the query, which are surrounded by quotes.
   * - If simple query, rewrite single words from word to word|word:* to be able
   * to find normalized words (e.g. müller -> müller|müller:*. Otherwhise
   * only muller would be found.
   * - If exact match (specified by a $ at the end of a word) look up only
   *   muller (if muller$) or muller:müller (if müller$) to find only the exact
   *   completion.
   */
  void rewriteQuery();

  //! Check if only one word.
  //! @todo misnomer
  bool isRightmostPart() const;

  /** Check whether \c *this matches an xml-tag.
   *
   *  \param  _tag            An xhtml-tag
   *  \param  isCompleteWord  Indicates if \c *this is to be taken as a
   *                          complete word or a prefix
   *  \return                 \c true iff \c *this matches the xml-tag \a _tag
   *  \pre                    \a _tag is a valid xml-tag. If not fulfilled,
   *                          the return value is undefined.
   *
   *  If \a isCompleteWord == \c true, then \c *this matches \a _tag if either
   *  \<\c *this\> or \</\c *this\> is equal to \a _tag. <BR>
   *  Otherwise, \c *this matches \a _tag if \c *this is a prefix of a string
   *  \a s, such that either \<\a s\> or \</\a s\> is equal to \a _tag.
   */
  bool matchesTag(const string& _tag, const bool isCompleteWord) const;

  /** Split Query at two or more dots.
   *
   * \return A vector containing the resulting parts
   *
   *  Splits \c *this at each occurrence of two or more dots.
   *  The returned parts have the same separators and keyword-expression as
   *  \c *this.
   */
  std::vector<Query> splitAtDots() const;

  /** Check whether \c *this is a keyword.
   * \return \c true, iff \c *this matches the regular expression for keywords.
   * @todo Keywords not yet supported; what are keywords?
   */
  bool isKeyword() { return false; }

  /** Split Query at a character.
   *  \param  c The character to split at
   *  \return A vector containing the resulting parts
   *
   *  The returned parts have the same separators and keyword-expression as
   *  \c *this.
   *  This function is currently (31Jul2006) only used in ExcerptsGenerator.h
   */
  // std::vector<Query> splitAt(const char& c) const
  std::vector<Query> splitAt(const string& c) const;

  //! Remove strange characters from query, to make highlighting easier.
  //! Strange characters are: '(', ')', '#', '[', ']', and '\n'.
  void cleanForHighlighting();
  //! Output operator; simply outputs the query string
  friend ostream& operator<<(ostream& os, const Query& query);
};  // class Query

#endif  // SERVER_QUERY_H__
