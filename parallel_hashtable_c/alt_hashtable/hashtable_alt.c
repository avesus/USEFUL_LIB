#include "hashtable_alt.h"
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>

//////////////////////////////////////////////////////////////////////
//hash function defined here
uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed){
  uint32_t h = seed;
  if (len > 3) {
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = len >> 2;
    do {

      uint32_t k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h = h * 5 + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

//murmur for a string when length is not stored
uint32_t murmur3_32_nolen(const uint8_t* key, uint32_t seed){
  uint32_t h = seed;
  const uint32_t* key_x4 = (const uint32_t*) key;
  size_t i = 0;
  uint32_t len = 0;
  int next;
  do {
    uint32_t k = *key_x4++;
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
    if((key[len] && key[len+1] && key[len+2] && key[len+3])||
       (len+4)>=MAX_KEY_HASH_LEN){
      break;
    }
  } while (1);
len+=4;
  key = (const uint8_t*) key_x4;
  if (len & 3) {
    i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

//////////////////////////////////////////////////////////////////////
//string test assuming length is stored in its own variable (depending
//on size of val (i.e string in this case), might cause worse cache
//performance
#ifdef str_test
short genTag(uint32_t val){
  short tag = val>>16;
  tag ^= val;
  return tag;
}

uint32_t strHash(node n){
  int hash_len = MIN(getKeyLen(n), MAX_KEY_HASH_LEN);
  uint32_t hv1 = murmur3_32((const uint8_t*)getKeyPtr(n.key),
			    hash_len>>1,
			    mseed);
  uint32_t hv2 = murmur3_32((const uint8_t*)getKeyPtr(n.key)+(hash_len>>1),
			    hash_len>>1,
			    mseed);
  short tag = genTag((hv1>>16)^(hv2<<16));
  setKeyTag(n.key, tag);
  return hv1^hv2;
}
#endif

//////////////////////////////////////////////////////////////////////
//get some better cache performance by storing length in key ptr but
//at the cost of only 8 bits for storing tag (more false comparisons)
//this also requires that key len < 255
#ifdef str_hblen_test
char genTag_hblen(uint32_t val){
  short tag = val>>16;
  tag ^= val;
  tag ^= (tag>>8);
  return tag&0xff;
}

uint32_t strHash_hblen(node n){
  int hash_len = MIN(getKeyLen(n), MAX_KEY_HASH_LEN);
  uint32_t hv1 = murmur3_32((const uint8_t*)getKeyPtr(n.key),
			    hash_len>>1,
			    mseed);
  uint32_t hv2 = murmur3_32((const uint8_t*)getKeyPtr(n.key)+(hash_len>>1),
			    hash_len>>1,
			    mseed);
  char tag = genTag_hblen((hv1>>16)^(hv2<<16));
  setKeyTag(n.key, tag);
  return hv1^hv2;
}
#endif

//////////////////////////////////////////////////////////////////////
//if you have keylen > 255 and want more nodes per cache line then
//might be best to not have string length store. There is modified
//hash function for that.
#ifdef str_nolen_test
short genTag_nolen(uint32_t val, short first_2){
  short tag = val>>16;
  tag ^= val;
  tag ^= first_2;
  return tag;
}


uint32_t strHash_nolen(node n){
  char* key = getKeyPtr(n.key);
  uint32_t hv = murmur3_32_nolen((const uint8_t*)key, mseed);
  short tag = genTag_nolen(hv, key[0]|(key[0]<<8));
  setKeyTag(n.key, tag);
  return hv;
}
#endif
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//comparison function for use defined here
#ifdef int_test
int compare_int(node a, node b){
  return !((a.key == b.key));
}
#endif

#ifdef str_test
int compare_str(node a, node b){
  if(a.key_len != b.key_len){
    return NOT_EQUALS;
  }
  if(getKeyTag(a.key)!=getKeyTag(b.key)){
    return NOT_EQUALS;
  }
  return (fast_bytecmp_u((const void*)getKeyPtr(a.key),
			 (const void*)getKeyPtr(b.key),
			 getKeyLen(a)));
}
#endif
#ifdef str_hblen_test
int compare_str_hblen(node a, node b){
  if(getKeyTag(a.key)!=getKeyTag(b.key)){
    return NOT_EQUALS;
  }
  return (fast_bytecmp_u((const void*)getKeyPtr(a.key),
			 (const void*)getKeyPtr(b.key),
			 getKeyLen(a)));

}
#endif

#ifdef str_nolen_test
int compare_str_nolen(node a, node b){
  if(getKeyTag(a.key)!=getKeyTag(b.key)){
    return NOT_EQUALS;
  }
  return (strcmp((char*)getKeyPtr(a.key),
		 (char*)getKeyPtr(b.key)));
}
#endif

//////////////////////////////////////////////////////////////////////






node_block* addCheck(node_block* ht, unsigned int slot, node n){
  int before;
  node_block* temp=ht+slot;
  node_block* ret=NULL;
  int ret_index=1;
  int i;
  while(temp){
    int taken_mask = getTaken(temp);
    if(!ret && (taken_mask!=fmask)){
      ret = temp;
      __asm__("bsf %1, %0" : "=r" (ret_index) : "rm" (getFree(ret)));
    }
    while(taken_mask){
      __asm__("bsf %1, %0" : "=r" (i) : "rm" (taken_mask));
      taken_mask ^= 1<<i;
      if(!compare_nodes(n, temp->ele[i])){
	return NULL;
      }
    }
    temp=getNext(temp);
  }
  setLoc(ret, ret_index);
  return ret;
}




int deleteNode_inner(node_block* ht, unsigned int slot, node n){
  node_block* temp=ht+slot;
  wrlock(ht+slot);
  int i;
  while(temp){
    int taken_mask = getTaken(temp);
    while(taken_mask){
      __asm__("bsf %1, %0" : "=r" (i) : "rm" (taken_mask));
      taken_mask ^= 1<<i;
      if(!compare_nodes(n, temp->ele[i])){
	setFree(temp, (1<<i));
	unlockWR(ht+slot);
	return 1;
      }
    }
    temp=getNext(temp);
  }
  unlockWR(ht+slot);
  return 0;
}


int deleteNode(hashTable* table, node n, int tid){
  resizePause(table);
  rdlock_resize(table, tid);
  unsigned int slot=hashFun(n);
  if(table->resizing==do_resize ||
     table->resizing==finish_resize){
    if(deleteNode_inner(table->ht, slot&((1<<(table->old_size))-1), n)){
      __atomic_sub_fetch(&(table->items), 1, __ATOMIC_RELAXED);
      unlockRD_resize(table, tid);
      return 1;
    }
    else if(deleteNode_inner(table->ht_new, slot&((1<<(table->size))-1), n)){
      __atomic_sub_fetch(&(table->items), 1, __ATOMIC_RELAXED);
      unlockRD_resize(table, tid);
      return 1;
    }
    unlockRD_resize(table, tid);
    return 0;
  }
  else{
    if(deleteNode_inner(table->ht, slot&((1<<(table->size))-1), n)){
      __atomic_sub_fetch(&(table->items), 1, __ATOMIC_RELAXED);
      unlockRD_resize(table, tid);
      return 1;
    }
    unlockRD_resize(table, tid);
    return 0;
  }
}

node* findNode_inner(node_block* ht, unsigned int slot, node n){
  node_block* temp=ht+slot;
  rdlock(ht+slot);
  
  int i;
  while(temp){
    int taken_mask = getTaken(temp);
    while(taken_mask){
      __asm__("bsf %1, %0" : "=r" (i) : "rm" (taken_mask));
      taken_mask ^= 1<<i;
      if(!compare_nodes(n, temp->ele[i])){
	unlockRD(ht+slot);
	return (&(temp->ele[i]));
      }
    }
    temp=getNext(temp);
  }
  unlockRD(ht+slot);
  return NULL;
}

node* findNode(hashTable* table, node n, int tid){

  resizePause(table);
  rdlock_resize(table, tid);
  unsigned int slot=hashFun(n);
  node* ret;
  if(table->resizing==do_resize ||
     table->resizing==finish_resize){
    ret = findNode_inner(table->ht, slot&((1<<(table->old_size))-1), n);
    if(ret){
      unlockRD_resize(table, tid);
      return ret;
    }
    ret = findNode_inner(table->ht_new, slot&((1<<table->size)-1), n);
    unlockRD_resize(table, tid);
    return ret;
  }else{
    ret = findNode_inner(table->ht, slot&((1<<table->size)-1), n);
    unlockRD_resize(table, tid);
    return ret;
  }
}

int addNode_inner(node_block* ht, int table_size, int type, unsigned int slot, node n){
  unsigned int full_val = slot;
  slot &= ((1<<table_size)-1);
  wrlock(ht+slot);
  node_block* ret = addCheck(ht, slot, n);
  if(ret){
    int i = getLoc(ret);
    ret = getNB(ret);
    if(!ret){
      ret=createBlock();
      i=0;
      setNext(ret, getNext((ht+slot)));
      setNext((ht+slot), ret);
    }
    ret->ele[i]=n;
    setFree(ret, (1<<i));
    if(type){
      setSlotBit(ret, (1<<i), ((full_val>>(table_size))&1)<<i);
    }
    unlockWR(ht+slot);
    return 1;
  }
  unlockWR(ht+slot);
  return 0;
}




int addNode(hashTable* table, node n, int tid){
  resizePause(table);
  rdlock_resize(table, tid); 
  unsigned int slot=hashFun(n);
  if(table->resizing==do_resize ||
     table->resizing==finish_resize){
    if(findNode_inner(table->ht, slot&((1<<(table->old_size))-1), n)){
      unlockRD_resize(table, tid);
      return 0;
    }
    if(addNode_inner(table->ht_new, table->size, getResizeType(table->resize), slot, n)){
      unlockRD_resize(table, tid);
      int items = __atomic_add_fetch(&(table->items), 1, __ATOMIC_RELAXED);
      return 1;
    }
    unlockRD_resize(table, tid);
    return 0;
    
  }else{
    if(addNode_inner(table->ht, table->size, getResizeType(table->resize), slot, n)){
      unlockRD_resize(table, tid);
      int items = __atomic_add_fetch(&(table->items), 1, __ATOMIC_RELAXED);
	if(items>(1<<table->size)&&(table->resizing==not_resize)){
	unsigned long expec_val=not_resize, new_val=start_resize;
	if(__atomic_compare_exchange(&(table->resizing),
				     &expec_val,
				     &new_val,
				     1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
	  pthread_t resize_tid;
	  mypthread_create(&resize_tid, NULL, resize, (void*)table);
	  return 1;
	}
      }

      return 1;
    }
    unlockRD_resize(table, tid);
    return 0;
  }
}


void fast_zero_nb(unsigned long* ptr){
  ptr[0]=0;
  ptr[1]=0;
  ptr[2]=0;
  ptr[3]=0;
  ptr[4]=0;
  ptr[5]=0;
  ptr[6]=0;
  ptr[7]=0;
}

node_block* createBlock(){
  node_block* ret = aligned_alloc(cache_line_size, sizeof(node_block));
  fast_zero_nb((unsigned long*)ret);
  initLock(ret);
  return ret;
}

#define small_normally 5
resize_free_nodes* resizeInitToFree(){
  resize_free_nodes* tf = mymalloc(sizeof(resize_free_nodes));
  fast_memset(tf, 0, sizeof(resize_free_nodes));
  tf->init_size = small_normally;
  tf->to_free[0] = mymalloc((1<<small_normally)*sizeof(void*));
  fast_memset(tf->to_free[0], 0, (1<<small_normally)*sizeof(void*));
  tf->next_x = (1<<small_normally);
  return tf;
}

void resizeAddToFree(resize_free_nodes* tf, void* n){
  tf->to_free[tf->x_slot][tf->y_slot] = n;
  tf->y_slot++;
  if(tf->y_slot == tf->next_x){
    tf->next_x <<= 1;
    tf->x_slot++;
    tf->to_free[tf->x_slot] = mymalloc(tf->next_x*sizeof(void**));
    tf->y_slot = 0;
  }
}

void resizeFreeToFree(resize_free_nodes* tf){
  int x_slot = tf->x_slot;
  int y_slot = tf->y_slot;
  int slot_size;
  for(int i =0;i<x_slot;i++){
    slot_size = (1<<(i+tf->init_size));
    for(int j=0;j<slot_size;j++){
      free(tf->to_free[i][j]);
    }
    free(tf->to_free[i]);
  }
  slot_size = (1<<(x_slot+tf->init_size));
  for(int i =0;i<y_slot;i++){
    free(tf->to_free[x_slot][i]);
  }
  free(tf->to_free[x_slot]);
  free(tf);
}


void addResize(node_block* new_table,unsigned int slot,unsigned int next_bit, node n, int type){
  node_block* temp=new_table+slot;
  int i;
  while(temp){
    int free_mask = getFree(temp);
    if(free_mask){
      __asm__("bsf %1, %0" : "=r" (i) : "rm" (free_mask));
      temp->ele[i]=n;
      setFree(temp, (1<<i));
      if(!type){
	setSlotBit(temp, (1<<i), (next_bit)<<i);
      }
      return;
    }
    temp=getNext(temp);
  }
  node_block* new_node = createBlock();
  new_node->ele[0]=n;
  setFree(new_node, (1));
  if(!type){
    setSlotBit(new_node, (1<<i), (next_bit)<<i);
  }
  setNext(new_node, getNext((new_table+slot)));
  setNext((new_table+slot), new_node);
}

void resizePause(hashTable* table){
  int temp_resize = __atomic_load_n(&table->resizing, __ATOMIC_RELAXED);
  while(temp_resize==start_resize ||
	temp_resize==finish_resize){
    do_sleep;
    temp_resize = __atomic_load_n(&table->resizing, __ATOMIC_RELAXED);
  }
}

void* resize(void* args){
  pthread_detach(pthread_self());
  hashTable* table = (hashTable*) args;
  pauseAll(table);
  table->resizing = do_resize;
  int type = getResizeType(table->resize);
  int old_size=(1<<table->size);
  table->old_size=table->size;
  table->resize++;
  table->size++;
  node_block* new_table=(node_block*)aligned_alloc(cache_line_size,
						   (1<<table->size)*sizeof(node_block));
  for(int iter=0;iter<(1<<table->size);iter++){
    fast_zero_nb((unsigned long*)(new_table+iter));
    initLock((&(new_table[iter])));
  }
  table->ht_new = new_table;
  unpauseAll(table);

  //prepare to free nodes (have to wait to do this till
  //after the table has been resized as parallel access
  //could otherwise cause invalid memory access
  resize_free_nodes* tf = resizeInitToFree();
  
  node_block* temp;
  unsigned int slot;
  int j;
  
  for(int i =0;i<old_size;i++){
    int to_free = 0;
    temp=table->ht+i;
    wrlock(table->ht+i);
    while(temp){
      int taken_mask = getTaken(temp);
      while(taken_mask){
	__asm__("bsf %1, %0" : "=r" (j) : "rm" (taken_mask));
	taken_mask^=(1<<j);
	int next_bit;
	if(type){
	  slot=(i | (getSlotBit(temp, j)<<(table->size-1)));
	}else{
	  slot = hashFun(temp->ele[j]);
	  next_bit=(slot>>table->size)&0x1;
	  slot&=((1<<table->size)-1);
	}

	wrlock(new_table+slot);
	addResize(table->ht_new, slot,next_bit,temp->ele[j], type);
	unlockWR(new_table+slot);

      }
      clearBits(temp);
      if(to_free){
	resizeAddToFree(tf, getPtr(temp));
      }
      temp=getNext(temp);
      to_free = 1;
    }
    unlockWR(table->ht+i);
  }
  table->resizing = finish_resize;
  pauseAll(table);
  free(table->ht);
  table->ht=table->ht_new;
  table->ht_new=NULL;
  table->resizing=not_resize;
  unpauseAll(table);
  resizeFreeToFree(tf);
  return NULL;
}

hashTable* initTable(int isize){
  hashTable* table=aligned_alloc(cache_line_size, sizeof(hashTable));
  
  fast_memset(table, 0, sizeof(hashTable));
  table->ht=aligned_alloc(cache_line_size, (1<<isize)*sizeof(node_block));

  for(int i=0;i<(1<<isize);i++){
    fast_zero_nb((unsigned long*)(table->ht+i));
    initLock((table->ht+i));
  }

  initLock_resize(table);
  table->resizing=0;
  table->size=isize;
  return table;
}

void pauseAll(hashTable* table){
  for(int i=0;i<max_threads;i++){
    wrlock_resize(table, i);
  }
}

void unpauseAll(hashTable* table){
  for(int i=0;i<max_threads;i++){
    unlockWR_resize(table, i);
  }

}


void freeTable(hashTable* table){
  int temp_resize = __atomic_load_n(&table->resizing, __ATOMIC_RELAXED);
  while(temp_resize!=not_resize){
    do_sleep;
    temp_resize = __atomic_load_n(&table->resizing, __ATOMIC_RELAXED);
  }
  
  int size = (1<<table->size);
  node_block* temp;
  free(table->resize_lock);
  for(int i=0;i<size;i++){
    temp=table->ht+i;
    int to_free = 0;
    while(temp){
      if(to_free){
	node_block* last = temp;
	temp=getNext(temp);
	free(last);
      }
      else{
	temp=getNext(temp);
      }
      to_free = 1;
    }
  }
  free(table->ht);
  free(table);
}



int printSlot(hashTable* table, int slot, int v){
  node_block* temp;
  int items=0;
  temp=table->ht+slot;
  if(v){
    fprintf(stderr,"%d -> ", slot);
  }
  while(temp){
    if(v){
      fprintf(stderr,"[");
    }
    for(int j=0;j<nnodes;j++){
      if(v){
	//	fprintf(stderr," %d:[%d][%d][%d] ",j,getTaken(temp)&(1<<j), temp->ele[j].key, temp->ele[j].val);
      }
      if(getTaken(temp)&(1<<j)){
	items++;
      }
    }
    if(v){
      fprintf(stderr,"]");
    }
    temp=getNext(temp);
  }
  if(v){
    fprintf(stderr,"\n");
  }
  return items;
}


void printTable(hashTable* table, int v){
  int items=0;
  for(int i =0;i<(1<<table->size);i++){
    items += printSlot(table, i, v);
  }
  fprintf(stderr,"%d == %d / %d\n", items, table->items, 1<<table->size);
}

