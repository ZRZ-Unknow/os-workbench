#include <common.h>


int ncli[MAX_CPU]={};
int intena[MAX_CPU]={};

void lock_init(spinlock_t *lk,char *name){
    lk->name=name;
    lk->locked=0;
    lk->cpu=-1;
}

void lock_acquire(spinlock_t *lk){
    pushcli();
    if(holding(lk)) panic("acquire");
    while(_atomic_xchg((intptr_t*)&lk->locked,1)!=0);
    __sync_synchronize();
    lk->cpu=_cpu();
    Log("cpu %d acquire lk %s",lk->cpu,lk->name);
}

void lock_release(spinlock_t *lk){
    if(!holding(lk)) panic("release");
    lk->cpu=-1;
    __sync_synchronize();
    _atomic_xchg((intptr_t*)&lk->locked,0);
    popcli();
}

int holding(spinlock_t *lk){
    int r;
    pushcli();
    r=lk->locked && lk->cpu==_cpu();
    popcli();
    return r;
}

void pushcli(void){
    uint32_t eflags;
    eflags=get_efl();
    cli();
    if(ncli[_cpu()]==0) intena[_cpu()]= eflags & FL_IF;
    ncli[_cpu()]+=1;
}

void popcli(void){
    if(get_efl() & FL_IF) panic("popcli interruptible");
    if(--ncli[_cpu()] <0 ) panic("popcli");
    if(ncli-_cpu()==0 && intena[_cpu()]) sti();
}