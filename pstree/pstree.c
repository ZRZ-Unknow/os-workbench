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
  printf("%d,%d,%d\n",P,N,V);
  return 0;
}