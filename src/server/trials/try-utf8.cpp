#include "Globals.h"
#include "Timer.h"

// NOTE: ü and Ü in utf8 are Ã¼ and Ãœ respectively

#define TRY(block) try { block } catch (exception& e) { cout << e.what() << endl << endl; exit(1); }
#define MAX_SHOW 200

void printUsage() 
{
  cout << "Usage: test-utf8 <any text file>\n\n"
          "reads file and times two methods for converting it to lower case:\n\n"
          "1. ordinary tolower (doesn't deal properly with utf8\n" 
          "2. my utf8_tolower\n\n";
}

int __utf8_tolower(char* p); // distinguish it from the one in Globals.h
void showStringUpToLength(char* s, unsigned int max);

int main(char argc, char** argv)
{
  cout << endl << EMPH_ON 
       << "TEST & TIME UTF8 TOLOWER (" << VERSION << ")" 
       << EMPH_OFF << endl << endl;

  cout << "Locale (LC_CTYPE) is: " << setlocale(LC_CTYPE, "en_US.utf8") << endl << endl;
  //cout << "Locale (LC_CTYPE) is: " << setlocale(LC_CTYPE, "en_US") << endl << endl;
  //cout << "Locale (LC_CTYPE) is: " << setlocale(LC_CTYPE, NULL) << endl << endl;
  
  if (argc <= 1) { printUsage(); exit(1); }
  const char* filename =  argv[1];
  FILE* file = fopen(filename, "r");
  if (file == NULL) { perror("fopen"); exit(1); }

  Timer timer;

  // READ WHOLE FILE INTO MEMORY
  cout << "reading \"" << filename << "\" ... " << flush;
  timer.start();
  off_t size;
  char* contents;
  struct stat buf;
  stat(filename, &buf);
  size = buf.st_size;
  TRY
  (
    contents = new char[size + 1];
    fread(contents, 1, size, file);
    fclose(file);
    contents[size] = 0;
  )
  timer.stop();
  if (strlen(contents) != size)
  {
    cerr << "ERROR: file has " << commaStr(size) << " bytes"
         << ", but got only " << commaStr(strlen(contents)) << "\n\n";
    exit(1);
  }
  cout << "done in " << timer << " (" << commaStr(size) << " bytes)" << endl << endl;

  showStringUpToLength(contents, MAX_SHOW);
  cout << endl << endl;

  // MAKE ORDINARY COPY OF STRING
  cout << "making copy of string ... " << flush;
  timer.start();
  char* contents_copy;
  TRY ( contents_copy = strdup(contents); )
  timer.stop();
  cout << "done in " << timer << endl << endl;

  // MAKE WIDE CHARACTER COPY OF STRING
  cout << "making wide character copy of string ... " << flush;
  timer.start();
  wchar_t* wcs;
  TRY ( wcs = new wchar_t[size]; ) 
  size_t wsize = mbstowcs(wcs, contents, size);
  timer.stop();
  cout << "done in " << timer << " (" << commaStr(wsize) << " characters)" << endl << endl;

  // CONVERT USING ORDINARY TOLOWER
  cout << "converting using ordinary tolower ... " << flush;
  timer.start();
  {
    char* p = contents;
    while (*p != 0) { *p = tolower(*p); ++p; }
  }
  timer.stop();
  cout << "done in " << timer << endl << endl;

  showStringUpToLength(contents, MAX_SHOW);
  cout << endl << endl;

  // CONVERT USING MY UTF8 TOLOWER
  cout << "converting using my utf8_tolower ... " << flush;
  timer.start();
  {
    char* p = contents_copy;
    try 
    {
      while (*p != 0) { p += __utf8_tolower(p); }
    }
    catch (exception &e)
    {
      cerr << "\n\nEXCEPTION CAUGHT: " << e.what() << "\n\n";
      showStringUpToLength(p, MAX_SHOW);
      exit(1);
    }
  }
  timer.stop();
  cout << "done in " << timer << endl << endl;

  showStringUpToLength(contents_copy, MAX_SHOW);
  cout << endl << endl;

  // CONVERT USING TOWLOWER
  cout << "converting wchar string using towlower ... " << flush;
  timer.start();
  {
    wchar_t* p = wcs;
    while (*p != 0) { *p = towlower(*p); ++p; }
  }
  timer.stop();
  cout << "done in " << timer << endl << endl;

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
//   NOTE: inline makes it about 10% faster (so not really worth it)
//
//   NOTE: advancing p inside of the function (and making it void) does
//   not help efficiency (and inline does not make a difference then)
//
inline int __utf8_tolower(char* p)
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
  if (len == 1 || len < 0)
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
    


//! PRINT STRING UP TO GIVEN LENGTH
void showStringUpToLength(char* s, unsigned int max)
{
  char* p = s;
  while (*p != 0 && p < s + max) 
  {
    cout << (isspace(*p) || (*p >= ' ' && *p <= '~') || *p >= '¢' ? *p : '·');
    p++;
  }
  if (*p != 0) cout << "...";
}
  

#if (0)

  cout << "Here is a utf-8 string: "; 
  char* s = "EselsbrÃ¼cke";
  cout << s << endl;
  
  cout << "Here it is converted using toupper: ";
  string s1 = s;
  for (unsigned int i = 0; i < s1.size(); ++i)
    s1[i] = toupper(s1[i]);
  cout << s1 << endl;

  cout << "Here it is converted using tolower: ";
  string s2 = s;
  for (unsigned int i = 0; i < s2.size(); ++i)
    s2[i] = tolower(s2[i]);
  cout << s2 << endl;

  cout << "Here it is converted using mbsrtowcs and towlower : ";
  string s1 = s;
  for (unsigned int i = 0; i < s1.size(); ++i)
    s1[i] = toupper(s1[i]);
}

#endif
