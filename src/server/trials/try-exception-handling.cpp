#include <stdio.h>
#include <unistd.h>
#include <pthread.h> /* don't forget to link with -lpthread; see http://yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html */
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

void* thread_function(void* args);
void my_function_1(unsigned int x, float p, unsigned int& ret);
void my_function_2(unsigned int x, float p, unsigned int& ret);

class Exception
{
 private:
  int _code;
 public:
  Exception(int code) : _code(code) { }
  int getCode() {  return _code; }
};

int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "TEST EXCEPTION HANDLING (" << VERSION << ")" << EMPH_OFF << endl << endl;
  unsigned int nof_threads = argc > 1 ? atoi(argv[1]) : 10;
  srand48(argc > 2 ? atoi(argv[2]) : 238947234);
  vector<pthread_t> pthread_ids;
  
  for (unsigned int i = 0; i < nof_threads; ++i)
  {
    pthread_t pthread_id;
    unsigned int threadId = i + 1;
    void* args = malloc(sizeof(unsigned int));
    *(unsigned int*)(args) = threadId;
    pthread_create(&pthread_id, NULL, thread_function, args);
    printf("Thread #%02u : created\n", threadId);
    pthread_ids.push_back(pthread_id);
    //pthread_join(pthread_id, NULL);
  }

  for (unsigned int i = 0; i < pthread_ids.size(); ++i) pthread_join(pthread_ids[i], NULL);
  cout << endl;
}

void* thread_function(void* args)
{
  unsigned int threadId = *(unsigned int*)(args);
  //cout << setwidth(2) << "Bla" << endl;
  srand48(threadId*817242323 + 429784234);
  printf("Thread #%02d : calling sequence of dummy functions\n", threadId);
    //printf("Thread #%02d : sleeping for %3u milliseconds\n", threadId, msecs);
    //usleep(1000*msecs);
  unsigned int x = (unsigned int)(drand48()*100) + 100;
  unsigned int ret = x * (x + 1);
  try
  {
    //unsigned int msecs = (unsigned int)(drand48()*100) + 100;
    my_function_1(x, 0.5 / (x*x), ret);
    printf("Thread #%02d : SUCCESS, return value is %u\n", threadId, ret);
  }
  catch (Exception e)
  {
    printf("Thread #%02d : EXCEPTION %u caught, return value thus far is %u\n", threadId, e.getCode(), ret);
  }
  printf("Thread #%02d : finished\n", threadId);
  free(args);
  return NULL;
}

void my_function_1(unsigned int n, float p, unsigned int& ret)
{
  for (unsigned int i = 0; i < n; ++i)
  {
    if (drand48() < p) throw Exception(1);
    ret--;
    my_function_2(n, p, ret);
  }
}

void my_function_2(unsigned int n, float p, unsigned int& ret)
{
  if (drand48() < p) throw Exception(2);
  if (n == 0) return;
  ret--;
  my_function_2(n - 1, p, ret);
}
