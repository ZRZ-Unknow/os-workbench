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
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  return 0;
}
