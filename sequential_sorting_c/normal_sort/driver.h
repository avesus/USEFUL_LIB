#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "helpers/locks.h"
#include "helpers/opt.h"
#include "helpers/util.h"
#include "helpers/arg.h"
#include "helpers/bits.h"
#include "helpers/timing.h"
#include "helpers/temp.h"


//args

int regtemp=0;
int verbose = 0;
size_t sort_len=20;
int rseed=0;
int ntrials=1;
int doChar=0, doShort=0, doInt=0, doLong=0, doAll;
int doMS=0, doQS=0, doAS=1;

int csv=0;
int corr_test=0;



//for qsort (correctness test)
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


//sort.h stuff
#define SORT_NAME sorter_ch
#define SORT_TYPE char
#include "sort.h"

#define SORT_NAME sorter_sh
#define SORT_TYPE short
#include "sort.h"

#undef SORT_NAME
#undef SORT_TYPE

#define SORT_NAME sorter_int
#define SORT_TYPE int
#include "sort.h"

#undef SORT_NAME
#undef SORT_TYPE

#define SORT_NAME sorter_ul
#define SORT_TYPE long
#include "sort.h"

#undef SORT_NAME
#undef SORT_TYPE


void sortTest_MS();
void sortTest_QS();

