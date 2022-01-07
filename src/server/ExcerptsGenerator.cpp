#include "ExcerptsGenerator.h"
#include "ConcurrentLog.h"

//! INITIALIZE STATIC MEMBERS
// The characters that are considered as separating words in the document.
// The last one ('\xa0') really occurs in documents. I have the vague idea
// that this is something like SHIFT+SPACE, but I am not sure.
const string ExcerptsGenerator::nonWordCharacters(" .,!?:;+-*_&#=`@\\|()[]{}>/\"\'\n\xa0\xa8");
// NEW 03Aug06 (Holger) : add-words.pl may add ^ also to tags (though it doesn't really make sense)
const string ExcerptsGenerator::specialCharactersWithinTags(":-_^"); // was ":-_"
const size_t ExcerptsGenerator::numberOfColors = 10;
const char* ExcerptsGenerator::color[10] = { "ffff66", "a0ffff", "99ff99", "ff9999", "ff66ff",
                                             "880000", "00aa00", "886800", "004699", "990099"};

extern char infoDelim;

// _____________________________________________________________________________
ExcerptsGenerator::ExcerptsGenerator(
    const std::string& filename, const size_t& cachesize) : _docsDB(filename)
{
  maxHits = 10;
  showRange = Range(-5, 5);
  minWordLength = 2;
  // NOTE: must be large enough, in order not to discard the ce:person:... monster words 
  // maxWordLength = 150; // was 50
  maxWordLength = 10 * 1024;
}


// _____________________________________________________________________________
void ExcerptsGenerator::computeWordsAndPositions(size_t until) const
{
  if (_listsComplete || until < _wordList.size()) return;
  std::string word, norm_word, norm_char;
  bool tag;
  unsigned long i, j, k;
  if (_positionList.empty()) i = j = 0;
  else i = j = _positionList.back().first + _positionList.back().second;
  pair<unsigned long, unsigned long> pos_pair;

  // Now scan the document as far as required by the "until" parameter.
  while (_wordList.size() <= until && !_listsComplete)
  {
    // Find the beginning of the next word
    i = _document->find_first_not_of(nonWordCharacters, j);
    if (i != std::string::npos)
    {
      // Check whether a tag starts at position i
      tag = false;
      if ((*_document)[i] == '<' && !(tag = tagFound(*_document, i, j))) j = i+1;  // if tag == true, then j is the first position after the tag
      else
      {
        if ((*_document)[i] != '<')
        // Find the first position after the current word
        j = _document->find_first_of(nonWordCharacters, i);
        // LOG << j << "\t"
        //     << i << "\t"
        //     << _document->substr(0, i + 1) << endl;
        if (tag ||
	    ((EG_MIN(j, _document->length()) - i >= minWordLength) &&
	     (EG_MIN(j, _document->length()) - i <= maxWordLength)))
        {
          word.clear();
          // NEW 07Aug07 (Holger): more sophisticated normalization now
          // i.p. can deal with utf8; see Globals.cpp
          k = i;
          unsigned long k_max = EG_MIN(_document->length(), j); 
          while (k < k_max)
            //for (k=i; k<EG_MIN(_document->length(), j); k++)
          {
            k += normalize(&((*_document)[k]), norm_char);
            word += norm_char;
              //normalize(tolower((*_document)[k]), norm_char);
              //length++;
          }
          if (k > k_max && k_max >= i) k = k_max;
          if (tag) capitalize(word);
          pos_pair.first = i;
	  // TODO(bast): k - i is of type unsigned long, while pos_pair is of
	  // type unsigned char. So if k - i > 255, as happens for the long
	  // Exentityx... words, everything after here will be random. FIX IT!!!
	  // CS_ASSERT_LE(k - i, 255);
          pos_pair.second = k - i;
	  // LOG << k << "\t" << i << "\t" << k - i << "\t" << (size_t)(pos_pair.second) << endl;
          _positionList.push_back(pos_pair);  // We also store the position of the word in the string
          _wordList.push_back(word); // The word is stored in the _wordList
        }
      }
    }
    if (i >= _document->length() || j >= _document->length()) _listsComplete = true;
  }
}

// _____________________________________________________________________________
bool ExcerptsGenerator::positionList(unsigned long i,
                                     unsigned long& position) const
{
  if (i < _positionList.size())
  {
    position = _positionList[i].first;
    return true;
  }
  else if (_listsComplete)
  {
    return false;
  }
  else
  {
    computeWordsAndPositions(i);
    if (i < _positionList.size())
    {
      position = _positionList[i].first;
      return true;
    }
    else return false;
  }
}

// _____________________________________________________________________________
bool ExcerptsGenerator::positionList(
    unsigned long i, pair<unsigned long, unsigned long>& pos_pair) const
{
  if (i < _positionList.size())
  {
    pos_pair = _positionList[i];
    return true;
  }
  else if (_listsComplete)
  {
    return false;
  }
  else
  {
    computeWordsAndPositions(i);
    if (i < _positionList.size())
    {
      pos_pair = _positionList[i];
      return true;
    }
    else return false;
  }
}

// _____________________________________________________________________________
bool ExcerptsGenerator::wordList(unsigned long i, std::string& word) const
{
  if (i < _wordList.size())
  {
    word = _wordList[i];
    return true;
  }
  else if (_listsComplete)
  {
    return false;
  }
  else
  {
    computeWordsAndPositions(i);
    if (i < _wordList.size())
    {
      word = _wordList[i];
      return true;
    }
    else return false;
  }
}

// _____________________________________________________________________________
string ExcerptsGenerator::cleanedUpExcerpt(const string& excerpt) const
{
  string excerptCleanedUp = "";
  unsigned int separatorCount = 0;  // The number of successive ^
  enum { COUNT, COPY, DISCARD } mode = COUNT; 
  for (size_t i = 0; i < excerpt.length(); i++)
  {
    // If ^ encountered, start counting or continue if previous were also ^.
    if (excerpt[i] == wordPartSeparator) 
    {
      if (mode == COUNT) ++separatorCount;
      else { mode = COUNT; separatorCount = 1; }
    }
    // otherwise proceed depending on length of last ^^ sequence
    else
    {
      if (mode == COUNT)
      {
	mode = separatorCount == 2 ? DISCARD : COPY;
	if (separatorCount > 2) excerptCleanedUp += ' ';
      }
      // NEW 05.03.2021 (Hannah): Escape " by \" and \ by \\ for clean JSON
      // strings. The only other characters that make problems in a JSON string
      // are control characters (form feed, newline, carriage return, tab),
      // which I assume are already removed at this point.
      //
      // NOTE: The " was escaped by &quot; before, I presume that \" also works
      // for XML?
      if (mode == COPY)
      {
        if (excerpt[i] == '"') excerptCleanedUp += "\\\"";   
        else if (excerpt[i] == '\\') excerptCleanedUp += "\\\\";   
        else excerptCleanedUp += excerpt[i]; 
      }
    }
  }
  return excerptCleanedUp;
}

// _____________________________________________________________________________
void ExcerptsGenerator::uniteIntervals(
    const vector<vector<pair<unsigned long, unsigned long> > >& listOfIntervals,
    vector<pair<unsigned long, unsigned long> >& unionOfIntervals) const
{
  std::vector<std::pair<unsigned long, signed char> > Bounds;
  std::pair<unsigned long, signed char> bound;

  // First, we store all interval bounds in a vector. Along with each bound (as the second component of the pair)
  // we store whether it is a lower (-1) or upper (1) bound.
  // We do not store empty intervals, i.e. ones with upper bound < lower bound.
  for (size_t i=0; i<listOfIntervals.size(); i++)
  {
    for (size_t j=0; j<listOfIntervals[i].size(); j++)
    {
      if (listOfIntervals[i][j].first <= listOfIntervals[i][j].second)
      {
        bound.first = listOfIntervals[i][j].first;
        bound.second = 1;
        Bounds.push_back(bound);
        bound.first = listOfIntervals[i][j].second;
        bound.second = -1;
        Bounds.push_back(bound);
      }
    }
  }
  unionOfIntervals.clear();
  if (Bounds.empty()) return;

  // Now we sort the bounds in non-descending order.
  EG_PairCmp<unsigned long, signed char> comp;
  sort<vector<pair<unsigned long, signed char> >::iterator,
       EG_PairCmp<unsigned long, signed char> > (Bounds.begin(), Bounds.end(), comp);

  // We go from left to right over the sorted bounds and count their depth using the flag indicating lower or upper.
  // Since we excluded empty intervals, the depth will always be non-negative.
  size_t Depth = 0;
  std::pair<unsigned long, unsigned long> interval;
  interval.first = Bounds[0].first;
  for (size_t i=0; i<Bounds.size(); i++)
  {
    Depth += Bounds[i].second;
    // If the depth reaches zero and does not immediately increase again, we have reached the upper bound of the
    // current interval.
    if (Depth == 0 && (i == Bounds.size()-1 || Bounds[i+1].first != Bounds[i].first+1))
    {
      interval.second = Bounds[i].first;
      unionOfIntervals.push_back(interval);
    
      // If there is a next bound (which then must be a lower one), this is the lower bound of a new interval.
      if (i<Bounds.size()-1) interval.first = Bounds[i+1].first;
    }
  }
}

// _____________________________________________________________________________
void ExcerptsGenerator::computePositionsAndIntervalsWithKeyword(
    const vector<Query>& queryParts,
    const size_t& keyIndex,
    const string& document,
    vector<pair<unsigned long, unsigned long> >& intervals,
    vector<vector<unsigned long> >& highlightedPositions,
    const short& max) const
{
  Query keyWord = queryParts[keyIndex];
  std::vector<std::pair<unsigned long, unsigned long> > keyIntervals;
  std::pair<unsigned long, unsigned long> interval;
  size_t i, j, k, l;

  // Use computePositionsAndIntervalsBaseCase() to determine the positions of the keyword.
  // This call also ensures that the _wordList is built from the document.
  std::vector<unsigned long> positions = computePositionsAndIntervalsBaseCase(keyWord, document, keyIntervals,
  highlightedPositions, SHRT_MAX, true);

  keyIntervals.clear();

  // Next, we turn the positions of the keyword into intervals.
  if (positions.size() > 0)
for (i=0; i<positions.size()-1; i+=2)
{
  interval.first = positions[i];
  interval.second = positions[i+1];
  keyIntervals.push_back(interval);
}

  short maxHitsPerPart = (short)(ceil(1.0*max/queryParts.size()));

  intervals.clear();
  highlightedPositions.clear();
  std::vector<std::vector<unsigned long> > highlightedPerKeyInterval;
  std::vector<std::pair<unsigned long, unsigned long> > showPerKeyInterval;
  bool stop;
  for (i=0; i<keyIntervals.size() && intervals.size()<(size_t)max; i++)
  {
highlightedPerKeyInterval.clear();
showPerKeyInterval.clear();
stop = false;
for (j=0; j<queryParts.size() && !stop; j++)
  if (j != keyIndex)
  {
    positions = computePositionsInInterval(queryParts[j], keyIntervals[i], showPerKeyInterval, highlightedPerKeyInterval, maxHitsPerPart);
    if (positions.size() == 0) stop = true;
  }
if (!stop)
{
  for (k=0; k<showPerKeyInterval.size(); k++) intervals.push_back(showPerKeyInterval[k]);
  if (highlight != HL_NONE)
  {
    highlightedPositions.push_back(std::vector<unsigned long>());
    for (k=0; k<highlightedPerKeyInterval.size(); k++)
      for (l=0; l<highlightedPerKeyInterval[k].size(); l++)
    highlightedPositions.back().push_back(highlightedPerKeyInterval[k][l]);
  }
}
  }
}

// _____________________________________________________________________________
vector<unsigned long> ExcerptsGenerator::computePositionsInInterval(
    const Query& query,
    const pair<unsigned long, unsigned long>& searchInterval,
    vector<pair<unsigned long, unsigned long> >& showIntervals,
    vector<vector<unsigned long> >& highlightedPositions,
    const short& max) const
{
  LOG << AT_BEGINNING_OF_METHOD << endl;
  Query queryFirst, queryLast;
  Separator splitSeparator;
  //      query.splitAtLastSeparator(queryFirst, queryLast, sepIndex);
  query.splitAtLastSeparator(&queryFirst, &queryLast, &splitSeparator);
  std::vector<unsigned long> returnPositions;
  if (queryLast.empty()) return returnPositions;

  // TODO !!!!!!!!! OR operator is not yet allowed
  ///// std::vector<Query> prefixes = queryLast.splitAt(queryLast.orOperator());
  /// This should/could also work, as/if there are no "orOperators" yet
  std::vector<Query> prefixes;
  prefixes.push_back(queryLast);

  std::vector<bool> isCompleteWord;
  for (size_t i=0; i<prefixes.size(); i++)
  {
    // A prefix not ending with a '*' is considered complete
    isCompleteWord.push_back(prefixes[i].getLastCharacter() == '*' ? false : true);
    
    // Chop the '*'
    if (!isCompleteWord.back()) prefixes[i].removeLastCharacter();
      }
    
      string word;
    
      if (queryFirst.empty())
      {
    for (size_t i=searchInterval.first+1; i<searchInterval.second && returnPositions.size()<(size_t)max; i++)
    {
      if (!(wordList(i, word))) assert(wordList(i, word));
      for (size_t j=0; j<prefixes.size(); j++)
      {
        if (isCompleteWord[j] ? isEqual(prefixes[j], word) : isPrefix(prefixes[j], word))
        {
          // If the current word matches the query, store its position and the corresponding interval
          returnPositions.push_back(i);
        showIntervals.push_back(std::pair<unsigned long, unsigned long>(EG_MAX(ULPLUS(i, showRange.first), searchInterval.first),
          EG_MIN(ULPLUS(i, showRange.second), searchInterval.second)));
          if (highlight != HL_NONE) 
                highlightedPositions.push_back(std::vector<unsigned long>(1, i));
          break;
        }
      }
    }
  }
  else
  {
    std::vector<std::vector<unsigned long> > highlightedFirst;
    std::vector<std::pair<unsigned long, unsigned long> > showFirst;
    std::vector<unsigned long> positionsFirst = computePositionsInInterval(queryFirst, searchInterval, showFirst, highlightedFirst);
    for (size_t i=0; i<positionsFirst.size() && returnPositions.size()<(size_t)max; i++)
    {
      if (positionsFirst[i] < searchInterval.second-1)
      {
        if (!(wordList(positionsFirst[i]+1, word))) assert(wordList(positionsFirst[i]+1, word));
        for (size_t j=0; j<prefixes.size(); j++)
	{
          if (isCompleteWord[j] ? isEqual(prefixes[j], word) : isPrefix(prefixes[j], word))
          {
            // If the current word matches the query, store its position and the corresponding interval
            returnPositions.push_back(positionsFirst[i]+1);
            showIntervals.push_back(std::pair<unsigned long, unsigned long>(
                EG_MAX(EG_MIN(ULPLUS(positionsFirst[i]+1, showRange.first),
		    showFirst[i].first), searchInterval.first),
                EG_MIN(searchInterval.second, EG_MAX(ULPLUS(positionsFirst[i]+1,
		      showRange.second), showFirst[i].second))));
            if (highlight != HL_NONE)
            {
              highlightedPositions.push_back(highlightedFirst[i]);
              highlightedPositions.back().push_back(positionsFirst[i]+1);
            }
            break;
          }
        }
      }
    }
  }
  return returnPositions;
}


// _____________________________________________________________________________
bool ExcerptsGenerator::isTag(std::string& _s) const
{
  std::string s = _s;
  if (s.length() < 3 || s[0] != '<' || s[s.length()-1] != '>') return false;
  size_t start = s.find_first_not_of('/', 1);
  if (start != 1 && start != 2) return false;
  if (start == 2 && s.length() == 3) return false;
  size_t i;
  for (i = start; i < s.length()-1; i++)
  {
    s[i] = toupper(s[i]);
    if (s[i] == tolower(s[i]) && specialCharactersWithinTags.find_first_of(s[i]) == std::string::npos) break;
  }
  if (i < s.length()-1) return false;

  _s = s;
  return true;
}


// _____________________________________________________________________________
bool ExcerptsGenerator::tagFound(const std::string& doc,
                                 const unsigned long & i,
				 unsigned long & j) const
{
  if (doc[i] != '<') return false;

  size_t end = doc.find_first_of('>', i);
  if (end == std::string::npos) return false;

  string word = doc.substr(i, end-i+1);
  if (isTag(word))
  {
    j = end+1;
    return true;
  } else return false;
}

// _____________________________________________________________________________
void ExcerptsGenerator::capitalize(string& word) const
{
  for (size_t i = 0; i < word.size(); i++) word[i] = toupper(word[i]);
}

// _____________________________________________________________________________
void ExcerptsGenerator::sortHighlightedPositions(
    const vector<vector<vector<unsigned long> > >& allHighlightedPositions,
    vector<pair<unsigned long, unsigned short> >& sortedHighlightedPositions) const
{
  sortedHighlightedPositions.clear();
  for (size_t i=0; i<allHighlightedPositions.size(); i++)
for (size_t j=0; j<allHighlightedPositions[i].size(); j++)
  for (size_t k=0; k<allHighlightedPositions[i][j].size(); k++)
    sortedHighlightedPositions.push_back(std::pair<unsigned long, unsigned short>(allHighlightedPositions[i][j][k], i));

  EG_PairCmp<unsigned long, unsigned short> comp;
  sort<vector<pair<unsigned long, unsigned short> >::iterator,
      EG_PairCmp<unsigned long, unsigned short> >
      (sortedHighlightedPositions.begin(), sortedHighlightedPositions.end(), comp);
  vector<pair<unsigned long, unsigned short> >::iterator it = sortedHighlightedPositions.begin();
  bool first = true;
  unsigned long last_position = 0;
  while (it != sortedHighlightedPositions.end())
  {
    if (first)
    {
      last_position = it->first;
      it++;
      first = false;
    }
    else
    {
      if (it->first == last_position)
      {
	it = sortedHighlightedPositions.erase(it);
      }
      else
      {
        last_position = it->first;
        it++;
      }
    }
  }
}

// _____________________________________________________________________________
void ExcerptsGenerator::getExcerpts(const Query&           query,      
				    const QueryParameters& queryParameters,
				    const QueryResult&     result,
					    vector<HitData>& hits)
{
  LOG << AT_BEGINNING_OF_METHOD << "; query is \"" << query << "\"" << endl;
  CS_ASSERT(hits.size() == 0);

  //const DocId first_hit = queryParameters.firstHit;
  //const DocId nof_hits = queryParameters.nofHits;
  //log << "* getting excerpts ... " << flush;
  
  setMaxHits(queryParameters.nofExcerptsPerHit);
  setExcerptRadius(-(queryParameters.excerptRadius), queryParameters.excerptRadius);
  unsigned int lastHit = queryParameters.firstHitToSend + queryParameters.nofHitsToSend;
  // NEW 02Apr09 (Hannah): cut off at number of hits computed and not at the
  // total number of hits; this is certainly correct, since we evaluate
  // result._topDocIds[i] in the loop with the index i going to lastHit - 1.
  unsigned int nofHitsComputed = result._topDocIds.size();
  if (lastHit > nofHitsComputed) lastHit = nofHitsComputed;
  for (unsigned int i = queryParameters.firstHitToSend; i < lastHit; ++i)
  {
    assert(i < result.nofTotalHits);
    assert(i < result._topDocIds.size());
    #ifndef NDEBUG
    cout << "i = " << i << ", score = " << result._topDocScores[i] << ", docId = " << result._topDocIds[i] << endl;
    #endif
    hits.push_back(hitDataForDocAndQuery(query, result._topDocIds[i], queryParameters.titleIndex, HL_XML));
    hits.back().score =  result._topDocScores[i];
    #ifndef NDEBUG
    cout << "pushed back hit (i = " << i << ")" << flush;
    #endif
  }
  LOG << AT_END_OF_METHOD << "; query was \"" << query << "\"" << endl;
}  


//! Computes the excerpt from the intervals defining it.
/*!
 *  \param 		document 			The document for which the excerpt is to be generated.
 *  \param 		intervals 			The vectors of intervals (of word positions) that define the excerpt.
 *  \param 		highlightedPositions 		The vector of positions and color-values for highlighting.
 *
 *  Highlighting is only done if the variable \a highlight is not \c HL_NONE.
 */
void ExcerptsGenerator::generateExcerptsFromIntervals
 (const std::string& document, 
  const std::vector<std::pair<unsigned long, unsigned long> >& intervals,
  const std::vector<std::pair<unsigned long, unsigned short> >& highlightedPositions) const
{
  // HACK
  // cout << "!!! DOCUMENT (generateExcerptsFromIntervals): " << flush;
  // for (size_t i = 0; i <= 100; ++i)
  //   cout << "[" << document[i] << "]{" << (int)(document[i]) << "}" << flush;
  // cout << endl;

  _excerpts.clear();
  string excerpt;
  string excerptCleanedUp;
  unsigned long LeftStrPos, RightStrPos, Low, Up=ULONG_MAX;
  static string chColorFirst("<b style=\"color:black;background-color:#");
  static string chColorMiddle("\">");
  static string chColorLast("</b>");
  static string insertFirst("");
  static pair<unsigned long, unsigned long> pos_pair =
    pair<unsigned long, unsigned long>(0, 0);
  static const string dots(" ... ");
  unsigned long index = 0, highlightStrPos = 0;

  // EMPTY DOCUMENT 
  // NEW 18Dec06 (Holger): if no excerpts don't print anything (not even empty line)
  if (_wordList.size() == 0)
  {
    //_excerpts.push_back("Document contains no words");
    return;
  }

  // CASE : NO MATCHES WITHIN DOCUMENT (but in title) -> return first words as excerpt
  if (intervals.size() == 0)
  {
      computeWordsAndPositions((size_t)(showRange.second - showRange.first));
      Up = EG_MIN(_wordList.size()-1, (size_t)(showRange.second - showRange.first));
      LeftStrPos = _positionList[0].first;
      RightStrPos = _positionList[Up].first + _positionList[Up].second;
        // NEW 24Sep06 (Holger): show document from very beginning (not just
        // from what is considered as the first real word) and also show all
        // non-space characters after what is considered the last real word)
        // E.g., for a small document "1 Examplar (verfügbar)" the excerpt that
        // would have been show so far was "Exemplar (verfügbar"
        LeftStrPos = 0; int shift = 0;
        while (RightStrPos < document.size() && 
                                                document[RightStrPos] != ' ' && shift++ < 5) RightStrPos++;
      excerpt = getPartOfString(document, LeftStrPos, RightStrPos - LeftStrPos);
      // remove special characters (until Aug'05 only the ^) from excerpt
      _excerpts.push_back(cleanedUpExcerpt(excerpt));
        //_excerpts.push_back(getPartOfString(document, LeftStrPos, RightStrPos - LeftStrPos));
  }
  else
  // CASE : AT LEAST ONE MATCH WITHIN DOCUMENT
  {
    for (size_t i = 0; i < intervals.size(); i++)
    {
      // the excerpt for this interval, initially empty
      excerpt = ""; 
      // To be on the safe side we explicitly avoid range exceedings
      computeWordsAndPositions(intervals[i].second);
      Low = EG_MAX(0, intervals[i].first);
      Up = EG_MIN(_wordList.size()-1, intervals[i].second);
      // Set i-th excerpt. Precede by ... if not beginnning of document.
      if (Low > 0) excerpt += dots;
      // NEW 10.02.2021: If Low == 0, take text from very beginning of document
      // (there may by non-word characters in the beginning). Also show all
      // non-space characters after the last word, like in the case above (but
      // without the shift, which - I guess - is there in order to avoid very
      // long excerpts in case of very long sequences of non-space chars).
      LeftStrPos = Low > 0 ? _positionList[Low].first : 0;
      RightStrPos = _positionList[Up].first + _positionList[Up].second;
      while (RightStrPos < document.size() && document[RightStrPos] != ' ') RightStrPos++;
      excerpt += getPartOfString(document, LeftStrPos, RightStrPos - LeftStrPos);

      // highlight matching query term in excerpt (if asked for)
      if (highlight != HL_NONE)
      {
        assert(index>=highlightedPositions.size() || highlightedPositions[index].first >= intervals[i].first);
        while (index<highlightedPositions.size() && highlightedPositions[index].first <= intervals[i].second)
        {
          positionList(highlightedPositions[index].first, pos_pair);
          highlightStrPos = pos_pair.first + excerpt.length() - RightStrPos;
          // NEW 14Nov07 (Holger): also support abstract xml highlighting 
          if      (highlight == HL_HTML) {
            insertFirst = chColorFirst+color[(highlightedPositions[index].second)%numberOfColors]+chColorMiddle;
          }
          else if (highlight == HL_XML) {
            ostringstream os;
            os << "<hl idx='" << highlightedPositions[index].second << "'>";
            insertFirst = os.str();
          }
          insertIntoExcerpt(highlightStrPos, insertFirst, &excerpt);
          highlightStrPos += insertFirst.length() + pos_pair.second;

          // NEW 14Nov07 (Holger): also support abstract xml highlighting
          if      (highlight == HL_HTML) insertIntoExcerpt(highlightStrPos, chColorLast, &excerpt);
          else if (highlight == HL_XML)  insertIntoExcerpt(highlightStrPos, "</hl>", &excerpt);
          index++;
        }
      }

      // add trailing ... if not end of document (TODO: can't this come before the highlighting?)
      if (!_listsComplete || Up < _wordList.size()-1) 
        excerpt += dots;
      // remove special characters (until Aug'05 only the ^) from excerpt
      _excerpts.push_back(cleanedUpExcerpt(excerpt));
        //excerptCleanedUp = "";
        //for (size_t i = 0; i < excerpt.length(); i++)
        //  if (excerpt[i] != wordPartSeparator) excerptCleanedUp += excerpt[i];
        //_excerpts.push_back(excerptCleanedUp);
    }
  }
}





// Computes relevant excerpts for given query and document.
HitData ExcerptsGenerator::hitDataForDocAndQuery(const Query& query,
                                                 const DocId& docId,
						 const unsigned int titleIndex,
                                                 int _highlight) const
{
  highlight = (Highlighting)(_highlight);

  // NEW 08Aug07 (Holger): use new Document class
  Document document;
    //ExcerptData documentData;         // title, url, and complete text of a single document
    
  // Get title, url and text of the document with the given id.
  _docsDB.getDocument(docId, document);

  // DEBUG(bast): find out why the highlighting puts a </hl> right in the middle
  // of a UTF-8 multibyte character (after Universit.tatsgeb.ude for
  // unifr.docs).
  // if (docId == 3935) 
  // {
  //   cout << "!!! DOCUMENT (hitDataForDocAndQuery): " << flush;
  //   const char* p = document.getText().c_str();
  //   for (size_t i = 0; i <= 100; ++i)
  //     cout << "[" << *(p + i) << "]{" << (int)(*(p + i)) << "}" << flush;
  //     // cout << "[" << document.getText()[i] << "]{" << (int)(document.getText()[i]) << "}" << flush;
  //   cout << endl;

  //   FILE* file = fopen("DOC3935.txt", "w");
  //   fwrite(document.getText().c_str(), 1, document.getText().size(), file);
  //   fclose(file);
  // }

  // During the first call of computePositionsAndIntervals(), _wordList will be filled with all words occuring
  // in the current document in the order they occur (duplicates inclusively). _positionList will be filled with
  // the positions of these words in the document string (in the same order).
  _wordList.clear();
  _positionList.clear();
  _document = &(document.getText());
  _listsComplete = false;

  // the list of excerpts that will be returned for this document, initially empty
  _excerpts.clear();

  std::vector<std::vector<std::pair<unsigned long, unsigned long> > > allIntervals;        
  // allIntervals will contain for each part of the query the list of intervals (of word positions) that contain
  // a hit for this part of the query.
  allIntervals.clear();

  std::vector<std::vector<std::vector<unsigned long> > > allHighlightedPositions;
  // allHighlightedPosisions[i][j] will contain the list of positions to be highlighted in the j-th interval for
  // the i-th part of the query.
  allHighlightedPositions.clear();

  // We split the query at space characters into blocks.
  // These two asserts are just checks. Not really necessary
  //        assert(fixed_separators._separators[SAME_DOC].first == " ");
  //        assert(fixed_separators._separators[PAIRS].first == "=");
  assert(fixed_separators._separators[SAME_DOC]._separatorString == " ");
  assert(fixed_separators._separators[PAIRS]._separatorString == "=");
  //        std::vector<Query> queryParts = query.splitAt(fixed_separators._separators[SAME_DOC].first + separators._separators[PAIRS].first);
  std::vector<Query> queryParts = query.splitAt(fixed_separators._separators[SAME_DOC]._separatorString + fixed_separators._separators[PAIRS]._separatorString);
  // maxHitsPerPart is the maximum number of hits that are computed for each part of the query
  short maxHitsPerPart = (short)(ceil(1.0*maxHits/queryParts.size()));

  size_t keyIndex;

  std::vector<Query> dotsSeparatedParts;

  std::vector<std::pair<unsigned long, unsigned long> > intervals;
  std::vector<std::pair<unsigned long, unsigned long> > unionOfIntervals;
  ////        std::vector<std::vector<std::pair<unsigned long, unsigned long> > > allIntervals;

  std::vector<std::vector<unsigned long> > highlightedPositions;
  std::vector<std::pair<unsigned long, unsigned short> > sortedHighlightedPositions;
  ////      std::vector<std::vector<std::vector<unsigned long> > > allHighlightedPositions;
  bool more = false;

  for (size_t j=0; j<queryParts.size(); j++)
  {
    // check whether queryParts[j] contains a keyword and call either computePositionsAndIntervals or
    // computePositionsAndIntervalsWithKeyword.
    dotsSeparatedParts = queryParts[j].splitAtDots();
    keyIndex = 0;
    while (keyIndex < dotsSeparatedParts.size() && !dotsSeparatedParts[keyIndex].isKeyword()) keyIndex++;

    // intervals is set to the list of intervals that contain a hit for the current part of the query.
    // highlightedPosisions contains for each such interval the list of positions to be highlighted
    // in that interval.
    // The number of hits that are computed is at least one and at most maxHitsPerPart.

    if (keyIndex == dotsSeparatedParts.size())
    {
      computePositionsAndIntervals(queryParts[j], document.getText(), intervals, highlightedPositions, maxHitsPerPart+1);
    }
    else
    {
      computePositionsAndIntervalsWithKeyword(dotsSeparatedParts, keyIndex, document.getText(), intervals,
          highlightedPositions, maxHitsPerPart+1);
    }
    if ((short)(intervals.size()) > maxHitsPerPart)
    {
      more = true;
      intervals.pop_back();
      if (highlight != HL_NONE) highlightedPositions.pop_back();
    }

    allIntervals.push_back(intervals);
    if (highlight != HL_NONE) allHighlightedPositions.push_back(highlightedPositions);
  }
  // LOG << "!!! Here are the intervals: " << flush;
  // for (size_t i = 0; i < intervals.size(); ++i)
  //   LOG << "[" << intervals[i].first << ", " << intervals[i].second << "]" << flush;
  // LOG << endl << endl;

  // All intervals are united and the union is stored in unionOfIntervals as an ordered list of disjoint intervals.
  uniteIntervals(allIntervals, unionOfIntervals);

  // The list of positions to be highlighted is sorted and duplicates are eliminated. The result is stored
  // in sortedHiglightedPositions, where each element is a pair of a position and an index defining the
  // color for the highlighting. The color index is equal to the index of the part of the query this position
  // belongs to.
  if (highlight != HL_NONE) sortHighlightedPositions(allHighlightedPositions, sortedHighlightedPositions);

  // The excerpt is built from the intervals in the union and stored in the string Excerpt. If highlight!=HL_NONE,
  // this string will contain the HTML-commands to highlight the positions in sortedHighlightedPositions in
  // the correct color.
  generateExcerptsFromIntervals(document.getText(), unionOfIntervals, sortedHighlightedPositions);

  // HACK.
  // for (size_t i = 0; i < _excerpts.size(); ++i) cout << "!!! \"" << _excerpts[i] << "\"" << endl;

  // if applicable, add a dummy excerpt indicating that more excerpts are there than we return
  if (!_excerpts.empty() && more)
  {
    _excerpts.push_back("... [there are more matches] ...");
  }


  // FINALLY SET HIT DATA AND ADD TO LIST OF HITS

  // title, url, and list of excerpts of a single hit (initialized)
  HitData hitData;                  
  // set docId
  hitData.docId = document.getDocId();
  // set title (with special characters removed)
  string documentTitle = document.getTitle();
  for (size_t j = 0; j < documentTitle.length(); j++)
    if (documentTitle[j] != wordPartSeparator) hitData.title += documentTitle[j];
  if (infoDelim != '\0')
    hitData.title = getPartOfMultipleField(titleIndex, hitData.title);
  // set URL (with special characters removed)
  string documentUrl = document.getUrl();
  for (size_t j = 0; j < documentUrl.length(); j++)
    if (documentUrl[j] != wordPartSeparator) hitData.url += documentUrl[j];
  // set list of excerpts
  hitData.excerpts = _excerpts;

  // add to list of hits
  return hitData;
}


//! Computes the positions of hits and the intervals of word positions that should be part of the excerpt.
std::vector<unsigned long> ExcerptsGenerator::computePositionsAndIntervals(
    Query query,
    const std::string& document,
    std::vector<std::pair<unsigned long, unsigned long> >& intervals,
    std::vector<std::vector<unsigned long> >& highlightedPositions,
    const short max) const
{
  query.cleanForHighlighting();

  // This vector is needed later to store which positions in the document have already been visited.
  static std::vector<bool> visited;

  // NEW 28Jan07 (Holger): replace all : in query by x, so that highlighting 
  // for words like ^^cxentityx^^albert^^einstein^^^Albert^^^the^^^Great works
  // TODO: this is a hack and should be done cleaner at some point, e.g., as it is 
  // now prefixes like cxen of real words (if such a thing exists) would also match
  string queryString = query.getQueryString();
  for (unsigned int i = 0; i < queryString.length(); ++i) 
    if (queryString[i] == ':') queryString[i] = 'x';
  query.setQueryString(queryString);

  // First, we split the query string at the last separator
  signed char sepIndex=0;
  Query queryFirst, querySecond;
  Separator splitSeparator;
  if (!query.splitAtLastSeparator(&queryFirst, &querySecond, &splitSeparator))
  {
    // This is the base case.
    return computePositionsAndIntervalsBaseCase(query, document, intervals, highlightedPositions, max);
  }
  else
  {
    sepIndex = splitSeparator._separatorIndex;
    assert((sepIndex == FULL) || (sepIndex < (signed char) fixed_separators._separators.size()));
    //        string separator = fixed_separators._separators[sepIndex].first;
    string separator = splitSeparator._separatorString;
    //        Range range = separators._separators[sepIndex].second;
    Range range = splitSeparator._intersectionWindow;


    // Split the second part of the query at the or-operators
    // TODO !!! or operators is not yet allowed!
    //////std::vector<Query> prefixes = querySecond.splitAt(querySecond.orOperator());
    std::vector<Query> prefixes;
    prefixes.push_back(querySecond);

    std::vector<bool> isCompleteWord;
    for (size_t i=0; i<prefixes.size(); i++)
      {
        // A prefix not ending with a '*' is considered complete
        isCompleteWord.push_back(prefixes[i].getLastCharacter() == '*' ? false : true);
        
        // Chop the '*'
        if (!isCompleteWord.back()) prefixes[i].removeLastCharacter();
      }

  std::vector<unsigned long> Positions, ReturnPositions;
  std::vector<std::pair<unsigned long, unsigned long> > intervalsFirst;
  std::vector<std::vector<unsigned long> > highlightedFirst;
  
  // Recursively compute the intervals and positions for the first part of the query
  Positions = computePositionsAndIntervals(queryFirst, document, intervalsFirst, highlightedFirst);
  intervals.clear();
  highlightedPositions.clear();
  
  // In this vector we store which position we have already visited.
  // In this way we avoid duplicates in the positions-vector.
  visited.clear();
  
  unsigned long i = 0, k;
  unsigned long j, jmax;
  
  while (i<Positions.size() && ReturnPositions.size()<(unsigned long)max)
  {
    computeWordsAndPositions(ULPLUS(Positions[i], range.second));
    j = ULPLUS(Positions[i], range.first);
    jmax = EG_MIN((unsigned long)(_wordList.size())-1, ULPLUS(Positions[i], range.second));
    while (j<=jmax && ReturnPositions.size()<(size_t)max)
    {
      if (j >= visited.size()) visited.resize(2*j+1, false);
      if (!visited[j])
      {
        for (k=0; k<prefixes.size(); k++)
        {
          if (isCompleteWord[k] ? isEqual(prefixes[k], _wordList[j]) : isPrefix(prefixes[k], _wordList[j]))
          {
            // If the current word matches the query, store its position and the corresponding interval
            ReturnPositions.push_back(j);
            computeWordsAndPositions(EG_MAX(ULPLUS(j, showRange.second), intervalsFirst[i].second));
            intervals.push_back(std::pair<unsigned long, unsigned long>(EG_MIN(ULPLUS(j, showRange.first), intervalsFirst[i].first),
              EG_MIN((unsigned long)(_wordList.size())-1, EG_MAX(ULPLUS(j, showRange.second), intervalsFirst[i].second))));
            if (highlight != HL_NONE)
            {
              highlightedPositions.push_back(highlightedFirst[i]);
              highlightedPositions.back().push_back(j);
            }
            break;
          }
        }
        visited[j] = true;
      }
      j++;
    }
    i++;
  }

  return ReturnPositions;
  }
}



//! Computes positions of hits and intervals of word positions if the query does not contain separators.
vector<unsigned long> ExcerptsGenerator::computePositionsAndIntervalsBaseCase(
    const Query& query,
    const std::string& doc,
    std::vector<std::pair<unsigned long, unsigned long> >& intervals,
    std::vector<std::vector<unsigned long> >& highlightedPositions,
    const short max, const bool keywordSearch) const
{
  std::vector<unsigned long> positions;
  unsigned long i, k;

  intervals.clear();
  highlightedPositions.clear();

  // Split the query at the or-operators
  // NEW 24Jan06 (Holger): maybe it works now [experimental]
  // TODO (for Ingmar): there is no orOperator() in Query anymore, so I (Holger) put the "|" explicitly below
  vector<Query> queryParts = query.splitAt("|");
  //std::vector<Query> queryParts = query.splitAt(query.orOperator());
  //std::vector<Query> queryParts; queryParts.push_back(query);

  vector<bool> isCompleteWord;
  for (i=0; i<queryParts.size(); i++)
  {
    // NEW 06Mar13 (baumgari): There might be some encoded special characters
    // like dots etc.. Those should be decoded now. See CompleterBase.cpp:237 8Feb13.
    queryParts[i].setQueryString(decodeHexNumbers(queryParts[i].getQueryString()));
    // A prefix not ending with a '*' is considered complete
    isCompleteWord.push_back(queryParts[i].getLastCharacter() == '*' ? false : true);
    // Chop the '*'
    if (!isCompleteWord.back()) queryParts[i].removeLastCharacter();
  }

  string word;
  i = 0;
  while (wordList(i, word) && positions.size() < (size_t)max)
  {
    for (k=0; k<queryParts.size(); k++)
    {
      if (keywordSearch)
      {
        // If we are looking for a keyword, the current word must be a tag. If the keyword(-prefix) matches this tag,
        // we store its position.
        if (isTag(word) && queryParts[k].matchesTag(word, isCompleteWord[k])) positions.push_back(i);
      }
      else
      {
        if (isCompleteWord[k] ? isEqual(queryParts[k], word) : isPrefix(queryParts[k], word))
        {
          positions.push_back(i);
          if (highlight != HL_NONE) 
                highlightedPositions.push_back(std::vector<unsigned long>(1, i));
          intervals.push_back(std::pair<unsigned long, unsigned long>(ULPLUS(i, showRange.first), ULPLUS(i, showRange.second)));
          // LOG << "!!! " << i << " {" << showRange.first << ", " << showRange.second << "} " 
          //     << "[" << intervals.back().first << ", " << intervals.back().second << "]" << endl;
          break;
        }
      }
    }
    i++;
  }
  return positions;
}


// _____________________________________________________________________________
void ExcerptsGenerator::insertIntoExcerpt(size_t pos, const string& insert,
                                          string* excerpt)
{
  assert(excerpt != NULL);
  pos += shiftIfInMiddleOfUtf8MultibyteSequence(*excerpt, pos);
  excerpt->insert(pos, insert);
}

// _____________________________________________________________________________
string ExcerptsGenerator::getPartOfString(const string& document, size_t pos,
                                          size_t length)
{
  size_t posBegin = pos + shiftIfInMiddleOfUtf8MultibyteSequence(document, pos);
  size_t posEnd = pos + length 
    + shiftIfInMiddleOfUtf8MultibyteSequence(document, pos + length);
  return document.substr(posBegin, posEnd - posBegin);
}

// _____________________________________________________________________________
bool ExcerptsGenerator::isRealPrefix(const string& candidate,
                                     const string& word)
{
  if (candidate.length() > word.length()) 
  {
    return false;
  }
  else
  {
    for (size_t i = 0; i < candidate.length(); i++)
    {
      if (candidate[i] != word[i]) return false;
    }
    return true;
  }
}

// _____________________________________________________________________________
bool ExcerptsGenerator::isEqual(const string& word_from_query,
                                const string& word_from_doc)
{
  // If word from doc does not start with a ^ then ordinary string comparison.
  if (word_from_doc.empty() || word_from_doc[0] != wordPartSeparator) 
  {
    return (word_from_doc == word_from_query);
  }
  // Otherwise an exact match with a part between ^...^ counts as equality.
  else
  {
    if (word_from_query.empty()) return false;
    size_t start = 0;
    size_t i;
    size_t j;
    while (start != string::npos)
    {
      // Skip sequence of ^ .
      while (start < word_from_doc.length() &&
	     word_from_doc[start] == wordPartSeparator) ++start; 
      i = start;
      j = 0;
      // See if candidate string matches from here, ignoring further ^. 
      // NEW: Stop at ^^ though. Otherwise this becomes very inefficient for
      // long words with many ^^ like we have them in Björn's version of Ester.
      bool previousCharacterWasSeparator = false;
      while ((i < word_from_doc.length()) && (j < word_from_query.length()))
      {
        if (word_from_doc[i] == wordPartSeparator)
	{
	  if (previousCharacterWasSeparator) break;
	  i++;
	  previousCharacterWasSeparator = true;
	}
        else if (word_from_doc[i] != word_from_query[j])
	{
	  break;
	}
        else
        {
          i++;
          j++;
	  previousCharacterWasSeparator = false;
        }
      }
      if (i == word_from_doc.length() && j == word_from_query.length())
      {
	return true;
      }
      else
      {
	start = word_from_doc.find(wordPartSeparator, start);
      }
    }
    return false;
  }
}

// _____________________________________________________________________________
bool ExcerptsGenerator::isEqual(const Query& word_from_query,
                                const string& word_from_doc)
{
  return isEqual(word_from_query.getQueryString(), word_from_doc);
}

// _____________________________________________________________________________
bool ExcerptsGenerator::isPrefix(const string& candidate, const string& word)
{
  LOG << IF_VERBOSITY_HIGH
      << "isPrefix(" << candidate << ", " << word << ") ... ";
  // If word does not start with a ^ do an ordinary prefix match.
  if (word.empty() || word[0] != wordPartSeparator) 
  {
    bool ret = isRealPrefix(candidate, word);
    LOG << IF_VERBOSITY_HIGH << (ret ? "TRUE" : "FALSE") << endl;
    return ret;
  }
  // Otherwise each ^ starts a new subword.
  else
  {
    size_t start = 0;
    size_t i;
    size_t j;
    while (start != string::npos)
    {
      // Skip sequence of ^ .
      while (start < word.length() && word[start] == wordPartSeparator) ++start;
      i = start;
      j = 0;
      // See if candidate string matches from here, ignoring further ^. 
      // NEW: Stop at ^^ though. Otherwise this becomes very inefficient for
      // long words with many ^^ like we have them in Björn's version of Ester.
      bool previousCharacterWasSeparator = false;
      while ((i < word.length()) && (j < candidate.length()))
      {
        if (word[i] == wordPartSeparator)
	{
	  if (previousCharacterWasSeparator) break;
	  i++;
	  previousCharacterWasSeparator = true;
	}
        else if (word[i] != candidate[j])
	{
	  break;
	}
        else
	{
	  i++;
	  j++;
	  previousCharacterWasSeparator = false;
	}
      }
      // Candidate string matched.
      if (j == candidate.length())
      {
        LOG << IF_VERBOSITY_HIGH << "TRUE" << endl;
        return true;
      }
      // Otherwise continue at the next occurrence of ^.
      else
      {
        start = word.find(wordPartSeparator, start);
      }
    }
    LOG << IF_VERBOSITY_HIGH << "FALSE" << endl;
    return false;
  }
}

// _____________________________________________________________________________
bool ExcerptsGenerator::isPrefix(const Query& candidate, const string& word)
{
  return isPrefix(candidate.getQueryString(), word);
}


// _____________________________________________________________________________
string ExcerptsGenerator::getPartOfMultipleField(const unsigned int titleIndex,
    					         const string& field)
{
  unsigned int actIndex = 0;
  size_t start = 0;
  size_t splitPos = field.find(infoDelim);
  size_t firstDelim = splitPos;
  while (splitPos != string::npos && actIndex != titleIndex)
  {
    start = splitPos + 1;
    splitPos = field.find(infoDelim, start);
    actIndex++;
  }
  if (titleIndex > actIndex)
    return field.substr(0, firstDelim);
  else
    return field.substr(start, splitPos - start);
}
