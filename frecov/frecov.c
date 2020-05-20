#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>



int main(int argc, char *argv[]) {
    char buf[128];
    for(int i=0;i<argc;i++){
        printf("%s\n",argv[i]);
    }
    return 0;
}
