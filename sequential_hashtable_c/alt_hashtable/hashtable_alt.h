#ifndef _HASHTABLE_ALT_H_
#define _HASHTABLE_ALT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include "helpers/bits.h"
#include "helpers/opt.h"
#include "test_config.h"


//table config is necessary
//////////////////////////////////////////////////////////////////////
//config for type hashtable will use

#ifdef int_test
typedef struct intNode{
  int key;
  int val;
}intNode;

//int type key
typedef struct intNode node;
int compare_int(node a, node b);
#define compare_nodes(X, Y) compare_int(X, Y)
#define hashFun(X) murmur3_32((const uint8_t*)(&((X).key)), getKeyLen((X)), mseed)
#define getKeyLen(X) sizeof(int)
#endif

#ifdef str_test
typedef struct strNode{
  char* key;
  char* val;
  int key_len;
  int val_len;
}strNode;

//str type key
typedef struct strNode node;
int compare_str(node a, node b);
short genTag(uint32_t val);
#define hashFun(X) strHash(X)
#define compare_nodes(X, Y) compare_str(X, Y)
#define getKeyLen(X) (X).key_len
#endif

#ifdef str_nolen_test
typedef struct strNode_nolen{
  char* key;
  char* val;
}strNode_nolen;

//str type (no len stored)

typedef struct strNode_nolen node;
int compare_str_nolen(node a, node b);
short genTag_nolen(uint32_t val, short first_2);
#define hashFun(X) strHash_nolen(X)
#define compare_nodes(X, Y) compare_str_nolen(X, Y)
#define getKeyLen(X)
#endif
#ifdef str_hblen_test
typedef struct strNode_hblen{
  char* key;
  char* val;
}strNode_hblen;

//str type (high bits store len)

typedef struct strNode_hblen node;
int compare_str_hblen(node a, node b);
char genTag_hblen(uint32_t val);
#define hashFun(X) strHash_hblen(X)
#define compare_nodes(X, Y) compare_str_hblen(X, Y)
#define getKeyLen(X) ((highBitsGet((void*)(X.key))>>8)&0xff)
#define setKeyLen(X, Y) (highBitsSet((void**)(&(X)), (Y)<<8))
#endif
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//Config for hash function table will use
#define mseed 0
uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);
uint32_t murmur3_32_nolen(const uint8_t* key, uint32_t seed);
uint32_t strHash(node n);
//////////////////////////////////////////////////////////////////////


#define cache_line_size 64
#define node_size sizeof(node)
#define meta_data sizeof(void*)
#define nnodes ((cache_line_size-meta_data)/node_size)
#define fmask ((1<<nnodes)-1)
#define padding_size (cache_line_size-(nnodes*sizeof(node)+meta_data))

//for cache aligning nodes
typedef struct node_block{
  node ele[nnodes+(nnodes==0)];
  struct node_block* next;
  unsigned char padding[padding_size];
}node_block;

//helpers to set meta info on nodes via
//low/high bits of node_block ptr
#define setLoc(X, Y) lowBitsSet((void**)(&(X)), Y)
#define getLoc(X) (lowBitsGet((void*)(X)))
#define getNB(X) ((node_block*)getPtr((void*)(X)))


#define getFree(X) ((~highBitsGet((void*)(X->next)))&fmask)
#define getTaken(X) ((highBitsGet((void*)(X->next)))&fmask)


#define getSlotBits(X) (highBitsGet((void*)(X->next))&(fmask<<8))
#define getOtherBits(X, Y) (highBitsGet((void*)(X->next))&(~((Y)<<8)))
#define getSlotBit(X, Y) ((highBitsGet((void*)(X->next))>>(8+Y))&0x1)

#define setSlotBit(X, Y, Z) highBitsSetMASK((void**)(&(X->next)), ((Z<<8)), ((Y<<8)))
#define setFree(X, Y) highBitsSetXOR((void**)(&(X->next)), ((Y)))

#define getNext(X) ((node_block*)getPtr((void*)(X->next)))
#define setNext(X, Y) (setPtr((void**)(&(X->next)), Y))

#if defined(str_test) || defined(str_nolen_test)
#define getKeyPtr(X) (getPtr((void*)(X)))
#define setKeyTag(X, Y) (highBitsSet((void**)(&(X)), (Y)))
#define getKeyTag(X) (highBitsGet((void*)(X)))
#endif
#ifdef str_hblen_test
#define getKeyPtr(X) (getPtr((void*)(X)))
#define setKeyTag(X, Y) (highBitsSetOR((void**)(&(X)), (Y)))
#define getKeyTag(X) ((highBitsGet((void*)(X)))&0xff)
#endif


//head of the hashtable
typedef struct hashTable{
  node_block* ht;
  int size;
  int items;
  int resize;
}hashTable;

#define getCanShrink(X) (X>>16)
#define setCanShrink(X) (X|=(1<<16))
#define getResizeType(X) ((!(X&0x1))<<8)
#define getResize(X) (X&((1<<16)-1))

//for creating block efficiently
void fast_zero_nb(unsigned long* ptr);
node_block* createBlock();

//resizing table
void resize(hashTable* table);
void addResize(node_block* new_table,
	       unsigned int slot,
	       unsigned int next_bit,
	       node n,
	       int type);

//printing table helper
int printSlot(hashTable* table, int slot, int v);
void printTable(hashTable* table, int v);

//add node
int addNode(hashTable* table, node n);
node_block* addCheck(hashTable* table, unsigned int slot, node n);

//delete node
int deleteNode(hashTable* table, node n);

//find node
node* findNode(hashTable* table, node n);

//initialize table with size (size is exponent)
hashTable* initTable(int isize);
void freeTable(hashTable* table);


#endif
