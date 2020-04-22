#include <common.h>


void kmt_init(){

}







MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .spin_init = lock_init,
  .spin_lock = lock_acquire,
  .spin_unlock = lock_release,
};