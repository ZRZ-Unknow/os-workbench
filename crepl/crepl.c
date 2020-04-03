#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  static char line[4096];
  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    for(int i=0;i<strlen(line);i++){
      printf("%d-",line[i]);
    }
    printf("Got %zu chars.\n", strlen(line)); // WTF?
  }
}
