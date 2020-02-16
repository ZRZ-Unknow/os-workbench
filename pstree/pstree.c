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
  return 0;
}