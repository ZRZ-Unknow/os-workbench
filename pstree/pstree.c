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
  char name[128];
  pid_t pid;
  pid_t ppid;
  int child_num;
  pid_t child_pid[128];
  struct proc *next;
}proc;

proc root_proc={.pid=1,.ppid=0,.child_num=0,.next=NULL};
proc *root=&root_proc;
char path1[600];
char path2[600];
char tmp;


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

void debugprint(){
  for(proc* p=root;p!=NULL;p=p->next){
    printf("%s,%d,ppid:%d,childs:",p->name,p->pid,p->ppid);  
    for(int i=0;i<p->child_num;i++){
      printf("%d,",p->child_pid[i]);
    }
    printf("\n");
  }
}

proc *find_proc(int pid){
  for(proc *p=root;p!=NULL;p=p->next){
    if(p->pid==pid){
      return p;
    }
  }
  return NULL;
}

void print_tree(proc *p){
  if(P){
    printf("%s(%d)-",p->name,p->pid);
  } 
  else{
    printf("%s-",p->name);
  }
  for(int i=0;i<p->child_num;i++){
    proc *child=find_proc(p->child_pid[i]);
    if(child){
      print_tree(child);
    }
  } 
  print("\n");
}


void get_procs(){
  DIR *dir=opendir("/proc");
  struct dirent *dire;
  while((dire=readdir(dir))!=NULL){
    if(dire->d_type==4 && is_num(dire->d_name)){
      sprintf(path1,"/proc/%s/stat",dire->d_name);
      sprintf(path2,"/proc/%s/task/%s/children",dire->d_name,dire->d_name);
      FILE *fp=fopen(path1,"r");
      if(strcmp(dire->d_name,"1")==0){
        fscanf(fp,"%d (%s %c %d",&root->pid,root->name,&tmp,&root->ppid);
        root->name[strlen(root->name)-1]='\0';
        fclose(fp);
        fp=fopen(path2,"r");
        pid_t child_pid;
        while(fscanf(fp,"%d",&child_pid)!=EOF){
          root->child_pid[root->child_num++]=child_pid;
          assert(root->child_num<=128);
        }
      }
      else{
        proc *last_proc=get_last_proc();
        proc *cur_proc=malloc(sizeof(proc));
        fscanf(fp,"%d (%s %c %d",&cur_proc->pid,cur_proc->name,&tmp,&cur_proc->ppid);
        cur_proc->name[strlen(cur_proc->name)-1]='\0';
        last_proc->next=cur_proc;
        fclose(fp);
        fp=fopen(path2,"r");
        pid_t child_pid;
        cur_proc->child_num=0;
        cur_proc->next=NULL;
        while(fscanf(fp,"%d",&child_pid)!=EOF){
          cur_proc->child_pid[cur_proc->child_num++]=child_pid;
          assert(cur_proc->child_num<=128);
        }
      }
      fclose(fp); 
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
    get_procs();
    //debugprint();
    print_tree(root);
  }
  return 0;
}