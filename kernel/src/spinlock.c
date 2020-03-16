#include <common.h>

void lock_init(spinlock *lk,char *name){
    lk->name=name;
    lk->locked=0;
    lk->_cpu=_cpu();
}

void lock_acquire(spinlock *lk){
    TODO();
}