#include <common.h>

void sem_init(sem_t *sem, const char *name, int value){
    sem->name=name;
    sem->value=value;
    lock_init(&sem->lock,name);
    sem->blocked_task.prev=NULL;
    sem->blocked_task.next=NULL;
    return;
}
void sem_wait(sem_t *sem){
    return;
}
void sem_signal(sem_t *sem){
    return;
}