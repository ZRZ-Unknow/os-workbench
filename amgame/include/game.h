#include <am.h>
#include <amdev.h>
#include <klib.h>

#ifndef bool
#define bool int8_t
#define false 0
#define true 1
#endif

#define MAX_LEN 100
#define SIDE 16
#define NONE 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

#define BLACK 0x000000
#define WHITE 0xffffff
#define RED 0xc73e3a
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
void draw_snake();
void _draw(int x, int y, int w, int h, uint32_t color);
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
