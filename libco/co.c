#include "co.h"

static struct co *co_current=NULL;
static struct co *coroutines=NULL;
static struct co *co_main=NULL;

__attribute__((constructor)) void co_init() {
  co_main=malloc(sizeof(struct co));
  co_main->status=CO_NEW;
  memset(co_main->stack,0,sizeof(co_main->stack));
  co_main->next=NULL;
  co_main->prev=NULL;
}
struct co *co_generate(const char *name, void (*func)(void *), void *arg){
  struct co *new_co=malloc(sizeof(struct co));
  new_co->status=CO_NEW;
  strcpy(new_co->name,name);
  new_co->func=func;
  new_co->arg=arg;
  if(coroutines==NULL){
    coroutines=new_co;
    coroutines->next=coroutines;
    coroutines->prev=coroutines;
  }
  else{
    struct co* prev=coroutines->prev;
    struct co* next=coroutines;
    new_co->next=next;
    next->prev=new_co;
    new_co->prev=prev;
    prev->next=new_co;
  }
  return new_co; 
}
struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co_current=co_generate(name,func,arg);
  
  return NULL;
}

void co_wait(struct co *co) {
}

void co_yield() {
}


static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1" : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1" : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}