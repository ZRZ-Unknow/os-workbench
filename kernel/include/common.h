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
  int _cpu;
}spinlock;

