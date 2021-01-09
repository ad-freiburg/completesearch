/*
 * Type 'CompleteServer' to see the usage of this server.
 *
 * If the server has been successfully started, a file $HOME/.CompleteServer:portnumber.pid is written.
 * This file is needed when the -k option is given.
 */

#include "HYBCompleter.h"
#include "INVCompleter.h"
#include "server.h"
#include "Timer.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <csignal>
#include "ExcerptsGenerator.h"
#include <clocale>
#include <sys/types.h>
#include <regex.h>
////#include "SynonymsDB.h"

using namespace std;
//using namespace aucmpl;

static char* VERSION = "06Apr06";

extern ofstream* logfile;
FILE* log_file;
// Scheiﬂ C++!! Hat nicht mal ne Funktion um aus einer Zahl einen String zu machen, oder?
template<typename T> inline string stringify(const T& x) { ostringstream s; s << x; return s.str(); } 

int port = 0;
bool killserver = false;
bool portname_given = false;
bool excerptsdb_given = false;
char excerptsdb[100];
char synonymsDB_filename[100] = "";
char portname[20];
char conffilename[100];
char indexdb[100];
char logfilename[100];
unsigned long cache_size = 16*1024*1024; // cache size of excerpts db in bytes
string vocabularyfile;
string completertype; // will be "INV" or "HYB"
size_t history_size = 32*1024*1024; // max history size in bytes

#include "QueryParameters.h"
 
void PrintUsage()
{
  cout << endl
       << "Usage: CompleteServer [options] index_db_file\n\n"
       << "  options: -p portnumber        The portnumber to listen at\n"
       << "           -n portname          A name specified in a config file such as ports.txt\n"
       << "           -f configfile        The name of a config file. Default is /opt/www/conf/ports.txt\n"
       << "           -l logfile           File name for log messages. By default logging is off\n"
       << "           -k portnumber        Stop the server\n"
       << "           -e exerpts_db_file   The name of an excerpts.db file\n"
    ////       << "           -s synonyms_db_file  The name of an synonyms.db file\n"
       << "           -c cache_size        Sets the cache size for the excerpts generator (default: 16 megabytes)\n\n"
       << "           -h history_size        Sets the history size (default: 32 megabytes)\n\n"
       << "  Cache sizes must be greater than 0 and are given in one of the forms\n"
       << "    *  -c n   meaning n bytes,\n"
       << "    *  -c nK  meaning n kilobytes or\n"
       << "    *  -c nM  meaning n megabytes\n\n"
       << "  If -k is given, then all other options have no effect and index_db_file may be omitted.\n"
       << "  Otherwise, either -p or -n must be specified, but not both of them.\n\n";
}



// Sets the history size according to the -h argument. Returns true iff the form of that argument is correct
bool setHistorySize(const char* n)
{
  unsigned long factor = 1;
  if (strlen(n) < 1) return false;
   
  char* s = new char[strlen(n)+1];
  strcpy(s, n);
  char* c = s+strlen(s)-1;
  switch (*c)
  {
    case 'k':
    case 'K':
      factor = 1024;
      *c = '\0';
      break;
    case 'm':
    case 'M':
      factor = 1024*1024;
      *c = '\0';
      break;
    default:
      if (*c<'0' || *c>'9')
      {
        delete[] s;
        return false;
      }
  }
  history_size = factor * atoi(s);
  delete[] s;

  if (history_size == 0) return false;
    else return true;
}


// Sets the cache size according to the -c argument. Returns true iff the form of that argument is correct
bool setCacheSize(const char* n)
{
  unsigned long factor = 1;
  if (strlen(n) < 1) return false;
   
  char* s = new char[strlen(n)+1];
  strcpy(s, n);
  char* c = s+strlen(s)-1;
  switch (*c)
  {
    case 'k':
    case 'K':
      factor = 1024;
      *c = '\0';
      break;
    case 'm':
    case 'M':
      factor = 1024*1024;
      *c = '\0';
      break;
    default:
      if (*c<'0' || *c>'9')
      {
        delete[] s;
        return false;
      }
  }
  cache_size = factor * atoi(s);
  delete[] s;

  if (cache_size == 0) return false;
    else return true;
}


// Checks the arguments given in the command line. Returns false if program should stop and true otherwise
bool CheckArgs(char argc, char** argv)
{
  if (argc < 2) return false;

  int i = 1;
  
  strcpy(conffilename, "/opt/www/conf/ports.txt");
  strcpy(logfilename, "/dev/null");
  while (i < argc-1)
  {
    if (!strcmp(argv[i], "-k"))
    {
      killserver = true;
      port = atoi(argv[i+1]);
      return true;
    }
    if (!strcmp(argv[i], "-p"))
    {
      if (i >= argc-2) return false;
      port = atoi(argv[i+1]);
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-n"))
    {
      if (i >= argc-2) return false;
      strcpy(portname, argv[i+1]);
      portname_given = true;
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-f"))
    {
      if (i >= argc-2) return false;
      strcpy(conffilename, argv[i+1]);
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-l"))
    {
      if (i >= argc-2) return false;
      strcpy(logfilename, argv[i+1]);
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-e"))
    {
      if (i >= argc-2) return false;
      strcpy(excerptsdb, argv[i+1]);
      excerptsdb_given = true;
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-s"))
    {
      if (i >= argc-2) return false;
      strcpy(synonymsDB_filename, argv[i+1]);
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-c"))
    {
      if (i >= argc-2) return false;
      if (!setCacheSize(argv[i+1])) return false;
      i += 2;
      continue;
    }
    if (!strcmp(argv[i], "-h"))
    {
      if (i >= argc-2) return false;
      if (!setHistorySize(argv[i+1])) return false;
      i += 2;
      continue;
    }
    return false;
  }
  if (!(port || portname_given) || (port && portname_given)) return false;
  
  strcpy(indexdb, argv[argc-1]);

  string::size_type pos;
  // vocabularyfile
  if((pos = string(indexdb).find(".inverted",0)) != string::npos)
    {
      vocabularyfile = string(indexdb).substr(0,pos) + string(".vocabulary");
      completertype = "INV";
    }
  else if((pos = string(indexdb).find(".hybrid",0)) != string::npos)
    {
      vocabularyfile = string(indexdb).substr(0,pos) + string(".vocabulary");
      completertype = "HYB";
    }
  else
    {
      cout << " File of index structure did not end in 'inverted' or 'hybrid'!" << endl
           << " Could not automatically derive the name of the vocabulary file" << endl
           << " and the type of completer (INV/HYB)! "<< endl;
      exit(1);
    }
  
  return true;
}


void GetPortFromFile(const char* filename)
{
  ifstream config(filename);
  if (!config.is_open())
  {
    cout << "Could not open file " << filename << endl;
    exit(0);
  }

  char line[500];
  char name[500];
  int number;
  int linenumber = 0;
  while (!config.eof())
  {
    config.getline(line, 500);
    linenumber++;
    if (strlen(line) > 0)
    {
      if (sscanf(line,"%s %d", name, &number) != 2)
      {
        cout << "File " << filename << " has wrong format (line number " << linenumber << ").\n";
        exit(0);
      }
      if (!strcmp(name, portname))
      {
        port = number;
        return;
      }
    }
  }
  cout << "Portname " << portname << " not found in file " << filename << endl;
  exit(0);
}


void MakeQueryWellFormed(Query& query)
{
  // Remove parantheses and newlines
  string::iterator it = query.begin();
  while (it != query.end())
    if (*it=='(' || *it==')' || *it=='[' || *it==']' || *it=='\n') it = query.erase(it);
    else it++;

    // Chop off trailing space characters
    if (!query.empty())
      while (query[query.size()-1] == ' ') query = query.substr(0,query.size()-1);

    // Implicitly assume * after last word in query
    // CHANGE BY HOLGER 14Jan05: don't do that, let it be done by search.js
    //if (!Query.empty() && Query[Query.size()-1] != '*') Query += '*';
}


void KillServer(int port)
{
  char* pidfilename = new char[100];
  sprintf(pidfilename, "%s/.CompleteServer:%d.pid",getenv("HOME"),port);
  ifstream pidfile(pidfilename);

  if (!pidfile.is_open())
  {
    cerr << "Could not read the .pid-file " << pidfilename << ". Exiting.\n";
    exit(0);
  }

  pid_t pid;
  pidfile >> pid;
  if (kill(pid, SIGTERM))
  {
    perror("CompleteServer");
    exit(0);
  }
  else
  {
    cout << "The server has been shut down" << endl;
    char* cmd = new char[100];
    sprintf(cmd, "rm %s", pidfilename);
    system(cmd);
    delete[] cmd;
  }
  
  delete[] pidfilename;
}


template<class Completer>
void DBCompleteServer(ExcerptsGenerator* exGen)
{

  Timer timer;

  cout << endl 
       << "--- RUNNING COMPLETION SERVER  VERSION " << VERSION
       << endl
       << endl; 
  cout << " * opening completer database/datastructure \"" << indexdb << "\"" << endl;
  timer.start();  

  unsigned long int initIntBuffSize = INTERSECTION_BUFFER_INIT;
  unsigned long int initUniBuffSize = UNION_BUFFER_INIT;

  Completer completer(indexdb,vocabularyfile, initIntBuffSize, initUniBuffSize);

  timer.stop();
    //cout << " done (" << setprecision(3) << timer.value()/1000000.0 << " seconds)" << endl

  completer._metaInfo.show();



  //Ingmar: 04Apr06 uncommented for compilation
  /*
  SynonymsDB *synonymsDB = NULL;
  if (*synonymsDB_filename)
  {
    cout << " * opening synonyms database \"" << synonymsDB_filename << "\"" << endl;
    // open synonyms DB with 1 MB cache
    synonymsDB = new SynonymsDB(synonymsDB_filename, 1024);
  }
  */

  cout << " * starting server at port \"" << port << "\"" << endl;
  cout << endl;

  Connector socket(port);


  QueryResult* result=NULL;
  vector<HitData> hits;
  char* query_string_raw;
  char* query_string = new char[501];
  for(int i=0;i<501;i++){query_string[i] = 0;}
  int query_string_length;
  bool bad_query_string;
  char* msg;
  socket.init(logfilename);

  // cerr << " ! JUST TESTING CERR (if you see this, it's fine)" << endl;
  // unsigned int x,y; x = y = 2; cout << 1/(x-y); 

  pid_t pid = getpid();
  char* pidfilename = new char[100];
  sprintf(pidfilename, "%s/.CompleteServer:%d.pid",getenv("HOME"),port);
  ofstream pidfile(pidfilename);
  if (!pidfile.is_open())
  {
    cerr << endl << "ERROR: Could not write .pid-file. Exiting." << endl;
    exit(0);
  }
  pidfile << pid;
  pidfile.close();
  delete[] pidfilename;
  
  if (!logfile->is_open()) 
  {
    cerr << endl << "Could not open logfile. Switching off logging." << endl;
  }

  //  ofstream historyfile("history.txt");

  cout << "\n* Waiting for first query .... " << endl;

  //
  // WAIT FOR REQUESTS (on port) AND ANSWER THEM
  //
  while (true)
  {
    //
    // WAIT FOR NEW QUERY
    //
    socket.Accept();
    time_t NOW = time(NULL);
    if (logfile->is_open()) 
    {  
      *logfile << endl << "--- NEW QUERY  " << ctime(&NOW);
    }


    query_string_raw = " ";
    // 
    // GET QUERY STRING
    //
    bad_query_string = false;
    query_string_length = 0;
    query_string_length = socket.Read(&query_string_raw, 500);
    for(int i=0;i<query_string_length; i++) {query_string[i] = query_string_raw[i];}

    // to assure that query_string[0] is always allocated (I am not sure whether it's really necessary)
    if (query_string_length == 0)
    {
      query_string = " ";
    }
    // null-terminate string, in case for some reason (empty string or veyr long string) it is not
    if (query_string[query_string_length] != 0) // valgrind: ==22018== Conditional jump or move depends on uninitialised value(s)
    {
      query_string[query_string_length] = 0;
    }
    if (logfile->is_open()) 
    {
      *logfile << " * received query string \"" << query_string << "\"" << endl;
    }

    
    //
    // EXTRACT PARAMETERS FROM REQUEST STRING (completely changed by Holger 02Sep05)
    //
    Query query = string(query_string);
    
    QueryParameters queryParameters;
    queryParameters.extractFromQuery(query);
    *logfile << " * parameters are: " << queryParameters << endl;
    *logfile << " * remaining part of query is \"" << query << "\"" << endl;
    

    //
    // BEGIN RESULT MESSAGE that will be send back to client
    //
    ostringstream result_stream;  // start with empty message (no idea how to clear it otherwise)
    result_stream << "<!--BEGIN REPORT DBCompleteServer-->\n";

    // CHANGE BY HOLGER:
    //   * for non-empty query, do the same as before
    //   * for empty query, send info about the index, currently
    //     the number of documents (which is then used by the java script)
    //     THOMAS 31MAY05: Also if the parameter string has the wrong syntax.
    //     HOLGER 22JUL05: that was ugly, empty string now gives error
    
    //
    // FOR EMPTY QUERY, send back some basic information about the collection (currently, the number of documents)
    //
    if (query.empty())
    {
      result_stream << "<div id=\"parameters\"><br>"
                    << "-1<br>" 
                    << completer._metaInfo.getNofDocs() << "<br>"
                    << "0<br>"
                    << "0</div>\n";
      *logfile << " ! EMPTY QUERY: sending back some basic info about the collection" << endl;
    }

    //
    // NON-EMPTY QUERY BEING PROCESSED
    //
    else
    {
      // 
      // REMOVE STRANGE CHARACTERS FROM QUERY AND MAKE SURE IT ENDS WITH A STAR
      //
      MakeQueryWellFormed(query);

      //// 04Apr06 Uncomment part on synonyms for now. 'or' is not yet supported
      /*
      //
      // ADD SYNONYMS TO QUERY if last word ends with a + and synonymsDB exists
      //

      size_t last_tilde_pos = query.rfind('~');
      string wordToExpand = "";

      if (synonymsDB != NULL && (last_tilde_pos == query.length() - 1 || last_tilde_pos == query.length() - 2))
      {
        *logfile << " * Query ends with \'~\' ..." << flush;
        // erase trailing + 
        query.erase(last_tilde_pos, 1);
        // GET LAST WORD 
        // get last part (may be an ORed sequence of words, e.g. hot|warm|tepid)
        Query query_first_part, query_last_part;
        size_t split_pos;
        query.splitAtLastSeparator(query_first_part, query_last_part, split_pos);
        // get last word from possibly ored sequence
        size_t last_or_operator_pos = query_last_part.rfind(query_last_part.orOperator());
        wordToExpand = (last_or_operator_pos == string::npos)
          ? query_last_part
          : query_last_part.substr(last_or_operator_pos + 1, query_last_part.length() - last_or_operator_pos - 1);
        // remove trailing star if one is there
        if (wordToExpand[wordToExpand.length()-1] == '*')
        {
          wordToExpand.erase(wordToExpand.length()-1, 1);
        }
        *logfile << " last word is \"" << wordToExpand << "\" ..." << flush;
        // get synonyms
        vector<string> synonyms = synonymsDB->get_synonyms(wordToExpand);
        // add to query 
        for (vector<string>::iterator it = synonyms.begin(); it != synonyms.end(); it++)
        {
          query += "|" + *it + "*";
        }
        *logfile << " enhanced query to \"" << query << "\"" << endl;
      } // end case: if (synonymsDB != NULL && (last_tilde_pos == query.length() - 1 || last_tilde_pos == query.length() - 2))
      // end commented part for synonyms
      */

      //
      // COMPUTE COMPLETION AND HITS (the main work)
      //
      result = NULL;
      completer.resetTimersAndCounters();
      timer.start();
      completer.completionsForQuery(query, result);
      timer.stop();
      assert(result);
      //      result.show(); // for debugging only

      // SHOW ONE LINE SUMMARY OF RESULTS
      cout << endl;
      cout << EMPH_ON << setw(26) << left << ( string("\"") + query + string("\"") ) << EMPH_OFF
           << " " << EMPH_ON << setw(4) << right << completer.completionsForQueryTimer.msecs() << " millisecs" << EMPH_OFF
           << "; " << setw(10) << commaStr((*result)._topDocIds.size()) << " hits"
           << "; " << setw(4) << commaStr((*result)._topCompletions.size()) << " completions" << flush;
      if (completer.nofBlocksReadFromFile > 0)
      cout << "; " << numberAndNoun(completer.nofBlocksReadFromFile,"block","blocks") 
           << " of volume " << commaStr(completer.doclistVolumeDecompressed/sizeof(DocId)) << " read" << endl;
      else if (completer.resultWasInHistory)
      cout << "; [was in history]" << endl;
      else if (completer.resultWasFilteredFrom != "")
      cout << "; [was filtered]" << endl;
      cout << endl;

      // NICE BREAKDOWN OF RUNNING TIMES (as used in answerQueries.cpp)
      completer.showStatistics(cout, "   ");
      cout << endl;

      if (logfile->is_open())
      {
        *logfile << " * computed " 
                 << (*result)._topCompletions.size() << ((*result)._topCompletions.size() == 1 ? " completion" : " completions")
                 << " in " << (*result)._topDocIds.size() << " documents"
                 << " using " << timer.value()/1000.0 << " milliseconds"
                 << endl;
      }
      
      // ADDED BY THOMAS: the server reports the number of hits and the indices of the first and the last hit
      // CHANGE BY HOLGER: if too many completions, send nof documents as nof hits
      result_stream 
        << "<div id=\"parameters\">"
        // should check for synonyms and else just use the prefix itself
        << (*result)._prefixCompleted << "<br>" 
        // the number of completions that were computed (by completionsFor Query)
        << (*result)._topCompletions.size()  << "<br>" 
        // the number of hits (if too many completions -> total number of documents)
        << (*result)._topDocIds.size() << "<br>"
        << queryParameters.firstHit << "<br>" 
        << int(queryParameters.firstHit + queryParameters.nofHits) - 1
        << "</div>\n";




      // WRITE COMPLETIONS AS HTML
      result_stream << "<div id=\"completions\">";
      //if (completer.maxExceeded()) result_stream << "__TOO_MANY__<br>" << completer.exceededFor() << "<br>";
      unsigned int nof_completions_send = (*result)._topCompletions.size() < queryParameters.nofCompletionsSend
                                            ? (*result)._topCompletions.size() 
                                            : queryParameters.nofCompletionsSend;
      // HOLGER 04Dec05: now sort by frequency (TODO: currently does atoi for every comparison, because of freqs like 12|23|35 !)

      timer.start();
      (*result)._topCompletions.sortByScore(nof_completions_send);
      timer.stop();
      *logfile << " * sorted completions in " << timer.value()/1000.0 << " milliseconds" << endl;

      for (unsigned int i = 0; i < nof_completions_send; i++)
        result_stream << (*result)._topCompletions[i].first << (i < nof_completions_send - 1 ? "<br>" : "");
      // if not all completions send, send another ... (the search.js javascript will detect this)
      if ((*result)._topCompletions.size() > queryParameters.nofCompletionsSend)
        result_stream << "<br>...";
      result_stream << "</div>\n";


      // ADDED BY HOLGER: range check for first_hit and nof_hits
      if (queryParameters.firstHit >= (*result)._topDocIds.size())
        queryParameters.firstHit = (*result)._topDocIds.size() == 0 ? 0 : (*result)._topDocIds.size()-1;
      if (queryParameters.firstHit+queryParameters.nofHits > (*result)._topDocIds.size())
        queryParameters.nofHits = (*result)._topDocIds.size() - queryParameters.firstHit;

      //
      // GETTING EXCERPTS
      //

      if (excerptsdb_given)
      {
        timer.start();
        exGen->setMaxHits(queryParameters.nofExcerptsPerHit);
        exGen->setExcerptRadius(-(queryParameters.excerptRadius), queryParameters.excerptRadius);

        const DocList* docs = &((*result)._topDocIds);

           
        assert((*docs).isSorted(true));
        const DocId first_hit = queryParameters.firstHit;
        const DocId nof_hits = queryParameters.nofHits;


        ExcerptsDB _database;
        
        ExcerptData documentData;         // title, url, and complete text of a single document

        DocId i;
  
                   
        std::vector<std::pair<DocId, Score> > sortedDocs;
        std::pair<DocId, unsigned int> doc_score_pair;
        
        assert((*docs).size() == (*result)._topDocScores.size());
        assert((*result)._topDocScores.isPositive());
        for (i = 0; i < (*docs).size(); i++)
          {            
            doc_score_pair.first = (*docs)[i];
            doc_score_pair.second = (*result)._topDocScores[i];
            sortedDocs.push_back(doc_score_pair);
          }

        EG_ScoreCmp<DocId, Score> score_cmp;
        // HOLGER 03May05: sort -> stable_sort
        std::stable_sort<std::vector<std::pair<DocId, Score> >::iterator,
          EG_ScoreCmp<DocId, Score> > (sortedDocs.begin(), sortedDocs.end(), score_cmp);


        // iterate over the subset of hits to be displayed
        // make sure not to go beyond the last hit!
        hits.clear();  // start with an empty list of hits

        for (i = first_hit; i < EG_MIN(first_hit+nof_hits, sortedDocs.size()); i++)
          {
            cout <<  "i = " << i <<", score = " << sortedDocs[i].second << ", docID = " << sortedDocs[i].first << endl;
            hits.push_back(exGen->hitDataForDocAndQuery(query, sortedDocs[i].first, true));
            hits.back().score = sortedDocs[i].second;

          }        

        timer.stop();


        if (logfile->is_open()) 
        {
          *logfile << " * retrieved "
                   << hits.size() << (hits.size() == 1 ? " hit" : " hits")
                   << " using " << timer.value()/1000.0 << " milliseconds" << endl;
        }

        // 
        // WRITE HITS AS HTML 
        //
        // HOLGER 01Jul05: Inserted nested div, because only the *contents* of the
        // outer div is copied in search.js, and so the class info got lost so far
        result_stream << "<div id=\"hits\"><div class=\"hits\">";
        // horizontal bar telling which part of how many hits
        result_stream << "<h1>";
        if ((*result)._topDocIds.size() == 0)
          result_stream << "No hits";
        else if ((*result)._topDocIds.size() == 1)
          result_stream << "A single hit";
        else if (queryParameters.firstHit == 0 && queryParameters.nofHits >= (*result)._topDocIds.size())
          result_stream << "All " << (*result)._topDocIds.size() << " hits shown";
        else
          result_stream << "Hits " << queryParameters.firstHit+1 << " - " << queryParameters.firstHit+queryParameters.nofHits 
                 << " of " << (*result)._topDocIds.size() << " shown"
                 << " (PageDown/PageUp for next/previous hits)";
        result_stream << "</h1>";

        //
        // generate list of hits (output format depends on the query parameter displayMode
        //
        if (queryParameters.displayMode <= 3)
        {
          // mode 1: show hit by hit, with all excerpts in a single string, and the URL in a seperate line
          // mode 2: like mode 1, but with a line break after each excerpt
          // mode 3: like mode 2, but without the URL line
          for (size_t i = 0; i < hits.size(); i++)
          {
            // output the title with href
            result_stream << "<dl><dt><a href=\"" << hits[i].url
                   // << "?q=" << result.query   // HOLGER 5May05: this gives problems with URLs from domino
                   << "\">" << hits[i].title << "</a>"
                   // << "<span class=\"score\">" << (unsigned int)((hits[i].score & 128) >> 7) << " "
                   //                             << (unsigned int)((hits[i].score &  96) >> 5) << " "
                   //                             << (unsigned int)((hits[i].score &  24) >> 3) << " "
                   //                             << (unsigned int)((hits[i].score &   7)     ) << "</span>"
                   << "</dt><dd>";
            // output the excerpts
            for (size_t j = 0; j < hits[i].excerpts.size(); j++) 
              result_stream << hits[i].excerpts[j] 
                            << ( queryParameters.displayMode != 1 && j < hits[i].excerpts.size()-1 ? "</dd><dd>" : "" );
            result_stream << "</dd>";
            // output the URL (except for mode 3)
            if (queryParameters.displayMode <= 2)
              result_stream 
                   // << "<dd>" << excerptshits[i].excerpt() << "<br>"
                   // << "<dd style=\"font-size:10px\">" << hits[i].excerpt() << "<br>"
                   << "<dd><i><a href=\"" << hits[i].url
                   << "?q=" << "dummyQuery" << "\">"
                   << hits[i].url 
                   << "</a></i></dd>";
            // output closing tag for this hit
            result_stream << "</dl>";
          }
        }
        else if (queryParameters.displayMode == 4)
        {
          // mode 4: show one line per excerpt 
          string titleAbridged;
          for (size_t i = 0; i < hits.size(); i++)
          {
            // ABRIDGE LONG TITLES 
            titleAbridged = hits[i].title;
            // remove all <...> (TODO: this is currently done in a very inefficient way)
            size_t position1, position2;
            while ( (position1 = titleAbridged.find('<')) != string::npos
                    && (position2 = titleAbridged.find('>',position1)) != string::npos )
              titleAbridged.erase(position1, position2-position1+1);
            // remove leading spaces
            position1 = titleAbridged.find_first_not_of(" ");
            if (position1 > 0 && position1 != string::npos)
              titleAbridged.erase(0,position1);
            // remove trailing spaces
            position2 = titleAbridged.find_last_not_of(" ");
            if (position2 < titleAbridged.length()-1 && position2 != string::npos)
              titleAbridged.erase(position2+1, titleAbridged.length()-position2);
            // finally remove part from middle if too long
            if (titleAbridged.length() > 40) 
              titleAbridged = titleAbridged.substr(0,22) + " ... " + titleAbridged.substr(titleAbridged.length()-13,13);

            for (size_t j = 0; j < hits[i].excerpts.size(); j++) 
            {
              result_stream 
                << "<a href=\"" << hits[i].url << "\">" << titleAbridged << "</a>"
                << " " << hits[i].excerpts[j] << "<br>";
            }
          }
        }
        else 
        {
          // invalid display mode
          result_stream << "[ Mode \"" << queryParameters.displayMode << "\" INVALID, no hits displayed ]<br>";
        }
        result_stream << "</div></div>\n";


      } // END OF GETTING EXCERPTS


    } // END OF NON-EMPTY QUERY BEING PROCESSED 
      

    //
    // END RESULT MESSAGE
    //
    //  !!! The end marker is very important, as the $socket->recv loop of the !!!
    //  !!! autocomplete.pl script uses it to know when it can stop reading    !!!
    //
    result_stream << "<!--END REPORT DBCompleteServer-->\n";

    //
    // SEND BACK THE RESULT 
    //
    *logfile << " * sending result string ... ";
    msg = new char[result_stream.str().length()+1];
    strcpy(msg, result_stream.str().c_str());
    socket.Write(msg);
    delete[] msg;
    *logfile << " done" << endl;


    //
    // CLOSE CONNECTION
    //
    socket.AcceptedGoodbye();
    // *logfile << " * connection to client closed" << endl;
    
    //    completer.printHistory(historyfile);
    //    historyfile.flush();


    if(completer.cutHistoryToSize(history_size)) // returns true, if it had to do something
      {
        *logfile << "\n * reduced history to " << history_size << " bytes ";
      }

  } // end of loop "while (1) ..."

  delete[] query_string;
}// end: DBCompleteServer

int main(char argc, char** argv)
{
  if (!CheckArgs(argc, argv))
  {
    PrintUsage();
    return 1;
  }

  if (killserver)
  {
    KillServer(port);
    return 0;
  }

  if (!setlocale(LC_CTYPE, "de_DE")) cerr << "Warning: locale \"de_DE\" not available" << endl;

  if (portname_given) GetPortFromFile(conffilename);

  ExcerptsGenerator *exGen = NULL;
  if (excerptsdb_given) 
  {
    cout << " * opening excerpts database \"" << excerptsdb << "\"" << endl;
    exGen = new ExcerptsGenerator(excerptsdb, cache_size);
      // HOLGER 30May05: mpi-webpages -> 3; others -> 10
      // exGen->setMaxHits(3);
  }

  assert((completertype == "INV") || (completertype == "HYB"));

  if(completertype == "INV")
    {
      DBCompleteServer<INVCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> > (exGen);
      //      DBCompleteServer<INVCompleter< WITH_POS + WITH_DUPS> > (exGen);
    }
  else if(completertype == "HYB")
    {
      DBCompleteServer<HYBCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> > (exGen);
      //      DBCompleteServer<HYBCompleter< WITH_POS + WITH_DUPS> > (exGen);
    }
  else
    {
      cout << endl << " Could not figure out, which completer type was desired! " << endl;
    }
  

  // will never come this point
  return 0;
} // end: main(...)
