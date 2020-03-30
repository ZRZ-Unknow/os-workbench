#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {


  char *exec_argv[argc+2]; //= { "strace", "-T"};
  for(int i=0;i<argc+2;i++){
    if(i==0) exec_argv[i]="strace";
    else if(i==1) exec_argv[i]="-T";
    else exec_argv[i+2]=argv[i];
  }
  for(int i=0;i<argc+2;i++)
    printf("%s",exec_argv[i]);
  char *exec_envp[] = { "PATH=/bin", NULL, };
  //execve("strace",          exec_argv, exec_envp);
  //execve("/bin/strace",     exec_argv, exec_envp);
  execve("/usr/bin/strace", exec_argv, exec_envp);

  perror(argv[0]);
  exit(EXIT_FAILURE);
}
