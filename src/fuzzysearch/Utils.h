// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_UTILS_H_
#define FUZZYSEARCH_UTILS_H_

#include <sys/time.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <utility>
#include <fstream>

#include "../fuzzysearch/Timer.h"

namespace FuzzySearch
{
using std::string;
using std::wstring;
using std::cout;
using std::vector;
using std::list;
using std::pair;
using std::fstream;

#define MY_MIN(a, b) ( (a) < (b) ? (a) : (b) )
#define MY_MAX(a, b) ( (a) < (b) ? (b) : (a) )

// maximum line length when reading string lines from a file
#define CWF_MAX_LINE_LENGTH 10 * 1000

// Maximum valid word length
#define MAX_WORD_LEN 45

// Minimum word length
#define MIN_WORD_LEN 4

// Maximum alowable edit distance
#define MAX_ED 3

// basic string hash function from Globals.h (no speed needed for what is used!)
template <class T>
class StringHash
{
  public:
    size_t operator()(const T& s) const
    {
      size_t HashValue = 0;
      for (size_t i = 0; i < s.length(); i++)
        HashValue = 31 * HashValue + s[i];
      return HashValue;
    }
};

// experimental
template <class T>
class StringIntHash
{
  public:
    size_t operator()(const pair<T, int>& s) const
    {
      size_t HashValue = 0;
      for (size_t i = 0; i < s.first.length(); i++)
        HashValue = 31 * HashValue + s.first[i];
      HashValue += s.second;
      return HashValue;
    }
};

class StringHashFunction
{
  public:
    size_t operator()(const string& s) const
    {
      size_t HashValue = 0;
      for (size_t i = 0; i < s.length(); i++)
        HashValue = 31 * HashValue + s[i];
      return HashValue;
    }
};

// convert an utf8 string to lower case
int utf8_tolower(char* p);

// conversion string to wstring according to the set locale
bool string2wstring(const string& str, wstring* wstr);

// converts wstring to string
bool wstring2string(const wstring& wstr, string* str);

// checks if s1 is strict prefix of s2. E.g. algo and algorithm,
// but not algo and algo.
bool isStrictPrefix(const string& s1, const string& s2);

// Class used for fast % (modulo) calculation
class Mod
{
  public:

    static vector<vector<int> > mod;

    static void init()
    {
      mod.resize(4 * (MAX_WORD_LEN + 1));
      for (size_t i = 0; i < mod.size(); i++)
        mod[i].resize(4 * (MAX_WORD_LEN + 1));
      for (size_t i = 0; i < mod.size(); i++)
        for (size_t j = 1; j < mod.size(); j++)
          mod[i][j] = i % j;
    }

    static int calc(int x, int y)
    {
      return mod[x][y];
    }
};

// Tells the percentage of the work done for some procedure
class ProgressIndicator
{
  private:
    // The last integer percetage done
    int _lastPercent;

    // The total value
    int _total;

    // The update interval (number of percents to be done such that the new
    // percentage is shown)
    int _interval;

  public:
    // Constructor
    /*! \param  total The total value
     *  \param  interval  Update interval, i.e. 2 means the percentage will
     *  be shown after 2% are done
     */
    ProgressIndicator(int total, int interval)
    {
      _total = total;
      if (_total <= 0)
        _total = 1;
      _lastPercent = 0;
      _interval = interval;
    }

    // reset the progress indicator
    inline void clear()
    {
      _lastPercent = 0;
    }

    // sets the total
    inline void setTotal(int total)
    {
      _total = total;
      if (_total <= 0)
        _total = 1;
    }

    // Updates the progress indicator
    inline void update(int newValue)
    {
      int newPercentage = 100 * newValue / _total;
      if (newPercentage >= _lastPercent + _interval)
      {
        cout << newPercentage << "% ";
        cout.flush();
        _lastPercent = newPercentage;
      }
    }
};

class StringHashFunctionMarjan
{
  public:
    size_t operator()(const string& word) const
    {
      size_t h = 1159241;
      for (size_t i = 0; i < word.length(); i++)
        h ^= ((h << 5) + word[i] + (h >> 2));
      h &= 0x7fffffff;
      return h;
    }
};

#define hashmask(n) (hashsize(n)-1)

#define hashsize(n) ((uint32_t)1 << (n))

template <class T>
class StringHashSet
{
 private:
  // Buckets, implemented as linked lists
  vector<list<pair<T, int> > > _buckets;

  // Holds the number of stored elements
  unsigned int _count;

  // Holds the number of buckets in the hash table
  unsigned int _size;

  unsigned int _bits;

  unsigned int _mask;

  off_t h;

  off_t HT_SEED;

  typename list<pair<T, int> >::iterator iter;

 public:

  // Reserves space for the buckets and sets all pointers to null
  // (which indicates that the bucket is empty)
  void init()
  {
    _count = 0;
    _mask = hashmask(_bits);
    _buckets.resize(_size);
    HT_SEED = 100000007;
  }

  StringHashSet()
  {
    this -> _bits = 20;
    this -> _size = 2 << (_bits - 1);
    init();
  }

  explicit StringHashSet(unsigned int bits)
  {
    if (bits > 31)
      bits = 31;
    this -> _bits = bits;
    this -> _size = 2 << (_bits - 1);
    init();
  }

  // Hash functions which maps strings -> integers
  inline off_t getHashValue(const T& str)
  {
    h = HT_SEED;
    for (unsigned int i = 0; i < str.length(); i++)
      h ^= ((h << 5) + str[i] + (h >> 2));
    return h & _mask;
  }

  inline int find(const T& x)
  {
    unsigned int h = getHashValue(x);
    for (iter = _buckets[h].begin(); iter != _buckets[h].end(); iter ++)
    {
      if ((*iter).first == x)
      {
        return (*iter).second;
      }
    }
    return -1;
  }

  inline bool insert(const pair<T, int>& x)
  {
    unsigned int h = getHashValue(x.first);
    for (iter = _buckets[h].begin(); iter != _buckets[h].end(); iter ++)
    {
      if ((*iter).first == x.first)
      {
        return false;
      }
    }
    _count++;
    _buckets[h].push_back(x);
    return true;
  }

  inline size_t size()
  {
    return _count;
  }
};

class IntHashSet
{
 private:
  // Buckets, implemented as linked lists
  vector<list<int> > _buckets;

  // Holds the number of stored elements
  unsigned int _count;

  // Holds the number of buckets in the hash table
  unsigned int _size;

  unsigned int _bits;

  unsigned int _mask;

  off_t h;

  off_t HT_SEED;

  list<int>::iterator iter;

 public:

  // Reserves space for the buckets and sets all pointers to null
  // (which indicates that the bucket is empty)
  void init()
  {
    _count = 0;
    _mask = hashmask(_bits);
    _buckets.resize(_size);
    HT_SEED = 100000007;
  }

  IntHashSet()
  {
    this -> _bits = 20;
    this -> _size = 2 << (_bits - 1);
    init();
  }

  explicit IntHashSet(unsigned int bits)
  {
    if (bits > 31)
      bits = 31;
    this -> _bits = bits;
    this -> _size = 2 << (_bits - 1);
    init();
  }

  // Hash functions which maps strings -> integers
  inline off_t getHashValue(int val)
  {
    return val & _mask;
  }

  inline bool find(int x)
  {
    unsigned int h = getHashValue(x);
    for (iter = _buckets[h].begin(); iter != _buckets[h].end(); iter ++)
    {
      if ((*iter) == x)
        return true;
    }
    return false;
  }

  inline bool insert(int x)
  {
    unsigned int h = getHashValue(x);
    for (iter = _buckets[h].begin(); iter != _buckets[h].end(); iter ++)
    {
      if (*iter == x)
        return false;
    }
    _count++;
    _buckets[h].push_back(x);
    return true;
  }

  inline size_t size()
  {
    return _count;
  }

  inline bool end()
  {
    return false;
  }
};
}

#endif  // FUZZYSEARCH_UTILS_H_

