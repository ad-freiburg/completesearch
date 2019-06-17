#include "Globals.h"
#include "assert.h"

//#include "Globals.h"

using namespace std;

/*
 * ASCII CHARACTER SET (ISO-8859-1, control characters replaced by x)
 *
 *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx !"#$%&'()*+,-./0123456789:;<=>?
 *   @ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~x
 *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿
 *   ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüışÿ
 *
 */
/*
//! CLASS CHARACTER SET (with quick lookup function)
class CharacterSet
{
  public:
    string _chars;
    string _index;
    char index(char c) const { assert(_index.length() == 256); return _index[(unsigned int)(c)]; }
    char size() const { return _chars.length(); }
    char operator[](char c) const { return _chars[c]; }
    CharacterSet(const char* chars);
};

CharacterSet::CharacterSet(const char* chars) : _chars(chars), _index(256, -1)
{
  assert(strlen(chars) < 127);
  for (unsigned int i = 0; chars[i] != 0; ++i) _index[(unsigned char)(chars[i])] = i; 
                                               // NOTE: (unsigned int) would converts chars > 127 to huge numbers!
}

const CharacterSet WORD_CHARS = ("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                                 "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõöøùúûüışÿ");
const CharacterSet        SEP_CHARS(".-_ ");
const CharacterSet MASKED_SEP_CHARS(";=¯¸");
const char ENHANCED_QUERY_BEG = '[';
const char ENHANCED_QUERY_END = ']';
const char ENHANCED_QUERY_SEP = '#';
*/
//! CONVERT AND MASK ENHANCED QUERY 
/*
 *   1.  [...#...#...] -> ...#...#... 
 *   2.  map separators from SEP_CHARS -> MASKED_SEP_CHARS (masking, see Globals.h)
 */
void convertAndMaskEnhancedQuery(string& query)
{
  assert(SEP_CHARS.size() == MASKED_SEP_CHARS.size());
  int pos_beg, pos_end;
  while (true) 
  { 
    // find opening bracket and delete (if no more bracket -> done)
    if ( (pos_beg = query.find(ENHANCED_QUERY_BEG)) != string::npos ) query.erase(pos_beg, 1);
    else break;
    // find closing bracket and delete (if no closing racket -> warning, and assume one at the end)
    if ( (pos_end = query.find(ENHANCED_QUERY_END)) != string::npos ) query.erase(pos_end, 1);
    else { pos_end = query.length(); cerr << " [WARNING: missing '" << ENHANCED_QUERY_END << "' in enhanced query] " << endl; }
    // convert from -> to in covered substring
    for (int i = pos_beg; i < pos_end; ++i)
    {
      char j = SEP_CHARS.index(query[i]);
      if (j != -1) { assert(j < MASKED_SEP_CHARS.size()); query[i] = MASKED_SEP_CHARS[j]; }
    }
  }
} // end of method convertAndMaskEnhancedQuery

    
//! SPLIT UNIT OF ENHANCED QUERY 
/*
 *   1.  <part1>#<part2>#<part3> -> vector of parts
 *   2.  map separators in parts from MASKED_SEP_CHARS -> (unmasking, see Globals.h)
 */
void splitUnitOfEnhancedQueryBlock(string& query, vector<string>& parts)
{
  // unmask all separators
  assert(SEP_CHARS.size() == MASKED_SEP_CHARS.size());
  for (int i = 0; i < query.length(); ++i)
  {
    char j = MASKED_SEP_CHARS.index(query[i]);
    if (j != -1) { assert(j < SEP_CHARS.size()); query[i] = SEP_CHARS[j]; }
  }
  // split at #
  int pos_beg = 0, pos_end;
  while (true)
  {
    pos_end = query.find(ENHANCED_QUERY_SEP, pos_beg); 
    if (pos_end == string::npos) pos_end = query.length();
    parts.push_back(query.substr(pos_beg, pos_end - pos_beg));
    if (pos_end == query.length()) break;
    pos_beg = pos_end + 1;
  }
}



int main(char argc, char** argv)
{
  if (argc <= 1) { cout << "Specify query!" << endl; exit(1); }
  string query = argv[1]; 
  cout << "  Before conversion : \"" << query << "\"" << endl;
  convertAndMaskEnhancedQuery(query);
  cout << "  After conversion  : \"" << query << "\"" << endl;
  vector<string> parts;
  splitUnitOfEnhancedQueryBlock(query, parts);
  cout << "  After split       : " << flush;
  for (unsigned int i = 0; i < parts.size(); ++i) 
    cout << "\"" << parts[i] << "\"" << (i < parts.size()-1 ? " | " : "") << flush;
  cout << endl;
}
