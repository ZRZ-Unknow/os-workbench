#include <common.h>


static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {
}
static page_t *head=NULL;
static list_head list_h;
static list_head *list=&list_h;

#define list_entry(ptr, type, member) \
  ((type *) \
    ( (char *)(ptr) - (uintptr_t)(&((type *)0)->member) ) \
  )

// ptr to struct list_head
//struct task *task = list_entry(ptr, struct task, wait_queue);

static void pmm_init() {
  uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);
  head=(page_t *)_heap.start;
  page_t *cp=head;
  list->next=&head->list;
  head->list.prev=list;
  for(int i=0;i<10;i++){
    cp->slab_size=rand()%32;
    cp->obj_cnt=rand()%3;
    cp->addr=(void*)cp;
    page_t *tmp=cp;
    cp++;
    tmp->list.next=&cp->list;
    cp->list.prev=&tmp->list;
  }
  page_t *p=head;
  for(int i=0;i<10;i++){    
    //p本身指向page的首地址，p->addr也是；p->list是page中member list的首地址，p->list.prev指向上一个page的list的首地址
    printf("%d,%d,%p,%p,%p,%p,%p\n",p->slab_size,p->obj_cnt,p,p->addr,&p->list,p->list.prev,p->list.next);
    page_t *task=list_entry(&p->list,page_t,list);
    printf("%p\n",task);
    p++;
  }
  panic("%d",PAGE_NUM);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
