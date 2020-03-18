#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>


#define KB *1024
#define MB KB*1024
#define PAGE_SIZE (8 KB)
#define HDR_SIZE 64
//126MB内存, 假设内存分配大小的上限是 4 KiB,

/*---------------------spinlock-------------------*/
typedef struct spinlock{
  bool locked;
  char *name;
  int cpu;
}spinlock_t;

void lock_init(spinlock_t *lk,char *name);
void lock_acquire(spinlock_t *lk);
void lock_release(spinlock_t *lk);
int holding(spinlock_t *lk);
void pushcli(void);
void popcli(void);


/*---------------------memory---------------------*/
typedef union page {
  struct {
    spinlock_t lock; // 锁，用于串行化分配和并发的 free
    int obj_cnt;     // 页面中已分配的对象数，减少到 0 时回收页面
    //list_head list;  // 属于同一个线程的页面的链表
  }; // 匿名结构体
  uint8_t header[HDR_SIZE], data[PAGE_SIZE - HDR_SIZE];
} __attribute__((packed)) page_t;