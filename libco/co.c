#include "co.h"
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define KB *1024
#define STACK_SIZE (64 KB)
#define DEBUG

#ifdef DEBUG
#define Log(format, ...) \
    printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define Log(format,...)
#endif

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1" : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1" : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

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
  void *stackptr;
  enum co_status status;  // 协程的状态
  jmp_buf context; // 寄存器现场 (setjmp.h)
  uint8_t stack[STACK_SIZE] __attribute__((aligned(16)));// 协程的堆栈
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
  co_main->stackptr=co_main->stack+sizeof(co_main->stack);
  co_main->next=NULL;
  co_main->prev=NULL;
}
struct co *co_generate(const char *name, void (*func)(void *), void *arg){
  struct co *new_co=malloc(sizeof(struct co));
  new_co->status=CO_NEW;
  strcpy(new_co->name,name);
  memset(new_co->stack,0,sizeof(new_co->stack));
  new_co->stackptr=new_co->stack+sizeof(new_co->stack);
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
  return co_generate(name,func,arg);
}
void co_delete(struct co *thd){
  if (coroutines==NULL) return;
  if(coroutines->next==coroutines && thd==coroutines){
    free(coroutines);
    coroutines=NULL;
    return;
  }
  struct co *next=thd->next;
  struct co *prev=thd->prev;
  if(coroutines==thd) coroutines=next;
  free(thd);
  next->prev=prev;
  prev->next=next;
};
void co_wait(struct co *co) {
  if(co_current==co) assert(0);

  int val=setjmp(co_current->context);
  Log("cur %s,thd %s,val:%d",co_current->name,co->name,val);
  if(val==0){
    while(co->status!=CO_DEAD){
      co_current=co;
      if(co_current->status==CO_NEW){
        co_current->status=CO_RUNNING;
        stack_switch_call(co_current->stackptr,co_current->func,(uintptr_t)co_current->arg);
        Log("a new co %s start to run\n",co_current->name);
        co_current->func(co_current->arg);
      }
      else{
        longjmp(co_current->context,1);
      }
      co_current->status=CO_DEAD;
    }
  }
  Log("cur %s,co %s,delete",co_current->name,co->name);
  co_delete(co);
}

void co_yield(){
  int val=setjmp(co_current->context);
  if(val==0){
    co_current=co_current->next;
    if(co_current->status==CO_NEW){
      co_current->status=CO_RUNNING;
      stack_switch_call(co_current->stackptr,co_current->func,(uintptr_t)co_current->arg);
      Log("a new co %s start to run\n",co_current->name);
      co_current->func(co_current->arg);
      co_current->status=CO_DEAD;
    }
    else{
      longjmp(co_current->context,1);
    }
  }
  Log("yield val!=0");
}

