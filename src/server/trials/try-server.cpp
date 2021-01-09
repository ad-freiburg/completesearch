//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> /* for open, etc. */
#include "assert.h"
#include "Timer.h"
#include "server.h"

using namespace std;

int main(char argc, char** argv)
{
  Timer timer;
  cout.setf(ios::fixed);
  cout.precision(3);
/*
  log_file = fopen("log", "w");
  if (log_file == NULL) { cerr << "error opening log file" << endl; exit(1); }
  log_fid = fileno(log_file);
  dup2(log_fid, STDOUT_FILENO);
  dup2(log_fid, STDERR_FILENO);
  //fclose(log_file);
*/
  int n = argc > 1 ? atoi(argv[1]) : 1000;
  int port = argc > 2 ? atoi(argv[2]) : 8888;
  char* bytes = new char[n];
  memset(bytes,1,n);
  Connector socket(port);
  cout << "SERVER: port = " << port << "; pid = " << getpid() << endl << endl;
  cout << "SERVER: waiting" << endl;
  socket.Accept();
  timer.start();
  char* msg;
  int msg_len = socket.Read(&msg, 1);
  cout << "SERVER: client said hello [" << msg[0] << "]" << endl;
  socket.Write(bytes, n);
  msg[0] = 'x';
  msg_len = socket.Read(&msg, 1);
  cout << "SERVER: client said goodbye [" << msg[0] << "]" << endl;
  timer.stop();
  cout << "SERVER: " << timer.usecs()/1000.0 << " milliseconds" << endl;
  socket.AcceptedGoodbye();
}

