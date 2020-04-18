#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>

#define NUM 1024

typedef struct system_call{
  char name[64];
  double time;
}system_call;
double total_time=0;
int syscall_num=0;
system_call sys_call[NUM];
char div_0[10]="\0\0\0\0\0\0\0\0\0\0";

void debugprint(){
  for(int i=0;i<syscall_num;i++){
    printf("%s:%f\n",sys_call[i].name,sys_call[i].time);
  }
}

void insert(char *name,double time){
  total_time+=time;
  int pos=-1;
  for(int i=0;i<syscall_num;i++){
    if(strcmp(name,sys_call[i].name)==0)
      pos=i;
  }
  if(pos==-1){   //一个新的系统调用
    sys_call[syscall_num].time=time;
    strcpy(sys_call[syscall_num].name,name);
    syscall_num++;
  }
  else{
    sys_call[pos].time+=time;
  }
}

int time_cmp(const void *syscall_1,const void *syscall_2){
  return ((system_call*)syscall_1)->time <= ((system_call*)syscall_2)->time;
}

void sort(){
  qsort(sys_call,syscall_num,sizeof(system_call),time_cmp);
}

void display(){
  for(int i=0;i<syscall_num;i++){
    if(i==0) printf("%20s \033[1;31m(%9.6lf%%)\033[0m\n",sys_call[i].name,100*sys_call[i].time/total_time);
    else if(sys_call[i].time/total_time>0.01) printf("%20s \033[1;32m(%9.6lf%%)\033[0m\n",sys_call[i].name,100*sys_call[i].time/total_time);
    else printf("%20s \033[1;30m(%9.6lf%%)\033[0m\n",sys_call[i].name,100*sys_call[i].time/total_time);
  }
  for(int i=0;i<8;i++) printf("%s",div_0);
  fflush(stdout);
}

char *find_path(char *Path,char *filename){
  char *path=strtok(Path,":");
  DIR *dir;
  struct dirent *entry;
  while(path){  //find file in path
    dir=opendir(path);
    if(!dir){
      path=strtok(NULL,":");
      continue;
    }
    while((entry=readdir(dir))!=NULL){
      if(strcmp(entry->d_name,filename)==0){
        closedir(dir);
        return path;
      }
    }
    closedir(dir);
    path=strtok(NULL,":");
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  
  char **exec_env=__environ;
  char *exec_argv[argc+2];
  exec_argv[0]="strace";
  exec_argv[1]="-Txx";
  memcpy(exec_argv+2,argv+1,argc*sizeof(char*));
  char cmd_path[256];
  char exec_path[256];
  
  char path1[512];
  strcpy(path1,getenv("PATH"));
  char *strace_path=find_path(path1,"strace");
  sprintf(exec_path,"%s/%s",strace_path,"strace");

  char path2[512];
  strcpy(path2,getenv("PATH"));
  if(strstr(argv[1],"/")==NULL){     
    char *_cmd_path=find_path(path2,argv[1]);       
    sprintf(cmd_path,"%s/%s",_cmd_path,argv[1]);
    exec_argv[2]=&cmd_path[0];
  }
  
  int fildes[2];
  if(pipe(fildes)!=0) assert(0);
  int pid=fork();
  if(pid==0){
    close(fildes[0]);
    int fd=open("/dev/null",O_RDWR);
    dup2(fd,STDOUT_FILENO);
    dup2(fildes[1],STDERR_FILENO);
    execve(exec_path, exec_argv, exec_env);
  }
  else{
    close(fildes[1]);
    dup2(fildes[0],STDIN_FILENO);
    //FILE *fp=fdopen(fildes[0],"r");
    char buf[4096];
    char *pattern="<([0-9]*\\.[0-9]*)>";
    regex_t reg;
    regmatch_t pmatch;
    int ret=regcomp(&reg,pattern,REG_EXTENDED);
    time_t begin,end;
    begin=time(NULL);
    while(fgets(buf,4096,stdin)!=NULL){
      if(strncmp(buf,"select",6)==0)
        printf("%s\n",buf);
      ret=regexec(&reg,buf,1,&pmatch,0);
      if(!ret){
        char time_buf[256],name_buf[256];
        memset(time_buf,'\0',sizeof(time_buf));
        memset(name_buf,'\0',sizeof(name_buf));
        double t=0;
        strncpy(&time_buf[0],buf+pmatch.rm_so+1,pmatch.rm_eo-pmatch.rm_so-2);
        /*int j;
        for(j=strlen(buf)-1;j>=0;j--){
          if(buf[j]=='<') break;
        }
        strncpy(&time_buf[0],buf+j,strlen(buf)-j);
        int i;
        for(i=0;i<strlen(buf);i++){
          if(buf[i]=='(') break;
        }
        strncpy(&name_buf[0],buf,i);*/
        //printf("%s\n",name_buf);
        sscanf(buf,"%[A-z0-9_](",name_buf);
        sscanf(time_buf,"%lf",&t);
        //for(int i=0;i<strlen(name_buf);i++)
          //name_buf[i]=tolower(name_buf[i]);
        //printf("%s:%f\n",name_buf,t);
        if(name_buf[0]=='\0' || t==0 || t>1){
          //printf("%s\n",buf);
          //assert(0);
          memset(buf,'\0',sizeof(buf));
          continue;
        }
        insert(name_buf,t);
      }
      else{
        memset(buf,'\0',sizeof(buf));
        continue;
      }
      end=time(NULL);
      if((end-begin)>1){
        sort();
        display();
        begin=time(NULL);
      }
      memset(buf,'\0',sizeof(buf));
    }
    sort();
    display();
    regfree(&reg);
    //fclose(fp);
    //printf("%s,%s,%s,%s\n",exec_path,exec_argv[0],exec_argv[1],exec_argv[2]);
  }
  return 0;
}
