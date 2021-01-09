// Albert-Ludwigs-University Freiburg
// Chair of Algorithms and Data Structures
// Copyright Jens Hoffmann 2010

#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include "./Vector.h"
#include "./ZipfCompressionAlgorithm.h"

using std::cout;
using std::endl;
using std::vector;
using std::flush;

void printUsage(const char *prog)
{
  cout << "Usage: " << prog << " <Increment elements by> <runs>" << endl;
}

int nofDecimalDigits(int value)
{
  int result = 0;
  while (value /= 10)
  {
    ++result;
  }
  return result;
}

//! Return time difference in ms.
long mtimeFromTimeDifference(const struct timeval& start,
                             const struct timeval& end)
{
  long seconds = end.tv_sec - start.tv_sec;
  long useconds = end.tv_usec- start.tv_usec;
  return ((seconds) * 1000 + useconds/1000.0) + 0.5;
}

int main(int argc, char **argv)
{
  ZipfCompressionAlgorithm<int> zipf;
  Vector<int> toCompress;
  int *target = NULL;
  int *decompressTarget = NULL;
  int nofElements, incElementsBy, runs;
  int i, j;
  struct timeval timeStart, timeEnd;
  long mtime;

  // Two arguments expected.
  if (argc != 3)
  {
    printUsage(argv[0]);
    exit(1);
  }
  
  // Init.
  target = new int[1024*1024];
  incElementsBy = atoi(argv[1]);
  runs = atoi(argv[2]);
  nofElements = 0;
  srand(time(NULL));

  // Run ZipfCompressionAlgorithm multiple times.
  cout << "Elements\tCompress (ms)\tDecompress (ms)" << endl;
  for (j = 0; j < runs; ++j)
  {
    toCompress.clear();
    nofElements += incElementsBy;
    decompressTarget = new int[nofElements];
    // Fill vector with nofElements elements.
    for (i = 0; i < nofElements; ++i)
    {
      toCompress.push_back(1);
    }
    cout << setw(8) << nofElements << "\t" << flush;

    // Compressing.
    gettimeofday(&timeStart, NULL);
    zipf.compress(toCompress, reinterpret_cast<void*>(target));
    gettimeofday(&timeEnd, NULL);
    // Evaluation.
    mtime = mtimeFromTimeDifference(timeStart, timeEnd);
    cout << setw(13) << mtime << "\t";

    // Decompressing.
    gettimeofday(&timeStart, NULL);
    zipf.decompress(reinterpret_cast<void*>(target),
                    decompressTarget, nofElements);
    gettimeofday(&timeEnd, NULL);
    // Evaluation.
    mtime = mtimeFromTimeDifference(timeStart, timeEnd);
    cout << setw(15) << mtime << "\t";
    cout << endl;
    delete[] decompressTarget;
  }
}

