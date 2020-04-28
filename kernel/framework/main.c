#include <kernel.h>
#include <klib.h>

int main() {
  printf("cpu reset\n");
  _ioe_init();
  _cte_init(os->trap);
  os->init();
  _mpe_init(os->run);
  return 1;
}
