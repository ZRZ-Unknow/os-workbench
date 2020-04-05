#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#define KB *1024
#define MB KB*1024
#define MEM_SIZE (126 MB)
#define PAGE_SIZE (8 KB)
#define HDR_SIZE 256
#define PAGE_NUM MEM_SIZE/PAGE_SIZE  //16128
#define SLAB_TYPE_NUM 10
#define SLAB_LIMIT 16
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
typedef struct list_head{
  struct list_head *next,*prev;
}list_head;

typedef union page {
  struct {
    spinlock_t lock; // 锁，用于串行化分配和并发的free
    int cpu;          //位于哪个cpu中
    int slab_size;    //如果是0，则表示它不在缓存而在大内存中
    int obj_cnt;     // 页面中已分配的对象数，减少到 0 时回收页面
    int obj_num;     //总对象数
    void *addr;      //首地址
    void *s_mem;     //slab中第一个对象的地址，其地址为: addr+(80+obj_num); 对象的大小为 slab_size
    list_head list;  // 属于同一个线程的页面的链表
    //uint8_t bitmap[512];  //往后obj_num个字节都属于bitmap;
    int bitmap[31];  //共32*31=992个bit，从第一个开始往后obj_num个bit都要用到
  }; // 匿名结构体
  uint8_t data[PAGE_SIZE];
} __attribute__((packed)) page_t;  //告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐

typedef struct kmem_cache{
  int cpu;
  spinlock_t lock;
  int free_num[SLAB_TYPE_NUM]; 
  list_head slab_list[SLAB_TYPE_NUM];
}kmem_cache;

/*--------------------pmm---------------------------*/

void debug_print();
void debug_slab_print(page_t *page);
void *get_free_obj(page_t* page);
page_t *get_free_page(int num,int slab_size,int cpu);

/*--------------------utils-------------------------*/
#define list_entry(ptr, type, member) \
  ((type *) \
    ( (char *)(ptr) - (uintptr_t)(&((type *)0)->member) ) \
  )
#define get_head_addr(addr) (addr-(((intptr_t)(addr))&(PAGE_SIZE-1)))   
#define setbit(x,pos)  x|=(1<<pos)     //将x的pos位置为1
#define clrbit(x,pos)  x&=~(1<<pos)    //将x的pos位置为0
#define getbit(x,pos)   ((x) >> (pos)&1)    //取x的pos位
   //intptr_t位数为平台位数，void在x86为4字节，在x86_64为8字节，而int在两个平台都是4字节

static inline int get_obj_pos(void *addr){
  page_t *page=get_head_addr(addr);
  int pos=(addr-page->s_mem)/page->slab_size;
  return pos;
}

static inline size_t align_size(size_t size){
  size_t ret=1;
  while(ret<size) ret<<=1;
  return ret;
}

/*-----------------------debug------------------------*/
extern spinlock_t lk;
//#define DEBUG

#ifdef DEBUG
#define Log(format, ...) \
  lock_acquire(&lk); \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  lock_release(&lk); 
#else
#define Log(format,...)
#endif

#define SLog(format,...) \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#define Spanic(format,...) \
  do { \
    SLog("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)


#ifdef assert
# undef assert
#endif
#ifdef panic
# undef panic
#endif

#define panic(format, ...) \
  do { \
    Log("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)

#define assert(cond) \
  do { \
    if (!(cond)) { \
      panic("Assertion failed: %s", #cond); \
    } \
  } while (0)

#define Assert(cond,format,...) \
  do{ \
    if(!(cond)) { \
      panic(format); \
    }  \
  }while(0)

#define TODO() panic("please implement me")