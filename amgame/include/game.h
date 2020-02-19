#include <am.h>
#include <amdev.h>
#include <klib.h>

void splash();
void print_key();
static inline void puts(const char *s) {
  for (; *s; s++) _putc(*s);
}
