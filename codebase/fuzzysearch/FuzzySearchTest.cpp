// Copyright [2009] <Marjan Celikik>  [legal/copyright]

#include <gtest/gtest.h>  // NOLINT
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "../fuzzysearch/FuzzySearchAlgorithm.h"
#include "../fuzzysearch/FastSS.h"
#include "../fuzzysearch/PermutedLexicon.h"
#include "../fuzzysearch/Utils.h"
#include "../fuzzysearch/StringDistances.h"
#include "../fuzzysearch/WordClusteringBuilder.h"

using std::string;
using std::wstring;
using std::cout;
using std::endl;
using std::vector;

using FuzzySearch::FastSS;
using FuzzySearch::PermutedLexicon;
using FuzzySearch::PlainEditDistance;
using FuzzySearch::ExtensionEditDistance;
using FuzzySearch::CyclicPermutationWithIndex;
using FuzzySearch::WordClusteringBuilder;

// test the word normalization functions
// when the normalization function changes
// this test should too
TEST(FuzzySearchTest, word_normalization_test)
{
  vector<string> testVocabulary;
  testVocabulary.push_back("chshysh");
  testVocabulary.push_back("marjan");
  testVocabulary.push_back("qwertrewq");
  testVocabulary.push_back("123456789");
  testVocabulary.push_back("ckaaashhsue");
  testVocabulary.push_back("");
  testVocabulary.push_back("ueoeaessss");
  testVocabulary.push_back("philosophy");
  testVocabulary.push_back("tttt");
  WordClusteringBuilder<string>::normalizeVocabulary(testVocabulary,
      &testVocabulary);
  ASSERT_EQ(9u, testVocabulary.size());
  ASSERT_EQ("csis", testVocabulary[0]);
  ASSERT_EQ("marjan", testVocabulary[1]);
  ASSERT_EQ("qwertrewq", testVocabulary[2]);
  ASSERT_EQ("123456789", testVocabulary[3]);
  ASSERT_EQ("kaashsu", testVocabulary[4]);
  ASSERT_EQ("", testVocabulary[5]);
  ASSERT_EQ("uoass", testVocabulary[6]);
  ASSERT_EQ("filosofi", testVocabulary[7]);
  ASSERT_EQ("tt", testVocabulary[8]);
}

// tests the used string distance (now only PlainEditDistance
// and extension distance are supported)
TEST(FuzzySearchTest, string_distances_test)
{
  // test on ascii strings
  PlainEditDistance distance;
  double d;
  d = distance.calculate("algorithm", "alogrithm", 3);
  ASSERT_EQ(static_cast<double>(2.0), d);
  d = distance.calculate("algorthm", "algorithm", 3);
  ASSERT_EQ(static_cast<double>(1.0), d);
  d = distance.calculate("algoritthm", "algorithm", 3);
  ASSERT_EQ(static_cast<double>(1.0), d);
  d = distance.calculate("algorifhm", "algorithm", 3);
  ASSERT_EQ(static_cast<double>(1.0), d);
  d = distance.calculate("algorithm", "algorihm", 3);
  ASSERT_EQ(static_cast<double>(1.0), d);
  d = distance.calculate("a1l2g3o4r5i6t7h8m", "algorithm", 10);
  ASSERT_EQ(static_cast<double>(8.0), d);

  // test on utf strings (with utf chars)
  wstring str1 = L"algorith";
  wstring str2 = L"algörithm";
  d = distance.calculate(str1, str2, 3);
  ASSERT_EQ(static_cast<double>(2), d);
  d = distance.calculate(str1, str2, 3);
  ASSERT_EQ(static_cast<double>(2), d);
  str1 = L"gluck";
  str2 = L"glück";
  d = distance.calculate(str1, str2, 3);
  ASSERT_EQ(static_cast<double>(1), d);

  // test extension distance
  ExtensionEditDistance extensionDistance;
  d = extensionDistance.calculate("alg", "algorithm", 3);
  ASSERT_EQ(0, d);
  d = extensionDistance.calculate("also", "algorithm", 3);
  ASSERT_EQ(1, d);
  d = extensionDistance.calculate("lgo", "algorithm", 3);
  ASSERT_EQ(1, d);
  d = extensionDistance.calculate("algorithm", "algorithm", 3);
  ASSERT_EQ(0, d);
  d = extensionDistance.calculate(L"glüc", L"glück", 3);
  ASSERT_EQ(0, d);
  d = extensionDistance.calculate(L"algörit", L"algörithm", 3);
  ASSERT_EQ(0, d);
}

// test the FastSS data structure for word similarity search
// test finding similar words with indexing a small dictionary
TEST(FuzzySearchTest, FastSS_words)
{
  FastSS<string> fastSS(2, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  vocabulary.push_back("algorithm");  // 0
  vocabulary.push_back("agorithm");   // 1
  vocabulary.push_back("xlgoorithm");  // 2
  vocabulary.push_back("algorithmiccomm");
  vocabulary.push_back("algomhtir");
  vocabulary.push_back("someword");
  vocabulary.push_back("algorihm");
  std::sort(vocabulary.begin(), vocabulary.end());
  fastSS.setFixedThreshold(2);
  fastSS.buildIndex(vocabulary, false);
  fastSS.findClosestWords("algorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(4u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(1, dist[1]);
  ASSERT_EQ(1, dist[2]);
  ASSERT_EQ(2, dist[3]);
  fastSS.findClosestWords("alorith", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(2u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  fastSS.findClosestWords("unknown", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(0u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  fastSS.saveDataStructureToFile("ds2389480239xcnbvx");
}

// test the FastSS data structure for word similarity search in UTF-8
// test finding similar words with indexing a small dictionary
TEST(FuzzySearchTest, FastSS_words_utf8)
{
  string locale = "en_US.utf8";
  if (setlocale(LC_ALL, locale.c_str()) == NULL)
    locale = "ERROR setting " + locale;
  FastSS<wstring> fastSS(2, 0);
  vector<wstring> vocabulary;
  vector<string> vocabulary1;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  // add words
  vocabulary1.push_back("algorithm");  // 0
  vocabulary1.push_back("agorithm");   // 1
  vocabulary1.push_back("xlgoorithm");  // 2
  vocabulary1.push_back("algorithmiccomm");
  vocabulary1.push_back("algomhtir");
  vocabulary1.push_back("someword");
  vocabulary1.push_back("algorihm");
  // convert to utf-16
  wstring tmpwstr;
  int conv_errors = 0;
  for (unsigned int i = 0; i < vocabulary1.size(); i++)
  {
    if (!FuzzySearch::string2wstring(vocabulary1[i], &tmpwstr))
      conv_errors++;
    vocabulary.push_back(tmpwstr);
  }
  std::sort(vocabulary.begin(), vocabulary.end());
  fastSS.setFixedThreshold(2);
  fastSS.buildIndex(vocabulary, false);
  fastSS.findClosestWords(L"algorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(4u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(1, dist[1]);
  ASSERT_EQ(1, dist[2]);
  ASSERT_EQ(2, dist[3]);
  fastSS.findClosestWords(L"alorith", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(2u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  fastSS.findClosestWords(L"unknown", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(0u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
}

// test the FastSS data structure for word similarity search
// test finding similar words when loading from file
TEST(FuzzySearchTest, FastSS_words_from_file)
{
  FastSS<string> fastSS(2, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  fastSS.loadDataStructureFromFile("ds2389480239xcnbvx", &vocabulary);
  ASSERT_EQ(7u, vocabulary.size());
  fastSS.findClosestWords("algorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(4u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(1, dist[1]);
  ASSERT_EQ(1, dist[2]);
  ASSERT_EQ(2, dist[3]);
  remove("ds2389480239xcnbvx");
}

bool orderIncr(const string& s1, const string& s2)
{
  return s1 > s2;
}

// test the FastSS data structure for prefix similarity search
// test finding similar words with indexing a small dictionary
TEST(FuzzySearchTest, FastSS_prefix1)
{
  FastSS<string> fastSS(3, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  vocabulary.push_back("algorithm");  // 0
  vocabulary.push_back("algorith");  // 0
  vocabulary.push_back("algorit");  // 0
  vocabulary.push_back("agorithm");   // del
  vocabulary.push_back("axlgorithm");   // ins
  vocabulary.push_back("axgorithm");   // subs
  vocabulary.push_back("xxalgorithm");   // 2
  vocabulary.push_back("allllgorithm");
  std::sort(vocabulary.begin(), vocabulary.end(), orderIncr);
  fastSS.setFixedThreshold(2);
  fastSS.buildIndex(vocabulary, false);
  fastSS.findClosestWords("algo", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(similarWordIds.size(), dist.size());
  ASSERT_EQ(7u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(!isInLexicon);
  ASSERT_EQ(0, dist[1]);
  ASSERT_EQ(0, dist[2]);
  ASSERT_EQ(1, dist[3]);
  ASSERT_EQ(1, dist[4]);
  ASSERT_EQ(1, dist[5]);
  fastSS.findClosestWords("xxalgorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(3u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(2, dist[1]);
  ASSERT_EQ(2, dist[2]);
  // ASSERT_EQ(3, dist[3]);
  fastSS.findClosestWords("alunknown", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(0u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  fastSS.saveDataStructureToFile("ds2389480239xcnbvx");
}

// test the FastSS data structure for prefix similarity search
// test finding similar words with indexing a small dictionary
TEST(FuzzySearchTest, FastSS_prefix2)
{
  FastSS<string> fastSS(3, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  vocabulary.push_back("baeza");
  vocabulary.push_back("beza");
  vocabulary.push_back("xeza");
  vocabulary.push_back("bxza");
  vocabulary.push_back("bexa");
  vocabulary.push_back("bezx");
  vocabulary.push_back("xbeza");
  vocabulary.push_back("bexza");
  vocabulary.push_back("bezax");
  vocabulary.push_back("testest123");
  std::sort(vocabulary.begin(), vocabulary.end(), orderIncr);
  fastSS.setFixedThreshold(1);
  fastSS.buildIndex(vocabulary, false);
  fastSS.findClosestWords("beza", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(similarWordIds.size(), dist.size());
  ASSERT_EQ(9u, similarWordIds.size());
  ASSERT_TRUE(isInLexicon);
}

// test the FastSS data structure for prefix similarity search
// test finding similar words when loading from file
TEST(FuzzySearchTest, FastSS_prefix_from_file)
{
  FastSS<string> fastSS(2, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  fastSS.loadDataStructureFromFile("ds2389480239xcnbvx", &vocabulary);
  ASSERT_EQ(8u, vocabulary.size());
  fastSS.findClosestWords("algo", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(similarWordIds.size(), dist.size());
  ASSERT_EQ(6u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(!isInLexicon);
  ASSERT_EQ(0, dist[1]);
  ASSERT_EQ(0, dist[2]);
  ASSERT_EQ(1, dist[3]);
  ASSERT_EQ(1, dist[4]);
  ASSERT_EQ(1, dist[5]);
  fastSS.findClosestWords("xxalgorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(4u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(2, dist[1]);
  ASSERT_EQ(2, dist[2]);
  ASSERT_EQ(3, dist[3]);
  fastSS.findClosestWords("alunknown", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(0u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  remove("ds2389480239xcnbvx");
}

// test the PermutedLexicon data structure for similarity search
// test finding similar words with indexing a small dictionary
TEST(FuzzySearchTest, PermutedLexicon_1)
{
  PermutedLexicon<string> pl(1, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  vocabulary.push_back("algorithm");  // 0
  vocabulary.push_back("agorithm");   // 1
  vocabulary.push_back("xlgoorithm");  // 2
  vocabulary.push_back("algorithmiccomm");
  vocabulary.push_back("algomhtir");
  vocabulary.push_back("someword");
  std::sort(vocabulary.begin(), vocabulary.end());
  pl.buildIndex(vocabulary, false);
  pl.findClosestWords("algorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(3u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(1, dist[1]);
  ASSERT_EQ(2, dist[2]);
  pl.findClosestWords("unknown", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(0u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  pl.saveDataStructureToFile("ds2389480239xcnbvx");
}

// test the PermutedLexicon data structure for similarity search
// test finding similar words when loading from file
TEST(FuzzySearchTest, PermutedLexicon_2)
{
  PermutedLexicon<string> pl(1, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  pl.loadDataStructureFromFile("ds2389480239xcnbvx", &vocabulary);
  ASSERT_EQ(6u, vocabulary.size());
  pl.findClosestWords("algorithm", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(3u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(1, dist[1]);
  ASSERT_EQ(2, dist[2]);
  ASSERT_TRUE(isInLexicon);
  remove("ds2389480239xcnbvx");
}

// test the PermutedLexicon data structure for similarity search
// test finding similar *long* words with indexing a small dictionary
TEST(FuzzySearchTest, PermutedLexicon_long_words)
{
  if (MAX_WORD_LEN < 42)
  {
    cout << "Warning: MAX_WORD_LEN should be set to at least 42 for this test!"
         << endl;
    return;
  }
  PermutedLexicon<string> pl(1, 0);
  pl.setFixedThreshold(3);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  // fastSS.buildIndex(vocabulary, false);  // empty vocabulary
  vocabulary.push_back("this should be avery long word");  // 0
  vocabulary.push_back("this shuld be a very long word");   // 1
  vocabulary.push_back("this should be a vey long word");  // 2
  vocabulary.push_back("this shoXuld be a very long word");
  vocabulary.push_back("this should be a very lozg word");
  vocabulary.push_back("ths should be a very log word");
  vocabulary.push_back("this should be some other veryy long wod");
  vocabulary.push_back("this should be x some other very long word");
  std::sort(vocabulary.begin(), vocabulary.end());
  pl.buildIndex(vocabulary, false);
  pl.findClosestWords("this should be a very long word", vocabulary, vocabulary,
      &isInLexicon, &similarWordIds, &dist);
  ASSERT_EQ(6u, similarWordIds.size());
  pl.findClosestWords("this should be some other very long word", vocabulary,
      vocabulary, &isInLexicon, &similarWordIds, &dist);
  ASSERT_EQ(2u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
}

// test the word clustering
TEST(FuzzySearchTest, WordClustering)
{
  WordClusteringBuilder<string> clusterBuilder;
  vector<string> vocabulary;
  vector<string> clusterCenters;
  vector<int> frequencyCentroids;
  vector<int> unclusteredWordIds;
  vector<int> frequencies;
  vector<vector<int> > clusters;
  vocabulary.push_back("alorith");
  vocabulary.push_back("algorth");
  vocabulary.push_back("algorihm");
  vocabulary.push_back("graphh");
  vocabulary.push_back("grph");
  vocabulary.push_back("algounkno");
  vocabulary.push_back("graphunko");
  vocabulary.push_back("varint");
  vocabulary.push_back("algorithm");
  vocabulary.push_back("algorithm1");
  vocabulary.push_back("graph");
  vocabulary.push_back("variant");
  vocabulary.push_back("variant1");
  vocabulary.push_back("variant2");
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(1);
  frequencies.push_back(100);
  frequencies.push_back(100);
  frequencies.push_back(100);
  frequencies.push_back(100);
  frequencies.push_back(100);
  frequencies.push_back(100);
  ASSERT_EQ(vocabulary.size(), frequencies.size());
  clusterBuilder.pickClusterCenters(vocabulary, frequencies, 100,
      &clusterCenters, &frequencyCentroids);
  ASSERT_EQ(6u, clusterCenters.size());

  // cluster frequent words
  vector<string> clusterCentersFW;
  vector<int> unclusteredWordIdsFW;
  clusterBuilder.buildWordClusteringFrequentWords(
                                    vocabulary,
                                    frequencies,
                                    2,
                                    100,
                                    &clusterCentersFW,
                                    &clusters,
                                    &unclusteredWordIdsFW);
  // clusters are ordered starting from the last frequent word
  ASSERT_EQ(4u, clusters.size());
  ASSERT_EQ("variant2", vocabulary[clusters[0][0]]);
  ASSERT_EQ(3u, clusters[0].size());  // of variant2
  ASSERT_EQ("variant1", vocabulary[clusters[1][0]]);
  ASSERT_EQ(3u, clusters[1].size());  // of variant1
  ASSERT_EQ("algorithm1", vocabulary[clusters[2][0]]);
  ASSERT_EQ(2u, clusters[2].size());  // of algorithm1
  ASSERT_EQ("algorithm", vocabulary[clusters[3][0]]);
  ASSERT_EQ(2u, clusters[3].size());  // of algorithm

  // cluster infrequent words
  FastSS<string> fastss(2, 1);
  fastss.setFixedThreshold(2);
  fastss.buildIndex(vocabulary, false);
  clusters.clear();
  clusterBuilder.buildWordClustering(vocabulary, clusterCenters,
      frequencies, fastss, 10, false,
      &clusters, &unclusteredWordIds);
  ASSERT_EQ(2u, clusters.size());
  ASSERT_EQ(3u, clusters[0].size());  // of algorithm
  ASSERT_EQ(2u, clusters[1].size());  // of graph
}

// test the prefix clustering
TEST(FuzzySearchTest, PrefixClustering)
{
  WordClusteringBuilder<string> clusterBuilder;
  vector<string> vocabulary;
  vector<vector<string> > clusterCentroids;
  vector<int> unclusteredWordIds;
  vector<int> frequencies;
  vector<vector<int> > clusters;
  vocabulary.push_back("algorithm");
  vocabulary.push_back("algorithmic");  // same
  vocabulary.push_back("algorihm");  // same
  vocabulary.push_back("algorihm1");  // same
  vocabulary.push_back("alxorithm");  // subs
  vocabulary.push_back("alorithm");  // del
  vocabulary.push_back("alxgorithm");  // ins (-)
  vocabulary.push_back("alxgoritm");  // ins (-)
  vocabulary.push_back("variant");
  vocabulary.push_back("ariant");
  vocabulary.push_back("variand");
  for (size_t i = 0; i < vocabulary.size(); i++)
    frequencies.push_back(1);
  clusterBuilder.sortParallel(&vocabulary, &frequencies, 1, true);
  int prefixLength = 4;
  clusterBuilder.pickClusterCentersComplStar(vocabulary, frequencies,
      1, prefixLength, 1, true, &clusterCentroids,
      NULL, NULL);
  ASSERT_EQ(6u, clusterCentroids.size());
  // cluster on *-blocks of length prefixLength
  clusterBuilder.buildCompletionClustering(vocabulary,
      clusterCentroids, frequencies, true, prefixLength,
      1, 1, 1, &clusters,
      NULL, NULL);
  ASSERT_EQ(4u, clusters.size());
  ASSERT_EQ("algorithmic", vocabulary[clusters[0][0]]);
  ASSERT_EQ("algorithm", vocabulary[clusters[0][1]]);
  ASSERT_EQ("algorihm1", vocabulary[clusters[0][2]]);
  ASSERT_EQ("algorihm", vocabulary[clusters[0][3]]);
  ASSERT_EQ("alorithm", vocabulary[clusters[1][0]]);
  ASSERT_EQ("alxorithm", vocabulary[clusters[1][1]]);
  ASSERT_EQ("alxgoritm", vocabulary[clusters[2][0]]);
  ASSERT_EQ("alxgorithm", vocabulary[clusters[2][1]]);
  ASSERT_EQ("ariant", vocabulary[clusters[3][0]]);
  ASSERT_EQ("variant", vocabulary[clusters[3][1]]);
  ASSERT_EQ("variand", vocabulary[clusters[3][2]]);
}

// test the FastSS data structure for word similarity search
// test finding similar words with indexing a small dictionary
TEST(FuzzySearchTest, FastSS_words_1)
{
  FastSS<string> fastSS(2, 0);
  vector<string> vocabulary;
  vector<int> similarWordIds;
  vector<double> dist;
  bool isInLexicon;
  vocabulary.push_back("complexity");  // 0
  vocabulary.push_back("cmplexity");   // 1
  vocabulary.push_back("omplexity");   // 1
  vocabulary.push_back("coomplexity");   // 1
  vocabulary.push_back("comple1xity");   // 1
  vocabulary.push_back("compl-xity");   // 1
  vocabulary.push_back("coplexxity");  // 2
  vocabulary.push_back("cozmplezxiy");  // 3
  vocabulary.push_back("complexityzzz");
  vocabulary.push_back("someword");
  vocabulary.push_back("zzzcomplexity");
  std::sort(vocabulary.begin(), vocabulary.end());
  fastSS.setFixedThreshold(2);
  fastSS.buildIndex(vocabulary, false);
  fastSS.findClosestWords("complexity", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(7u, similarWordIds.size());
  ASSERT_EQ(0, dist[0]);
  ASSERT_TRUE(isInLexicon);
  ASSERT_EQ(1, dist[1]);
  ASSERT_EQ(1, dist[2]);
  ASSERT_EQ(1, dist[3]);
  ASSERT_EQ(1, dist[4]);
  ASSERT_EQ(1, dist[5]);
  ASSERT_EQ(2, dist[6]);
  fastSS.findClosestWords("cmplxiy", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(1u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
  fastSS.findClosestWords("unknown", vocabulary, vocabulary, &isInLexicon,
      &similarWordIds, &dist);
  ASSERT_EQ(0u, similarWordIds.size());
  ASSERT_FALSE(isInLexicon);
}

int main(int argc, char** argv)
{
  cout << "----------------" << endl;
  cout << "Google Unit Test" << endl;
  cout << "----------------" << endl;
  cout << "Testing FUZZYSEARCH" << endl << endl;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
