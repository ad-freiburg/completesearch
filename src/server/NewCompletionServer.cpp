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
//#include "SynonymsDB.h"
#include "QueryParameters.h"
#include <pthread.h> /* don't forget to link with -lpthread !!! */

using namespace std;
//using namespace aucmpl;

void* dummy(void*) { return NULL; }

//! USAGE INFO
void printUsage()
{
  cout << endl
       << "Usage: NewCompletionServer [options] index_file_name" << endl 
       << endl
       << "Options:" << endl
       << endl
       << " -p port number       The port where the server is listening (default: 8888)" << endl
       << " -l log file name     File name for log messages (default: LOG)" << endl
       << " -k port number       Stop server running at specified port (ignores all other options/parameters)" << endl
       //<< "           -s synonyms_db_file  The name of an synonyms.db file\n"
       << " -c cache size        Sets the cache size for the excerpts generator (default: 16 megabytes)" << endl
       << " -h history size      Sets the history size (default: 32 megabytes)" << endl 
       << " -e exerpts file      Name of BerkeleyDB file containing with document info (default: <db>.docs.db)" << endl
       << " -t [INV|HYB]         Type of index (default: guess from index file name)" << endl
       << endl
       << "Cache/history sizes must be greater than 0 and are given in one of the forms:" << endl 
       << endl
       << "  n   meaning n bytes" << endl
       << "  nK  meaning n kilobytes" << endl
       << "  nM  meaning n megabytes" << endl 
       << endl;
}

// PARAMETERS with defaults
int port = 8888;
bool portToKill = 0;
string indexFileName = "";
string excerptsDBFileName = "";
string synonymsDBFileName = "";
string logFileName = "LOG";
string vocabularyFileName;
string indexTypeName = "HYB"; /* "INV" or "HYB" */
size_t excerptsDBCacheSize = 16*1024*1024; /* in bytes */
size_t historySize = 32*1024*1024; /* in bytes */

// OTHER GLOBALS (should become part of Completer object)
ExcerptsGenerator* exGen = NULL;

// FUNCTION DECLARATIONS (implementation below, after main)
extern ofstream* logfile;
int atoi_ext(string s);
void makeQueryWellFormed(Query& query);
void killServer(int port);
template<class Completer> void startServer();

 

//! MAIN LOOP
int main(char argc, char** argv)
{
  // WE WANT LOCALE de_DE (because htdig sorted according to that)
  if (!setlocale(LC_CTYPE, "de_DE")) cerr << "Warning: locale \"de_DE\" not available" << endl;

  // PARSE PARAMETERS
  while (true)
  {
    int c = getopt(argc, argv, "p:k:l:e:s:c:h:v:t:");
    if (c == -1) break;
    switch (c)
    {
      case 'p': port = atoi(optarg); break;
      case 'k': portToKill = atoi(optarg); break;
      case 'l': logFileName = optarg; break;
      case 'e': excerptsDBFileName = optarg; break;
      case 's': synonymsDBFileName = optarg; break;
      case 'c': excerptsDBCacheSize = atoi_ext(optarg); break; /* permits suffix K or M */
      case 'h': historySize = atoi_ext(optarg); break; /* dito */
      case 'v': vocabularyFileName = optarg; break;
      case 't': indexTypeName = optarg; break;
      default : printUsage(); exit(1); break;
    }
  }
  if (optind >= argc) { printUsage(); exit(1); }
  indexFileName = argv[optind++];
  string::size_type pos = indexFileName.find(".");
  string baseName = pos != string::npos ? indexFileName.substr(0, pos) : indexFileName;
  if (excerptsDBFileName == "") excerptsDBFileName = baseName + ".docs.db";
  if (vocabularyFileName == "") vocabularyFileName = baseName + ".vocabulary";
  if (indexTypeName != "INV" && indexTypeName != "HYB") 
    if (indexFileName.find("inverted") != string::npos) indexTypeName = "INV";
    else if (indexFileName.find("hybrid") != string::npos) indexTypeName = "HYB";
    else { cerr << "! could not guess index type" << endl << endl; printUsage(); exit(1); }

  // KILL SERVER?
  if (portToKill > 0) { killServer(portToKill); exit(0); }

  // START SERVER
  if (indexTypeName == "INV") startServer<INVCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> >();
  else if (indexTypeName == "HYB") startServer<HYBCompleter<WITH_SCORES + WITH_POS + WITH_DUPS> >();
  else { assert(false); exit(1); }
  
} // end of MAIN


template<class Completer>
void startServer()
{
  cout << endl << EMPH_ON << "AUTOCOMPLETION SERVER (NEW VERSION 14Jun06)" << EMPH_OFF << endl << endl;
  Timer timer;

  // INIT INDEX FILE
  cout << "* completing from index \"" << indexFileName << "\"" << endl;
  unsigned long int initIntBuffSize = INTERSECTION_BUFFER_INIT;
  unsigned long int initUniBuffSize = UNION_BUFFER_INIT;
  timer.start();  
  Completer completer(indexFileName, vocabularyFileName, initIntBuffSize, initUniBuffSize);
  timer.stop();
    //cout << " done (" << setprecision(3) << timer.value()/1000000.0 << " seconds)" << endl
  cout << endl;
  completer._metaInfo.show();
  cout << endl;

  // INIT EXCERPTS DB
  if (excerptsDBFileName != "") 
  {
    exGen = new ExcerptsGenerator(excerptsDBFileName, excerptsDBCacheSize);
    cout << "* openend excerpts DB \"" << excerptsDBFileName << "\" with " 
         << excerptsDBCacheSize/(1024*1024) << " MB cache" << endl << endl;
  }
  else
  {
    exGen = NULL;
    cout << "! NO EXCERPT DATABASE SPECIFIED" << endl << endl;
  }

  // SYNONYMS DB
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

  // INIT SOCKET CONNECTION
  cout << "* starting server at port \"" << port << "\"" << endl << endl;
  Connector socket(port);
  socket.init(logFileName.c_str());
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
  if (!logfile->is_open()) { cerr << endl << "Could not open logfile. Switching off logging." << endl; }

  // LISTEN FOR REQUESTS (forever until killed)
  while (true)
  {
    // WAIT FOR NEW QUERY
    cout << "\n* Waiting for first query .... " << endl;
    socket.Accept();

    // PROCESS QUERY 
    time_t NOW = time(NULL);
    if (logfile->is_open()) *logfile << endl << "--- NEW QUERY  " << ctime(&NOW);
    //void* (*fct_ptr)(void*); 
    //fct_ptr = dummy;
    //fct_ptr = (void*(*)(void*))(processQuery<Completer>);
    void* args[2];
    args[0] = (void*)(&socket);
    args[1] = (void*)(&completer);
    pthread_t pthread_id;
    pthread_create(&pthread_id, NULL, processQuery<Completer>, args);
    pthread_join(pthread_id, NULL);

    /* connection will/should be closed down by processQuery */

  } // end of while(true)

} // end of startServer






//! CONVERT TO INTEGER  accepting things like 10K (=10*1024) or 10M (=10*1024*1024)
int atoi_ext(string s)
{
  if (s.length() == 0) return 0;
  switch (s[s.length()-1])
  {
    case 'k': case 'K': return atoi(s.substr(0,s.length()-1).c_str())*1024;
    case 'm': case 'M': return atoi(s.substr(0,s.length()-1).c_str())*1024*1024;
    default: return atoi(s.c_str());
  }
}





void killServer(int port)
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




/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/


//
// OLD STUFF (delete it, once the whole thing compiles and runs again)
//

/*
 
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

*/
