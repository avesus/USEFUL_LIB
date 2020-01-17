#ifndef _TEMP_H_
#define _TEMP_H_

#include <dirent.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"

// package to monitor temperature of cores in a multicore processor.
// Specific to linux.  Tested on Ubuntu.

//initializes num cores, path to core temps and memory for data
//storage
int initTemp(int trials, int num_threads);

//free allocated resources
void freeTemp();

//gets the data from the files, read .c for explination of args
void doTemps(int index, double* dest, int cnt);

//wont let thread past until its temp is under threshold (1 sec sleep
//between tests).x
void enforceTemps(int num_threads, int maxWait);

// get the current temps as the enforcement limits within +-delta
// celcius degrees.
void setEnforcedTemps(double delta, int num_threads);

#endif
