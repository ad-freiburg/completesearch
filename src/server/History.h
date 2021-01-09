#ifndef __HISTORY_H__
#define __HISTORY_H__

#include <pthread.h>
#include "Globals.h" 
#include "QueryResult.h"
#include <string>
#include <vector>
#include <unordered_set>

#define MUTEX_TIMEOUT 100000 // in microseconds

#define DONT_LOCK false
#define DO_LOCK true


using std::string;
using std::vector;
using std::unordered_map;
typedef std::unordered_map<std::string, QueryResult, StringHashFunction> QueryResultMap;

// TODO: Have bool 'lock' parameter for all method calls which employ mutexes (to avoid double-locking)
// TODO: After a lock is obtained check/assert the state

//! History / Cache of already computed results.
/*   
 *    Essential for fast processing of search-as-you-type queries by filtering
 *    from previously computed result (e.g. compute result for "schedul" by
 *    filtering from result for "schedu")
 *
 *    Useful as a caching mechanism for recent / frequent queries. TODO: what is
 *    the current eviction strategy, i.e. which queries are thrown out when the
 *    history has grown too large.
 *
 *    Multiple threads accessing the same object will be serialized. TODO: still
 *    a bit rough though.
 */
class History
{
 protected:

    //! Results stored by query string
    QueryResultMap _results;

    //! Current total size of the results stored, in bytes
    size_t _currentSize;

    //! The query strings and the corresponding result size, in a vector.
    vector<pair<string, size_t> > _queries;

    //! Keep results for these queries in history (= don't remove in cut down).
    unordered_set<string, StringHashFunction> _keepInHistoryQueries;

    //! Mutex for serializing history change operations
    mutable pthread_mutex_t history_change;

  public:

    // Dump object as string.
    string asString(void) const;

    // NEW(hagn, 28Jan11):
    //! History Flags (If the query string is not enough to use it as an id)
    /*
     *   10-15: used when howToRankDocs == QueryParameters::RANK_DOCS_BY_COMPLETION_SCORES
     *      27: used when howToRankDocs == QueryParameters::RANK_DOCS_BY_FUZZY_SCORE
     */
    enum ExtraHistoryFlag { 
      HISTORY_FLAG_STD                        =  0,
      HISTORY_FLAG_RANK_DOCS_BY_SCORE         = 10,
      HISTORY_FLAG_RANK_DOCS_BY_DOC_COUNT     = 11,
      HISTORY_FLAG_RANK_DOCS_BY_OCC_COUNT     = 12,
      HISTORY_FLAG_RANK_DOCS_BY_WORD_ID       = 13,
      HISTORY_FLAG_RANK_DOCS_BY_DOC_ID        = 14,
      HISTORY_FLAG_RANK_DOCS_BY_EDIT_DISTANCE = 15,
      HISTORY_FLAG_RANK_DOCS_BY_FUZZY_SCORE   = 27
    };

    //! Constructor; creates empty history.
    History();

    //! Virtual destructor that does nothing.
    virtual ~History() { }

    //! Get current number of queries
    unsigned int getNofQueries() const { return _queries.size(); }

    //! Get current total size (in bytes) of results in history
    size_t sizeInBytes(bool doLock = true) const;

    //! Remove old queries if history size is above given threshold; return true iff indeed queries removed.
    /*
     *   Note: queries from _stringsToKeep (defined in Globals, TODO: why?),
     *   like "ct:author:*" are never removed, so history may be larger than
     *   specified size even after the call
     */
    bool cutToSizeAndNumber(size_t maxSizeInBytes, unsigned int maxNofQueries, bool doLock = true);

    //! Add query to keepInHistoryQueries. Will add query with flags &hf=0 and
    //&hf=1.
    void addKeepInHistoryQuery(string query);

    //! Ask if given string (query + hf flag) is in keepInHistoryQueries.
    bool keepInHistory(string query) const;

    //! Get number of keepInHistoryQueries.
    size_t getNofKeepInHistoryQueries() const {
      return _keepInHistoryQueries.size();
    }
  
    //! TODO: in which way does this method "clean up"?
    bool cleanUp(bool doLock = true);

    //! CHECK WHETHER HISTORY IS IN CONSISTENT STATE
    /*
     *    for each query in vector _queries check that it is the hash_map
     *    _results
     *
     *    if checkEntryStatus == true: also check that statuses are
     *    IS_FINISHED return true if everything is ok, false otherwise
     *
     *    TODO: why is this function necessary?
     *
     *    TODO (formulated by Ingmar, I don't understand it): add checks for
     *    dirty/finished flags here
     */
    bool check(bool doLock = true, bool checkEntryStatus = false) const;

    //! Remove all queries; TODO: does not remove them from the hash map!?
    void clear(bool doLock = true);

    //! Show query string and status of all entries.
    void show(bool doLock = true) const;

    //! TODO: explain
    void finalizeSize(const std::string& key, bool doLock = true);

    //! Add a result. 
    void add(const std::string& key, const QueryResult& result, bool doLock = true);

    //! Remove entry for given query; TODO: correct?
    bool remove(std::string key, bool doLock = true);

    //! Get size of result for given query, in bytes.
    size_t sizeOfEntry(const std::string& key, bool doLock = true) const;

    //! Get pointer to result for given query or NULL if not in history
    QueryResult* isContained(const std::string& key, bool doLock = true);

    //! Get pointer to result for given query or NULL if not in history, CONST version (TODO: why?)
    const QueryResult* isContainedConst(const std::string& key, bool doLock = true) const;

    //! Get status of result for given query (IS_FINISHED, UNDER_CONSTRUCTION, etc.)
    int getStatusOfEntry(const std::string& key, bool doLock = true) const;

    //! Set status of result for given query (IS_FINISHED, UNDER_CONSTRUCTION, etc.)
    void setStatusOfEntry(const std::string& key, int status, bool doLock = true);

    //! Get result for given query if in history; same functionality as above with different interface; TODO: why?
    /*!
     *  Returns \c true if key is found and \c false otherwise.
     *  In case \a key is found, \a result is set to the data stored under that key.
     *  Otherwise, result remains unchanged.
     *
     *    Note (from Ingmar): not 'const' due to timer. TODO: why not simply make the timer mutable then?
     */
    bool isContained(const std::string& key, QueryResult*& result, bool doLock = true);

};  // end of History class


class TimedHistory : public History
{
 private:
  Timer    _timer;

 public:

  void resetTimer(void) { _timer.reset(); }
  const Timer& getTimer(void) const { return _timer; }

  bool cutToSizeAndNumber(size_t maxInBytes, unsigned int maxNofQueries)
  {
    bool retval;
    _timer.cont();
    retval = History::cutToSizeAndNumber(maxInBytes, maxNofQueries);
    _timer.stop();
    return retval;
  }

  bool cleanUp(void)
  {
    bool retval;
    _timer.cont();
    retval = History::cleanUp();
    _timer.stop();
    return retval;
  }

  void clear(void)
  {
    _timer.cont();
    History::clear();
    _timer.stop();
  }
};

#endif
