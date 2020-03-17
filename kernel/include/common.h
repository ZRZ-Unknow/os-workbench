#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>
#include <debug.h>


#define B *1
#define KB B*1024
#define MB KB*1024
#define GB MB*1204

typedef struct spinlock{
  bool locked;
  //For debugging
  char *name;
  int cpu;
}spinlock;

void lock_init(spinlock *lk,char *name);
void lock_acquire(spinlock *lk);
void lock_release(spinlock *lk);
int holding(spinlock *lk);
void pushcli(void);
void popcli(void);