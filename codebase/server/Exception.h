#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "Globals.h"

// Throw exception with additional assert-like info 
#define CS_THROW(e, m) { ostringstream __os; __os << m; throw Exception(e,  __os.str(), __FILE__, __LINE__, __PRETTY_FUNCTION__); }

// Rethrow an exception
#define CS_RETHROW(e) throw Exception(e.getErrorCode(), e.getErrorDetails())

// Needed for Cygwin (will be an identical redefine for *nixes)
#define __STRING(x) #x

//! Custom assert which does not abort but throws an exception
/*!
 *   NOTE: Should be used only for asserts which affect the total running only
 *   very insignificantly. Counterexample: don't use this in an inner loop that
 *   is executed million of times, and has otherwise little code.
 */
#define CS_ASSERT(condition) { if (!(condition)) { CS_THROW(Exception::ASSERT_FAILED, __STRING(condition)); } }

//! Assert equality, and show values if fails
#define CS_ASSERT_EQ(t1, t2) { if (!((t1) == (t2))) { CS_THROW(Exception::ASSERT_FAILED, __STRING(t1 == t2) << "; " << (t1) << " != " << (t2)); } }

//! Assert less than, and show values if fails
#define CS_ASSERT_LT(t1, t2) { if (!((t1) < (t2))) { CS_THROW(Exception::ASSERT_FAILED, __STRING(t1 < t2) << "; " << (t1) << " >= " << (t2)); } }

//! Assert greater than, and show values if fails
#define CS_ASSERT_GT(t1, t2) { if (!((t1) > (t2))) { CS_THROW(Exception::ASSERT_FAILED, __STRING(t1 > t2) << "; " << (t1) << " <= " << (t2)); } }

//! Assert less or equal than, and show values if fails
#define CS_ASSERT_LE(t1, t2) { if (!((t1) <= (t2))) { CS_THROW(Exception::ASSERT_FAILED, __STRING(t1 <= t2) << "; " << (t1) << " > " << (t2)); } }

//! Assert greater or equal than, and show values if fails
#define CS_ASSERT_GE(t1, t2) { if (!((t1) >= (t2))) { CS_THROW(Exception::ASSERT_FAILED, __STRING(t1 >= t2) << "; " << (t1) << " < " << (t2)); } }


// For debugging
#define CS_DEBUG_INFO(message) log << "! DEBUG INFO: " << message << " (" << __FILE__ << ":" << __LINE__ << ", " << __PRETTY_FUNCTION__ << ")" << endl

using namespace std;

//! EXCEPTION CLASS
/*
 *   For any kind of exception that occurs during processing a query
 *
 *   The following three items are associated with each exception:
 *
 *   1. The error code, e.g., COULD_NOT_GET_MUTEX (#defined in Globals.h)
 *
 *   2. The error message pertaining to that code, e.g. "MUTEX EXCEPTION: Could
 *   not get lock on mutex" (provided by method errorCodeAsString below)
 *
 *   3. Additional details specified when throwing the expection, e.g., "when
 *   accessing history item for query xyz"
 *
 *   TODO: use of the defines in Globals.h is deprecated, e.g., use
 *   Exception::QUERY_TOO_SHORT instead of just QUERY_TOO_SHORT
 */
class Exception
{
 private:

  //! Error code
  int _errorCode;

  //! Detailed information (beyond what the code already says, optionally provided by thrower)
  string _errorDetails;

 public:

  //! Error codes 
  enum ExceptionType {
    // range errors
    WORD_ID_OUT_OF_RANGE          = 16 * 1 + 1,
    // query formatting errors
    SINGLE_STAR_NOT_ALLOWED       = 16 * 2 + 1,
    CONTAINS_INVALID_SEPARATOR    = 16 * 2 + 2,
    ENHANCED_PART_TOO_SHORT       = 16 * 2 + 3,
    QUERY_TOO_SHORT               = 16 * 2 + 4,
    OR_PART_TOO_SHORT             = 16 * 2 + 5,
    BAD_REQUEST                   = 16 * 2 + 6,
    BAD_QUERY                     = 16 * 2 + 7,
    // memory allocation errors
    REALLOC_FAILED                = 16 * 3 + 1,
    NEW_FAILED                    = 16 * 3 + 2,
    // intersect errors
    ODD_LIST_LENGTH_FOR_TAG       = 16 * 4 + 1,
    DOCS_NOT_PAIRED_FOR_TAG       = 16 * 4 + 2,
    // history errors
    HISTORY_ENTRY_CONFLICT        = 16 * 5 + 1,
    HISTORY_ENTRY_NOT_FOUND       = 16 * 5 + 2,
    BAD_HISTORY_ENTRY             = 16 * 5 + 3,
    BAD_QUERY_RESULT              = 16 * 5 + 4,
    // (de)compression errors
    UNCOMPRESS_ERROR              = 16 * 6 + 1,
    // multithreading-related 
    COULD_NOT_GET_MUTEX           = 16 * 7 + 1,
    RESULT_LOCKED_FOR_READING     = 16 * 7 + 2,
    RESULT_LOCKED_FOR_WRITING     = 16 * 7 + 3,
    RESULT_NOT_LOCKED_FOR_READING = 16 * 7 + 4,
    RESULT_NOT_LOCKED_FOR_WRITING = 16 * 7 + 5,
    COULD_NOT_CREATE_THREAD       = 16 * 7 + 6,
    // socket related
    COULD_NOT_CREATE_SOCKET       = 17 * 8 + 1,
    // general errors
    ASSERT_FAILED                 = 16 * 9 + 1,
    CB_READ_FAILED                = 16 * 9 + 2,
    ERROR_PASSED_ON               = 16 * 9 + 3,
    ZLIB_ERROR                    = 16 * 9 + 4,
    NOT_YET_IMPLEMENTED           = 16 * 9 + 5,
    INVALID_PARAMETER_VALUE       = 16 * 9 + 6,
    CHECK_FAILED                  = 16 * 9 + 7,
    QUERY_TIMEOUT                 = 16 * 9 * 8,
    // unknown error
    OTHER                         = 0
  };

  //! Error messages (one per code)
  //
  //    NOTE: putting string as a return type worked then compiling under Debian
  //    Sarge, but gave the following strange linker message on Debian Etch:
  //    ... referenced in section ... defined in discarded section ...
  //
  const char* errorCodeAsString(int errorCode) const
  {
    switch (errorCode)
    {
      case WORD_ID_OUT_OF_RANGE          : return "WORD ID OUT OF RANGE";
      case SINGLE_STAR_NOT_ALLOWED       : return "SINGLE * NOT ALLOWED AS QUERY PART";
      case CONTAINS_INVALID_SEPARATOR    : return "WRONG QUERY FORMATTING: Invalid separator found";
      case ENHANCED_PART_TOO_SHORT       : return "WRONG QUERY FORMATTING: Enhanced part too short";
      case QUERY_TOO_SHORT               : return "WRONG QUERY FORMATTING: Query too short";
      case OR_PART_TOO_SHORT             : return "WRONG QUERY FORMATTING: Or part too short";
      case BAD_REQUEST                   : return "BAD REQUEST STRING";
      case BAD_QUERY                     : return "BAD QUERY FORMAT";
      case REALLOC_FAILED                : return "MEMORY ALLOCATION ERROR: Realloc failed";
      case NEW_FAILED                    : return "MEMORY ALLOCATION ERROR: new failed";
      case ODD_LIST_LENGTH_FOR_TAG       : return "WRONG LIST FOR INTERSECTION: Got an odd list length when intersecting ``pairs''";
      case DOCS_NOT_PAIRED_FOR_TAG       : return "WRONG LIST FOR INTERSECTION: Docs were not paired when intersecting ``pairs''";
      case ERROR_PASSED_ON               : return "PASSING ON ERROR";
      case HISTORY_ENTRY_CONFLICT        : return "HISTORY EXCEPTION: History entry already exists as under construction";
      case HISTORY_ENTRY_NOT_FOUND       : return "HISTORY ENTRY NOT FOUND";
      case BAD_HISTORY_ENTRY             : return "BAD HISTORY ENTRY";
      case BAD_QUERY_RESULT              : return "QUERY EXCEPTION: check of query result failed";
      case UNCOMPRESS_ERROR              : return "UNCOMPRESSION PROBLEM";
      case COULD_NOT_GET_MUTEX           : return "MUTEX EXCEPTION: Could not get lock on mutex";
      case RESULT_LOCKED_FOR_READING     : return "LOCK PROBLEM: QueryResult was locked for reading, and can't be written";
      case RESULT_LOCKED_FOR_WRITING     : return "LOCK PROBLEM: QueryResult was locked for writing, and can't be read";
      case RESULT_NOT_LOCKED_FOR_READING : return "LOCK PROBLEM: QueryResult was not locked for reading, but should be, as it's being read)";
      case RESULT_NOT_LOCKED_FOR_WRITING : return "LOCK PROBLEM: QueryResult was not locked for writing, but should still be";
      case COULD_NOT_CREATE_THREAD       : return "PTHREADS: error creating thread";
      case COULD_NOT_CREATE_SOCKET       : return "SOCKET ERROR: could not create socket";
      case ASSERT_FAILED                 : return "ASSERT FAILED";
      case ZLIB_ERROR                    : return "ERROR USING ZLIB";
      case NOT_YET_IMPLEMENTED           : return "NOT YET IMPLEMENTED";
      case INVALID_PARAMETER_VALUE       : return "INVALID PARAMETER VALUE";
      case CHECK_FAILED                  : return "CHECK FAILED";
      case QUERY_TIMEOUT                 : return "PROCESSING OF QUERY TOOK TOO LONG";
      case OTHER                         : return "ERROR";
      default: ostringstream os; os << "UNKNOWN ERROR: Code is " << errorCode;  return os.str().c_str();
    } 
  } 

  //! Constructor (code only)
  Exception(int errorCode) { _errorCode = errorCode; _errorDetails = ""; }
  
  //! Constructor (code + details)
  Exception(int errorCode, string errorDetails) { _errorCode = errorCode; _errorDetails = errorDetails; }

  //! Constructor (code + details + file name + line number + enclosing method)
  Exception(int errorCode, string errorDetails, const char* file_name, int line_no, const char* fct_name) 
  {
    _errorCode    = errorCode; 
    ostringstream os;
    if (errorDetails.size() > 0) os << errorDetails << "; ";
    os << "in " << file_name << ", line " << line_no << ", function " << fct_name;
    _errorDetails = os.str();
  }

  //! Set error code
  void setErrorCode(int errorCode) { _errorCode = errorCode; }

  //! Set error details
  void setErrorDetails(string errorDetails) { _errorDetails = _errorDetails; }

  //! Get error Code
  int getErrorCode() const { return _errorCode; }
  
  //! Get error message pertaining to code
  string getErrorMessage() const { return errorCodeAsString(_errorCode); }

  //! Get error details
  string getErrorDetails() const { return _errorDetails; }

  //! Get full error message (generic message + specific details if available)
  string getFullErrorMessage() const
  {
    return _errorDetails.length() > 0 
            ? errorCodeAsString() + " (" + _errorDetails + ")"
		: errorCodeAsString();
  }

  //! DEPRECATED
  string errorMessage() const { return string("! ") + getFullErrorMessage(); }
  string errorCodeAsString() const { return getErrorMessage(); }

};

#endif
