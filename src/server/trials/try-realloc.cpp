#include "Globals.h"
#include "Timer.h"

using namespace std;

int main(char argc, char** argv)
{
  cout << endl << EMPH_ON << "DOES REALLOC COPY WHEN DOWNSIZING?" << EMPH_OFF << endl << endl;
  Timer timer;

  unsigned int n = argc >= 2 ? atoi(argv[1]) : 1000000000;

  {
    cout << "* constructing vector with " << commaStr(n) << " bytes using STL::vector ... " << flush;
    timer.start();
    vector<char> v(n);
    timer.stop();
    cout << "done in " << timer << " (" << n/timer.usecs() << " MiB/second)" << endl << endl;

    cout << "! press any key to downsize ... " << flush;
    waitUntilKeyPressed();
    cout << endl << endl;

    cout << "* resizing to 10\% of the size using resize ... " << flush;
    timer.start();
    v.resize(n/10); 
    timer.stop();
    cout << "done in " << timer << " (" << n/10/timer.usecs() << " MiB/second wrt reduced size)" << endl << endl;
    assert(v.capacity() == n);
    
    cout << "! press any key to swap ... " << flush;
    waitUntilKeyPressed();
    cout << endl << endl;

    cout << "* reducing capacity using swap ... " << flush;
    timer.start();
    vector<char>(v).swap(v);
    timer.stop();
    cout << "done in " << timer << " (" << n/10/timer.usecs() << " MiB/second wrt reduced size)" << endl << endl;
    assert(v.capacity() == n/10);

    cout << "! press any key to continue ... " << flush;
    waitUntilKeyPressed();
    cout << endl << endl;

  }

  cout << "* allocating and initializing " << commaStr(n) << " bytes using malloc and memset ... " << flush;
  timer.start();
  char* bytes = (char*)(malloc(n));
  if (bytes == NULL) { cout << "FAILED!" << endl << endl; exit(1); }
  memset(bytes, 0, n);
  timer.stop();
  cout << "done in " << timer << " (" << n/timer.usecs() << " MiB/second)" << endl << endl;

  cout << "! press any key to downsize ... " << flush;
  waitUntilKeyPressed();
  cout << endl << endl;
  
  cout << "* resizing to 10\% of the size using realloc ... " << flush;
  timer.start();
  char* newBytes = (char*)realloc(bytes, n/10);
  if (newBytes == NULL) { cout << "FAILED!" << endl << endl; exit(1); }
  timer.stop();
  cout << "done in " << timer << " (" << n/10/timer.usecs() << " MiB/second wrt reduced size)" << endl << endl;
   
  cout << "* pointer returned by malloc and by realloc are " 
       << EMPH_ON << ( newBytes == bytes ? "SAME" : "DIFFERENT" ) << EMPH_OFF
       << endl << endl;

  cout << "! press any key to exit ... " << flush;
  waitUntilKeyPressed();
  cout << endl << endl;
  
}
