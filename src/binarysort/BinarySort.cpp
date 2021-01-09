// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#include "./BinarySort.h"
#include <limits.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
// _____________________________________________________________________________
// A Vector of this typ will sorted with stxxl::sort
// It consists four variables of type unsigned int
class sortedType
{
 public:
  unsigned int a;
  unsigned int b;
  unsigned int c;
  unsigned int d;
  // Definition of the smallest element in sortedType
  static sortedType min_value()
  {
    sortedType min;
    min.a = min.b = min.c = min.d = 0;
    return min;
  }
  // Definition of the bigest element in sortedType
  static sortedType max_value()
  {
    sortedType max;
    max.a = max.b = max.c = max.d = UINT_MAX;
    return max;
  }
};
// _____________________________________________________________________________
// Comapare funktion, comparison function, which decides according
// to what criteria to compare two elements of type sortedType
class Cmp
{
 public:
  bool operator()(const sortedType& x, const sortedType& y) const
  {
    // Compare a, if the same, than compare b, if the same, than compare d
    return x.a < y.a ||
          (x.a == y.a && (x.b < y.b || (x.b == y.b && x.d < y.d)));
  }
  // Smallest element
  static sortedType min_value()
  {
    return sortedType::min_value();
  }
  // Bigest element
  static sortedType max_value()
  {
    return sortedType::max_value();
  }
};
// _____________________________________________________________________________
// 7.Oct10 Bjoern: Added equality operator to support unique.
// Operator that checks for equality of tuples
class EqualFourTuple
{
 public:
  bool operator()(const sortedType& x, const sortedType& y) const
  {
    return (x.a == y.a && x.b == y.b && x.c == y.c && x.d == y.d);
  }
};
// _____________________________________________________________________________
// Sort binary File, file must have 4 columns.
// First, second and fourth column will be sorted
void BinarySort::sortBinaryWordsFile(const string& file)
{
  checkFile(file);
  stxxl::syscall_file f(file, stxxl::file::DIRECT | stxxl::file::RDWR);

  // Memory to use
  unsigned memory_to_use = 1024 * 1024 * 1024;
  typedef stxxl::vector<sortedType> vector_type;
  vector_type v(&f);

  // Sort funktion of library stxxl
  stxxl::sort(v.begin(), v.end(), Cmp(), memory_to_use);

  // 7.Oct 10 Bjoern: Added the possibility to eliminate duplicates
  // Eliminate duplicates
  if (_removeDuplicates)
  {
    vector_type::iterator newEnd =
      std::unique(v.begin(), v.end(), EqualFourTuple());
    v.resize(newEnd - v.begin());
  }
}
// _____________________________________________________________________________
// This funktion schow given binary file, that consists 4*n Elements of
// unsigned int. Out is a matrix with n lines and 4 columns
void BinarySort::showBinaryWordsFileInAscii(const string& file)
{
  checkFile(file);
  std::fstream bFile(file.c_str(), std::ios::in | std::ios::binary);
  int buf;

  while ( !bFile.eof() )
  {
    for (int i = 0; i < 4; i++)
    {
      // Reinterpret, that convert readed number from char array in integer
      bFile.read(reinterpret_cast<char *>(&buf), sizeof(buf));

      if ( !bFile.eof() ) cout << buf << " ";
    }
    cout << endl;
  }
}
// _____________________________________________________________________________
void BinarySort::checkFile(const string& file)
{
  std::fstream bFile(file.c_str(), std::ios::in | std::ios::binary);
  if (!bFile.is_open())
  {
    cerr << "File could not be open: " << file << endl;
    exit(1);
  }
  bFile.close();
}
