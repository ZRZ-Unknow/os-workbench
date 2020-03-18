#include <common.h>
spinlock_t lk;
static void os_init() {  //必须在这里完成所有必要的初始化
  pmm->init();
  lock_init(&lk,"test");
}
struct a{
  spinlock_t lk;
  int a;
};

page_t page1;
page_t *page=&page1;


static void os_run() {   //可以随意改动
  page->lock.locked=0;
page->obj_cnt=1;
page->addr=_heap.start;
//page->list->next=page->list;
//page->list->prev=page->list;
  while(1){
    lock_acquire(&lk);
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
      _putc(*s == '*' ? '0' + _cpu() : *s);
    }
    printf("%d,%d,%p",page->lock.locked,page->obj_cnt,page->addr);
    //printf("%d,%d,%d\n",sizeof(struct a),sizeof(spinlock_t),sizeof(int));
    lock_release(&lk);
  }
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
/*
extern mod_os_t __os_obj;  //extern表示定义在别的文件或模块中
mod_os_t *os = &__os_obj;
mod_os_t __os_obj = {
  .init = os_init,
  .run = os_run,
};
*/