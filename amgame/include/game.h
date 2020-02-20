#include <am.h>
#include <amdev.h>
#include <klib.h>

#ifndef bool
#define bool int8_t
#define false 0
#define true 1
#endif

#define MAX_LEN 6
#define SIDE 16
#define NONE 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

#define BLACK 0x000000
#define WHITE 0xffffff
#define RED 0xc73e3a
#define CHOCOLATE 0xd2691e
#define PURPLE 0x800080
#define GREEN 0x869900


struct Node{
  int x;
  int y;
};
struct Snake{
  int dire;
  int lenth;
  struct Node node[MAX_LEN];
};

void draw_snake();
void init();
void draw_food();
void splash();
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
#define Log(format, ...) \
  printf("\33[0m[\33[1;35mLog\33[0m]\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
    __FILE__, __LINE__, __func__, ## __VA_ARGS__)

