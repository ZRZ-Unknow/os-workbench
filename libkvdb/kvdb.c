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

#define B *1
#define KB B*1024
#define MB KB*1024
#define JSIZE (20 MB)
#define KEYSIZE (128 B)
#define SVALUESIZE (4 KB)
#define LVALUESIZE (16 MB)
#define KEYLINE (163 B)
#define KEYNUM (4 KB)


//144处为第二行key开始

//读写文件数据 (以及管理偏移量) 时使用 read, write 和 lseek，同步数据时使用 fsync。
typedef struct keyline{
  char flag;
  char keylen[11];
  char valuelen[11];
  char valuepos[11];
  char key[129];
}keyline;

typedef struct jnkl{
  char flag;
  char keylen[11];
  char valuelen[11];
  char valuepos[11];
}jnkl;

struct kvdb {
  int fd;
  int size;
  int committing;
  char filename[128];
};

struct stat buf;

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
}


int replay(struct kvdb *db){
  lseek(db->fd,0,SEEK_SET);
  char c;
  jnkl *jkl=malloc(sizeof(jnkl));
  if(read(db->fd,jkl,sizeof(jnkl))>0){
    if(jkl->flag=='!'){  //begin replay
      int keylen=strtol(jkl->keylen,NULL,10);
      int valuelen=strtol(jkl->valuelen,NULL,10);
      int valuepos=strtol(jkl->valuepos,NULL,10);
      char *key=malloc(keylen+1);
      char *value=malloc(valuelen+1);
      read(db->fd,key,keylen+1);
      read(db->fd,value,valuelen);
      value[valuelen]='\0';
      replay_put(db,key,value);
      lseek(db->fd,0,SEEK_SET);
      write(db->fd,"*",1);
      fsync(db->fd);
    }
  }
  return 0;
}



struct kvdb *kvdb_open(const char *filename) {
  int fd=open(filename,O_RDWR|O_CREAT,S_IRUSR|S_IXUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  if(stat(filename,&buf)!=0) assert(0);
  struct kvdb *db=malloc(sizeof(struct kvdb));
  db->fd=fd;
  strncpy(db->filename,filename,sizeof(db->filename));
  flock(db->fd,LOCK_EX);
  db->committing=0;
  if(buf.st_size==0){
    write(db->fd,"*",1);
    lseek(db->fd,JSIZE-2,SEEK_CUR);
    write(db->fd,"\n",1);
    lseek(db->fd,KEYLINE*KEYNUM-1,SEEK_CUR);
    write(db->fd," ",1);
    stat(filename,&buf);
    db->size=buf.st_size;
    assert(db->size==JSIZE+KEYLINE*KEYNUM);
  }
  else{
    db->size=buf.st_size;
    replay();
  }
  flock(db->fd,LOCK_UN);
  return db;
}

int kvdb_close(struct kvdb *db) {
  while(db->committing==1);
  flock(db->fd,LOCK_EX);
  close(db->fd);
  free(db);
  return 0;
}

char *kvdb_get(struct kvdb *db, const char *key) {
  flock(db->fd,LOCK_EX);
  lseek(db->fd,JSIZE,SEEK_SET);
  keyline *kl;
  while(true){
    kl=malloc(sizeof(keyline));
    read(db->fd,kl,sizeof(keyline));
    if(kl->flag!='!') break;
    if(strcmp(key,kl->key)==0){
      int valuelen=strtol(kl->valuelen,NULL,10);
      int valuepos=strtol(kl->valuepos,NULL,10);
      char *value=malloc(valuelen+1);
      memset(value,'\0',valuelen+1);
      lseek(db->fd,valuepos,SEEK_SET);
      read(db->fd,value,valuelen);
      flock(db->fd,LOCK_UN);
      free(kl);
      return value;
    }
    free(kl);
  }
  free(kl);
  flock(db->fd,LOCK_UN);
  return NULL;
}


//需要在调用者里上锁
int get_valuepos(struct kvdb *db,const char *key,const char *value){
  lseek(db->fd,JSIZE,SEEK_SET);
  keyline *kl;
  while(true){
    kl=malloc(sizeof(keyline));
    read(db->fd,kl,sizeof(keyline));
    if(kl->flag!='!') break;
    if(strcmp(key,kl->key)==0){
      int valuelen=strtol(kl->valuelen,NULL,10);
      if(strlen(value)<=SVALUESIZE || (strlen(value)>SVALUESIZE && valuelen>SVALUESIZE)){
        int valuepos=strtol(kl->valuepos,NULL,10);
        free(kl);
        return valuepos;
      }
      else break;
    }
    free(kl);
  }
  free(kl);
  //到这里表明没有找到相同的key,或者需要重新定位valuepos
  return db->size;
}

int journal_put(struct kvdb *db,const char *key,const char *value){
  flock(db->fd,LOCK_EX);
  lseek(db->fd,0,SEEK_SET);
  write(db->fd,"*",1);
  fsync(db->fd);
  int keylen=strlen(key);
  int valuelen=strlen(value);
  int valuepos=get_valuepos(db,key,value);
  char *kl=malloc(34+keylen+1);
  memset(kl,'\0',35+keylen);
  sprintf(kl,"*%-11d%-11d%-11d%s",keylen,valuelen,valuepos,key);
  lseek(db->fd,0,SEEK_SET);
  write(db->fd,kl,35+keylen);
  free(kl);
  write(db->fd,value,valuelen);
  lseek(db->fd,0,SEEK_SET);
  fsync(db->fd);
  write(db->fd,"!",1);
  fsync(db->fd);
  flock(db->fd,LOCK_UN);
  return 0;
}

char *gen_keyline(int keylen,int valuelen,int valuepos,const char *key){
  char *kl=malloc(34+keylen+1);
  memset(kl,'\0',35+keylen);
  sprintf(kl,"!%-11d%-11d%-11d%s",keylen,valuelen,valuepos,key);
  Log("%s",kl);
  return kl;
}

int kvdb_put(struct kvdb *db, const char *key, const char *value) {
  Log("%s,%s",key,value);
  journal_put(db,key,value);            //this is journal_put! 
  flock(db->fd,LOCK_EX);
  lseek(db->fd,JSIZE,SEEK_SET);
  keyline *kl;
  while(true){
    kl=malloc(sizeof(keyline));
    read(db->fd,kl,sizeof(keyline));
    if(kl->flag!='!') break;
    if(strcmp(key,kl->key)==0){
      int valuelen=strtol(kl->valuelen,NULL,10);
      if(strlen(value)<=SVALUESIZE || (strlen(value)>SVALUESIZE && valuelen>SVALUESIZE)){
        int key_len=strlen(key);
        int value_len=strlen(value);
        int value_pos=strtol(kl->valuepos,NULL,10);
        char *key_line=gen_keyline(key_len,value_len,value_pos,key);
        lseek(db->fd,-sizeof(keyline),SEEK_CUR);
        write(db->fd,key_line,35+key_len);
        free(key_line);
        lseek(db->fd,value_pos,SEEK_SET);
        write(db->fd,value,value_len);
        fsync(db->fd);
        lseek(db->fd,0,SEEK_SET);
        write(db->fd,"*",1);
        fsync(db->fd);
        flock(db->fd,LOCK_UN);
        free(kl);
        return 0;
      }
      else break;
    }
    free(kl);
  }
  free(kl);
  //到这里表明没有找到相同的key,或者需要重新定位valuepos
  int keylen=strlen(key);
  int valuelen=strlen(value);
  int valuepos=db->size;
  char *key_line=gen_keyline(keylen,valuelen,valuepos,key);
  lseek(db->fd,-sizeof(keyline),SEEK_CUR);
  write(db->fd,key_line,35+keylen);
  free(key_line);
  lseek(db->fd,0,SEEK_END);
  if(valuelen<SVALUESIZE){
    write(db->fd,value,valuelen);
    lseek(db->fd,SVALUESIZE-valuelen-1,SEEK_CUR);
    write(db->fd," ",1);
  }
  else if(valuelen==SVALUESIZE || valuelen==LVALUESIZE){
    write(db->fd,value,valuelen);
  }
  else{
    write(db->fd,value,LVALUESIZE);
    lseek(db->fd,LVALUESIZE-valuelen-1,SEEK_CUR);
    write(db->fd," ",1);
  }
  fsync(db->fd);
  lseek(db->fd,0,SEEK_SET);
  write(db->fd,"*",1);
  fsync(db->fd);
  stat(db->filename,&buf);
  db->size=buf.st_size;
  flock(db->fd,LOCK_UN);
  Log("after size:%d",db->size);
  return 0;
}



