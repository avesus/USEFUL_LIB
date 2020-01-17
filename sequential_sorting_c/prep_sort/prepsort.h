#ifndef PREPSORT_H
#define PREPSORT_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "helpers/bits.h"
#include "helpers/opt.h"


#define getItem(x, y, z) ((x)+((y)*(z)))
#define MAX(x,y) (((x) > (y) ? (x)   : (y)))
#define MIN(x,y) (((x) < (y) ? (x)   : (y)))

//if you want to add a special case add function pointer to union
typedef union sort_funs{
  void (*sort_fun_generic)(void*, const size_t, const size_t,int (*compar_fun)(const void*, const void*));
  void (*sort_fun_long)(long*, const size_t);
  void (*sort_fun_int)(int*, const size_t);
  void (*sort_fun_short)(short*, const size_t);
  void (*sort_fun_char)(char*, const size_t);
  void (*sort_fun_str)(char**, const size_t);

}sort_funs;


//arr to be sorted
//length of array (amount of items)
//size of each item (thus total array size is len*itemSize)
//bool of whether it can be sorted as int/ul/float/etc...
//max possible value of values in array
//min possible value of values in array
//function that turns type being sorted into a value (really only for strings...)
//compare function as many sorts require that
//sort_funs is union of sort functions that can be used, just cleans up cases
void prepSort(void* arr, const size_t len, const size_t itemSize, int ordered,
	      const void* max,const void* min,
	      unsigned long (*val_fun)(const void*, const size_t),
	      int (*compar_fun)(const void*, const void*),sort_funs f);

//alternatively you can use these if you know the size ahead of time
//if you really want to optimize you can drop sort_funs arg and include the sort you
//want to use in the function
void prepSort_1(char* arr, const size_t len, const void* max,const void* min, sort_funs f);
void prepSort_2(short* arr, const size_t len, const void* max,const void* min, sort_funs f);
void prepSort_4(int* arr, const size_t len, const void* max,const void* min, sort_funs f);
void prepSort_8(long* arr, const size_t len, const void* max,const void* min, sort_funs f);



	
#endif


