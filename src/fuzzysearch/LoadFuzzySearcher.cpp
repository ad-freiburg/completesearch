// Copyright 2009, University of Freiburg, Chair of Algorithms and Data
// Structures.
// Authors: Marjan Celikik <celikik>

#include <vector>
#include <string>
#include "../fuzzysearch/FuzzySearcher.h"
#include "../fuzzysearch/Timer.h"
#include "../fuzzysearch/Utils.h"

using std::cout;
using std::wcout;
using std::cin;
using std::string;
using std::wstring;
using std::endl;
using std::vector;
using FuzzySearch::WordClusteringBuilder;
using FuzzySearch::FuzzySearcherBase;
using FuzzySearch::FuzzySearcherUtf8;
using FuzzySearch::FuzzySearcherIso88591;
using FuzzySearch::wstring2string;

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    cout << "Usage: loadFuzzySearcher <db> <encoding> "
         << "<0/1 for using normalization>" << endl << endl;
    return 1;
  }
  string dbname = argv[1];
  string encodingStr = argv[2];
  int normalization = atoi(argv[3]);
  int encoding = WordClusteringBuilder<string>::ISO_8859_1;
  if (encodingStr.find("utf8") != string::npos)
    encoding = WordClusteringBuilder<string>::UTF_8;

  cout << "Encoding from command line: "
      << FuzzySearch::ENCODINGS[encoding] << endl;

  setlocale(LC_CTYPE, encodingStr.c_str());
  setlocale(LC_COLLATE, encodingStr.c_str());
  setlocale(LC_ALL, encodingStr.c_str());
  // the first two are not needed?
  FuzzySearcherBase* fuzzySearcher;
  if (encoding == WordClusteringBuilder<string>::UTF_8)
    fuzzySearcher = new FuzzySearcherUtf8();
  else
    fuzzySearcher = new FuzzySearcherIso88591();
  fuzzySearcher->useNormalization(normalization);
  fuzzySearcher->init(dbname);

  vector<int> similarWords;
  vector<double> distances;
  bool qiil;

  string str;
  Timer timer;
  while (true)
  {
    cout << endl;
    cout << "query word: ";
    getline(cin, str);
    cout << endl;
    timer.start();
    fuzzySearcher->findClosestWords(str, false, &qiil,
        &similarWords, &distances);
    timer.stop();
    cout << endl;
    for (unsigned int i = 0; i < similarWords.size(); i++)
    {
      if (encoding == WordClusteringBuilder<string>::UTF_8)
      {
        string str;
        wstring2string((static_cast<FuzzySearcherUtf8*>(fuzzySearcher))
                    ->clusterCenters[similarWords[i]], &str);
        cout << str << " - " << distances[i] << endl;
      }
      else
        cout << (static_cast<FuzzySearcherIso88591*>(fuzzySearcher))
        ->clusterCenters[similarWords[i]] << " - " << distances[i] << endl;
    }
    cout << endl;
    cout << "[ " << timer.usecs() << " usec ] Number of similar words: "
         << similarWords.size() << endl;
    cout << endl;
  }
  return 0;
}
