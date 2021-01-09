#include "../Globals.h"

void printUsage()
{
  cout << "Usage: try-ranking-templated-or-not <size n of array> <range of size m> <method: 1, 2, or 3>" << endl
       << endl
	   << "Generate n random integers from a range of size m, sort them, and count" << endl
	   << "the number of occurrences of each integer. (This mimicks the doc and word" << endl
	   << "ranking in CompleteSearch). Do the ranking in two ways: (1) with an if in" << endl
	   << "the inner loop, deciding which method to pick; (2) with the method" << endl
	   << "fixed (mimicking templating)." << endl
	   << endl;
}

// Observations / Conclusion from this experiment, 14Apr08: 
//
// * The difference between the templated and the non-templated version is
//   largest, when the word range is small (and the innermost loop is executed a
//   significant number of times and dominates the surrounding code in the outer
//   loop, running-time wise_.
//
// * Even when the difference is largest, it is only about 20% - 25%.
//
// * The other code (sorting, if the lists are relatively small, maintaining
//   counters in an array otherwise) always takes at least as much (and often
//   more) time than the mere aggregation / counting. This reduces the absolute
//   improvement of using templating to half or less.
//
// * It is hence not worth a considerable effort to employ templating here,
//   especially not, if it makes the code more complicated and/or harder to
//   write and maintain.


int main(int argc, char** argv)
{
  cout << endl
       << "\033[1mRanking in CompleteSearch, templated vs. non-templated\033[21m" << endl
       << endl;

  if (argc < 4) { printUsage(); exit(1); }

  unsigned int n = atoi(argv[1]);
  unsigned int m = atoi(argv[2]);
  unsigned int method = atoi(argv[3]);
  unsigned int R = argc > 4 ? atoi(argv[4]) : 3;
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

  // First, copy and sort
  for (unsigned int r = 1; r <= R; ++r)
  {
    cout << "First, the copy and sort ... " << flush;
	timer.start();
    vector<unsigned int> sortedItems = items;
	vector<unsigned int> counts;
    sort(sortedItems.begin(), sortedItems.end());
	timer.stop();
	cout << "done in " << timer << endl;
  }
  cout << endl;

  // Or alternatively, maintain counter in array 
  for (unsigned int r = 1; r <= R; ++r)
  {
    cout << "Or alternatively, maintain counters in array ... " << flush;
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


  // Counting with if in inner loop (non-templated version)
  for (unsigned int r = 1; r <= R; ++r)
  {
    vector<unsigned int> sortedItems = items;
	vector<unsigned int> counts;
    sort(sortedItems.begin(), sortedItems.end());
    cout << "Counting, with if in inner loop ... " << flush;
    timer.start();
	unsigned int currentItem;
	unsigned int currentCount;
	unsigned int i = 0;
	while (i < sortedItems.size())
	{
	  currentItem = sortedItems[i];
	  currentCount = 1;
	  ++i;
	  while (i < sortedItems.size() && sortedItems[i] == currentItem)
	  {
		if      (method == 1) currentCount += 1;
		else if (method == 2) currentCount += 2;
		else if (method == 3) currentCount += 3;
		++i;
	  }
	  counts.push_back(currentCount);
	}
    timer.stop();
    cout << "done in " << timer << endl;
  }
  cout << endl;


  // Without if in inner loop (corresponds to templated version)
  for (unsigned int r = 1; r <= R; ++r)
  {
    vector<unsigned int> sortedItems = items;
	vector<unsigned int> counts;
    sort(sortedItems.begin(), sortedItems.end());
    cout << "Coutning, without if in inner loop ... " << flush;
    timer.start();
	unsigned int currentItem;
	unsigned int currentCount;
	unsigned int i = 0;
	while (i < sortedItems.size())
	{
	  currentItem = sortedItems[i];
	  currentCount = 1;
	  ++i;
	  while (i < sortedItems.size() && sortedItems[i] == currentItem)
	  {
		currentCount += method;
		++i;
	  }
	  counts.push_back(currentCount);
	}
    timer.stop();
    cout << "done in " << timer << endl;
  }
  cout << endl;
}    

