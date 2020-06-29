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
//#define DEBUG
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

struct co *co_main,*co_current;

__attribute__((constructor)) void co_init() {
}

struct co* co_start(const char *name, void (*func)(void *), void *arg){
  struct co *co_new=malloc(sizeof(struct co));
  co_new->name=name;
  co_new->func=func;
  co_new->arg=arg;
  co_new->status=CO_NEW;
  return NULL;
};
void co_yield(){

};
void co_wait(struct co *co){

};