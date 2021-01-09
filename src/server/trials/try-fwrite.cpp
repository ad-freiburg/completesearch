#include <iostream>
#include "Timer.h"

using namespace std;

int main(char argc, char** argv)
{
  Timer timer;
  cout.setf(ios::fixed);
  cout.precision(3);
  int n = argc > 1 ? atoi(argv[1]) : 1000;
  char* bytes = new char[n];
  FILE* file = fopen("xxx","w");
  timer.start();
  fwrite(bytes, 1, n, file);
  timer.stop();
  cout << "WRITE: " << timer.usecs()/1000.0 << " milliseconds" << endl;
  timer.start();
  fclose(file);
  timer.stop();
  cout << "CLOSE: " << timer.usecs()/1000.0 << " milliseconds" << endl;
}


  

