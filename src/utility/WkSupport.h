// Copyright 2010, Weitkaemper Technology
//
// Author: Wolfgang Bartsch <weitkaemper>
//
// Contains some support functions
//

#ifndef UTILITY_WKSUPPORT_H_
#define UTILITY_WKSUPPORT_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using std::string;
using std::vector;
using std::ifstream;


// Tokenizes the string sTokString by the separator character cSep.
// The tokens are written to vector pvsToken.
//
// Sample usage:
// string s = "#eins#zwei#"
// vector<string> vs;
// size_t nNumbOfToken;
// nNumbOfToken = putTokenInVector(s, '#', &vs);
// // nNumbOfToken is 4
// // vs[0] is "", vs[1] is "eins", vs[2] is "zwei", vs[3] is ""
//
size_t putTokenInVector(const string&   sTokString,
                        char            cSep,
                        vector<string>* pvsToken);

// Reads the content of file named sFilename and writes every line
// starting with string sPrefix into the vector pvsToken, but without
// the prefix.
// nMaxLineLength must greater or equal to the  maximum size of a
// line in the file.
// Return value is false if the file could not be opened or if a line
// in the file is bigger than nMaxLineLength
// Usage example:
// vector<string> vsFileContent;
// readFileInVector("test.txt", "test:", 1000, &vsFileContent);
// After that, vsFileContent contains every line from file test.txt
// that starts with prefix "test:" but without the prefix "test:"
//
bool readFileInVector(const string&   sFilename,
                      const string&   sPrefix,
                      const size_t    nMaxLineLength,
                      vector<string>* pvsFileContent);

// Converts unsigned int value to a string
string intToString(unsigned int n);

// In case of reading a DOS-File under UNIX:
// Delete \r = LF = Line Feed = 10 (0x0A)
// (DOS-Files have a \r\n sequence (carriage return + line feed)
// instead of \n)
string replaceLinefeed(const string& sLine);

// Replaces all occurrences of sFind in sString by sReplace
string replaceAll(const string& sString,
                  const string& sFind,
                  const string& sReplace);

// Splits string sToSplit by the first occurrence of string sSep
// into the strings sFirst and sRest
void splitString(const string& sToSplit,
                 const string& sSep,
                 string* sFirst,
                 string* sRest);

void chopTail(char cChop, string* sToChop);
void chopHead(char cChop, string* sToChop);
void chop(char cChop, string* sToChop);


#endif  // UTILITY_WKSUPPORT_H_
