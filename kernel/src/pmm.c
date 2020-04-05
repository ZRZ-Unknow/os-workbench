#include <common.h>

static heap_mem heap_free_mem;
static page_t *mem_start=NULL;
static kmem_cache kmc[8];
//static spinlock_t lock_global;
int SLAB_SIZE[SLAB_TYPE_NUM]={8,16,32,64,128,256,512,1024,2048,4096};

int get_slab_pos(int size){
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
  assert(pos!=-1);
  return pos;
}
//调用前先上锁,调用一定返回非空。
//[8  ,16 ,32 ,64 ,128,256,512,1024,2048,4096]
//[992,496,248,124,62 ,31 ,15 ,7   ,3   ,1]
//[31 ,16 ,8  ,4  ,2  ,1  ,1  ,1   ,1   ,1]
void *get_free_obj(page_t* page){
  void *ret=NULL;
  int bitmap_num=(page->obj_num%32==0) ? (page->obj_num/32) : (page->obj_num/32+1);
  for(int i=0;i<bitmap_num;i++){
    if(page->bitmap[i]==I) continue;
    int pos=0;
    while(1){
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
  }
  assert(0);
  /*for(;pos<page->obj_num;pos++){
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
  return ret;*/
}

page_t *get_free_page(int num,int slab_size,int cpu){
  lock_acquire(&heap_free_mem.lock_global);

  if(heap_free_mem.num<num){
    lock_release(&heap_free_mem.lock_global);
    return NULL;
  }
  
  assert(heap_free_mem.freepage_list.next!=NULL);
  
  page_t *first_page=list_entry(heap_free_mem.freepage_list.next,page_t,list);
  page_t *mp=first_page;

  for(int i=0;i<num;i++){
    if((void*)mp>=_heap.end || mp==NULL){
      lock_release(&heap_free_mem.lock_global);
      return NULL;
    }

    mp->cpu=cpu;
    mp->slab_size=slab_size;
    mp->obj_cnt=0;
    mp->obj_num=(PAGE_SIZE-HDR_SIZE)/mp->slab_size;
    mp->addr=mp;
    mp->s_mem=(slab_size<=HDR_SIZE) ? (mp->addr+HDR_SIZE) : (mp->addr+slab_size);
    lock_init(&mp->lock,"");
    //if(slab_size>HDR_SIZE) mp->s_mem=mp->addr+slab_size;
    //else mp->s_mem=mp->addr+HDR_SIZE;
    
    heap_free_mem.num--;
    if(mp->list.next==NULL){
      if(i==num-1){
        assert(heap_free_mem.num==0);
        heap_free_mem.freepage_list.next=NULL;
        lock_release(&heap_free_mem.lock_global);
        return first_page;
      }
      else{
        assert(0);
      }
    }
    mp=list_entry(mp->list.next,page_t,list);
  }

  //fix list
  assert(((void*)mp)<_heap.end);
  assert(mp!=NULL);
  page_t *tmp=list_entry(mp->list.prev,page_t,list);
  heap_free_mem.freepage_list.next=&mp->list;
  mp->list.prev=&heap_free_mem.freepage_list;
  tmp->list.next=NULL;

  lock_release(&heap_free_mem.lock_global);
  return first_page;
}

void heap_init(){
  lock_init(&heap_free_mem.lock_global,"lock_global");
  heap_free_mem.num=PAGE_NUM;
  page_t *p=mem_start;
  page_t *prev=mem_start;
  heap_free_mem.freepage_list.prev=NULL;
  heap_free_mem.freepage_list.next=&p->list;
  p->list.prev=&heap_free_mem.freepage_list;
  p++;
  while((void*)p<_heap.end){
    prev->list.next=&p->list;
    p->list.prev=&prev->list;
    p->list.next=NULL;
    prev++;
    p++;
  }
  /*page_t *pp=list_entry(heap_free_mem.freepage_list.next,page_t,list);
  while(1){
    printf("%p\n",(void*)pp);
    if(pp->list.next==NULL) break;
    pp=list_entry(pp->list.next,page_t,list);
  }
  printf("%p,%p\n",_heap.start,_heap.end);
  while(1){
    printf("%p\n",(void*)pp);
    if(pp->list.prev->prev==NULL) break;
    pp=list_entry(pp->list.prev,page_t,list);
  }
  panic();*/
}
static void pmm_init() {
  //uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  //printf("Got %d MiB heap: [%p, %p),cpu num:%d\n", pmsize >> 20, _heap.start, _heap.end,_ncpu());
  mem_start=(page_t *) _heap.start;
  memset(mem_start,0,MEM_SIZE);
  heap_init();
  //为每个cpu的每种slab分配一个页面
  for(int i=0;i<_ncpu();i++){
    kmc[i].cpu=i;
    char name[5]="";
    sprintf(&name[0],"cpu%d",i);
    lock_init(&kmc[i].lock,&name[0]);
    for(int j=0;j<SLAB_TYPE_NUM;j++){
      page_t *new_page=get_free_page(1,SLAB_SIZE[j],i);
      kmc[i].slab_list[j].next=&new_page->list;
      kmc[i].freeslab_list[j].next=&new_page->list;
      new_page->list.prev=&kmc[i].slab_list[j];
      kmc[i].free_num[j]=1;
    }
    //debug_slab_print(new_page);
  }
  //debug_print();
  //panic("test");
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
  
  Log("cpu:%d start alloc size:%d,heap_free_page_num:%d",cpu,size,heap_free_mem.num);
  //这里需要锁cpu:防止其他cpu并发的free
  lock_acquire(&kmc[cpu].lock);

  list_head *fs_list=kmc[cpu].freeslab_list[sl_pos].next;
  page_t *fs_page=list_entry(fs_list,page_t,list);

  lock_acquire(&fs_page->lock);
  if(fs_page->obj_cnt<fs_page->obj_num){  //有空闲对象则直接分配
      ret=get_free_obj(fs_page);   //这里需要有cpu的free_num的改变：一个完全free的page被分配，对应的free_num要-1
      lock_release(&fs_page->lock);
  }
  else{   //遍历链表找到有空闲对象的page，
    lock_release(&fs_page->lock);
    list_head *lh=kmc[cpu].slab_list[sl_pos].next;
    page_t *page=list_entry(lh,page_t,list);
    assert(page->obj_cnt<=page->obj_num);
    assert(page->cpu==cpu);
    while(page->obj_cnt==page->obj_num && lh->next!=NULL){  //已分配对象数小于总对象数时才可分配
      lh=lh->next;
      page=list_entry(lh,page_t,list);
      Assert(page->cpu==cpu,"%d,%d",page->cpu,cpu);
    }
    kmc[cpu].freeslab_list[sl_pos].next=&page->list;
    if(page->obj_cnt<page->obj_num){  //此时要么page可分配，要么lh指向链表中最后一个page
      lock_acquire(&page->lock);
      assert(page->obj_cnt<page->obj_num);
      ret=get_free_obj(page);   //这里需要有cpu的free_num的改变：一个完全free的page被分配，对应的free_num要-1
      lock_release(&page->lock);
    }
  }


  if(!ret){  //意味着链表中无空闲page，fs_page中无空闲对象，全局分配一个页面
    page_t *page=get_free_page(1,SLAB_SIZE[sl_pos],cpu); //ddddddd
    if(!page){
      lock_release(&kmc[cpu].lock);
      return NULL;
    }
    assert(page->cpu==cpu);
    assert(page->list.next==NULL);
    
    fs_list=kmc[cpu].freeslab_list[sl_pos].next;
    fs_page=list_entry(fs_list,page_t,list);
    
    lock_acquire(&fs_page->lock);
    assert(fs_page->list.next==NULL);
    assert(fs_page->slab_size!=0);
    fs_page->list.next=&page->list;
    page->list.prev=&fs_page->list;
    lock_release(&fs_page->lock);
    
    kmc[cpu].freeslab_list[sl_pos].next=&page->list;
    kmc[cpu].free_num[sl_pos]+=1;

    lock_acquire(&page->lock);
    ret=get_free_obj(page);
    lock_release(&page->lock);
  } 
  lock_release(&kmc[cpu].lock);
  Log("cpu %d alloc ptr:%p,size:%d,heap_free_page_num:%d",cpu,ret,size,heap_free_mem.num);
  assert( !(((intptr_t)ret)%size));  //align
  return ret;
/*
  if(kmc[cpu].freeslab_list[sl_pos].next!=NULL){
    Log("%d",cpu);
    list_head *lh=kmc[cpu].slab_list[sl_pos].next;
    page_t *page=list_entry(lh,page_t,list);
    assert(page->obj_cnt<=page->obj_num);
    assert(page->cpu==cpu);
    while(page->obj_cnt==page->obj_num && lh->next!=NULL){  //已分配对象数小于总对象数时才可分配
      lh=lh->next;
      page=list_entry(lh,page_t,list);
      Assert(page->cpu==cpu,"%d,%d",page->cpu,cpu);
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
    int n=(size>=1024)?3:2;
    page_t *page=get_free_page(n,SLAB_SIZE[sl_pos],cpu); //ddddddd
    if(!page){
      lock_release(&kmc[cpu].lock);
      return NULL;
    }
    assert(page->cpu==cpu);
    list_head *lh=&kmc[cpu].slab_list[sl_pos];
    while(lh->next!=NULL) lh=lh->next;
    assert(lh);
    lh->next=&page->list;
    page->list.prev=lh;
    assert(page->bitmap[0]==0);
    page->bitmap[0]=1;
    page->obj_cnt++;    //误：这个时候cpu的free_num是不变的：加了一个并不free的page
    kmc[cpu].free_num[sl_pos]+=4;     //dddddddddddd
    ret=page->s_mem;
  }
  lock_release(&kmc[cpu].lock);
  Log("alloc %p,%d",ret,size);
  assert( !(((intptr_t)ret)%size));  //align 
  return ret;
*/
}

static void kfree(void *ptr) {
  Log("free:%p",ptr);
  page_t *page=get_head_addr(ptr);
  assert(page);

  lock_acquire(&page->lock);

  int offset=(ptr-page->s_mem)/page->slab_size;
  int i=offset/32;
  int pos=offset-i*32;
  Assert( getbit(page->bitmap[i],pos)==1, "kfree 0 ##ptr:[%p,%p),size:%d",ptr,ptr+page->slab_size,page->slab_size);
  page->obj_cnt--;
  clrbit(page->bitmap[i],pos);
  memset(ptr,0,page->slab_size);
  Log("heap free page num:%d",heap_free_mem.num);
 
  if(page->obj_cnt==0){
    int cpu=page->cpu;
    int n=get_slab_pos(page->slab_size);
    //归还页面
    lock_acquire(&kmc[cpu].lock);
    if(kmc[cpu].freeslab_list->next==&page->list){  //此时不用归还
      kmc[cpu].free_num[n]++;
      lock_release(&kmc[cpu].lock);
      lock_release(&page->lock);
      return;
    }
    page->cpu=-1;
    page->slab_size=0;
    page->obj_num=0;
    page->s_mem=NULL;
    list_head *prev=page->list.prev;
    list_head *next=page->list.next;
    prev->next=next;
    if(next) next->prev=prev;
    lock_release(&kmc[cpu].lock);
    //fix list
    lock_acquire(&heap_free_mem.lock_global);
    list_head *tmp=heap_free_mem.freepage_list.next;
    heap_free_mem.freepage_list.next=&page->list;
    page->list.prev=&heap_free_mem.freepage_list;
    page->list.next=tmp;
    if(tmp) tmp->prev=&page->list;
    heap_free_mem.num++;
    lock_release(&heap_free_mem.lock_global);
  }
  lock_release(&page->lock);
/*
  if(page->obj_cnt==0){
    //需要对cpu上锁
    int cpu=page->cpu;

    lock_acquire(&kmc[cpu].lock);

    int n=get_slab_pos(page->slab_size);
    Log("cpu:%d,slab_type:%d,free_page_num:%d",cpu,n,kmc[cpu].free_num[n]);
    kmc[cpu].free_num[n]++;

    if(kmc[cpu].free_num[n]>SLAB_LIMIT){  //归还页面
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
      
      //fix list
      list_head *tmp=heap_free_mem.freepage_list.next;
      heap_free_mem.freepage_list.next=&page->list;
      page->list.prev=&heap_free_mem.freepage_list;
      page->list.next=tmp;
      if(tmp) tmp->prev=&page->list;
      heap_free_mem.num++;
      lock_release(&heap_free_mem.lock_global);
      kmc[cpu].free_num[n]--;
    
    }
    lock_release(&kmc[cpu].lock);
  }
  lock_release(&page->lock);
*/
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