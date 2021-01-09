#include <stdio.h>
#include <unistd.h>
#include <pthread.h> /* don't forget to link with -lpthread; see http://yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html */
#include <iomanip>
#include <iostream>
#include <vector>
#include "Timer.h"
#include "assert.h"
#define EMPH_ON  "\033[1m"
#define EMPH_OFF "\033[21m"

using namespace std;

void printUsage()
{
  cout << "Usage: test-thread-overhead <number of threads> -f <level of file operation>" << endl
       << endl
       << "Launches the given number of threads and computes the average time for creation + execution" << endl
       << endl
       << "The code executed depends on the argument to the -f option" << endl
       << endl
       << "-f 0 : nothing happens in the thread but the time measurement" << endl
       << "-f 2 : additionally, a file is opened and close (the same file for all threads)" << endl
       << "-f 3 : additionally, an integer is read from that file (different position for each thread)" << endl
       << "-f 1 : the integer is read with pread with a common file descriptor for all threads" << endl
       << endl;
}


void* thread_function(void* args);

unsigned int thread_count = 0;
unsigned long long int usecs_total = 0; 
char* filename = "xxxxx";
int mode = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
FILE* file_global = NULL;
int fd_global = 0;

int main(int argc, char** argv)
{
  cout << endl << EMPH_ON << "TEST & TIME THREAD OVERHEAD (" << VERSION << ")" << EMPH_OFF << endl << endl;
  if (argc <= 1) { printUsage(); exit(1); }
  srand48(9824932);

  while (true)
  {
    int c = getopt(argc, argv, "f:s:");
    if (c == -1) break;
    switch (c)
    {
      case 'f': mode = atoi(optarg); break;
      case 's': srand48(atoi(optarg)); break;
    }
  }
  unsigned int MAX_THREAD_COUNT = optind < argc ? atoi(argv[optind++]) : 512;

  vector<pthread_t> pthread_ids;
  if (mode >= 1)
  {
    file_global = fopen(filename, "w");
    for (unsigned int i = 0; i < MAX_THREAD_COUNT; i++)
      fwrite(&i, 1, sizeof(unsigned int), file_global);
    fclose(file_global);
    if (mode == 1)
    {
      file_global = fopen(filename, "r");
      fd_global = fileno(file_global);
    }
  }
  
  for (unsigned int i = 1; i <= MAX_THREAD_COUNT; i++)
  {
    pthread_t pthread_id;
    Timer* timer = new Timer();
    timer->start();
    void* args;
    (Timer*)(args) = timer;
    int ret = pthread_create(&pthread_id, NULL, thread_function, args);
    if (ret != 0) { fprintf(stderr, "ERROR calling pthread_create #%u: ", i); perror(""); }
    //printf("Thread #%02u : created\n", threadId);
    pthread_ids.push_back(pthread_id);
    //pthread_join(pthread_id, NULL);
  }

  sleep(1);
  //for (unsigned int i = 0; i < pthread_ids.size(); ++i) pthread_join(pthread_ids[i], NULL);
  cout << endl << endl;
  if (mode == 1) fclose(file_global);
  if (mode >= 1) unlink(filename);
}

void* thread_function(void* args)
{
  pthread_mutex_lock(&mutex);
  unsigned int count = ++thread_count;
  pthread_mutex_unlock(&mutex);
  if (mode == 1)
  {
    unsigned int value;
    pread(fd_global, &value, sizeof(unsigned int), (count-1)*sizeof(unsigned int));
    assert(value == count-1);
  }
  if (mode >= 2)
  {
    FILE* file = fopen(filename, "r");
    if (mode >= 3)
    {
      fseek(file, (count-1)*sizeof(unsigned int), SEEK_SET);
      unsigned int value;
      fread(&value, 1, sizeof(unsigned int), file);
      assert(value == count-1);
    }
    fclose(file);
  }
  Timer* timer = (Timer*)(args);
  timer->stop();
  unsigned int usecs = timer->usecs();
  delete timer;
  pthread_mutex_lock(&mutex);
  usecs_total += usecs;
  printf("Average time for creating and calling %3u threads: %llu microseconds\n", count, usecs_total / count);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

