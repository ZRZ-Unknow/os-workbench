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

void debugprint(struct process *proc){
  printf("pid:%d ppid:%d name:%s state:%c\n",proc->pid,proc->ppid,proc->name,proc->state);
  for(struct ChildList *p=proc->children;p!=NULL;p=p->next){
    debugprint(p->child);
  }
}

//从树中的proc的位置开始搜索它的子树中是否有pid的进程
struct process *proc_find(pid_t pid,struct process *proc)
{
  if(proc->pid==pid) return proc;
  for(struct ChildList *p=proc->children;p!=NULL;p=p->next){
    struct process *res=proc_find(pid,p->child);
    if(res) return res;
  }
  return NULL; 
}

//在父节点中添加孩子节点到孩子列表中
void insert(struct process *proc,struct ChildList *child){
  if(!HAV_N || proc->children==NULL){
    child->next=proc->children;
    proc->children=child;
  }
  else{
    if(proc->children->child->pid>child->child->pid){
      child->next=proc->children;
      proc->children=child;
      return;
    }
    struct ChildList *p=proc->children->next;
    struct ChildList *pre=proc->children;
    while(p){
      if(pre->child->pid<=child->child->pid && p->child->pid>=child->child->pid)
        break;
      p=p->next;
      pre=pre->next;
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
        sprintf(thread->name,"{%.16s}",proc->name);//thread显示名字要加花括号 
        //printf("thread:%d %d %s %c\n",thread->pid,thread->ppid,thread->name,thread->state);
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
    //printf("proc:%d %d \n",child->pid,child->ppid);
    search(child);
  }
  fclose(fp);
}

//扫描/proc文件夹，将不在树中的进程加入进去
void scan(){
  DIR *procdir=opendir("/proc");
  struct dirent *entry;
  while((entry=readdir(procdir))!=NULL){
    if(strspn(entry->d_name,"0123456789")==strlen(entry->d_name)){  //是否是数字
      pid_t proc_pid=atoi(entry->d_name);
      char statpath[64]="";
      sprintf(statpath,"/proc/%d/stat",proc_pid);
      struct process *proc=malloc(sizeof(struct process));
      proc->children=NULL;proc->parent=NULL;
      FILE *fp=fopen(statpath,"r");
      fscanf(fp,"%d (%s %c %d",&proc->pid,proc->name,&proc->state,&proc->ppid);
      proc->name[strlen(proc->name)-1]='\0';
      fclose(fp);
      //看这个进程是否在树中:它本身在树中或者它的父母不在树中(进程的父母编号要比孩子小，这保证了若它的父母不应在树中则它也不应在树中)
      if(proc->ppid==0 || proc_find(proc_pid,root)!=NULL || proc_find(proc->ppid,root)==NULL){  
        free(proc);
        continue;
      }
    }
  }
  closedir(procdir);
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
  scan();
  //debugprint(root);
  return 0;
}