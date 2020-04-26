#include <common.h>

#define current cpu_task[_cpu()].current
#define cur_idle cpu_task[_cpu()].idle

static int task_num=0;
static list_head task_list={NULL,NULL};
extern spinlock_t os_trap_lk;

void kmt_task_print(){
  list_head *lh=task_list.next;
  while(lh!=NULL){
    #ifdef DEBUG
    task_t *task=list_entry(lh,task_t,list);
    Log("task pid:%d,cpu:%d,status:%d,name:%s",task->pid,task->cpu,task->status,task->name);
    #endif
    lh=lh->next;
  }
}

static void protect_canary(task_t *task){
  Assert(task->canary==MAGIC,"canary being damaged!");
}

static _Context *kmt_context_save(_Event ev,_Context *context){
  if(current){
    current->status=SLEEP;
    current->context=context;
    protect_canary(current);
  }
  else{
    cur_idle=context;
  }
  return NULL;
}

static _Context *kmt_schedule(_Event ev,_Context *context){
  bool flag=false;
  if(current && current->list.next!=NULL){
    list_head *lh=current->list.next;
    while(lh!=NULL){
      task_t *task=list_entry(lh,task_t,list);
      protect_canary(task);
      if(task->status==SLEEP && task->cpu==_cpu()){
        current=task;
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
      protect_canary(task);
      if(task->status==SLEEP && task->pid!=pid && task->cpu==_cpu()){
        current=task;
        current->status=RUN;
        flag=true;
        break;
      }
      lh=lh->next;
    }
  }
  if(!flag){  //如果没有sleep的线程，则继续执行原线程不变
    if(current){
      current->cpu=_cpu();
      current->status=RUN;
    }
    else{
      current=NULL;
      return cur_idle;
    }
  }
  assert(current);
  protect_canary(current);
  Log("switch to thread:%s,pid:%d,cpu:%d",current->name,current->pid,current->cpu);
  return current->context;
}

static void kmt_init(){
  //lock_init(&kmt_lk,"kmt_lk");
  for(int i=0;i<_ncpu();i++){
    current=NULL;
    cur_idle=NULL;
  }
  os->on_irq(INI_MIN,_EVENT_NULL,kmt_context_save);
  os->on_irq(INI_MAX,_EVENT_NULL,kmt_schedule);
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg){
  
  task->status=SLEEP;
  task->name=name;
  task->entry=entry;
  task->arg=arg;
  _Area stack=(_Area){&task->canary+1,task+1};
  task->context=_kcontext(stack,entry,arg);
  task->sem_list.prev=NULL;
  task->sem_list.next=NULL;
  task->canary=MAGIC;
  
  lock_acquire(&os_trap_lk);
  task->pid=task_num++;
  task->cpu=task->pid % _ncpu();     
  list_head *lh=&task_list;
  while(lh->next!=NULL) lh=lh->next;
  lh->next=&task->list;
  task->list.prev=lh;
  task->list.next=NULL;
  lock_release(&os_trap_lk);
  return 0;
}

void kmt_teardown(task_t *task){
  lock_acquire(&os_trap_lk);
  list_head *prev=task->list.prev;
  list_head *next=task->list.next;
  prev->next=next;
  if(next) next->prev=prev;
  pmm->free(task);
  lock_release(&os_trap_lk);
}



MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
  .spin_init = lock_init,
  .spin_lock = lock_acquire,
  .spin_unlock = lock_release,
  .sem_init = sem_init,
  .sem_wait = sem_wait,
  .sem_signal = sem_signal,
};