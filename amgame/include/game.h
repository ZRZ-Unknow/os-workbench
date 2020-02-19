#include <am.h>
#include <amdev.h>
#include <klib.h>

#define MAX_LEN 100
#define SIDE 16
#define NONE 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

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
