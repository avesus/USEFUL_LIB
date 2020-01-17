#include "bits.h"
void setPtr(void** n, void* new_ptr);
void* getPtr(void* n);
void lowBitsSetDECR(void** n);
void lowBitsSetINCR(void** n);
void lowBitsSetADD(void** n, int bits);
void lowBitsSetMASK(void** n, int bits, unsigned long mask);
void lowBitsSetAND(void** n, int bits);
void lowBitsSetOR(void** n, int bits);
void lowBitsSetXOR(void** n, int bits);
void lowBitsSet(void** n, int bits);
int lowBitsGet(void* n);
void* lowBitsGetPtr(void* n);
void highBitsSetDECR(void** n);
void highBitsSetINCR(void** n);
void highBitsSetADD(void** n, unsigned long bits);
void highBitsSetMASK(void**n, unsigned long bits, unsigned long mask);
void highBitsSetAND(void**n, unsigned long bits);
void highBitsSetOR(void**n, unsigned long bits);
void highBitsSetXOR(void**n, unsigned long bits);
void highBitsSet(void** n, unsigned short bits);
unsigned short highBitsGet(void* n);
void* highBitsGetPtr(void* n);

//drops high bits info returns ptr
void* highBitsGetPtr(void* n){
  return (void*)((((unsigned long)n)&highBitsPtrMask));
}



unsigned short highBitsGet(void* n){
  return (unsigned short)(((unsigned long)n)>>highBits);
}


void highBitsSet(void** n, unsigned short bits){
  unsigned long newPtr=bits;
  newPtr=newPtr<<highBits;
  newPtr|=(unsigned long)(highBitsGetPtr(*n));
  *n=(void*)newPtr;
}


void highBitsSetXOR(void**n, unsigned long bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr^=(bits<<highBits);
  *n=(void*)newPtr;
}


void highBitsSetOR(void**n, unsigned long bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr|=(bits<<highBits);
  *n=(void*)newPtr;
}


void highBitsSetAND(void**n, unsigned long bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr&=(bits<<highBits);
  *n=(void*)newPtr;
}


void highBitsSetMASK(void**n, unsigned long bits, unsigned long mask){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr&=~((mask)<<highBits);
  newPtr|=bits<<highBits;
  *n=(void*)newPtr;
}


void highBitsSetADD(void** n, unsigned long bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr+=(bits<<highBits);
  *n=(void*)newPtr;
}


void highBitsSetINCR(void** n){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr+=(1UL<<highBits);
  *n=(void*)newPtr;
}


//decrements highBits, this is unsafe
void highBitsSetDECR(void** n){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr-=(1UL<<highBits);
  *n=(void*)newPtr;
}


void* lowBitsGetPtr(void* n){
  return (void*)(((unsigned long)(n))&lowBitsPtrMask);
}



int lowBitsGet(void* n){
  return ((unsigned long)n)&lowBitsMask;
}




//sets low bits of n to b, this is unsafe
void lowBitsSet(void** n, int bits){
  *n=(void*)(((unsigned long)(lowBitsGetPtr(*n)))|bits);
}


void lowBitsSetXOR(void** n, int bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr^=bits;
  *n=(void*)newPtr;
}


void lowBitsSetOR(void** n, int bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr|=bits;
  *n=(void*)newPtr;
}


void lowBitsSetAND(void** n, int bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr&=bits;
  *n=(void*)newPtr;
}


void lowBitsSetMASK(void** n, int bits, unsigned long mask){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr&=(~(mask));
  newPtr|=bits;
  *n=(void*)newPtr;
}


void lowBitsSetADD(void** n, int bits){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr+=bits;
  *n=(void*)newPtr;
}


void lowBitsSetINCR(void** n){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr++;
  *n=(void*)newPtr;
}


void lowBitsSetDECR(void** n){
  unsigned long newPtr=(unsigned long)(*n);
  newPtr++;
  *n=(void*)newPtr;
}


//gets valid ptr, use if both low and high bits in use
void* getPtr(void* n){
  return (void*)((((unsigned long)n)&ptrMask));
}


void setPtr(void** n, void* new_ptr){
  int hb = highBitsGet(*n);
  int lb = lowBitsGet(*n);
  *n = new_ptr;
  highBitsSet(n, hb);
  lowBitsSet(n, lb);
}

