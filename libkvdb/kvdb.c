#include <stdio.h>
#include <assert.h>
#include <sys/file.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "kvdb.h"
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

//#define DEBUG
#ifdef DEBUG
#define Log(format, ...) \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define Log(format,...)
#endif

/*在Journal中，用!表示有效，*表示无效；在db中，用０表示短value，１表示长value，２表示长改短
*/

#define SV 0
#define LV 1
#define LSV 2


#define B *1
#define KB B*1024
#define MB KB*1024
#define JSIZE (20 MB)
#define KEYSIZE (128 B)
#define SVALUESIZE (4 KB)
#define LVALUESIZE (16 MB)
#define DBSL (131+SVALUESIZE)     //db-shortline
#define DBLL (131+LVALUESIZE)     //db-longline

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
  //int start;  
  int size;
  int committing;
  char filename[128];
};

struct stat buf;

/*char *myread(int fd,int db_case){
  if(db_case==0){   //readkey
    char *key=malloc(KEYSIZE+1);
    char tmp;
    for(int i=0;i<KEYSIZE+1;i++){
      read(fd,&tmp,1);
      if(tmp==' ' || tmp=='\n'){
        key[i]='\0';
        //printf("key:%s\n",key);
        return key;
      }
      key[i]=tmp;
    }
    free(key);
  }
  else if(db_case==1){       //readvalue
    char *value=malloc(VALUESIZE+1);
    char tmp;
    for(int i=0;i<VALUESIZE+1;i++){
      read(fd,&tmp,1);
      if(tmp==' ' || tmp=='\n'){
        value[i]='\0';
        //printf("value:%s\n",value);
        return value;
      }
      value[i]=tmp;
    }
    free(value);
  }
  else{
    assert(0);
  }
  return NULL; 
}*/
/*
bool find_key(struct kvdb *db,const char *key){
  for(int i=0;i<(db->size-db->start)/LINESIZE;i++){
    lseek(db->fd,db->start+i*LINESIZE,SEEK_SET);
    char *k=myread(db->fd,0);
    if(k==NULL || strcmp(k,key)!=0){
      free(k);
      continue;
    }
    free(k);
    return true;
  }
  return false;
}*/
/*
int replay_put(struct kvdb *db, const char *key, const char *value) {
  if(find_key(db,key)==false){
    lseek(db->fd,0,SEEK_END);
    write(db->fd,key,strlen(key));
    write(db->fd," ",1);
    write(db->fd,value,strlen(value));
  }
  else{
    write(db->fd,value,strlen(value));
  }
  if(strlen(value)+strlen(key)+2<LINESIZE){
      write(db->fd," ",1);
      for(int i=0;i<LINESIZE-strlen(key)-strlen(value)-3;i++){
        write(db->fd,"0",1);
      }
    }
  write(db->fd,"\n",1);
  fsync(db->fd);
  stat(db->filename,&buf);
  db->size=buf.st_size;
  return 0;
}*/
/*
int replay(struct kvdb *db){
  lseek(db->fd,0,SEEK_SET);
  char c;
  if(read(db->fd,&c,1)>0){
    if(c=='!'){  //begin replay
      lseek(db->fd,2,SEEK_SET);
      char *key=myread(db->fd,0);
      lseek(db->fd,288,SEEK_SET);
      char *value=myread(db->fd,1);
      //printf("replay_put:%s,%s\n",key,value);
      replay_put(db,key,value);
      lseek(db->fd,0,SEEK_SET);
      write(db->fd,"*",1);
      fsync(db->fd);
    }
  }
  return 0;
}*/



struct kvdb *kvdb_open(const char *filename) {
  int fd=open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IXUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  if(stat(filename,&buf)!=0) assert(0);
  struct kvdb *db=malloc(sizeof(struct kvdb));
  db->fd=fd;
  strncpy(db->filename,filename,sizeof(db->filename));
  db->committing=0;
  //flock(db->fd,LOCK_EX);
  if(buf.st_size==0){
    char *tmp=malloc(JSIZE-2);
    memset(tmp,0,JSIZE-2);
    write(db->fd,"*",1);
    write(db->fd,tmp,JSIZE-2);
    write(db->fd,"\n",1);
    stat(filename,&buf);
    db->size=buf.st_size;
    assert(db->size==JSIZE);
  }
  else{
    db->size=buf.st_size;
    printf("size:%d\n",db->size);
    //lseek(db->fd,db->size,SEEK_SET);
    //write(db->fd,"h",1);
    //replay(db);
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
  //flock(db->fd,LOCK_UN);
  return db;
}

int kvdb_close(struct kvdb *db) {
  while(db->committing==1);
  close(db->fd);
  free(db);
  return 0;
}



char *kvdb_get(struct kvdb *db, const char *key) {
  //flock(db->fd,LOCK_EX);
  int offset=JSIZE;
  while(offset<db->size){
    lseek(db->fd,offset,SEEK_SET);
    char flag;
    read(db->fd,&flag,1);
    switch (flag)
    {
    case '0': 
      char *str=malloc(DBSL+1);
      read(db->fd,str,DBSL)
      break;
    
    default:
      break;
    }
  }
  /*for(int i=0;i<(db->size-db->start)/LINESIZE;i++){
    lseek(db->fd,db->start+i*LINESIZE,SEEK_SET);
    char *k=myread(db->fd,0);
    if(k==NULL || strcmp(k,key)!=0){
      free(k);
      continue;
    }
    char *value=myread(db->fd,1);
    //flock(db->fd,LOCK_UN);
    return value;
  }*/
  //flock(db->fd,LOCK_UN);
  return NULL;
}

int journal_put(struct kvdb *db,const char *key,const char *value){
  lseek(db->fd,0,SEEK_SET);
  write(db->fd,"*",1);
  int len1=strlen(key);
  int len2=strlen(value);
  lseek(db->fd,2,SEEK_SET);
  //char len[64];
  //sprintf(len,"%d %d ",len1,len2);
  //printf("%s,%d\n",len,(int)strlen(len));
  //write(db->fd,&len,strlen(len));
  write(db->fd,key,len1);
  if(len1<=128) write(db->fd," ",1);
  lseek(db->fd,288,SEEK_SET); 
  write(db->fd,value,len2);
  if(len2<4 KB) write(db->fd," ",1);
  fsync(db->fd);
  lseek(db->fd,0,SEEK_SET);
  write(db->fd,"!",1);
  fsync(db->fd);
  return 0;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
  /*Log("%s,%s",key,value); 
  //flock(db->fd,LOCK_EX);
  journal_put(db,key,value);
  if(find_key(db,key)==false){
    lseek(db->fd,0,SEEK_END);
    write(db->fd,key,strlen(key));
    write(db->fd," ",1);
    write(db->fd,value,strlen(value));
  }
  else{
    write(db->fd,value,strlen(value));
  }
  if(strlen(value)+strlen(key)+2<LINESIZE){
      write(db->fd," ",1);
      for(int i=0;i<LINESIZE-strlen(key)-strlen(value)-3;i++){
        write(db->fd,"0",1);
      }
    }
  write(db->fd,"\n",1);
  fsync(db->fd);
  //flock(db->fd,LOCK_UN);
  stat(db->filename,&buf);
  db->size=buf.st_size;*/
  return 0;
}



