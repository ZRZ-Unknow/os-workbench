#include <common.h>

#define current cpu_task[_cpu()].current

void sem_init(sem_t *sem, const char *name, int value){
  sem->name=name;
  sem->count=value;
  lock_init(&sem->lock,name);
  sem->blocked_task.prev=NULL;
  sem->blocked_task.next=NULL;
}
void sem_wait(sem_t *sem){
  lock_acquire(&sem->lock);
  sem->count--;
  if(sem->count<0){
    current->status=WAIT;
    lock_release(&sem->lock);
    _yield();
  }
  else lock_release(&sem->lock);
}
void sem_signal(sem_t *sem){
  lock_acquire(&sem->lock);
  sem->count++;
  lock_release(&sem->lock);
}