#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>


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




#define DEBUG
#ifdef DEBUG
#define Log(format, ...) \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#ifdef assert
# undef assert
#endif
#ifdef panic
# undef panic
#endif
#define panic(format, ...) \
  do { \
    Log("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    _halt(1); \
  } while (0)

#define assert(cond) \
  do { \
    if (!(cond)) { \
      panic("Assertion failed: %s", #cond); \
    } \
  } while (0)

#define TODO() panic("please implement me")

#endif