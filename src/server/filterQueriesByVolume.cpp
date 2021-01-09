#include <iostream>
#include <fstream>
#include <getopt.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "INVCompleter.h"

using namespace std;

void printUsage() 
{
  cout << EMPH_ON << "Usage: filterQueriesByVolume inverted-index-file query-file volume-treshold" << EMPH_OFF << endl
       << endl;
}

bool isPositionalIndex = false;

off_t volumeOfPrefix(INVCompleter<0>& completer, string& prefix)
{
  File& indexFile = completer.getIndexStructureFile();
  vector<off_t>& byteOffsets = completer._byteOffsetsForDoclists;
  WordRange wordRange = completer.prefixToRange(prefix);
  off_t volume = 0;
  unsigned long nofDocs;
  off_t offset;
  off_t dummy[2];
  for (WordId i = wordRange.firstElement(); i <= wordRange.lastElement(); ++i)
  {
    offset = byteOffsets[i]; 
    if (isPositionalIndex) { indexFile.read(&dummy, 2*sizeof(off_t), offset); offset += 2*sizeof(off_t); }
    indexFile.read(&nofDocs, sizeof(unsigned long), offset);
    //cout << completer.getWordFromVocabulary(i) << " : " << nofDocs << endl;
    volume += nofDocs;
  }
  //cout << "--> volume of \"" << prefix << "\" : " << volume << endl;
  return volume;
}

//
// MAIN : parse command line argument and call answerQueries
//
int main(int argc, char *argv[])
{
  cerr << endl << EMPH_ON << "FILTER QUERIES BY VOLUME" << EMPH_OFF << endl << endl;
 
  //
  // PROCESS COMMAND LINE ARGUMENTS (gloabal variables declared after printUsage() above)
  //
  while (true)
  {
    char c = getopt(argc, argv, "l");
    if (c == -1) break;
    switch (c)
    {
      case 'l':
        isPositionalIndex = true;
        break;
      default:
        cout << endl << "! ERROR in processing options (getopt returned '" << c << "' = 0x" << setbase(16) << int(c) << ")" << endl << endl;
        exit(1);
    }
  }
  if (argc - optind != 3) { printUsage(); exit(1); }
  string indexFileName = argv[optind];
  string vocFileName = indexFileName;
  vocFileName = vocFileName.erase(vocFileName.rfind('.')) + ".vocabulary";
  string queriesFileName = argv[optind+1];
  unsigned int volumeTreshold = atoi(argv[optind+2]);

  INVCompleter<0> completer(indexFileName, vocFileName);

  //
  // READ QUERIES FROM FILE
  //
  vector<string> queries;
  FILE* queries_file = fopen(queriesFileName.c_str(),"r");
  char query_buf[1000];
  unsigned int len;
  while (true)
  {
    fgets(query_buf,1000,queries_file);
    if (feof(queries_file)) break;
    len = strlen(query_buf);
    while (iswspace(query_buf[len-1])) query_buf[--len] = 0;
    queries.push_back(query_buf);
  }

  //
  // PROCESS
  //
  string query;
  string prefix;
  for (unsigned int i = 0; i < queries.size(); ++i)
    for (unsigned int j = 0; j < queries[i].length(); ++j)
    {
      if (queries[i][j] == ' ') continue;
      query = queries[i].substr(0,j+1);
      prefix = query;
      if (prefix.rfind(' ') != string::npos) prefix.erase(0,prefix.rfind(' ')+1);
      //cout << "\"" << prefix << "\"" << endl;
      if (volumeOfPrefix(completer, prefix) > volumeTreshold)
      {
        cerr << "! prefix " << setw(10) << left << ( "\"" + prefix + "\"" ) << " of query "
             << setw(40) << left << ( "\"" + query + "\"" ) << " has volume larger than " << volumeTreshold << endl;
      }
      else
      {
        cout << query << endl;
      }
      //cout << "\"" << queries[i].substr(0,j+1) << "\"" << endl;
    }

}


