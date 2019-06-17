template<class T> bool fct(T a, T b) { return true; }
//template<class T> class Test { public: bool fct(T a, T b) { return true; } };
class Test { public: bool fct(int a, int b) { return true; } };

int main(char argc, char** argv)
{
  Test test;
  //bool(*fctPtr)(int,int)=fct<int>;
  //bool(*fctPtr)(int,int)=Test<int>::fct;
  bool(*fctPtr)(int,int)=test.fct;
  fct(1,2); //call directly
  fctPtr(1,2);  //call through pointer
}
