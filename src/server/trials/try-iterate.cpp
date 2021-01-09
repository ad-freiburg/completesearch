#include "Globals.h"

int main(char argc, char** argv)
{

  cout << endl << "C++ TIMING DEMO" << endl << endl;
  Timer timer;
  int n = argc > 1 ? atoi(argv[1]) : 250*1000*1000;
  int* a = new int[n];
  for (int i = 0; i < n; i++) { a[i] = i; }

  for (int run = 1; run <= 3; run++)
  {
    cout << "* iterating over array of " << commaStr(n) << " ints ... " << flush;
    int sum = 0;
    timer.start();
    for (int i = 0; i < n; i++) { sum += a[i]; }
    timer.stop();
    cout << "done in " << timer.msecs() << " millisecs (" << n/timer.usecs() << " MiB per second)" << endl;
  }
  cout << endl;

  delete[] a;

}
