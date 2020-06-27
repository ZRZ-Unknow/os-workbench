#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
//思路：先从1进程为根进行搜索建树，然后搜索/proc的其他文件夹，把不在树中的加入进去

#define pid_t int

bool P=false;
bool N=false;
bool V=false;

typedef struct childs{
  pid_t pid;
  struct childs* next;
}childs;

typedef struct proc{
  char* name;
  pid_t pid;
  pid_t ppid;
  struct proc *next;
  childs *child;
}proc;

proc root_proc={.pid=1,.ppid=0,.next=NULL};
proc *root=&root_proc;

bool is_num(char *str){
  for(int i=0;i<strlen(str);i++){
    if(isdigit(str[i])==0) return false;
  }
  return true;
}

proc *get_last_proc(){
  proc *pr=root;
  while(pr->next!=NULL) pr=pr->next;
  return pr;
}

void build_tree(){

}
char path[300];
void get_procs(){
  DIR *dir=opendir("/proc");
  struct dirent *dire;
  while((dire=readdir(dir))!=NULL){
    if(dire->d_type==4 && is_num(dire->d_name)){
      sprintf(path,"/proc/%s/stat",dire->d_name);
      FILE *fp=fopen(path,"r");
      if(strcmp(dire->d_name,"1")==0){

      }
      else{
        proc *last_proc=get_last_proc();
        char name[32];
        int pid;
        int ppid;
        char s;
        fscanf(fp,"%d (%s %c %d",&pid,&name[0],&s,&ppid);
        name[strlen(name)-1]='\0';
        printf("%s,%d,%d\n",name,pid,ppid);
      }
      
    }
  }
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i],"-p")==0 || strcmp(argv[i],"--show-pids")==0){
      P=true;
    }
    else if(strcmp(argv[i],"-n")==0 || strcmp(argv[i],"--numeric-sort")==0){
      N=true;
    }
    else if(strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0){
      V=true;
    }
  }
  if(V){
    fprintf(stderr,"pstree version 1.0 CopyRight (C) 2020 ZRZ\n");
  }
  else{
    build_tree();
    get_procs();
  }
  return 0;
}