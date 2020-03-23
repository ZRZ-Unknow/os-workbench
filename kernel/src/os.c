#include <common.h>

spinlock_t lk;

spinlock_t test_lk;
int count=0;
int cnt[8]={0,0,0,0,0,0,0,0};
void *ptr[80000];
int _size[20];
int j=0;
static void os_init() {  //必须在这里完成所有必要的初始化
  srand(uptime());
  lock_init(&lk,"printf_lock");
  lock_init(&test_lk,"test_lk");
  pmm->init();
}

static void os_run() {   //可以随意改动
  while(1){
    for(int i=0;i<10000;i++){
      size_t size=rand()%64;
      void *ret=pmm->alloc(size);
      int cpu=_cpu();
      ptr[i+cpu*10000]=ret;
      /*cnt[_cpu()]++;
      lock_acquire(&test_lk);
      ptr[count]=ret;
      _size[count++]=size;
      lock_release(&test_lk);
      assert(ret);*/
      lock_acquire(&lk);
      printf("cpu %d alloc [%p,%p),size:%d,cnt:%d,all_count:%d\n",_cpu(),ret,ret+size,size);
      lock_release(&lk);
    }  /*if(rand()%3==0){
        lock_acquire(&test_lk);
        int jj=j;
        int cc=count;
        lock_release(&test_lk);
        if(jj>=cc) {
          continue;
        }
        lock_acquire(&lk);
        printf("cpu %d free [%p,%p),size:%d,j:%d\n",_cpu(),ptr[jj],ptr[jj]+_size[jj],_size[jj],jj);
        lock_release(&lk);
;
        lock_acquire(&test_lk);
        j++;
        pmm->free(ptr[jj]);
        lock_release(&test_lk);
      }*/
    for(int j=0;j<10000;j++){
      int cpu=_cpu();
      int n=_ncpu();
      pmm->free(ptr[j+(n-cpu-1)*10000]);
      lock_acquire(&lk);
      printf("cpu %d free [%p,?)\n",cpu,ptr[j+(n-cpu-1)*10000]);
      lock_release(&lk);
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