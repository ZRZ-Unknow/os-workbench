#include <common.h>

list_head task_list={NULL,NULL};
#define current cpu_task[_cpu()].current
task_t idle_task[MAX_CPU];
int task_num=0;
spinlock_t kmt_lk;

_Context *kmt_context_save(_Event ev,_Context *context){
  if(current){
    current->cpu=-1;
    current->status=SLEEP;
    current->context=context;
  }
  return NULL;
}
_Context *kmt_schedule(_Event ev,_Context *context){
  bool flag=false;
  if(current && current->list.next!=NULL){
    list_head *lh=current->list.next;
    while(lh!=NULL){
      task_t *task=list_entry(lh,task_t,list);
      if(task->status==SLEEP){
        Log("task %s,pid:%d",task->name,task->pid);
        current=task;
        current->cpu=_cpu();
        current->status=RUN;
        flag=true;
        break;
      }
      lh=lh->next;
    }
  }
  if(!flag){
    int pid=-1;
    if(current) pid=current->pid;
    list_head *lh=task_list.next;
    while(lh!=NULL){
      task_t *task=list_entry(lh,task_t,list);
      if(task->status==SLEEP && task->pid!=pid){
        Log("task %s,pid:%d",task->name,task->pid);
        current=task;
        current->cpu=_cpu();
        current->status=RUN;
        flag=true;
        break;
      }
      lh=lh->next;
    }
  }
  assert(current);
  if(!flag){
    current->cpu=_cpu();
    current->status=RUN;
  }
  Log("switch to thread:%s,pid:%d,cpu:%d",current->name,current->pid,current->cpu);
  return current->context;
}

void kmt_init(){
  lock_init(&kmt_lk,"kmt_lk");
  for(int i=0;i<_ncpu();i++){
    idle_task[i].name="idle";
    idle_task[i].cpu=-2;
    idle_task[i].pid=-(i+1);
    idle_task[i].status=SLEEP;
    idle_task[i].canary=MAGIC;
  }
  os->on_irq(INI_MIN,_EVENT_NULL,kmt_context_save);
  os->on_irq(INI_MAX,_EVENT_NULL,kmt_schedule);
}

void kmt_task_print(){
  list_head *lh=task_list.next;
  while(lh!=NULL){
    task_t *task=list_entry(lh,task_t,list);
    Log("task pid:%d,cpu:%d,status:%d,name:%s",task->pid,task->cpu,task->status,task->name);
    lh=lh->next;
  }
}
int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  
  task->cpu=-1;
  task->status=SLEEP;
  task->name=name;
  task->entry=entry;
  task->arg=arg;
  _Area stack=(_Area){&task->canary+1,task+1};
  task->context=_kcontext(stack,entry,arg);
  task->canary=MAGIC;
  
  lock_acquire(&kmt_lk);
  task->pid=task_num++;
  list_head *lh=&task_list;
  while(lh->next!=NULL) lh=lh->next;
  lh->next=&task->list;
  task->list.prev=lh;
  task->list.next=NULL;
  lock_release(&kmt_lk);
  //kmt_task_print();
  return 0;
}

void kmt_teardown(task_t *task){
  lock_acquire(&kmt_lk);
  list_head *prev=task->list.prev;
  list_head *next=task->list.next;
  prev->next=next;
  if(next) next->prev=prev;
  pmm->free(task);
  lock_release(&kmt_lk);
}




MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
  .spin_init = lock_init,
  .spin_lock = lock_acquire,
  .spin_unlock = lock_release,
};