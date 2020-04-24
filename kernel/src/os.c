#include <common.h>

spinlock_t printf_lk;
spinlock_t os_trap_lk;
static os_handler_array os_handlers={.handler_num=0};

//#define TEST_MEM
#ifdef TEST_MEM
extern int SLAB_SIZE[SLAB_TYPE_NUM];
void *ptr[800000];
int N=300000;

struct workload {
  int prob[SLAB_TYPE_NUM], sum; // sum = prob[0] + prob[1] + ... prob[N-1]
};

struct workload
  wl_typical = {.prob = {10,10,20,40,50,15,5,2,1,1},.sum=0 },
  wl_stress  = {.prob = {0,0,0,300,400,200,100,2,1,1},.sum=0 },
  wl_page    = {.prob = {0,0,0,0,20,10,20,80,100,80},.sum=0 }
;
static struct workload *workload = &wl_typical;

static void mem_test(){
  int begin=uptime();
    for(int i=0;i<N;i++){
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
      ptr[i+cpu*N]=ret;
      lock_acquire(&printf_lk);
      printf("cpu %d alloc [%p,%p),size:%d\n",_cpu(),ret,ret+size,size);
      lock_release(&printf_lk);
    }
    for(int j=0;j<N;j++){
      int cpu=_cpu();
      int n=_ncpu();
      pmm->free(ptr[j+(n-cpu-1)*N]);
      lock_acquire(&printf_lk);
      printf("cpu %d free [%p,?)\n",cpu,ptr[j+(n-cpu-1)*N]);
      lock_release(&printf_lk);
    }
    int end=uptime();
    printf("time:%d\n",end-begin);
    assert(0);
}
#endif

void producer(void *arg) { while (1) {  _putc('(');  } }
void consumer(void *arg) { while (1) {  _putc(')');  } }
void func(void *arg){
  while(1){
    lock_acquire(&printf_lk);
    printf("hello frome thread %s,cpu:%d\n",arg,_cpu());
    lock_release(&printf_lk);
    for (int volatile i = 0; i < 100000; i++) ; 
  }
}
static void os_init() {  //必须在这里完成所有必要的初始化
  lock_init(&printf_lk,"printf_lock");
  lock_init(&os_trap_lk,"os_trap_lk");
  pmm->init();
  kmt->init();
  kmt->create(pmm->alloc(sizeof(task_t)),"A",func,"A"); 
  kmt->create(pmm->alloc(sizeof(task_t)),"B",func,"B");
  kmt->create(pmm->alloc(sizeof(task_t)),"C",func,"C");
  kmt->create(pmm->alloc(sizeof(task_t)),"D",func,"D");
  #ifdef TEST_MEM
  srand(uptime());
  for(int i=0;i<SLAB_TYPE_NUM;i++) 
    workload->sum+=workload->prob[i];
  #endif
}
     
extern void kmt_task_print();
static void os_run() {   //可以随意改动
  lock_acquire(&printf_lk);
  printf("Hello from cpu%d\n",_cpu());
  lock_release(&printf_lk);
  _intr_write(1);
  while(1){
    #ifdef TEST_MEM
    mem_test();
    #endif
    //kmt_task_print();
    //_yield();
  }
}

/*类似与thread-os-mp.c中的on_interrupt，每次中断后，AM会保存现场，然后调用os_trap（可以进行进程切换等）进行中断处理，os_trap
  返回后，AM会恢复现场*/
static _Context *os_trap(_Event ev,_Context *context){
  _Context *next=NULL;
  lock_acquire(&os_trap_lk);
  for(int i=0;i<os_handlers.handler_num;i++){
    if(os_handlers.os_handler[i].event==_EVENT_NULL || os_handlers.os_handler[i].event==ev.event){
      _Context *r=os_handlers.os_handler[i].handler(ev,context);
      panic_on(r&&next , "returning multiple contexts");
      if(r) next=r;
    }
  }
  lock_release(&os_trap_lk);
  panic_on(!next,"returning NULL context");
  return next;
}

static void os_on_irq(int seq,int event, handler_t handler){
  //insert a handler into handler_array whose seq form small to big
  os_handlers.os_handler[os_handlers.handler_num].seq=seq;
  os_handlers.os_handler[os_handlers.handler_num].event=event;
  os_handlers.os_handler[os_handlers.handler_num].handler=handler;
  os_handlers.handler_num++;
  bool flag=true; 
  for(int i=0;i<os_handlers.handler_num;i++){
    if(flag==false) break; 
    flag=false;
    for(int j=i+1;j<os_handlers.handler_num;j++){
      if(os_handlers.os_handler[i].seq>os_handlers.os_handler[j].seq){
        os_single_handler tmp=os_handlers.os_handler[i];
        os_handlers.os_handler[i]=os_handlers.os_handler[j];
        os_handlers.os_handler[j]=tmp;
        flag=true;
      }
    }
  }
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
/*
extern mod_os_t __os_obj;  //extern表示定义在别的文件或模块中
mod_os_t *os = &__os_obj;
mod_os_t __os_obj = {
  .init = os_init,
  .run = os_run,
};
*/