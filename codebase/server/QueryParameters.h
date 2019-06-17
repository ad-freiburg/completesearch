#ifndef __QUERY_PARAMETERS_H__
#define __QUERY_PARAMETERS_H__

#include "Globals.h"
#include "Query.h"
#include "ScoreAggregators.h"
#include <fstream>
#include <regex.h>
#include "Vector.h"

//extern ofstream* logfile;

using namespace std;


//! THE PARAMETERS OF AN AUTOCOMPLETION QUERY (passed as string appended to the query)
class QueryParameters
{
  private:

    // flag telling whether the regex has been compiled (initialized to false below)
    static bool regexesCompiled;
    // the regex strings (OLD and HTTP)
    static const char* regexStringOld;
    static const char* regexStringHttp;
    // the compiled regex (OLD and HTTP)
    static regex_t* compiledRegexOld;
    static regex_t* compiledRegexHttp;

  public:

    //! Dump object as string.
    std::string asString(void) const;

    //! query
    std::string query;

    //! number of top hits to compute
    unsigned int nofTopHitsToCompute;

    //! number of top hits to send
    unsigned int nofHitsToSend;

    //! number of top completions to compute
    unsigned int nofTopCompletionsToCompute;

    //! number of top completions to send
    unsigned int nofCompletionsToSend;

    //! send hits starting from this one
    DocId firstHitToSend;

    //! how to score with fuzzy
    float fuzzyDamping;

    //! how many excerpts per hit
    unsigned short nofExcerptsPerHit;

    //! how many hits per group (only relevant when howToRank is group by word)
    unsigned int nofHitsPerGroup;

    //! how many words around each highlighted words
    unsigned int excerptRadius;

    //! how to display hits
    unsigned int displayMode;

    //! synonymy mode (not yet implemented)
    unsigned int synonymMode;

    //! Index of part of title.
    unsigned int titleIndex;

    //! whether filtering may be used or not (default: true, turned off only for some efficiency experiments)
    bool useFiltering;

    // NEW (baumgari) 12Nov14:
    // Variables to define the neighboorhood size, which is used when the
    // operator .. is used.
    //! Defines where the neighbourhood starts.
    unsigned int neighbourhoodStart;
    
    //! Defines where the neighbourhood ends.
    unsigned int neighbourhoodEnd;


    //! whether to use hash join or merge jon (default: hash join)
    enum MergeOrHashJoin {
      MERGE_JOIN = 0,
      HASH_JOIN  = 1
    } howToJoin;

    //! How to rank documents
    enum HowToRankDocsEnum {
      RANK_DOCS_BY_SCORE     = 0,
      RANK_DOCS_BY_DOC_ID    = 1,
      RANK_DOCS_BY_WORD_ID   = 2,
      //RANK_DOCS_BY_WORD_COUNT = 3,
      //RANK_DOCS_BY_OCC_COUNT = 4
      RANK_DOCS_BY_COMPLETION_SCORES = 5,
      GROUP_DOCS_BY_WORD_ID = 6,
      RANK_DOCS_BY_FUZZY_SCORE = 7,
    } howToRankDocs;

    //! How to rank words
    enum HowToRankWordsEnum {
      RANK_WORDS_BY_SCORE     = 0,
      RANK_WORDS_BY_DOC_COUNT = 1,
      RANK_WORDS_BY_OCC_COUNT = 2,
      RANK_WORDS_BY_WORD_ID   = 3,
      RANK_WORDS_BY_DOC_ID    = 4,
      RANK_WORDS_BY_EDIT_DISTANCE = 5,
    } howToRankWords;

    //! Docs or words
    enum DocsOrWordsEnum {
      DOCS  = 0,
      WORDS = 1
    };

    //! Response format
    enum ResponseFormatEnum {
      XML   = 0,
      JSON  = 1,
      JSONP = 2
    };

    //! Query type
    enum QueryTypeEnum {
      NONE = 0,
      NORMAL = 1,
      EXE = 2
    };

    //!  Query type
    QueryTypeEnum queryType;

    //! Response format
    ResponseFormatEnum format;

    //! Jsonp callback function
    std::string callback;

    //! Rank order for documents
    SortOrderEnum sortOrderDocs;

    //! Rank order for words
    SortOrderEnum sortOrderWords;

    //! How to aggregate scores from the different query part
    ScoreAggregation docScoreAggDifferentQueryParts;

    //! How to aggregate doc scores of multiple occurrences of the same word in a document
    ScoreAggregation docScoreAggSameCompletion;

    //! How to aggregate doc scores of occurrences of different words/completions in a document
    ScoreAggregation docScoreAggDifferentCompletions;

    //! How to aggregate word scores of multiple occurences of the same word in the same document
    ScoreAggregation wordScoreAggSameDocument;

    //! How to aggregate word scores from different documents
    ScoreAggregation wordScoreAggDifferentDocuments;

    //! Constructor: compiles regular expression needed for query parameters parsing (only once for the whole class)
    QueryParameters();

    //! Set a single score aggregation via a descriptive character
    /*!
     *     If the argument is one of the characters S, M, or B, the given score
     *     aggregation default is set to SCORE_AGG_SUM, SCORE_AGG_MAX, or
     *     SCORE_AGG_SUM_WITH_BONUS respectively.o
     *
     *     If the argument is none of these characters, an exception is thrown
     *     and the given score aggregation default is not changed.
     */
    static void setScoreAggregation
                 (ScoreAggregation& scoreAggregation,
	          const char        method);

    //! Get descriptive character for a single score aggregation
    char getScoreAggregationChar(const ScoreAggregation& scoreAggregation);

    //! Set all four score aggregations, according to four descriptive characters
    /*!
     *    the argument must be a 5-letter string, with each letter pertaining to
     *    one of the five score aggregation methods above, in the order they are
     *    declared.
     *
     *    if the input string doesn't have 5 letters, an exception is thrown and
     *    the function call will not have any effect. the letters will be
     *    processed one after the other. if a letter is none of s, m, or b, a
     *    warning message will be logged, and the respective score aggregation
     *    default will not be changed.
     */
    void setAllScoreAggregations(string scoreAggChars);

    //! Set all four score aggregation defaults; as described above
    static void setAllScoreAggregationDefaults(string scoreAggChars);

    //! Set response format to xml, json or jsonp
    void setResponseFormat(const string& format);

    //! Set neighbour hood size for nearby search triggered by the operator ..
    void setNeighbourhoodSize(const string& value);

    //! Get four character description of all four score aggregations
    string getScoreAggregationChars();

    //! Set parameters to reasonable default values
    void setToDefaultValues();

    //! Set how to rank (either words or documents); if value has neither a nor d as
    //! suffix, sortOrder is unchanged.
    template <class T>
    static void setHowToRank(const string& value, T& howToRank, SortOrderEnum& sortOrder);

    //! Extract query and parameters from query in HTTP request form GET /?q=<query>&m=10&c=2&...
    bool extractFromRequestStringHttp(string& queryString);

    //! Output parameters to a stream, e.g. cout
    friend ostream& operator<<(ostream&, QueryParameters&);
};

#endif
