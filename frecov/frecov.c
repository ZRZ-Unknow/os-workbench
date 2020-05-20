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


char filename[128];
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
  uint32_t BPB_FATSz32;
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;
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

struct fat {
  struct fat_header header;
  //uint8_t padding[PADDING_SIZE];
  //struct fat_table fat[FAT_COPIES];
  //struct cluster clusters[CLUSTER_SIZE];
} __attribute__((packed)); 

void recover(){
  int fd=open(filename,O_RDONLY);
  assert(fd!=-1);
  struct stat buf;
  fstat(fd,&buf);
  void *fat_fs=mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);
  assert(fat_fs!=MAP_FAILED);
  struct fat_header *header=file;
  printf("%x\n",header->BS_jmpBoot[0]);
  assert(header->Signature_word==0xaa55);
  printf("%d\n",header->BPB_RsvdSecCnt);
  printf("%d\n",header->BPB_NumFATs);
  void *data_begin=(header->BPB_RsvdSecCnt+header->BPB_NumFATs*header->BPB_FATSz32+(header->BPB_RootClus-2)*header->BPB_SecPerClus)*header->BPB_BytsPerSec;
  printf("%p\n",data_begin);
}


int main(int argc, char *argv[]) {
  assert(argc>=3);
  assert(strcmp(argv[1],"frecov")==0);
  assert(sizeof(struct fat_header)==512);
  sprintf(filename,"%s",argv[2]);
  printf("%s\n",filename);
  recover();
  return 0;
}
