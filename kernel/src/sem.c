#include <common.h>

#define current cpu_task[_cpu()].current
extern spinlock_t os_trap_lk;

void sem_init(sem_t *sem, const char *name, int value){
  sem->name=name;
  sem->count=value;
  lock_init(&sem->lock,name);
  sem->blocked_task.prev=NULL;
  sem->blocked_task.next=NULL;
}
void sem_wait(sem_t *sem){
  lock_acquire(&sem->lock);
  Assert(sem->count>=0,"sem %s count:%d",sem->name,sem->count);
  if(sem->count==0){
    lock_acquire(&os_trap_lk);
    current->status=WAIT;
    lock_release(&os_trap_lk);
    //add current to sem->blocked_list
    list_head *lh=&sem->blocked_task;
    while(lh->next!=NULL)  lh=lh->next;
    lh->next=&current->sem_list;
    current->sem_list.prev=lh;
    current->sem_list.next=NULL;
    //lock_release(&os_trap_lk);
    lock_release(&sem->lock);
    _yield();
  }
  else{
    sem->count--;
    lock_release(&sem->lock);
  }
}
void sem_signal(sem_t *sem){
  lock_acquire(&sem->lock);
  sem->count++;
  if(sem->blocked_task.next!=NULL){
    //delete the first task in sem->blocked_list
    int i=holding(&os_trap_lk);
    if(!i) lock_acquire(&os_trap_lk);
    list_head *lh=sem->blocked_task.next;
    task_t *task=list_entry(lh,task_t,sem_list);
    list_head *next=lh->next;
    sem->blocked_task.next=next;
    if(next) next->prev=&sem->blocked_task;
    task->sem_list.prev=NULL;
    task->sem_list.next=NULL;
    task->status=SLEEP;
    if(!i) lock_release(&os_trap_lk);
  }
  lock_release(&sem->lock);
}