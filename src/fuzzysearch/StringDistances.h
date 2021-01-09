// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_STRINGDISTANCES_H_
#define FUZZYSEARCH_STRINGDISTANCES_H_

#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unordered_map>

#include "../fuzzysearch/Utils.h"

namespace FuzzySearch
{

// Generalized edit distance
class GeneralizedEditDistance
{
  private:
    double C[MAX_WORD_LEN + 2][MAX_WORD_LEN + 2];
    // needed by compute
    string _rule;
    wstring _wrule;
    double _val;
    double _minPen;
    int _ii;
    int _jj;

    // compute the minimum among 4 doubles
    double min4(double a, double b, double c, double d);

  public:

    // rules should be given as: "x-y"
    std::unordered_map<string, double, StringHash<string> > rulesHash;
    std::unordered_map<wstring, double, StringHash<wstring> > rulesHashUtf8;

    // def. constructor
    GeneralizedEditDistance();

    // get the rules from phonetic-rules-utf8.txt and
    // phonetic-rules-iso88591.txt (these 2 files must exist!)
    void getRules();

    // old getRules() with hard coded rules
    void getRulesOld();

    // a hack to normalize the rules
    void transformRules()
    {
      for (auto it = rulesHash.begin(); it != rulesHash.end(); it++)
      {
        it->second /= 200;
      }
      for (auto it = rulesHashUtf8.begin(); it != rulesHashUtf8.end(); it++)
      {
        it->second /= 200;
      }
    }

    // calculate distance between 2 strings
    inline double calculate(const string& str1, const string& str2)
    {
     return calculate(str1, str2, true);
    }

    // calculate distance between 2 strings
    inline double calculate(const string& str1, const string& str2, bool norm)
    {
     return calculate(str1, str2, 1.0);
    }

    // calculate distance between 2 strings
    double calculate(
        const string& str1,
        const string& str2,
        double threshold);

    // calculate distance between 2 utf8 strings
    inline double calculateUtf8(const string& str1, const string& str2,
        double threshold)
    {
      wstring wstr1;
      wstring wstr2;
      string2wstring(str1, &wstr1);
      string2wstring(str2, &wstr2);
      return calculate(wstr1, wstr2, threshold);
    }

    // calculate distance between 2 utf8 wstrings
    double calculate(
        const wstring& str1,
        const wstring& str2,
        double threshold);

    // calculate distance between 2 utf8 wstrings
    double calculate(
        const wstring& s1,
        const wstring& s2,
        bool norm)
    {
      return calculate(s1, s2, 1.0);
    }

    // calculate distance between 2 utf8 wstrings
    double calculate(const wstring& s1,
                             const wstring& s2)
    {
      return calculate(s1, s2, 1.0);
    }

    // say what this distance does
    string describe()
    {
     return "Generalized edit distance between two strings with "
            "rules of the form x -> y";
    }
};

// Just plain edit distance
class PlainEditDistance
{
  private:
    unsigned min(const unsigned a, const unsigned b, const unsigned c)
    {
      unsigned x = (a < b ? a : b);
      return (x < c ? x : c);
    }

    // Needed for PlainEditDistance in order not to re-initialize this matrix
    // whenever a distance computations is called.
    unsigned d[2][MAX_WORD_LEN + 1];

    // The table needed for edit distance computation with O(mn) space
    unsigned dd[MAX_WORD_LEN + 1][MAX_WORD_LEN + 1];

    // needed for CalculateOpt (Boytsov)
    unsigned int   dp[3][2 * MAX_ED + 1];
    unsigned int   minVal[3];

  public:
    // Default constructor
    PlainEditDistance()
    {
      for (int j = 0; j <= MAX_WORD_LEN; j++)
      {
        dd[0][j] = j;
        dd[j][0] = j;
      }
    }

    // Computes the distance between words str1 and str2
    // param norm  Indicates whether normlaized (norm=true)
    // or un-normalized distance (norm=false) should be returned
    double calculate(const string& s1, const string& s2, unsigned threshold);

    // Computes the distance between two wstrings str1 and str2
    double calculate(const wstring& s1, const wstring& s2, unsigned threshold);

    // Computes the distance between words str1 and str2
    // param norm  Indicates whether normlaized (norm=true)
    // or un-normalized distance (norm=false) should be returned
    // pr is a prefix length with the previous word (s2)
    double calculatep(const string& s1,
                      const string& s2,
                      int pr);

    // Computes the distance between two wstrings str1 and str2
    // pr is a prefix length with the previous word (s2)
    double calculatep(const wstring& s1,
                      const wstring& s2,
                      int pr);

    // an optimized version of Levenshtein checking algorithm that calculates
    // only 2 * k + 1 main matrix diagonals (GPL Boytsov)
    int calculateOpt(bool hasTransp, const std::string & s1,
        const std::string & s2, int maxDist);

    int calculateOpt(bool hasTransp, const std::wstring & s1,
        const std::wstring & s2, int maxDist);

    // describes what this distance does
    string describe()
    {
      return string("Plain edit distance between strings x and y");
    }
};

// Extension edit distance (between a word and a prefix)
class ExtensionEditDistance
{
  private:
    // computes the minimum between 3 unsigned ints
    inline unsigned min(const unsigned a, const unsigned b, const unsigned c)
    {
      unsigned x = (a < b ? a : b);
      return (x < c ? x : c);
    }

    // Needed for PlainEditDistance in order not to re-initialize this matrix
    // whenever a distance computations is called.
    unsigned d[2][MAX_WORD_LEN + 1];

  public:
    // default constructor
    ExtensionEditDistance()
    {
      for (int i = 0; i <= MAX_WORD_LEN; i++)
      {
        sd[i][0] = i;
        sd[0][i] = i;
      }
    }

    // Used for fast computation of all similar prefixes of
    // a string s2 to a string s1
    unsigned sd[MAX_WORD_LEN + 1][MAX_WORD_LEN + 1];

    // Computes the distance between a prefix s2 and a word s1
    // param norm  Indicates whether normlaized (norm=true)
    // or un-normalized distance (norm=false) should be returned
    double calculate(const string& s2, const string& s1, unsigned thr);

    // Computes the distance between a prefix s2 and a word s1
    double calculate(const wstring& s2, const wstring& s1, unsigned thr);

    // Computes the normalized ext. distance between words str1 and str2
    inline double calculate(const string& str1, const string& str2)
    {
      return calculate(str1, str2, true);
    }

    // Computes the normalized ext. distance between words str1 and str2
    inline double calculate(const wstring& s1,
                            const wstring& s2)
    {
      return calculate(s1, s2, true);
    }

    // Compute all prefixes of s1 similar to s2
    bool calculateSimilarPrefixes(
        const string& s2,
        const string& s1,
        int threshold,
        int prefixLen,
        vector<string>* simPrefixes);
        // hash_set<string, StringHashFunction>*
        // simPrefixes

    // used for incremental distance calculation
    // Note: due to template <class T> can't be move to .cpp
    template <class T>
    inline double calculate(
                     const T& s2,
                     const T& s1,
                     unsigned thr,
                     vector<int16_t>& lastRow)
    {
      unsigned i, j;
      unsigned
        n = s1.length(),
        m = s2.length();

      if (n == 0)
        return 100.0;
      if (m == 0)
        return 100.0;

      if (m > MAX_WORD_LEN)
        return MAX_WORD_LEN;
      if (n > MAX_WORD_LEN)
        return MAX_WORD_LEN;

      unsigned minDist = thr + 1;
      for (i = 1; i <= n; i++)
      {
        for (j = 1; j <= m; j++)
        {
          sd[i][j] = min(sd[i-1][j] + 1,
                            sd[i][j - 1] + 1,
                            sd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
        }
        if (minDist > sd[i][m])
          minDist = sd[i][m];
        // else
        //  if (minDist < sd[i][m])
        //    break;
      }
      if (minDist <= thr)
      {
        int count = 0;
        int start = m - thr;
        int stop  = m + thr;
        for (int i = start; i <= stop; i++)
          lastRow[count++] = sd[i][m];
      }
      return minDist;
    }

    // calculate the distance bewteen a prefix s2 and a word s1 and return
    // the position in s1 where the edit distance is minimum
    // Note: due to template <class T> can't be moved to .cpp
    template <class T>
    inline double calculate(const T& s2, const T& s1, unsigned thr, int& pos)
    {
      unsigned i, j;
      unsigned
        n = s1.length(),
        m = s2.length();

      if (n == 0)
        return 100.0;
      if (m == 0)
        return 100.0;

      if (m > MAX_WORD_LEN)
        return MAX_WORD_LEN;
      if (n > MAX_WORD_LEN)
        return MAX_WORD_LEN;

      unsigned minDist = INT_MAX;
      unsigned limit;
      for (i = 1; i <= n; i++)
      {
        limit = INT_MAX;
        for (j = 1; j <= m; j++)
        {
          sd[i][j] = min(sd[i-1][j] + 1,
                            sd[i][j - 1] + 1,
                            sd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
          if (limit > sd[i][j])
            limit = sd[i][j];
        }
        if (limit > thr && minDist > thr)
          return thr + 1;
        if (minDist >= sd[i][m])
        {
          minDist = sd[i][m];
          pos = i;
        }
      }
      return minDist;
    }

    // used for incremental distance calculation
    // Note: due to template <class T> can't be moved to .cpp
    template <class T>
    inline double calculate_next(
                          const T& s2,
                          const T& s1,
                          vector<int16_t>& prevRow)
    {
      char c = s2[s2.length()-1];
      if (prevRow.size() == 3)
      {
        int end = s1.length() - s2.length() + 2;
        prevRow[0] = MY_MIN(prevRow[1]+1, prevRow[0] +
            (c == s1[s2.length()-2] ? 0 : 1));
        double minVal = prevRow[0];
        if (end > 1)
        {
          prevRow[1] = min(prevRow[0]+1, prevRow[1] +
              (c == s1[s2.length()-1] ? 0 : 1), prevRow[2]+1);
          if (minVal > prevRow[1])
            minVal = prevRow[1];
        }
        if (end > 2)
        {
          prevRow[2] = MY_MIN(prevRow[1]+1, prevRow[2] +
              (c == s1[s2.length()] ? 0 : 1));
          if (minVal > prevRow[2])
            minVal = prevRow[2];
        }
        return minVal;
      }
      else
      if (prevRow.size() == 5)
      {
        prevRow[0] = MY_MIN(prevRow[1]+1, prevRow[0] +
            (c == s1[s2.length()-3] ? 0 : 1));
        double minVal = prevRow[0];
        int16_t end = s1.length() - s2.length() + 3;
        if (end > 5)
          end = 5;
        int end1 = MY_MIN(end, 4);
        for (int16_t i = 1; i < end1; i++)
        {
          prevRow[i] = min(prevRow[i-1]+1, prevRow[i] +
              (c == s1[s2.length()+(i-3)] ? 0 : 1), prevRow[i+1]+1);
          if (minVal > prevRow[i])
            minVal = prevRow[i];
        }
        if (end == 5)
        {
          prevRow[4] = MY_MIN(prevRow[3]+1, prevRow[4] +
              (c == s1[s2.length()+1] ? 0 : 1));
          if (minVal > prevRow[4])
            minVal = prevRow[4];
        }
        return minVal;
      }
      /*
      prevRow[1] = min(prevRow[0]+1,
      prevRow[1] + (c == s1[s2.length()-2] ? 0 : 1), prevRow[2]+1);
      prevRow[2] = min(prevRow[1]+1,
      prevRow[2] + (c == s1[s2.length()-1] ? 0 : 1), prevRow[3]+1);
      prevRow[3] = min(prevRow[2]+1,
      prevRow[3] + (c == s1[s2.length()] ? 0 : 1), prevRow[4]+1);
      prevRow[4] = MY_MIN(prevRow[3]+1,
      prevRow[4] + (c == s1[s2.length()+1] ? 0 : 1));
      */
      return 100;
    }

    // describes what this distance does
    string describe()
    {
      return string("Extension edit distance between a string and a prefix");
    }
};

// Number of letters in the alphabet
#define LETTERS_NO 256

// Note: This class is not being used (legacy code)
// A class computing the modified Levenshtein distance between two words
/*! Note: Three weight *profiles* setting different weights for the
*   edit operations. Weight is a penalty of the particular error
*  (insertions, deletions, substitution and transpositions).
*   Three weight profiles are available: human-typed text, ocr-ed text
*   and a plain profile where the basic Levenshtein distance
*   (including transpositions) is computed.
*   Note: Some values which don't belong to this class are also set in the profile.
*/
class EditDistance
{
  private:
    double min(const double a, const double b, const double c, const double d);
    double min(const double a, const double b, const double c);
    double min(const double a, const double b);
    double max(const double a, const double b);

    // Contains the weight of the insertion errors
    double _w_ins;

    // Contains the weight of the substitution errors
    double _w_sub;

    // Contains the weight of the transposition errors
    double _w_transp;

    // Contains the weight of the deletion errors
    double  _w_del;

    // Contains the weight of the insertion errors when combined with the
    // position in the word
    double _w_insc;

    // Contains the weight of the substitution errors when combined with
    // the position in the word
    double _w_subc;

    // Contains the weight of the transposition errors when combined with
    // the position in the word
    double _w_transpc;

    // Contains the weight of the deletion errors when combined with
    // the position in the word
    double  _w_delc;

    // Contain extra penalty of the error depending on its position
    // in the word
    double _pen_beg, _pen_end;

    double delta;

    // Temporary strings holding previous words
    const string *_old_str, *_old_str2;

    // Temporary strings holding current words
    const string *_str1, *_str2;

    // The cost matrix of the dynamic programming algorithm
    double _costMatrix[MAX_WORD_LEN][MAX_WORD_LEN];

    // A value holding the weight profile of the distance
    // (0 = human-typed, 1 = OCR-ed text, 2 = plain)
    int  _profile;

    // A value which indicates whether distance (sim=false) or 1-distance
    // should be returned (sim=true)
    bool _sim;

    // Length of the first word
    size_t _m;

    // Length of the second word
    size_t _n;

    // Contains different weights for neighborhing and non-neighborhing
    // keyboard keys
    static double confusionMatrix[LETTERS_NO][LETTERS_NO];

    // Indicates whether the confusionMatrix (see above) is initialized
    static bool init;

    // Initializes the confusionMatrix (see above)
    static void initQUERTY(double n, double nn);

    // keeps the running time in micro. spent on computing the distance
    unsigned int _microseconds;

    // Timer
    Timer _timer;

  public:

    // Constructor
    // \param profile Weight profile (0, 1 or 2, read above)
    explicit EditDistance(int profile);

    //  Constructor
    //   \param profile Weight profile (0, 1 or 2, read above)
    //   \param sim Indicates whether distance or 1-distance should be
    //    returned (see above)
    EditDistance(int profile, bool sim);

    // Virtual destructor
    virtual ~EditDistance() {}

    //  Computes the distance between words str1 and str2
    //  \param norm  Indicates whether normlaized (norm=true) or un-normalized
    //  distance (norm=false) should be returned
    virtual double calculate(const string& str1, const string& str2, bool norm);

    // Computes the normalized distance between words str1 and str2
    virtual double calculate(const string& str1, const string& str2);

    // TODO(hannah): Why are these not implemented? The code does not even
    // compile without this.
    virtual double calculate(const wstring& s1,
                             const wstring& s2,
                             bool norm)
    {
      perror("NOT IMPLEMENTED!");
      exit(1);
    }

    // TODO(hannah): Why are these not implemented? The code does not even
    // compile without this.
    virtual double calculate(const wstring& s1,
                             const wstring& s2)
    {
      perror("NOT IMPLEMENTED!");
      exit(1);
    }

    // Sets the weights profile (see above)
    void setProfile(int profile);

    // Returns short description of the distance
    string describe();

    // void updateRule(Rule r, MTFHashTableRules htr)
    // void getRules(MTFHashTableRules htr);

    // returns the number of milliseconds spent on computing distances
    int getMicroseconds()
    {
      return _microseconds;
    }

    // resets the timer
    void resetMicroseconds()
    {
      _microseconds = 0;
    }
};

// Below: Levenshtein distance computation based on the bit-parallel version of
// the algorithm from M. Myers. Original code taken from Leonid Boytsov's web.

typedef unsigned int MaskType;
// typedef unsigned long long MaskType;

unsigned int CalcDistGlob(
    const unsigned char* p,
    bool HasTransp,
    unsigned int PatLen,
    MaskType Checker,
    MaskType VP0,
    MaskType *PMText);

unsigned int CalcDistGlob(
    const unsigned wchar_t* p,
    bool HasTransp,
    unsigned int PatLen,
    MaskType Checker,
    MaskType VP0,
    MaskType *PMText);

class CMyersEdistFastPair
{
 public:
    CMyersEdistFastPair()  { memset(mPMText, 0, sizeof mPMText); }
    inline unsigned int calculate(
        const std::string& s1,
        const std::string& s2,
        bool bHasTransp)
    {
        const char* pStr1 = s1.c_str();
        const char* pStr2 = s2.c_str();
        MaskType        Checker = 0;
        MaskType        VP0     = 0;
        MaskType        Flag    = 1;
        unsigned int    PatLen  = 0;

        /* Init pattern mask */
        // should be initialized only once for subsequent calls this function
        // on the same pattern
        for (const unsigned char *p = reinterpret_cast<const unsigned char*>(pStr1);*p; ++PatLen, Flag <<= 1, ++p)  // NOLINT
        {
            if (!Flag)
            {
                throw std::runtime_error("Pattern is too long, should be no more than the computer word size");  // NOLINT
            }
            mPMText[*p] |= Flag;
            VP0         |= Flag;
            Checker      = Flag;
        }

        unsigned int dist = CalcDistGlob(reinterpret_cast<const unsigned char*>(pStr2), bHasTransp, PatLen, Checker, VP0, mPMText);  // NOLINT

        /* Clear pattern mask */
        for (const unsigned char *p = reinterpret_cast<const unsigned char*>(pStr1);*p; ++p)  // NOLINT
        {
            mPMText[*p] = 0;
        }

        return dist;
    }

    inline unsigned int calculate(
        const std::wstring& s1,
        const std::wstring& s2,
        bool bHasTransp)
    {
        const wchar_t* pStr1 = s1.c_str();
        const wchar_t* pStr2 = s2.c_str();
        MaskType        Checker = 0;
        MaskType        VP0     = 0;
        MaskType        Flag    = 1;
        unsigned int    PatLen  = 0;

        /* Init pattern mask */
        for (const unsigned char *p = reinterpret_cast<const unsigned char*>(pStr1);*p; ++PatLen, Flag <<= 1, ++p)  // NOLINT
        {
            if (!Flag)
            {
                throw std::runtime_error("Pattern is too long, should be no more than the computer word size");  // NOLINT
            }
            mPMText[*p] |= Flag;
            VP0         |= Flag;
            Checker      = Flag;
        }

        unsigned int dist = CalcDistGlob(reinterpret_cast<const unsigned wchar_t*>(pStr2), bHasTransp, PatLen, Checker, VP0, mPMText);  // NOLINT

        /* Clear pattern mask */
        for (const unsigned wchar_t *p = reinterpret_cast<const unsigned wchar_t*>(pStr1);*p; ++p)  // NOLINT
        {
            mPMText[*p] = 0;
        }
        return dist;
    }

 private:
    MaskType mPMText[256];
};

// Returns x to the power of n (to avoid including math.h)
int powInt(int x, int n);

// Note: this class is not being used (legacy code).
// Q-gram dictinary: a class used for storing q-grams
/*! This class is used for computing union and intersections of q-grams in
 *  words and it is used by q-gram distances.
 */
class qGramDict
{
  private:
    // Indicates the maximum number of q-grams to be stored
    const static int _CAPACITY = 500;  // NOLINT

    // Hold the number of different letters in the alphabet
    const static int _VEC_SIZE = 256;  // NOLINT

    // Holds the number of distinct q-grams that can be stored
    int _size;

    // Indicates the number of stored q-grams in the data structure
    int _count;

    // Indicates the q-gram length
    int _q;

    // Indicates whether the i-th q-gram (items[i]) is stored or not
    bool* _items;

    // Holds the stored q-grams in consequtive order (used for faster clearing
    // the data structure)
    int _qg[_CAPACITY];

    // Returns the integer identification of a particular q-gram
    int toInt(const string& qgram);
  public:

    // Default constructor
    qGramDict();

    // Constructor
    /*! \param  q Indicates the q-gram length
     */
    explicit qGramDict(const int q);

    // Destructor
    ~qGramDict();

    // Initializes the dictionary variables
    void initialize(const int q);

    // Returns true if the q-gram is stored and false otherwise
    bool find(const string &qgram);

    // Stores the q-gram
    void set(const string &qgram);

    // Removes the q-gram
    void unset(const string &qgram);

    // Makes the data structure empty (no q-grams stored)
    void clear();

    // Returns the number of q-grams stored
    int count();
};

// Note: this class is not being used (legacy code).
// A class computing q-gram based distances on 2 words (Ukkonen's,
// Jaccard and Cosine)
/*! Note: This class uses the qGramDict class for storing q-grams
 */
class QGramDistances
{
  private:

    // Indicates the q-gram distance to use (0=jaccard, 1=cosine, 2=ukkonen
    // un-normalized, 3=ukkonen normalized)
    int _w;

    // Holds the q-gram length (in letters)
    int _q;

    // q-gram dictionary (qGramDict) holding the q-grams of the first word
    qGramDict _qd1;

    // q-gram dictionary (qGramDict) holding the q-grams of the second word
    qGramDict _qd2;

    // q-gram dictionary (qGramDict) holding the union of q-grams
    // (of both words)
    qGramDict _qd3;

    // Temp. variable holding the number of q-grams in the first word
    int _x1;

    // Temp. variable holding the number of q-grams in the second word
    int _x2;

    // Temp. variable holding the number of q-grams in the union of q-grams
    int _x3;

    // Computes the number of common q-grams between two words
    int intersect(const string& str1, const string& str2);

  public:

    // Constructor
    /*! \param  q q-gram length
     *  \param  w q-gram distance choice (for details see above) 
     */
    QGramDistances(const int q, const int w);

    // Virtual destructor
    ~QGramDistances() {}

    // Computes the chosen q-gram distance on two words
    double calculate(const string& str1, const string& str2);

    // Identical as above (the parameter norm has no function)
    double calculate(const string& str1, const string& str2, const bool norm);

    // Changes the q-gram length
    void setQ(const int q);

    // Returns short description for the chosen distance
    string describe();
};
}

#endif  // FUZZYSEARCH_STRINGDISTANCES_H_
