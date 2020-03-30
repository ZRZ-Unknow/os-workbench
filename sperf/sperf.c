#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>

#define NUM 1024

typedef struct system_call{
  char name[64];
  double time;
}system_call;
double total_time=0;
int syscall_num=0;
system_call sys_call[NUM];
char div_0[10]="\0\0\0\0\0\0\0\0\0\0";

void debugprint(){
  for(int i=0;i<syscall_num;i++){
    printf("%s:%f\n",sys_call[i].name,sys_call[i].time);
  }
}

void insert(char *name,double time){
  total_time+=time;
  int pos=-1;
  for(int i=0;i<syscall_num;i++){
    if(strcmp(name,sys_call[i].name)==0)
      pos=i;
  }
  if(pos==-1){   //一个新的系统调用
    sys_call[syscall_num].time=time;
    strcpy(sys_call[syscall_num].name,name);
    syscall_num++;
  }
  else{
    sys_call[pos].time+=time;
  }
}

int time_cmp(const void *syscall_1,const void *syscall_2){
  system_call *sys1=(system_call*)syscall_1;
  system_call *sys2=(system_call*)syscall_2;
  return sys1->time <= sys2->time;
}

void sort(){
  qsort(sys_call,syscall_num,sizeof(system_call),time_cmp);
}

void display(){
  system("clear");
  for(int i=0;i<syscall_num;i++){
    if(i==0) printf("%20s:\033[1;31m(%9.6lf%%)\033[0m\n",sys_call[i].name,100*sys_call[i].time/total_time);
    else if(sys_call[i].time/total_time>0.01) printf("%20s:\033[1;32m(%9.6lf%%)\033[0m\n",sys_call[i].name,100*sys_call[i].time/total_time);
    else printf("%20s:\033[1;30m(%9.6lf%%)\033[0m\n",sys_call[i].name,100*sys_call[i].time/total_time);
  }
  for(int i=0;i<8;i++) printf("%s",div_0);
  fflush(stdout);
}
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
    char *pattern="<([0-9]*\\.[0-9]*)>";
    regex_t reg;
    regmatch_t pmatch;
    int ret=regcomp(&reg,pattern,REG_EXTENDED);
    while(fgets(buf,1024,fp)!=NULL){
      //printf("%s\n",buf);
      ret=regexec(&reg,buf,1,&pmatch,0);
      if(!ret){
        char time_buf[64],name_buf[64];
        double t;
        strncpy(&time_buf[0],buf+pmatch.rm_so+1,pmatch.rm_eo-pmatch.rm_so-2);
        sscanf(buf,"%[A-z0-9_]",name_buf);
        sscanf(time_buf,"%lf",&t);
        //printf("%s:%f\n",name_buf,t);
        insert(name_buf,t);
      }
      else{
        printf("no match:%s",buf);
      }
    }
    regfree(&reg);
    sort();
    display();
    //debugprint();
  }
  return 0;
}
