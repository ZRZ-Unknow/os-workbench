#include <amtest.h>

static _Context *uctx;
static _AddressSpace prot;
static uintptr_t st = 0;
static int first_trap = 1;

void *simple_pgalloc(size_t size) {
  if (st == 0) { st = (uintptr_t)_heap.start; }
  while (st % size != 0) st++;
  void *ret = (void *)st;
  st += size;
  return ret;
}

void simple_pgfree(void *ptr) {
}

_Context* vm_handler(_Event ev, _Context *ctx) {
  switch (ev.event) {
    case _EVENT_YIELD:
      break;
    case _EVENT_IRQ_TIMER:
    case _EVENT_IRQ_IODEV:
      printf("==== interrupt (%s)  ====\n", ev.msg);
      break;
    case _EVENT_PAGEFAULT:
      printf("PF: %x %s%s%s\n",
        ev.ref,
        (ev.cause & _PROT_NONE)  ? "[not present]" : "",
        (ev.cause & _PROT_READ)  ? "[read fail]"   : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]"  : "");
      break;
    case _EVENT_SYSCALL:
      _intr_write(1);
      for (int volatile i = 0; i < 1000000; i++) ;
      printf("%d ", ctx->GPRx);
      break;
    default:
      assert(0);
  }
  if (first_trap) {
    first_trap = 0;
    return uctx;
  } else {
    return ctx;
  }
}

uint8_t code[] = {
#ifdef __ARCH_NATIVE
  0x31, 0xc0,             // xor %eax, %eax
  0x83, 0xc0, 0x01,       // add $1, %eax
  0xff, 0x14, 0x25, 0x00, 0x00, 0x10, 0x00, // call *0x100000
  0xeb, 0xf4,             // jmp 2
#else
  0x31, 0xc0,             // xor %eax, %eax
  0x8d, 0xb6,             // lea 0(%esi), %esi
  0x00, 0x00, 0x00, 0x00,
  0x83, 0xc0, 0x01,       // add $1, %eax
  0xcd, 0x80,             // int $0x80
  0xeb, 0xf9,             // jmp 8
#endif

};

void vm_test() {
  if (!strncmp(__ISA__, "x86", 3) == 0 &&
      !strcmp(__ISA__, "native") == 0) {
    printf("Not supported architecture.\n");
    return;
  }
  _protect(&prot);
  printf("Protected address space: [%p, %p)\n", prot.area.start, prot.area.end);

  uint8_t *ptr = (void*)((uintptr_t)(prot.area.start) +
     ((uintptr_t)(prot.area.end) - (uintptr_t)(prot.area.start)) / 2);

  void *pg = simple_pgalloc(prot.pgsize);
  memcpy(pg, code, sizeof(code));

  _map(&prot, ptr, pg, _PROT_WRITE | _PROT_READ | _PROT_EXEC);
  printf("Code copied to %p (physical %p) execute\n", ptr, pg);

  static uint8_t stack[4096];
  uctx = _ucontext(&prot, RANGE(stack, stack + sizeof(stack)), ptr);

  _intr_write(1);
  _yield();
}
