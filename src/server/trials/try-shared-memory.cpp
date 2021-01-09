#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include "Timer.h"
#include <sstream>

/* check out http://users.actcom.co.il/~choo/lupg/tutorials/multi-process/multi-process.html#shmem */

using namespace std;

extern void parent();
extern void child();
extern void lock_file(int fid, int lock_type);
FILE* log_file;
int log_fid;
enum LockType { _LOCK = F_WRLCK, _UNLOCK = F_UNLCK };
ostream& operator<<(ostream& os, LockType& lock_type) { lock_file(log_fid, lock_type); return os; }
LockType LOCK = _LOCK; 
LockType UNLOCK = _UNLOCK;

int S;
int n;
int shm_key_start;
int* shm_key;
int* shm_id;
char** shm_addr;
pid_t pid;
Timer timer;


int main(char argc, char** argv)
{
  S = 1024*1024;
  n = argc > 1 ? atoi(argv[1]) : 1024; /* SHMMAX on mpiao95XX is 32768*1024 bytes */
  shm_key_start = argc > 2 ? atoi(argv[2]) : 1000;  
  shm_key = new int[n];
  shm_id = new int[n];
  shm_addr = new char*[n];
  /* redirect all output */
  cout.setf(ios::fixed);
  cout.precision(3);
  log_file = fopen("log", "w");
  if (log_file == NULL) { perror("fopen log file"); exit(1); }
  log_fid = fileno(log_file);
  dup2(log_fid, STDOUT_FILENO);
  dup2(log_fid, STDERR_FILENO);
  fclose(log_file);
  /* memset of ordinary memory (for comparison) */
  char** bytes = new char*[n];
  timer.start();
  for (int i = 0; i < n; ++i) bytes[i] = new char[S];
  timer.stop();
  cout << "MAIN: new for " << n << " segments of " << S/(1024*1024) 
       << " MB each took " << timer.usecs()/1000.0 << " milliseconds" << endl;
  timer.start();
  for (int i = 0; i < n; ++i) memset((void*)(bytes[i]),0,S);
  timer.stop();
  cout << "MAIN: memset to all 0 took " << timer.usecs()/1000.0 << " milliseconds" << endl;
  int sum = 0;
  timer.start();
  for (int i = 0; i < n; ++i) for (int j = 0; j < S; ++j) sum += *((char*)(bytes[i])+j);
  timer.stop();
  cout << LOCK << "MAIN: sum took " << timer.usecs()/1000.0 << " milliseconds; average value is " << sum/(1.0*n*S) << endl << UNLOCK;
  /* allocate n shared memory segments */
  timer.start();
  for (int i = 0; i < n; ++i) 
  {
    shm_id[i] = shmget(shm_key_start + i, S, IPC_CREAT /* | IPC_EXCL */ | 0600);
    if (shm_id[i] == -1) { perror("shmget"); exit(1); }
  }
  timer.stop();
  cout << "MAIN: shmget for " << n << " segments of " << S/(1024*1024) 
       << " MB each took " << timer.usecs()/1000.0 << " milliseconds" << endl;
  /* fork */
  pid = fork();
  if (pid == -1) { perror("fork"); exit(1); }
  /* add to memory page table of this process */
  for (int i = 0; i < n; ++i) 
  {
    shm_addr[i] = (char*)(shmat(shm_id[i], NULL, 0));
    if (shm_addr[i] == 0) { perror("shmat"); exit(1); }
  }
  /* now different code for parent and child */
  pid > 0 ? parent() : child();
}

void parent()
{
  /* set memory segment to all 1 */
  timer.start();
  for (int i = 0; i < n; ++i) memset((void*)(shm_addr[i]),1,S);
  timer.stop();
  cout << LOCK << "PARENT: memset to all 1 took " << timer.usecs()/1000.0 << " milliseconds" << endl << UNLOCK;
  /* compute sum */
  int sum = 0;
  timer.start();
  for (int i = 0; i < n; ++i) for (int j = 0; j < S; ++j) sum += *((char*)(shm_addr[i])+j);
  timer.stop();
  cout << LOCK << "PARENT: sum took " << timer.usecs()/1000.0 << " milliseconds; average value is " << sum/(1.0*n*S) << endl << UNLOCK;
  /* destroy the shared memory segment. */
  wait();
  struct shmid_ds shm_desc;
  for (int i = 0; i < n; ++i)
  {
    int ret = shmctl(shm_id[i], IPC_RMID, &shm_desc);
    if (ret == -1)  perror("PARENT: shmctl");
  }
}

void child()
{
  //exit(0);
  /* set memory segment to all 2 */
  timer.start();
  for (int i = 0; i < n; ++i) memset((void*)(shm_addr[i]),1,S);
  timer.stop();
  cout << LOCK << "CHILD: memset to all 2 took " << timer.usecs()/1000.0 << " milliseconds" << endl << UNLOCK;
  /* compute sum */
  int sum = 0;
  timer.start();
  for (int i = 0; i < n; ++i) for (int j = 0; j < S; ++j) sum += *((char*)(shm_addr[i])+j);
  timer.stop();
  cout << LOCK << "CHILD: sum took " << timer.usecs()/1000.0 << " milliseconds; average value is " << sum/(1.0*n*S) << endl << UNLOCK;
}

void lock_file(int fid, int lock_type)
{
  struct flock fl;
  fl.l_type   = lock_type;  /* F_RDLCK, F_WRLCK, F_UNLCK    */
  fl.l_whence = SEEK_SET; /* SEEK_SET, SEEK_CUR, SEEK_END */
  fl.l_start  = 0;        /* Offset from l_whence         */
  fl.l_len    = 0;        /* length, 0 = to EOF           */
  fl.l_pid    = getpid(); /* our PID                      */
  fcntl(fid, lock_type == F_UNLCK ? F_SETLK : F_SETLKW, &fl);  /* lock (wait if necessary) or unlock */
}


// LOGGING WITH LOCKS (didn't work)
class Log
{ 
  public: 
  FILE* file;
  Log(char* file_name) { file = fopen(file_name, "w"); if (file == NULL) { perror("fopen"); exit(1); } }
  ~Log() { int ret = fclose(file); if (ret != 0) { perror("fclose"); exit(1); } }
};
template<class T>
Log& operator<<(Log& log, T& x)
{
  ostringstream os;
  os.setf(ios::fixed);
  os.precision(3);
  os << x;
  fwrite((void*)(os.str().c_str()), 1, os.str().length(), log.file);
  return log;
}
Log LOG("log");

