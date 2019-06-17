#include "Globals.h"

using namespace std;

template<class T, unsigned int size> class fvector
{
  private:
    T elements[size];
  public:
    T& operator[](unsigned int i) { assert(i < size); return elements[i]; }
};

FILE* log_file;

int main(char argc, char** argv)
{
  Timer timer;
  cout.setf(ios::fixed);
  cout.precision(1);
  cout << endl;

  // PARAMETERS
  unsigned int n = argc > 1 ? atoi(argv[1]) : 1000000;
  unsigned int M = argc > 2 ? atoi(argv[2]) : 100*1000000;
  const unsigned int l = 5;

  // GENERATE RANDOM SORTED LISTS 
  cout << "* generating two random sorted lists of " << n/1000000 
       << " million integers from 1.." << M << " ... " << flush;
  vector<unsigned int> x(n);
  for (unsigned int i = 0; i < n; ++i) x[i] = (n/2 + i*i) % M + 1;
  sort(x.begin(), x.end());
  assert(x[0] >= 1 && x[n-1] <= M);
  for (unsigned int i = 0; i < n-1; ++i) assert(x[i] <= x[i+1]);
  vector<unsigned int> y(n);
  for (unsigned int i = 0; i < n; ++i) y[i] = (n/2 + i*i*i) % M + 1;
  sort(y.begin(), y.end());
  assert(y[0] >= 1 && y[n-1] <= M);
  for (unsigned int i = 0; i < n-1; ++i) assert(y[i] <= y[i+1]);
  cout << "done" << endl;

  // GENERATE PARALLEL LISTS WITH RANDOM DATA
  cout << "* generate " << l << " lists of same size and with random data ... " << flush;
  fvector< vector<unsigned int> , l > xx;
  vector< fvector<unsigned int,l+1> > xxx(n);
  for (unsigned int i = 0; i < n; ++i) xxx[i][0] = x[i];
  for (unsigned int j = 0; j < l; ++j) 
  {
    yy[j].resize(n, l + j*j);
    for (unsigned int i = 0; i < n; ++i) xyy[i][1+j] = yy[j][i];
  }
  cout << "done" << endl;

  cout << endl;
}

  
  


