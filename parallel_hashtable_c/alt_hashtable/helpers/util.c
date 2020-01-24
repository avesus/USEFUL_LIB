#include "util.h"


const char* progname;

//////////////////////////////////////////////////////////////////////
//error functions
void dieOnErrno(const char* fn, int ln, int en, const char* msg, ...){
  va_list ap;
  va_start(ap, msg);
  fprintf(stderr, "%s:%d:", __FILE__, __LINE__);
  vfprintf(stderr, msg, ap);
  va_end (ap);
  fprintf(stderr, "\t%d:%s\n", en, strerror(en));
  exit(-1);
}

  
void die(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: ", progname);
  vfprintf(stderr, fmt, ap);
  va_end (ap);
  fprintf(stderr, "\n");
  exit(-1);
}

//////////////////////////////////////////////////////////////////////
//alloc stuff
void* myCalloc(size_t nmemb, size_t size, const char* fname, const int ln) {
  void* p = calloc(nmemb, size);
  if(!p){
    die("Failed to allocate memory at %s:%d", fname, ln);
  }
  return p;
}

void* myMalloc(size_t size, const char* fname, const int ln) {
  void* p = malloc(size);
  if(!p){
    die("Failed to allocate memory at %s:%d", fname, ln);
  }
  return p;
}

void* myAAlloc(size_t alignment, size_t size, const char* fname, const int ln){
  void* p = aligned_alloc(alignment, size);
  if(!p){
    die("Failed to allocate memory at %s:%d", fname, ln);
  }
  return p;
}

void* myACalloc(size_t alignment, size_t nmemb, size_t size, const char* fname, const int ln){
  void* p = aligned_alloc(alignment, size);
  if(!p){
    die("Failed to allocate memory at %s:%d", fname, ln);
  }
  fast_memset(p, 0, nmemb*size);
  return p;
}

void myFree(void* ptr){
  if(ptr){
    free(ptr);
  }
}

//////////////////////////////////////////////////////////////////////
//thread stuff
void mySet_Core(pthread_attr_t* attr,
		size_t core,
		const char* fname,
		const int ln){
  if(pthread_attr_init(attr)){
    errdie("Failed to init thread attr %s:%d\n", fname, ln);
  }
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  if(pthread_attr_setaffinity_np(attr, sizeof(cpu_set_t), &cpuset)){
    errdie("Failed to set core affinity %s:%d\n", fname, ln);
  }
}

void myPthread_Create(pthread_t* tid,
		      pthread_attr_t* attr,
		      void* (fun)(void*),
		      void* args,
		      const char* fname,
		      const int ln){
  if(pthread_create(tid, attr, fun, args)){
    errdie("Failed to create thread at %s:%d\n", fname, ln);
  }
}

void myBarrierInit(pthread_barrier_t* barrier,
		   int nthreads,
		   const char* fname,
		   const int ln){
  if(pthread_barrier_init(barrier, NULL, nthreads)){
    errdie("Failed to init barrier at %s:%d\n", fname, ln);
  }
}

//C IO fd
int myOpen2(const char* path, int flags, const char* fname, const int ln){
  int fd = open(path, flags);
  if(fd==-1){
    errdie("Failed to open %s at %s:%d\n", path, fname, ln);
  }
  return fd;
}

int myOpen3(const char* path, int flags, mode_t mode, const char* fname, const int ln){
  int fd = open(path, flags, mode);
  if(fd==-1){
    errdie("Failed to open %s at %s:%d\n", path, fname, ln);
  }
  return fd;
}

int myRead(int fd, void* buf, size_t count, const char* fname, const int ln){
  int result = read(fd, buf, count);
  if(result == -1){
    errdie("Failed to read at %s:%d\n", fname, ln);
  }
  return result;
}

int myWrite(int fd, void* buf, size_t nbytes, const char* fname, const int ln){
  int result = write(fd, buf, nbytes);
  if(result == -1){
    errdie("Failed to write at %s:%d\n", fname, ln);
  }
  return result;
}

//C IO fp
FILE* myFOpen(const char* path, const char* mode, const char* fname, const int ln){
  FILE* fp = fopen(path, mode);
  if(!fp){
    errdie("Failed to open %s at %s:%d\n", path, fname, ln);
  }
  return fp;
}

int myFRead(void* ptr, size_t size, size_t nmemb, FILE* fp, const char* fname, const int ln){
  int result = fread(ptr, size, nmemb, fp);
  if(!result){
    errdie("Failed to read at %s:%d\n", fname, ln);
  }
  return result;
}

int myFWrite(void* ptr, size_t size, size_t nmemb, FILE* fp, const char* fname, const int ln){
  int result = fwrite(ptr, size, nmemb, fp);
  if(!result){
    errdie("Failed to read at %s:%d\n", fname, ln);
  }
  return result;
}

static int dblcomp(const void* a, const void *b) {
  return *(double*)b - *(double*)a;
}

double getMedian(unsigned long* arr, int len){
#ifdef USAGE_CHECK
  if((!len) || (!arr)){
    die("Bad len or array: %p[%d]\n", arr, len);
  }
#endif
  double* arr_dbl = (double*)mycalloc(len, sizeof(double));
  for(int i=0;i<len;i++){
    arr_dbl[i] = (double)arr[i];
  }
  qsort(arr_dbl, len, sizeof(double), dblcomp);
  double median;
  if (len&0x1) {
    median = arr_dbl[len >> 1];
  }
  else {
    median = (arr_dbl[(len-1) >> 1] + arr_dbl[((len-1) >> 1)+1])/2.0;
  }
  free(arr_dbl);
  return median;
}

double getMean(unsigned long* arr, int len){
#ifdef USAGE_CHECK
  if((!len) || (!arr)){
    die("Bad len or array: %p[%d]\n", arr, len);
  }
#endif
  double total = 0.0;
  for (int i=0; i<len; i++) {
    total += (double)arr[i];
  }
  return total/(double)len;
}

double getSD(unsigned long* arr, int len){
#ifdef USAGE_CHECK
  if((!len) || (!arr)){
    die("Bad len or array: %p[%d]\n", arr, len);
  }
#endif
  if(len==1){
    return 0.0;
  }
  double sum = 0.0;
  double mean;
  double sd = 0.0;
  for(int i=0; i<len; i++){
    sum += (double)arr[i];
  }
  mean = sum/(double)len;

  for(int i=0; i<len; i++){
    sd += pow(arr[i] - mean, 2);
  }
  return sqrt(sd/(len-1));
}

double getVar(unsigned long* arr, int len){
#ifdef USAGE_CHECK
  if((!len) || (!arr)){
    die("Bad len or array: %p[%d]\n", arr, len);
  }
#endif

  double sum = 0.0;
  double mean;
  double sd = 0.0;
  for(int i=0; i<len; i++){
    sum += (double)arr[i];
  }
  mean = sum/(double)len;

  for(int i=0; i<len; i++){
    sd += pow(arr[i] - mean, 2);
  }
  return sqrt(sd/(len));
}

  
double getMin(unsigned long* arr, int len) {
#ifdef USAGE_CHECK
  if(!len || !arr){
    die("Bad len or array: %p[%d]\n", arr, len);
  }
#endif
  double m = arr[0];
  for(int i=0; i<len; i++)
    if (m > (double)arr[i]){
      m = (double)arr[i];
    }
  return m;
}


double getMax(unsigned long* arr, int len) {
#ifdef USAGE_CHECK
  if(!len || !arr){
    die("Bad len or array: %p[%d]\n", arr, len);
  }
#endif
  double m = arr[0];
  for(int i=0; i<len; i++)
    if (m < (double)arr[i]){
      m = (double)arr[i];
    }
  return m;
}

