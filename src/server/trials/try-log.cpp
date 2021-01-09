#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

class ConcurrentLog
{
 public:
  ostringstream os;
  int _id;
  ConcurrentLog() { _id = -1; } 
  void setId(int id) { _id = id; } 
};

template<class T>
ConcurrentLog& operator<<(ConcurrentLog& ms, T x) 
{ 
  ms.os << x;
  return ms;
}

ConcurrentLog& operator<<(ConcurrentLog& ms, ConcurrentLog&(*f)(ConcurrentLog&))
{
  return f(ms);
}

ConcurrentLog& flush(ConcurrentLog& ms)
{ 
  ms.os << flush;
  return ms;
}

ConcurrentLog& endl(ConcurrentLog& ms)
{ 
  cout << "[" << ms._id << "] " << ms.os.str() << endl;
  ms.os.str("");
  //ms.atBeginningOfLine = true;
  return ms;
}


int main(char argc, char** argv)
{
  //log << 5;
  ConcurrentLog log;
  log.setId(23);
  log << "Here is a (formatted) number: " << setfill('*') << setw(10) << 2145 << endl;
  log << "Hello," << flush;
  log << " hello" << endl;
  log << "The End!" << endl << endl;
}
