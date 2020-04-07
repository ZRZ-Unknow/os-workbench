#include <stdio.h>
#include <string.h>

static char line[4096];
static char func_name[128];
static char wrap_name[128];
static int func_num=0;
static int wrap_num=0;

void compile(){
}

int main(int argc, char *argv[]) {
  static char tmp[4];
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
