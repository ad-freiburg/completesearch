//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> /* for open, etc. */
#include "assert.h"
#include "Timer.h"
//#include "server.h"

using namespace std;

extern void parent();
extern void child();
extern void grand_child();
extern void lock_file(int fid, int lock_or_unlock);

FILE* log_file;
int log_fid;
Timer timer_fork, timer_memset;
char* bytes;
int n;

int main(char argc, char** argv)
{
  cout.setf(ios::fixed);
  cout.precision(3);
  log_file = fopen("log", "w");
  if (log_file == NULL) { cerr << "error opening log file" << endl; exit(1); }
  log_fid = fileno(log_file);
  dup2(log_fid, STDOUT_FILENO);
  dup2(log_fid, STDERR_FILENO);
  //fclose(log_file);

  n = argc > 1 ? atoi(argv[1]) : 1000;
  bytes = new char[n];
  timer_memset.start();
  memset(bytes,0,n);
  timer_memset.stop();
  cout << "MAIN: allocated " << n << " bytes (memset took " << timer_memset.usecs()/1000.0 << " milliseconds)" << endl;
  char* bytes2 = new char[n];
  timer_memset.start();
  memcpy((void*)bytes2,(void*)bytes,n);
  timer_memset.stop();
  cout << "MAIN: copied " << n << " bytes (took " << timer_memset.usecs()/1000.0 << " milliseconds)" << endl;
  delete[] bytes2;
  //cout << "parents message to cout" << endl;
  //cerr << "parents message to cerr" << endl;
  //assert(1 == 0);
  //int port = argc > 2 ? atoi(argv[1]) : 8888;
  //Connector socket(port);
  //execv("ls", NULL);
  //fid = open("log", O_WRONLY|O_CREAT, 0644);
  // first fork (parent, child)
  timer_fork.start();
  pid_t pid = fork();
  timer_fork.stop();
  if (pid < 0) { cerr << "error in first fork (" << strerror(errno) << ")" << endl; exit(1); }
  if (pid > 0) { parent(); }
  // second fork (child, grand child)
  pid = fork();
  if (pid < 0) { cerr << "error in second fork (" << strerror(errno) << ")" << endl; exit(1); }
  if (pid > 0) { child(); }
  grand_child();
}

// parent exits
void parent()
{
  //cerr << "PARENT: sleeping for 2 seconds" << endl; sleep(2);
  /*
  timer_memset.start();
  memset(bytes,1,n);
  timer_memset.stop();
  cout << "PARENT: set bytes to 1 (took " << timer_memset.usecs()/1000.0 << " milliseconds)" << endl;
  */
  lock_file(log_fid, F_WRLCK);
  cout << "PARENT: value of first byte is " << (int)(bytes[0]) << endl;
  cout << "PARENT: fork took " << timer_fork.usecs()/1000.0 << " milliseconds" << endl;
  cout << "PARENT: exiting " << endl;
  lock_file(log_fid, F_UNLCK);
  exit(0);
}

// child remains alive (waiting for grand_child to die)
void child()
{
  dup2(log_fid, STDOUT_FILENO);
  dup2(log_fid, STDERR_FILENO);
  //fclose(log_file);
  lock_file(log_fid, F_WRLCK);
  cout << "CHILD: fork took " << timer_fork.usecs()/1000.0 << " milliseconds" << endl;
  lock_file(log_fid, F_UNLCK);
  //cerr << "CHILD: sleeping for 2 seconds" << endl; sleep(2);
  timer_memset.start();
  memset(bytes,2,n);
  timer_memset.stop();
  lock_file(log_fid, F_WRLCK);
  cout << "CHILD: set bytes to 2 (took " << timer_memset.usecs()/1000.0 << " milliseconds)" << endl;
  cout << "CHILD: waiting for grandchild" << endl;
  lock_file(log_fid, F_UNLCK);
  int status;
  pid_t pid = wait(&status);
  lock_file(log_fid, F_WRLCK);
  cout << "CHILD: value of first byte is " << (int)(bytes[0]) << endl;
  cerr << "CHILD: grand_child (pid " << pid << ") exited with status " << status << " (cf. man 7 signal)" << endl;
  cerr << "CHILD: exiting" << endl;
  lock_file(log_fid, F_UNLCK);
  exit(0);
}

// grandchild 
void grand_child()
{
  dup2(log_fid, STDOUT_FILENO);
  dup2(log_fid, STDERR_FILENO);
  //fclose(log_file);
  lock_file(log_fid, F_WRLCK);
  cout << "GRANDCHILD: message to cout" << endl;
  cerr << "GRANDCHILD: message to cerr" << endl;
  cerr << "GRANDCHILD: sleeping for 1 second" << endl;
  lock_file(log_fid, F_UNLCK);
  sleep(1); /* sleep for this many seconds */
  //while (true) {} /* infinite loop */
  //assert(1 == 0);
  int* p = 0;
  *p = 0; /* force a segmentation fault */
  lock_file(log_fid, F_WRLCK);
  cerr << "GRANDCHILD: exiting" << endl;
  lock_file(log_fid, F_UNLCK);
}

void lock_file(int fid, int lock_type)
{
  struct flock fl;
  fl.l_type   = lock_type;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
  fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
  fl.l_start  = 0;        /* Offset from l_whence         */
  fl.l_len    = 0;        /* length, 0 = to EOF           */
  fl.l_pid    = getpid(); /* our PID                      */
  fcntl(fid, lock_type == F_UNLCK ? F_SETLK : F_SETLKW, &fl);  /* lock (wait if necessary) or unlock */
}
