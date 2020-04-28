#include <common.h>

spinlock_t printf_lk;
spinlock_t os_trap_lk;
static os_handler_array os_handlers={.handler_num=0};

#define TEST_KMT
#ifdef TEST_KMT
sem_t empty,fill;
void producer(void *arg){
  while(1){
    kmt->sem_wait(&empty);
    for(int volatile i = 0; i < 100000; i++) ; 
    _putc('(');  
    /*lock_acquire(&printf_lk);
    printf("\33[1;32mhello producer\33[0m\n");
    lock_release(&printf_lk);*/
    kmt->sem_signal(&fill);
  } 
}
void consumer(void *arg){
  while(1){
    kmt->sem_wait(&fill);
    _putc(')');
    for (int volatile i = 0; i < 100000; i++) ; 
    /*lock_acquire(&printf_lk);
    printf("\33[1;33mhello consumer\33[0m\n");
    lock_release(&printf_lk);*/
    kmt->sem_signal(&empty);
  } 
}
void func(void *arg){
  bool flag=false;
  while(1){
    kmt->spin_lock(&printf_lk);
    printf("hello from thread %s,cpu:%d\n",arg,_cpu());
    kmt->spin_unlock(&printf_lk);
    _yield();
    for (int volatile i = 0; i < 100000; i++) ; 
    if(flag==false){
      for(int i=0;i<4;i++){
        kmt->create(pmm->alloc(sizeof(task_t)),"producer",producer,NULL); 
      }
      for(int i=0;i<5;i++){
        kmt->create(pmm->alloc(sizeof(task_t)),"consumer",consumer,NULL);
      }
      flag=true;
    }
  }
}
void yielder()  { while (1) _yield(); }
#endif
const char *name[]={"A","B","C","D","E","F","G","H","I","J"};
extern void kmt_task_print();
static void os_init() {  //必须在这里完成所有必要的初始化
  lock_init(&printf_lk,"printf_lk");
  lock_init(&os_trap_lk,"os_trap_lk");
  pmm->init();
  kmt->init();

  #ifdef TEST_KMT
  srand(uptime());
  kmt->sem_init(&empty,"empty",5);
  kmt->sem_init(&fill,"fill",0);
  for(int i=0;i<4;i++){
    kmt->create(pmm->alloc(sizeof(task_t)),"producer",producer,NULL); 
  }
  for(int i=0;i<5;i++){
    kmt->create(pmm->alloc(sizeof(task_t)),"consumer",consumer,NULL);
  }
  for(int i=0;i<10;i++){
    kmt->create(pmm->alloc(sizeof(task_t)),name[i%10],func,(void*)name[i%10]);
  }
  for(int i=0;i<30;i++){
    kmt->create(pmm->alloc(sizeof(task_t)),"yield",yielder,NULL);
  }
  kmt_task_print();
  #endif
}
     
static void os_run() {   //可以随意改动
  _intr_write(1);
  while(1){
    _yield();
  };
}

/*类似与thread-os-mp.c中的on_interrupt，每次中断后，AM会保存现场，然后调用os_trap进行中断处理，os_trap
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