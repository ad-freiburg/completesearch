#include "QueryParameters.h"
#include "ScoreAggregators.h"

//! Always compute at least that mnay hits / completions
unsigned int nofTopHitsToComputeAtLeast        = 100;
unsigned int nofTopCompletionsToComputeAtLeast = 100;

//! Default values for number of hits / completions to send
unsigned int nofHitsToSendDefault              = 10;
unsigned int nofCompletionsToSendDefault       = 10;

unsigned int neighbourhoodStartDefault         = -10;
unsigned int neighbourhoodEndDefault           =  10;

//! Default values for ranking 
QueryParameters::HowToRankDocsEnum  howToRankDocsDefault  = QueryParameters::RANK_DOCS_BY_SCORE; 
QueryParameters::HowToRankWordsEnum howToRankWordsDefault = QueryParameters::RANK_WORDS_BY_SCORE; 
                 SortOrderEnum      sortOrderDocsDefault  =                  SORT_ORDER_DESCENDING; 
                 SortOrderEnum      sortOrderWordsDefault =                  SORT_ORDER_DESCENDING;

//! Default value for response format
QueryParameters::ResponseFormatEnum formatDefault = QueryParameters::XML;

//! Default value for jsonp callback function
const char* callbackDefault = "callback";

//! Default query (type) values.
const char* queryDefault                        = "";
QueryParameters::QueryTypeEnum queryTypeDefault = QueryParameters::NONE;

// ! Default title index: in case there is no delimiter or no part specified the
// first should be taken.
unsigned int titleIndexDefault = 0;

//! Default values for score aggregation
ScoreAggregation docScoreAggDifferentQueryPartsDefault  = SCORE_AGG_SUM;
ScoreAggregation docScoreAggSameCompletionDefault       = SCORE_AGG_SUM;
ScoreAggregation docScoreAggDifferentCompletionsDefault = SCORE_AGG_SUM;
ScoreAggregation wordScoreAggSameDocumentDefault        = SCORE_AGG_SUM;
ScoreAggregation wordScoreAggDifferentDocumentsDefault  = SCORE_AGG_SUM;


//! CONSTRUCTOR compiles regular expression needed for parsing (only once for the whole class)
QueryParameters::QueryParameters()
{
  setToDefaultValues();
}

//! SET REASONABLE DEFAULT VALUES
void QueryParameters::setToDefaultValues()
{
  // how many hits to compute send
  nofTopHitsToCompute        = nofTopHitsToComputeAtLeast;
  nofTopCompletionsToCompute = nofTopCompletionsToComputeAtLeast;
  nofHitsToSend              = 30;
  nofCompletionsToSend       = 10;
  nofHitsPerGroup            = 3;
  firstHitToSend             = 0; 

  // how to score with fuzzy
  fuzzyDamping               = 0.5;

  // how to compute
  useFiltering               = true;
  howToJoin                  = HASH_JOIN;

  // how to aggregate scores
  docScoreAggDifferentQueryParts  = docScoreAggDifferentQueryPartsDefault;
  docScoreAggSameCompletion       = docScoreAggSameCompletionDefault;
  docScoreAggDifferentCompletions = docScoreAggDifferentCompletionsDefault;
  wordScoreAggSameDocument        = wordScoreAggSameDocumentDefault;
  wordScoreAggDifferentDocuments  = wordScoreAggDifferentDocumentsDefault;

  // how to rank
  howToRankDocs  = howToRankDocsDefault;
  howToRankWords = howToRankWordsDefault;
  sortOrderDocs  = sortOrderDocsDefault;
  sortOrderWords = sortOrderWordsDefault;

  // parameters for excerpt generation
  nofExcerptsPerHit          = 5; 
  excerptRadius              = 5;
  displayMode                = 1; 
  synonymMode                = 0;

  // response format
  format = formatDefault;

  // query (type)
  query     = queryDefault;
  queryType = queryTypeDefault;
  
  titleIndex = titleIndexDefault;

  // jsonp callback function
  callback = callbackDefault;

  // window size, if operator .. is used for searching for nearby words.
  neighbourhoodStart = neighbourhoodStartDefault;
  neighbourhoodEnd = neighbourhoodEndDefault;
}


//! Set a single score aggregation via a descriptive character
/*!
 *     If the argument is one of the characters S, M, or B, the given score
 *     aggregation default is set to SCORE_AGG_SUM, SCORE_AGG_MAX, or
 *     SCORE_AGG_SUM_WITH_BONUS respectively. The corresponding lowercase
 *     characters s, m, or b do the job equally well.
 *
 *     If the argument is none of these characters, an exception is thrown
 *     and the given score aggregation default is not changed.
 */
void QueryParameters::setScoreAggregation
      (ScoreAggregation& scoreAggregation,
       const char        method)
{
  switch (toupper(method))
  {
    case 'S' : scoreAggregation = SCORE_AGG_SUM; break;
    case 'M' : scoreAggregation = SCORE_AGG_MAX; break;
    case 'B' : scoreAggregation = SCORE_AGG_SUM_WITH_BONUS; break;
    default  : CS_THROW(Exception::INVALID_PARAMETER_VALUE, "method = '" << method << "'");
  }
}



//! Set all four score aggregations, according to four descriptive characters
/*!
 *    the argument must be a four-letter string, with each letter pertaining to
 *    one of the five score aggregation methods above, in the order they are
 *    declared. 
 *
 *    if the input string doesn't have four letters, an exception is thrown and
 *    the function call will not have any effect. the letters will be
 *    processed one after the other. if a letter is none of s, m, or b, a
 *    warning message will be logged, and the respective score aggregation
 *    default will not be changed.
 */
void QueryParameters::setAllScoreAggregations(string scoreAggChars)
{
  if (scoreAggChars.length() != 4)
    CS_THROW(Exception::INVALID_PARAMETER_VALUE, "scoreAggChars must have 4 letters, but has " 
                                                << scoreAggChars.length() << ": \""
                                                << scoreAggChars << "\"");

  //setScoreAggregation(docScoreAggDifferentQueryParts , scoreAggChars[0]);
  setScoreAggregation(docScoreAggSameCompletion      , scoreAggChars[0]);
  setScoreAggregation(docScoreAggDifferentCompletions, scoreAggChars[1]);
  setScoreAggregation(wordScoreAggSameDocument       , scoreAggChars[2]);
  setScoreAggregation(wordScoreAggDifferentDocuments , scoreAggChars[3]);
}


//! Set all five score aggregation defaults; as described above
void QueryParameters::setAllScoreAggregationDefaults(string scoreAggChars)
{
  if (scoreAggChars.length() != 4)
    CS_THROW(Exception::INVALID_PARAMETER_VALUE, "scoreAggChars must have 4 letters, but has " 
                                                << scoreAggChars.length() << ": \""
                                                << scoreAggChars << "\"");

  //setScoreAggregation(docScoreAggDifferentQueryPartsDefault , scoreAggChars[0]);
  setScoreAggregation(docScoreAggSameCompletionDefault      , scoreAggChars[0]);
  setScoreAggregation(docScoreAggDifferentCompletionsDefault, scoreAggChars[1]);
  setScoreAggregation(wordScoreAggSameDocumentDefault       , scoreAggChars[2]);
  setScoreAggregation(wordScoreAggDifferentDocumentsDefault , scoreAggChars[3]);
}

//! Set response format to xml, json or jsonp
void QueryParameters::setResponseFormat(const string& value)
{
  if      (value == "json")  format = JSON;
  else if (value == "jsonp") format = JSONP;
  else if (value == "xml")   format = XML;
  else LOG << "! WARNING PARSING QUERY: format has to be xml (default), "
           << "json oder jsonp. Actual value: \"" << value << "\"." << endl;
}

void QueryParameters::setNeighbourhoodSize(const string& value)
{
  size_t sep = value.find(',');
  if (sep == string::npos)
  {
    LOG << "! WARNING PARSING QUERY: Format of the neighbourhood size changing"
        << " option is misformed. It should be "
	<< "<neighbourhood start>,<neighbourhood end>, e.g. -10,10." << endl;
    neighbourhoodStart = neighbourhoodStartDefault;
    neighbourhoodEnd = neighbourhoodEndDefault;
    return;
  }
  neighbourhoodStart = atoi(value.substr(0, sep).c_str());
  neighbourhoodEnd   = atoi(value.substr(sep + 1).c_str());
  if (neighbourhoodStart > neighbourhoodEnd)
  {
    LOG << "! WARNING PARSING QUERY: Format of the neighbourhood size changing"
        << " option is misformed. The start is bigger than the end." << endl;
    neighbourhoodStart = neighbourhoodStartDefault;
    neighbourhoodEnd = neighbourhoodEndDefault;
    return;
  }
}

//! Get descriptive character for a single score aggregation
char QueryParameters::getScoreAggregationChar(const ScoreAggregation& scoreAggregation)
{
  switch (scoreAggregation)
  {
    case SCORE_AGG_SUM:            return 'S'; 
    case SCORE_AGG_MAX:            return 'M';
    case SCORE_AGG_SUM_WITH_BONUS: return 'B';
    default                      : return '?';
  }
}



//! Get 5-character description of all five score aggregations 
string QueryParameters::getScoreAggregationChars()
{
  string description = "????";
  CS_ASSERT(description.length() == 4);
  //description[0] = getScoreAggregationChar(docScoreAggDifferentQueryParts);
  description[0] = getScoreAggregationChar(docScoreAggSameCompletion);
  description[1] = getScoreAggregationChar(docScoreAggDifferentCompletions);
  description[2] = getScoreAggregationChar(wordScoreAggSameDocument);
  description[3] = getScoreAggregationChar(wordScoreAggDifferentDocuments);
  return description;
}


//! Set how to rank (either words or documents); if value has neither a nor d as
//! suffix, sortOrder is unchanged.
template <class T>
void QueryParameters::setHowToRank(const string& value, T& howToRank, SortOrderEnum& sortOrder)
{
  howToRank = (T)(atoi(value.c_str()));
  char sortOrderChar = value.length() > 0 ? tolower(value[value.length() - 1]) : 0;
  if (sortOrderChar == 'a') sortOrder = SORT_ORDER_ASCENDING;
  if (sortOrderChar == 'd') sortOrder = SORT_ORDER_DESCENDING;
}

//! Explicit instantiation
template void QueryParameters::setHowToRank<QueryParameters::HowToRankDocsEnum>
               (const string& value,
                QueryParameters::HowToRankDocsEnum& howToRank, 
	        SortOrderEnum& sortOrder);
template void QueryParameters::setHowToRank<QueryParameters::HowToRankWordsEnum>
               (const string& value,
                QueryParameters::HowToRankWordsEnum& howToRank, 
	        SortOrderEnum& sortOrder);

//! Extract parameters from query in HTTP URL form ?q=<query>&m=10&c=2&...
bool QueryParameters::extractFromRequestStringHttp(string& queryString)
{
  size_t pos = 0;
  size_t pos_0;
  while (pos < queryString.size())
  {
    // expecting '&'
    if (queryString[pos] != '&' && queryString[pos] != '?') {
      cerr << "! ERROR PARSING QUERY: expected '&' or '?' at position " << pos 
           << " of queryString \""  << queryString << "\"" << endl;
      return false;
    }
    // advance by one
    ++pos;
    if (pos >= queryString.size()) {
      cerr << "! ERROR PARSING QUERY: parameter name expected after '&' at position " << pos 
           << " of queryString \""  << queryString << "\"" << endl;
      return false;
    }
    // looking for next '='
    pos_0 = pos;
    while (pos < queryString.size() && queryString[pos] != '=') ++pos;
    if (pos >= queryString.size() || queryString[pos] != '=') {
      cerr << "! ERROR PARSING QUERY: expected '=' after '&' at position " << pos 
           << " of queryString \""  << queryString << "\"" << endl;
      return false;
    }
    string parameter = queryString.substr(pos_0, pos - pos_0);
    // advance by one
    ++pos;
    pos_0 = pos;
    // looking for next '&' or end of string
    while (pos < queryString.size() && queryString[pos] != '&') ++pos;
    string value = (pos_0 != pos ? queryString.substr(pos_0, pos - pos_0) : "");
    // set queryString parameter
    if      (parameter == "q")  { query = value; queryType = NORMAL; }
    else if (parameter == "c")  nofCompletionsToSend = atoi(value.c_str());
    else if (parameter == "h")  nofHitsToSend        = atoi(value.c_str());
    else if (parameter == "f")  firstHitToSend       = atoi(value.c_str());
    else if (parameter == "en") nofExcerptsPerHit    = atoi(value.c_str());
    else if (parameter == "er") excerptRadius        = atoi(value.c_str());
    else if (parameter == "rd") setHowToRank(value, howToRankDocs, sortOrderDocs);
    else if (parameter == "rw") setHowToRank(value, howToRankWords, sortOrderWords);
    else if (parameter == "n")  setNeighbourhoodSize(value);
    else if (parameter == "fd") fuzzyDamping         = atof(value.c_str());
    else if (parameter == "s")  setAllScoreAggregations(value);
    else if (parameter == "p")  titleIndex           = atoi(value.c_str());
    else if (parameter == "format")   setResponseFormat(value);
    else if (parameter == "callback") { callback = value; format = JSONP; }
    else if (parameter == "exe") { query = value; queryType = EXE; }
    // NEW (baumgari) 10Jan14: This a a parameter which is usually sent by
    // AJAX-Requests. Just ignore it.
    else if (parameter == "_") { }
    else LOG << "! WARNING PARSING QUERY: unknown parameter \"" << parameter << "\"" << endl;
  } 

  // these are currently not user-definable, so set them as follows: compute
  // sufficiently many top hits, and always a certain minimum
  // TODO: this causes a top-k recomputation for every "next page operation",
  // once firstHitToSend is beyond nofTopHitsToComputeAtLeast. Better round to
  // next multiple of nofTopHitsToComputeAtLeast.
  nofTopHitsToCompute        = MAX(firstHitToSend + nofHitsToSend, nofTopHitsToComputeAtLeast);
  nofTopCompletionsToCompute = MAX(nofCompletionsToSend, nofTopCompletionsToComputeAtLeast);

  // NEW(hagn, 15Jun11): 0.01 <= fuzzyDamping <= 1.00
  fuzzyDamping = MAX(fuzzyDamping, 0.01);
  fuzzyDamping = MIN(fuzzyDamping, 1.00);

  return true;

} // end of method extractFromQuery



//! Output parameters to a stream, e.g. cout
ostream& operator<<(ostream& os, QueryParameters& queryParameters)
{
  os << "D(isplay) = "            << queryParameters.displayMode
     << "; M(ax) = "              << queryParameters.nofTopHitsToCompute
     << "; C(ompletions) = "      << queryParameters.nofCompletionsToSend
     << "; F(irst) = "            << queryParameters.firstHitToSend
     << "; H(its) = "             << queryParameters.nofHitsToSend
     << "; E(xcerpts) = "         << queryParameters.nofExcerptsPerHit
     << "; R(adius) = "           << queryParameters.excerptRadius
     << "; S(ynonyms) = "         << queryParameters.synonymMode
     << "; (R)ank (W)ords = "     << queryParameters.howToRankWords
     << "; (R)ank (D)ocs = "      << queryParameters.howToRankDocs
     << "; (F)uzzy (D)amping = "  << queryParameters.fuzzyDamping
     << "; Format = "             << queryParameters.format;
  if (queryParameters.format == QueryParameters::JSONP)
     os << "; Callback = "    << queryParameters.callback;
  return os;
}

std::string QueryParameters::asString(void) const
{
  stringstream ss;

  ss << "Nof top hits to compute              : " << nofTopHitsToCompute << std::endl
     << "Nof hits to send                     : " << nofHitsToSend << std::endl
     << "Nof top completions to compute       : " << nofTopCompletionsToCompute << std::endl
     << "Nof completions to send              : " << nofCompletionsToSend << std::endl
     << "First hit to send                    : " << firstHitToSend << std::endl
     << "Nof excerpts per hit                 : " << nofExcerptsPerHit << std::endl
     << "Excerpt radius                       : " << excerptRadius << std::endl
     << "Display mode                         : " << displayMode << std::endl
     << "Synonym mode                         : " << synonymMode << std::endl
     << "Use Filtering                        : " << useFiltering << std::endl
     << "Merge (0) or hash (1) join           : " << howToJoin << std::endl
     << "How to rank docs                     : " << howToRankDocs << std::endl
     << "How to rank words                    : " << howToRankWords << std::endl
     << "Fuzzy damping                        : " << fuzzyDamping << std::endl
     << "Docs sort order                      : " << sortOrderDocs << std::endl
     << "Words sort order                     : " << sortOrderWords << std::endl
     << "Docs score agg different query parts : " << docScoreAggDifferentQueryParts << std::endl
     << "Docs score agg same completion       : " << docScoreAggSameCompletion << std::endl
     << "Docs score agg different completion  : " << docScoreAggDifferentCompletions << std::endl
     << "Word score agg same document         : " << wordScoreAggSameDocument << std::endl
     << "Word score agg different document    : " << wordScoreAggDifferentDocuments;
  return ss.str();
}

