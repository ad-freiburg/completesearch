#include <iostream>
#include <unistd.h>

using namespace std;

int main(char argc, char** argv)
{
  FILE* log_file = fopen("LOG","w");
  if (log_file == NULL) { perror("! main: fopen logfile"); exit(1); }
  dup2(STDOUT_FILENO, fileno(log_file));
  dup2(STDERR_FILENO, fileno(log_file));
  //dup2(fileno(log_file), STDOUT_FILENO);
  //dup2(fileno(log_file), STDERR_FILENO);
  fclose(log_file);
  cout << "A message to cout" << endl;
  cerr << "A message to cerr" << endl;
}

