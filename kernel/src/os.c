#include <common.h>

spinlock_t lk;
int count=0;
int cnt[8]={0,0,0,0,0,0,0,0};
void *ptr[1<<20];
int _size[1<<20];
int j=0;
static void os_init() {  //必须在这里完成所有必要的初始化
  srand(uptime());
  lock_init(&lk,"printf_lock");
  pmm->init();
}

static void os_run() {   //可以随意改动
  while(1){
      size_t size=rand()%280;
      void *ret=pmm->alloc(size);
      cnt[_cpu()]++;
      ptr[count]=ret;
      _size[count++]=size;
      assert(ret);
      lock_acquire(&lk);
      printf("cpu %d alloc [%p,%p),size:%d,cnt:%d,all_count:%d\n",_cpu(),ret,ret+size,size,cnt[_cpu()],count);
      lock_release(&lk);
      if(rand()%2==0){
        if(j>=count) continue;
        pmm->free(ptr[j]);
        lock_acquire(&lk);
        printf("cpu %d free [%p,%p),size:%d,j:%d\n",_cpu(),ptr[j],ptr[j]+_size[j],_size[j],j);
        lock_release(&lk);
        j++;
      }
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