#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <wait.h>
#include <dlfcn.h>
#include <stdbool.h>

#if defined(__i386__)
  #define TARGET "-m32"
#elif defined(__x86_64__)
  #define TARGET "-m64"
#endif

static char line[4096];
static char tmp[4];
static char src_filename[32];
static char dst_filename[32];
static int (*f)();
static int value;

void compile(bool func){
  sprintf(src_filename,"/tmp/func_c_XXXXXX");
  sprintf(dst_filename,"/tmp/func_so_XXXXXX");
  if(mkstemp(src_filename)==-1) printf("\033[1;31m      Mkstemp Failed!\033[0m\n");
  if(mkstemp(dst_filename)==-1) printf("\033[1;31m      Mkstemp Failed!\033[0m\n");
  FILE *fp=fopen(src_filename,"w");
  if(func) fprintf(fp,"%s",line);
  else fprintf(fp,"int wrap_func(){return (%s);}",line);
  fclose(fp);
  char *exec_argv[]={"gcc",TARGET,"-x","c","-fPIC","-w","-shared","-o",dst_filename,src_filename,NULL};
  int pid=fork();
  if(pid==0){
    int fd=open("/dev/null",O_RDWR);
    dup2(fd,STDOUT_FILENO);
    dup2(fd,STDERR_FILENO);
    execvp(exec_argv[0],exec_argv);
  }
  else{
    int status;
    wait(&status);
    if(status!=0) printf("\033[1;31m      Compile Error!\033[0m\n");
    else{
      void *handle=dlopen(dst_filename,RTLD_NOW|RTLD_GLOBAL);
      if(!handle) printf("\033[1;31m      Compile Error!\033[0m\n");
      else{ 
        if(func) printf("\033[1;32m      Added: \033[1;30m%s\033[0m",line);
        else{
          f=dlsym(handle,"wrap_func");
          value=f();
          printf("\033[1;32m      Result: \033[1;30m%d\033[0m\n",value);
          dlclose(handle); 
        }
      }
    }
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
    compile(strncmp(line,"int",3)==0);
  }
}

