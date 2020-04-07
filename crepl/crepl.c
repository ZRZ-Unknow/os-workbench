#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <wait.h>

#if defined(__i386__)
  #define TARGET "-m32"
#elif defined(__x86_64__)
  #define TARGET "-m64"
#endif

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
  char *exec_argv[]={"gcc",TARGET,"-x","c","-fPIC","-w","-shared","-o",dst_filename,src_filename,NULL};
  int fildes[2];
  if(pipe(fildes)!=0) assert(0);
  int pid=fork();
  if(pid==0){
    int fd=open("/dev/null",O_RDWR);
    dup2(fd,STDOUT_FILENO);
    dup2(fd,STDERR_FILENO);
    execvp(exec_argv[0],exec_argv);
  }
  else{
/*wait(int *status):父进程一旦调用了wait就立即阻塞自己，由wait自动分析是否当前进程的某个子进程已经退出，如果让它找到了这样一个已经变成僵尸的子进程，
  wait就会收集这个子进程的信息，并把它彻底销毁后返回；如果没有找到这样一个子进程，wait就会一直阻塞在这里，直到有一个出现为止。*/
    int status;
    wait(&status);
    if(WIFEXITED(status)!=0){
      if(WEXITSTATUS(status)!=0) printf("\033[1;31m      Compile Error!\033[0m\n");
      else printf("\033[1;31m      Compile OK!\033[0m\n");;
    }
    else{
      printf("\033[1;31m      Compile Error!\033[0m\n");
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
    if(strncmp(line,"int",3)==0){
      compile();
    }
    else{
      printf("It's a expression\n");
    }
  }
}
