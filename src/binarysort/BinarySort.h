// Copyright 2010, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Author: Simon Skilevic <skilevis>

#ifndef BINARYSORT_BINARYSORT_H_
#define BINARYSORT_BINARYSORT_H_

#include <stxxl/io>
#include <stxxl/mng>
#include <stxxl/sort>
#include <stxxl/vector>
#include <string>

using std::string;
using std::cout;
using std::endl;
using std::cerr;

// This class allows time efficiently sorting of large data files with the help
// of stxxl::sort. Sort function is SortBinary::sortBinaryWordsFile.
// As input this function needs a binary file with four integer columns.
// Stxxl needs a virtual disc, which is described in .stxxl (stxxl config file)
// This disc can be created with the help of the tool Creatdisc, wich you can d
// ownload at the site of stxxl library.
// The virtual disk must be greater than the sorted data.
// If this is not the case or the config file is completely missing, then the
// program automatically creates a virtual disc of the needed size. The sorting
// will then take about 20% longer.
// If the input file fits in main memory, then it will be sorted main memory.
// The size of main memory to use can be changed by memory_to_use.
// You can find memory_to_use in the function
// SortBinary::sortBinaryWordsFile in SortBinary.cpp.
// You can also see the contents of the binary file in ascii by using
// SortBinary::schowBinaryWordsFileInAscii.

class BinarySort
{
 public:
  // Default construktor

  BinarySort()
  {
  }

  // Sort binary file, file must have 4 columns.
  // First, second and fourth column will be sorted
  void sortBinaryWordsFile(const string& file);

  // Schow given binary file, that consists 4*n Elements of
  // unsigned int, in Ascii. Out is a matrix with n lines and 4 columns
  void showBinaryWordsFileInAscii(const string& file);

  // Bjoern 07 Oct 10: added an option to remove duplicates
  void setRemoveDuplicates(bool value)
  {
    _removeDuplicates = value;
  }

 private:
  // Flag to be set that determines if duplicates are eliminated during sort
  bool _removeDuplicates;
  // Check if file exists
  void checkFile(const string& file);
};
#endif  // BINARYSORT_BINARYSORT_H_
