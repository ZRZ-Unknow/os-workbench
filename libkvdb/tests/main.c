#include <kvdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define random(x) (rand()%x)

char string[27]="abcdefghijklmnopqrstuvwxyz";

char *gen_string(int n){
  char *str=malloc(n);
  for(int i=0;i<n-1;i++){
    str[i]=string[random(26)];
  }
  str[n-1]='\0';
  return str;
}

int test(){
  struct stat buf;
  char filename[5]="b.db";
  int fd=open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IXUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  
  write(fd,"The first try\n",14);
  stat(filename,&buf);
  printf("%d\n",buf.st_size);
  lseek(fd,8,SEEK_END);
  write(fd,"please",6);
  stat(filename,&buf);
  printf("%d\n",buf.st_size);
  close(fd);
  return 0;
}



int main(){
  struct kvdb *db=kvdb_open("a.db");
  const char *key = "operating-systems";
  char *value;

  //panic_on(!(db = kvdb_open("a.db")), "cannot open db"); // 打开数据库
  //printf("%d,%d\n",strlen(tmp),strlen(k));
  //kvdb_put(db,k,tmp); 
  //kvdb_put(db,"non","ksdjf-385");
  kvdb_put(db, key, "three-easy-pieces"); // db[key] = "three-easy-pieces"
  value=kvdb_get(db, key); // value = db[key];
  //printf("[%s]: [%s]\n",k,kvdb_get(db,k));
  //kvdb_close(db); // 关闭数据库
  printf("[%s]: [%s]\n", key, value);
  free(value);
  for(int i=0;i<50;i++){
    int n1=random(128);
    char *k=gen_string(n1);
    int n2=random(4096);
    char *v=gen_string(n2);
    kvdb_put(db,k,v);
  }
  kvdb_close(db);
  test();
  return 0;
}