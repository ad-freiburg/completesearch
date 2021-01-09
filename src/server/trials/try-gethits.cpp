#include "HYBCompleter.h"

using namespace std;

int main(int argc, char** argv)
{
  

  cout << endl << "Initializing completion server" << endl << endl;
  string indexFileName = "/KM/ir/autocomplete/databases/papers/papers+.hybrid";
  string vocabularyFileName = "/KM/ir/autocomplete/databases/papers/papers+.vocabulary";
  
  while (true)
  {
        
       
        
      char c = getopt(argc, argv, "i:v:"); /* mit : = mit Paramter, ohne : = ohne */
      if (c == -1) break;
      switch (c)
      {
            
          case 'i': indexFileName=optarg;break;
          case 'v': vocabularyFileName=optarg;break;
           
      //case 'b': b=true; break;
          default: cout << "ERROR! Argument:" << c << endl << endl; exit(1);
      }
  }
  if (optind != argc-2) { cerr << "Usage: test-gethits [-i index] [-v vocabulary] <query> <k>" << endl; exit(1); }
    
   
  Query query = string(argv[optind++]);
  unsigned int k = atoi(argv[optind++]);

  cout << endl << "DUMMY: Initializing completion server" << endl << endl;
    
  cout << "Options : " <<endl;
  cout << "Index : " <<indexFileName<<endl;
  cout << "Vocabulary : " <<vocabularyFileName<<endl;
  cout << "Query : " << query << endl;
  cout << "k : " << k <<endl;
  
  HYBCompleter<WITH_SCORES+WITH_POS+WITH_DUPS> completer(indexFileName, vocabularyFileName);

  QueryResult* result = NULL;
  completer.topCompletionsAndHitsForQuery(query, result, k);

  cout << endl << "Top " << k << " hits for query \"" << query << "\" (document ids with scores):" << endl << endl;
  assert(result->_topDocIds.size() == results->_topDocScores.size());

  cout << "number \t DocID \t Score"<<endl;
  cout << "------------"<<endl;
  for (unsigned int i = 0; i < result->_topDocIds.size(); ++i)
    cout << i+1 << ".\t" <<  result->_topDocIds[i] << "\t" << result->_topDocScores[i] << "" << endl;
cout << "------------"<<endl;
  if (result->_topDocIds.size() > 0) cout << endl;
  if (result->_topDocIds.size() < k) cout << "[less than " << k << " hits]" << endl << endl;
}

