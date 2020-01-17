#include "opt.h"


void fast_memset(void* loc, size_t val, size_t len){
  size_t len_8 = (len>>3);
  
  unsigned long* loc_ul = (unsigned long*)loc;
  int i;
  for(i=0;i<len_8;i++){
    loc_ul[i] = val;
  }
  i <<= 3;
  //if mem for some reason is not 8 byte aligned
  unsigned char* loc_ch = (unsigned char*)loc;
  char* val_bytes = (char*)(&val);
  for(;i<len;i++){
    loc_ch[i] = val_bytes[i&0x7];
  }
}

void fast_bytecopy(void* dst, void* src, size_t len){
  unsigned long* dst_ul = (unsigned long*)dst;
  unsigned long* src_ul = (unsigned long*)src;
  size_t len_8 = len>>3;
  int i=0;
  
  for(i=0;i<len_8;i++){
    dst_ul[i]=src_ul[i];
  }
  i <<= 3;
  unsigned char* dst_ch = (unsigned char*)dst;
  unsigned char* src_ch = (unsigned char*)src;
  for(;i<len;i++){
    dst_ch[i]=src_ch[i];
  }
}

int fast_bytecmp_u(const void* a, const void* b, size_t len){
  unsigned long* a_ul = (unsigned long*)a;
  unsigned long* b_ul = (unsigned long*)b;
  int i;
  size_t len_8 = len>>3;
  for(i=0;i<len_8;i++){
    if(a_ul[i]!=b_ul[i]){
      return a_ul[i] > b_ul[i] ? 1 : -1;

    }
  }
  i <<= 3;
  unsigned char* a_ch = (unsigned char*)a;
  unsigned char* b_ch = (unsigned char*)b;
  for(;i<len;i++){
    if(a_ch[i]!=b_ch[i]){
      return a_ch[i] > b_ch[i] ? 1 : -1;
      }
  }
  return 0;
}


int fast_bytecmp(const void* a, const void* b, size_t len){
  long* a_ul = (long*)a;
  long* b_ul = (long*)b;
  int i;
  size_t len_8 = len>>3;
  for(i=0;i<len_8;i++){
    if(a_ul[i]!=b_ul[i]){
      return a_ul[i] > b_ul[i] ? 1 : -1;

    }
  }
  i <<= 3;
  char* a_ch = (char*)a;
  char* b_ch = (char*)b;
  for(;i<len;i++){
    if(a_ch[i]!=b_ch[i]){
      return a_ch[i] > b_ch[i] ? 1 : -1;
      }
  }
  return 0;
}

int ff1_asm(int x){
  int loc;
  __asm__("bsf %1, %0" : "=r" (loc) : "rm" (x));
  return loc;
}
int ff0_asm(int x){
  int loc;
  __asm__("bsf %1, %0" : "=r" (loc) : "rm" ((~x)));
  return loc;
}

int fl1_asm(int x){
  int loc;
  __asm__("bsr %1, %0" : "=r" (loc) : "rm" (x));
  return loc;
}

int fl0_asm(int x){
  int loc;
  __asm__("bsr %1, %0" : "=r" (loc) : "rm" ((~x)));
  return loc;  
}
