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
  int fildes[2];
  if(pipe(fildes)!=0) assert(0);
  int pid=fork();
  if(pid==0){
    //关闭读端
    close(fildes[0]);
    printf("%d,%d,%d\n",fildes[0],fildes[1],pid);
    execve("/usr/bin/strace", exec_argv, exec_envp);
  }
  else{
    printf("%d,%d,%d\n",fildes[0],fildes[1],pid);
  }
  return 0;
}
