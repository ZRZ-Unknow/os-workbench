#include <common.h>

spinlock_t lk;
spinlock_t test_lk;
extern int SLAB_SIZE[SLAB_TYPE_NUM];
void *ptr[80000];

struct workload {
  int prob[SLAB_TYPE_NUM], sum; // sum = prob[0] + prob[1] + ... prob[N-1]
};

struct workload
  wl_typical = {.prob = {10,20,40,50,15,5,2,1},.sum=0 },
  wl_stress  = {.prob = {0,0,0,400,200,100,2,1},.sum=0 },
  wl_page    = {.prob = {0,0,0,0,10,20,80,100},.sum=0 }
;
static struct workload *workload = &wl_page;

static void os_init() {  //必须在这里完成所有必要的初始化
  srand(uptime());
  lock_init(&lk,"printf_lock");
  lock_init(&test_lk,"test_lk");
  pmm->init();
  for(int i=0;i<SLAB_TYPE_NUM;i++) 
    workload->sum+=workload->prob[i];
}
     

static void os_run() {   //可以随意改动
  while(1){
    for(int i=0;i<10000;i++){
      int choice=rand()%workload->sum;
      int n=0,sum=0;
      for(int j=0;j<SLAB_TYPE_NUM;j++){
        if(choice>=sum && choice<sum+workload->prob[j]){
          n=j;
          break;
        }
        sum+=workload->prob[j];
      }
      size_t size= (n==0) ? rand()%SLAB_SIZE[n] : (SLAB_SIZE[n-1]+rand()%(SLAB_SIZE[n]-SLAB_SIZE[n-1]));
      void *ret=pmm->alloc(size);
      int cpu=_cpu();
      ptr[i+cpu*10000]=ret;
      lock_acquire(&lk);
      printf("cpu %d alloc [%p,%p),size:%d\n",_cpu(),ret,ret+size,size);
      lock_release(&lk);
    }
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