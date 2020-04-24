#include <common.h>

static int ncli[MAX_CPU]={0};
static int intena[MAX_CPU]={0};
static int intr_flag=-1;

void lock_init(spinlock_t *lk,const char *name){
    lk->name=name;
    lk->locked=0;
    lk->cpu=-1;
}

void lock_acquire(spinlock_t *lk){
    intr_flag=_intr_read();
    _intr_write(0);
    pushcli();
    if(holding(lk)) Spanic("acquire,name:%s",lk->name); //当lk为1且cpu为当前cpu时，即禁止重入
    while(_atomic_xchg((intptr_t*)&lk->locked,1)!=0);
    __sync_synchronize();
    lk->cpu=_cpu();
}

void lock_release(spinlock_t *lk){
    if(!holding(lk)) Spanic("release,name:%s",lk->name);  //当lk为0或者 lk的cpu不为当前cpu
    lk->cpu=-1;
    __sync_synchronize();
    _atomic_xchg((intptr_t*)&lk->locked,0);
    popcli();
    if(intr_flag) _intr_write(1);
}

int holding(spinlock_t *lk){
    int r;
    pushcli();
    r=lk->locked && lk->cpu==_cpu();
    popcli();
    return r;
}
void pushcli(void){
    uint32_t eflags=get_efl();
    cli();
    if(ncli[_cpu()]==0) intena[_cpu()]= eflags & FL_IF;
    ncli[_cpu()]+=1;
}

void popcli(void){
    if(get_efl() & FL_IF) Spanic("popcli interruptible");
    if(--ncli[_cpu()] <0 ) Spanic("popcli");
    if(ncli[_cpu()]==0 && intena[_cpu()]) sti();
}