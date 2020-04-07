#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char line[4096];
static char tmp[4];
static char func_name[128];
static char wrap_name[128];

void compile(){
  sprintf(func_name,"/tmp/zrz_crepl/func_XXXXXX");
  int fd=mkstemp(func_name);
  if(fd==-1){
    printf("error\n");
  }
}

int main(int argc, char *argv[]) {
  while (1) {
    printf("crepl> ");
    memset(line,'\0',sizeof(line));
    memset(tmp,'\0',sizeof(tmp));
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    sscanf(line,"%3s",tmp);
    if(strncmp(line,"int",3)==0){
      compile();
    }
    else{
      printf("It's a expression\n");
    }
  }
}
