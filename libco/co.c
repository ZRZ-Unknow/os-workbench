#include "co.h"
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define STACK_SIZE 4096
enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};
struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
  struct co *next;
  struct co *prev;
};
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