#include <iostream>
#include <string>
#include "Globals.h"
#include "WordsFile.h"
#include "Timer.h"
#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;

hash_map<string, unsigned int, StringHashFunction> synonyms;

//
// USAGE
//
void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: enhanceExcerptsBySynonyms <old docs file> <new docs file> <synonym clusters file>" << EMPH_OFF << endl
       << endl 
       << "In all the excerpts, replace <word> in cluster <id> by ^^sx<id>x^<word>" << endl
       << endl;
}

char IDX_TAG = '';
bool removeIdxTags = true;

// 
// READ SYNONYMS 
//
void readSynonyms(string& synonymsFileName, hash_map<string, unsigned int, StringHashFunction>& synonyms)
{
  FILE* file = fopen(synonymsFileName.c_str(), "r");
  if (file == NULL) { perror(synonymsFileName.c_str()); exit(1); }
  const unsigned int MAX_WORD_LEN = 1000;
  char buf[MAX_WORD_LEN+1];
  string word;
  unsigned int id;
  int ret;
  while (!feof(file))
  {
    buf[MAX_WORD_LEN] = 0;
    ret = fscanf(file, "%1000s\t%d\n", buf, &id);
    if (ret != 2) { cerr << " [skipping line with wrong format in synonyms file] " << flush; continue; }
    word = buf;
    synonyms[word] = id;
  }
}


//
// MAIN
//
int main(int argc, char** argv)
{
  cout << endl << "ENHANCE EXCERPTS BY SYNONYMS (" << VERSION << ")" << endl << endl;

  if (argc != 4) { printUsage(); exit(1); }
  string oldDocsFileName = argv[1];
  string newDocsFileName = argv[2];
  string synonymsFileName = argv[3];
  string logFileName = string(argv[0]) + ".LOG";

  // log file
  FILE* log_file = fopen(logFileName.c_str(), "w");
  if (log_file == NULL) { perror(logFileName.c_str()); exit(1); }

  // docs files
  FILE* old_docs_file = fopen(oldDocsFileName.c_str(), "r");
  if (old_docs_file == NULL) { perror(oldDocsFileName.c_str()); exit(1); }
  FILE* new_docs_file = fopen(newDocsFileName.c_str(), "w");
  if (new_docs_file == NULL) { perror(newDocsFileName.c_str()); exit(1); }
  const unsigned int MAX_DOC_SIZE = 100*1000*1000;
  char* doc_buf = (char*)(calloc(MAX_DOC_SIZE + 2, 1));
  doc_buf[MAX_DOC_SIZE-1] = '\n';
  doc_buf[MAX_DOC_SIZE] = IDX_TAG;
  char* doc_buf_end = doc_buf;
  char* d_0 = doc_buf;
  char* d;

  // synonyms file
  cout << "reading synonyms from \"" << synonymsFileName << "\" ... " << flush;
  readSynonyms(synonymsFileName, synonyms);
  cout << "done (" << synonyms.size() << " words)" << endl << endl;
  
  string word;
  while (true)
  {
    // read line from docs file
    char* ret = fgets(doc_buf, MAX_DOC_SIZE, old_docs_file);
    if (ret == NULL || feof(old_docs_file)) break;
    assert(doc_buf[MAX_DOC_SIZE-1] == '\n');
    doc_buf_end = doc_buf;
    while (*doc_buf_end != '\n') doc_buf_end++;
    doc_buf_end++;
      // d_0 = doc_buf;
      //DocId docid = atoi(d_0);

    // move forward to excerpt
    d = d_0 = doc_buf;
    for (unsigned int i = 1; i <= 3; i++)
    {
      while (*d  != '\t') d++;
      d++;
    }
    assert(d < doc_buf_end);
    fwrite(d_0, d - d_0, 1, new_docs_file);

    // process excerpt
    while (*d != '\n')
    {
      // skip and write non-word characters
      d_0 = d;
      while (!isalnum(*d) && *d != '\n') d++;
      fwrite(d_0, d - d_0, 1, new_docs_file);
      if (*d == '\n') break; 

      // find next word
      d_0 = d;
      while (isalnum(*d)) d++;
      word.assign(d_0, d - d_0);
      if (synonyms.count(word) > 0)
        fprintf(new_docs_file, "^^sx%dx^", synonyms[word]);
      fwrite(d_0, d - d_0, 1, new_docs_file);
    }
    fwrite(d, 1, 1, new_docs_file);

  }

}
