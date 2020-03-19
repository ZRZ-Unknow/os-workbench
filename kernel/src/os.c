#include <common.h>

static void os_init() {  //必须在这里完成所有必要的初始化
  pmm->init();
  srand(uptime());
  lock_init(&lk,"printf_lock");
}

static void os_run() {   //可以随意改动
  //lock_acquire(&lk);
  //lock_release(&lk);
  //while(1){
    /*for(int i=0;i<10;i++){
      size_t size=rand()%128;
      void *ret=pmm->alloc(size);
      while(!ret);
      lock_acquire(&lk);
      printf("cpu %d alloc [%p,%p),size:%d.\n",_cpu(),ret,ret+size,size);
      lock_release(&lk);
    }*/
  //}
  printf("ddd\n");
  while(1);
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