#include "locks.h"

void lb_readLock(void** ptr) {
  unsigned long expec =(unsigned long)(*ptr);
  while(expec&lb_write_locked == lb_write_locked){
    do_sleep;
    expec =(unsigned long)(*ptr);
  } 
  unsigned long new_val = expec + 1;
  while(!__atomic_compare_exchange(ptr,
				   (void**)(&expec),
				   (void**)(&new_val),
				   1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
    do_sleep;
    expec =(unsigned long)(*ptr);
    while(expec&lb_write_locked == lb_write_locked){
      do_sleep;
      expec =(unsigned long)(*ptr);
    }
    new_val = expec + 1;
  }
}


void lb_writeLock(void** ptr){

  unsigned long expec = (unsigned long)(*ptr);
  expec&=lowBitsPtrMask;
  unsigned long new_val = expec|lb_write_locked;
  while(!__atomic_compare_exchange(ptr,
				   (void**)(&expec),
				   (void**)(&new_val),
				   1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
    do_sleep;
    expec = (unsigned long)(*ptr);
    expec&=lowBitsPtrMask;
    new_val = expec|lb_write_locked;
  }
}



void lb_unlock_rd(void** ptr){
  __atomic_sub_fetch((unsigned long*)ptr, 1, __ATOMIC_RELAXED);
}


void lb_unlock_wr(void** ptr){
  unsigned long new_val = (unsigned long)(*ptr);
  new_val&=lowBitsPtrMask;
  __atomic_store_n((unsigned long*)ptr, new_val, __ATOMIC_RELAXED);
}



void hb_readLock(void** ptr) {
  unsigned long expec =(unsigned long)(*ptr);
  while(((expec>>highBits)&hb_write_locked) == hb_write_locked){
      do_sleep;
      expec =(unsigned long)(*ptr);
    }
    unsigned long new_val = expec + (1UL << highBits);
  while(!__atomic_compare_exchange(ptr,
				   (void**)(&expec),
				   (void**)(&new_val),
				   1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
    do_sleep;
    expec =(unsigned long)(*ptr);
    while((expec>>highBits)&hb_write_locked == hb_write_locked){
      do_sleep;
      expec =(unsigned long)(*ptr);
    }
    new_val = expec + (1UL << highBits);
  }
}


void hb_writeLock(void** ptr){

  unsigned long expec = (unsigned long)(*ptr);
  expec&=highBitsPtrMask;
  unsigned long new_val = expec|(hb_write_locked<<highBits);
  while(!__atomic_compare_exchange(ptr,
				   (void**)(&expec),
				   (void**)(&new_val),
				   1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
    do_sleep;
    expec = (unsigned long)(*ptr);
    expec&=highBitsPtrMask;
    new_val = expec|(hb_write_locked<<highBits);
  }
}



void hb_unlock_rd(void** ptr){
  __atomic_sub_fetch((unsigned long*)ptr, (1UL<<highBits), __ATOMIC_RELAXED);
}


void hb_unlock_wr(void** ptr){
  unsigned long new_val = (unsigned long)(*ptr);
  new_val&=highBitsPtrMask;
  __atomic_store_n((unsigned long*)ptr, new_val, __ATOMIC_RELAXED);
}



void ab_readLock(unsigned long* ptr) {
  unsigned long expec = (unsigned long)(*ptr);
  while(expec == ab_write_locked){
    do_sleep;
    expec =(unsigned long)(*ptr);
  }
  unsigned long new_val = expec + 1;
  while(!__atomic_compare_exchange(ptr,
				   (unsigned long*)(&expec),
				   (unsigned long*)(&new_val),
				   1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
    do_sleep;
    expec =(unsigned long)(*ptr);
    while(expec == ab_write_locked){
      do_sleep;
      expec =(unsigned long)(*ptr);
    }
    new_val = expec + 1;
  }
}


void ab_writeLock(unsigned long* ptr){
  unsigned long expec = unlocked;
  unsigned long new_val = ab_write_locked;
  while(!__atomic_compare_exchange(ptr,
				   (unsigned long*)(&expec),
				   (unsigned long*)(&new_val),
				   1, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
    do_sleep;
    expec = unlocked;
    new_val = ab_write_locked;
  }
}



void ab_unlock_rd(unsigned long* ptr){
  __atomic_sub_fetch((unsigned long*)ptr, 1, __ATOMIC_RELAXED);
}


void ab_unlock_wr(unsigned long* ptr){
    __atomic_store_n((unsigned long*)ptr, unlocked, __ATOMIC_RELAXED);
}

