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

struct r{
  char flag;
  char ksize[3];
  char vsize[7];
  char key[128];
};


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
  lseek(fd,-8,SEEK_CUR);
  write(fd,"hello",5);
  stat(filename,&buf);
  printf("%d\n",buf.st_size);
  close(fd);
  return 0;
}

int test1(){
  struct stat buf;
  char filename[5]="c.db";
  int fd=open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IXUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  //struct r *p=malloc(sizeof(struct r));
  /*printf("dddd\n");
  char t[150];
  char *tmp=&t[0];
  read(fd,tmp,150);
  for(int i=0;i<5;i++){
    printf("%s",tmp[i]);
  }*/
  /*printf("%s ",p->flag);
  printf("%s ",p->ksize);
  printf("%s ",p->vsize);
  printf("%s\n",p->key);*/
  char tmp[150]="aksdfj aksldjfasdf  ";
  char *p=strtok(tmp," ");
  while(p){
    printf("k:%s,%d\n",p,strlen(p));
    p=strtok(NULL," ");
  }
  char *a=malloc(16*1024*1024);
  assert(a);
  return 0;
  
}

int db_test(){
  struct kvdb *db=kvdb_open("a.db");
  const char *key = "operating-systems";
  char *value;

  //panic_on(!(db = kvdb_open("a.db")), "cannot open db"); // 打开数据库
  //printf("%d,%d\n",strlen(tmp),strlen(k));
  //kvdb_put(db,k,tmp); 
  //kvdb_put(db,"non","ksdjf-385");
  kvdb_put(db, key, "three-easy-pieces"); // db[key] = "three-easy-pieces"
  //value=kvdb_get(db, key); // value = db[key];
  //printf("[%s]: [%s]\n",k,kvdb_get(db,k));
  printf("[%s]: [%s]\n", key, value);
  //free(value);
  /*for(int i=0;i<10;i++){
    int n1=random(128);
    char *k=gen_string(n1);
    int n2=random(4096);
    char *v=gen_string(n2);
    kvdb_put(db,k,v);
    char *p=kvdb_get(db,k);
    assert(0);
    printf("%s\n",p);
    assert(strcmp(p,v)==0);
  }*/
  kvdb_close(db);
}

int main(){
  db_test();
  return 0;
}