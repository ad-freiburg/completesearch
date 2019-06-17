// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#include <valarray>
#include <algorithm>
#include <vector>
#include <string>

#include "../fuzzysearch/StringDistances.h"
#include "../fuzzysearch/Utils.h"

namespace FuzzySearch
{
// ____________________________________________________________________________
double GeneralizedEditDistance::min4(double a, double b, double c, double d)
{
  double min = a;
  if (min > b)
    min = b;
  if (min > c)
    min = c;
  if (min > d)
    min = d;
  return min;
}

// ____________________________________________________________________________
GeneralizedEditDistance::GeneralizedEditDistance()
{
  for (unsigned int i = 0; i < MAX_WORD_LEN + 1; i++)
  {
    C[i][0] = i;
    C[0][i] = i;
  }
  _rule.reserve(100);
  getRules();
  transformRules();
  _ii = 0;
  _jj = 0;
}

// ____________________________________________________________________________
void GeneralizedEditDistance::getRules()
{
  cout << "* Reading phonetic rules for generalized edit distance ... "
       << std::flush;
  fstream f1("phonetic-rules-iso88591.txt", std::ios::in);
  if (f1.fail())
  {
    std::cerr << "Error opening \"phonetic-rules-iso88591.txt\", exiting."
         << std::endl;
    exit(1);
  }
  fstream f2("phonetic-rules-utf8.txt", std::ios::in);
  if (f2.fail())
  {
    std::cerr << "Error opening \"phonetic-rules-utf8.txt\", exiting."
              << std::endl;
    exit(1);
  }
  int intVal;
  string strVal;
  while (true)
  {
    f1 >> strVal;
    if (f1.eof())
      break;
    string lr = strVal;
    f1 >> strVal;
    string rr = strVal;
    if (rr == "-")
      rr = "";
    f1 >> strVal;
    f1 >> strVal;
    f1 >> strVal;
    intVal = atoi(strVal.c_str());
    strVal = lr+"-"+rr;
    rulesHash[strVal] = intVal;
    strVal = rr+"-"+lr;
    rulesHash[strVal] = intVal;
    f1 >> strVal;
  }
  f1.close();
  while (true)
  {
    f2 >> strVal;
    if (f2.eof())
      break;
    wstring wstrVal;
    string lr = strVal;
    f2 >> strVal;
    string rr = strVal;
    if (rr == "-")
      rr = "";
    f2 >> strVal;
    f2 >> strVal;
    f2 >> strVal;
    intVal = atoi(strVal.c_str());
    string2wstring(lr+"-"+rr, &wstrVal);
    rulesHashUtf8[wstrVal] = intVal;
    wstrVal.clear();
    string2wstring(rr+"-"+lr, &wstrVal);
    rulesHashUtf8[wstrVal] = intVal;
    f2 >> strVal;
  }
  f2.close();
  std::cout << "done." << std::endl;
}

// ____________________________________________________________________________
void GeneralizedEditDistance::getRulesOld()
{
  rulesHash["ij-i"] = 10;
  rulesHash["iy-i"] = 10;
  rulesHash["iy-y"] = 30;
  rulesHash["i-j"] = 30;
  rulesHash["i-y"] = 30;
  rulesHash["j-y"] = 20;
  rulesHash["ie-i"] = 5;
  rulesHash["ie-"] = 120;
  rulesHash["eu-oi"] = 10;
  rulesHash["eu-oy"] = 10;
  rulesHash["oi-oy"] = 5;
  rulesHash["ou-u"] = 5;
  rulesHash["ah-a"] = 5;
  rulesHash["h-"] = 50;
  rulesHash["ae-a"] = 60;
  rulesHash["a-er"] = 70;
  rulesHash["ae-e"] = 20;
  rulesHash["ae-"] = 110;
  rulesHash["ai-ay"] = 5;
  rulesHash["ai-ei"] = 5;
  rulesHash["ai-ey"] = 10;
  rulesHash["ay-ei"] = 10;
  rulesHash["ay-ey"] = 5;
  rulesHash["ai-ey"] = 5;
  rulesHash["a-o"] = 70;
  rulesHash["o-oe"] = 0;
  rulesHash["\xe4-e"] = 20;  // NOLINT
  rulesHash["\xe4-aa"]= 0;   // NOLINT
  rulesHash["\xe4-ae"] = 0;  // NOLINT
  rulesHash["\xe4-a"] = 40;  // NOLINT
  rulesHash["\xf6-o"] = 40;  // NOLINT
  rulesHash["\xd8-\xd6"] = 0;   // NOLINT
  rulesHash["\xd8-OE"] = 10; // NOLINT
  rulesHash["\xd8-O"] = 30;  // NOLINT
  rulesHash["oe-\xf6"] = 30; // NOLINT
  rulesHash["\xdf-ss"] = 20;
  rulesHash["\xdf-s"] = 10;
  rulesHash["\xfc-ue"] = 20;  // NOLINT
  rulesHash["\xfc-u"] = 40;   // NOLINT
  rulesHash["\xfc-y"] = 30;   // NOLINT
  rulesHash["oe-o"] = 60;
  rulesHash["oe-"] = 110;
  rulesHash["chs-x"] = 100;
  rulesHash["cks-x"] = 30;
  rulesHash["ck-k"] = 5;
  rulesHash["c-k"] = 30;
  rulesHash["chs-"] = 200;
  rulesHash["ch-"] = 130;
  rulesHash["cks-"] = 200;
  rulesHash["ck-"] = 110;
  rulesHash["dt-t"] = 5;
  rulesHash["d-t"] = 30;
  rulesHash["th-t"] = 5;
  rulesHash["dt-"] = 110;
  rulesHash["ks-x"] = 5;
  rulesHash["gs-x"] = 10;
  rulesHash["g-k"] = 50;
  rulesHash["qu-kw"] = 10;
  rulesHash["q-k"] = 10;
  rulesHash["nch-nsch"] = 10;
  rulesHash["nch-nsh"] = 10;
  rulesHash["ntx-nch"] = 20;
  rulesHash["ntx-nsch"] = 20;
  rulesHash["ntx-nsh"] = 20;
  rulesHash["ng-nk"] = 20;
  rulesHash["sch-sh"] = 5;
  rulesHash["sch-sz"] = 20;
  rulesHash["sch-s"] = 100;
  rulesHash["sch-"] = 200;
  rulesHash["tz-z"] = 5;
  rulesHash["tia-zia"] = 20;
  rulesHash["z-c"] = 40;
  rulesHash["z-s"] = 50;
  rulesHash["m-n"] = 70;
  rulesHash["n-u"] = 70;
  rulesHash["ph-f"] = 5;
  rulesHash["pf-f"] = 5;
  rulesHash["b-p"] = 40;
  rulesHash["f-v"] = 20;
  rulesHash["w-v"] = 20;
  rulesHash["ue-u"] = 60;
  rulesHash["ue-"] = 110;

  rulesHash["ss-s"] = 5;

  for (auto it = rulesHash.begin(); it != rulesHash.end(); it++)
  {
    int pos = it->first.find('-');
    string lr = it->first.substr(0, pos);
    string rr = it->first.substr(pos+1, it->first.length()-pos);
    string inversRule = rr + "-" + lr;
    rulesHash[inversRule] = it->second;
    wstring wstr;
    string2wstring(it->first, &wstr);
    rulesHashUtf8[wstr] = it->second;
    wstr.clear();
    string2wstring(inversRule, &wstr);
    rulesHashUtf8[wstr] = it->second;
  }
}

// ____________________________________________________________________________
double GeneralizedEditDistance::calculate(
    const string& str1,
    const string& str2,
    double threshold)
{
  int m = str1.length();
  int n = str2.length();
  double maxmn = MY_MAX(m, n);

  if (abs(m-n) > threshold * maxmn)
    return 1;

  // int count;
  for (int i = 1; i <= m; i++)
  {
    for (int j = 1; j <= n; j++)
    {
      _minPen = maxmn;
      int len1 = MY_MAX(0, i-2);
      int len2 = MY_MAX(0, j-2);
      for (int i1 = len1; i1 <= i; i1++)
      {
        for (int j1 = len2; j1 <= j; j1++)
        {
          _rule = str1.substr(i1, i - i1) + "-" +str2.substr(j1, j - j1);
          if (rulesHash.count(_rule) > 0)
          {
            _val = rulesHash[_rule];
            if (_val < _minPen)
            {
              _minPen = _val;
              _ii = i1;
              _jj = j1;
            }
          }
        }
      }
      C[i][j] = min4(C[_ii][_jj]+_minPen, C[i-1][j]+1, C[i][j-1]+1,
         C[i-1][j-1] + (str1[i - 1] == str2[j - 1] ? 0 : 1));
    }
  }
  return C[m][n] / maxmn;
}

// ____________________________________________________________________________
double GeneralizedEditDistance::calculate(
    const wstring& str1,
    const wstring& str2,
    double threshold)
{
  int m = str1.length();
  int n = str2.length();

  double maxmn = MY_MAX(m, n);

  if (abs(m-n) > threshold * maxmn)
    return 1;

  // int count;
  for (int i = 1; i <= m; i++)
  {
    for (int j = 1; j <= n; j++)
    {
      _minPen = maxmn;
      // _rule.resize(7);
      int len1 = MY_MAX(0, i-2);
      int len2 = MY_MAX(0, j-2);
      for (int i1 = len1; i1 <= i; i1++)
      {
        for (int j1 = len2; j1 <= j; j1++)
        {
          _wrule = str1.substr(i1, i - i1) + L"-" +str2.substr(j1, j - j1);
          if (rulesHashUtf8.count(_wrule) > 0)
          {
            _val = rulesHashUtf8[_wrule];
            if (_val < _minPen)
            {
              _minPen = _val;
              _ii = i1;
              _jj = j1;
            }
          }
        }
      }
      C[i][j] = min4(C[_ii][_jj]+_minPen, C[i-1][j]+1, C[i][j-1]+1,
          C[i-1][j-1] + (str1[i - 1] == str2[j - 1] ? 0 : 1));
    }
  }
  return C[m][n] / maxmn;
}

// ____________________________________________________________________________
double PlainEditDistance::calculate(
    const string& s1,
    const string& s2,
    unsigned threshold)
{
  unsigned i, j;
  unsigned
    n = s1.length(),
    m = s2.length();

  if (n == 0 || m > MAX_WORD_LEN)
    return m;
  if (m == 0 || n > MAX_WORD_LEN)
    return n;

  unsigned limit;
  for (i = 1; i <= n; i++)
  {
    limit = threshold + 1;
    for (j = 1; j <= m; j++)
    {
      dd[i][j] = min(dd[i-1][j] + 1,
                        dd[i][j - 1] + 1,
                        dd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
      if (limit > dd[i][j])
        limit = dd[i][j];
    }
    if (limit > threshold)
      return threshold + 1;
  }
  return dd[n][m];
}

// ____________________________________________________________________________
double PlainEditDistance::calculate(
    const wstring& s1,
    const wstring& s2,
    unsigned threshold)
{
  unsigned i, j;
  unsigned
    n = s1.length(),
    m = s2.length();

  if (n == 0 || m > MAX_WORD_LEN)
    return m;
  if (m == 0 || n > MAX_WORD_LEN)
    return n;

  unsigned limit;
  for (i = 1; i <= n; i++)
  {
    limit = threshold + 1;
    for (j = 1; j <= m; j++)
    {
      dd[i][j] = min(dd[i-1][j] + 1,
                        dd[i][j - 1] + 1,
                        dd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
      if (limit > dd[i][j])
        limit = dd[i][j];
    }
    if (limit > threshold)
      return threshold + 1;
  }
  return dd[n][m];
}

// ____________________________________________________________________________
double PlainEditDistance::calculatep(
    const string& s1,
    const string& s2,
    int prefixLen)
{
  unsigned i, j;
  unsigned
    n = s1.length(),
    m = s2.length();

  if (n == 0 || m > MAX_WORD_LEN)
    return m;
  if (m == 0 || n > MAX_WORD_LEN)
    return n;

  unsigned pl = prefixLen + 1;
  for (i = 1; i <= n; i++)
  {
    for (j = pl; j <= m; j++)
      dd[i][j] = min(dd[i-1][j] + 1,
                        dd[i][j - 1] + 1,
                        dd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
  }
  return dd[n][m];
}

// ____________________________________________________________________________
double PlainEditDistance::calculatep(
    const wstring& s1,
    const wstring& s2,
    int prefixLen)
{
  unsigned i, j;
  unsigned
    n = s1.length(),
    m = s2.length();

  if (n == 0 || m > MAX_WORD_LEN)
    return m;
  if (m == 0 || n > MAX_WORD_LEN)
    return n;

  unsigned pl = prefixLen + 1;
  for (i = 1; i <= n; i++)
  {
    for (j = pl; j <= m; j++)
      dd[i][j] = min(dd[i-1][j] + 1,
                        dd[i][j - 1] + 1,
                        dd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
  }
  return dd[n][m];
}

// ____________________________________________________________________________
double ExtensionEditDistance::calculate(
    const string& s1,
    const string& s2,
    unsigned thr)
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
  for (j = 1; j <= m; j++)
  {
    limit = thr + 1;
    for (i = 1; i <= n; i++)
    {
      sd[i][j] = min(sd[i-1][j] + 1,
                        sd[i][j - 1] + 1,
                        sd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
      if (limit > sd[i][j])
        limit = sd[i][j];
    }
    if (limit > thr && minDist > thr)
      return thr + 1;
    if (minDist > sd[n][j])
      minDist = sd[n][j];
  }
  return minDist;
}

// ____________________________________________________________________________
double ExtensionEditDistance::calculate(
    const wstring& s1,
    const wstring& s2,
    unsigned thr)
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
  for (j = 1; j <= m; j++)
  {
    limit = thr + 1;
    for (i = 1; i <= n; i++)
    {
      sd[i][j] = min(sd[i-1][j] + 1,
                        sd[i][j - 1] + 1,
                        sd[i-1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
      if (limit > sd[i][j])
        limit = sd[i][j];
    }
    if (limit > thr && minDist > thr)
      return thr + 1;
    if (minDist > sd[n][j])
      minDist = sd[n][j];
  }
  return minDist;
}

// ____________________________________________________________________________
bool ExtensionEditDistance::calculateSimilarPrefixes(
    const string& s2,
    const string& s1,
    int threshold,
    int prefixLen,
    vector<string>* simPrefixes)
    // hash_set<string, StringHashFunction>*
    // simPrefixes
{
  unsigned i, j;
  unsigned
    n = s1.length(),
    m = s2.length();

  if (n == 0)
    return false;
  if (m == 0)
    return false;

  if (n > MAX_WORD_LEN)
  assert(m <= MAX_WORD_LEN);
  assert(n <= MAX_WORD_LEN);

  for (i = 1 + prefixLen; i <= n; i++)
  {
    for (j = 1; j <= m; j++)
      sd[i][j] = min(sd[i-1][j] + 1,
                        sd[i][j-1] + 1,
                        sd[i-1][j-1] + (s1[i-1] == s2[j-1] ? 0 : 1));
    if (static_cast<int>(sd[i][m]) <= threshold)
    {
      simPrefixes->push_back(s1.substr(0, i));
      return true;
      // simPrefixes->insert(s1.substr(0, i));
    }
    else
      if (i > m)
        return false;
  }
  return false;
}

// Below: Levenshtein distance computation based on the bit-parallel version of
// the algorithm from M. Myers. Original code taken from Leonid Boytsov's web.
// ____________________________________________________________________________
unsigned int CalcDistGlob(
    const unsigned char* p,
    bool HasTransp,
    unsigned int PatLen,
    MaskType Checker,
    MaskType VP0,
    MaskType PMText[])
{
    unsigned int dist = PatLen;
    MaskType    VP = VP0,
                VN = 0,
                HP = 0,
                HN = 0,
                D0 = 0,
                PrevPM = 0;
    if (!HasTransp)
    {
        for (;*p; ++p)
        {
            MaskType PM = PMText[*p];

            D0 = (((PM & VP) + VP) ^ VP) | PM | VN;

            HP = VN | ~(D0 | VP);
            HN = D0 & VP;

            if (HP & Checker) ++dist;
            else if (HN & Checker) --dist;

            MaskType HPShift = (HP << 1) | 1;

            VP = (HN << 1) | ~(D0 | HPShift);
            VN = D0 & HPShift;
        }
    }
    else
    {
        for (;*p; ++p)
        {
            MaskType PM = PMText[*p];
            MaskType TR = (((~D0) & PM) << 1) & PrevPM;
            PrevPM = PM;

            D0 = (((PM & VP) + VP) ^ VP) | PM | VN | TR;

            HP = VN | ~(D0 | VP);
            HN = D0 & VP;

            if (HP & Checker) ++dist;
            else if (HN & Checker) --dist;

            MaskType HPShift = (HP << 1) | 1;

            VP = (HN << 1) | ~(D0 | HPShift);
            VN = D0 & HPShift;
        }
    }
    return dist;
}

// ____________________________________________________________________________
unsigned int CalcDistGlob(
    const unsigned wchar_t* p,
    bool HasTransp,
    unsigned int PatLen,
    MaskType Checker,
    MaskType VP0,
    MaskType PMText[])
{
    unsigned int dist = PatLen;
    MaskType    VP = VP0,
                VN = 0,
                HP = 0,
                HN = 0,
                D0 = 0,
                PrevPM = 0;

    if (!HasTransp)
    {
        for (;*p; ++p)
        {
            MaskType PM = PMText[*p];

            D0 = (((PM & VP) + VP) ^ VP) | PM | VN;

            HP = VN | ~(D0 | VP);
            HN = D0 & VP;

            if (HP & Checker) ++dist;
            else if (HN & Checker) --dist;

            MaskType HPShift = (HP << 1) | 1;

            VP = (HN << 1) | ~(D0 | HPShift);
            VN = D0 & HPShift;
        }
    }
    else
    {
        for (;*p; ++p)
        {
            MaskType PM = PMText[*p];
            MaskType TR = (((~D0) & PM) << 1) & PrevPM;
            PrevPM = PM;

            D0 = (((PM & VP) + VP) ^ VP) | PM | VN | TR;

            HP = VN | ~(D0 | VP);
            HN = D0 & VP;

            if (HP & Checker) ++dist;
            else if (HN & Checker) --dist;

            MaskType HPShift = (HP << 1) | 1;

            VP = (HN << 1) | ~(D0 | HPShift);
            VN = D0 & HPShift;
        }
    }
    return dist;
}

// ____________________________________________________________________________
// Note: Legacy code. Not being used for fuzzysearch.

double EditDistance::confusionMatrix[LETTERS_NO][LETTERS_NO];
bool EditDistance::init = false;

EditDistance::EditDistance(int profile)
{
  EditDistance(profile, false);
}

EditDistance::EditDistance(int profile, bool sim)
{
  this -> _profile = profile;
  this -> _sim = sim;
  setProfile(_profile);
  _costMatrix[0][0] = 0;
  _microseconds = 0;
  // Initialize the CostMatrix
  if (_profile == 0 || _profile == 1 || _profile == 2)
  {
    for (int i = 1; i <= MAX_WORD_LEN; i++)
    {
      if (i <= 2)
        _costMatrix[0][i] += _costMatrix[0][i-1] + (1.0f + _pen_beg);
          else
        _costMatrix[0][i] += _costMatrix[0][i-1] + 1.0f;
    }
    for (int j = 1; j <= MAX_WORD_LEN; j++)
    {
      if (j <= 2)
        _costMatrix[j][0] += _costMatrix[j-1][0] + (1.0f + _pen_beg);
      else
        _costMatrix[j][0] += _costMatrix[j-1][0] + 1.0f;
    }
  }
}

double EditDistance::calculate(const string& str1, const string& str2)
{
  return calculate(str1, str2, true);
}

double EditDistance::calculate(const string& str1,
                               const string& str2, const bool norm)
{
  _timer.start();
  this -> _str1 = &str1;
  this -> _str2 = &str2;
  _m = _str1 -> length();
  _n = _str2 -> length();
  unsigned char letter1, letter2;
  int prefix = 1;

  // Save some running time by reusing the previus matrix
  // (up to the common prefixes)
  if (_str2 == _old_str2)
  {
    int minl = MY_MIN(_m, _old_str -> length());
    for (int i = prefix - 1; i < minl; i++)
    if (_str1 -> at(i) == _old_str -> at(i))
    {
      prefix++;
    }
      else
        break;
  }

  // The computations
  for (uint32_t i = prefix; i <= _m; i++)
  {
    for (uint32_t j = 1; j <= _n; j++)
    {
      if (_str1 -> at(i-1) == _str2 -> at(j-1))
        _w_subc = 0;
      else
      {
        _w_subc = 2.0f;
      }
      _w_insc = _w_ins;
      _w_delc = _w_del;

      // Error in the end of the word(s)
      if (i >= _m-2 && j >= _n-2)
      {
        _w_insc -= /*inscp(i, j)*/ _pen_end;
        _w_delc -= /*delcp(i, j)*/ _pen_end;
        if (_w_subc != 0)
        {
          letter1 = _str2 -> at(j-1);
          letter2 = _str1 -> at(i-1);
          _w_subc = EditDistance::confusionMatrix[letter1][letter2] - _pen_end;
        }
      }
      else
      // Error in the beginning of the word(s)
      if (i <= 2 && j <= 2)
      {
        _w_insc += /*inscp(i, j)*/ _pen_beg;
        _w_delc += /*delcp(i, j)*/ _pen_beg;
        if (_w_subc != 0)
        {
          letter1 = _str2 -> at(j-1);
          letter2 = _str1 -> at(i-1);
          _w_subc = EditDistance::confusionMatrix[letter1][letter2] + _pen_beg;
        }
      }
      // Error in the middle of the word(s)
      else
      {
        // _w_insc = _w_ins; //inscp(i, j);
        // _w_delc = _w_del; //delcp(i, j);
        if (_w_subc != 0)
        {
          letter1 = _str2 -> at(j-1);
          letter2 = _str1 -> at(i-1);
          _w_subc = EditDistance::confusionMatrix[letter1][letter2];
        }
      }
      if (i > 1 && j > 1)
      {
        // Include transposition error in the cost
        if (_str1 -> at(i-2) == _str2 -> at(j-1) && _str1 -> at(i-1)
            == _str2 -> at(j-2))
          _costMatrix[i][j] = min(_costMatrix[i-1][j] + _w_delc,
              _costMatrix[i][j-1] + _w_insc, _costMatrix[i-1][j-1] + _w_subc,
              _costMatrix[i-2][j-2] + _w_transp);
        else
          _costMatrix[i][j] = min(_costMatrix[i-1][j] + _w_delc,
              _costMatrix[i][j-1] + _w_insc, _costMatrix[i-1][j-1] + _w_subc);
      }
      else
        // There can't be a transposition error
        _costMatrix[i][j] = min(_costMatrix[i-1][j] + _w_delc,
            _costMatrix[i][j-1] + _w_insc, _costMatrix[i-1][j-1] + _w_subc);
    }
  }

  _old_str = _str1;
  _old_str2 = _str2;

  _timer.stop();
  _microseconds += _timer.usecs();
  // If normalized (norm = true)
  if (norm)
  {
    if (!_sim)
      return _costMatrix[_m][_n] / max(_m, _n);
    else
      return 1 - _costMatrix[_m][_n] / max(_m, _n);
  }
  // "Not normalized" distance (norm = false)
  else
    return _costMatrix[_m][_n];
}

string EditDistance::describe()
{
  return "Modified Levenshtein Distance";
}

void EditDistance::setProfile(int _profile)
{
  switch (_profile)
  {
    case 1 :  // OCR
    {
      _pen_beg  = 0.05f;
      _pen_end  = 0.025f;
      _w_ins    = 1.0f;
      _w_del    = 1.0f;
      _w_sub    = 1.5f;
      _w_transp = 1.1f;
//    Clustering.pho_pen = 0.05f;
      delta    = 0.075f;
      initQUERTY(1.5f, 1.5f);
      break;
    }
    case 0 :  // Keyboard model
    {
      _pen_beg  = 0.05f;
      _pen_end  = 0.05f;
      _w_ins    = 1.0f;
      _w_del    = 1.0f;
      _w_sub    = 1.1f;
      _w_transp = 1.0f;
//    Clustering.pho_pen = 0.05f;
      delta    = 0.075f;
      initQUERTY(1.0f, 1.5f);
      break;
    }
    case 2 :  // plain
    {
      _pen_beg  = 0.0f;
      _pen_end  = 0.0f;
      _w_ins    = 1.0f;
      _w_del    = 1.0f;
      _w_sub    = 1.0f;
      _w_transp = 100.0f;
//    Clustering.pho_pen = 0.0f;
      delta    = 0.075f;
      for (int i = 0; i < LETTERS_NO; i++)
        for (int j = 0; j< LETTERS_NO; j++)
        {
          EditDistance::confusionMatrix[i][j] = 1;
          EditDistance::confusionMatrix[j][i] = 1;
        }
      EditDistance::init = true;
      break;
    }
  }
}

double EditDistance::min(const double a, const double b,
                         const double c, const double d)
{
  double min = a;
  if (min > b)
    min = b;
  if (min > c)
    min = c;
  if (min > d)
    min = d;
  return min;
}

double EditDistance::min(const double a, const double b, const double c)
{
  double min = a;
  if (min > b)
    min = b;
  if (min > c)
    min = c;
  return min;
}

double EditDistance::min(const double a, const double b)
{
  if (a > b)
    return b;
  else
    return a;
}

double EditDistance::max(const double a, const double b)
{
  if (a > b)
    return a;
  else
    return b;
}

int powInt(int x, int n)
{
  if (n == 0)
    return 1;
  else
    return x * powInt(x, n-1);
}

qGramDict::qGramDict()
{
  this -> _items = 0;
}

qGramDict::qGramDict(const int q)
{
  this -> _items = 0;
  initialize(q);
}

void qGramDict::initialize(const int q)
{
  this -> _q = q;
  this -> _count = 0;
  this -> _size = powInt(_VEC_SIZE, _q);
  if (_items != 0)
    delete[] _items;
  this -> _items = new bool[_size];
}

int qGramDict::toInt(const string& qgram)
{
  int c = 0;
  for (int j = 0; j < _q; j++)
    c += powInt(_VEC_SIZE, _q-j-1) * static_cast<uint32_t>(qgram[j]);
  if (c > _size || c < 0)
  {
//  cerr << "Bad q-gram id: " << c << endl;
    return -1;
  }
  return c;
}

bool qGramDict::find(const string &qgram)
{
  int i = toInt(qgram);
  if (i != -1)
    return _items[i];
  return false;
}

void qGramDict::set(const string &qgram)
{
  int i = toInt(qgram);
  if (i != -1 && !(_count > _CAPACITY))
  {
    if (_items[i] == false)
    {
      _items[i] = true;
      _qg[_count ++] = i;
    }
  }
}

void qGramDict::unset(const string &qgram)
{
  int i = toInt(qgram);
  if (i != -1)
  {
    if (_items[i] == true)
    {
      _items[i] = false;
      _count--;
    }
  }
}

void qGramDict::clear()
{
  for (int i = 0; i < _count; i++)
  {
    _items[_qg[i]] = false;
  }
  _count = 0;
}

int qGramDict::count()
{
  return _count;
}

qGramDict::~qGramDict()
{
  delete[] _items;
}

QGramDistances::QGramDistances(const int q, const int w)
{
    this -> _q = q;
    this -> _w = w;
    _qd1.initialize(q);
    _qd2.initialize(q);
    _qd3.initialize(q);
}

int QGramDistances::intersect(const string& str1, const string& str2)
{
  _qd1.clear();
  _qd2.clear();
  _qd3.clear();
  for (uint32_t i = 0; i < str1.length() - _q + 1; i++)
  {
    string qgram = str1.substr(i, _q);
    _qd1.set(qgram);
    _qd3.set(qgram);
  }
  for (uint32_t i = 0; i < str2.length() - _q + 1; i++)
  {
    string qgram = str2.substr(i, _q);
    _qd2.set(qgram);
    _qd3.set(qgram);
  }
  _x1 = _qd1.count();
  _x2 = _qd2.count();
  _x3 = _qd3.count();
  return _x1 + _x2 - _x3;
}

double QGramDistances::calculate(const string& str1, const string& str2)
{
    if (_w == 0)
      return 1 - static_cast<double>(intersect(str1, str2)) / _x3;
    if (_w == 1)
      return 1 - static_cast<double>(intersect(str1, str2))
      / pow(static_cast<double>(_x1 * _x2), 0.5);
    if (_w == 2)
    {
      intersect(str1, str2);
      return 2 * _x3 - (_x1 + _x2);
    }
    intersect(str1, str2);
    return 1 - static_cast<double>(2 * (_x1 + _x2 - _x3)) / (_x1 + _x2);
}

double QGramDistances::calculate(const string& str1,
                                 const string& str2, bool norm)
{
  return calculate(str1, str2);
}

void QGramDistances::setQ(const int q)
{
  this -> _q = q;
}

string QGramDistances::describe()
{
  if (_w == 0)
    return ("Jaccard distance");
  if (_w == 1)
    return ("Cosine distance");
  if (_w == 2)
    return ("Ukkonen aprox. un.");
  if (_w == 3)
    return ("Ukkonen aprox. no.");
  return ("?");
}

void EditDistance::initQUERTY(double n, double nn)
{
  for (int i = 0; i < LETTERS_NO; i++)
    for (int j = 0; j< LETTERS_NO; j++)
    {
      EditDistance::confusionMatrix[i][j] = nn;
      EditDistance::confusionMatrix[j][i] = nn;
    }
  string layout[LETTERS_NO];
  layout[(uint32_t)'q'] =  "wedsa";
  layout[(uint32_t)'w'] =  "qasde";
  layout[(uint32_t)'e'] =  "wsdfr";
  layout[(uint32_t)'r'] =  "edfgt";
  layout[(uint32_t)'t'] =  "rfghy";
  layout[(uint32_t)'y'] =  "tghju";
  layout[(uint32_t)'u'] =  "yhjki";
  layout[(uint32_t)'i'] =  "ujklo";
  layout[(uint32_t)'o'] =  "iklp";
  layout[(uint32_t)'p'] =  "ol";
  layout[(uint32_t)'a'] =  "qwsxz";
  layout[(uint32_t)'s'] =  "qwedcxza";
  layout[(uint32_t)'d'] =  "werfsxcv";
  layout[(uint32_t)'f'] =  "ertdgcvb";
  layout[(uint32_t)'g'] =  "rtyfhvbn";
  layout[(uint32_t)'h'] =  "tyugjbnm";
  layout[(uint32_t)'j'] =  "yuihknm";
  layout[(uint32_t)'k'] =  "uioljm";
  layout[(uint32_t)'l'] =  "pokm";
  layout[(uint32_t)'z'] =  "asdx";
  layout[(uint32_t)'x'] =  "zasdc";
  layout[(uint32_t)'c'] =  "xsdfv";
  layout[(uint32_t)'v'] =  "cdfgb";
  layout[(uint32_t)'b'] =  "vfghn";
  layout[(uint32_t)'n'] =  "bghjm";
  layout[(uint32_t)'m'] =  "nhjkl";
  for (uint32_t i = 0; i <= 10; i++)
  {
    for (uint32_t j = 0; j < layout[i].length(); j++)
    {
      EditDistance::confusionMatrix[i][(uint32_t)layout[i][j]] = n;
    }
  }
  EditDistance::init = true;
}
}
