#include <iostream>
#include <signal.h>

using namespace std;

/*
struct sigaction 
{
  void (*sa_handler)(int);
  void (*sa_sigaction)(int, siginfo_t *, void *);
  sigset_t sa_mask;
  int sa_flags;
  void (*sa_restorer)(void);
}
*/

void myhandler(int signum)
{
  cerr << "Pech: FPE (" << signum << ")" << endl;
  signal(SIGFPE, SIG_IGN);
}

int main(char argc, char** argv)
{
  signal(SIGFPE, &myhandler);
  /*
  struct sigaction act;
  act.sa_handler = &myhandler;
  act.sa_flags = 0;
  int ret = sigaction(SIGFPE, &act, NULL);
  if (ret != 0) { cerr << "Shit" << endl; exit(1); }
  */
  int x = 1 / atoi("xxx");
  cout << "Hallo " << x << endl;
}
