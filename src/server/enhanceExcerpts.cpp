#include <iostream>
#include <string>
#include "Globals.h"
#include "WordsFile.h"
#include "Timer.h"

using namespace std;

//
// USAGE
//
void printUsage()
{
  cout << endl
       << EMPH_ON << "Usage: enhanceExcerpts <old docs file> <new docs file> <enhancing words file>" << EMPH_OFF << endl
       << endl;
}

char IDX_TAG = '';
bool removeIdxTags = true;

//
// MAIN
//
int main(int argc, char** argv)
{
  if (argc != 4) { printUsage(); exit(1); }
  string oldDocsFileName = argv[1];
  string newDocsFileName = argv[2];
  string wordsFileName = argv[3];
  string logFileName = string(argv[0]) + ".LOG";

  // log file
  FILE* log_file = fopen(logFileName.c_str(), "w");
  if (log_file == NULL) { perror(logFileName.c_str()); exit(1); }

  // docs files
  FILE* old_docs_file = fopen(oldDocsFileName.c_str(), "r");
  if (old_docs_file == NULL) { perror(oldDocsFileName.c_str()); exit(1); }
  FILE* new_docs_file = fopen(newDocsFileName.c_str(), "w");
  if (new_docs_file == NULL) { perror(newDocsFileName.c_str()); exit(1); }
  const unsigned int MAX_DOC_SIZE = 10*1000*1000;
  char* doc_buf = (char*)(calloc(MAX_DOC_SIZE + 2, 1));
  doc_buf[MAX_DOC_SIZE-1] = '\n';
  doc_buf[MAX_DOC_SIZE] = IDX_TAG;
  char* doc_buf_end = doc_buf;
  char* d_0 = doc_buf;
  char* d;
  DocId docsDocId = 0;
  DocId docsPos = 0;

  // words file
  WordsFile wordsFile(wordsFileName);
  string word;
  DocId wordsDocId;
  DiskScore wordsScore; 
  Position wordsPos; 

  while (true)
  {
    // read next enhancing word 
    bool success = wordsFile.getNextLine(word, wordsDocId, wordsScore, wordsPos);
    if (!success && !wordsFile.isEof()) continue;
    
    // discarding words at position 0 !!!
    if (wordsPos == 0) continue;

    // move forward to right document
    while (docsDocId < wordsDocId || wordsFile.isEof()) 
    {
      if (removeIdxTags)
        for (char* p = d_0; p < doc_buf_end; p++) if (*p == IDX_TAG) *p = ' ';
      fwrite(d_0, doc_buf_end - d_0, 1, new_docs_file);
      char* ret = fgets(doc_buf, MAX_DOC_SIZE, old_docs_file);
      if (ret == NULL) break;
      assert(doc_buf[MAX_DOC_SIZE-1] == '\n');
      doc_buf_end = doc_buf;
      while (*doc_buf_end != '\n') doc_buf_end++;
      doc_buf_end++;
      d_0 = doc_buf;
      DocId tmp = atoi(d_0);
      assert(tmp > docsDocId);
      docsDocId = tmp;
      docsPos = 0;
    }
    if (feof(old_docs_file)) break;
    if (docsDocId != wordsDocId) { printf("\nERROR: could not find doc with id %u, found only %u\n\n", wordsDocId, docsDocId); exit(1); }
    assert(d_0 != NULL);

    // move forward to indexed word matching current enhancing word
    d = d_0;
    if (docsPos >= wordsPos) { printf ("\nERROR: looks like positions are not in strictly increasing order in doc with id #%u (%u >= %u)\n\n", docsDocId, docsPos, wordsPos); }
    while (docsPos < wordsPos)
    {
      do { d++; } while (*d != IDX_TAG);
      if (d >= doc_buf_end) { printf("\nWARNING: could not find indexed word #%u in doc with id %u (has only %u)\n\n", wordsPos, docsDocId, docsPos); break; }
      //if (d >= doc_buf_end) { printf("\nERROR: could not find indexed word #%d in doc with id %d (has only %d)\n\n", wordsPos, docsDocId, docsPos); exit(1); }
      docsPos++;
    }
    if (d >= doc_buf_end) { d--; continue; }

    // report match
    assert(*d == IDX_TAG);
    const char* d_1 = d; do { d_1--; } while (isalnum(*d_1) || *d_1 == '^'); d_1++;
    if (removeIdxTags)
      for (char* p = d_0; p < d_1; p++) if (*p == IDX_TAG) *p = ' ';
    fwrite(d_0, d_1 - d_0, 1, new_docs_file);
    if (d_1 < d) fprintf(new_docs_file, "^^%s^", word.c_str());
    fwrite(d_1, d - d_1, 1, new_docs_file);
      //fprintf(new_docs_file, "[MATCH: %s <-> %s]", word.c_str(), d_1);
    d_0 = d;
  }

}
