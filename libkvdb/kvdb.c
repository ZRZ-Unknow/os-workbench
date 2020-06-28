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
#define KEYSIZE (128 B)
#define VALUESIZE (4 KB)
#define LINESIZE (KEYSIZE+1+VALUESIZE+1)
#define LEN1 (3 B)
#define LEN2 (8 B)


//144处为第二行key开始

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
  db->committing=0;
  if(buf.st_size==0){
    for(int i=0;i<2;i++){
      write(db->fd,"0",1);
      for(int j=0;j<71;j++){
        write(db->fd,"00",2);
      }
      write(db->fd,"\n",1);
    }
    for(int i=0;i<2;i++){
      for(int j=0;j<512;j++){
        write(db->fd,"00000000",8);
      }
      write(db->fd,"\n",1);
    }
    printf("%ld\n",buf.st_size);
  }
  else{
    //recover
    /*printf("size:%ld\n",buf.st_size);
    char c;
    write(db->fd,"1 kaer7324",10);
    lseek(db->fd,144,SEEK_SET);
    write(db->fd,"key2",4);
    lseek(db->fd,288,SEEK_SET);
    write(db->fd,"value1",6);
    lseek(db->fd,288+4096+1,SEEK_SET);
    write(db->fd,"value2",6);
    lseek(db->fd,288+4097+4097,SEEK_SET);
    write(db->fd,"kd",2);
    while(read(db->fd,&c,1)!=0){
      printf("%s",&c);
    }*/
  }
  return NULL;
}

int kvdb_close(struct kvdb *db) {
  while(db->committing==1);
  close(db->fd);
  free(db);
  return 0;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
  return -1;
}

char *kvdb_get(struct kvdb *db, const char *key) {
  return NULL;
}
