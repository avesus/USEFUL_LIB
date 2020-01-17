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

enum time_unit{s = 1, ms = ms_per_sec, us = us_per_sec, ns = ns_per_sec};



typedef struct tevent{
  char** headers;
  nanoseconds** difs;
  struct timespec* times;
  int ndifs;
  int ntimers;
  int index;

}tevent;

typedef struct my_timers {
  FILE* outfile;
  tevent* events;
  char** ev_headers;
  int nevents;
}my_timers;

extern my_timers cur_timers;

unsigned long to_nsecs(struct timespec t);
unsigned long to_usecs(struct timespec t);
unsigned long to_msecs(struct timespec t);
unsigned long to_secs(struct timespec t);


unsigned long ns_diff(struct timespec t1, struct timespec t2);
unsigned long us_diff(struct timespec t1, struct timespec t2);
unsigned long ms_diff(struct timespec t1, struct timespec t2);
unsigned long s_diff(struct timespec t1, struct timespec t2);

double unit_convert(double time_ns, enum time_unit desired);
const char* unit_to_str(enum time_unit u);

void printTimeHR(unsigned long ns);

void takeTime(int event_num);
struct timespec grabTime();

void initTiming(int nevents, int* ntimers, char*** dif_headers, char** ev_headers, FILE* outfile);
void freeTiming();

struct timespec getTimeN(int event_num, int n);

void diffPattern1(int event_num);
void diffPatternN(int event_num, int n);

void printEvent(int csv_flag, int csv_header, enum time_unit ptype, int event_num);
void printStats(int csv_flag, enum time_unit ptype);

#endif
