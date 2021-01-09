#include <iostream>

using namespace std;

void a()
{
  throw 5;
}

void b() //throw()
{
  a();
}

int main(char argc, char** argv)
{
  try
  {
    b();
  }
  /*
  catch (exception e)
  {
    cerr << "EXCEPTION: " << e.what() << endl;
  }
  catch (int i)
  {
    cerr << "EXCEPTION: " << i << endl;
  }
  */
  catch (...)
  {
    cerr << "Blödmann" << endl;
  }
  //throw "Heini";
}

//  unsigned int n = 1024*1024*1024;
//  while (true)
//  {
//    cout << "!" << flush;
//    try
//    {
//      char* p = new char[n];
//        //char* p = (char*)(malloc(n));
//      *p = 0;
//    }
//    catch (exception e)
//    {
//      cerr << "EXCEPTION: " << e.what() << endl;
//    }
//  }
