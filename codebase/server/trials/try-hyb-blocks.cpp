#include <iostream>
#include <vector>
#include "Timer.h"
#include "assert.h"

using namespace std;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

int main(char argc, char** argv)
{
  cout << endl << EMPH_ON << "SOME TIMINGS FOR WRITING MANY HYB BLOCKS" << EMPH_OFF << endl << endl;
  cout.setf(ios::fixed);
  cout.precision(2);
  Timer timer;

  unsigned int n = argc > 1 ? atoi(argv[1]) : 1000000;
  unsigned int k = argc > 2 ? atoi(argv[2]) : 100;
  unsigned int R = argc > 3 ? atoi(argv[3]) : 3;

  cout << "* generate array A of " << n << " pseudo-random ints ... " << flush;
  vector<int> A(n);
  vector<unsigned int> counts(k, 0);
  for (unsigned int i = 0; i < n; ++i) 
  {
    A[i] = ((160481183 * i) % 198491317) % k;
   ++counts[ A[i] ];
  }
  cout << "done" << endl << endl;

  for (int run = 1; run <= R; ++run)
  {
    cout << "* just copy elements to another vector ... " << flush;
    vector<int> B(n);
    timer.start();
    for (unsigned int i = 0; i < n; ++i) B[i] = A[i];
    timer.stop();
    cout << "done in " << (float)(timer.usecs())/1000 << " milliseconds (" 
         << EMPH_ON << (n*sizeof(int))/timer.usecs() << " MiB/second" << EMPH_OFF << ")" << endl;
  }
  cout << endl;

  for (int run = 1; run <= R; ++run)
  {
    cout << "* copy with memcpy ... " << flush;
    vector<int> B(n);
    timer.start();
    memcpy((void*)(&B[0]),(void*)(&A[0]),n*sizeof(int));
    timer.stop();
    cout << "done in " << (float)(timer.usecs())/1000 << " milliseconds (" 
         << EMPH_ON << (n*sizeof(int))/timer.usecs() << " MiB/second" << EMPH_OFF << ")" << endl;
  }
  cout << endl;

  for (int run = 1; run <= R; ++run)
  {
    cout << "* append elements randomly to " << k << " other (preallocated) arrays ... " << flush;
    vector< vector<int> > B(k);
    for (unsigned int i = 0; i < k; ++i) B[i].resize(counts[i]); 
    vector<int> c(k, -1);
    int j;
    timer.start();
    for (unsigned int i = 0; i < n; ++i) { j = A[i]; B[j][++c[j]] = j; }
    timer.stop();
    cout << "done in " << (float)(timer.usecs())/1000 << " milliseconds (" 
         << EMPH_ON << (n*sizeof(int))/timer.usecs() << " MiB/second" << EMPH_OFF << ")" << endl;
  }
  cout << endl;

}
