#include "../Globals.h"

void printUsage()
{
  cout << "Usage: try-rank-words <number n of items> <range m>" << endl
       << endl
       << "Generates a random sequence of n integers, each from a range of size m." << endl
       << "Then times two alternative algorithms for counting the number of" << endl 
       << "occurrences of each integer:" << endl
       << endl
       << "(1) copy the sequence, sort it, and then count" << endl
       << endl
       << "(2) scan through the sequence, maintaing counters in an array of size m" << endl
       << endl
       << "(3) scan through the sequence, maintaing counters in a hash map" << endl
       << endl
       << "Conclusions drawn from this experiment 14Apr08: (1) is best as long as n is" << endl
       << "significantly smaller than m, say n < m / 10. When n = m, (1) is significantly worse" << endl
       << "so there seems to be no way around implementing both variants and picking the" << endl
       << "appropriate one depending on the ratio of n / m. (3) is always worse" << endl
       << endl;
}

int main(int argc, char** argv)
{
  cout << endl
       << "\033[1mRanking words in CompleteSearch: three alternative\033[21m" << endl
       << endl;

  if (argc < 2) { printUsage(); exit(1); }

  unsigned int n = atoi(argv[1]);
  unsigned int m = atoi(argv[2]);
  unsigned int R = argc > 3 ? atoi(argv[3]) : 3;
  Timer timer;

  cout << "Creating sequence of " << commaStr(n) 
       << " random integers from range of size " << commaStr(m) 
       << endl << endl;
  timer.start();
  srand48(time(NULL));
  vector<unsigned int> items(n);
  for (unsigned int i = 0; i < items.size(); ++i) {
    items[i] = (unsigned int)(m * drand48());
    if (i < 10) cout << items[i] << ", " << flush;
  }
  timer.stop();
  cout << "..." << endl << endl;

  for (unsigned int r = 1; r <= R; ++r)
  {
    cout << "Copy, sort, count ... " << flush;
    timer.start();
    vector<unsigned int> sortedItems = items;
    sort(sortedItems.begin(), sortedItems.end());
    timer.stop();
    cout << "done in " << timer << endl;
  }
  cout << endl;


  for (unsigned int r = 1; r <= R; ++r)
  {
    cout << "Scan, counters in array ... " << flush;
    timer.start();
    vector<unsigned int> counts(m, 0);
    for (unsigned int i = 0; i < items.size(); ++i)
    {
      ++counts[items[i]];
    }
    timer.stop();
    cout << "done in " << timer << endl;
  }
  cout << endl;

  for (unsigned int r = 1; r <= R; ++r)
  {
    cout << "Baseline: just scan, without counting ... " << flush;
    timer.start();
    unsigned int count = 0;
    for (unsigned int i = 0; i < items.size(); ++i)
    {
      count += items[i];
    }
    if (count == 0) cout << "\x00";
    timer.stop();
    cout << "done in " << timer << endl;
  }
  cout << endl;

  for (unsigned int r = 1; r <= R; ++r)
  {
    cout << "Scan, counters in hash ... " << flush;
    timer.start();
    hash_map<unsigned int, unsigned int> counts;
    for (unsigned int i = 0; i < items.size(); ++i)
    {
      ++counts[items[i]];
    }
    timer.stop();
    cout << "done in " << timer << endl;
  }
  cout << endl;

}    



