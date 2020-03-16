#include <am.h>

#define MODULE(mod) \
  typedef struct mod_##mod##_t mod_##mod##_t; \
  extern mod_##mod##_t *mod; \
  struct mod_##mod##_t

#define MODULE_DEF(mod) \
  extern mod_##mod##_t __##mod##_obj; \
  mod_##mod##_t *mod = &__##mod##_obj; \
  mod_##mod##_t __##mod##_obj

MODULE(os) {
  void (*init)();
  void (*run)();
};
/*
typedef struct mod_os_t mod_os_t;
extern mod_os_t *mod;
struct mod_os_t{
  void (*init)();
  void (*run)();
}
*/


MODULE(pmm) {
  void  (*init)();
  void *(*alloc)(size_t size);
  void  (*free)(void *ptr);
};
