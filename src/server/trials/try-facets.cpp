#include "Globals.h"
#include "HYBCompleter.h"
#include "CompletionServer.h"

void printUsage() { cout << "Usage: test-facets <db-name>" << endl << endl; }
   
//! MAIN 
int main(char argc, char** argv)
{
  cout << endl << EMPH_ON << "TEST DBLP FACETS (" << VERSION << ")" << EMPH_OFF << endl << endl;

  if (optind >= argc) { printUsage(); exit(1); }
  string baseName = argv[optind++];
  string indexFileName = baseName + ".hybrid";
  string excerptsDBFileName = baseName + ".docs.db";
  string vocabularyFileName = baseName + ".vocabulary";
  string logFileName = baseName + ".log";
  string prefix = optind < argc ? argv[optind++] : "b";
  Timer timer;

  const int MODE = WITH_SCORES + WITH_POS + WITH_DUPS;
  HYBIndex index(indexFileName, vocabularyFileName, MODE);
  History history;
  HYBCompleter<MODE> completer(index, history);
  cout << endl;
  completer._metaInfo.show();
  cout << endl;

  // find all authors
  Query query("ct:author:*");
  QueryResult* result = NULL; 
  bool filterResults = true; // if true, use filtering from history wherever possible
  bool useLinearWordlistIntersection = false; // if true -> use linear intersection, else use "hash" intersection
  DocId k1_docs = 0;
  WordId k2_words = INT_MAX;
  try {
    completer.topCompletionsAndHitsForQuery(query, result, k1_docs, k2_words, filterResults, useLinearWordlistIntersection);
  } catch(Exception e) {
    cerr << "ERROR: " << e.errorMessage() << endl; exit(1);
  }
  cerr << endl;

  // for each author, get & output the facets
  Completions& completions = result->_topCompletions;
  k2_words = 5;
  for (unsigned int i = 0; i < completions.size(); ++i)
  {
    string& author = completions[i].first;
    unsigned int pos = author.rfind(':');
    if (pos == string::npos) { cerr << "! WARNING: missing ':' in author word (" << author << ")" << endl; continue; }
    string name = author.substr(pos + 1);
    for (unsigned int j = 0; j < name.size(); ++j) if (name[j] == '_') name[j] = ' ';
    author.resize(pos);
    Query query(author + ":* ct:conference:*");
      //cout << "[" << query << "]" << endl;
    QueryResult* result = NULL; 
    completer.topCompletionsAndHitsForQuery(query, result, k1_docs, k2_words, filterResults, useLinearWordlistIntersection);
    Completions& completions = result->_topCompletions;
    printf("%d. \"%s\": ", i+1, name.c_str());
    for (unsigned int j = 0; j < completions.size(); ++j)
      cout << completions[j].first << (j < completions.size() - 1 ? ", " : "\n");
  }
  cout << endl;

} // end of startServer
