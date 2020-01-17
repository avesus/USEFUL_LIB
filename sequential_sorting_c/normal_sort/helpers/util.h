#ifndef _UTIL_H_
#define _UTIL_H_

#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"
#include "opt.h"

extern const char* progname;

//error handling
#define errdie(msg, args...) dieOnErrno(__FILE__, __LINE__, errno, msg, ##args)
void dieOnErrno(const char* fn, int ln, int en, const char* msg, ...);
void die(const char* fmt, ...);

//alloc wrappers
#define mycalloc(x, y) myCalloc((x), (y), __FILE__, __LINE__)
void* myCalloc(size_t nmemb, size_t size, const char* fname, const int ln);

#define mymalloc(x) myMalloc((x), __FILE__, __LINE__)
void* myMalloc(size_t size, const char* fname, const int ln);

#define myacalloc(x, y, z) myACalloc((x), (y), (z), __FILE__, __LINE__);
void* myACalloc(size_t alignment, size_t nmemb, size_t size, const char* fname, const int ln);

#define myaalloc(x, y) myACalloc((x), (y), __FILE__, __LINE__);
void* myAAlloc(size_t alignment, size_t size, const char* fname, const int ln);

//thread creation
#define mypthread_create(w, x, y, z) myPthread_Create((w), (x), (y), (z), __FILE__, __LINE__)
void myPthread_Create(pthread_t* tid,
		      pthread_attr_t* attr,
		      void* (fun)(void*),
		      void* args,
		      const char* fname,
		      const int ln);

//set core affinity for thread attr
#define myset_core(x, y) mySet_Core((x), (y), __FILE__, __LINE__)
void mySet_Core(pthread_attr_t* attr,
		size_t core,
		const char* fname,
		const int ln);

//IO
#define myopen2(x, y) myOpen((x), (y), __FILE__, __LINE__)
int myOpen2(const char* path, int flags, const char* fname, const int ln);

#define myopen3(x, y, z) myOpen((x), (y), (z), __FILE__, __LINE__)
int myOpen3(const char* path, int flags, mode_t mode, const char* fname, const int ln);

#define myread(x, y, z) myRead((x), (y), (z), __FILE__, __LINE__)
int myRead(int fd, void* buf, size_t count, const char* fname, const int ln);

#define mywrite(x, y, z) myWrite((x), (y), (z), __FILE__, __LINE__)
int myWrite(int fd, void* buf, size_t nbytes, const char* fname, const int ln);

#define myfopen(x, y) myFOpen((x), (y), __FILE__, __LINE__)
FILE* myFOpen(const char* path, const char* mode, const char* fname, const int ln);

#define myfread(w, x, y, z) myFRead((w), (x), (y), (z), __FILE__, __LINE__)
int myFRead(void* ptr, size_t size, size_t nmemb, FILE* fp, const char* fname, const int ln);

#define myfwrite(w, x, y, z) myFWrite((w), (x), (y), (z), __FILE__, __LINE__)
int myFWrite(void* ptr, size_t size, size_t nmemb, FILE* fp, const char* fname, const int ln);


double getMedian(unsigned long* arr, int len);
double getMean(unsigned long* arr, int len);
double getSD(unsigned long* arr, int len);
double getVar(unsigned long* arr, int len);
double getMin(unsigned long* arr, int len) ;
double getMax(unsigned long* arr, int len) ;



#endif
