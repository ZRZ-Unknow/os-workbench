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
  char vsize[8];
  char vpos[10];
  char key[12];
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
  /*write(fd,"mypen",5);
  lseek(fd,8-5-1,SEEK_CUR);
  write(fd,"*",1);
  write(fd,"mypen",5);
  lseek(fd,3-1,SEEK_CUR);
  write(fd,"*",1);*/

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
  int keylen=76;
  int valuelen=987;
  int valuepos=90837653;
  char *key="osppt";
  char *kl=malloc(34+strlen(key)+1);
  sprintf(kl,"!%-'#11d%-'#11d%-'#11d%s\0",keylen,valuelen,valuepos,key);
  printf("%s,%d\n",kl,strlen(kl));
  write(fd,kl,strlen(kl)+1);
  char *a=malloc(100);
  lseek(fd,0,SEEK_SET);
  read(fd,a,100);
  if(strcmp(a,kl)==0){
    printf("dd\n");
  } 
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
  value=kvdb_get(db, key); // value = db[key];
  //printf("[%s]: [%s]\n",k,kvdb_get(db,k));
  printf("[%s]: [%s]\n", key, value);
  free(value);
  for(int i=0;i<100;i++){
    int n1=random(128);
    char *k=gen_string(n1);
    int n2=random(4096*3);
    char *v=gen_string(n2);
    kvdb_put(db,k,v);
    char *p=kvdb_get(db,k);
    //printf("%s\n",p);
    assert(strcmp(p,v)==0);
    free(p);
    char *v2=gen_string(random(4096*3));
    kvdb_put(db,k,v2);
    p=kvdb_get(db,k);
    //printf("%s\n",p);
    assert(strcmp(p,v2)==0);
    free(p);
    free(k);
    free(v);
    free(v2);
  }
  kvdb_close(db);
}

int test2(){
  struct stat buf;
  char filename[5]="c.db";
  int fd=open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IXUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  struct r *p=malloc(sizeof(struct r));
  printf("dddd\n");
  read(fd,p,sizeof(struct r));
  for(int i=0;i<sizeof(struct r);i++){
    printf("%s",p[i]);
  }
  int q=strtol(p->ksize,NULL,10);
  printf(" %d\n",q);
  /*printf("%s ",p->ksize);
  printf("%s ",p->vsize);
  printf("%s ",p->vpos);
  printf("%s\n",p->key);*/
}

int main(){
  db_test();
  return 0;
}