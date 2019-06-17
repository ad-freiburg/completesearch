#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define VERSION __DATE__ " " __TIME__

/* the following two defines would give a redefinition warning!? */
//#define _FILE_OFFSET_BITS 64 
//#define _LARGEFILE_SOURCE 

#include <stdio.h>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string.h>  // for strerror
#include <vector>
#include <limits.h>  // for UINT_MAX
#include <limits>  // for numeric_limits
#include <unistd.h> // NEW 28Feb11 (baumgari): needed for hostname
#include <termios.h>
#include <pthread.h> /* don't forget to link with -lpthread */
#include <fcntl.h> 
#include <errno.h> /* NEW 7Aug06 (Holger): needed for cygwin */
#include <exception>
#include <sys/stat.h> /* NEW 18Aug11 (baumgari): needed to compile, since /usr/include/bits/stat.h throws an error */
#include "assert.h"
#include "Timer.h"
#include "Exception.h"
#include "../utility/StringConverter.h"
//#include "Separator.h"

// #include <ext/hash_map> // for hash of queries, which don't contribute to history
// #include <ext/hash_set> 
#include <unordered_map>

class Vocabulary;

using namespace std;
// using namespace __gnu_cxx; /* for hash_map */

#define METHOD_NOT_YET_IMPLEMENTED { cout << "! Method " << __PRETTY_FUNCTION__ \
                                          << " in " << __FILE__ << " not yet implemented" \
                                          << endl; exit(1); }


// GLOBAL TYPES
typedef signed   int  WordId; /* must be signed! Negative indicates special cases */
typedef unsigned int  DocId;
typedef unsigned int  Position; /* needed in WordsFile introduced by Holger */
typedef unsigned int  BlockId;
typedef unsigned int  Score;
typedef unsigned char DiskScore;
struct Encoding { enum { UTF8, ISO88591 }; }; 
class ExcerptsGenerator;
class Query;


// GLOBAL VARIABLES / PARAMETERS 
//
//   TODO: some of these should rather be members of the appropriate class
//
extern int encoding;
extern string localeString;
extern bool showStatistics;
extern ExcerptsGenerator* excerptsGenerator;
extern int nofRunningProcessorThreads; 
extern size_t excerptsDBCacheSize;
extern size_t historyMaxSizeInBytes;
extern unsigned int historyMaxNofQueries;
extern bool runMultithreaded;
extern size_t maxBlockVolume;
extern StringConverter globalStringConverter;
extern bool cleanupQueryBeforeProcessing;
extern bool useSuffixForExactQuery;
extern char wordPartSep;
extern off_t queryTimeout;
extern string keepInHistoryQueriesFileName;
extern string warmHistoryQueriesFileName;

// Added 25Jan06 to circumvent a gcc compiler bug 
// See http://gcc.gnu.org/ml/gcc-bugs/2001-05/msg00250.html
class True { public: inline bool operator()() const {return true;} };
class False { public: inline bool operator()() const {return false;} };
const static True doCheckWordIds = True();
const static False doNotCheckWordIds = False();

const static True notIntersection = True();
const static False andIntersection = False();


//class blockSeparationMode { public: virtual inline unsigned char operator()() const {return (unsigned char) -1;}};
class blockSeparationMode { public: virtual inline signed char operator()() const = 0; virtual ~blockSeparationMode() { } };

class Zero : public blockSeparationMode {public: inline signed char operator()() const {return 0;}};
class One : public blockSeparationMode  {public: inline signed char operator()() const {return 1;}};
class Two : public blockSeparationMode  {public: inline signed char operator()() const {return 2;}};
class Three : public blockSeparationMode {public: inline signed char operator()() const {return 3;}};
class Four : public blockSeparationMode {public: inline signed char operator()() const {return 4;}};
class Ten : public blockSeparationMode {public: inline signed char operator()() const {return 10;}};
class MinusTwo : public blockSeparationMode {public: inline signed char operator()() const {return -1;}};

// These are names for different modi of intersecting/scoring
const static Zero SameDoc  = Zero(); // " "
const static One  Adjacent = One();  // "."
const static Two  Near     = Two();  // ".."
const static Three Pairs    = Three(); // To mark lists which are to be interpreted as pairs
const static MinusTwo Flexi = MinusTwo(); // To mark the flexible intersection mode // needed?
const static Ten Full    = Ten(); // To mark the very first word


// used for .. = (- NEIGHBORHOOD_SIZE, NEIGHBORHOOD_SIZE). And for NEAR_MATCH_BONUS
#define NEIGHBORHOOD_SIZE 10

// DEFAULT NUMBER OF THINGS TO COMPUTE (VIA TOP-K MECHANISM)
#define DEFAULT_NOF_HITS_TO_COMPUTE 100 //INT_MAX //100
#define DEFAULT_NOF_COMPLETIONS_TO_COMPUTE 10 //-INT_MAX //10

#define MIN_QUERY_LENGTH 0 // Was temporarily used to block "a b c d"


// DIFFERENT MODI CORRESPOND TO DIFFERENT CHARS
#define FLEXI    (-2)
#define SAME_DOC 0
#define ADJACENT 1
#define NEAR     2
#define PAIRS    3
#define JOIN     5
#define FULL     10

// FOR COMPATIBILITY WITH INGMAR'S OLD NAMES
#define DOCID DocId
#define WORDID WordId
#define BLOCKID BlockId
#define COMPLETIONSCORE CompletionScore

#define INFTY_DOCID UINT_MAX /* a very large (infinite), non-existent docId. To simplify intersection code. */
#define MIN_DOC_ID  0
#define MAX_DOC_ID  UINT_MAX
#define MIN_WORD_ID INT_MIN
#define MAX_WORD_ID INT_MAX 
// NEW(bast, 12Dec10): Special word id and position, used for special posting
// for aggregated score of a document from first part of query. See
// CompleterBase.Intersect.cpp::intersectTwoPostingListsNew.
#define SPECIAL_WORD_ID -1
#define SPECIAL_POSITION 99999

// Added by Holger 24Mar08 (used in new intersect)
//#define NO_DOC_ID   UINT_MAX
//#define NO_WORD_ID  INT_MAX
//#define NO_POSITION UINT_MAX
//#define NO_SCORE    UINT_MAX

// Completer MODEs 
// >0 indicates that one has to check for duplicates
#define NO_DUPS_NO_POS 0 //also no scores, ONLY USED IN "showWordidsFile.cpp"
#define WITH_DUPS 1
#define WITH_POS 2
#define WITH_SCORES 4
#define WITH_DUPS_NO_POS WITH_DUPS
#define WITH_DUPS_WITH_POS (WITH_DUPS + WITH_POS)

// Intersect MODEs
#define NO_WORD_ID_CHECK     1   // all word ids in a block are in range
#define INVERT_SECOND_LIST   2   // for Ingmar's "not" intersection
#define TAKE_FROM_FIRST_LIST 4   // take result elements from first list 


// BUFFER SIZES AND OTHER PARAMETERS /* variables are initialized in Globals.cpp */
//#define MAX_SCORE 255 /* 255 works for 1 byte scores, the raw scores are assumed to be in [0,1]  */
#define MAX_SCORE 100 /* 100 used to get cleaner scores. Easier to check */
#define MAX_LINE_LENGTH 10000 /* max line length in .words file */
#define MAX_WORD_LENGTH MAX_LINE_LENGTH /* max word length in .vocabulary file */
// The intersection buffers and union buffers are in the history
// These buffer sizes are changed anyways when intersecting
/* buffer sizes are in NUMBER OF ELEMENTS !! */
// set to large value if multiple blocks are involved
#define INTERSECTION_BUFFER_INIT_DEFAULT (1) /* buffer for results of intersections */
  // Ingmar: result of intersection is no longer intermediary but becomes
  // (without further copying) the entry in the history. Vector size is set to
  // an upper bound, and whatever is not needed in the end is freed again.
  // With multiple index, many resizes occur (one can see this in the log for
  // queries like a*), and this is the only case where it would make sense to
  // set this constant to a large value
  //// NEW 26Jan07 (Holger): increased buffer size from 2 to 10 million (items or bytes?)
  //#define INTERSECTION_BUFFER_INIT (10*1024*1024) /* buffer for results of intersections */
// union buffer is SMALLER than interseciton buffer (as no dups are kept there)
#define UNION_BUFFER_INIT_DEFAULT (1) /* buffer for results of unions, i.e., for the list without the duplicates  */
// The compression buffer is used for reading from disk and is global to the Completer object
// This buffer size should not be changed all the time when the Completer object is being reused
//#define COMPRESSION_BUFFER_INIT_DEFAULT (1300*1000*1000) /* size in bytes, for a compressed doc list */
#define COMPRESSION_BUFFER_INIT_DEFAULT (1) /* size in bytes, for a compressed doc list */
extern string MSG_BEG; /* prefix of messages like buffer resize etc. */
extern string MSG_END; /* postfix of messages like buffer resize etc. */
extern unsigned int HYB_BLOCK_VOLUME; 
extern string HYB_BOUNDARY_WORDS_FILE_NAME; 
extern bool SHOW_HUFFMAN_STAT; 
#define FILE_BUFFER_SIZE 1000000 /* buffer size when using fread, fwrite, etc. */

// NEW 12Jul06: for enhanced queries with units of type [...#...#...]

/*
 * ASCII CHARACTER SET (ISO-8859-1, control characters replaced by x)
 *
 *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx !"#$%&'()*+,-./0123456789:;<=>?
 *   @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~x
 *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx �������������������������������
 *   ����������������������������������������������������������������
 *
 */

//! CLASS CHARACTER SET (with index for quick lookup)
class CharacterSet
{
  public:
    string _chars;
    string _index;
    char index(char c) const { assert(_index.length() == 256); return _index[(unsigned int)(unsigned char)(c)]; }
    char size() const { return _chars.length(); }
    char operator[](char c) const { return _chars[c]; }
    CharacterSet(const char* chars);
};

extern const CharacterSet WORD_CHARS;
extern const CharacterSet        SEP_CHARS;
extern const CharacterSet MASKED_SEP_CHARS;
extern const char ENHANCED_QUERY_BEG;
extern const char ENHANCED_QUERY_END;
extern const char ENHANCED_QUERY_SEP;
extern const char       OR_QUERY_SEP;
extern const char      NOT_QUERY_SEP;

extern Score BEST_MATCH_BONUS;

// SCORES FOR RANKING DOCS AND COMPLETIONS
//
// Each word-in-document pair in the raw result list has a precomputed score.
// From these, a score for each matching document (doc score), and a score for
// each matching word (word score) is computed. Then the documents with the
// top doc scores and the words with the top word scores are returned.
//
// More precisely, a doc score is obtained by (D.1) aggregating the scores 
// of multiple occurrences of the same word within a document, then (D.2)
// aggregating the scores of words in the same block, and then (D.3) aggregate
// these score per block to a single score per document. 
//
// Graphically, for the score of matching document d:
//
// one score for each occurrence of a matching word w in d
//   --- (D.1) --> one score for each distinct matching word w
//   --- (D.2) --> one score per block of matching words
//   --- (D.3) --> a single score for d

// A word score is obtained by first (S.1) aggregating the scores for multiple
// occurrences of that word within the same document, and then (S.2) aggregating these
// scores over all documents.
//
// For the score of a matching word w
//
// one score for each occurrence of w in a matching document d
//   --- (S.1) --> one score per block of neighboring occurences of w
//   --- (S.2) --> one score per matching document containg w
//   --- (S.3) --> a single score for w
//
// S.1
// aggregate the scores of multiple completions of the same prefix within
// a document. E.g., the to be completed prefix is "str", and some doc has
// occurrences of "string" with aggregated (as below) score s_1 and of "strlen"
// with aggregated (as below) score s_2, how to aggregate s_1 and s_2 ? Default:
// Max.
#define WITHIN_DOC_AGGREGATION SumAggregation() 
// TODO: rename to MULTIPLE_COMPLETIONS_AGGREGATION

// S.2
// NOTE: if this is max, the ranking for DBLP goes completely wrong (authors
// seem to come in alphapetical order then, years too, etc.)
// Default: Sum (Note: for DBLP, must not be changed to Max)
#define BETWEEN_DOCS_AGGREGATION SumAggregation() //after combining scores for a completion within a doc (as above), the scores between docs are combined this way
// TODO: rename to 


// D.1
// How to aggregate the score of several occurrences of the same word in the same
// document. E.g., a document has two occurrences of "strlen" with scores s_1
// and s_2, respectively, how to aggregate s_1 and s_2 ? Default so far: MAX
#define WITHIN_COMPLETION_AGGREGATION SumAggregation() 
// TODO: rename to MULTIPLE_OCCURRENCE_OF_SAME_WORD_AGGREGATION
  
// D.2
// E.g., query "str" and document contains "strx" and "stry", what to do with
// the two scores (-> document score). Default: SUM.
#define BETWEEN_COMPLETIONS_AGGREGATION MaxAggregation() //to combine scores for a given doc stemming from different completions


// D.3 ?
// Possible: SumProx (default), Sum, Max
#define LOCATION_SCORE_AGGREGATION SumProxAggregation() //This is used in the intersection routine to combine locations scores from the last match and the current matching position

// D.4 ?
#define BETWEEN_BLOCKS_AGGREGATION SumAggregation() //Used to combine document scores for the last '..' block with the doc score for the current block only
//#define BETWEEN_BLOCKS_AGGREGATION SumAggregation() //Used to combine document scores for the last '..' block with the doc score for the current block only




// FOR GENERAL USE (code of functions in Globals.cpp)
#define MIN(a,b) ( (a) < (b)  ? (a) : (b) )
#define MAX(a,b) ( (a) > (b)  ? (a) : (b) )
#define ABS(a) ( (a > 0)  ? (a) : (-a) )
char* timeAsString(float microseconds); 
ostream& operator<<(ostream& os, Timer& timer); 
string megsPerSecond(off_t nofBytes, off_t usecs);
string commaStr(off_t number); 
string alignedDbl(double number, unsigned short end = 4);
string numberAndNoun(unsigned int number, const char* singular, const char* plural);
//! TRY TO GET A LOCK ON A MUTEX ONCE IMMEDIATELY AND AGAIN AFTER SOME TIME
int pthread_mutex_timed_trylock( pthread_mutex_t* mutex, useconds_t microseconds);

// Escape text for use in an XML tag. For example: "this < that" --> "this
//&lt; that" or --> "<![CDATA[this < that]]>. Currently simply scans the string
//for occurrences of one of the symbols in <>& and if it finds one such
//occurrence puts the text into <![CDATA...]]>. If an occurrence of ]]> is
//detected, it is replaced by ]}>.
// NEW (baumgari) 13Dec12: We added a feature to print some text unescaped. This
// makes sense, in case we know, that the result is proper xml. Nevertheless
// there are some fields which should be escaped always (in case of a invalid
// character) like completions. To support this, I added a boolean
// "alreadyWellformedXml" as parameter.
string addCdataTagIfNecessary(string text, bool alreadyWellformedXml);

// Decode a text (replace + by spaces (standard for urls), and replace %hh by
// the corresponding character). See CompleterBase.cpp:237 8Feb13.
string decodeHexNumbers(const string& text);

// Encode a text by replacing special characters by the corresponding hex %hh
// character. See CompleterBase.cpp:237 8Feb13.
string encodeQuotedQueryParts(const string& query);

string replaceWordPartSeparatorFrontendToBackend(const string& query);
string replaceWordPartSeparatorBackendToFrontend(const string& query);

// Compute right shift to the end of the current UTF-8 multi-byte sequence, if
// in the middle of it. Effectively advances the position as long as the byte at
// the current position is of the form 110..... and returns the number of
// position advances. Note that pos + the returned shift are at most the length
// of the string. If at the beginning of a regular UTF-8 multibyte sequence or a
// regular ASCII character (code < 128), the shift returned is zero.
size_t shiftIfInMiddleOfUtf8MultibyteSequence(const string& s, size_t pos);


// FOR FORMATTING ASCII OUPUT (especially EMPH is useful when there is a lot of output)
#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[0m"
#define BLACK    "\033[30m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define BROWN    "\033[33m"
#define BLUE     "\033[34m"
#define RESET    "\033[0m"
#define ALERT    "\a"



//! A HASH FUNCTION FOR STRINGS (used for query history, -> INGMAR: why not use built-in char* function?)
// (simply copy+pasted from current Thomas' CompleterBase)
class StringHashFunction
{
  public:
    size_t operator()(const string& s) const
    {
      size_t HashValue = 0;
      for (size_t i = 0; i < s.length(); i++)
	HashValue = 31*HashValue + s[i];
      return HashValue;
    }
}; /* end of class StringHashFunction */

//! WAIT UNTIL KEY PRESSED (key will not be echoed on terminal)
extern char keyPressed;
void waitUntilKeyPressed();

//! GET HOSTNAME (return default if not exists)
string gethostname(int maxLength, const char* default_if_not_exists);

//! GET ENVIRONMENT VARIABLE (default if not exists)
string getenv(const char* var_name, const char* default_if_not_exists);


//! LOCK/UNLOCK FILE
void lock_file(int fid, int lock_type);


//! LOCK/UNLOCK LOG FILE (for use in cout/cerr)
extern FILE* log_file;
extern pthread_mutex_t log_file_mutex;
extern pthread_mutex_t process_query_thread_mutex;
extern const char* empty_string;
const char* lock_log_file();
const char* unlock_log_file();
#define LOCK ("")
#define UNLOCK ("")
//#define LOCK (lock_log_file())
//#define UNLOCK (unlock_log_file())


//! READ WORDS FROM FILE (one word per line)
//
//    - used for reading vocabulary of an index (<db>.vocabulary)
//    - used for reading HYB block boundary words (<db>.prefixes)
//
void readWordsFromFile(const string fileName, Vocabulary& words, string what = "words");


//! MASK CTRL CHARS IN STRING (so that it can be printed)
string printable(string s);


//! UT8 TOLOWER (detailed description in cpp file)
int utf8_tolower(char* p);

//! FOR MULTIPLE MATCHES IN EXCERPTS (e.g., ^pro^seminar)
extern const char wordPartSeparator;

//! NEW 07Aug07 (Holger): new normalization function
int normalize(const char* p, string& nc);

//! CHARCTER NORMALIZATION MAP (e.g., � -> A or � -> ue)
extern const char* NORM_CHAR_MAP;

//! NORMALIZE A CHARACTER (according to the map above)
void normalize(unsigned char c, std::string& nc);

//! NORMALIZE A WHOLE STRING (according to the function above)
void normalize(const std::string& word, std::string& norm_word);

//! PREFIX CHECK (is needle prefix of haystack, TODO: stupid names!)
bool isRealPrefix(const std::string& haystack, const std::string& needle);

//! EQUALITY TEST (ignoring any word part separators, e.g. proseminar == ^pro^seminar)
bool isEqual(const std::string& word_from_doc, const std::string& word_from_query);
bool isEqual(const std::string& word_from_doc, const Query& word_from_query);

//! PREFIX CHECK taking ^ into account (e.g., semin is prefix of ^pro^seminar)
bool isPrefix(const std::string& haystack, const std::string& needle);
bool isPrefix(const std::string& haystack, const Query& needle);
#endif
