#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

typedef intptr_t ssize_t;
typedef intptr_t off_t;
typedef int32_t pid_t;

#ifndef MAX_CPU
 #define MAX_CPU 8
#endif

#define KB *1024
#define MB KB*1024
#define MEM_SIZE (126 MB)
#define PAGE_SIZE (8 KB)
#define HDR_SIZE 256
#define PAGE_NUM MEM_SIZE/PAGE_SIZE  //16128
#define SLAB_TYPE_NUM 10
#define SLAB_LIMIT 16
#define INIT_PAGENUM 10
#define I 0b11111111111111111111111111111111
//126MB内存, 假设内存分配大小的上限是 4 KiB

#define TASK_SIZE (4 KB)
#define MAGIC 0x5a5aa5a5

/*---------------------spinlock-------------------*/
typedef struct spinlock{
  bool locked;
  const char *name;
  int cpu;
}spinlock_t;

void lock_init(spinlock_t *lk,const char *name);
void lock_acquire(spinlock_t *lk);
void lock_release(spinlock_t *lk);
int holding(spinlock_t *lk);
void pushcli(void);
void popcli(void);

/*---------------------pmm---------------------*/
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
    unsigned int bitmap[31];  //共32*31=992个bit，从第一个开始往后obj_num个bit都要用到
  }; // 匿名结构体
  uint8_t data[PAGE_SIZE];
} __attribute__((packed)) page_t;  //告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐

typedef struct kmem_cache{
  int cpu;
  spinlock_t lock;
  int free_num[SLAB_TYPE_NUM]; 
  list_head slab_list[SLAB_TYPE_NUM];
  list_head *freepage[SLAB_TYPE_NUM];
}kmem_cache;

/*----------------------kmt-----------------------*/
#define MAX_HANDLER_NUM 32
#define SLEEP 0
#define RUN 1
#define WAIT 2
#define INI_MIN -9999999
#define INI_MAX  9999999

typedef struct os_single_handler{
  int seq;
  int event;
  handler_t handler;
}os_single_handler;
typedef struct os_handler_array{
  int handler_num;
  os_single_handler os_handler[MAX_HANDLER_NUM];
}os_handler_array;

typedef struct task{
  union{
    struct{
      int pid,cpu,status;
      const char *name;
      void (*entry)(void*);
      void *arg;
      _Context *context;
      list_head list;
      list_head sem_list;
      uint32_t canary;
    };
    uint8_t data[TASK_SIZE];
  };
} __attribute__((packed))task_t;

struct cpu_local_task{
  task_t *current;
  task_t idle_task;
} cpu_task[MAX_CPU];



/*------------------semaphore----------------------*/
typedef struct semaphore{
  const char *name;
  int count;
  spinlock_t lock;
  list_head blocked_task;  //被阻塞的task
}sem_t;

void sem_init(sem_t *sem, const char *name, int value);
void sem_wait(sem_t *sem);
void sem_signal(sem_t *sem);

/*--------------------utils-------------------------*/
#define list_entry(ptr, type, member) \
  ((type *) \
    ( (char *)(ptr) - (uintptr_t)(&((type *)0)->member) ) \
  )
#define get_head_addr(addr) (addr-(((intptr_t)(addr))&(PAGE_SIZE-1)))   
#define setbit(x,pos)  x|=(1<<(pos))     //将x的pos位置为1
#define clrbit(x,pos)  x&=~(1<<(pos))    //将x的pos位置为0
#define getbit(x,pos)   ((x) >> (pos)&1)    //取x的pos位
   //intptr_t位数为平台位数，void在x86为4字节，在x86_64为8字节，而int在两个平台都是4字节




/*-----------------------debug------------------------*/
extern spinlock_t printf_lk;
//#define DEBUG

#ifdef DEBUG
#define Log(format, ...) \
  lock_acquire(&printf_lk); \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  lock_release(&printf_lk); 
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
#ifdef panic_on
# undef panic_on
#endif

#define panic(format, ...) \
  do { \
    Log("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)

#define panic_on(cond,format,...) \
  do{ \
    if(cond) { \
      panic(format); \
    }  \
  }while(0)

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