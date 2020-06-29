#include <kvdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
  struct kvdb *db=kvdb_open("a.db");
  const char *key = "operating-systems";
  char *value;

  //panic_on(!(db = kvdb_open("a.db")), "cannot open db"); // 打开数据库
  char *tmp=malloc(4096);
  memset(tmp,'k',4096);
  printf("%s\n",tmp);
  kvdb_put(db, key, tmp); // db[key] = "three-easy-pieces"
  value=kvdb_get(db, key); // value = db[key];
  kvdb_close(db); // 关闭数据库
  printf("[%s]: [%s]\n", key, value);
  free(value);
  return 0;
}