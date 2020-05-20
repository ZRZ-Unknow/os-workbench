#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

char filename[128];

void recover(){

}


int main(int argc, char *argv[]) {
  assert(argc>=3);
  assert(strcmp(argv[1],"frecov")==0);
  sprintf(filename,"%s",argv[2]);
  printf("%s\n",filename);
  recover();
  return 0;
}
