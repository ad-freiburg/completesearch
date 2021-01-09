#ifndef __EXCERPTSGENERATOR_H__
#define __EXCERPTSGENERATOR_H__

#include <string>
#include <utility>
#include <vector>
#include <ctype.h>
#include <cmath>
#include "Query.h"
#include "QueryResult.h"
#include "QueryParameters.h"
#include "DocsDB.h"
#include "Separator.h"
#include <gtest/gtest.h>

using namespace std;

// Macros used for computing the maximum and minum of two elements (used by
// ExcerptsGenerator).
#define EG_MAX(a,b) ((a) > (b) ? (a) : (b))
#define EG_MIN(a,b) ((a) < (b) ? (a) : (b))

// A macro for safely adding an integral (signed or unsigned) number b to an
// unsigned long a.  Safely means that no errors occur due to casts and that
// negative results are set to zero and results larger than ULONG_MAX are set to
// ULONG_MAX.
#define ULPLUS(a,b) ((b) < 0 ? \
    ((a) < (unsigned long)(-(b)) ? 0 : (a) - (unsigned long)(-(b))) : \
    (ULONG_MAX - (a) < (unsigned long)(b) ? ULONG_MAX : (a) + (b)))

// A Comparison function for pairs. Used for sorting elements of type
// pair<FirstType, SecondType>.
template<class FirstType, class SecondType>
class EG_PairCmp
{
  public:
    bool operator()(const pair<FirstType, SecondType>& x,
        const pair<FirstType, SecondType>& y) const
    {
      return x < y;
    }
};

template<class IDType, class ScoreType>
class EG_ScoreCmp
{
  public:
    bool operator()(const std::pair<IDType, ScoreType>& x,
        const std::pair<IDType, ScoreType>& y) const
    {
      return x.second > y.second;
    }
};

// The data associated with a hit: title, URL, a list of excerpts, and a score.
// NOTE: this is very similar to ExcerptData, which Thomas used for both the
// Data associated with a documents (in which case the excerpt is the whole
// document) AND for the data associated with a hit. I (Holger) think the two
// should be separated, and I do that with this class.
class HitData
{
  public:
    DocId docId;
    string title;
    string url;
    unsigned int score;
    vector<string> excerpts;
    HitData() :
      docId(0), title(""), url(""), score(0)
    {
    }
};

// Main class for excerpt generation.
/*!
 *  An object of type \c ExcerptsGenerator uses a database of type ExcerptsDB to generate excerpts of documents
 *  from a query-string. It is also able to highlight the search results. The excerpts are returned
 *  in HTML-format (NEW 14Nov07: or XML-format).
 */
class ExcerptsGenerator
{
  private:
    // The database from which the documents are read.
    DocsDB _docsDB;
  public:
    // Whether the search results should be highlighted and how.
    // TODO(bast): explain how HTML and XML highlighting differ.
    mutable enum Highlighting
    {
      HL_NONE, HL_HTML, HL_XML
    } highlight;
  private:
    // The maximum number of hits that are computed for an excerpt.
    // NOTE: If the query string has more parts than this, more excerpts can be
    // computed.
    short maxHits;
    // The minimum number of characters that a matching word must have.
    size_t minWordLength;
    // The maximum number of characters that a matching word may have.
    size_t maxWordLength;
    // The characters that are recognized as word separators in a document.
    static const string nonWordCharacters;
    // Additional characters that are allowed within tags.
    static const string specialCharactersWithinTags;
    // Defines the window around a matching position that is put into the excerpt.
    // NOTE(bast): Fixed a really mean bug on 9Dec09. This used to be a
    // pair<signed long, signed long>. On 32-bit machines this worked, but on
    // 64-bit machines, a long has 8 bytes and the assignment of -5 to
    // showRange.first in the constructor actually first converts it to an
    // unsigned int (whereby it becomes UINT_MAX - 5) and then to a signed long.
    // Anyway, the whole dealing with unsigned long, signed long, and adding other
    // types is a terrible mess here. But with this simple fix, it seems to work
    // again.
    typedef pair<int, int> Range;
    Range showRange;
    // A vector of color values for highlighting the search results.
    // TODO(bast): Isn't this done on the client side now?
    static const char* color[];
    // The number of different colors that are defined for highlighting.
    static const size_t numberOfColors;
    // The list of words a document consists of.
    mutable vector<string> _wordList;
    // For each of these words, its position in the document string + the length
    // of the word.
    mutable vector<pair<unsigned long, unsigned long> > _positionList;
    // Pointer to the current document.
    mutable const string* _document;
    // Indicates whether _wordList and _positionList are complete.
    mutable bool _listsComplete;
    // The excerpts for a document.
    mutable vector<string> _excerpts;
    // The hit data objects that will be returned (containing title, url, and
    // excerpts).
    mutable vector<HitData> _hits;

    // Computes _wordList and _positionList up to the i-th word.
    void computeWordsAndPositions(size_t i) const;
    // Get the position of the i-th word in the document. Calls
    // computeWordsAndPositions if not enough words from the document have been
    // parsed so far. Returns true iff there was an i-th word.
    bool positionList(unsigned long i, unsigned long& position) const;
    // Same, but returns the position and length of the word.
    bool positionList(unsigned long i,
        pair<unsigned long, unsigned long>& pos_pair) const;
    // Same but returns the word.
    bool wordList(unsigned long i, std::string& word) const;

    // Check if candidate string is a prefix of word. The "Real" pertains to the
    // fact that no special characters like ^ are considered here, it's just a
    // plain prefix check. Also, a word is always a prefix of itself.
    static bool isRealPrefix(const string& candidate, const string& word);FRIEND_TEST(ExcerptsGeneratorTest, isRealPrefix);
    // Check if two words are equal. Ignore any word part separators. For example,
    // isEqual("proseminar", "^pro^seminar") == true
    // isEqual("seminar", "^pro^seminar") == true
    // isEqual("pro", "^pro^seminar") == true
    static bool isEqual(const string& word_from_query,
        const string& word_from_doc);
    static bool isEqual(const Query& word_from_query,
        const string& word_from_doc);FRIEND_TEST(ExcerptsGeneratorTest, isEqual);
    // Check if candidate is a prefix of word, taking ^ into account. For example
    // isPrefix("semin", "^pro^seminar")
    // isPrefix("pro", "^pro^seminar")
    // isPrefix("prose", "^pro^seminar")
    static bool isPrefix(const string& candidate, const string& word);
    static bool isPrefix(const Query& candidate, const string& word);FRIEND_TEST(ExcerptsGeneratorTest, isPrefix);

    // Deal with words containing ^ and ^^ and ^^^ (used to deal with all kinds of
    // non-literal matches). All ^ are removed. Parts following ^^ up to the next
    // ^ are removed. ^^^ are replaced by a space. For example, the (single) word
    // ^^cexpersonx^^kurtmehlhorn^Kurt^^^Mehlhorn becomes "Kurt Mehlhorn" (without
    // the quotes).
    string cleanedUpExcerpt(const string& excerpt) const;

    // Compute excerpts from given intervals and highlighting positions. The
    // interval bounds are given as word positions, and so are the highlighting
    // positions. If highlight is set to HL_NONE, no highlighting is done.
    void
        generateExcerptsFromIntervals(
            const string& document,
            const vector<pair<unsigned long, unsigned long> >& intervals,
            const vector<pair<unsigned long, unsigned short> >& highlightedPositions) const;

    // Given a number of sequences of intervals, compute their union as a single
    // sequence of intervals.
    void
        uniteIntervals(
            const vector<vector<pair<unsigned long, unsigned long> > >& listOfIntervals,
            vector<pair<unsigned long, unsigned long> >& unionOfIntervals) const;

    // The base case of the recursive computation of the intervals of word positions in the document that
    // should be part of the excerpt. The query does not contain any separators but the or-operator.
    // Its or-separated parts are either complete words or prefixes (each ending with a '*').
    // The word positions that match the query are returned in a vector.
    // The number of positions returned is at least one and at most max.

    // Computes the interval of the positions of the words that will appear in the
    // excerpts. Also computes the positions of the terms to be highlighted
    // (unless highligh == HL_NONE). At most max hits are computed.
    // TODO(bast): what is a "hit" in this context?
    // TODO(bast): which positions are returned in the vector<unsigned long>?
    //
    // This function is called for each one of the space-separated parts of the query that do not contain keywords. It computes
    // at most \a max positions of hits matching \a query. It also computes for each such position an interval of word-positions
    // that will appear in the excerpt. If the variable \a highlight is not \c HL_NONE, also a list of positions to be highlighted
    // is computed for each returned hit.
    //
    // This function operates recursively. In the base case, i.e. if the query does not contain any more separators except
    // or-operators, it calls computePositionsAndIntervalsBaseCase().
    vector<unsigned long> computePositionsAndIntervals(Query query,
        const string& document,
        vector<pair<unsigned long, unsigned long> >& intervals,
        vector<vector<unsigned long> >& highlightedPositions,
        const short max = SHRT_MAX) const;

    //! Computes positions of hits and intervals of word positions if the query does not contain separators.
    /*!
     *  \param      query               Query without separators except or-operators.
     *  \param      doc                 The document for which the excerpt is generated.
     *  \param      intervals           Is set to the list of intervals of word positions that will appear in the excerpt.
     *  \param      highlightedPositions        Will contain for each hit a list of positions to be highlighted, if \a highlight != \c HL_NONE.
     *  \param      max                 The number of hits computed is at most \a max.
     *  \param      keywordSearch           Indicates that the query is a keyword that should be matched against tags.
     *  \return     A list of positions of hits for \a query.
     *
     *  This function is called from the recursive function computePositionsAndIntervals(). It handles the base case,
     *  in which the query does not contain any separators exept or-operators. It computes at most \a max positions
     *  of hits matching \a query. It also computes for each such position an interval of word-positions that will
     *  appear in the excerpt. If the variable \a highligh is set to \c true, also a list of positions to be highlighted
     *  is computed for each returned hit.
     */
    vector<unsigned long> computePositionsAndIntervalsBaseCase(
        const Query& query, const string& doc,
        vector<pair<unsigned long, unsigned long> >& intervals,
        vector<vector<unsigned long> >& highlightedPositions, const short max,
        const bool keywordSearch = false) const;

    //! Computes the positions of hits and the intervals of word positions if the query contains a keyword.
    /*!
     *  \param      queryParts          Query-parts without space-characters and without occurrences of more than one dot.
     *  \param      keyIndex            Index such that \a queryParts[\a keyIndex] is a keyword.
     *  \param      document            The document from which the excerpt is to be generated.
     *  \param      intervals           Is set to the list of intervals of word positions that will appear in the excerpt.
     *  \param      highlightedPositions        Will contain for each hit a list of positions to be highlighted, if \a highlight!=\c HL_NONE.
     *  \param      max                 The number of hits computed is at most \a max + \a queryParts.size().
     *
     *  This function is called for each one of the space-separated parts of the query that contain a keyword. These parts are
     *  splitted at occurrences of two or more dots. The resulting parts are given to this function as argument \a queryParts.
     *  \a keyIndex is the index of the keyword in \a queryParts. It determines at most \a max + \a queryParts.size() positions
     *  of hits matching the query. It computes for each such position an interval of word-positions that will appear in the
     *  excerpt. If the variable \a highlight is not \c HL_NONE, also a list of positions to be highlighted is computed for
     *  each such interval.
     *
     *  This function calls computePositionsAndIntervalsBaseCase() to determine the intervals defined by the keyword.
     *  It uses computePositionsInInterval() to compute hits in these intervals.
     */
    void computePositionsAndIntervalsWithKeyword(
        const vector<Query>& queryParts, const size_t& keyIndex,
        const string& document,
        vector<pair<unsigned long, unsigned long> >& intervals,
        vector<vector<unsigned long> >& highlightedPositions,
        const short& max = SHRT_MAX) const;

    //! Computes the positions of hits and the intervals of word positions in a given interval.
    /*!
     *  \param      query               Query string that does not contain space-characters or occurrences of more than one dot.
     *  \param      searchInterval          Interval in which hits are looked for.
     *  \param      showIntervals           The intervals that should appear in the excerpt are appended to this list.
     *  \param      highlightedPositions        Will contain for each hit a list of positions to be highlighted, if \a highlight!=\c HL_NONE.
     *  \param      max                 The number of hits computed is at most \a max.
     *
     *  Given a query string without spaces or occurrences of more than one dot, this function finds at most \a max positions
     *  of hits matching \a query in the interval \a searchInterval. It computes for each such position an interval of word-positions
     *  that will appear in the excerpt. These intervals are appended to the list \a showIntervals. If the variable \a highlight
     *  is not \c HL_NONE, also a list of positions to be highlighted is computed for each such interval.
     */
    vector<unsigned long> computePositionsInInterval(const Query& query,
        const pair<unsigned long, unsigned long>& searchInterval,
        vector<pair<unsigned long, unsigned long> >& showIntervals,
        vector<vector<unsigned long> >& highlightedPositions,
        const short& max = SHRT_MAX) const;

    //! Checks whether a string is a tag.
    /*!
     *  A string is considered a tag if it starts with a '\<', eventually followed by a '/', ends with a '\>'
     *  and contains only letters and characters from \a specialCharactersWithinTags in between.
     *  \return     \c true iff \a s is a tag.
     */
    bool isTag(std::string& _s) const;

    //! Checks whether there starts a tag in a document at position \a i.
    /*!
     *  \param      doc                 The document in which a tag is searched.
     *  \param      i               The position in \a doc at which the tag is supposed to start.
     *  \param      j               Is set to the end position of the tag if one is found.
     *  \return     \c true iff a tag is found.
     *
     *  If there starts a tag at position \a i in \a doc, \a j is set to the end position of that tag and \c true
     *  is returned. Otherwise, \c false is returned.
     */
    bool tagFound(const string& doc, const unsigned long & i,
        unsigned long & j) const;

    //! Converts each character to upper-case.
    void capitalize(string& word) const;

    // sorts the list of highlighted positions and removes duplicate positions. Each element in the list
    // is a pair of a position and a color-value.
    //! Sorts the list of highlighted positions.
    /*!
     *  \param      allHighlightedPositions     The positions to be sorted.
     *  \param      sortedHighlightedPositions  The sorted list.
     *
     *  Before this function is called, for each space-separated part of the query a list of hits is computed and
     *  for each such hit a list of positions to be highlighted. The value \a allHighlightedPositions[\a i][\a j][\a k]
     *  is the \a k-th position to be highlighted in the \a j-th hit of the \a i-th query-part.
     *  These positions are sorted into the list \a sortedHighlightedPositions, where duplicates are eliminated.
     *  In the sorted list, to each position the corresponding index \a i (the index of the query part) is added.
     *  This index will be taken as the color-value for the highlighting.
     */
    void
        sortHighlightedPositions(
            const vector<vector<vector<unsigned long> > >& allHighlightedPositions,
            vector<pair<unsigned long, unsigned short> >& sortedHighlightedPositions) const;
    // NEW 03.01.13 (baumgari): Added a feature to allow multiple titles by
    // separating them by infoDelim and choosing one by specifying an index as
    // queryParameter.
    //! Split field into parts by using info-delimitter and return the indexed one.
    /*!
     * \param	index	Specifies the index of the part within the string.
     * \param	field	The field which should by processed (by now just "title").
     * */
    static string getPartOfMultipleField(const unsigned int index, const string& field);
    FRIEND_TEST(ExcerptsGeneratorTest, getPartOfMultipleField);

  public:
    // Construct from docs.DB file.
    ExcerptsGenerator(const std::string& filename,
        const size_t& cachesize = 512 * 1024);

    //! Get excerpts for a given query result
    void getExcerpts(const Query& query,
        const QueryParameters& queryParameters, const QueryResult& result,
        vector<HitData>& hits);

    // Sets the maximum number of hits to be computed.
    // NOTE: this limits the number of hits displayed in the excerpts, but can
    // be slightly exceeded.  The number of hits can be as large as maxHits +
    // number of query parts + sum of lengths of keyword parts.
    void setMaxHits(const unsigned short& m)
    {
      maxHits = m;
    }
    // Sets the radius of the window around a matching position shown in an excerpt.
    void setExcerptRadius(const signed long l, const signed long r)
    {
      showRange.first = l;
      showRange.second = r;
    }
    // Returns the maximum number of hits to be computed.
    short getMaxHits() const
    {
      return maxHits;
    }
    // Sets the minimum number of characters that a matching word must have.
    void setMinWordLength(const short& m)
    {
      minWordLength = m;
    }
    // Sets the maximum number of characters that a matching word may have.
    void setMaxWordLength(const short& m)
    {
      maxWordLength = m;
    }
    // Returns the minimum number of characters that a matching word must have.
    short getMinWordLength() const
    {
      return minWordLength;
    }
    // Returns the maximum number of characters that a matching word may have.
    short getMaxWordLength() const
    {
      return maxWordLength;
    }

    // Computes relevant excerpts for given query and document.
    HitData hitDataForDocAndQuery(const Query& query, const DocId& docId,
	const unsigned int titleIndex, int _highlight = HL_HTML) const;
    // Insert a string into an excerpt, paying attention not to insert in the
    // middle of a UTF-8 multibyte sequence.
    static void insertIntoExcerpt(size_t pos, const string& insert,
        string* excerpt);
    // Get part of string, paying attention not to cut in the middle of a UTF-8
    // multibyte sequence.
    static string getPartOfString(const string& document, size_t pos,
        size_t length);

}; // class ExcerptsGenerator

#endif
