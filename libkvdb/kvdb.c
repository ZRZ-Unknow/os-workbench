#include <stdio.h>
#include <assert.h>
#include <sys/file.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "kvdb.h"
#include <unistd.h>


#define B *1
#define KB B*1024
#define MB KB*1024
#define JSIZE (32 MB)

//读写文件数据 (以及管理偏移量) 时使用 read, write 和 lseek，同步数据时使用 fsync。
typedef struct journal{
  int key_lenth,value_lenth;
  char *key,*value;
}journal;

struct kvdb {
  int fd;
  int start;  
  int size;
  int committing;
  journal jn;
};

struct kvdb *kvdb_open(const char *filename) {
  int fd=open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IXUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  struct stat buf;
  if(stat(filename,&buf)!=0) assert(0);
  struct kvdb *db=malloc(sizeof(struct kvdb));
  db->fd=fd;
  if(buf.st_size==0){
    write(db->fd,"d",1);
  }
  else{
    //recover

  }
  printf("%ld",buf.st_size);
  return NULL;
}

int kvdb_close(struct kvdb *db) {
  return -1;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
  return -1;
}

char *kvdb_get(struct kvdb *db, const char *key) {
  return NULL;
}
