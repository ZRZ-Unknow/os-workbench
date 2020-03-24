#include <common.h>

static heap_mem heap_free_mem;
static page_t *mem_start=NULL;
static kmem_cache kmc[8];
//static spinlock_t lock_global;
int SLAB_SIZE[SLAB_TYPE_NUM]={16,32,64,128,256,512,1024,4096};
int get_slab_pos(int size){
  int pos=-1;
  switch (size){
    case 16: pos=0;break;
    case 32: pos=1;break;
    case 64: pos=2;break;
    case 128: pos=3;break;
    case 256: pos=4;break;
    case 512: pos=5;break;
    case 1024: pos=6;break;
    case 4096: pos=7;break;
    default:break;
  }
  assert(pos!=-1);
  return pos;
}
//调用前先上锁
void *get_free_obj(page_t* page){
  int pos=0;
  void *ret=NULL;
  for(;pos<page->obj_num;pos++){
    if(page->bitmap[pos]==0){
      //Log("find free pos:%d",pos);
      assert(page->bitmap[pos]==0);
      if(page->obj_cnt==0){  //改变cpu的free_num值
        int cpu=page->cpu;
        int n=get_slab_pos(page->slab_size);
        kmc[cpu].free_num[n]--;
      }
      page->bitmap[pos]=1;
      page->obj_cnt++;
      int offset=pos*page->slab_size;
      ret=page->s_mem+offset;
      break;
    }
  }
  return ret;
}
//调用前先上锁
page_t *get_free_page(int num,int slab_size,int cpu){
  page_t *mp=mem_start;
  page_t *first_page=NULL;
  int i=0;
  while(i<num){
    if(mp->slab_size==0){
      i++;
      assert(mp->slab_size==0);
      mp->cpu=cpu;
      mp->slab_size=slab_size;
      mp->obj_cnt=0;
      mp->obj_num=(PAGE_SIZE-HDR_SIZE)/mp->slab_size;
      mp->addr=mp;
      if(slab_size>HDR_SIZE) mp->s_mem=mp->addr+slab_size;
      else mp->s_mem=mp->addr+HDR_SIZE;
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
    assert(((void*)mp)<_heap.end);
  }
  return first_page;
}

static void heap_init(){
  page_t *p=mem_start;
  page_t *prev=mem_start;
  heap_free_mem.freepage_list.next=&p->list;
  p->list.prev=&heap_free_mem.freepage_list;
  p++;
  int i=1;
  while((void*)p<(void*)_heap.end){
    prev->list.next=&p->list;
    p->list.prev=&prev->list;
    prev++;
    p++;
    i++;
  }
  prev->list.next->next=NULL;
  page_t *pp=list_entry(heap_free_mem.freepage_list.next,page_t,list);
  for(;pp!=NULL;pp++){
    printf("%p\n",(void*)pp);
  }
  panic();
}
static void pmm_init() {
  //uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  //printf("Got %d MiB heap: [%p, %p),cpu num:%d\n", pmsize >> 20, _heap.start, _heap.end,_ncpu());
  mem_start=(page_t *) _heap.start;
  lock_init(&heap_free_mem.lock_global,"lock_global");
  heap_init();
  for(int i=0;i<_ncpu();i++){
    kmc[i].cpu=i;
    char name[5]="";
    sprintf(&name[0],"cpu%d",i);
    lock_init(&kmc[i].lock,&name[0]);
    for(int j=0;j<SLAB_TYPE_NUM;j++){
      page_t *new_page=get_free_page(30,SLAB_SIZE[j],i);
      kmc[i].slab_list[j].next=&new_page->list;
      new_page->list.prev=&kmc[i].slab_list[j];
      kmc[i].free_num[j]=30;
    }
    //debug_slab_print(new_page);
  }
  //debug_print();
  //panic("test");
}

static void *kalloc(size_t size) {
  size=align_size(size);
  Log("start alloc size %d",size); 
  int cpu=_cpu();
  void *ret=NULL;
  int sl_pos=0;  //slablist_pos
  for(;sl_pos<SLAB_TYPE_NUM;sl_pos++){
    if(size<=SLAB_SIZE[sl_pos]) break;
  }
  assert(sl_pos<=SLAB_TYPE_NUM);
  //这里是否需要锁cpu?先锁一个试试
  lock_acquire(&kmc[cpu].lock);
  if(kmc[cpu].slab_list[sl_pos].next!=NULL){
    list_head *lh=kmc[cpu].slab_list[sl_pos].next;
    page_t *page=list_entry(lh,page_t,list);
    assert(page->obj_cnt<=page->obj_num);
    assert(page->cpu==cpu);
    while(page->obj_cnt==page->obj_num && lh->next!=NULL){  //已分配对象数小于总对象数时才可分配
      lh=lh->next;
      page=list_entry(lh,page_t,list);
      assert(page->obj_cnt<=page->obj_num);
      assert(page->cpu==cpu);
    }
    if(lh!=NULL){
      lock_acquire(&page->lock);
      assert(page->obj_cnt<=page->obj_num);
      ret=get_free_obj(page);   //这里需要有cpu的free_num的改变：一个完全free的page被分配，对应的free_num要-1
      lock_release(&page->lock);
    }
  }
  else assert(0);  //should never happen
  if(!ret){  //需要从_heap中分配，加一把大锁
    lock_acquire(&heap_free_mem.lock_global);
    page_t *page=get_free_page(1,SLAB_SIZE[sl_pos],cpu);
    if(!page){
      lock_release(&heap_free_mem.lock_global);
      return NULL;
    }
    assert(page->cpu==cpu);
    list_head *lh=&kmc[cpu].slab_list[sl_pos];
    while(lh->next!=NULL) lh=lh->next;
    assert(lh);
    lh->next=&page->list;
    page->list.prev=lh;
    page->list.next=NULL;
    assert(page->bitmap[0]==0);
    page->bitmap[0]=1;
    page->obj_cnt++;    //这个时候cpu的free_num是不变的：加了一个并不free的page
    ret=page->s_mem;
    lock_release(&heap_free_mem.lock_global);
  }
  lock_release(&kmc[cpu].lock);
  Log("alloc %p",ret);
  assert( !(((intptr_t)ret)%size));  //align 
  return ret;
}

static void kfree(void *ptr) {
  Log("free:%p",ptr);
  page_t *page=get_head_addr(ptr);
  assert(page);
  lock_acquire(&page->lock);
  int pos=(ptr-page->s_mem)/page->slab_size;
  page->obj_cnt--;
  Assert(page->bitmap[pos]==1,"ptr:[%p,%p),size:%d",ptr,ptr+page->slab_size,page->slab_size);
  page->bitmap[pos]=0;
  memset(ptr,0,page->slab_size);
  if(page->obj_cnt==0){
    //需要对cpu上锁
    int cpu=page->cpu;
    lock_acquire(&kmc[cpu].lock);
    int n=get_slab_pos(page->slab_size);
    Log("cpu:%d,slab_type:%d,free_page_num:%d",cpu,n,kmc[cpu].free_num[n]);
    kmc[cpu].free_num[n]++;
    if(kmc[cpu].free_num[n]>=SLAB_LIMIT){  //归还页面
      Log("cpu%d,slab_type:%d,free_page_num:%d,return page to _heap",cpu,n,kmc[cpu].free_num[n]);
      lock_acquire(&heap_free_mem.lock_global);
      page->cpu=-1;
      page->slab_size=0;
      page->obj_num=0;
      page->s_mem=NULL;
      list_head *prev=page->list.prev;
      list_head *next=page->list.next;
      prev->next=next;
      if(next) next->prev=prev;
      lock_release(&heap_free_mem.lock_global);
      kmc[cpu].free_num[n]--;
    }
    lock_release(&kmc[cpu].lock);
  }
  lock_release(&page->lock);
}


MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};



/*---------------------------debug-------------------------*/

//p本身指向page的首地址，p->addr也是；p->list是page中member list的首地址，p->list.prev指向上一个page的list的首地址
//printf("%d,%d,%p,%p,%p,%p,%p\n",p->slab_size,p->obj_cnt,p,p->addr,&p->list,p->list.prev,p->list.next);
//page_t *task=list_entry(&p->list,page_t,list)
void debug_print(){
  for(int i=0;i<_ncpu();i++){
    for(int j=0;j<SLAB_TYPE_NUM;j++){
      printf("cpu:%d,free_num:%d\n",kmc[i].cpu,kmc[i].free_num[j]);
      for(list_head *p=kmc[i].slab_list[j].next;p!=NULL;p=p->next){
        page_t *page=list_entry(p,page_t,list);
        printf("lock:%d,slab_size:%d,obj_cnt:%d,obj_num:%d,addr:%p,s_mem:%p,self:%p,prev:%p,next:%p\n",page->lock.locked,
          page->slab_size,page->obj_cnt,page->obj_num,page->addr,page->s_mem,&page->list,page->list.prev,page->list.next);
      }
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







/*----------------------------waste----------------------*/  //may be useful someday
/*page_t *page_init(int num){
  int i=0,j=0;
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
}*/