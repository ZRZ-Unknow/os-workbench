#include <am.h>
#include <amdev.h>
#include <klib.h>

#define MAX_LEN 100

struct Node{
  int x;
  int y;
};
struct Snake{
  int dire;
  int lenth;
  struct Node node[MAX_LEN];
};






void splash();
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
