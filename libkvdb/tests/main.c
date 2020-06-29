#include <kvdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char string[27]="abcdefghijklmnopqrstuvwxyz";

char *gen_string(int n){
  char *str=malloc(n);
  for(int i=0;i<n-1;i++){
    str[i]=string[rand(25)];
  }
  str[n-1]='\0';
  return str;
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
  kvdb_close(db); // 关闭数据库
  printf("[%s]: [%s]\n", key, value);
  free(value);
  for(int i=0;i<50;i++){
    int n1=rand(128);
    char *k=gen_string(n1);
    int n2=rand(4096);
    char *v=gen_string(n2);
    kvdb_put(db,k,v);
  }
  return 0;
}