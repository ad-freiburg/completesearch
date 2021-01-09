// Copyright 2010, Weitkaemper Technology
//
// Author: Wolfgang Bartsch <weitkaemper>

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "./WkSupport.h"
#include "./StringConverter.h"


using std::cout;
using std::endl;


StringConverter gSC;

// Testing function StringConversion::init
TEST(init, StringConverter)
{
  bool bRet;
  bRet = gSC.init();
  ASSERT_TRUE(bRet);
}

TEST(convert, StringConverter)
{
  vector<string> vs;
  size_t         nRet;

  readFileInVector("utf8.map.test", "StringConverter::convert:", 1000, &vs);
  nRet = vs.size();

  // vs should contain at least 4 lines
  // sOrig, sNorm, sLower and sUpper lines should alternate,
  // that means: the original line comes first, followed by the
  // normalized, lowercased and uppercased versions of the original line
  ASSERT_GT(nRet, (unsigned) 4);

  size_t n = 0;

  string sOrig;
  string sNorm;
  string sLower;
  string sUpper;
  string sRes;

  while (1)
  {
    if (n <= (nRet - 4))
    {
      sOrig = vs[n++];
      sNorm  = vs[n++];
      sLower = vs[n++];
      sUpper = vs[n++];


      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_UTF8,
                         StringConverter::CONVERSION_TO_NORM);
      ASSERT_EQ(sNorm, sRes);


      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_UTF8,
                         StringConverter::CONVERSION_TO_LOWER);
      ASSERT_EQ(sLower, sRes);

      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_UTF8,
                         StringConverter::CONVERSION_TO_UPPER);
      ASSERT_EQ(sUpper, sRes);

      sRes = gSC.convert(sUpper,
                         StringConverter::ENCODING_UTF8,
                          StringConverter::CONVERSION_TO_LOWER);
      ASSERT_EQ(sLower, sRes);

      sRes = gSC.convert(sLower,
                         StringConverter::ENCODING_UTF8,
                         StringConverter::CONVERSION_TO_UPPER);
      ASSERT_EQ(sUpper, sRes);

      sRes = gSC.convert(sUpper,
                         StringConverter::ENCODING_UTF8,
                         StringConverter::CONVERSION_TO_NORM);
      ASSERT_EQ(sNorm, sRes);

      sRes = gSC.convert(sLower,
                         StringConverter::ENCODING_UTF8,
                         StringConverter::CONVERSION_TO_NORM);
      ASSERT_EQ(sNorm, sRes);

      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_UTF8,
                         (StringConverter::Conversion)23);
      ASSERT_EQ(sOrig, sRes);

      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_UTF8,
                         (StringConverter::Conversion)-23);
      ASSERT_EQ(sOrig, sRes);
    }
    else
    {
      break;
    }
  }

  readFileInVector("iso8859-1.map.test", "StringConverter::convert:",
                   1000, &vs);
  nRet = vs.size();


  // vs  should contain at least 4 lines.
  // sOrig, sNorm, sLower and sUpper lines should alternate,
  // that means: the original line comes first, followed by the
  // normalized, lowercased and uppercased versions of the original line
  ASSERT_GT(nRet, (unsigned) 4);

  n = 0;
  while (1)
  {
    if (n <= (nRet - 4))
    {
      sOrig = vs[n++];
      sNorm  = vs[n++];
      sLower = vs[n++];
      sUpper = vs[n++];

      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_NORM);
      ASSERT_EQ(sNorm, sRes);

      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_LOWER);
      ASSERT_EQ(sLower, sRes);

      sRes = gSC.convert(sOrig,
                         StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_UPPER);
      ASSERT_EQ(sUpper, sRes);

      sRes = gSC.convert(sUpper,
                         StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_LOWER);
      ASSERT_EQ(sLower, sRes);

      sRes = gSC.convert(sLower,
                        StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_UPPER);
      ASSERT_EQ(sUpper, sRes);

      sRes = gSC.convert(sUpper,
                         StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_NORM);
      ASSERT_EQ(sNorm, sRes);

      sRes = gSC.convert(sLower,
                         StringConverter::ENCODING_ISO8859_1,
                         StringConverter::CONVERSION_TO_NORM);
      ASSERT_EQ(sNorm, sRes);
    }
    else
    {
      break;
    }
  }
}


TEST(urlEncode, StringConverter)
{
  string sToEncode = " !\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~";
  string sUrlEncoded;

  sUrlEncoded  = "%20%21%22%23%24%25%26%27%28%29%2a%2b%2c-.%2f%3a%3b%3c%3d%3e";
  sUrlEncoded += "%3f%40%5b%5c%5d%5e_%60%7b%7c%7d~";

  string sRes = gSC.urlEncode(sToEncode);
  ASSERT_EQ(sUrlEncoded, sRes);
}

TEST(mask, StringConverter)
{
  string sToEncode = "ab.ü.ba";
  string sEncoded = gSC.convert(sToEncode,
      StringConverter::ENCODING_UTF8,
      StringConverter::CONVERSION_TO_MASK);
  sEncoded = gSC.convert(sEncoded,
      StringConverter::ENCODING_UTF8,
      StringConverter::CONVERSION_FROM_MASK);
  ASSERT_EQ(sToEncode, sEncoded);
}

TEST(convertDate, StringConverter)
{
  string sIn    = "7.6.2010";
  string sResOk = "07-06-2010";
  string sRes;

  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "2010-9-6";
  sResOk = "06-09-2010";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "2010-992-6";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "-----------------";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "aaaaaaaaaaaaaa-aaaaaaaaaaaaaaaaaaaaa";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "10-10-21021";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "10-10-0021";
  sResOk = "10-10-0021";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "0-0";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);

  sIn    = "0-0-";
  sResOk = "";
  sRes = gSC.convertDate(sIn);
  ASSERT_EQ(sResOk, sRes);
}


TEST(replaceNonAlphanum, StringConverter)
{
  vector<string> vs;
  size_t         nRet;
  string         sStringToReplace;
  string         sExpectedResult;
  string         sRes;

  readFileInVector("utf8.map.test",
                   "StringConverter::replaceNonAlphanum:false:",
                   1000, &vs);
  nRet = vs.size();

  ASSERT_GT(nRet, (unsigned) 0);


  size_t n = 0;
  vector<size_t> vnSavedCodepoints;
  while (n < vs.size() - 1)
  {
    sStringToReplace = vs[n++];
    sExpectedResult = vs[n++];

    sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                  StringConverter::ENCODING_UTF8,
                                  '#',
                                  false,
                                  vnSavedCodepoints);
    ASSERT_EQ(sExpectedResult, sRes);

    sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                  StringConverter::ENCODING_UTF8,
                                  '_',
                                  false,
                                  vnSavedCodepoints);
    sRes = replaceAll(sRes, "_", "#");
    ASSERT_EQ(sExpectedResult, sRes);
  }



  readFileInVector("utf8.map.test",
                   "StringConverter::replaceNonAlphanum:true:",
                   1000, &vs);
  nRet = vs.size();
  ASSERT_GT(nRet, (unsigned) 0);

  n = 0;
  while (n < vs.size() - 1)
  {
    sStringToReplace = vs[n++];
    sExpectedResult = vs[n++];

    sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                  StringConverter::ENCODING_UTF8,
                                  '#',
                                  true,
                                  vnSavedCodepoints);
    ASSERT_EQ(sExpectedResult, sRes);

    sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                  StringConverter::ENCODING_UTF8,
                                  '_',
                                  true,
                                  vnSavedCodepoints);
    sRes = replaceAll(sRes, "_", "#");
    ASSERT_EQ(sExpectedResult, sRes);
  }




  // Save SPACE, $ and &:
  vnSavedCodepoints.push_back(32);
  vnSavedCodepoints.push_back(36);
  vnSavedCodepoints.push_back(38);
  sStringToReplace = " !\"#$%&'()*+,-./0123456789:;<=>?@";
  sExpectedResult =  " ###$#&#########0123456789#######";


  sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                StringConverter::ENCODING_UTF8,
                                '#',
                                false,
                                vnSavedCodepoints);
  ASSERT_EQ(sExpectedResult, sRes);

  readFileInVector("iso8859-1.map.test",
                   "StringConverter::replaceNonAlphanum:",
                   1000, &vs);
  nRet = vs.size();

  n = 0;
  vnSavedCodepoints.clear();
  while (n < vs.size() - 1)
  {
    sStringToReplace = vs[n++];
    sExpectedResult = vs[n++];

    sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                  StringConverter::ENCODING_ISO8859_1,
                                  '#',
                                  false,
                                  vnSavedCodepoints);
    ASSERT_EQ(sExpectedResult, sRes);

    sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                  StringConverter::ENCODING_ISO8859_1,
                                  '_',
                                  false,
                                  vnSavedCodepoints);
    sRes = replaceAll(sRes, "_", "#");
    ASSERT_EQ(sExpectedResult, sRes);
  }

  // Save SPACE, $ and &:
  vnSavedCodepoints.push_back(32);
  vnSavedCodepoints.push_back(36);
  vnSavedCodepoints.push_back(38);

  sStringToReplace = " !\"#$%&'()*+,-./0123456789:;<=>?@";
  sExpectedResult =  " ###$#&#########0123456789#######";

  sRes = gSC.replaceNonAlphanum(sStringToReplace,
                                StringConverter::ENCODING_ISO8859_1,
                                '#',
                                false,
                                vnSavedCodepoints);
  ASSERT_EQ(sExpectedResult, sRes);
}

