#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {


  char *exec_argv[argc+2] = { "strace", "-T"};
  for(int i=0;i<argc;i++) exec_argv[i+2]=argv[i];
  char *exec_envp[] = { "PATH=/bin", NULL, };
  //execve("strace",          exec_argv, exec_envp);
  //execve("/bin/strace",     exec_argv, exec_envp);
  execve("/usr/bin/strace", exec_argv, exec_envp);
  for(int i=0;i<argc+2;i++)
    printf("%s",exec_argv[i]);
  perror(argv[0]);
  exit(EXIT_FAILURE);
}
