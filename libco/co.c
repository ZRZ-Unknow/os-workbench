#include "co.h"
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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
#if defined(__i386__)
  #define SP "%%esp"
#elif defined(__x86_64__)
  #define SP "%%rsp"
#endif
#define PUSH(sp) \
  asm volatile("mov " SP ", %0": "=g"(sp)); 
#define PULL(newsp) \
  asm volatile("mov %0, " SP : "+g"(newsp));
#define PU(sp,newsp)\
  asm volatile("mov " SP ", %0": "=g"(sp)); \
  asm volatile("mov %0, " SP : "+g"(newsp));

static int id=1;
enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};
struct co {
  int id;
  const char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;
  void *stackptr;
  enum co_status status;  // 协程的状态
  jmp_buf context; // 寄存器现场 (setjmp.h)
  struct co *next;
  struct co *prev;
  struct co *waiter;
  uint8_t stack[STACK_SIZE] __attribute__ ((aligned(16)));// 协程的堆栈
};

static struct co *co_current=NULL;
static struct co *coroutines=NULL;
static struct co *co_main=NULL;

void wrapper(struct co* co) {
  co->status = CO_RUNNING;
  co->func(co->arg);
  co->status = CO_DEAD;
  if (co->waiter) co->waiter->status = CO_RUNNING;
  co_yield();
}

__attribute__((constructor)) void co_init() {
  co_main=malloc(sizeof(struct co));
  co_main->id=0;
  co_main->name="main";
  co_main->status=CO_RUNNING;
  co_main->arg=NULL;
  memset(co_main->stack,0,sizeof(co_main->stack));
  co_main->stackptr=co_main->stack+sizeof(co_main->stack);
  co_main->next=co_main;
  co_main->prev=co_main;
  co_current=co_main;
  coroutines=co_main;
  srand(time(NULL));
}
struct co *co_generate(const char *name, void (*func)(void *), void *arg){
  struct co *new_co=malloc(sizeof(struct co));
  new_co->status=CO_NEW;
  new_co->id=id++;
  new_co->name=name;
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
  Log("create co %d,%s",id,name);
  return co_generate(name,func,arg);
}
void co_delete(struct co *thd){
  if (coroutines==NULL) return;
  struct co *next=thd->next;
  struct co *prev=thd->prev;
  if(coroutines==thd) coroutines=next;
  free(thd);
  id--;
  assert(next&&prev);
  next->prev=prev;
  prev->next=next;
};
void co_yield(){
  int val=setjmp(co_current->context);
  if(val==0){
    int r=rand()%(id);
    struct co *prev=co_current;
    co_current=coroutines;
    while(r>0 || co_current->status==CO_DEAD || co_current->status==CO_WAITING){
      co_current=co_current->next;
      r--;
    }
    if(co_current->status==CO_NEW){
      Log("a new co %s %d start to run",co_current->name,co_current->id);
      stack_switch_call(co_current->stackptr,wrapper,(uintptr_t)NULL);
    }
    else{
      Log("longjmp to %d %s from yield",co_current->id,co_current->name);
      longjmp(co_current->context,1); 
    }
  }
  Log("yield finish");
}
void co_wait(struct co *co){
  Log("co wait for %s %d",co->name,co->id);
  if(co->status==CO_DEAD){
    co_delete(co);
    return;
  }
  co->waiter=co_current;
  co_current->status=CO_WAITING;
  co_yield();
  co_delete(co);
}

















/*__attribute__((constructor)) void co_init() {
  co_main=malloc(sizeof(struct co));
  co_main->id=0;
  co_main->name="main";
  co_main->status=CO_RUNNING;
  co_main->arg=NULL;
  memset(co_main->stack,0,sizeof(co_main->stack));
  co_main->stackptr=co_main->stack+sizeof(co_main->stack);
  co_main->next=NULL;
  co_main->prev=NULL;
  co_current=co_main;
}

struct co *co_generate(const char *name, void (*func)(void *), void *arg){
  struct co *new_co=malloc(sizeof(struct co));
  new_co->status=CO_NEW;
  new_co->id=id++;
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
  Log("create co %d,%s",id,name);
  return co_generate(name,func,arg);
}
void co_delete(struct co *thd){
  if (coroutines==NULL) return;
  if(coroutines->next==coroutines && thd==coroutines){
    free(coroutines);
    coroutines=NULL;
    id--;
    return;
  }
  struct co *next=thd->next;
  struct co *prev=thd->prev;
  if(coroutines==thd) coroutines=next;
  free(thd);
  id--;
  next->prev=prev;
  prev->next=next;
};
void co_wait(struct co *co) {
  Log("start wait");
  int val=setjmp(co_current->context);
  Log("cur %d start wait for thd %d,val:%d",co_current->id,co->id,val);
  if(val==0){
      co_current=co;
      if(co_current->status==CO_NEW){
        co_current->status=CO_RUNNING;
        Log("a new co %d start to run",co_current->id);
        PU(co_main->stackptr,co_current->stackptr);
        co_current->func(co_current->arg);
        co_current->status=CO_DEAD;
        longjmp(co_main->context,1);
      }
      else{
        Log("long jmp to %d from wait",co_current->id);
        longjmp(co_current->context,1);
      }
  }
  Log("cur %d,co %d,delete",co_current->id,co->id);
  co_current=co_main;
  co_delete(co);
}

void co_yield(){
  if(coroutines==NULL) return;
  int val=setjmp(co_current->context);
  if(val==0){
    if(co_current==co_main){
      if(coroutines->status==CO_NEW){
        coroutines->status=CO_RUNNING;
        Log("a new co %d start to run",coroutines->id);
        PU(co_current->stackptr,coroutines->stackptr);
        co_current=coroutines;
        co_current->func(co_current->arg);
        co_current->status=CO_DEAD;
        longjmp(co_main->context,1);
      }
      else{
        co_current=coroutines;
        Log("longjmp to %d from yield",co_current->id);
        longjmp(co_current->context,1);
      }
    }
    else{
      if(co_current->next->status==CO_NEW){
        co_current->next->status=CO_RUNNING;
        Log("a new co %d start to run",co_current->next->id);
        PU(co_current->stackptr,co_current->next->stackptr);
        Log("%d,%d",co_current->id,co_current->next->id);
        co_current=co_current->next;
        co_current->func(co_current->arg);
        co_current->status=CO_DEAD;
        longjmp(co_main->context,1);
      }
      else{
        co_current=co_current->next;
        Log("longjmp to %d from yield",co_current->id);
        longjmp(co_current->context,1);
      }
    }
  }
  Log("yield finish");
}*/

