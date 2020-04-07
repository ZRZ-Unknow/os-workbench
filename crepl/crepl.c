#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <wait.h>
#include <dlfcn.h>

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
int (*f)();

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
      else{
        void *handle=dlopen(dst_filename,RTLD_LAZY|RTLD_GLOBAL);
        if(!handle) printf("\033[1;31m      Compile Error!\033[0m\n");
        else printf("\033[1;32m      Added: \033[1;30m%s\033[0m",line);
      }
    }
    else printf("\033[1;31m      Compile Error!\033[0m\n");
  }
  unlink(src_filename);
  unlink(dst_filename);
}
void run(){
  sprintf(src_filename,"/tmp/func_c_XXXXXX");
  sprintf(dst_filename,"/tmp/func_so_XXXXXX");
  if(mkstemp(src_filename)==-1) printf("mkstemp failed\n");
  if(mkstemp(dst_filename)==-1) printf("mkstemp failed\n");
  FILE *fp=fopen(src_filename,"w");
  fprintf(fp,"int wrap_func(){return %s;}",line);
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
    int status;
    wait(&status);
    if(WIFEXITED(status)!=0){
      if(WEXITSTATUS(status)!=0) printf("\033[1;31m      Compile Error!\033[0m\n");
      else{
        void *handle=dlopen(dst_filename,RTLD_LAZY|RTLD_GLOBAL);
        if(!handle) printf("\033[1;31m      Compile Error!\033[0m\n");
        else{
          f=dlsym(handle,"wrap_func");
          int value=f();
          printf("\033[1;32m      Result: \033[1;30m%d\033[0m\n",value); 
        } 
        dlclose(handle);
      }
    }
    else printf("\033[1;31m      Compile Error!\033[0m\n");
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
      run();
    }
  }
}
/*void *dlopen( const char * pathname, int mode );
以指定模式打开指定的动态链接库,返回一个句柄(handle),失败返回NULL
  1、解析方式
    RTLD_LAZY：在dlopen返回前，对于动态库中的未定义的符号不执行解析（只对函数引用有效，对于变量引用总是立即解析）。
    RTLD_NOW： 需要在dlopen返回前，解析出所有未定义符号，如果解析不出来，在dlopen会返回NULL，错误为：: undefined symbol: xxxx.......
  2、作用范围，可与解析方式通过“|”组合使用。 
    RTLD_GLOBAL：动态库中定义的符号可被其后打开的其它库重定位。 
    RTLD_LOCAL： 与RTLD_GLOBAL作用相反，动态库中定义的符号不能被其后打开的其它库重定位。如果没有指明是RTLD_GLOBAL还是RTLD_LOCAL，则缺省为RTLD_LOCAL
  3、作用方式
    RTLD_NODELETE： 在dlclose()期间不卸载库，并且在以后使用dlopen()重新加载库时不初始化库中的静态变量。这个flag不是POSIX-2001标准。 
    RTLD_NOLOAD： 不加载库。可用于测试库是否已加载(dlopen()返回NULL说明未加载，否则说明已加载），也可用于改变已加载库的flag，如：先前加载库的flag为RTLD＿LOCAL，用dlopen(RTLD_NOLOAD|RTLD_GLOBAL)后flag将变成RTLD_GLOBAL。这个flag不是POSIX-2001标准。
    RTLD_DEEPBIND：在搜索全局符号前先搜索库内的符号，避免同名符号的冲突。这个flag不是POSIX-2001标准。
void *dlsym(void *handle, const char* symbol);
handle是使用dlopen函数之后返回的句柄，symbol是要求获取的函数的名称，函数，返回值是void*,指向函数的地址，供调用使用*/
