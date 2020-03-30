#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {


  char *exec_argv[argc+2];
  for(int i=0;i<argc+2;i++){
    if(i==0) exec_argv[i]="strace";
    else if(i==1) exec_argv[i]="-T";
    else if(i==argc+1) exec_argv[i]=NULL;
    else exec_argv[i]=argv[i-1];
  }
  char *exec_envp[] = { "PATH=/bin", NULL, };
  //execve("/usr/bin/strace", exec_argv, exec_envp);
  int pid=fork();
  if(pid==0){
    execve("/usr/bin/strace", exec_argv, exec_envp);
  }
  else{
    printf("ddd\n");
  }
  return 0;
}
