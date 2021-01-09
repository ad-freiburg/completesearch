#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> /* for open, etc. */
#include "assert.h"
#include "Timer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

int main(char argc, char** argv)
{
  Timer timer;
  cout.setf(ios::fixed);
  cout.precision(3);

  int B = 1000;
  int n = argc > 1 ? atoi(argv[1]) : 1000;
  int port = argc > 2 ? atoi(argv[2]) : 8888;
  const char* host_name = argc > 3 ? argv[3] : "mpiao9534";
  char* buf = new char[n];
  memset(buf,0,n);
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serv_addr;
  struct hostent *server = gethostbyname(host_name);
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
  serv_addr.sin_port = htons(port);
  int ret = connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
  if (ret < 0) { cerr << strerror(errno) << endl; exit(1); }
  char* hello = "hello";
  write(sockfd, hello, 1);
  timer.start();
  cout << "CLIENT: buf[0] = " << int(buf[0]) << "; buf[n-1] = " << int(buf[n-1]) << endl;
  for (int i = 0; i < n; i += B) ret = recv(sockfd, buf + i, B, 0);
  cout << "CLIENT: buf[0] = " << int(buf[0]) << "; buf[n-1] = " << int(buf[n-1]) << endl;
  timer.stop();
  cout << "CLIENT: " << timer.usecs()/1000.0 << " milliseconds" << endl;
  if (ret < 0) { cerr << strerror(errno) << endl; exit(1); }
  char* goodbye = "goodbye";
  write(sockfd, goodbye, 1);
  //read(sockfd, buf, n);
}
