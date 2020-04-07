#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  static char line[4096];
  while (1) {
    printf("crepl> ");
    memset(line,'\0',sizeof(line));
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    if(strncmp(line,"int",3)==0){
      printf("It's a function\n");
    }
    else{
      printf("It's a expression\n");
    }
    for(int i=0;i<strlen(line);i++){
      printf("%d-",line[i]);
    }
    printf("##%d,%d##\n",'\n','r');
    printf("Got %zu chars.\n", strlen(line)); // WTF?
  }
}
