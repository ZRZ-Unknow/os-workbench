#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define pid_t int

struct process
{
  pid_t pid;
  pid_t ppid;
  char state;
  char name[64];
  struct process *parent;
  struct ChildList *children; 
}root_proc={.pid=1,.ppid=0,.parent=NULL,.children=NULL};

struct ChildList{
  struct process *child;
  struct ChildList *next;
};

bool HAV_V=false;
bool HAV_N=false;
bool HAV_P=false;
struct process *root=&root_proc;
//在父节点中添加孩子节点到孩子列表中
void insert(struct process *proc,struct ChildList *child){
  if(!HAV_N || proc->children==NULL){
    child->next=proc->children;
    proc->children=child;
  }
  else{
    struct ChildList *p=proc->children;
    struct ChildList *pre=p;
    int i=0;
    while(p!=NULL && p->child->pid<child->child->pid){
      if(i==0) p=p->next;
      else{
        p=p->next;
        pre=pre->next;
      }
      i++;
    }
    pre->next=child;
    child->next=p;
  }
}
void search(struct process *proc){
  //读取文件，填入进程的信息
  char statpath[64],childpath[64],threadpath[64];
  sprintf(statpath,"/proc/%d/stat",proc->pid);
  sprintf(childpath,"/proc/%d/task/%d/children",proc->pid,proc->pid);
  sprintf(threadpath,"/proc/%d/task/",proc->pid);
  FILE *fp=fopen(statpath,"r");
  fscanf(fp,"%d (%s %c %d",&proc->pid,proc->name,&proc->state,&proc->ppid);
  proc->name[strlen(proc->name)-1]='\0';
  printf("root:%d %d %s %c\n",proc->pid,proc->ppid,proc->name,proc->state);
  fclose(fp);
   // 打开thread,thread是没有孩子的
  DIR *taskdir=opendir(threadpath);
  struct dirent* entry;
  while((entry=readdir(taskdir))!=NULL){
    if(strspn(entry->d_name,"0123456789")==strlen(entry->d_name)){
      if(atoi(entry->d_name)!=proc->pid){
        struct process *thread=malloc(sizeof(struct process));
        thread->pid=atoi(entry->d_name); thread->ppid=proc->pid; thread->state=proc->state; thread->parent=proc; thread->children=NULL;
        sprintf(thread->name,"{%.16s}",proc->name); 
        printf("thread:%d %d %s %c\n",thread->pid,thread->ppid,thread->name,thread->state);
        struct ChildList *thread_child=malloc(sizeof(struct ChildList));
        thread_child->child=thread;
        insert(proc,thread_child);
      }
    }
  }
  closedir(taskdir);
  //递归寻找孩子
  fp=fopen(childpath,"r");
  pid_t child_id;
  while(fscanf(fp,"%d",&child_id)!=EOF){
    struct process *child=malloc(sizeof(struct process));
    child->pid=child_id; child->ppid=proc->pid; child->parent=proc; child->children=NULL;
    struct ChildList *_child=malloc(sizeof(struct ChildList));
    _child->child=child;
    insert(proc,_child); 
    printf("proc:%d %d \n",child->pid,child->ppid);
    search(child);
  }
  fclose(fp);
}
int main(int argc, char *argv[]) {
  int i;
  for (i = 1; i < argc; i++) {
    if(strcmp(argv[i],"pstree")==0){
      continue;
    }
    if(strcmp(argv[i],"-p")==0 || strcmp(argv[i],"--show-pids")==0)  HAV_P=true;
    else if(strcmp(argv[i],"-n")==0 || strcmp(argv[i],"--numeric-sort")==0) HAV_N=true;
    else if(strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0) HAV_V=true;
    else{
      printf("only these arguments are allowed:-p,--show-pids,-n,--numeric-sort,-V,--version\n");
      return -1;
    }
  }
  if(HAV_V){
    printf("pstree v1.0\nCopyright (C) 2020 ZRZ\n");
    return 0;
  }
  search(root);
  return 0;
}