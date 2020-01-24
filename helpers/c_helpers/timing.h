#ifndef _TIMING_H_
#define _TIMING_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "config.h"


#define unit_change 1000
#define ns_per_sec 1000000000
#define us_per_sec 1000000
#define ms_per_sec 1000
#define human_readable_threshold (1.0/1000.0)


//enum for doing units
enum time_unit{s = 1, ms = ms_per_sec, us = us_per_sec, ns = ns_per_sec};


//each event to be timed has one of these structs
typedef struct tevent{
  //the idea of difs/headers is that if you are timing
  //at N points for a given event (i.e time within a given function)
  //you can label each one
  char** headers;
  nanoseconds** difs;
  struct timespec* times;
  int ndifs;
  int ntimers;
  int index;

}tevent;

//overall timer struct
typedef struct my_timers {
  FILE* outfile;
  tevent* events;
  char** ev_headers;
  int nevents;
}my_timers;

//you can access this anywhere
extern my_timers cur_timers;

//general helpers for unit conversion
unsigned long to_nsecs(struct timespec t);
unsigned long to_usecs(struct timespec t);
unsigned long to_msecs(struct timespec t);
unsigned long to_secs(struct timespec t);

//dif between timestructs for dif units
unsigned long ns_diff(struct timespec t1, struct timespec t2);
unsigned long us_diff(struct timespec t1, struct timespec t2);
unsigned long ms_diff(struct timespec t1, struct timespec t2);
unsigned long s_diff(struct timespec t1, struct timespec t2);

//convert from ns to desired (specified in enum)
double unit_convert(double time_ns, enum time_unit desired);

//create a string of unit (simply ms -> "ms")
const char* unit_to_str(enum time_unit u);

//print time, give ns will printout in human readable
//format
void printTimeHR(unsigned long ns);

//take time (assumes you have setup curTimers)
void takeTime(int event_num);

//can use this to just take a timestamp whenever, returns
//timespec that just recorded cur time
struct timespec grabTime();

//initialize cur_timers
void initTiming(int nevents, int* ntimers, char*** dif_headers, char** ev_headers, FILE* outfile);

//cautiously frees some memory (will free the outside ptr for dif_headers and ev_header
//but not the inside strings as those are more likely to be constant. Maybe worth
//updating so take a value to determine how much it will free)
void freeTiming();

//return timestruct N in event_num 
struct timespec getTimeN(int event_num, int n);

//diff pattern 1 is just the raw array of timespec basically
void diffPattern1(int event_num);

//can use this to create diffs assuming N different time steps per iteration
//i.e timing sort would be diffpatternN(ev, 2), start setup, start sort, end sort
//would require diffpatternN(ev, 3), etc...
void diffPatternN(int event_num, int n);

//prints an event, no csv_flag will be semi human readable. specify ptype for
//units
void printEvent(int csv_flag, int csv_header, enum time_unit ptype, int event_num);

//will print event for all events in cur_timers.
void printStats(int csv_flag, enum time_unit ptype);

//initializes barrier, just calls robust init function in utils
void timingBarrierInit(pthread_barrier_t* barrier, int nthreads);

//waits at barrier and has tid 0 takeTime(event_num)
void timingBarrierWait(pthread_barrier_t* barrier, int event_num, int tid);

#endif
