// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Marjan Celikik <celikik>, Hannah Bast <bast>

// Compute clusters and build fuzzy-search index for fast similarity search.

#include <getopt.h>
#include <string.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_set>

#include "../fuzzysearch/FuzzySearchAlgorithm.h"
#include "../fuzzysearch/WordClusteringBuilder.h"
#include "../fuzzysearch/StringDistances.h"
#include "../fuzzysearch/Utils.h"
#include "../fuzzysearch/FastSS.h"
#include "../fuzzysearch/PermutedLexicon.h"

using std::cout;
using std::vector;
using std::string;
using std::sort;
using std::endl;
using std::flush;
using std::wstring;
using std::cerr;

using FuzzySearch::WordClusteringBuilder;
using FuzzySearch::FuzzySearchAlgorithm;
using FuzzySearch::ENCODINGS;
using FuzzySearch::PlainEditDistance;
using FuzzySearch::Mod;
using FuzzySearch::FastSS;
using FuzzySearch::PermutedLexicon;
using FuzzySearch::StringHash;

void printUsage()
{
  cout
  << "Usage: buildFuzzySearchClusters [options] <db>" << endl
  << endl
  << "Options:" << endl
  << endl
  << " -d, --dictionary           the name of the dictionary of correct "
      "words" << endl
  << " -l, --locale               specifies the used locale (en_US.iso88591 "
      "is  the default)" << endl
  << " -t, --threshold            threshold for the similarity searching "
      "(if applicable)" << endl
  << " -r  --rare_words           if specified, the rare words will be "
      "clustered, too" << endl
  << " -m, --mode                 0 = mode1 (default), 1 = mode2 (only frequent"
      " words are cluster centers)" << endl
  << " -n, --normalize            use normalization over the vocabulary" << endl
  << " -c, --completion-matching  generate fuzzy completion matching permuted"
      " lexicon" << endl
  << " -a, --fuzzy-algorithm      type of fuzzy search algorithm (currently "
      "0 = permuted lexicon-based, 1 = fastss-based)" << endl
  << " -s, --distance             distance type (currently 0 = normalized edit"
      " distance, 1 = 2/5 edit distance)" << endl
  << " -k, --clustering-only      do not build and save a fuzzy search index "
      "for the clustered words" << endl
  << " -j, --no-clustering        do not build and save fuzzy clusters" << endl
  << " -i, --max-infrequent       max. number of clusters an infrequent word"
      " can be assigned to" << endl
  << " -o, --max-frequent         max. number of clusters a frequent word can"
      " be assigned to (proportional to the index space overhead!)" << endl
  << " -q, --min-frequency        minimum frequency for a frequent word" << endl
  << " -T, --build-trivial        build trivial clustering where each word is"
  << "a singleton cluster" << endl

  << endl;
}

double threshold = 0.28;
bool permutedOnly = false;
bool clusterRareWords = false;
bool useNormalization = false;
bool completionMatching = false;
bool noindex = false;
bool buildClustering = true;
bool buildTrivialClustering = false;
int encoding;
int fuzzySearchAlgorithm = 0;
int distanceMode = 0;
int minimumFrequency = 500;
int maxClustersPerFrequentWord = 1;
int maxClustersPerInfrequentWord = 10;

static enum {mode1 = 0, mode2 = 1, nofreq = 2} mode;
const char algDesc[][17] = {"PermutedLexicon", "FastSS"};
const char distanceModeDesc[][25] = {"normalized e. distance\0",
    "2/5 e. distance\0"};

template <class ST> void startClustering(
    string& vocabularyFileName, string& dataStructureFileName,
    string& clusterIdsFileName, string& dbName,
    string& dictionaryFileName, string& locale);

int main(int argc, char** argv)
{
  string vocabularyFileName;
  string dataStructureFileName;
  string clusterIdsFileName;
  string dbName;
  string dictionaryFileName;
  string locale;

  mode = mode1;
  int modeInt = 0;
  dbName = "db";
  dictionaryFileName = "english+german.utf8.dict";
  locale = "en_US.iso88591";

  while (true)
  {
    static struct option long_options[] =
    {
      {"dictionary"          , 1, NULL, 'd'},
      {"locale"              , 1, NULL, 'l'},
      {"threshold"           , 1, NULL, 't'},
      {"mode"                , 1, NULL, 'm'},
      {"frequency"           , 0, NULL, 'f'},
      {"rare_words"          , 0, NULL, 'r'},
      {"normalize"           , 0, NULL, 'n'},
      {"completion-matching" , 0, NULL, 'c'},
      {"fuzzy-algorithm"     , 0, NULL, 'a'},
      {"distance"            , 0, NULL, 's'},
      {"clustering-only"     , 0, NULL, 'k'},
      {"no-clustering"       , 0, NULL, 'j'},
      {"max-frequent"        , 0, NULL, 'o'},
      {"max-infrequent"      , 0, NULL, 'i'},
      {"min-frequency"       , 0, NULL, 'q'},
      {"build-trivial"       , 0, NULL, 'T'},
      {NULL                  , 0, NULL,  0 }
    };

    int c = getopt_long(argc, argv, "d:l:t:rna:s:km:cji:o:q:Tf",
        long_options, NULL);
    if (c == -1) break;
    switch (c)
    {
      case 'd': dictionaryFileName = optarg; break;
      case 'l': locale = optarg; break;
      case 't': threshold = atof(optarg); break;
      case 'p': permutedOnly = true; break;
      case 'r': clusterRareWords = true; break;
      case 'm': modeInt = atoi(optarg); break;
      case 'n': useNormalization = true; break;
      case 'c': completionMatching = true; break;
      case 'a': fuzzySearchAlgorithm = atoi(optarg); break;
      case 's': distanceMode = atoi(optarg); break;
      case 'k': noindex = true; break;
      case 'j': buildClustering = false; break;
      case 'o': maxClustersPerFrequentWord = atoi(optarg); break;
      case 'i': maxClustersPerInfrequentWord = atoi(optarg); break;
      case 'q': minimumFrequency = atoi(optarg); break;
      case 'T': buildTrivialClustering = true; break;
      case 'f': break;  // for backwards compatibility
      default : printUsage();
                exit(1);
                break;
    }
  }

  if (modeInt >= 1)
    mode = mode2;
  else
    mode = mode1;
  if (optind >= argc)
  {
    printUsage();
    exit(1);
  }
  dbName = argv[optind++];

  vocabularyFileName = dbName + ".vocabulary";
  dataStructureFileName = dbName + ".fuzzysearch-datastructure";
  clusterIdsFileName = dbName + ".fuzzysearch-clusters";

  encoding = WordClusteringBuilder<string>::ISO_8859_1;
  if (locale.find("utf8") != string::npos)
    encoding = WordClusteringBuilder<string>::UTF_8;

  if (encoding == WordClusteringBuilder<string>::ISO_8859_1)
  {
    startClustering<string>(vocabularyFileName,
                            dataStructureFileName,
                            clusterIdsFileName,
                            dbName,
                            dictionaryFileName,
                            locale);
  }
  else
  {
    startClustering<wstring>(vocabularyFileName,
                             dataStructureFileName,
                             clusterIdsFileName,
                             dbName,
                             dictionaryFileName,
                             locale);
  }
}

// Creates a FuzzySearchAlgorithm object according to the selected distance
// function and algorithm
template <class T>
void createNewFuzzySearchAlgorithmObject(FuzzySearchAlgorithm<T>* fsAlgorithm)
{
  if (fuzzySearchAlgorithm == 0)
  {
    if (completionMatching)
    {
      if (distanceMode == 0)
        fsAlgorithm = new PermutedLexicon<T>(3, threshold);
      else
        fsAlgorithm = new PermutedLexicon<T>(2, 0);
    }
    else
    {
      if (distanceMode == 0)
        fsAlgorithm = new PermutedLexicon<T>(0, threshold);
      else
        fsAlgorithm = new PermutedLexicon<T>(1, 0);
    }
  }
  else
  {
    if (completionMatching)
    {
      if (distanceMode == 0)
      {
        cerr << "Normalized edit distance is currently not supported by the "
            "FastSS-based algorithm!" << endl;
        exit(1);
      }
      else
        fsAlgorithm = new FastSS<T>(3, 0);
    }
    else
    {
      if (distanceMode == 0)
      {
        cerr << "Normalized edit distance is currently not supported by the "
            "FastSS-based algorithm!" << endl;
        exit(1);
      }
      else
      {
        fsAlgorithm = new FastSS<T>(2, 0);
      }
    }
  }
}

// used to sort in alphabetically increasing order
template <class T>
bool isLarger(const T& x, const T& y)
{
  return x > y;
}

// Does the actual clustering and fuzzy search data structure
// as well as saving those into files
template <class T>
void startClustering(
    string& vocabularyFileName, string& dataStructureFileName,
    string& clusterIdsFileName, string& dbName,
    string& dictionaryFileName, string& locale)
{
  WordClusteringBuilder<T> clusterBuilder;

  clusterBuilder.setEncoding(encoding);
  if (setlocale(LC_ALL, locale.c_str()) == NULL)
    locale = "ERROR setting " + locale;
  clusterBuilder.useNormalization(useNormalization);

  if (fuzzySearchAlgorithm < 0 || fuzzySearchAlgorithm > 1)
    fuzzySearchAlgorithm = 1;
  if (distanceMode != 0)
    distanceMode = 1;

  FuzzySearchAlgorithm<T>* fsAlgorithm = NULL;
  if (fuzzySearchAlgorithm == 0)
  {
    if (completionMatching)
    {
      if (distanceMode == 0)
        fsAlgorithm = new PermutedLexicon<T>(3, threshold);
      else
        fsAlgorithm = new PermutedLexicon<T>(2, 0);
    }
    else
    {
      if (distanceMode == 0)
        fsAlgorithm = new PermutedLexicon<T>(0, threshold);
      else
        fsAlgorithm = new PermutedLexicon<T>(1, 0);
    }
  }
  else
  {
    if (completionMatching)
    {
      if (distanceMode == 0)
      {
        cerr << "Normalized edit distance is currently not supported by the "
            "FastSS-based algorithm!" << endl;
        exit(1);
      }
      else
        fsAlgorithm = new FastSS<T>(3, 0);
    }
    else
    {
      if (distanceMode == 0)
      {
        cerr << "Normalized edit distance is currently not supported by the "
            "FastSS-based algorithm!" << endl;
        exit(1);
      }
      else
      {
        fsAlgorithm = new FastSS<T>(2, 0);
      }
    }
  }
  assert(fsAlgorithm != NULL);

  cout << endl;
  cout << "Templated version of FuzzySearch v. 2" << endl << endl;
  cout << "Locale                   : " << locale << endl;
  cout << "Using word normalization : ";
  useNormalization ? cout << "yes" : cout << "no";
  cout << endl;
  cout << "Fuzzy-search algorithm   : " << algDesc[fuzzySearchAlgorithm]
       << endl;
  cout << "Distance mode            : "
       << distanceModeDesc[distanceMode] << endl;
  cout << "Threshold                : ";
  if (distanceMode == 1)
    cout << "1 (for query length <= 5), 2 otherwise" << endl;
  else
    cout << threshold << endl;
  if (buildClustering)
  {
    cout << "Clustering mode          : " << mode << endl;
    cout << "Mode                     : ";
    completionMatching ? cout << "fuzzy completion-matching" << endl :
        cout << "fuzzy word-matching" << endl;
    cout << "Cluster rare words       : ";
    clusterRareWords ? cout << "yes" << endl : cout << "no" << endl;
    cout << "Minimum freq.            : " << minimumFrequency << endl;
    cout << "Clusters per freq. word  : "
        << maxClustersPerFrequentWord << endl;
    cout << "Clusters per infr. word  : "
        << maxClustersPerInfrequentWord << endl;
  }
  else
  {
    cout << "No clustering will be built." << endl;
  }
  cout << endl;

  vector<T> vocabulary;
  vector<T> originalVocabulary;
  vector<T> clusterCentroids;
  vector<T> clusterCenters1;
  vector<T> unclusteredWords;
  vector<int> unclusteredWordIds;
  vector<vector<int> > clusters;
  vector<vector<int> > clusters1;
  vector<vector<int> > clustersRareWords;
  vector<int> correctWordIds;
  vector<int> frequencies;
  vector<int> restFrequencies;
  vector<T> clusterCentroidsAll;
  int prefixLength;
  int clusterIdFWBeg = -1;
  int clusterIdFWEnd = -1;
  Timer timer;

  timer.start();
  vocabularyFileName += "+frequencies";
  clusterBuilder.readVocabularyFromDisk(vocabularyFileName, true, INT_MAX,
      &vocabulary, &frequencies);
  if (useNormalization)
  {
    originalVocabulary = vocabulary;
    WordClusteringBuilder<T>::normalizeVocabulary(originalVocabulary,
        &vocabulary);
  }
  // **************** trivial clustering (singleton clusters) ***************
  if (buildTrivialClustering && buildClustering)
  {
    cout << "Building trivial clustering ... " << endl << flush;
    vector<int> frequencyCentroids;
    clusterBuilder.pickClusterCenters(vocabulary, frequencies,
        minimumFrequency, &clusterCentroids, &frequencyCentroids);
    string centroidFileName = dbName + ".fuzzysearch-centroid-frequencies";
    WordClusteringBuilder<T>::writeVocabularyToFile(clusterCentroids,
        frequencyCentroids, centroidFileName);
    clusterCentroidsAll = clusterCentroids;
    // singleton clusters of frequent words
    clusterIdFWBeg = clusters.size();
    for (size_t i = 0; i < vocabulary.size(); i++)
      if (clusterBuilder.isClusterCentroid(i))
      {
        vector<int> tempCls;
        tempCls.push_back(i);
        clusters.push_back(tempCls);
      }
    clusterIdFWEnd = clusters.size() - 1;
    // singleton clusters of infrequent words
    for (size_t i = 0; i < vocabulary.size(); i++)
      if (!clusterBuilder.isClusterCentroid(i))
      {
        vector<int> tempCls;
        tempCls.push_back(i);
        clusters.push_back(tempCls);
      }
  }
  else
  if (completionMatching && buildClustering)
  {
    // ******************* fuzzy completion clustering **********************
    if (mode == mode1)
    {
      // 1. cluster the frequent prefixes using *-blocks

      vector<T> vocabulary1;
      vector<T> clusterCentroids1;
      vector<int> frequencies1;
      vector<vector<T> > clusterCentroids2;
      clusterBuilder.sortParallel(&vocabulary, &frequencies, 1, true);
      clusterIdFWBeg = clusters.size();
      prefixLength = 4;
      clusterBuilder.pickClusterCentersComplStar(vocabulary, frequencies,
          minimumFrequency, prefixLength, 1, true, &clusterCentroids2,
          &vocabulary1, &frequencies1);
      clusterCentroidsAll.clear();
      for (size_t j = 0; j < vocabulary.size(); j++)
      {
        if (clusterBuilder.isClusterCentroid(j))
          clusterCentroidsAll.push_back(vocabulary[j]);
      }
      // cluster on *-blocks of length prefixLength
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids2, frequencies, true, prefixLength,
          maxClustersPerFrequentWord, minimumFrequency, 1, &clusters,
          &unclusteredWordIds, &restFrequencies);

      // cluster on prefixes of length prefixLength
      clusterBuilder.chopWords(clusterCentroidsAll, prefixLength, 0,
          &clusterCentroids1);
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids1, frequencies, true, prefixLength,
          1, minimumFrequency, 0, &clusters,
          &unclusteredWordIds, &restFrequencies);

      // cluster on *-blocks of length prefixLength + 1
      // comment: uncomment the following lines in case they're commented!
      // (used only for hyb construction experiments)
      prefixLength++;
      clusterBuilder.pickClusterCentersComplStar(vocabulary1, frequencies1,
          1, prefixLength, 1, false, &clusterCentroids2);
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids2, frequencies, true, prefixLength,
          maxClustersPerFrequentWord, minimumFrequency, 1, &clusters,
          &unclusteredWordIds, &restFrequencies);

      // 2. Cluster the rare prefixes

      clusterIdFWEnd = clusters.size() - 1;

      prefixLength = 4;
      for (size_t j = 0; j < vocabulary.size(); j++)
      {
        if (!clusterBuilder.isClusterCentroid(j))
          vocabulary1.push_back(vocabulary[j]);
      }
      vector<T> temp;
      clusterBuilder.chopWords(vocabulary1, prefixLength, 0, &temp);
      for (size_t j = 0; j < temp.size(); j++)
        clusterCentroids1.push_back(temp[j]);
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids1, frequencies, false, prefixLength,
          maxClustersPerInfrequentWord, minimumFrequency, 1, &clusters,
          &unclusteredWordIds, &restFrequencies);
      vocabulary1.clear();
      clusterBuilder.buildCompletionClustering(vocabulary,
          temp, frequencies, false, prefixLength, 1, minimumFrequency,
          0, &clusters, &unclusteredWordIds, &restFrequencies);

      // old method using frequent prefixes as cluster centroids
      // (it does not work very well)
      /*
      prefixLength = 4;
      vector<T> vocabulary1;
      vector<T> clusterCentroids1;
      vector<int> frequencies1;
      vector<vector<T> > clusterCentroids2;
      clusterBuilder.pickClusterCentersCompl(vocabulary, frequencies,
         minimumFrequency, prefixLength, true, &clusterCentroids1);
      clusterCentroids2.push_back(clusterCentroids1);
      clusterCentroidsAll.clear();
      for (size_t j = 0; j < vocabulary.size(); j++)
      {
        if (clusterBuilder.isClusterCentroid(j))
          clusterCentroidsAll.push_back(vocabulary[j]);
      }
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids2, frequencies, true, prefixLength, 2,
          minimumFrequency, false, 1, &clusters, &unclusteredWordIds,
           &restFrequencies);

      clusterIdFWEnd = clusters.size() - 1;

      prefixLength = 4;
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids2, frequencies, false, prefixLength, 10,
          minimumFrequency, false, 1, &clusters, &unclusteredWordIds,
          &restFrequencies);
      */
    }
    else
    {
      // 1. cluster the frequent words only on short prefixes
      vector<T> clusterCentroids1;
      clusterIdFWBeg = clusters.size();
      prefixLength = 4;
      clusterBuilder.sortParallel(&vocabulary, &frequencies, 1, true);

      // cluster on *-blocks prefixes
      clusterBuilder.pickClusterCentersCompl(vocabulary, frequencies,
           minimumFrequency, prefixLength, true, &clusterCentroids1);
      clusterBuilder.chopWords(clusterCentroids1, prefixLength, 1,
          &clusterCentroids);
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids, frequencies, true, prefixLength,
          maxClustersPerFrequentWord, INT_MAX, 1, &clusters,
          &unclusteredWordIds, &restFrequencies);

      // cluster on ordinary prefixes
      clusterBuilder.chopWords(clusterCentroids1, prefixLength, 0,
          &clusterCentroids);
      clusterBuilder.buildCompletionClustering(vocabulary,
          clusterCentroids, frequencies, true, prefixLength, 1,
          INT_MAX, 0, &clusters, &unclusteredWordIds, &restFrequencies);

      clusterIdFWEnd = clusters.size() - 1;

      // 2. cluster the infrequent words
      prefixLength = MAX_WORD_LEN;
      clusterBuilder.pickClusterCentersCompl(vocabulary, frequencies,
           minimumFrequency, prefixLength, false, &clusterCentroids);
      clusterCentroidsAll.clear();
      for (size_t j = 0; j < vocabulary.size(); j++)
      {
        if (clusterBuilder.isClusterCentroid(j))
          clusterCentroidsAll.push_back(vocabulary[j]);
      }
      fsAlgorithm->buildIndex(vocabulary, false);
      clusterBuilder.buildCompletionClustering(vocabulary, clusterCentroids,
          frequencies, *fsAlgorithm, prefixLength,
          maxClustersPerInfrequentWord, false, 1, 0, &clusters,
          &unclusteredWordIds, &restFrequencies);
    }
  }
  else
  if (!completionMatching && buildClustering)
  {
    // ********************** fuzzy word clustering *************************
    vector<int> frequencyCentroids;
    clusterBuilder.pickClusterCenters(vocabulary, frequencies,
        minimumFrequency, &clusterCentroids, &frequencyCentroids);
    string centroidFileName = dbName + ".fuzzysearch-centroid-frequencies";
    WordClusteringBuilder<T>::writeVocabularyToFile(clusterCentroids,
        frequencyCentroids, centroidFileName);
    clusterCentroidsAll = clusterCentroids;
    fsAlgorithm->buildIndex(vocabulary, false);
    clusterBuilder.buildWordClustering(vocabulary, clusterCentroids,
        frequencies, *fsAlgorithm, maxClustersPerInfrequentWord, false,
        &clusters, &unclusteredWordIds);
    vector<T> clusterCentersFW;
    vector<int> unclusteredWordIdsFW;
    clusterIdFWBeg = clusters.size();
    clusterBuilder.buildWordClusteringFrequentWords(
                                      vocabulary,
                                      frequencies,
                                      maxClustersPerFrequentWord,
                                      minimumFrequency,
                                      &clusterCentersFW,
                                      &clusters,
                                      &unclusteredWordIdsFW);
    clusterIdFWEnd = clusters.size() - 1;
  }

  if (clusterRareWords && unclusteredWordIds.size() > 0)
  {
    /*
    cout << "Step 2: Clustering the remaining "
         << unclusteredWordIds.size() << " infrequent words (non-overlapping"
         << " clustering) ..." << endl;
    cout << endl;
    */
    frequencies.clear();
    clusterCentroids.clear();
    unclusteredWords.resize(unclusteredWordIds.size());
    for (unsigned int i = 0; i < unclusteredWordIds.size(); i++)
      unclusteredWords[i] = vocabulary[unclusteredWordIds[i]];
    if (!completionMatching)
    {
      /*
      clusterBuilder.pickClusterCenters(unclusteredWords, *fsAlgorithm, 1.0,
          10, &clusterCenters, NULL);
      fsAlgorithm->buildIndex(clusterCenters, false);
      clusterBuilder.buildWordClusteringOld(unclusteredWords, clusterCenters,
          frequencies, *fsAlgorithm, 1, true, &clustersRareWords, NULL);
      for (size_t i = 0; i < clusterCenters.size(); i++)
        clusterCentroidsAll.push_back(clusterCenters[i]);
      */
    }
    else
    {
      /*
      WordClusteringBuilder<T>::writeVocabularyToFile(unclusteredWords,
           "unclustered");
      vector<T> clusterCentroidsRare;
      clusterBuilder.sortParallel(&unclusteredWords, &restFrequencies, 2);
      sort(unclusteredWords.begin(), unclusteredWords.end());
      clusterBuilder.processUnclusteredCompl(unclusteredWords, *fsAlgorithm,
          5, &clusterCentroidsRare, &clusters);
      */
    }
  }


  // write cluster id range for frequent words (if used)
  if (clusterIdFWBeg != -1 && clusterIdFWEnd != -1)
  {
    std::fstream f((dbName+".fuzzysearch-fwrange").c_str(), std::ios::out);
    f << clusterIdFWBeg << endl;
    f << clusterIdFWEnd << endl;
    f.close();
  }

  // write the clusters to a file
  if (buildClustering)
    cout << "Writing clusters ... " << flush;
  vector<T> clusteredWords;
  vector<int> clusterIds;
  const vector<T>* voca1;
  const vector<T>* voca2;
  vector<T> unclusteredOriginalWords(unclusteredWordIds.size());
  if (useNormalization)
  {
    voca1 = &originalVocabulary;
    for (unsigned int i = 0; i < unclusteredWordIds.size(); i++)
      unclusteredOriginalWords[i] = originalVocabulary[unclusteredWordIds[i]];  // NOLINT
    voca2 = &unclusteredOriginalWords;
  }
  else
  {
    voca1 = &vocabulary;
    voca2 = &unclusteredWords;
  }
  for (size_t i = 0; i < clusters.size(); i++)
    for (size_t j = 0; j < clusters[i].size(); j++)
    {
      clusteredWords.push_back((*voca1)[clusters[i][j]]);
      clusterIds.push_back(i);
    }

  // if (clusterRareWords && unclusteredWordIds.size() > 0)
  //  for (size_t i = 0; i < clustersRareWords.size(); i++)
  //    for (size_t j = 0; j < clustersRareWords[i].size(); j++)
  //    {
  //      clusteredWords.push_back((*voca2)[clustersRareWords[i][j]]);
  //      clusterIds.push_back(i + clusters.size());
  //    }

  // autocompletion fuzzy search requires sorting the vocabulary in reverse
  // order (a fuzzy matching filter in FastSS.cpp requires this)
  bool sortReverse = completionMatching;
  if (buildClustering)
  {
    WordClusteringBuilder<T>::sortParallel(&clusteredWords, &clusterIds,
        1, sortReverse);
    clusterBuilder.writeClustersToFile(clusteredWords, clusterIds,
        clusterIdsFileName);
      cout << "done." << flush << endl;
  }
  else
  {
    clusteredWords = vocabulary;
    FILE* f = fopen(clusterIdsFileName.c_str(), "w");
    fclose(f);
  }

  // write the fuzzy search index
  delete fsAlgorithm;  // delete the old data structure first to save memory
  if (!noindex)
  {
    if (fuzzySearchAlgorithm == 0)  // create a new object
    {
      if (completionMatching)
      {
        if (distanceMode == 0)
          fsAlgorithm = new PermutedLexicon<T>(3, threshold);
        else
          fsAlgorithm = new PermutedLexicon<T>(2, 0);
      }
      else
      {
        if (distanceMode == 0)
          fsAlgorithm = new PermutedLexicon<T>(0, threshold);
        else
          fsAlgorithm = new PermutedLexicon<T>(1, 0);
      }
    }
    else
    {
      if (completionMatching)
      {
        if (distanceMode == 0)
        {
          cerr << "Normalized edit distance is not supported by the "
              "FastSS-based algorithm!" << endl;
          exit(1);
        }
        else
          fsAlgorithm = new FastSS<T>(3, 0);
      }
      else
      {
        if (distanceMode == 0)
        {
          cerr << "Normalized edit distance is not supported by the "
              "FastSS-based algorithm!" << endl;
          exit(1);
        }
        else
        {
          fsAlgorithm = new FastSS<T>(2, 0);
        }
      }
    }
    cout << endl;
    cout << "Generating fuzzy-search data structure for the clustering."
         << endl << endl;
    std::unordered_set<T, StringHash<T> > hashSet;
    vector<T> uniqueClusteredWords;
    for (size_t i = 0; i < clusteredWords.size(); i++)
      hashSet.insert(clusteredWords[i]);
    for (size_t i = 0; i < clusterCentroidsAll.size(); i++)
      hashSet.insert(clusterCentroidsAll[i]);
    // insert unclustered words in the data structure
    if (clusterRareWords)
      for (size_t i = 0; i < voca2->size(); i++)
        hashSet.insert((*voca2)[i]);
    vector<T> tmp;
    clusteredWords = tmp;
    for (auto it = hashSet.begin(); it != hashSet.end(); ++it)
      uniqueClusteredWords.push_back(*it);
    std::cout << uniqueClusteredWords.size() << " unique words." << std::endl;
    if (!sortReverse)
      sort(uniqueClusteredWords.begin(), uniqueClusteredWords.end());
    else
      sort(uniqueClusteredWords.begin(), uniqueClusteredWords.end(),
          isLarger<T>);
    FILE* f = fopen((dbName+".fuzzysearch-clustercentroids").c_str(), "w");
    for (size_t i = 0; i < clusterCentroidsAll.size(); i++)
    {
      int low  = 0;
      int high = uniqueClusteredWords.size() - 1;
      int mid  = 0;
      while (low < high)
      {
        mid = (low + high) / 2;
        if (clusterCentroidsAll[i] < uniqueClusteredWords[mid])
          low = mid + 1;
        else
          high = mid;
      }
      mid = low;
      if (uniqueClusteredWords[mid] == clusterCentroidsAll[i])
        fprintf(f, "%d\n", mid);
    }
    fclose(f);
    if (useNormalization)
      WordClusteringBuilder<T>::normalizeVocabulary(uniqueClusteredWords,
          &uniqueClusteredWords);
    fsAlgorithm->buildIndex(uniqueClusteredWords, false);
    fsAlgorithm->saveDataStructureToFile(dataStructureFileName);
  }
  timer.stop();
  cout << endl;
  cout << "Total FuzzySearch build time: " << timer.secs() << " secs."<< endl;
  cout << endl;
}

