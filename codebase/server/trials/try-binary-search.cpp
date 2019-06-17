#include "Globals.h"
#include "Timer.h"

using namespace std;

int main(char argc, char** argv)
{
  cout << endl << EMPH_ON << "TIME BINARY SEARCH (" << VERSION << ")" 
       << EMPH_OFF << endl << endl;
  cout << setiosflags(ios::fixed);
  cout << setprecision(2);
  Timer timer;

  unsigned int n = argc > 1 ? atoi(argv[1]) : 1000000;
  unsigned int R = argc > 2 ? atoi(argv[2]) : 50;

  cout << "creating array of " << n << " ints ... " << flush;
  timer.start();
  vector<unsigned int> v;
  try {
    v.resize(n);
  }
  catch (exception& e) { 
    cerr << "EXCEPTION: " << e.what() << endl << endl; exit(1); 
  }
  for (unsigned int i = 0; i < n; i++) v[i] = i;
  timer.stop();
  cout << "done in " << timer << endl << endl;

  for (unsigned int r = 1; r <= R; r++)
  {
    cout << "binary search for random element ... " << flush;
    unsigned int x = (unsigned int)(drand48()*n);
    timer.start();
    unsigned int l = 0;
    unsigned int r = n-1;
    // keep the invariant that v[l] <= x <= v[r]
    while (l < r)
    {
      unsigned int m = (r + l)/2;
      if (x <= v[m]) r = m; else l = m + 1;
    }
    timer.stop();
    assert(l == r);
    assert(v[l] == x);
    cout << "done in " << timer << endl;
  }
  cout << endl;
}
