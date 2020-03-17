#include <common.h>
spinlock lk;
static void os_init() {  //必须在这里完成所有必要的初始化
  pmm->init();
  lock_init(&lk,"test");
}

static void os_run() {   //可以随意改动
  while(1){
    lock_acquire(&lk);
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
      _putc(*s == '*' ? '0' + _cpu() : *s);
    }
    lock_release(&lk);
    break;
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