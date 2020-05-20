#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>



int main(int argc, char *argv[]) {
  char buf[128];
  assert(argc>=3);
  assert(argv[1]=="frecov");
  sprintf(buf,"%s",argv[2]);
  printf("%s\n",buf);
  
  return 0;
}
