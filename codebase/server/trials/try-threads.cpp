#include <iostream>
#include <pthread.h> /* don't forget to link with -lpthread !!! */

/* check out the very good tutorial at http://yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html ! */

using namespace std;

void* my_thread_function(void* args);

int main(char argc, char** argv)
{
  pthread_t thread_id;
  //pthread_attr_t thread_attr;
  //pthread_attr_init(&thread_attr);
  char* arg = "arg";
  pthread_create(&thread_id, NULL, my_thread_function, (void*)(arg));
  pthread_join(thread_id, NULL); /* wait for this thread to return */
}


void* my_thread_function(void* args)
{
  cout << "THREAD: Hi!" << endl;
  return NULL;
}
