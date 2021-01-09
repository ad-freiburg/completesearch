// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright Jens Hoffmann 2010

#include <stdio.h>
#include <gtest/gtest.h>
#include "./codes.h"

// _____________________________________________________________________________
void printBitsOfInteger(int code)
{
  int i;
  int bit = 0;
  printf("Bits of %d: ", code);
  for (i = 0; i < 32; ++i)
  {
    bit = (code & 0x80000000) >> 31;
    printf("%d ", bit);
    code = code << 1;
  }
  printf("\n");
}

// _____________________________________________________________________________
TEST(codesTest, Simple9_enc)
{
  int mask1 = 0x01;
  int mask2 = 0x03;
  int mask3 = 0x07;
  int mask4 = 0x0F;
  int mask5 = 0x1F;
  int mask7 = 0x7F;
  int mask9 = 0x01FF;
  int mask14 = 0x3FFF;
  int mask28 = 0x0FFFFFFF;
  unsigned int code[128];
  unsigned int usedIntegers;

  // Next 28 values fit into one.
  {
    int cleartext[] = { 1, 1, 1, 0,
                        0, 1, 0, 1,
                        1, 1, 1, 0,
                        0, 0, 0, 0,
                        0, 1, 1, 0,
                        1, 1, 0, 0,
                        1, 1, 0, 1
                      };
    usedIntegers = Simple9_enc(28, cleartext, code);
    ASSERT_EQ((unsigned) 1, usedIntegers);
    ASSERT_EQ((unsigned) 1, code[0] >>  0 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >>  1 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >>  2 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >>  3 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >>  4 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >>  5 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >>  6 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >>  7 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >>  8 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >>  9 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 10 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 11 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 12 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 13 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 14 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 15 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 16 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 17 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 18 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 19 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 20 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 21 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 22 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 23 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 24 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 25 & mask1);
    ASSERT_EQ((unsigned) 0, code[0] >> 26 & mask1);
    ASSERT_EQ((unsigned) 1, code[0] >> 27 & mask1);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 8, code[0] >> 28 & mask4);
  }
  // Next 14 values fit into one.
  {
    int cleartext[] = { 2, 3, 3, 1,
                        0, 2, 0, 2,
                        3, 1, 1, 2,
                        0, 1
                      };
    usedIntegers = Simple9_enc(14, cleartext, code);
    ASSERT_EQ((unsigned) 1, usedIntegers);
    ASSERT_EQ((unsigned) 2, code[0] >>  0 & mask2);
    ASSERT_EQ((unsigned) 3, code[0] >>  2 & mask2);
    ASSERT_EQ((unsigned) 3, code[0] >>  4 & mask2);
    ASSERT_EQ((unsigned) 1, code[0] >>  6 & mask2);
    ASSERT_EQ((unsigned) 0, code[0] >>  8 & mask2);
    ASSERT_EQ((unsigned) 2, code[0] >> 10 & mask2);
    ASSERT_EQ((unsigned) 0, code[0] >> 12 & mask2);
    ASSERT_EQ((unsigned) 2, code[0] >> 14 & mask2);
    ASSERT_EQ((unsigned) 3, code[0] >> 16 & mask2);
    ASSERT_EQ((unsigned) 1, code[0] >> 18 & mask2);
    ASSERT_EQ((unsigned) 1, code[0] >> 20 & mask2);
    ASSERT_EQ((unsigned) 2, code[0] >> 22 & mask2);
    ASSERT_EQ((unsigned) 0, code[0] >> 24 & mask2);
    ASSERT_EQ((unsigned) 1, code[0] >> 26 & mask2);
    // The folowing bit is unused but should be 0.
    ASSERT_EQ((unsigned) 0, code[0] >> 27 & mask1);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 7, code[0] >> 28 & mask4);
  }
  // Next 9 values fit into one.
  {
    int cleartext[] = { 2, 7, 4, 6,
                        0, 2, 3, 5,
                        1
                      };
    usedIntegers = Simple9_enc(9, cleartext, code);
    ASSERT_EQ((unsigned) 1, usedIntegers);
    ASSERT_EQ((unsigned) 2, code[0] >>  0 & mask3);
    ASSERT_EQ((unsigned) 7, code[0] >>  3 & mask3);
    ASSERT_EQ((unsigned) 4, code[0] >>  6 & mask3);
    ASSERT_EQ((unsigned) 6, code[0] >>  9 & mask3);
    ASSERT_EQ((unsigned) 0, code[0] >> 12 & mask3);
    ASSERT_EQ((unsigned) 2, code[0] >> 15 & mask3);
    ASSERT_EQ((unsigned) 3, code[0] >> 18 & mask3);
    ASSERT_EQ((unsigned) 5, code[0] >> 21 & mask3);
    ASSERT_EQ((unsigned) 1, code[0] >> 24 & mask3);
    // The folowing bit is unused but should be 0.
    ASSERT_EQ((unsigned) 0, code[0] >> 27 & mask1);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 6, code[0] >> 28 & mask4);
  }
  // Next 7 values fit into one.
  {
    int cleartext[] = { 15, 7, 4, 6,
                        12, 13, 1
                      };
    usedIntegers = Simple9_enc(7, cleartext, code);
    ASSERT_EQ((unsigned) 1, usedIntegers);
    ASSERT_EQ((unsigned) 15, code[0] >>  0 & mask4);
    ASSERT_EQ((unsigned) 7,  code[0] >>  4 & mask4);
    ASSERT_EQ((unsigned) 4,  code[0] >>  8 & mask4);
    ASSERT_EQ((unsigned) 6,  code[0] >> 12 & mask4);
    ASSERT_EQ((unsigned) 12, code[0] >> 16 & mask4);
    ASSERT_EQ((unsigned) 13, code[0] >> 20 & mask4);
    ASSERT_EQ((unsigned) 1,  code[0] >> 24 & mask4);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 5, code[0] >> 28 & mask4);
  }
  // Next 5 values fit into one.
  {
    int cleartext[] = { 7, 31, 12, 21,
                        30
                      };
    usedIntegers = Simple9_enc(5, cleartext, code);
    ASSERT_EQ((unsigned) 1,  usedIntegers);
    ASSERT_EQ((unsigned) 7,  code[0] >>  0 & mask5);
    ASSERT_EQ((unsigned) 31, code[0] >>  5 & mask5);
    ASSERT_EQ((unsigned) 12, code[0] >> 10 & mask5);
    ASSERT_EQ((unsigned) 21, code[0] >> 15 & mask5);
    ASSERT_EQ((unsigned) 30, code[0] >> 20 & mask5);
    // The folowing 3 bits are unused but should be 0.
    ASSERT_EQ((unsigned) 0,  code[0] >> 25 & mask3);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 4, code[0] >> 28 & mask4);
  }
  // Next 4 values fit into one.
  {
    int cleartext[] = { 7, 31, 127, 21 };
    usedIntegers = Simple9_enc(4, cleartext, code);
    ASSERT_EQ((unsigned) 1,   usedIntegers);
    ASSERT_EQ((unsigned) 7,   code[0] >>  0 & mask7);
    ASSERT_EQ((unsigned) 31,  code[0] >>  7 & mask7);
    ASSERT_EQ((unsigned) 127, code[0] >> 14 & mask7);
    ASSERT_EQ((unsigned) 21,  code[0] >> 21 & mask7);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 3,   code[0] >> 28 & mask4);
  }
  // Next 3 values fit into one.
  {
    int cleartext[] = { 7, 255, 127 };
    usedIntegers = Simple9_enc(3, cleartext, code);
    ASSERT_EQ((unsigned) 1,   usedIntegers);
    ASSERT_EQ((unsigned) 7,   code[0] >>  0 & mask9);
    ASSERT_EQ((unsigned) 255, code[0] >>  9 & mask9);
    ASSERT_EQ((unsigned) 127, code[0] >> 18 & mask9);
    // The folowing bit is unused but should be 0.
    ASSERT_EQ((unsigned) 0,   code[0] >> 25 & mask1);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 2,   code[0] >> 28 & mask4);
  }
  // Next 2 values fit into one.
  {
    int cleartext[] = { mask14, 1 };
    usedIntegers = Simple9_enc(2, cleartext, code);
    ASSERT_EQ((unsigned) 1,      usedIntegers);
    ASSERT_EQ((unsigned) mask14, code[0] >>  0 & mask14);
    ASSERT_EQ((unsigned) 1,      code[0] >> 14 & mask14);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 1,      code[0] >> 28 & mask4);
  }
  // Next 1 value fits into one.
  {
    int cleartext[] = { mask28 };
    usedIntegers = Simple9_enc(1, cleartext, code);
    ASSERT_EQ((unsigned) 1,      usedIntegers);
    ASSERT_EQ((unsigned) mask28, code[0] >>  0 & mask28);
    // The last 4 bits tell how this code was partitioned.
    ASSERT_EQ((unsigned) 0,      code[0] >> 28 & mask4);
  }
  // Next 2 values, then next 3 values, then 1 remaining fit into three.
  {
    int cleartext[] = { mask14, 1, mask9 - 1, 1, 1, 0 };
    usedIntegers = Simple9_enc(6, cleartext, code);
    ASSERT_EQ((unsigned) 3, usedIntegers);
    ASSERT_EQ((unsigned) mask14,  code[0] >>  0 & mask14);
    ASSERT_EQ((unsigned) 1,       code[0] >> 14 & mask14);
    ASSERT_EQ((unsigned) 1,       code[0] >> 28 & mask4);
    ASSERT_EQ((unsigned) mask9-1, code[1] >>  0 & mask9);
    ASSERT_EQ((unsigned) 1,       code[1] >>  9 & mask9);
    ASSERT_EQ((unsigned) 1,       code[1] >> 18 & mask9);
    ASSERT_EQ((unsigned) 2,       code[1] >> 28 & mask4);
    ASSERT_EQ((unsigned) 0,       code[2] >> 0  & mask28);
    // For the last trailing bit a 1 Bit encoding can be used.
    ASSERT_EQ((unsigned) 8,       code[2] >> 28 & mask4);
  }
}

// _____________________________________________________________________________
int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

