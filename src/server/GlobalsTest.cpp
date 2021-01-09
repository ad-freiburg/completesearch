#include "Globals.h"
#include <gtest/gtest.h>

TEST(GlobalsTest, AddCdataTagIfNecessary)
{
  ASSERT_EQ("Normal", addCdataTagIfNecessary("Normal", false));
  ASSERT_EQ(" ", addCdataTagIfNecessary(" ", false));
  ASSERT_EQ("", addCdataTagIfNecessary("", false));
  ASSERT_EQ("<![CDATA[this < that]]>", addCdataTagIfNecessary("this < that", false));
  ASSERT_EQ("<![CDATA[this > that]]>", addCdataTagIfNecessary("this > that", false));
  ASSERT_EQ("<![CDATA[this & that]]>", addCdataTagIfNecessary("this & that", false));
  ASSERT_EQ("<![CDATA[This is evil! ]}>]]>", addCdataTagIfNecessary("This is evil! ]]>", false));
  ASSERT_EQ("<![CDATA[ctrl chars: \t\x19]]>", addCdataTagIfNecessary("ctrl chars: \t\x19", false));
}

TEST(GlobalsTest, replaceWordPartSeparator)
{
  // A string without a wordPartSep should not change.
  string test = "daswandernistdesmüllerslust";
  ASSERT_EQ(test, replaceWordPartSeparatorBackendToFrontend(test));
  ASSERT_EQ(test, replaceWordPartSeparatorFrontendToBackend(test));

  // Default behaviour: wordPartSep (backend separator) == wordPartSepFrontend
  string test1 =   wordPartSep + string("facet")
                 + wordPartSep + string("author")
                 + wordPartSep + string("*");
  ASSERT_EQ(test1, replaceWordPartSeparatorBackendToFrontend(test1));
  ASSERT_EQ(test1, replaceWordPartSeparatorFrontendToBackend(test1));

  // Use some character which is probably not the default char. Moreover it
  // should not occur within the given string.
  string qq = "";
  qq.assign(2, wordPartSep);
  string test3 = qq + "a" + qq + "b" + qq;

  wordPartSep = 'q';

  // Test in case of different characters.
  string test2 = "qfacetqauthorq*";
  ASSERT_EQ(test2, replaceWordPartSeparatorFrontendToBackend(test1));
  ASSERT_EQ(test1, replaceWordPartSeparatorBackendToFrontend(test2));

  // Repeating occurences should not be an issue too.
  string test4 = "qqaqqbqq";
  ASSERT_EQ(test4, replaceWordPartSeparatorFrontendToBackend(test3));
  ASSERT_EQ(test3, replaceWordPartSeparatorBackendToFrontend(test4));

}

TEST(GlobalsTest, decodeHexNumbers)
{
  ASSERT_EQ("a b", decodeHexNumbers("a+b"));
  ASSERT_EQ("ab ", decodeHexNumbers("ab+"));
  ASSERT_EQ(" ab", decodeHexNumbers("+ab"));
  ASSERT_EQ(" ", decodeHexNumbers("+"));

  ASSERT_EQ("a b", decodeHexNumbers("a\%20b"));
  ASSERT_EQ("ab ", decodeHexNumbers("ab\%20"));
  ASSERT_EQ(" ab", decodeHexNumbers("\%20ab"));
  ASSERT_EQ(" ", decodeHexNumbers("%20"));

  ASSERT_EQ("abc", decodeHexNumbers("\%61b%63"));

  ASSERT_EQ("", decodeHexNumbers(""));
  ASSERT_EQ("\%ag", decodeHexNumbers("\%ag"));
  ASSERT_EQ("\%:5", decodeHexNumbers("\%:5"));
  ASSERT_EQ("123 \%a", decodeHexNumbers("123\%20\%a"));
}

TEST(GlobalsTest, encodeQuotedQueryParts)
{
  // Quotes should always be erased.
  ASSERT_EQ("%97%32%98", encodeQuotedQueryParts("\"a b\""));
  // In case of an unclosed quote, the rest should be encoded.
  ASSERT_EQ("$%124%40%41", encodeQuotedQueryParts("$\"|()"));
  ASSERT_EQ("$%124%40%41", encodeQuotedQueryParts("$\"|()\""));
  ASSERT_EQ("%36%124%40%41", encodeQuotedQueryParts("\"$|()\""));
  ASSERT_EQ("%36%99%124%40%97%98%49%48%41", encodeQuotedQueryParts("\"$c|(ab10)\""));
  // Not quoted parts should not be encoded.
  ASSERT_EQ("%36%124%40%41 $|() %36%124%40%41",
            encodeQuotedQueryParts("\"$|()\" $|() \"$|()\""));
  
}

TEST(GlobalsTest, shiftIfInMiddleOfUtf8MultibyteSequence)
{
  ASSERT_EQ((unsigned) 0, shiftIfInMiddleOfUtf8MultibyteSequence("", 0));
  ASSERT_EQ((unsigned) 0, shiftIfInMiddleOfUtf8MultibyteSequence(" ", 0));
  ASSERT_EQ((unsigned) 0, shiftIfInMiddleOfUtf8MultibyteSequence(" ", 1));
  ASSERT_EQ((unsigned) 0, shiftIfInMiddleOfUtf8MultibyteSequence("\xc3\xa4", 0));
  ASSERT_EQ((unsigned) 1, shiftIfInMiddleOfUtf8MultibyteSequence("\xc3\xa4", 1));
  ASSERT_EQ((unsigned) 1, shiftIfInMiddleOfUtf8MultibyteSequence("\xc3\xa4 ", 1));
  ASSERT_EQ((unsigned) 2, shiftIfInMiddleOfUtf8MultibyteSequence("\xc3\xa4\xa4", 1));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
