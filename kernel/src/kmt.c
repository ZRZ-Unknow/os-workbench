#include <common.h>

list_head task_list={NULL,NULL};
int task_num=0;

_Context * hello(){_putc('h');return NULL;}

void kmt_init(){
  os->on_irq(INI_MIN,_EVENT_NULL,hello);
}

int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  task->pid=task_num++;
  task->cpu=_cpu();
  task->status=INIT;
  task->name=name;
  task->entry=entry;
  task->arg=arg;
  _Area stack=(_Area){&task->canary+1,task+1};
  task->context=_kcontext(stack,entry,arg);
  task->canary=MAGIC;
  list_head *lh=&task_list;
  while(lh->next!=NULL) lh=lh->next;
  lh->next=&task->list;
  task->list.prev=lh;
  task->list.next=NULL;
  return 0;
}

void kmt_teardown(task_t *task){
  return;
}




MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
  .spin_init = lock_init,
  .spin_lock = lock_acquire,
  .spin_unlock = lock_release,
};