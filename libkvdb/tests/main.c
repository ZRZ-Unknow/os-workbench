#include <kvdb.h>
#include <stdlib.h>
#include <stdio.h>

int main(){
  struct kvdb *db=kvdb_open("a.db");
  const char *key = "operating-systems";
  char *value="d";

  //panic_on(!(db = kvdb_open("a.db")), "cannot open db"); // 打开数据库

  //kvdb_put(db, key, "three-easy-pieces"); // db[key] = "three-easy-pieces"
  //value = kvdb_get(db, key); // value = db[key];
  kvdb_close(db); // 关闭数据库
  //printf("[%s]: [%s]\n", key, value);
  free(value);
  return 0;
}