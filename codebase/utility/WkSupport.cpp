// Copyright 2010, Weitkaemper Technology
//
// Author: Wolfgang Bartsch <weitkaemper>

#include "./WkSupport.h"

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using std::string;
using std::vector;
using std::ifstream;


// _____________________________________________________________________________
size_t putTokenInVector(const string&   sTokString,
                        char            cSep,
                        vector<string>* pvsToken)
{
  string sTmp;
  size_t nc = 0;

  pvsToken->clear();
  string::const_iterator p = sTokString.begin();

  while (p != sTokString.end())
  {
    if (*p != cSep)
    {
      sTmp += *p;
    }
    else
    {
      pvsToken->push_back(sTmp);
      nc++;
      sTmp = "";
    }
    p++;
  }
  pvsToken->push_back(sTmp);

  return ++nc;
}


// _____________________________________________________________________________
bool readFileInVector(const string&   sFilename,
                      const string&   sPrefix,
                      const size_t    nMaxLineLength,
                      vector<string>* pvsFileContent)
{
  FILE* file = fopen(sFilename.c_str(), "r");

  if (file == NULL)
  {
    return false;
  }

  string sLine;
  char*  szLine;
  char*  cptrRet = NULL;

  szLine = new char[nMaxLineLength + 2];

  pvsFileContent->clear();
  while (true)
  {
    cptrRet = fgets(szLine, nMaxLineLength+2, file);
    if (cptrRet == NULL)
    {
      break;
    }
    sLine = szLine;

    if (sLine.find("\n") == string::npos)
    {
      delete[] szLine;
      return false;
    }
    sLine.erase(sLine.length() - 1);
    sLine = replaceLinefeed(sLine);

    if (sLine.find(sPrefix) == 0)
    {
      sLine.erase(0, sPrefix.length());
      pvsFileContent->push_back(sLine);
    }
  }

  delete[] szLine;
  return true;
}


// _____________________________________________________________________________
string intToString(unsigned int n)
{
  string s;
  char   szBuf[128];

  snprintf(szBuf, sizeof(szBuf), "%d", n);
  s = szBuf;
  return s;
}


// _____________________________________________________________________________
string replaceLinefeed(const string& sLine)
{
  string sLineWithoutLinefeed = sLine;
  if (sLineWithoutLinefeed.find("\r") != string::npos)
  {
    // DOS-File under UNIX  delete \r = LF = Line Feed = 10 (0x0A)
    // (DOS-Files have a \r\n sequence (carriage return + line feed)
    // instead of \n)
    sLineWithoutLinefeed.replace(sLine.find("\r"), 1, "");
  }
  return sLineWithoutLinefeed;
}


// _____________________________________________________________________________
string replaceAll(const string& sString,
                  const string& sFind,
                  const string& sReplace)
{
  string sRet = sString;
  size_t nPos = sRet.find(sFind);

  while (nPos != string::npos)
  {
    sRet.replace(nPos, sFind.length(), sReplace);
    nPos = sRet.find(sFind);
  }
  return sRet;
}


// _____________________________________________________________________________
void splitString(const string& sToSplit,
                 const string& sSep,
                 string* sFirst,
                 string* sRest)
{
  string::size_type n = sToSplit.find(sSep);
  if (n == string::npos)
  {
    *sFirst = sToSplit;
    *sRest = "";
  }
  else
  {
    *sFirst = sToSplit.substr(0, n);
    *sRest = sToSplit.substr(n + sSep.length(), string::npos);
  }
}


// _____________________________________________________________________________
void chopTail(char cChop, string* sToChop)
{
  string::iterator p;
  size_t lCount = 0;

  if (sToChop->empty()) return;

  p = sToChop->end();
  --p;
  while (p != sToChop->begin() && *p == cChop)
  {
    lCount++;
    --p;
  }
  if (*p == cChop)
  {
    lCount++;
  }
  sToChop->erase(sToChop->length() - lCount, string::npos);
}


// _____________________________________________________________________________
void chopHead(char cChop, string* sToChop)
{
  string::iterator p;
  size_t lCount = 0;

  if (sToChop->empty()) return;

  p = sToChop->begin();
  while (p != sToChop->end() && *p == cChop)
  {
    lCount++;
    p++;
  }

  sToChop->erase(0, lCount);
}


// _____________________________________________________________________________
void chop(char cChop, string* sToChop)
{
  chopHead(cChop, sToChop);
  chopTail(cChop, sToChop);
}
