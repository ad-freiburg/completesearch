#include "Globals.h"
#include "ConcurrentLog.h"
#include "Query.h"
#include "QueryParameters.h"

//! Global log verbosity; see class LogVerbosity 
int logVerbosity = LogVerbosity::NORMAL;

//! General log (for methods running outside of worker threads)
ConcurrentLog LOG;

//! Strip of the arguments from a __PRETTY_FUNCTION__ string (global function)
string functionName(string prettyFunction)
{
  size_t pos;

  // strip off return type
  pos = prettyFunction.find(' ');
  if (pos != string::npos) prettyFunction.erase(0, pos + 1);

  // strip off arguments 
  pos = prettyFunction.find('(');
  if (pos != string::npos) prettyFunction = prettyFunction.substr(0, pos);

  return prettyFunction;
}

//! Write all kinds of objects to the log
template<class T>
ConcurrentLog& operator<<(ConcurrentLog& ms, T x) 
{ 
  ms.os << x;
  return ms;
}

//! Write a verbosity object, e.g. log << IF_VERBOSITY_HIGH
ConcurrentLog& operator<<(ConcurrentLog& ms, const LogVerbosity& v)
{
  ms.setVerbosity(v.getVerbosity());
  return ms;
}

//! Write a modifier to the log, e.g. setw, setfill, setprecision, etc.
ConcurrentLog& operator<<(ConcurrentLog& ms, ConcurrentLog&(*f)(ConcurrentLog&))
{
  return f(ms);
}

//! Write log << flush
ConcurrentLog& flush(ConcurrentLog& ms)
{ 
  ms.os << flush;
  return ms;
}

//! Write log << endl
ConcurrentLog& endl(ConcurrentLog& ms)
{ 
  // output only if verbosity high enough
  if (::logVerbosity >= ms._verbosity)
  {
    cout << "[" << setw(5) << setfill(' ') << ms._id << "] " << ms.os.str() << endl;
  }
  ms.os.str("");
  ms.setVerbosity(LogVerbosity::NORMAL);
  return ms;
}

//! Write log << newline; TODO: needed for what?
ConcurrentLog& newline(ConcurrentLog& ms)
{
  cout << "[" << ms._id << "] " << ms.os.str();
  ms.os.str("");
  return ms;
}
  
#include "HYBCompleter.h"
//! For DEBUGGING
void TestLog()
{
  HybCompleter<7> completer;
  completer.log << IF_VERBOSITY_HIGH << "Hallo" << endl;
};


//! EXPLICIT INSTANTIATION (so that actual code gets generated)
template ConcurrentLog& operator<<<int>(ConcurrentLog&, int);
template ConcurrentLog& operator<<<unsigned int>(ConcurrentLog&, unsigned int);
template ConcurrentLog& operator<<<unsigned long>(ConcurrentLog&, unsigned long);
template ConcurrentLog& operator<<<long int>(ConcurrentLog&, long int);
template ConcurrentLog& operator<<<long long>(ConcurrentLog&, long long);
template ConcurrentLog& operator<<<char>(ConcurrentLog&, char);
template ConcurrentLog& operator<<<signed char>(ConcurrentLog&, signed char);
template ConcurrentLog& operator<<<const char*>(ConcurrentLog&, const char*);
template ConcurrentLog& operator<<<char*>(ConcurrentLog&, char*);
template ConcurrentLog& operator<<<bool>(ConcurrentLog&, bool);
template ConcurrentLog& operator<<<double>(ConcurrentLog&, double);
template ConcurrentLog& operator<<<string>(ConcurrentLog&, string);
template ConcurrentLog& operator<<<Query>(ConcurrentLog&, Query);
template ConcurrentLog& operator<<<QueryParameters>(ConcurrentLog&, QueryParameters);
template ConcurrentLog& operator<<<WordRange>(ConcurrentLog&, WordRange);
template ConcurrentLog& operator<<<std::_Setiosflags>(ConcurrentLog&, std::_Setiosflags);
template ConcurrentLog& operator<<<std::_Setprecision>(ConcurrentLog&, std::_Setprecision);
template ConcurrentLog& operator<<<std::_Setw>(ConcurrentLog&, std::_Setw);
template ConcurrentLog& operator<<<std::ios_base&(*)(std::ios_base&)>(ConcurrentLog&, std::ios_base&(*)(std::ios_base&));
template ConcurrentLog& operator<<<Timer>(ConcurrentLog&, Timer);
