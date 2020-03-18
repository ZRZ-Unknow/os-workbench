#include <common.h>


static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {
}
static page_t *head=NULL;
static list_head list_h;
static list_head *list=&list_h;

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
    cp+=1;
  }
  page_t *p=head;
  for(int i=0;i<10;i++){
    printf("%d,%d,%p,%p,%p\n",p->slab_size,p->obj_cnt,p,p->addr,p->list.next);
    p++;
  }
  panic("ddd");
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
