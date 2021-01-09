#include "History.h"

//! Constructor; creates empty history.
History::History() : _currentSize(0)
{
  pthread_mutex_init(&history_change, NULL);
  assert(&history_change);
}


//! Get current total size (in bytes) of results in history
size_t History::sizeInBytes(bool doLock) const
{
  // LOCK MUTEX
  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
      cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
      throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method sizeInBytes");
      //                  pthread_exit(NULL);
    }
  }
  size_t currentSize = _currentSize;
  // UNLOCK MUTEX
  if (doLock) pthread_mutex_unlock(&history_change); 
  return currentSize;
}


//! Show query string and status of all entries
void History::show(bool doLock) const
{
  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change, MUTEX_TIMEOUT ))
    {
	  ostringstream os;
      os << "timeout = " << timeAsString(MUTEX_TIMEOUT);
      CS_THROW(Exception::COULD_NOT_GET_MUTEX, os.str());
    }
  }

  for (unsigned int i = 0; i < _queries.size(); ++i)
  {
    cout << "History entry #" << (i+1) << " : query = " << _queries[i].first 
	     << ", status = " << int(getStatusOfEntry(_queries[i].first, DONT_LOCK)) << endl;
  }

  if (doLock) pthread_mutex_unlock(&history_change);
}

//! Add query to keepInHistoryQueries. Will add query with flags &hf=0 and
//&hf=1.
void History::addKeepInHistoryQuery(string query) {
  _keepInHistoryQueries.insert(query + "&hf=0");
  _keepInHistoryQueries.insert(query + "&hf=1");
}

//! Ask if given string (query + hf flag) is in keepInHistoryQueries.
bool History::keepInHistory(string queryWithFlag) const {
  return _keepInHistoryQueries.count(queryWithFlag) > 0;
}

//! TODO: in which way does this method "clean up"?
bool History::cleanUp(bool doLock)
{
  #ifndef NDEBUG
  cout << " In cleanUp (History) "<< endl;
  #endif
  assert(check(DONT_LOCK));

  // only one thread at a time
  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
	  ostringstream os;
      os << "timeout = " << timeAsString(MUTEX_TIMEOUT);
      CS_THROW(Exception::COULD_NOT_GET_MUTEX, os.str());
    }
  }


  //
  // 1. Find all entries in _results without query string in _queries, and
  //    remove them from _results; TODO: it's quadratic!!!
  //
  
  #ifndef NDEBUG
  cout << "checking all entries (even if not in _queries[i])" << endl;
  #endif
  vector<string> stringsToRemove;
  // TODO: Wie bitte??? Für jeden query in der hash_map in allen queries im
  // vector nachgucken? Das ist quadratisch (und ich wundere mich wo die Zeit
  // hingeht). Das kann man doch ganz einfach andersrum machen, oder nicht?
  for(QueryResultMap::iterator it = _results.begin(); it != _results.end(); ++it)
  {
    bool found = false;
    for (unsigned int i = 0; i < _queries.size(); ++i)
    {
      if (_queries[i].first == it->first)
      {
        found = true;
        break;
      }                   
    }
    if (!found)
    {
      #ifndef NDEBUG
      cout << " marking for removal this history entry"
              " as it has no _queries[i] correspondence ... " 
           << it->first << flush;
      #endif
      stringsToRemove.push_back(it->first);

      #ifndef NDEBUG
      cout << " ... done " << endl;
      #endif
    }
  }
  #ifndef NDEBUG
  cout << "done with checking all entries, doing the rest now" << endl;
  cout << " removing marked entries " << endl;
  #endif
  for (unsigned int i = 0; i < stringsToRemove.size(); ++i)
  {
    _results.erase(stringsToRemove[i]);
  }


  //
  // 2. Now remove all entries from _queries which are not contained in
  //    _results, and remove them from _queries; TODO: quadratic too!!!
  //
  
  #ifndef NDEBUG
  cout << " done removing marked entries, doing the rest now (iterate over _queries[i])" << endl;
  #endif
  unsigned int nofTriesLeft = _queries.size();
  unsigned int i = 0;
  while (nofTriesLeft > 0)
  {
    assert(_queries.size() > 0);
    assert(i < _queries.size());
    #ifndef NDEBUG
    cout << "Clean up: Inspecting entry (iterating over _queries[i]): " << _queries[i].first << endl;
    #endif
    if (isContainedConst(_queries[i].first, DONT_LOCK) == NULL)
    {
      #ifndef NDEBUG
      cout << "Clean up: Removing string '" << _queries[i].first << "' from history (as only in _queries[i] and not with data)" << endl;
      #endif
      _queries.erase(_queries.begin() + i);
      continue;
    }

    // check here if first query has size 0
    if (isContainedConst(_queries[i].first, DONT_LOCK)->check() == false)
    {
      #ifndef NDEBUG
      cout << "Clean up: Removing string '" << _queries[i].first << "' from history (as it did not pass the check)" << endl;
      #endif
      if (remove(_queries[i].first, DONT_LOCK) == false) ++i;
    }
    else if (getStatusOfEntry(_queries[i].first, DONT_LOCK) & QueryResult::UNDER_CONSTRUCTION)
    {
      #ifndef NDEBUG
      cout << "Clean up: Removing string '" << _queries[i].first << "' from history (as it is marked as under construction)" << endl;
      #endif
      if (remove(_queries[i].first, DONT_LOCK) == false) ++i;
    }
    else if (getStatusOfEntry(_queries[i].first, DONT_LOCK) & QueryResult::IN_USE)
    {
      #ifndef NDEBUG
      cout << "Clean up: Resetting status of " << _queries[i].first << " to a simple IS_FINISHED" << endl;
      #endif
      setStatusOfEntry(_queries[i].first, QueryResult::FINISHED, DONT_LOCK);
      assert(getStatusOfEntry(_queries[i].first, DONT_LOCK) == QueryResult::FINISHED);
      ++i;
    }
    else 
	{
	  ++i;
	}
    --nofTriesLeft;
  } // end: while nofTries


  //
  // 3. Was soll das denn jetzt noch?
  //

  #ifndef NDEBUG
  cout << "Clean up: Starting to fix/check the size (and unlock all entries for reading/writing) " << endl;
  #endif
  size_t accounted_size = 0;
  // Iterates of _queries and NOT over keys of hash (which in multithreaded case could be different)
  for (unsigned int i = 0; i < _queries.size(); i++)
  {
	// Wieso jetzt nochmal?
    if (isContainedConst(_queries[i].first, DONT_LOCK) == NULL)
    {
      #ifndef NDEBUG
      cout << "! History check failed in cleanUp: Did not find entry " << _queries[i].first << endl;
      #endif
      if(doLock) { pthread_mutex_unlock(&history_change); }
      return false;
    }
    assert(!keepInHistory(_queries[i].first) || _queries[i].second == 0);

    if(!(isContainedConst(_queries[i].first, DONT_LOCK)->_status & QueryResult::FINISHED))
    {
      #ifndef NDEBUG
      cout << "! History check failedin cleanUp: Found entry " << _queries[i].first << " with status : " << int(isContainedConst(_queries[i].first, DONT_LOCK)->_status) << endl;
      #endif
      if(doLock) { pthread_mutex_unlock(&history_change); }
      return false;
    }     
    isContainedConst(_queries[i].first, DONT_LOCK)->unlock();         
    accounted_size += _queries[i].second;
  }

  if (_currentSize != accounted_size)
  {
    #ifndef NDEBUG
    cout << "Clean up: Fixing the size now " << endl;
    #endif
    _currentSize = accounted_size;
  }

  #ifndef NDEBUG
  cout << "Clean up: Done fixing/checking the size " << endl;
  #endif

  if (doLock) pthread_mutex_unlock(&history_change); 
  return true;
}



//! Remove old _queries if history size is above given threshold; return true iff indeed _queries removed.
/*
 *   Note: _queries from _stringsToKeep (defined in Globals, TODO: why?),
 *   like "ct:author:*" are never removed, so history may be larger than
 *   specified size even after the call
 */
bool History::cutToSizeAndNumber(size_t maxSizeInBytes, unsigned int maxNofQueries, bool doLock)
{
  assert(check(DONT_LOCK));
  // if below both thresholds (size and number) nothing to do
  if (sizeInBytes(DONT_LOCK) <= maxSizeInBytes && _queries.size() <= maxNofQueries) return false;

  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
	  ostringstream os;
      os << "timeout = " << timeAsString(MUTEX_TIMEOUT);
      CS_THROW(Exception::COULD_NOT_GET_MUTEX, os.str());
    }
  }

  unsigned int nofTriesLeft = _queries.size();
  unsigned int i = 0;
  while (nofTriesLeft > 0 && (sizeInBytes(DONT_LOCK) > maxSizeInBytes || _queries.size() > maxNofQueries))
  {
    assert(_queries.size() > 0);
    assert(i < _queries.size());
    assert(isContainedConst(_queries[i].first, DONT_LOCK));
    // Case 1: to-be-kept query string -> move to end
    if (keepInHistory(_queries[i].first))
    {
      // cout << "NOT removing string '" << _queries[i].first << "' from history (special string)" << endl;
      LOG << "NOT removing string '" << _queries[i].first << "' from history (special string)" << endl;
      // if it has size zero do NOT remove but simply erase teh query string and 
      // append it to the end
      assert(_queries[i].second == 0);
      string specialString = _queries[i].first;
      // TODO: CHECK THAT THE KEY IS NOT UNDER CONSTRUCTION
      _queries.erase(_queries.begin());
      _queries.push_back(pair<string, size_t > (specialString, 0));
      assert(check(DONT_LOCK));
    }
	// Case 2: ordinary query -> just remove the result
    else
    {
      // cout << "Removing string '" << _queries[i].first << "' from history " << endl;
      LOG << IF_VERBOSITY_HIGH << "Removing string '"
          << _queries[i].first << "' from history " << endl;
      if (remove(_queries[i].first, DONT_LOCK) == false) ++i;
    }
    assert(((sizeInBytes(DONT_LOCK) > 0) && ( _queries.size() > 0) && (check(DONT_LOCK))) || (sizeInBytes(DONT_LOCK) == 0));
    --nofTriesLeft;
  } // end: while size ...
  if (doLock) pthread_mutex_unlock(&history_change); 
  return true;
}


//! CHECK WHETHER HISTORY IS IN CONSISTENT STATE
/*
 *    for each query (in _queries) check that it is the hash_map _results
 *    if alsoCheckEntryStatus: also check that statuses are IS_FINISHED 
 *    return true if everything is ok, false otherwise
 *
 *    TODO: why is this function necessary?
 *
 *    TODO (formulated by Ingmar, I don't understand it): add checks for
 *    dirty/finished flags here
 */
bool History::check(bool doLock, bool alsoCheckEntryStatus) const
{
  size_t accounted_size = 0;
  if (doLock && pthread_mutex_timed_trylock(&history_change, MUTEX_TIMEOUT))
  {
    cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
    throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method check(..)");
    //pthread_exit(NULL);
  }

  // Iterates of _queries and NOT over keys of hash (which in multithreaded case could be different)
  // Holger wonders: what's the difference?
  for (unsigned int i = 0; i < _queries.size(); i++)
  {
    // check whether query is in history hash_map
    if (!isContainedConst(_queries[i].first, DONT_LOCK))
    {
      #ifndef NDEBUG
      cout << "! History check failed: Did not find entry " << _queries[i].first << endl;
      #endif
      if (doLock) pthread_mutex_unlock(&history_change); 
      return false;
    }
    assert(!keepInHistory(_queries[i].first) || _queries[i].second == 0);

    // check whether status is IS_FINISHED (conditional on second arg)
    if ((alsoCheckEntryStatus) && !(isContainedConst(_queries[i].first, DONT_LOCK)->_status & QueryResult::FINISHED))
    {
      #ifndef NDEBUG
      cout << "! History check failed: Found entry " << _queries[i].first << " with status : " 
        << int(isContainedConst(_queries[i].first, DONT_LOCK)->_status) << endl;
      #endif
      if (doLock) { pthread_mutex_unlock(&history_change); }
      return false;
    }              
    accounted_size += _queries[i].second;
  }

  #ifndef NDEBUG
  size_t currentSize = _currentSize;
  if (currentSize != accounted_size) cerr << "! History check failed: size misMatch. Will be fixed later. " << endl;   
  #endif
  if(doLock) pthread_mutex_unlock(&history_change); 
  return true;
}


//! Remove all _queries; TODO: does not remove them from the hash map!?
void History::clear(bool doLock)
{
  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
	  ostringstream os;
      os << "timeout = " << timeAsString(MUTEX_TIMEOUT);
      CS_THROW(Exception::COULD_NOT_GET_MUTEX, os.str());
    }
  }
  _results.clear(); 
  _queries.clear();
  _currentSize = 0;
  if(doLock) { pthread_mutex_unlock(&history_change); }
}

// TODO: CURRENTLY THE STRING IS ADDED TO THE LIST OF QUERIES HERE
// Currently (20Jul2006) never locks (as it is assumed that it's already locked)
void History::finalizeSize(const std::string& key, bool doLock)
{
  // NEW (baumgari) 06Mar13: See CompleterBase:237 8Feb13
  // NEW (baumgari) 18Apr13: This leads to the problem, that e.g. quer* =
  // "quer*", but that's not right! In the first version * should be
  // as quer with all its completions and the seconds one means quer* and
  // nothing else.
  // std::string key = decodeHexNumbers(keyOriginal);

  // LOCK MUTEX
  if (doLock)
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
      cout << endl << " ERROR: Could not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
      throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method finalizeSize");
      //                pthread_exit(NULL);
    }
  assert(check(DONT_LOCK));
  assert(isContainedConst(key, DONT_LOCK));
  //! Add the query string and its size
  assert(isContainedConst(key, DONT_LOCK)->_status == QueryResult::UNDER_CONSTRUCTION ); 


  #ifndef NDEBUG
  for (unsigned int i=0; i<_queries.size(); i++)
  {
    // either a new query, or an 'extended' (more hits) query
    assert((_queries[i].first != key ) || (_queries[i].second < isContainedConst(key, DONT_LOCK)->sizeInBytes() ));
  }
  #endif

  bool stringFound = false;
  for (unsigned int i=0; i<_queries.size(); i++)
  {
    if (_queries[i].first == key)
    {
      if (!keepInHistory(key))
      {
        assert(isContainedConst(key, DONT_LOCK)->sizeInBytes() - _queries[i].second > 0);
        _currentSize += (isContainedConst(key, DONT_LOCK)->sizeInBytes() - _queries[i].second) ;
        _queries[i].second = isContainedConst(key, DONT_LOCK)->sizeInBytes() ;
      }
      stringFound = true;
      break;
    }
  }

  if (!stringFound)
  {
    if (keepInHistory(key))
    {
      _queries.push_back( pair<std::string, size_t>(key, 0) );
    }
    else
    {
      _queries.push_back( pair<std::string, size_t>(key, isContainedConst(key, DONT_LOCK)->sizeInBytes()) );

    }
    _currentSize += _queries.back().second;
    assert(keepInHistory(key) || ( isContainedConst(key, DONT_LOCK)->sizeInBytes() == _queries.back().second ));
  }

  assert(check(DONT_LOCK));
  // UNLOCK MUTEX
  if (doLock) {pthread_mutex_unlock(&history_change);}
}

void History::add(const std::string& key, const QueryResult& result, bool doLock)
{
  // NEW (baumgari) 06Mar13: See CompleterBase:237 8Feb13
  // NEW (baumgari) 18Apr13: This leads to the problem, that e.g. quer* =
  // "quer*", but that's not right! In the first version * should be
  // as quer with all its completions and the seconds one means quer* and
  // nothing else.
  // std::string key = decodeHexNumbers(keyOriginal);

  #ifndef NDEBUG
  cout << "! Trying to add string \"" << key << "\" to history" << endl << flush;
  #endif

  //      historyTimer.cont();
  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
      cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
      throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method add(..)");
      //               pthread_exit(NULL);
    }
  }
  if(isContainedConst(key, DONT_LOCK))
  {
    if(doLock) { pthread_mutex_unlock(&history_change); }
    CS_THROW(Exception::HISTORY_ENTRY_CONFLICT, string("key: ") + key);
  }
  assert(!isContainedConst(key, DONT_LOCK));
  assert(result._lastBlockScores.size() == 0); //currently always adding an empty element to history, which is then filled
  assert(( result._lastBlockScores.size() ==  result._topDocIds.size()) || (   result._lastBlockScores.size() == 0));
  assert(( result._lastBlockScores.size() ==  0) || (   result._lastBlockScores.isPositive()));
  assert(result._docIds.size() == result._wordIds.size());
  assert(result._topDocIds.size() <=  result._docIds.size());
  assert(result._topWordIds.size() <= result._wordIds.size() );
  assert(((result._topDocIds.size()>0  )&&( result._topWordIds.size()>0  ))  ||  (( result._topDocIds.size()==0  ) && (result._topWordIds.size()==0  )));
  // TODO: CHECK THAT THE KEY IS NOT CONTAINED/IS NOT UNDER CONSTRUCTION
  // This also calls the copy constructor
  if (_results.find(key) != _results.end())
  {
    if(doLock) { pthread_mutex_unlock(&history_change); }
    CS_THROW(Exception::HISTORY_ENTRY_CONFLICT, string("key: ") + key);
  }
  _results[key] = result; // here an initially empty object is copied (as result will be empty)
  assert ( _results.find(key) != _results.end() );
  assert(isContainedConst(key,DONT_LOCK));
  if (doLock) { pthread_mutex_unlock(&history_change); }

  #ifndef NDEBUG
  cout << "! Added string \"" << key << "\" to history" << endl << flush;
  #endif
  //      historyTimer.stop();
}// end: add(...)


bool History::remove(std::string key, bool doLock)
{
  #ifndef NDEBUG
  cout << "! Trying to remove string \"" << key << "\" from history" << endl << flush;
  #endif

  if (doLock)
  {
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
      cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
      throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method remove");
      //                pthread_exit(NULL);
    }
  }

  assert(isContainedConst(key, DONT_LOCK));
  if (isContainedConst(key, DONT_LOCK)->_status & QueryResult::IN_USE)
  {
    assert(isContainedConst(key, DONT_LOCK)->_status & QueryResult::FINISHED);
    cout << "! NOT removing string '" << key << "' from history, as it is being used !" << endl;
    if (doLock) { pthread_mutex_unlock(&history_change); }
    return false;
  }

  assert(check(DONT_LOCK));

  // assert(isContainedConst(key, DONT_LOCK)->_status == IS_FINISHED); // when using errorStatus also removing incomplete entries
  //      historyTimer.cont();
  //          if (isContainedConst(key, DONT_LOCK)->_status == IS_FINISHED)
  if (true)
  {
    //find string in vector of _queries
    signed int string_index = -1;
    for (unsigned int i=0; i<_queries.size(); ++i)
    {
      if (_queries[i].first == key)
      {
        string_index = i;
        break;
      }
    }
    if (string_index < 0)
    {
      // can happen if result is/was UNDER_CONSTRUCTION. Not part of the list of _queries then
      #ifndef NDEBUG
      cout << "Error in History (remove): Looking for string (not in _queries[i]), only removing the data (if found) .. " << key << flush;
      #endif
      if (_results.find(key) != _results.end())
      {
        #ifndef NDEBUG
        cout << " .. found! " << endl;
        #endif
        _results.erase(key);
      }
      else
      {
        #ifndef NDEBUG
        cout << " .. NOT found! " << endl;
        #endif
      }
      if (doLock) { pthread_mutex_unlock(&history_change); }
      return true;
    }
    //               #endif
    //              assert(string_index >= 0);

    #ifndef NDEBUG
    unsigned int nofElementsBefore = _queries.size();
    #endif
    if (string_index >=0)
    {
      assert(_queries[string_index].first == key);
      _queries.erase(_queries.begin() + string_index );
      assert(_queries.size() == (nofElementsBefore -1));

      assert(((unsigned int) string_index == _queries.size()  ) || (_queries[string_index].first != key));
    }
    //// only decrease size if not a special query term
    _currentSize -=  isContainedConst(key, DONT_LOCK)->sizeInBytes();
    assert(_currentSize >= 0);
    assert(isContainedConst(key, DONT_LOCK));
  } // end: is contained with status IS_FINISHED (and so we have to remove the query string from the history)
  if(_results.find(key) == _results.end())
  {
    if (doLock) { pthread_mutex_unlock(&history_change); }
    CS_THROW(Exception::HISTORY_ENTRY_NOT_FOUND, string("key: ") + key);
  }
  else
  {
    #ifndef NDEBUG
    cout << "! Removing entry for key " << key << " from history" << endl;
    #endif
  }
  _results.erase(key);
  assert(!isContainedConst(key, DONT_LOCK));
  assert(check(DONT_LOCK));
  //      historyTimer.stop();
  if (doLock) { pthread_mutex_unlock(&history_change); }
  return true;
}

size_t History::sizeOfEntry(const std::string& key, bool doLock) const
{ 
  // NEW (baumgari) 06Mar13: See CompleterBase.cpp:237 8Feb13
  // NEW (baumgari) 18Apr13: See History.cpp:433
  //std::string key = decodeHexNumbers(keyOriginal);

  assert(isContainedConst(key, DONT_LOCK));
  if (doLock)
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
      cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
      throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method sizeOfEntry");
      //             pthread_exit(NULL);
    }
  assert(isContainedConst(key, DONT_LOCK)->_status & QueryResult::FINISHED);
  const size_t sizeOfEntry_cpy = isContainedConst(key, DONT_LOCK)->sizeInBytes();
  if (doLock) { pthread_mutex_unlock(&history_change); }
  return sizeOfEntry_cpy;
}

QueryResult* History::isContained(const std::string& key, bool doLock)
{
  // NEW (baumgari) 06Mar13: See CompleterBase.cpp:237 8Feb13
  // NEW (baumgari) 18Apr13: See History.cpp:433
  //std::string key = decodeHexNumbers(keyOriginal);

  //      historyTimer.cont();
  if (doLock)
    if (pthread_mutex_timed_trylock(&history_change , MUTEX_TIMEOUT ))
    {
      cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
      throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method isContained");
      //             pthread_exit(NULL);
    }
  QueryResultMap::iterator it = _results.find(key); // do NOT use a const_iterator here
  if (doLock) { pthread_mutex_unlock(&history_change); }
  if (it == _results.end())
  {
    //          historyTimer.stop();
    return NULL;
  }
  else
  {
    //          historyTimer.stop();
    return &(it->second);
  }
}



//! GET STATUS OF A HISTORY ENTRY (does not exist, under construction, is finished, is being used)
int History::getStatusOfEntry(const std::string& key, bool doLock) const
{
  // NEW (baumgari) 06Mar13: See CompleterBase.cpp:237 8Feb13
  // NEW (baumgari) 18Apr13: See History.cpp:433
  //std::string key = decodeHexNumbers(keyOriginal);

  if (doLock && pthread_mutex_timed_trylock(&history_change, MUTEX_TIMEOUT))
  {
    cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
    throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method getStatusOfEntry");
  }

  QueryResultMap::const_iterator it = _results.find(key); // use a const_iterator here
  if (it == _results.end())
  {
    if (doLock) pthread_mutex_unlock(&history_change); 
    return QueryResult::DOES_NOT_EXIST;
  }
  else
  {
    unsigned char status = (it->second)._status;
    if (doLock) pthread_mutex_unlock(&history_change); 
    return status;
  }
}


//! SET STATUS OF HISTORY ENTRY (does not exist, under construction, is finished, is being used)
void History::setStatusOfEntry(const std::string& key, int status, bool doLock)
{
  assert((status == QueryResult::UNDER_CONSTRUCTION) || (status & QueryResult::FINISHED));
  assert(!((status & QueryResult::UNDER_CONSTRUCTION) && (status & QueryResult::FINISHED)));
  assert(!((status & QueryResult::UNDER_CONSTRUCTION) && (status & QueryResult::IN_USE)));

  // NEW (baumgari) 06Mar13: See CompleterBase.cpp:237 8Feb13
  // NEW (baumgari) 18Apr13: See History.cpp:433
  //std::string key = decodeHexNumbers(keyOriginal);

  if (doLock && pthread_mutex_timed_trylock(&history_change, MUTEX_TIMEOUT))
  {
    cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
    throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method setStatusOfEntry");
  }

  QueryResultMap::iterator it = _results.find(key); // use a const_iterator here

  // CASE: entry does not exist
  if (it == _results.end())
  {
    if (doLock) pthread_mutex_unlock(&history_change); 
    CS_THROW(Exception::HISTORY_ENTRY_NOT_FOUND, string("key: ") + key);
  }

  // CASE: setting to "under construction" when this bit is already set
  assert(it != _results.end());
  if(!((!(status & QueryResult::UNDER_CONSTRUCTION)) || (!((it->second)._status & QueryResult::UNDER_CONSTRUCTION ))))
  {
    if (doLock) pthread_mutex_unlock(&history_change); 
    CS_THROW(Exception::BAD_HISTORY_ENTRY, string("key: ") + key);
  }

  // CASE: setting to "is being used" when still "under construction"
  if(!((!(status & QueryResult::IN_USE)) || (!((it->second)._status & QueryResult::UNDER_CONSTRUCTION ))))
  {
    if(doLock) pthread_mutex_unlock(&history_change); 
    CS_THROW(Exception::BAD_HISTORY_ENTRY, string("key: ") + key);
  }

  assert((!(status & QueryResult::UNDER_CONSTRUCTION)) || (!((it->second)._status & QueryResult::UNDER_CONSTRUCTION )));
  assert((!(status & QueryResult::IN_USE)) || (!((it->second)._status & QueryResult::UNDER_CONSTRUCTION )));

  // OTHERWISE: DO SET THE STATUS 
  (it->second)._status = (QueryResult::StatusEnum)(status);

  if (doLock) pthread_mutex_unlock(&history_change); 
}


//! FIND GIVEN QUERY IN HISTORY (and return result or NULL if query not in history)
const QueryResult* History::isContainedConst(const std::string& key, bool doLock) const
{
  // NEW (baumgari) 06Mar13: See CompleterBase.cpp:237 8Feb13
  // NEW (baumgari) 18Apr13: See History.cpp:433
  //std::string key = decodeHexNumbers(keyOriginal);

  //historyTimer.cont();
  if (doLock && pthread_mutex_timed_trylock(&history_change, MUTEX_TIMEOUT))
  {
    cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
    throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method isContainedConst");
    //pthread_exit(NULL);
  }
  QueryResultMap::const_iterator it = _results.find(key); // use a const_iterator here
  if (doLock) pthread_mutex_unlock(&history_change); 
  //historyTimer.stop();
  return it == _results.end() ? NULL : &(it->second);
}


//! SAME AS ABOVE, but not const (TODO: is it still needed somewhere?)
bool History::isContained(const std::string& key, QueryResult*& result, bool doLock)
{
  assert((result == NULL)||(result->nofTotalHits >= result->_topDocIds.size() ));
  //historyTimer.cont();

  // NEW (baumgari) 06Mar13: See CompleterBase.cpp:237 8Feb13
  // NEW (baumgari) 18Apr13: See History.cpp:433
  //std::string key = decodeHexNumbers(keyOriginal);

  if (doLock && pthread_mutex_timed_trylock(&history_change, MUTEX_TIMEOUT))
  {
    cout << endl << " ERROR: Cut not get lock on history_change mutex within " << timeAsString(MUTEX_TIMEOUT) << endl << flush;
    throw Exception(Exception::COULD_NOT_GET_MUTEX, "in method isContained(key, result, bool)");
    //pthread_exit(NULL);
  }
  QueryResultMap::iterator it = _results.find(key);
  if (it == _results.end())
  {
    //historyTimer.stop();
    if (doLock) pthread_mutex_unlock(&history_change); 
    return false;
  }
  else
  {
    result = &(it->second);
    //historyTimer.stop();
    if (doLock) pthread_mutex_unlock(&history_change); 
    return true;
  }
} 

// _____________________________________________________________________________
std::string History::asString(void) const
{
  stringstream ss;
  QueryResultMap::const_iterator it;

  ss << "Current size : " << _currentSize << std::endl
     << "Results      : {";
  
  for (it = _results.begin(); it != _results.end(); ++it)
  {
    ss << " \"" << it->first << "\": " << it->second._topCompletions.asString() << ",";
  }
  ss << "}";
  return ss.str();
}

