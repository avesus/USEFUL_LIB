#ifndef _LOCKS_H_
#define _LOCKS_H_

#include "bits.h"
#include <unistd.h>

#define unlocked 0
#define usleep_time 50
#define do_sleep usleep(usleep_time);

#define lb_max_threads 6
#define lb_write_locked 7

#define hb_max_threads (65534UL)
#define hb_write_locked (65535UL)

//this is kind of unnecissary...
#define ab_max_threads ((~(0UL))-1)
#define ab_write_locked (~(0UL))


//does locking with CAS on high bits of ptr
void hb_unlock_wr(void** ptr);
void hb_unlock_rd(void** ptr);
void hb_writeLock(void** ptr);
void hb_readLock(void** ptr);

//does lock with CAS on low bits of ptr
void lb_unlock_wr(void** ptr);
void lb_unlock_rd(void** ptr);
void lb_writeLock(void** ptr);
void lb_readLock(void** ptr);

//does lock with CAS on an unsigned long
void ab_unlock_wr(unsigned long* ptr);
void ab_unlock_rd(unsigned long* ptr);
void ab_writeLock(unsigned long* ptr);
void ab_readLock(unsigned long* ptr);


#endif
