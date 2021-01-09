#include "server/Globals.h"
#include "server/Vocabulary.h"
#include "server/Timer.h"


//! Get the word with the given id (as const, not as lvalue)
const string& Vocabulary::operator[](unsigned int id) const
{
  if (id > _words.size()) 
  {
    ostringstream os;
    os << id;
    CS_THROW(Exception::WORD_ID_OUT_OF_RANGE, os.str());
  }
  return _words[id];
}


//! Append a word to the vocabulary. Precondition: must be larger than last one.
void Vocabulary::push_back(const string& word)
{
  //CS_ASSERT(_words.size() == 0 || word > _words.back());
  _words.push_back(word);
}


//! Return true iff x is strictly less than y, but also "helloanything" < "hello*"
bool Vocabulary::lessThan(const string& x, const string& y) const
{
  unsigned int yl = y.length();
  
  // Case 1: consider y as prefix, with trailing * larger than any character
  if (yl > 0 && y[yl - 1] == '*') 
    return x.substr(0,  yl - 1) <= y;

  // Case 2: consider y as normal string
  return x < y;
}

/*
//! Compare two strings normally, operator returns true iff x < y
bool Vocabulary::CompareNormal::operator()(const string &x, const string &y)
{
  return x < y; 
}


//! Compare two strings normally, but taking y as y* with * larger than any character
bool Vocabulary::ComparePrefix::operator()(const string &x, const string &y)
{
  return x.substr(0, y.length()) <= y;
}
*/

//! Search for a word, returns id of first word (in the given range) which is not strictly smaller
/*
 *   for empty vocabulary, always returns zero
 *   if all words in range are strictly smaller, returns largest id plus one
 *   if empty range, returns UINT_MAX
 *   currently implemented as a simple binary search
 */
unsigned int Vocabulary::findWord(const string& word, 
                                  unsigned int  low,
				  unsigned int  high) const
{
  if (_words.size() == 0)            return 0;

  if (low  >= _words.size())         return UINT_MAX;
  if (high >= _words.size())         high = _words.size() - 1;
  if (low > high)                    return UINT_MAX;

  if (!lessThan(_words[low], word))  return low;
  if (lessThan(_words.back(), word)) return high + 1;

  unsigned int middle;
  while (low < high)
  {
    middle = (low + high) / 2;
    if (lessThan(_words[middle], word))
      low = middle + 1;
    else
      high = middle;
  }
  CS_ASSERT(low == high);
  CS_ASSERT(!lessThan(_words[low], word)); // case word > _words.back() already dealt with above
  CS_ASSERT(low > 0 && lessThan(_words[low-1], word)); // case word <= _words[0] already dealt with above
    //CS_ASSERT(high == _vocabulary.size()-1 || _words[high+1] > word); // assumes words are distinct
  return low;
}

// _____________________________________________________________________________
void Vocabulary::precomputeWordIdMap()
{
  CS_ASSERT(_wordIdMap.size() == 0);
  Timer timer;
  cerr << "* NEW: computing word id map ... " << flush;
  timer.start();
  _wordIdMap.resize(_words.size());
  WordId wordId, mappedWordId;
  string word;
  string wordTmp;
  string filterPrefix = wordPartSep + string("filter") + wordPartSep;
  for (size_t i = 0; i < _words.size(); ++i)
  {
    // By default, word id is mapped to itself.
    mappedWordId = i;
    // If word is of the form "C:1234:algorithm" (without the quotes) map this
    // word id to the word id of "algorithm".
    word = _words[i];
    wordTmp = word;
    // Mapping is required in following cases (C can also be S instead):
    // 1. <wordNorm>:<word> to <word>
    // 2. :filter:<field>:<wordNorm>:<word> to :filter:field:<word>
    // 3. C:1234:<word> to <word>
    // 4. <wordNorm>:C:1234:<word> to <word>
    // 5. :filter:<field>:C:1234:<word> to :filter:<field>:<word>
    // 6. :filter:<field>:C:1234:<wordNorm>:<word> to :filter:<field>:<word>
   
    size_t nextColon = string::npos;
    size_t startOfClusterNum = string::npos;
    string prefix = "";
    // We've found a filter word. Get the whole prefix;
    if (word.compare(0, filterPrefix.size(), filterPrefix) == 0)
    {
      nextColon = word.find(wordPartSep, filterPrefix.size() + 1);
      CS_ASSERT(nextColon != string::npos);
      prefix = word.substr(0, nextColon + 1);
      word = word.substr(nextColon + 1);
    }
    nextColon = word.find(wordPartSep);
    // Minimized to cases 1, 3, 4 and 6. There still might be words like
    // ct:... etc., but no more :filter:<field>:<word> or <word>.
    if (nextColon != string::npos && nextColon != 0 && nextColon < word.size())
    {
      // Case 3 or 6.
      if (nextColon == 1 &&
          (word[0] == 'C' || word[0] == 'S'))
        startOfClusterNum = 2;
      // Case 4.
      else if  (nextColon + 2 < word.size()
            &&  word[nextColon + 2] == wordPartSep
            && (word[nextColon + 1] == 'C' || word[nextColon + 1] == 'S'))
        startOfClusterNum = nextColon + 3;
      // Case: is a cluster and therefore has to be mapped!
      if (startOfClusterNum != string::npos)
      {
        // Find end of numbers.
        nextColon = word.find(wordPartSep, startOfClusterNum);
        // Starting at nextColon it's now <word> or <wordNorm>:<word>.
        size_t nextColonTmp = nextColon;
        if ((nextColon = word.find(wordPartSep, nextColonTmp + 1)) != string::npos)
          word = prefix + word.substr(nextColon + 1);
        else
          word = prefix + word.substr(nextColonTmp + 1);
        wordId = findWord(word);
        CS_ASSERT_LE(0, wordId);
        if (static_cast<size_t>(wordId) < _words.size() &&
            _words[wordId] == word) mappedWordId = wordId;
      }
      // Case 1.
      else if (word.find(wordPartSep, nextColon + 1) == string::npos)
      {
        word = prefix + word.substr(nextColon + 1);
        wordId = findWord(word);
        CS_ASSERT_LE(0, wordId);
        if (static_cast<size_t>(wordId) < _words.size() &&
            _words[wordId] == word) mappedWordId = wordId;
      }
    }
    _wordIdMap[i] = mappedWordId;
  }
  timer.stop();
  cerr << "done in " << timer << endl;
  CS_ASSERT_EQ(_words.size(), _wordIdMap.size());
}

// _____________________________________________________________________________
WordId Vocabulary::mappedWordId(WordId wordId)
{
  if (_wordIdMap.size() == 0) return wordId;
  if (wordId < 0) return wordId;
  if (static_cast<size_t>(wordId) >= _wordIdMap.size()) return wordId;
  return _wordIdMap[wordId];
}

// _____________________________________________________________________________
std::string Vocabulary::asString(void) const
{
  std::stringstream ss;
  ss << "Size  : " << size() << std::endl
     << "Words : ... TODO ... ";
  return ss.str();
}
