#ifndef __CONCURRENT_LOG_H__
#define __CONCURRENT_LOG_H__

#include "Globals.h"

//! Global log verbosity; see class LogVerbosity 
extern int logVerbosity;

//! Objects for writing to ConcurrentLog (with <<) to indicate the current level of verbosity
/*!
 *   Usage: Write something line
 *
 *   log << IF_VERBOSITY_HIGH << ... << ... << .. << endl
 *
 *   will write the line to the output only if ::verbosity is larger than
 *   VERBOSITY _HIGH. Possible objects are IF_VERBOSITY_HIGH,
 *   IF_VERBOSITY_HIGHER, and IF_VERBOSITY_HIGHEST; see below.
 */
class LogVerbosity
{
 public:

  //! Possible levels of verbosity
  enum VerbosityEnum {
    ZERO    = 0,
    NORMAL  = 1,
    HIGH    = 2,
    HIGHER  = 3,
    HIGHEST = 4
  };

  //! Level of verbosity of this object
  int _verbosity;

  //! Construct with normal verbosity 
  LogVerbosity(int verbosity) { _verbosity = verbosity; }
  void setVerbosity(int verbosity) { _verbosity = verbosity; }
  int getVerbosity() const { return _verbosity; }
};

//! Fixed verbosity objects, one for each level
const LogVerbosity IF_VERBOSITY_ANY(LogVerbosity::ZERO);
const LogVerbosity IF_VERBOSITY_NORMAL(LogVerbosity::NORMAL);
const LogVerbosity IF_VERBOSITY_HIGH(LogVerbosity::HIGH);
const LogVerbosity IF_VERBOSITY_HIGHER(LogVerbosity::HIGHER);
const LogVerbosity IF_VERBOSITY_HIGHEST(LogVerbosity::HIGHEST);


//! These are deprecated
#define COUT_NORMAL       if (::logVerbosity >= LogVerbosity::NORMAL ) cout
#define COUT_HIGH         if (::logVerbosity >= LogVerbosity::HIGH   ) cout
#define COUT_HIGHER       if (::logVerbosity >= LogVerbosity::HIGHER ) cout
#define COUT_HIGHEST      if (::logVerbosity >= LogVerbosity::HIGHEST) cout

//! Strip of the arguments from a __PRETTY_FUNCTION__ string (global function)
string functionName(string prettyFunction);

//! Standard log message at beginning of method (if verbosity high)
#define AT_BEGINNING_OF_METHOD IF_VERBOSITY_HIGH << "! at beg of method " << functionName(__PRETTY_FUNCTION__)

//! Standard log message at end of method (if verbosity high)
#define AT_END_OF_METHOD IF_VERBOSITY_HIGH << "! at end of method " << functionName(__PRETTY_FUNCTION__)


//! A log the can be written to concurrently, with an id for each writer
/*!
 *    Usage: A log object can be used just like cout, and most the stream
 *    modifying operators can be used in the same way, e.g.
 *
 *    log << setw(10) << setfill('.') << myCounter << endl;
 *
 *    The current implementation is as follows: Each thread creates its own log
 *    object, and gives it a id via the setId method. Whenever endl is written
 *    via << everything written to the log until then is written to stdout.
 *
 *    Warning: it has to be << endl, a string containing a \n will not work.
 *
 *    Each line will be written using a single printf. That way, there is
 *    usually no intermingling of characters (written concurrently from
 *    different threads) in the output. 
 *
 *    Warning: this is just the behaviour of printf in practice, there is no
 *    gurantuee for this, it's not in the specification, and in fact, I have
 *    seen intermingling of characters for two concurrent printf's but it's
 *    rare.
 */
class ConcurrentLog
{
 public:
  ostringstream os;
  int _id;
  int _verbosity;
  ConcurrentLog() { _id = 0; _verbosity = LogVerbosity::NORMAL; } 
  void setId(int id) { _id = id; } 
  void setVerbosity(int verbosity) { _verbosity = verbosity; }
  int getVerbosity() {return _verbosity; }
  // Dump objects as string.
  std::string asString(void) { return os.str(); }
};


//! Write a verbosity object, e.g. log << IF_VERBOSITY_HIGH
ConcurrentLog& operator<<(ConcurrentLog& ms, const LogVerbosity& v);

//! Write all kinds of objects to the log
template<class T>
ConcurrentLog& operator<<(ConcurrentLog& ms, T x);

//! Write a modifier to the log, e.g. setw, setfill, setprecision, etc.
ConcurrentLog& operator<<(ConcurrentLog& ms, ConcurrentLog&(*f)(ConcurrentLog&));

//! Write log << flush
ConcurrentLog& flush(ConcurrentLog& ms);

//! Write log << endl
ConcurrentLog& endl(ConcurrentLog& ms);

//! Write log << newline; TODO: needed for what?
ConcurrentLog& newline(ConcurrentLog& ms);

//! General log (for methods running outside of worker threads)
extern ConcurrentLog LOG;

#endif
