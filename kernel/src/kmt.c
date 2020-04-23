#include <common.h>

list_head task_list={NULL,NULL};
#define current cpu_task[_cpu()].current

int task_num=0;
void func(){while(1) printf("ddd\n");}

_Context *kmt_context_save(_Event ev,_Context *context){
  if(current){
    current->cpu=-1;
    current->status=SLEEP;
    current->context=context;
  }
  return NULL;
}
_Context *kmt_schedule(_Event ev,_Context *context){
  int id=-1;
  if(current){
    current->cpu=-1;
    current->status=SLEEP;
    id=current->pid;
  }
  list_head *lh=task_list.next;
  while(lh!=NULL){
    task_t *task=list_entry(lh,task_t,list);
    if(task->status==SLEEP && task->pid!=id){
      current=task;
      current->cpu=_cpu();
      current->status=RUN;
      break;
    }
  }
  _Context *ret=ret=current->context;
  return ret;
}

void kmt_init(){
  os->on_irq(INI_MIN,_EVENT_NULL,kmt_context_save);
  os->on_irq(INI_MAX,_EVENT_NULL,kmt_schedule);
}

int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  task->pid=task_num++;
  task->cpu=_cpu();
  task->status=SLEEP;
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
  list_head *prev=task->list.prev;
  list_head *next=task->list.next;
  prev->next=next;
  if(next) next->prev=prev;
  pmm->free(task);
}




MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
  .spin_init = lock_init,
  .spin_lock = lock_acquire,
  .spin_unlock = lock_release,
};