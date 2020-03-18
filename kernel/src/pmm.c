#include <common.h>

typedef union page {
  struct {
    spinlock_t lock; // 锁，用于串行化分配和并发的 free
    int obj_cnt;     // 页面中已分配的对象数，减少到 0 时回收页面
    int slab_size;
    void *addr;      //首地址
    //list_head list;  // 属于同一个线程的页面的链表
    union page *next;
  }; // 匿名结构体
  uint8_t header[HDR_SIZE], data[PAGE_SIZE - HDR_SIZE];
} __attribute__((packed)) page_t;  //告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐

static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {
}
static page_t *head=NULL;
static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);
  head=(page_t *)_heap.start;
  page_t *cp=head;
  for(int i=0;i<10;i++){
    cp->slab_size=rand()%32;
    cp->obj_cnt=rand()%3;
    cp->addr=(void*)cp;
    cp++;
  }
  page_t *p=head;
  for(int i=0;i<10;i++){
    printf("%d,%d,%p,%p,%p\n",p->slab_size,p->obj_cnt,p,p->addr,p->next);
    p++;
  }
  panic("ddd");
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
