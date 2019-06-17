#ifndef __CODES_H__
#define __CODES_H__

#include <stdio.h>

// I was here

// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
// NEW 8Feb06 : returns first position in code not yet encoded
int Simple9_dec(unsigned int n,int * cleartext, const unsigned int *code);
// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
// Here, 4,2,3,1,... are decoded to 4,6,9,10
void Simple9_dec_gaps(unsigned int n,int * cleartext, const unsigned int *code);
// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
// If all numbers are > 0 then the output is the same as above
// zeros indicate that a "gapped run" is over and that the next number is to be read "raw"
// In a sequence of zeros only the first zero is not decoded
void Simple9_dec_gaps_with_boundaries(unsigned int n,int * cleartext, const unsigned int *code);
// n = elements to encode, cleartext must have size at least n*sizeof(int) bytes
// code must have sufficient size, for simple9 at most n*sizeof(int) bytes
// return value is number of UNSIGNED INTS used for code
int  Simple9_enc(unsigned int n, const int * cleartext,unsigned int *code);

// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
void Byte_Aligned_dec(unsigned int n,int * cleartext,unsigned int *code);
// n = elements to encode, cleartext must have size at least n*sizeof(int) bytes
// code must have sufficient size, for simple9 at most n*sizeof(int) bytes
// CANNOT COMPRESS ELEMENTS LARGER/EQUAL THAN 2^28
// return value is number of BYTES used for code
int  Byte_Aligned_enc(unsigned int n,int * cleartext,unsigned int *code);

// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
void Elias_Delta_dec(unsigned int n,int * cleartext,unsigned int *code);
// n = elements to encode, cleartext must have size at least n*sizeof(int) bytes
// code must have sufficient size, for Elias_Delta_enc, twice size of cleartext will suffice
// return value is number of BITS used for code
int  Elias_Delta_enc(unsigned int n,int * cleartext,unsigned int *code);

// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
void Elias_Gamma_dec(unsigned int n,int * cleartext,unsigned int *code);
// n = elements to encode, cleartext must have size at least n*sizeof(int) bytes
// code must have sufficient size, for Elias_Delta_enc, twice size of cleartext will suffice
// return value is number of BITS used for code
int  Elias_Gamma_enc(unsigned int n,int * cleartext,unsigned int *code);

// SPACE USE NOT CLEAR
int Golomb_unary_enc(unsigned int n,int * cleartext,unsigned int *code,int q);
void  Golomb_unary_dec(unsigned int n,int * cleartext,unsigned int *code,int q);

// n = elements to decode, compressed length NOT given
// cleartext array must have sufficient size (n*sizeof(int) bytes)
void  Golomb_Gamma_dec(unsigned int n,int * cleartext,unsigned int *code,int q);
// n = elements to encode, cleartext must have size at least n*sizeof(int) bytes
// code must have sufficient size, for Elias_Delta_enc, three times size of cleartext will suffice
// return value is number of BITS used for code
int Golomb_Gamma_enc(unsigned int n,int * cleartext,unsigned int *code,int q);

#endif
