#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>


#define KB *1024
#define MB KB*1024
#define MEM_SIZE (126 MB)
#define PAGE_SIZE (8 KB)
#define HDR_SIZE 128
#define PAGE_NUM MEM_SIZE/PAGE_SIZE
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



/*--------------------utils-------------------------*/
#define list_entry(ptr, type, member) \
  ((type *) \
    ( (char *)(ptr) - (uintptr_t)(&((type *)0)->member) ) \
  )


/*---------------------memory---------------------*/
typedef struct list_head{
  struct list_head *next,*prev;
}list_head;

typedef union page {
  struct {
    spinlock_t lock; // 锁，用于串行化分配和并发的free
    int slab_size;    //如果是0，则表示它不在缓存而在大内存中
    int obj_cnt;     // 页面中已分配的对象数，减少到 0 时回收页面
    int obj_num;     //总对象数
    void *addr;      //首地址
    void *s_mem;     //slab中第一个对象的地址，其地址为: addr+HDR_SIZE; 对象的大小为 slab_size
    uint8_t bitmap[1024];
    list_head list;  // 属于同一个线程的页面的链表
  }; // 匿名结构体
  uint8_t data[PAGE_SIZE];
} __attribute__((packed)) page_t;  //告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐

typedef struct slab_obj{
  bool free;
}slab_obj;

typedef struct A{
    spinlock_t lock; // 锁，用于串行化分配和并发的free
    int slab_size;    //如果是0，则表示它不在缓存而在大内存中,包括了header的大小
    int obj_cnt;     // 页面中已分配的对象数，减少到 0 时回收页面
    int obj_num;
    void *addr;      //首地址
    void *s_mem;     //slab中第一个对象
    uint8_t bitmap[1024];
    list_head list;  // 属于同一个线程的页面的链表
}A; 

typedef struct kmem_cache{
  int cpu;
  int slab_num[3]; //free,full,partial
  list_head free_slab;
  list_head full_slab;
  list_head partial_slab; 
}kmem_cache;

static inline int align_size(int size){
  int ret=1;
  while(ret<size) ret<<=1;
  return ret;
} 