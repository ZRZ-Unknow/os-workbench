#include <common.h>

#define NUM 7

static page_t *mem_start=NULL;
static kmem_cache kmc[MAX_CPU];
static spinlock_t lock_global;
static int SLAB_SIZE[NUM]={16,32,64,128,256,512,4096};

//调用前先上锁
void *get_free_obj(page_t* page){
  int pos=0;
  void *ret=NULL;
  for(;pos<page->obj_num;pos++){
    if(page->bitmap[pos]==0){
      Log("find free pos:%d",pos);
      page->bitmap[pos]=1;
      int offset=pos*page->slab_size;
      ret=page->s_mem+offset;
      break;
    }
  }
  return ret;
}

page_t *get_free_page(int num,int slab_size){
  lock_acquire(&lock_global);
  page_t *mp=mem_start;
  page_t *first_page=NULL;
  int i=0;
  while(i<num){
    if(mp->slab_size==0){
      i++;
      mp->slab_size=slab_size;
      mp->obj_cnt=0;
      mp->obj_num=(PAGE_SIZE-HDR_SIZE)/mp->slab_size;
      mp->addr=mp;
      mp->s_mem=mp->addr+HDR_SIZE;
      mp->list.next=NULL;
      mp->bitmap[0]=1;
      lock_init(&mp->lock,"");
      if(first_page==NULL){
        first_page=mp;
      }
      else{
        list_head *p=&first_page->list;
        while(p->next!=NULL) p=p->next;
        p->next=&mp->list;
        mp->list.prev=p;
      }
    }
    mp++;
    assert(((void*)mp)<_heap.end);
  }
  lock_release(&lock_global);
  return first_page;
}

static page_t *page_init(int num){
  int i=0;
  page_t *mp=mem_start;
  page_t *first_page=NULL;
  while(i<num){
    if(mp->slab_size==0){
      mp->slab_size=SLAB_SIZE[i++];
      mp->obj_cnt=0;
      mp->obj_num=(PAGE_SIZE-HDR_SIZE)/mp->slab_size;
      mp->addr=mp;
      mp->s_mem=mp->addr+HDR_SIZE;
      mp->list.next=NULL;
      lock_init(&mp->lock,"");
      if(first_page==NULL){
        first_page=mp;
      }
      else{
        list_head *p=&first_page->list;
        while(p->next!=NULL) p=p->next;
        p->next=&mp->list;
        mp->list.prev=p;
      }
    }
    mp++;
  }
  return first_page;
}

static void pmm_init() {
  //uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  //printf("Got %d MiB heap: [%p, %p),cpu num:%d\n", pmsize >> 20, _heap.start, _heap.end,_ncpu());
  mem_start=(page_t *) _heap.start;
  lock_init(&lock_global,"lock_global");
  for(int i=0;i<_ncpu();i++){
    kmc[i].cpu=i;
    kmc[i].slab_num[0]=NUM;   
    kmc[i].slab_num[1]=0;
    kmc[i].slab_num[2]=0;
    kmc[i].free_slab.prev=NULL;
    kmc[i].full_slab.prev=NULL;
    kmc[i].partial_slab.prev=NULL;
    page_t *new_page=page_init(NUM);
    kmc[i].free_slab.next=&new_page->list;
    //debug_slab_print(new_page);
  }
  //debug_print();
  //panic("test");
}

static void *kalloc(size_t size) {
  size=align_size(size);
  //Log("start alloc size %d",size); 
  int cpu=_cpu();
  void *ret=NULL;
  if(kmc[cpu].free_slab.next!=NULL){
    list_head *lh=kmc[cpu].free_slab.next;
    page_t *page=list_entry(lh,page_t,list);
    while(page->slab_size<size){
      lh=lh->next;
      if(lh==NULL) break;
      page=list_entry(lh,page_t,list);
    }
    if(lh==NULL){
      TODO();
    }
    else{
      //lock_acquire(&page->lock);
      ret=get_free_obj(page);
      //lock_release(&page->lock);
    }
  }
  else{
    TODO();
  }
  assert( !(((intptr_t)ret)%size));  //align */
  return ret;
}

static void kfree(void *ptr) {
}


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};


//p本身指向page的首地址，p->addr也是；p->list是page中member list的首地址，p->list.prev指向上一个page的list的首地址
//printf("%d,%d,%p,%p,%p,%p,%p\n",p->slab_size,p->obj_cnt,p,p->addr,&p->list,p->list.prev,p->list.next);
//page_t *task=list_entry(&p->list,page_t,list)

/*---------------------------debug-------------------------*/
void debug_print(){
  for(int i=0;i<_ncpu();i++){
    printf("cpu:%d,free_num:%d,full_num:%d,partial_num:%d\n",kmc[i].cpu,kmc[i].slab_num[0],kmc[i].slab_num[1],kmc[i].slab_num[2]);
    for(list_head *p=kmc[i].free_slab.next;p!=NULL;p=p->next){
      page_t *page=list_entry(p,page_t,list);
      printf("lock:%d,slab_size:%d,obj_cnt:%d,obj_num:%d,addr:%p,s_mem:%p,self:%p,prev:%p,next:%p\n",page->lock.locked,
        page->slab_size,page->obj_cnt,page->obj_num,page->addr,page->s_mem,&page->list,page->list.prev,page->list.next);
    }

  }
}
void debug_slab_print(page_t *page){
  int pos=0;
  for(;pos<page->obj_num;pos++){
    int offset=pos*page->slab_size;
    void *ret=page->s_mem+offset;
    int p=get_obj_pos(ret);
    assert(p==pos);
    printf("pos:%d,bitmap:%d,addr:[%p,%p)\n",p,page->bitmap[pos],ret,ret+page->slab_size);
  }
}