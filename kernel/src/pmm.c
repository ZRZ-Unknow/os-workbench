#include <common.h>

static page_t *mem_start=NULL;
static kmem_cache kmc[8];
static spinlock_t lock_global;
int SLAB_SIZE[SLAB_TYPE_NUM]={8,16,32,64,128,256,512,1024,2048,4096};

static inline int get_obj_pos(void *addr){
  page_t *page=get_head_addr(addr);
  int pos=(addr-page->s_mem)/page->slab_size;
  return pos;
}

static inline size_t align_size(size_t size){
  size_t ret=1;
  while(ret<size) ret<<=1;
  return ret;
}

static int get_slab_pos(int size){
  int pos=-1;
  switch (size){
    case 8: pos=0;break;
    case 16: pos=1;break;
    case 32: pos=2;break;
    case 64: pos=3;break;
    case 128: pos=4;break;
    case 256: pos=5;break;
    case 512: pos=6;break;
    case 1024: pos=7;break;
    case 2048: pos=8;break;
    case 4096: pos=9;break;
    default:break;
  }
  Assert(pos!=-1,"size:%d",size);
  return pos;
}
//调用前先上锁
static void *get_free_obj(page_t* page){
  void *ret=NULL;
  int bitmap_num= (page->obj_num%32==0) ? (page->obj_num/32) : (page->obj_num/32+1);
  for(int i=0;i<bitmap_num;i++){
    if(page->bitmap[i]==I) continue;
    int pos=0;
    while((i*32+pos)<page->obj_num){
      if(getbit(page->bitmap[i],pos)==0){
        if(page->obj_cnt==0){  //改变cpu的free_num值
          int n=get_slab_pos(page->slab_size);
          kmc[page->cpu].free_num[n]--;
        }
        setbit(page->bitmap[i],pos);
        page->obj_cnt++;
        ret=page->s_mem+(i*32+pos)*page->slab_size;
        return ret;
      }
      pos++;
      assert(pos<32);
    }
  Log("i==%d,bitmap_num:%d,obj_cnt:%d,obj_num:%d,slab_size:%d,bit:%d",
       i,bitmap_num,page->obj_cnt,page->obj_num,page->slab_size,getbit(page->bitmap[i],0));
  }
  assert(0);
  return NULL;
}

static page_t *get_free_page(int num,int slab_size,int cpu){
  lock_acquire(&lock_global);
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
      mp->s_mem=(slab_size<=HDR_SIZE) ? (mp->addr+HDR_SIZE) : (mp->addr+slab_size);
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
    if((void*)mp==_heap.end && i<num){
      lock_release(&lock_global);
      return NULL;
    }  
  }
  lock_release(&lock_global);
  return first_page;
}

static void pmm_init() {
  mem_start=(page_t *) _heap.start;
  lock_init(&lock_global,"lock_global");
  for(int i=0;i<_ncpu();i++){
    kmc[i].cpu=i;
    char name[5]="";
    sprintf(&name[0],"cpu%d",i);
    lock_init(&kmc[i].lock,&name[0]);
    for(int j=0;j<SLAB_TYPE_NUM;j++){
      page_t *new_page=get_free_page(INIT_PAGENUM,SLAB_SIZE[j],i);
      kmc[i].slab_list[j].next=&new_page->list;
      kmc[i].freepage[j]=&new_page->list;
      new_page->list.prev=&kmc[i].slab_list[j];
      kmc[i].free_num[j]=INIT_PAGENUM;
    }
  }
}

static void *kalloc(size_t size) {

  size=align_size(size);
  int cpu=_cpu();
  void *ret=NULL;
  int sl_pos=0;  //slablist_pos
  for(;sl_pos<SLAB_TYPE_NUM;sl_pos++){
    if(size<=SLAB_SIZE[sl_pos]) break;
  }
  assert(sl_pos<SLAB_TYPE_NUM);
  
  Log("cpu:%d start alloc size:%d",cpu,size);
  //这里需要锁cpu:防止其他cpu并发的free
  lock_acquire(&kmc[cpu].lock);

  page_t *fs_page=list_entry(kmc[cpu].freepage[sl_pos],page_t,list);
  
  lock_acquire(&fs_page->lock);
  if(fs_page->obj_cnt<fs_page->obj_num){  //有空闲对象则直接分配
      ret=get_free_obj(fs_page);   
      lock_release(&fs_page->lock);
  }
  else{
    lock_release(&fs_page->lock);

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
    kmc[cpu].freepage[sl_pos]=&page->list;
    if(page->obj_cnt<page->obj_num){  //此时要么page可分配，要么lh指向链表中最后一个page且不可分配
      lock_acquire(&page->lock);
      ret=get_free_obj(page);  
      lock_release(&page->lock);
    }
  }

  if(!ret){  ///意味着链表中无空闲page，fs_page中无空闲对象，全局分配一个页面，且fs_page刚好是最后一个页面
    page_t *page=get_free_page(1,SLAB_SIZE[sl_pos],cpu);
    if(!page){
      return NULL;
    }
    assert(page->cpu==cpu);
    
    list_head *lh=kmc[cpu].freepage[sl_pos];
    assert(lh->next==NULL);
    lh->next=&page->list;
    page->list.prev=lh;
    kmc[cpu].free_num[sl_pos]++;
    ret=get_free_obj(page);
    kmc[cpu].freepage[sl_pos]=&page->list;
  }
  lock_release(&kmc[cpu].lock);
  Log("alloc %p",ret);
  assert( !(((intptr_t)ret)%size));  //align 
  return ret;
}

static void kfree(void *ptr) {
  Log("free:%p",ptr);
  page_t *page=get_head_addr(ptr);
  lock_acquire(&page->lock);

  int offset=(ptr-page->s_mem)/page->slab_size;
  int i=offset/32;
  int pos=offset-i*32;
  Assert( getbit(page->bitmap[i],pos)==1, "cpu:%d free ptr:[%p,%p),size:%d",page->cpu,ptr,ptr+page->slab_size,page->slab_size);
  page->obj_cnt--;
  clrbit(page->bitmap[i],pos);
  memset(ptr,0,page->slab_size);
  if(page->obj_cnt==0){
    //需要对cpu上锁
    int cpu=page->cpu;
    lock_acquire(&kmc[cpu].lock);
    int n=get_slab_pos(page->slab_size);
    Log("cpu:%d,slab_type:%d,free_page_num:%d",cpu,n,kmc[cpu].free_num[n]);
    kmc[cpu].free_num[n]++;
    if(kmc[cpu].free_num[n]>=SLAB_LIMIT && kmc[cpu].freepage[n]!=&page->list){  //归还页面到_heap
      Log("cpu%d,slab_type:%d,free_page_num:%d,return page to _heap",cpu,n,kmc[cpu].free_num[n]);

      page->cpu=-1;
      page->slab_size=0;
      page->obj_num=0;
      page->s_mem=NULL;
      list_head *prev=page->list.prev;
      list_head *next=page->list.next;
      prev->next=next;
      if(next) next->prev=prev;
      
      kmc[cpu].free_num[n]--;
    }
    lock_release(&kmc[cpu].lock);
  }
  lock_release(&page->lock);
}

static void *kalloc_safe(size_t size){
  int i=_intr_read();
  _intr_write(0); //关闭中断
  void *ret=kalloc(size);
  if(i) _intr_write(1);   //打开中断
  return ret;
}

static void kfree_safe(void *ptr){
  int i=_intr_read();
  _intr_write(0);
  kfree(ptr);
  if(i) _intr_write(1);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc_safe,
  .free  = kfree_safe,
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