#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define getbit(x,pos)   ((x) >> (pos)&1) 
char filename[64];
char long_name_buf[64];
char shla[41]="d60e7d3d2b47d19418af5b0ba52406b86ec6ef83";

int long_name_lenth=0;
struct fat_header {
  uint8_t  BS_jmpBoot[3];
  uint8_t  BS_OEMName[8];
  uint16_t BPB_BytsPerSec;  //每扇区字节数
  uint8_t BPB_SecPerClus;    //每簇扇区数
  uint16_t BPB_RsvdSecCnt;   //保留扇区数
  uint8_t BPB_NumFATs;      //fat表个数
  uint16_t BPB_RootEntCnt;   //in FAT32 must be 0
  uint16_t BPB_TotSec16;   //int FAT32 must be 0
  uint8_t BPB_Media;
  uint16_t BPB_FATSz16;     //0
  uint16_t BPB_SecPerTrk;
  uint16_t BPB_NumHeads;
  uint32_t BPB_HiddSec;
  uint32_t BPB_TotSec32;
  uint32_t BPB_FATSz32;   //FAT表扇区数
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;   //FAT根目录起始簇号
  uint16_t BPB_FSInfo;
  uint16_t BPB_BkBootSec;
  uint8_t BPB_Reserved[12];
  uint8_t BS_DrvNum;
  uint8_t BS_Reserved1;
  uint8_t BS_BootSig;
  uint32_t BS_VollD;
  uint8_t BS_VolLab[11];
  uint8_t BS_FilSysType[8];
  uint8_t  padding[420];
  uint16_t Signature_word;
} __attribute__((packed));

struct fat_short_dir{
  uint8_t DIR_Name[11];  //名字+后缀
  uint8_t DIR_Attr;    //属性字节
  uint8_t DIR_NTRes;   //系统保留
  uint8_t DIR_CrtTimeTenth;  
  uint8_t DIR_CrtTime[2];
  uint8_t DIR_CrtDate[2];
  uint8_t DIR_LastAccDate[2];
  uint16_t DIR_FstClusHI;   //文件起始簇号高16位
  uint8_t DIR_WrtTime[2];  
  uint8_t DIR_WrtDate[2];
  uint16_t DIR_FstClusLO;  //文件起始簇号低16位
  uint32_t DIR_FileSize;  //文件长度
}__attribute__((packed));

struct fat_long_dir{
  uint8_t LDIR_Ord;  //属性字节
  uint16_t LDIR_Name1[5];
  uint8_t LDIR_Attr;  //目录项标志,0FH
  uint8_t LDIR_Type;  //系统保留
  uint8_t LDIR_Chksum;
  uint16_t LDIR_Name2[6];
  uint8_t LDIR_FstClusLO[2];  //0
  uint16_t LDIR_Name3[2];
}__attribute__((packed));

struct DIR{
  uint8_t data[32];
}__attribute__((packed));

void recover(){
  int fd=open(filename,O_RDONLY);
  assert(fd!=-1);
  struct stat buf;
  fstat(fd,&buf);
  void *fat_fs=mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
  assert(fat_fs!=MAP_FAILED);
  struct fat_header *header=fat_fs;
  assert(header->Signature_word==0xaa55);
  void *data_begin=(void*)(intptr_t)((header->BPB_RsvdSecCnt+header->BPB_NumFATs*header->BPB_FATSz32+(header->BPB_RootClus-2)*header->BPB_SecPerClus)*header->BPB_BytsPerSec);
  printf("start:%p,data_begin:%p,size:%ld\n",fat_fs,data_begin,buf.st_size);
  struct DIR *dir=(void*)((intptr_t)fat_fs+(intptr_t)data_begin);
  while((uintptr_t)dir++<(uintptr_t)(fat_fs+buf.st_size)){
    if(dir->data[0]==0x00 || dir->data[0]==0xE5 || dir->data[11]==0x0F){
      continue;
    }
    if(dir->data[8]=='B' && dir->data[9]=='M' && dir->data[10]=='P'){
      if(dir->data[6]=='~'){    //是长文件的短文件名目录,需要倒推
        memset(long_name_buf,0,sizeof(long_name_buf));
        long_name_lenth=0; 
        struct fat_long_dir *long_dir=(struct fat_long_dir*)(dir-1);
        while(long_dir->LDIR_Attr==0x0F){
          bool reach_end=false;
          for(int i=0;i<5;i++){
            if(long_dir->LDIR_Name1[i]==0x00){
              reach_end=true;
              break;
            }
            long_name_buf[long_name_lenth++]=long_dir->LDIR_Name1[i];
          } 
          if(!reach_end){
            for(int i=0;i<6;i++){
              if(long_dir->LDIR_Name2[i]==0xFFFF){
                reach_end=true;
                break;
              }
              long_name_buf[long_name_lenth++]=long_dir->LDIR_Name2[i];
            }
          }
          if(!reach_end){
            for(int i=0;i<2;i++){
              if(long_dir->LDIR_Name3[i]==0xFFFF){
                reach_end=true;
                break;
              }
              long_name_buf[long_name_lenth++]=long_dir->LDIR_Name3[i];
            }
          }
          if(getbit(long_dir->LDIR_Ord,6)==1){  //长文件名的最后一个目录项
            printf("%s %s\n",shla,long_name_buf);
            break;
          }
          long_dir--;
        }
      }
      else{
        char *short_name=malloc(16);
        memset(short_name,0,16);
        for(int i=0;i<8;i++){
          if(dir->data[i]==0x20) break;
          short_name[i]=dir->data[i];
        }
        strcat(short_name,".bmp");
        printf("%s %s\n",shla,short_name);
      }
      fflush(stdout);
    }
  }
  printf("done\n");
}


int main(int argc, char *argv[]) {
  /**assert(argc>=3);
  assert(strcmp(argv[1],"frecov")==0);
  assert(sizeof(struct fat_header)==512);**/
  sprintf(filename,"%s","/home/zrz/temp/M5-frecov.img");
  memset(long_name_buf,0,sizeof(long_name_buf));
  recover();
  return 0;
}
