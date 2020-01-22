#include "hashtable_alt.h"

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
int compare_int(node a, node b){
  return !((a.key == b.key));
}

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

node_block* addCheck(hashTable* table, unsigned int slot, node n){
  int before;
  node_block* temp=table->ht+slot;
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


int deleteNode(hashTable* table, node n){
  unsigned int slot = hashFun(n)&((1<<table->size)-1);
  node_block* temp=table->ht+slot;
  int i;
  while(temp){
    int taken_mask = getTaken(temp);
    while(taken_mask){
      __asm__("bsf %1, %0" : "=r" (i) : "rm" (taken_mask));
      taken_mask ^= 1<<i;
      if(!compare_nodes(n, temp->ele[i])){
	setFree(temp, (1<<i));
	table->items--;
	return 1;
      }
    }
    temp=getNext(temp);
  }
  return 0;
}

node* findNode(hashTable* table, node n){
  unsigned int slot = hashFun(n)&((1<<table->size)-1);
  node_block* temp=table->ht+slot;
  int i;
  while(temp){
    int taken_mask = getTaken(temp);
    while(taken_mask){
      __asm__("bsf %1, %0" : "=r" (i) : "rm" (taken_mask));
      taken_mask ^= 1<<i;
      if(!compare_nodes(n, temp->ele[i])){
	return (&(temp->ele[i]));
      }
    }
    temp=getNext(temp);
  }
  return NULL;
}

int addNode(hashTable* table, node n){
  unsigned int slot=hashFun(n);
  unsigned int full_val = slot;
  slot &= ((1<<table->size)-1);
  node_block* ret = addCheck(table, slot, n);
  if(ret){
    int i = getLoc(ret);
    ret = getNB(ret);
    if(!ret){
      ret=createBlock();
      i=0;
      setNext(ret, getNext((table->ht+slot)));
      setNext((table->ht+slot), ret);
    }
    ret->ele[i]=n;
    table->items++;
    setFree(ret, (1<<i));
    if(getResizeType(table->resize)){
      setSlotBit(ret, (1<<i), ((full_val>>(table->size))&1)<<i);
    }

    else if(table->items>(1<<table->size)){
      resize(table);
    }


    return 1;
  }
  return 0;
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
  return ret;
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

void resize(hashTable* table){
  int type = getResizeType(table->resize);
  int oldSize=(1<<table->size);
  table->resize++;
  table->size++;
  node_block* new_table=(node_block*)aligned_alloc(cache_line_size,
						   (1<<table->size)*sizeof(node_block));
  for(int iter=0;iter<(1<<table->size);iter++){
    fast_zero_nb((unsigned long*)(new_table+iter));
  }
  node_block* temp;
  unsigned int slot;
  int j;

  for(int i =0;i<oldSize;i++){
    int to_free = 0;
    temp=table->ht+i;
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
	addResize(new_table, slot,next_bit,temp->ele[j], type);
      }
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
  table->ht=new_table;
}

hashTable* initTable(int isize){
  hashTable* table=calloc(1, sizeof(hashTable));
  table->ht=aligned_alloc(cache_line_size, (1<<isize)*sizeof(node_block));
  for(int i=0;i<(1<<isize);i++){
    fast_zero_nb((unsigned long*)(table->ht+i));
  }
  table->size=isize;
  return table;
}

void freeTable(hashTable* table){
  int size = (1<<table->size);
  node_block* temp;
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
#ifdef int_test
	fprintf(stderr," %d:[%d][%d][%d] ",j,getTaken(temp)&(1<<j), temp->ele[j].key, temp->ele[j].val);
#endif
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
  for(int i =0;i<table->size;i++){
    items += printSlot(table, i, v);
  }
  fprintf(stderr,"%d == %d / %d\n", items, table->items, 1<<table->size);
}
