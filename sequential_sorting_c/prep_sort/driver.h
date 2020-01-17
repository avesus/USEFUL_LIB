#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "helpers/locks.h"
#include "helpers/util.h"
#include "helpers/arg.h"
#include "helpers/bits.h"
#include "helpers/timing.h"
#include "helpers/temp.h"
#include "sorts/timsort.h"
#include "prepsort.h"


//args
int regtemp=0;
int verbose = 0;
size_t sort_len=20;
size_t sb_len=32;
int rseed=0;
int ntrials=1;
int doChar=0, doShort=0, doInt=0, doLong=0, doStr=0, doByte, doAll=0;
int type_bit_mask = 0;
int csv=0;
int corr_test=0;




//these are basic value functions that perform decently for me
unsigned long charVal(const void* i, const size_t item_size){
return *(char*)i;
}

unsigned long shortVal(const void* i, const size_t item_size){
  return *(short*)i;
}

unsigned long intVal(const void* i, const size_t item_size){
  return *(int*)i;
}

unsigned long longVal(const void* i, const size_t item_size){
  return *(long*)i;
}





//for strval
unsigned long swapUL(unsigned long x){
  x=(((x & 0xffffffff00000000ull)>>32ull)|((x & 0x0000000ffffffffull)<<32ull));
  x=(((x & 0xffff0000ffff0000ull)>>16ull)|((x & 0x0000ffff0000ffffull)<<16ull));
  x=(((x & 0xff00ff00ff00ff00ull)>>8ull)|((x & 0x00ff00ff00ff00ffull)<<8ull));
  return x;
}

unsigned long swapUI(unsigned long x){
  x=(((x & 0xffff0000)>>16)|((x & 0x0000ffff)<<16));
  x=(((x & 0xff00ff00)>>8)|((x & 0x00ff00ff)<<8));
  return x;
}

unsigned long swapUS(unsigned long x){
  x=(((x & 0xff00)>>8)|((x & 0x00ff)<<8));
  return x;
}



unsigned long strVal(const void* str, const size_t item_size){
  int str_len = highBitsGet((void*)(*((char**)str)));
  char* real_str = (char*)getPtr((void*)(*((char**)str)));

#ifdef debug_mode
  assert(str_len>1);
#endif
  if(str_len>=8){
    unsigned long val=swapUL((*(unsigned long*)real_str));
    return val;
  }
  else if(str_len<8&&str_len>=4){
    unsigned long val=swapUI((*(unsigned long*)real_str)&0xffffffff);
    return val;
  }
  else if(str_len<4&&str_len>=2){
    unsigned long val=swapUS((*(unsigned long*)real_str)&0xffff);
    return val;
  }
  else{
    return (*(unsigned long*)real_str)&0xff;
  }
}

unsigned long byteVal(const void* bytes, const size_t item_size){
  if(item_size>=8){
    unsigned long val=swapUL((*(unsigned long*)bytes));
    return val;
  }
  else if(item_size<8&&item_size>=4){
    unsigned long val=swapUI((*(unsigned long*)bytes)&0xffffffff);
    return val;
  }
  else if(item_size<4&&item_size>=2){
    unsigned long val=swapUS((*(unsigned long*)bytes)&0xffff);
    return val;
  }
  else{
    return (*(unsigned long*)bytes)&0xff;
  }
}



int charCompare(const void* a, const void* b){
  return *(char*)a > *(char*)b;
}

int shortCompare(const void* a, const void* b){
  return *(short*)a > *(short*)b;
}

int intCompare(const void* a, const void* b){
  return *(int*)a > *(int*)b;
}

int longCompare(const void* a, const void* b){
  return *(long*)a > *(long*)b;
}


int strCompare(const void*a, const void* b){
  char* str_a = (char*)getPtr((void*)(*((char**)a)));
  char* str_b = (char*)getPtr((void*)(*((char**)b)));
  return strcmp((char*)str_a, (char*)str_b);
}


int byteCompare(const void* a, const void* b){
  return fast_bytecmp(a, b, sb_len);
}






//sort.h stuff
#define SORT_NAME sorter_ch
#define SORT_TYPE char
#include "sorts/sort.h"

#define SORT_NAME sorter_sh
#define SORT_TYPE short
#include "sorts/sort.h"

#undef SORT_NAME
#undef SORT_TYPE

#define SORT_NAME sorter_int
#define SORT_TYPE int
#include "sorts/sort.h"

#undef SORT_NAME
#undef SORT_TYPE

#define SORT_NAME sorter_ul
#define SORT_TYPE long
#include "sorts/sort.h"

#undef SORT_NAME
#undef SORT_TYPE




//tests
void prepSortTest();

