#include "server/Globals.h"
#include "server/Query.h"
#include "server/Vocabulary.h"

// PARAMETERS
string MSG_BEG = "! "; /* per default put two newlines after messages like buffer resize etc. */
string MSG_END = "\n"; /* per default put two newlines after messages like buffer resize etc. */
unsigned int HYB_BLOCK_VOLUME = 200*1000; /* the default will give about 1 MB for a block */
                                          /* can be changed via -b option of buildIndex */
string HYB_BOUNDARY_WORDS_FILE_NAME = ""; /* files of prefixes for block division of HYB */
                                          /* can be changed via -b option of buildIndex */
bool SHOW_HUFFMAN_STAT = false;

// GLOBAL VARIABLES / PARAMETERS 
//
//   TODO: some of these should rather be members of  the appropriate class
//
int encoding = Encoding::UTF8;
string localeString = "";
bool showStatistics = true;
char    wordPartSep = '!';

ExcerptsGenerator* excerptsGenerator = NULL;
int nofRunningProcessorThreads = 0; 
size_t excerptsDBCacheSize = 16*1024*1024; /* in bytes */
size_t historyMaxSizeInBytes = 32*1024*1024; 
unsigned int historyMaxNofQueries = 200; // Note: current impl. is quadratic!
bool runMultithreaded = false; // Note: not yet stable, turn on with -m
//! The maximal number of items in a block/list of HYB/INV. Block ignored
//! otherwise. To avoid buildIndex crash when lists > 2GB encountered (e.g. the
//! for terabyte). This feature is NOT YET IMPLEMENTED though! At the time of
//! this writing (20Jan08) I was about to implement it for INV.
size_t maxBlockVolume = std::numeric_limits<size_t>::max();
bool cleanupQueryBeforeProcessing = false;
bool useSuffixForExactQuery = false;
string keepInHistoryQueriesFileName = "";
string warmHistoryQueriesFileName = "";
Score BEST_MATCH_BONUS = 0;

StringConverter globalStringConverter;

//const CharacterSet        SEP_CHARS(".-_ =");
//const CharacterSet MASKED_SEP_CHARS(";+¯¸~");
// Want '-' as a word char. See prefixToRange().
//const CharacterSet        SEP_CHARS("._ =");
//const CharacterSet MASKED_SEP_CHARS(";¯¸~");
// NEW (baumgari) 27Oct14:
// Changed the masked pendant of the . from ° to ? and of the space from ¸ to {.
// Reason: The old letters were not within the ascii range and caused errors.
// For example the ø in Jørgen was recognized as separator.
const CharacterSet        SEP_CHARS(". =,;");//Ingmar: 24Jan07
const CharacterSet MASKED_SEP_CHARS("?{~'^");//Ingmar: 24Jan07
const char ENHANCED_QUERY_BEG = '[';
const char ENHANCED_QUERY_END = ']';
const char ENHANCED_QUERY_SEP = '#';
const char       OR_QUERY_SEP = '|';
const char      NOT_QUERY_SEP = '-';
const CharacterSet WORD_CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                                "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ";

//! If the api uses another word part separator than the server, it should be
//! replaced within the query at the beginning (api sep -> server sep) and
//! within the completions when returning (server sep -> api sep).
char wordPartSepFrontend = wordPartSep;

//! CREATE INDEX FOR SET OF CHARACTERS (for fast lookup)
CharacterSet::CharacterSet(const char* chars) : _chars(chars), _index(256, -1)
{
  assert(strlen(chars) < 127);
  for (unsigned int i = 0; chars[i] != 0; ++i) _index[(unsigned char)(chars[i])] = i; 
                                               // NOTE: (unsigned int) would converts chars > 127 to huge numbers!
}

//! BRING A TIMER VALUE INTO NICE FORMAT, e.g. 2.3 second, 45 microseconds, etc.
char* timeAsString(float microseconds)
{
  char* s = new char[50]; 
  if (microseconds > 60000000) { sprintf(s, "%.1f minutes", microseconds/60000000.0f); }
  else if (microseconds > 1000000) { sprintf(s, "%.1f seconds", microseconds/1000000.0f); }
  else if (microseconds > 1000) { sprintf(s, "%.1f milliseconds", microseconds/1000.0f); }
  else if (microseconds > 1) { sprintf(s, "%.1f microseconds", microseconds); }
  else { sprintf(s, "%8.3f microseconds", microseconds); }
  return s;
}

//! WRITE TIMER VALUE TO OSTREAM with << timer
ostream& operator<<(ostream& os, Timer& timer) { os << timeAsString(timer.usecs()); return os; }

//! PRINT A RATE LIKE 234 MiB/sec
string megsPerSecond(off_t nofBytes, off_t usecs)
{
  ostringstream os;
  if (usecs > 0) os << nofBytes/usecs; else os << "---";
  os << " MiB/sec"; 
  return os.str();
}

//! FORMAT VERY LARGE NUMBERS e.g. 1234567 -> 1,234,567 (for better legibility)
string commaStr(off_t number)
{
  const int size = 15;    // max number of digits
  static char str[size];  // string to return
  char *ptr1, *ptr2;      // place holders
  char tempStr[size];     // workplace
  int counter = 0;        // three's counter
  ptr1 = tempStr;
  do {
    // grab rightmost digit and add value of character zero
    *ptr1++ = (char)(number % 10) + '0';
    // strip off rightmost digit
    number /= 10;
    // if moved over three digits insert comma into string
    if (number &&  !(++counter % 3))
      *ptr1++ = ',';
    // continue until number equal zero
  } while(number);
  //*ptr1=0; cout << endl << tempStr << endl;
  // this loop reverses characters in a string
  for( --ptr1, ptr2 = str; ptr1 >= tempStr; --ptr1)
    *ptr2++ = *ptr1;
  // add the zero string terminator
  *ptr2 = 0;
  return string(str);
}

//! PRINT A NUMBER OF ITEMS e.g.  1 query  but  2 queries
string numberAndNoun(unsigned int number, const char* singular, const char* plural)
{
  ostringstream os;
  if (number == 1) os << number << " " << singular;
  else os << number << " " << plural;
  return os.str();
}

// Try to lock a mutex once immediately and, if this fails, again after an interval.
// Default interval is 0, in which case the method is identicyl to pthread_mutex_trylock
// Returns the return value of the last call to pthread_mutex_trylock
int pthread_mutex_timed_trylock( pthread_mutex_t* mutex, useconds_t microseconds = 0)
{
  // FIRST TRY TO LOCK MUTEX
  assert(mutex);
  int trylock_return =  pthread_mutex_trylock( mutex ); 
  assert( (trylock_return == 0) || (trylock_return == EBUSY));
  if ((trylock_return != 0) && (microseconds > 0))
    {
      // WAIT FOR SOME TIME (could use nanosleep instead)
      usleep(microseconds);
      // SECOND TRY TO LOCK MUTEX
      trylock_return =  pthread_mutex_trylock( mutex ); 
      assert( (trylock_return == 0) || (trylock_return == EBUSY));
    }// end case: first try to lock mutex was unsuccessful
  
  return trylock_return;
}

//! WAIT UNTIL KEY PRESSED (key will not be echoed on terminal)
char keyPressed = 0;
void waitUntilKeyPressed()
{
  /* get current terminal settings */
  struct termios old_settings;
  tcgetattr(0, &old_settings);
  struct termios new_settings = old_settings;
  /* disable canonical mode (line-by-line) and echo */
  new_settings.c_lflag &= ~(ICANON | ECHO);
  /* set buffer size to 1 and timeout to zero */
  new_settings.c_cc[VMIN] = 1;
  new_settings.c_cc[VTIME] = 0;
  /* activate new settings (TCSANOW = immediately) */
  tcsetattr(0, TCSANOW, &new_settings);
  /* wait for the key press */
  keyPressed = getchar();
  /* restore old settings */
  tcsetattr(0, TCSANOW, &old_settings);
}

//! GET HOSTNAME (return default if not exists)
string gethostname(int maxLength, const char* default_if_not_exists)
{
  char value[maxLength];
  int success = gethostname(value, sizeof(value));
  if (success < 0) cout << "! getHostname: hostname not found; using \"" << default_if_not_exists << "\"" << endl;
  return success >= 0 ? value : default_if_not_exists;
}

//! GET ENVIRONMENT VARIABLE (return default if not exists)
string getenv(const char* var_name, const char* default_if_not_exists)
{
  char* value = getenv(var_name);
  if (value == NULL) cout << "! getenv: variable " << var_name << " not found; using \"" << default_if_not_exists << "\"" << endl;
  return value != NULL ? value : default_if_not_exists; 
}


//! LOCK/UNLOCK FILE
void lock_file(int fid, int lock_type)
{
  struct flock fl; fl.l_type   = lock_type;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
  fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
  fl.l_start  = 0;        /* Offset from l_whence         */
  fl.l_len    = 0;        /* length, 0 = to EOF           */
  fl.l_pid    = getpid(); /* our PID                      */
  fcntl(fid, lock_type == F_UNLCK ? F_SETLK : F_SETLKW, &fl);  /* lock (wait if necessary) or unlock */
}



//! LOCK/UNLOCK LOG FILE (for use in cout/cerr)
pthread_mutex_t log_file_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t process_query_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* log_file = NULL;
const char* empty_string = "";
//const char* lock_log_file() { pthread_mutex_lock(&log_file_mutex); lock_file(fileno(log_file), F_WRLCK); return empty_string; }
//const char* unlock_log_file() { lock_file(fileno(log_file), F_UNLCK); pthread_mutex_unlock(&log_file_mutex); return empty_string; }


// _____________________________________________________________________________
string addCdataTagIfNecessary(string text, bool alreadyWellformedXml)
{
  if (alreadyWellformedXml) return text;
  bool needsEscaping = false; 
  for (unsigned int i = 0; i < text.size(); ++i) {
    signed char c = text[i];
    if (c == '<' || c == '>' || c == '&' || c < 32) {
      needsEscaping = true;
      if (i >= 2 && text.compare(i - 2, 3, "]]>") == 0) {
        text[i-1] = '}';
      }
    }
  }
  return needsEscaping == true ? string("<![CDATA[") + text + string("]]>") : text;
}

// _____________________________________________________________________________
string replaceWordPartSeparatorFrontendToBackend(const string& text)
{
  if (wordPartSep == wordPartSepFrontend) return text;
  // Replace any separators by the new wordPartSep.
  string standardizedText;
  standardizedText.reserve(text.size());
  for (size_t i = 0; i < text.size(); i++)
  {
    if (text[i] == wordPartSepFrontend) standardizedText += wordPartSep;
    else standardizedText += text[i];
  }
  return standardizedText;
}

// _____________________________________________________________________________
string replaceWordPartSeparatorBackendToFrontend(const string& text)
{
  if (wordPartSep == wordPartSepFrontend) return text;
  // Replace any separators by the new wordPartSep.
  string apiText;
  apiText.reserve(text.size());
  for (size_t i = 0; i < text.size(); i++)
  {
    if (text[i] == wordPartSep) apiText += wordPartSepFrontend;
    else apiText += text[i];
  }
  return apiText;
}

// _____________________________________________________________________________
string decodeHexNumbers(const string& text)
{
  string decodedText = "";
  for (size_t i = 0; i < text.size(); ++i)
  {
    // Replace + by a whitespace.
    if (text[i] == '+') decodedText += ' ';
    // Escaped character found.
    else if (text[i] == '%' && i + 2 < text.size())
    {
      // First part of hex digit.
      char h1 = tolower(text[i+1]);
      // Iff number: convert the written integer to a real integer by
      // subtrating its ascii value.
      if (h1 >= '0' && h1 <= '9') h1 = h1 - '0';
      // Iff letter: convert the letter to the corresponding integer by
      // subtrating its ascii value and adding 10. E.g: d -> 5 + 10 = 15.
      else if (h1 >= 'a' && h1 <= 'f') h1 = h1 - 'a' + 10;
      // Mistake, % didn't indicate an escaped character.
      else h1 = -1;
      // Do the same for the second part.
      char h2 = tolower(text[i+2]);
      if (h2 >= '0' && h2 <= '9') h2 = h2 - '0';
      else if (h2 >= 'a' && h2 <= 'f') h2 = h2 - 'a' + 10;
      else h2 = -1;
      // Iff escaped character, convert to corresponding char.
      if (h1 != -1 && h2 != -1)
      {
        decodedText += (char)(h1 * 16 + h2);
        i += 2;
      }
      // Iff mistake, % stays %.
      else decodedText += '%';
    }
    // If there is no escaped character, just append the given character.
    else decodedText += text[i];
  }
  return decodedText;
}

// _____________________________________________________________________________
string encodeQuotedQueryParts(const string& query)
{
  string escapedQuery = "";
  string toEscape = "";
  bool isQuoted = false;
  for (size_t i = 0; i < query.size(); i++)
  {
    // Flip escaping mode if an unescaped quote occurs.
    if ((query[i] == '\"' && (i == 0 || query[i - 1] != '\\'))
        || ((i == query.size() - 1) && isQuoted))
    {
      if ((i == query.size() - 1) && (query[i] != '\"'))
        toEscape += query[i];
      if (isQuoted)
      {
        if (!globalStringConverter._bAlreadyInitialized)
          globalStringConverter.init();
        escapedQuery += globalStringConverter.convert(toEscape,
                                   StringConverter::ENCODING_UTF8,
                                   StringConverter::CONVERSION_TO_MASK);
        toEscape.clear();
      }
      isQuoted = !isQuoted;
    }
    // Nonescaping mode: append unmodified.
    else if (!isQuoted) {
      if (i == 0 || query[i - 1] != '\\') escapedQuery += query[i];
      // NEW (baumgari) 12Nov14:
      // If a quote is escaped, erase the escape char and just add the quote.
      // This is done, to be able to find facets containing quotes.
      else escapedQuery[escapedQuery.size() - 1] = '\"';
    }
    // Escaping mode.
    else if (isQuoted)
    {
      toEscape += query[i];
    }
  }
  return escapedQuery;
}


// _____________________________________________________________________________
size_t shiftIfInMiddleOfUtf8MultibyteSequence(const string& s, size_t pos)
{
  size_t i = pos;
  while (i < s.size() && ((s[i] & 192) == 128)) ++i;
  return i - pos;
}


//! READ WORDS FROM FILE (one word per line)
//
//    - used for reading vocabulary of an index (<db>.vocabulary)
//    - used for reading HYB block boundary words (<db>.prefixes)
//
//  template <unsigned char MODE> void CompleterBase<MODE>::
void readWordsFromFile(const string fileName, Vocabulary& words, string what)
{
  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << fileName 
    << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  assert(words.size() == 0);
  cerr << "* reading " << what << " from file \"" << fileName << "\" ... " << flush;
  char line[MAX_WORD_LENGTH+1];
  for(unsigned int i=0; i<MAX_WORD_LENGTH+1;++i) {line[i] = 0;}

  while (fgets(line, MAX_WORD_LENGTH, file) != NULL)
  {
    int len = strlen(line);
    assert(len < MAX_WORD_LENGTH);
    if (len == MAX_WORD_LENGTH - 1)
    { cout << endl << "! ERROR reading from file \"" << fileName
      << "\" (line too long)" << endl << endl;
    len = MAX_WORD_LENGTH - 1;
    line[len] = 0;
      // exit(1);
    }
    while (len > 0 && iswspace(line[len-1])) line[--len] = 0; // remove trailing whitespace (ip newline)
    if (len > 0) words.push_back(line); /* ignore empty lines */
  }
  cerr << "done (" << commaStr(words.size()) << " words)" << endl;
  fclose(file);
  assert(words.size() > 0);
}


//! MASK CTRL CHARS IN STRING (so that it can be printed)
string printable(string s)
{
  for (unsigned int i = 0; i < s.length(); ++i) 
    if (!isprint(s[i])) s[i] = '·';
      //if (isspace(s[i]) || s[i] >= ' ' && s[i] <= '~' || s[i] <= 'À') s[i] = '·';
  return s;
}


//! UTF8 TOLOWER
//
//   looks at the character under the given pointer
//
//   if bit 7 is not set -> perform ordinary tolower
//
//   otherwise check for utf8 multibyte character and convert that to lower, but
//   only if the converted thing has the same number of characters
//
//   if anything goes wrong, just do nothing
//
//   returns the number of bytes converted, or 1 in case of error
//
//   NOTE: always return at least 1 (to avoid infinite loop)
//
//   NOTE: about 50% slower than tolower on wikipedia_en.xml
//   
//   NOTE: inline makes it about 10% faster (so not really worth it)
//
//   NOTE: advancing p inside of the function (and making it void) does
//   not help efficiency (and inline does not make a difference then)
//
int utf8_tolower(char* p)
{
  // CASE: normal character
  if (*p > 0) 
  { 
    *p = tolower(*p);
    return 1;
  }

  // CASE: some more complex thingy
  wchar_t wc;
  int len = mbtowc(&wc, p, 4);
  if (len == 1)
  {
    *p = tolower(*p);
    return 1;
  }
  else if (len > 1)
  {
    wc = towlower(wc);
    int len2 = wctomb(p, wc);
    if (len2 > len) 
    { 
      cerr << "\n\nLENGTH DIFFERENCE AFTER LOWERCASE:\n\n"; 
      if (strlen(p) > 50) p[50] = 0;
      cerr << "Next characters are: '" << p << "'" << endl;
      cerr << "Length before: " << len << endl;
      cerr << "Length after : " << len2 << endl;
      exit(1); 
    }
    return len;
  }
  else
  {
    return 1;
  }
}


//! FOR MULTIPLE MATCHES IN EXCERPTS (e.g., ^pro^seminar)
const char wordPartSeparator = '^';


//! NEW 07Aug07 (Holger): new normalization function
//
//   NOTE: mbtowc only works if locale is utf8
//
//   NOTE: not thread-safe so far, switch to mbrtowcr etc.
//
int normalize(const char* p, string& nc)
{
  // CASE: ordinary character or isolatin1 -> tolower does it
  if (*p < 0 || encoding == Encoding::ISO88591) 
  {
    nc = tolower(*p);
    return 1;
  }

  // CASE: a multibyte utf8 character
  wchar_t wc;
  int len = mbtowc(&wc, p, 4);
  if (len <= 1)
  {
    nc = tolower(*p);
    return 1;
  }
  else
  {
    wc = towlower(wc);
    static char tmp[10];
    if (MB_CUR_MAX > 9) return 1;
    int len2 = wctomb(tmp, wc);
    tmp[len2] = 0;
    nc = tmp;
    return len;
  }
}


//! CHARCTER NORMALIZATION MAP (e.g., À -> A or ü -> ue)
const char* NORM_CHAR_MAP = "A\0" "A\0" "A\0" "A\0" "Ae"  "A\0" "Ae"  "C\0"
                            "E\0" "E\0" "E\0" "E\0" "I\0" "I\0" "I\0" "I\0"
                            "D\0" "N\0" "O\0" "O\0" "O\0" "O\0" "Oe"  "x\0"
                            "Oe"  "U\0" "U\0" "U\0" "Ue"  "Y\0" "P\0" "ss"
                            "a\0" "a\0" "a\0" "a\0" "ae"  "a\0" "ae"  "c\0"
                            "e\0" "e\0" "e\0" "e\0" "i\0" "i\0" "i\0" "i\0"
                            "d\0" "n\0" "o\0" "o\0" "o\0" "o\0" "oe"  "x\0"
                            "oe"  "u\0" "u\0" "u\0" "ue"  "y\0" "p\0" "y\0";

//! NORMALIZE A CHARACTER (according to the map above)
//
//    NOTE: ifdef DONT_NORMALIZE_WORDS, does not do anything
//
void normalize(unsigned char c, std::string& nc)
{
  // Ingmar 09Nov05: Uncommented as Tree does not (currently) normalize either
  // Holger 02Dec05: need normalization to compile the in-use DBCompleteServer 
  // Ingmar 12Dec05: Now complile with STATISTICS defined to turn off normalization
  #ifdef DONT_NORMALIZE_WORDS
  nc = c;
  return;
  #endif

  if (c >= 192)
  {
    nc = NORM_CHAR_MAP[2*(c-192)];
      // std::cerr << "DEBUG MESSAGE: Character \'" << c // << "\'"
      //           << "-> second char of map has code " << (int)NORM_CHAR_MAP[2*(c-192)+1] << std::endl;
    if (NORM_CHAR_MAP[2*(c-192)+1] != '\0')
    {
      // std::cerr << "DEBUG MESSAGE: Converted to TWO characters!" << std::endl;
      nc += NORM_CHAR_MAP[2*(c-192)+1];
    }
  }
  else
  {
    nc = c;
  }
}


//! NORMALIZE A WHOLE STRING (according to the function above)
void normalize(const std::string& word, std::string& norm_word)
{
  norm_word.clear();
  string norm_char;
  for (size_t i = 0; i < word.length(); i++)
  {
    normalize(word[i], norm_char);
    norm_word += norm_char;
  }
}
