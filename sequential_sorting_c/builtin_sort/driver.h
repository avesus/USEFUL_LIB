#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "helpers/locks.h"
#include "helpers/temp.h"
#include "helpers/opt.h"
#include "helpers/util.h"
#include "helpers/arg.h"
#include "helpers/bits.h"
#include "helpers/timing.h"


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


void sortTest();
