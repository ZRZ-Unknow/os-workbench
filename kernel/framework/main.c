#include <kernel.h>
#include <klib.h>

int main() {
  os->init();
  _mpe_init(os->run);
  return 1;
}
