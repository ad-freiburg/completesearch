// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#include <iostream>
#include <vector>
#include "../utils/Timer.h"

using std::vector;
using std::cout;
using std::endl;
using std::flush;

class MyComplexElement
{
  public:

    MyComplexElement()
    {
    }

    MyComplexElement(size_t a, size_t b, size_t c, size_t d) :
      _a(a), _b(b), _c(c), _d(d)
    {
    }

    size_t _a;
    size_t _b;
    size_t _c;
    size_t _d;
};

class MyPayload
{
  public:

    MyPayload()
    {
    }

    MyPayload(size_t b, size_t c, size_t d) :
      _b(b), _c(c), _d(d)
    {
    }

    size_t _b;
    size_t _c;
    size_t _d;
};

class MyElement
{
    MyElement()
    {
    }

    MyElement(size_t a, size_t b, size_t c, size_t d) :
      _a(a), _pay(b, c, d)
    {
    }
    size_t _a;
    MyPayload _pay;
};

int main(int argc, char **argv)
{
  // Timers
  semsearch::Timer t1;
  semsearch::Timer t2;
  semsearch::Timer t3;

  // The data structures
  vector<MyComplexElement> complex;
  vector<size_t> parA;
  vector<size_t> parB;
  vector<size_t> parC;
  vector<size_t> parD;

  vector<size_t> withPayA;
  vector<MyPayload> pay;

  size_t MAX = 100000000;
  // Insert without reserve
  cout << "Insert without reserve: " << endl;
  t1.start();
  for (size_t i = 0; i < MAX; ++i)
  {
    complex.push_back(MyComplexElement(i, MAX - i, 2 * i, i + 2));
  }
  t1.stop();
  complex.resize(0);

  cout << "Complex Array took: " << t1.usecs() / 1000 << "ms." << endl;

  t2.start();
  for (size_t i = 0; i < MAX; ++i)
  {
    parA.push_back(i);
    parB.push_back(MAX - i);
    parC.push_back(2 * i);
    parD.push_back(i + 2);
  }
  t2.stop();

  cout << "Parallel Arrays took: " << t2.usecs() / 1000 << "ms." << endl;

  parA.resize(0);
  parB.resize(0);
  parC.resize(0);
  parD.resize(0);

  t3.start();
  for (size_t i = 0; i < MAX; ++i)
  {
    withPayA.push_back(i);
    pay.push_back(MyPayload(MAX - i, 2 * i, i + 2));
  }
  t3.stop();

  cout << "With payloads took: " << t3.usecs() / 1000 << "ms." << endl << endl;

  withPayA.resize(0);
  pay.resize(0);

  // Insert with reserve
  cout << "Insert with reserve: " << endl;

  t1.start();
  complex.reserve(MAX);
  for (size_t i = 0; i < MAX; ++i)
  {
    complex.push_back(MyComplexElement(i, MAX - i, 2 * i, i + 2));
  }
  t1.stop();

  cout << "Complex Array took: " << t1.usecs() / 1000 << "ms." << endl;

  t2.start();
  parA.reserve(MAX);
  parB.reserve(MAX);
  parC.reserve(MAX);
  parD.reserve(MAX);

  for (size_t i = 0; i < MAX; ++i)
  {
    parA.push_back(i);
    parB.push_back(MAX - i);
    parC.push_back(2 * i);
    parD.push_back(i + 2);
  }
  t2.stop();

  cout << "Parallel Arrays took: " << t2.usecs() / 1000 << "ms." << endl;

  t3.start();
  withPayA.reserve(MAX);
  pay.reserve(MAX);
  for (size_t i = 0; i < MAX; ++i)
  {
    withPayA.push_back(i);
    pay.push_back(MyPayload(MAX - i, 2 * i, i + 2));
  }
  t3.stop();

  cout << "With payloads took: " << t3.usecs() / 1000 << "ms." << endl << endl;

  // Random Acces
  cout << "Random Access to all lists: " << endl;

  t1.start();
  size_t sum = 0;
  for (size_t i = 1; i < MAX; i += 50)
  {
    sum += (complex[i]._b + complex[i]._c - complex[i]._d - complex[i]._a);
  }
  t1.stop();

  cout << "Complex Array took: " << t1.usecs() / 1000 << "ms. result: " << sum
      << endl;

  t2.start();
  sum = 0;
  for (size_t i = 1; i < MAX; i += 50)
  {
    sum += (parB[i] + parC[i] - parD[i] - parA[i]);
  }
  t2.stop();

  cout << "Parallel Arrays took: " << t2.usecs() / 1000 << "ms.result: "
      << sum << endl;

  t3.start();
  sum = 0;
  for (size_t i = 1; i < MAX; i += 50)
  {
    sum += (pay[i]._b + pay[i]._c - pay[i]._d - withPayA[i]);
  }
  t3.stop();

  cout << "With Payload took: " << t3.usecs() / 1000 << "ms.result: " << sum
      << endl << endl;

  // Random Acces
  cout << "Random Access to one list: " << endl;

  t1.start();
  sum = 0;
  for (size_t i = 1; i < MAX; i += 50)
  {
    sum += complex[i]._a;
  }
  t1.stop();

  cout << "Complex Array took: " << t1.usecs() / 1000 << "ms. result: " << sum
      << endl;

  t2.start();
  sum = 0;
  for (size_t i = 1; i < MAX; i += 50)
  {
    sum += parA[i];
  }
  t2.stop();

  cout << "Parallel Arrays took: " << t2.usecs() / 1000 << "ms.result: "
      << sum << endl;

  t3.start();
  sum = 0;
  for (size_t i = 1; i < MAX; i += 50)
  {
    sum += withPayA[i];
  }
  t3.stop();

  cout << "With Payload took: " << t3.usecs() / 1000 << "ms.result: " << sum
      << endl << endl;

  // Intersect with a single list.
  for (int gap = 5; gap < MAX / 2; gap *= 10)
  {
    vector<size_t> intersSingle;
    vector<size_t> intersSingleB;
    for (size_t i = 0; i < MAX; i += gap)
    {
      intersSingle.push_back(i);
      intersSingleB.push_back(i);
    }
    size_t bSize = intersSingle.size();

    cout << "Intersecting with a list of single elements." << endl
        << "Only keeping elements from the original list without gaps."
        << endl << "Gap-size in the single-elements list: " << gap << endl;
    t1.start();
    size_t a = 0;
    size_t b = 0;

    vector<MyComplexElement> cres;
    while (a < MAX && b < bSize)
    {
      if (complex[a]._a == intersSingleB[b])
      {
        cres.push_back(complex[a]);
        ++a;
        ++b;
      }
      else if (complex[a]._a < intersSingleB[b])
      {
        ++a;
      }
      else if (complex[a]._a > intersSingleB[b])
      {
        ++b;
      }
    }
    t1.stop();

    cout << "Complex Array took: " << t1.usecs() / 1000 << "ms." << endl;

    t2.start();

    a = 0;
    b = 0;

    vector<size_t> resA;
    vector<size_t> resB;
    vector<size_t> resC;
    vector<size_t> resD;
    while (a < MAX && b < bSize)
    {
      if (parA[a] == intersSingle[b])
      {
        resA.push_back(parA[a]);
        resB.push_back(parB[a]);
        resC.push_back(parC[a]);
        resD.push_back(parD[a]);
        ++a;
        ++b;
      }
      else if (parA[a] < intersSingle[b])
      {
        ++a;
      }
      else if (parA[a] > intersSingle[b])
      {
        ++b;
      }
    }
    t2.stop();

    cout << "Parallel Arrays took: " << t2.usecs() / 1000 << "ms." << endl;

    t3.start();

    a = 0;
    b = 0;

    vector<size_t> resWithPay;
    vector<MyPayload> resPay;

    while (a < MAX && b < bSize)
    {
      if (withPayA[a] == intersSingle[b])
      {
        resWithPay.push_back(withPayA[a]);
        resPay.push_back(pay[a]);

        ++a;
        ++b;
      }
      else if (withPayA[a] < intersSingle[b])
      {
        ++a;
      }
      else if (withPayA[a] > intersSingle[b])
      {
        ++b;
      }
    }
    t3.stop();

    cout << "With Payload took: " << t3.usecs() / 1000 << "ms." << endl
        << endl;
  }
  cout << endl;

  // Intersect with similar data structures
  for (int gap = 5; gap < MAX / 2; gap *= 10)
  {
    vector<size_t> intersA;
    vector<size_t> intersB;
    vector<size_t> intersC;
    vector<size_t> intersD;
    vector<size_t> intersAWithP;
    vector<MyPayload> intersPay;
    vector<MyComplexElement> intersCom;
    for (size_t i = 0; i < MAX; i += gap)
    {
      intersA.push_back(i);
      intersB.push_back(i);
      intersC.push_back(i);
      intersD.push_back(i);
      intersAWithP.push_back(i);
      intersPay.push_back(MyPayload(i, i, i));
      intersCom.push_back(MyComplexElement(i, i, i, i));
    }
    size_t bSize = intersA.size();

    cout << "Intersecting with similar data structures" << endl
        << "Keeps elements from both lists when first element matches."
        << endl << "Gap-size: " << gap << endl;

    t1.start();
    size_t a = 0;
    size_t b = 0;

    vector<MyComplexElement> cres;
    while (a < MAX && b < bSize)
    {
      if (complex[a]._a == intersCom[b]._a)
      {
        cres.push_back(complex[a]);
        cres.push_back(intersCom[b]);
        ++a;
        ++b;
      }
      else if (complex[a]._a < intersCom[b]._a)
      {
        ++a;
      }
      else if (complex[a]._a > intersCom[b]._a)
      {
        ++b;
      }
    }
    t1.stop();

    cout << "Complex Array took: " << t1.usecs() / 1000 << "ms." << endl;

    t2.start();

    a = 0;
    b = 0;

    vector<size_t> resA;
    vector<size_t> resB;
    vector<size_t> resC;
    vector<size_t> resD;
    while (a < MAX && b < bSize)
    {
      if (parA[a] == intersA[b])
      {
        resA.push_back(parA[a]);
        resB.push_back(parB[a]);
        resC.push_back(parC[a]);
        resD.push_back(parD[a]);
        resA.push_back(intersA[b]);
        resB.push_back(intersB[b]);
        resC.push_back(intersC[b]);
        resD.push_back(intersD[b]);
        ++a;
        ++b;
      }
      else if (parA[a] < intersA[b])
      {
        ++a;
      }
      else if (parA[a] > intersA[b])
      {
        ++b;
      }
    }
    t2.stop();

    cout << "Parallel Arrays took: " << t2.usecs() / 1000 << "ms." << endl;

    t3.start();

    a = 0;
    b = 0;

    vector<size_t> resWithPay;
    vector<MyPayload> resPay;

    while (a < MAX && b < bSize)
    {
      if (withPayA[a] == intersAWithP[b])
      {
        resWithPay.push_back(withPayA[a]);
        resPay.push_back(pay[a]);
        resWithPay.push_back(intersAWithP[b]);
        resPay.push_back(intersPay[b]);
        ++a;
        ++b;
      }
      else if (withPayA[a] < intersAWithP[b])
      {
        ++a;
      }
      else if (withPayA[a] > intersAWithP[b])
      {
        ++b;
      }
    }
    t3.stop();

    cout << "With Payload took: " << t3.usecs() / 1000 << "ms." << endl
        << endl;
  }
  cout << endl;
  // Sort
}
