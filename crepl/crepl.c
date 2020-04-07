#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

static char line[4096];
static char tmp[4];
static char src_filename[32];
static char dst_filename[32];
static char wrap_name[32];

void compile(){
  sprintf(src_filename,"/tmp/func_c_XXXXXX");
  sprintf(dst_filename,"/tmp/func_so_XXXXXX");
  if(mkstemp(src_filename)==-1) printf("mkstemp failed\n");
  if(mkstemp(dst_filename)==-1) printf("mkstemp failed\n");
  FILE *fp=fopen(src_filename,"w");
  fprintf(fp,"%s",line);
  fclose(fp);
  char *exec_argv[]={"gcc","-fPIC","-w","-shared","-o",dst_filename,src_filename,NULL};
  int fildes[2];
  if(pipe(fildes)!=0) assert(0);
  int pid=fork();
  if(pid==0){
    int fd=open("/dev/null",O_RDWR);
    dup2(fd,STDOUT_FILENO);
    dup2(fd,STDERR_FILENO);
    execvp(exec_argv[0],exec_argv);
  }
  unlink(src_filename);
  unlink(dst_filename);
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
