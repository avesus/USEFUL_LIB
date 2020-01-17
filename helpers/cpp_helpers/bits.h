#ifndef _BITS_H_
#define _BITS_H_

#include "config.h"

//assuming 48 bt VM address table
#define highBits 48
#define highBitsPtrMask ((1UL << highBits)-1)
//assuming 8 byte alignment
#define lowBits 3
#define lowBitsPtrMask (~((1UL<<lowBits)-1))
#define lowBitsMask 0x7
#define ptrMask (highBitsPtrMask&lowBitsPtrMask)



void setPtr(void** n, void* new_ptr);
void* getPtr(void* n);
void lowBitsSetMASK(void** n, int bits, unsigned long mask);
void lowBitsSetAND(void** n, int bits);
void lowBitsSetOR(void** n, int bits);
void lowBitsSetXOR(void** n, int bits);
void lowBitsSet(void** n, int bits);
int lowBitsGet(void* n);
void* lowBitsGetPtr(void* n);
void highBitsDecr(void** n);
void highBitsIncr(void** n);
void highBitsSetMASK(void**n, unsigned long bits, unsigned long mask);
void highBitsSetAND(void**n, unsigned long bits);
void highBitsSetOR(void**n, unsigned long bits);
void highBitsSetXOR(void**n, unsigned long bits);
void highBitsSet(void** n, unsigned short bits);
unsigned short highBitsGet(void* n);
void* highBitsGetPtr(void* n);

#endif
