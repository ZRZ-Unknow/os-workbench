#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <string.h>

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
    int fd=open("dev/null",O_WRONLY);
    dup2(fd,STDOUT_FILENO);
    dup2(fildes[1],STDERR_FILENO);
    execve("/usr/bin/strace", exec_argv, exec_envp);
  }
  else{
    close(fildes[1]);
    FILE *fp=fdopen(fildes[0],"r");
    char buf[1024];
    char *pattern="<[0-9]*\\\.?[0-9]*>";//"^[1-9]/d*/./d*|0/./d*[1-9]/d*$";//"<[0-9]\.[0-9]>";
    regex_t reg;
    regmatch_t pmatch;
    int ret=regcomp(&reg,pattern,REG_EXTENDED);
    while(fgets(buf,1024,fp)!=NULL){
      //printf("%s\n",buf);
      ret=regexec(&reg,buf,1,&pmatch,0);
      if(ret==0){
        char buff[64];
        strncpy(&buff[0],buf+pmatch.rm_so,pmatch.rm_eo-pmatch.rm_so);
        printf("%s\n",buff);
      }
      else{
        printf("ddd\n");
      }
    }
    printf("%d,%d,%d\n",fildes[0],fildes[1],pid);
  }
  return 0;
}
