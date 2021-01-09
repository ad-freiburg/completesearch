#include "Globals.h"
#include "Timer.h"
#include <zlib.h>
 
using namespace std;

void printUsage() 
{ 
  cout << "Usage: test-zipped-excerpts <gzipped docs file>" 
       << endl << endl; 
}

int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "TEST READING FROM ZIPPED EXCERPT"
       << " (VERSION " << VERSION << ")" << EMPH_OFF << endl << endl;
  Timer timer;

  if (argc <= 1) { printUsage(); exit(1); }
  char* filename = argv[1];
  unsigned int R = argc > 2 ? atoi(argv[2]) : 10;

  // open gzipped file
  cout << "opening gzipped file \"" << filename << "\" ... " << flush;
  gzFile file = gzopen(filename, "r");
  if (file == NULL) 
  { 
    cout << endl << "! ERROR opening file \"" << fileName 
         << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); 
  }
  struct stat* buf;
  int ret = stat(file_name, buf);
  assert(ret == 0);
  unsigned int size = buf.st_size;
  cout << "done, size is " << commaStr(size) << " bytes" << endl;

  for (unsigned int r = 1; r <= R; r++)
  {
    // seek random line
    z_off_t offset = (z_off_t)(drand48()*size);
    while (true)
    {
      z_off_t ret = gzseek(file, offset, SEEK_SET);
      char c;
      int ret == 
    while (true)




  assert(words.size() == 0);
  cerr << "reading " << what << " from file \"" << fileName << "\" ... " << flush;
  char line[MAX_WORD_LENGTH+1];
  for(unsigned int i=0; i<MAX_WORD_LENGTH+1;++i) {line[i] = 0;}

  while (gzgets(file, line, MAX_WORD_LENGTH) != Z_NULL)
  {
    int len = strlen(line);
    assert(len < MAX_WORD_LENGTH);
    if (len == MAX_WORD_LENGTH - 1) { cout << endl << "! ERROR reading from file \"" << fileName
                                           << "\" (line too long)" << endl << endl; exit(1); }
    while (len > 0 && iswspace(line[len-1])) line[--len] = 0; // remove trailing whitespace (ip newline)
    if (len > 0) words.push_back(line); /* ignore empty lines */
  }

  timer.stop();
  cerr << "done in " << timer << " (" << commaStr(words.size()) << " words)" << endl;
  gzclose(file);
  assert(words.size() > 0);
}



  cout << "WITHOUT COMPRESSION" << endl << endl;
  string fileName1 = argv[1];
  vector<string> words1;
  readWords(fileName1, words1, "words");
  cout << endl;

  cout << "WITH COMPRESSION" << endl << endl;
  string fileName2 = string(argv[1]) + string(".gz");
  vector<string> words2;
  readWordsGzipped(fileName2, words2, "words");
  cout << endl;

  cout << "EQUALITY CHECK" << endl << endl;
  cout << "the two vectors constructed are ... " << flush;
  bool same = true;
  if (words1.size() != words2.size()) same = false;
  else for (unsigned int i = 0; i < words1.size(); ++i)
         if (words1[i] != words2[i]) { same = false; break; }
  cout << (same ? "EQUAL" : "DIFFERENT") << endl << endl;
}

void readWords(const string fileName, vector<string>& words, string what)
{
  Timer timer;
  timer.start();
  FILE* file = fopen(fileName.c_str(), "r");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << fileName 
                           << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  assert(words.size() == 0);
  cerr << "reading " << what << " from file \"" << fileName << "\" ... " << flush;
  char line[MAX_WORD_LENGTH+1];
  for(unsigned int i=0; i<MAX_WORD_LENGTH+1;++i) {line[i] = 0;}

  while (fgets(line, MAX_WORD_LENGTH, file) != NULL)
    {
      int len = strlen(line);
      assert(len < MAX_WORD_LENGTH);
      if (len == MAX_WORD_LENGTH - 1) { cout << endl << "! ERROR reading from file \"" << fileName
                                             << "\" (line too long)" << endl << endl; exit(1); }
      while (len > 0 && iswspace(line[len-1])) line[--len] = 0; // remove trailing whitespace (ip newline)
      if (len > 0) words.push_back(line); /* ignore empty lines */
    }

  timer.stop();
  cerr << "done in " << timer << " (" << commaStr(words.size()) << " words)" << endl;
  fclose(file);
  assert(words.size() > 0);
}



void readWordsGzipped(const string fileName, vector<string>& words, string what)
{
  Timer timer;
  timer.start();
  gzFile file = gzopen(fileName.c_str(), "r");
  if (file == NULL) { cout << endl << "! ERROR opening file \"" << fileName 
                           << "\" (" << strerror(errno) << ")" << endl << endl; exit(1); }
  assert(words.size() == 0);
  cerr << "reading " << what << " from file \"" << fileName << "\" ... " << flush;
  char line[MAX_WORD_LENGTH+1];
  for(unsigned int i=0; i<MAX_WORD_LENGTH+1;++i) {line[i] = 0;}

  while (gzgets(file, line, MAX_WORD_LENGTH) != Z_NULL)
  {
    int len = strlen(line);
    assert(len < MAX_WORD_LENGTH);
    if (len == MAX_WORD_LENGTH - 1) { cout << endl << "! ERROR reading from file \"" << fileName
                                           << "\" (line too long)" << endl << endl; exit(1); }
    while (len > 0 && iswspace(line[len-1])) line[--len] = 0; // remove trailing whitespace (ip newline)
    if (len > 0) words.push_back(line); /* ignore empty lines */
  }

  timer.stop();
  cerr << "done in " << timer << " (" << commaStr(words.size()) << " words)" << endl;
  gzclose(file);
  assert(words.size() > 0);
}
