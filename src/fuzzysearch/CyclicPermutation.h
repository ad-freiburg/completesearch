// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#ifndef FUZZYSEARCH_CYCLICPERMUTATION_H_
#define FUZZYSEARCH_CYCLICPERMUTATION_H_

// #undef NDEBUG

#include <stdint.h>
#include <string>
#include <vector>

#include "../fuzzysearch/Utils.h"

namespace FuzzySearch
{
// Class that represents a cyclic permutation (as opposed to having a real
// string). It is used for searching the similar string in the permuted lexicon
// Note: this class does not have getters and setters
template <class T>
class CyclicPermutationWithPointer
{
  public:
    // Pointer to the string of which this object is a cyclic permutation.
    // This pointer is needed for words that do not belong to the correct words
    // i.e. does not have a wordIndex
    const T* wordPtr;

    // How much the original word is shifted to the right to obtain this
    // permutation. For example, if the original word is "house" then for the
    // cyclic permutation "useho" the shift is 2.
    uint32_t shift;

    // even if 8 bit shift is used, the size of the d.s. will still be 8 bytes!
    // uint8_t shift;

    // initializes the string and the shift of the cyclic permutation
    void init(const T& wordPtr, int32_t shift)
    {
      this -> wordPtr = &wordPtr;
      this -> shift = shift;
    }

    // Lexicographic comparison of two permutations of two (not necessarily the
    // same) words from the lexicon.
    static bool comparator(const CyclicPermutationWithPointer& perm1,
                           const CyclicPermutationWithPointer& perm2)
    {
      int pos1, pos2;
      int length1 = perm1.wordPtr -> length();
      int length2 = perm2.wordPtr -> length();
      int minl = MY_MIN(length1, length2);
      for (int j = 0; j < minl; j++)
      {
        pos1 = (j + perm1.shift) % length1;
        pos2 = (j + perm2.shift) % length2;
        if ((*perm1.wordPtr)[pos1] < (*perm2.wordPtr)[pos2])
          return true;
        else
        {
          if ((*perm1.wordPtr)[pos1] > (*perm2.wordPtr)[pos2])
            return false;
        }
      }
      if (length1 > length2)
        return false;
      else
        return true;
    }

    // Converts the CyclicPermutation into a string (the argument)
    void getPerm(T* s)
    {
      (*s).resize(wordPtr -> length());
      for (size_t j = 0; j < wordPtr -> length(); j++)
      {
        (*s)[j] = wordPtr -> at((j+shift) % wordPtr -> length());
      }
    }

    // Converts the CyclicPermutation into a string (the argument) and returns
    // that as a string
    T getPermAsString() const
    {
      T s;
      s.resize(wordPtr -> length());
      for (size_t j = 0; j < wordPtr -> length(); j++)
      {
         s[j] = wordPtr -> at((j+shift) % wordPtr -> length());
      }
      return s;
    }
};


// Class that represents a more generalized cyclic permutation i.e. the
// permutation is defined on a substring of the string that points to
// (see above)
template <class T>
class CyclicPermutationSubstringWithIndex
{
  public:

    // Index of the word (in the list of correct words) this cyclic permutation
    // corresponds to
    uint32_t wordIndex;

    // all words that contain this prefix (experimental)
    // vector<int> words;

    int wordGroupIndex;

    // How much the original word is shifted to the right to obtain this
    // permutation. For example, if the original word is "house" then for the
    // cyclic permutation "useho" the shift is 2.
    uint8_t shift;

    // at which position it begins
    uint8_t beginning;

    // at which position it ends
    uint8_t len;

    // pointer to the list of correct word where wordIndex is the index
    static const vector<T>* strings;

    // experimental
    // static const vector<vector<int> >* wordIds;

    // initializes the string and the shift of the cyclic permutation
    void init(uint32_t wordId, uint8_t shift, uint8_t beginning, uint8_t len)
    {
      this->wordIndex = wordId;
      this->shift = shift;
      this->beginning = beginning;
      this->len = len;
      wordGroupIndex = -1;
    }

    // returns true if the two words are equal (not the permutations!)
    // bool isEqualTo(const CyclicPermutationWithPointer<T>& perm)
    // {
    //  return ((*strings)[wordIndex] == (*perm.wordPtr));
    // }

    // Lexicographic comparison of two permutations of two (not necessarily the
    // same) words from the lexicon.
    static bool comparator(const CyclicPermutationSubstringWithIndex& perm1,
                           const CyclicPermutationSubstringWithIndex& perm2)
    {
      int pos1, pos2;
      int minl = MY_MIN(perm1.len, perm2.len);
      int wordIndex1 = perm1.wordIndex;
      int wordIndex2 = perm2.wordIndex;
      for (int j = 0; j < minl; j++)
      {
        pos1 = (j + perm1.beginning + perm1.shift) % perm1.len;
        pos2 = (j + perm2.beginning + perm2.shift) % perm2.len;

        if ((*strings)[wordIndex1][pos1] < (*strings)[wordIndex2][pos2])  // NOLINT
          return true;
        else
        {
          if ((*strings)[wordIndex1][pos1] > (*strings)[wordIndex2][pos2])  // NOLINT
            return false;
        }
      }

      if (perm1.len > perm2.len)
        return false;
      else
        return true;
    }

    // Lexicographic comparison of two permutations of two (not necessarily the
    // same) words from the lexicon.
    static bool comparator1(
                    const CyclicPermutationWithPointer<T>& perm1,
                    const CyclicPermutationSubstringWithIndex<T>& perm2)
    {
      int pos1, pos2;
      int length1 = perm1.wordPtr->length();
      int length2 = perm2.len;
      int wordIndex = perm2.wordIndex;
      int minl = MY_MIN(length1, length2);
      for (int j = 0; j < minl; j++)
      {
        pos1 = Mod::calc((j + perm1.shift), length1);
        pos2 = Mod::calc((j + perm2.beginning + perm2.shift), length2);
        if ((*perm1.wordPtr)[pos1] < (*strings)[wordIndex][pos2])
          return false;
        else
        {
          if ((*perm1.wordPtr)[pos1] > (*strings)[wordIndex][pos2])
            return true;
        }
      }
      if (length1 > length2)
        return true;
      else
        return false;
    }

    // Converts the CyclicPermutation into a string (the argument)
    // void getPerm(T* s)
    // {
    //  (*s).resize(len);
    //  for (unsigned int j = 0; j < len; j++)
    //  {
    //    (*s)[j] = (*strings)[wordIndex][(j+beginning+shift) % len]; // NOLINT
    //  }
    // }

    // Converts the CyclicPermutation into a string (the argument) and returns
    // that as a string
    T getPermAsString() const
    {
      T s;
      s.resize(len);
      for (unsigned int j = 0; j < len; j++)
      {
        s[j] = (*strings)[wordIndex][(j+beginning+shift) % len]; // NOLINT
      }
      return s;
    }
};

// Class that represents a cyclic permutation (as opposed to having a real
// string). It is used for constructing the permuted lexicon
// Note: this class does not have getters and setters
template <class T>
class CyclicPermutationWithIndex
{
  public:

    // Index of the word (in the list of correct words) this cyclic permutation
    // corresponds to
    uint32_t wordIndex;

    // How much the original word is shifted to the right to obtain this
    // permutation. For example, if the original word is "house" then for the
    // cyclic permutation "useho" the shift is 2.
    uint32_t shift;

    // even if 8 bit shift is used, the size of the d.s. will still be 8 bytes!
    // uint8_t shift;

    // pointer to the list of correct word where wordIndex is the index
    static const vector<T>* strings;

    // initializes the string and the shift of the cyclic permutation
    void init(uint32_t wordId, uint32_t shift)
    {
      this -> wordIndex = wordId;
      this -> shift = shift;
    }

    // returns true if the two words are equal (not the permutations!)
    bool isEqualTo(const CyclicPermutationWithPointer<T>& perm)
    {
      return ((*strings)[wordIndex] == (*perm.wordPtr));
    }

    // Lexicographic comparison of two permutations of two (not necessarily the
    // same) words from the lexicon.
    static bool comparator(const CyclicPermutationWithIndex& perm1,
                           const CyclicPermutationWithIndex& perm2)
    {
      // assert(perm1.wordIndex < strings -> size());
      // assert(perm2.wordIndex < strings -> size());

      int length1 = (*strings)[perm1.wordIndex].length();
      int length2 = (*strings)[perm2.wordIndex].length();

      int pos1, pos2;
      int minl = MY_MIN(length1, length2);
      for (int j = 0; j < minl; j++)
      {
        pos1 = (j + perm1.shift) % length1;
        pos2 = (j + perm2.shift) % length2;

        if ((*strings)[perm1.wordIndex][pos1] < (*strings)[perm2.wordIndex][pos2])  // NOLINT
          return true;
        else
        {
          if ((*strings)[perm1.wordIndex][pos1] > (*strings)[perm2.wordIndex][pos2])  // NOLINT
            return false;
        }
      }

      if (length1 > length2)
        return false;
      else
        return true;
    }

    // Lexicographic comparison of two permutations of two (not necessarily the
    // same) words from the lexicon.
    static bool comparator1(const CyclicPermutationWithPointer<T>& perm1,
                            const CyclicPermutationWithIndex<T>& perm2)
    {
      int pos1, pos2;
      int length1 = perm1.wordPtr->length();
      int length2 = (*strings)[perm2.wordIndex].length();
      int minl = MY_MIN(length1, length2);
      for (int j = 0; j < minl; j++)
      {
        pos1 = Mod::calc((j + perm1.shift), length1);
        pos2 = Mod::calc((j + perm2.shift), length2);
        if ((*perm1.wordPtr)[pos1] < (*strings)[perm2.wordIndex][pos2])
          return false;
        else
        {
          if ((*perm1.wordPtr)[pos1] > (*strings)[perm2.wordIndex][pos2])
            return true;
        }
      }
      if (length1 > length2)
        return true;
      else
        return false;
    }

    // Converts the CyclicPermutation into a string (the argument)
    void getPerm(T* s)
    {
      (*s).resize((*strings)[wordIndex].length());
      for (unsigned int j = 0; j < (*strings)[wordIndex].length(); j++)
      {
        (*s)[j] = (*strings)[wordIndex][(j+shift) % (*strings)[wordIndex].length()]; // NOLINT
      }
    }

    // Converts the CyclicPermutation into a string (the argument) and returns
    // that as a string
    T getPermAsString() const
    {
      T s;

      s.resize((*strings)[wordIndex].length());
      for (unsigned int j = 0; j < (*strings)[wordIndex].length(); j++)
      {
        s[j] = (*strings)[wordIndex][(j+shift) % (*strings)[wordIndex].length()]; // NOLINT
      }
      return s;
    }
};
}

#endif  // FUZZYSEARCH_CYCLICPERMUTATION_H_
