#include "driver.h"

//////////////////////////////////////////////////////////////////////
//This is a driver for prepsort.c/prepsort.h prepsort is more
//effective on string/byte array types as well prepsort is NOT
//inplace, i.e it requires copying your data.
//build: make clean; make
//to see usage: ./driver -h
//to see setup for various sorts go to driver.h where
//all the value and compare functions can be found.

#define SUCCESS 0
#define FAILURE -1


//this is use for properly using timing.c/.h
#define num_sorts 1
#define num_types 6
#define name_size 32

//this is important for temp regulation library
#define nthreads 1
#define AllowedTempVariance 1.1
#define timeout 60


//sleep after intializing data. This ensures each run
//is done on an empty cache
#define stime 1



#define all_types ((!!doChar) + (!!doShort) + (!!doInt) + (!!doLong) + (!!doStr) + (!!doByte))
#define Version "0.1"

static ArgOption args[] = {
  // Kind, 	  Method,		name,	     reqd,  variable,		 help
  { KindOption,   Integer, 		"-v", 	     0,     &verbose, 		 "Set verbosity level" },
  { KindOption,   Set, 	        	"--csv",     0,     &csv, 		 "Set verbosity level" },
  { KindOption,   Integer,              "--len",     0,     &sort_len,           "Sets power for array size (i.e size= 1<<len)" },
  { KindOption,   Integer,              "--sb_len",  0,     &sb_len,             "Sets size of strings/byte arrays to be sorted" },
  { KindOption,   Integer,              "--seed",    0,     &rseed,              "Set random seed" },
  { KindOption,   Set,                  "--char",    0,     &doChar,             "do char sort test" },
  { KindOption,   Set,                  "--short",   0,     &doShort,            "do short sort test" },
  { KindOption,   Set,                  "--int",     0,     &doInt,              "do int sort test" },
  { KindOption,   Set,                  "--long",    0,     &doLong,             "do long sort test" },
  { KindOption,   Set,                  "--str",     0,     &doStr,              "do str sort test (this means each sorted item is a char* pointing to a string)" },
  { KindOption,   Set,                  "--bytes",   0,     &doByte,             "do sort on arbitrary length set of bytes (all stored in congiguous memory)" },
  { KindOption,   Set,                  "--all",     0,     &doAll,              "do all sort tests" },
  { KindOption,   Integer,              "--trials",  0,     &ntrials,            "number of trials to run" },
  { KindOption,   Set,                  "--regtemp", 0,     &regtemp,            "number of trials to run" },
  { KindOption,   Set,                  "--correct", 0,     &corr_test,          "test correctness" },
  { KindHelp,     Help, 	        "-h" },
  { KindEnd }
};
static ArgDefs argp = { args, "prepsort", Version, NULL };

//these are for printing in time.h
char purpose_header[name_size] = "Execution Time";
char type_headers[num_types][name_size] = {
  "chars",
  "shorts",
  "int",
  "long",
  "string",
  "bytes"
};

//sleep on matter what, regtemp, if on, will ensure
//core temps are below set threshold/timeout passes
//before continueing
void waitTill(){
  if(!corr_test){
    if(regtemp){
      enforceTemps(nthreads, timeout);
    }
    sleep(stime); //this is to ensure no hot cache
  }
}

//where whatever function(s) being tested should be called from
void* runner(void* args){

  for(int i=0;i<ntrials;i++){
    if(verbose){
      printf("Trial: %d\n", i);
    }
    //code that calls relevant function(s)
    prepSortTest();
  }
  return NULL;
}

void setup(){
  //setting up what to run
  if(doAll){
    type_bit_mask = (1<<num_types)-1;
    doChar = 1;
    doShort = 2;
    doInt = 3;
    doLong = 4;
    doStr = 5;
    doByte = 6;
  }
  else{
    doChar *= 1;
    doShort *= 2;
    doInt *= 3;
    doLong *= 4;
    doStr *= 5;
    doByte *= 6;
    
    type_bit_mask |= (1<<doChar);
    type_bit_mask |= (1<<doShort);
    type_bit_mask |= (1<<doInt);
    type_bit_mask |= (1<<doLong);
    type_bit_mask |= (1<<doStr);
    type_bit_mask |= (1<<doByte);
    type_bit_mask >>= 1;
  }


  //timing setup
  int * ntimers = (int*)mycalloc(all_types, sizeof(int));
  for(int i=0;i<all_types;i++){
    ntimers[i] = ntrials<<1;
  }

  //i know this kinda sucks but for multiple events/multi
  //timers in each event this is needed
  char*** dif_headers = (char***)mycalloc(all_types, sizeof(char**));
  for(int i=0;i<all_types;i++){
    dif_headers[i] = (char**)mycalloc(1, sizeof(char*));
    dif_headers[i][0] = (char*)purpose_header;
  }
  
  char** ev_headers = (char**)mycalloc(all_types, sizeof(char*));

  //setup each event (each different type being sorted)
  int type_index = 0;
  for(int j=0;j<num_types;j++){
    if((1<<j)&type_bit_mask){
      ev_headers[type_index] = (char*)type_headers[j];
      type_index++;
    }
  }

  //initializing the actual timing struct
  initTiming(all_types, ntimers, dif_headers, ev_headers, stdout);
  if(regtemp){
    //if regtemp initialize temp enforcer
    if(initTemp(ntrials, nthreads)){
      die("error accessing CPU temp/cores\n");
    }
    setEnforcedTemps(AllowedTempVariance, nthreads);
  }

  //NO MEMORY LEAKS!
  free(ntimers);
  free(dif_headers);
}

int main(int argc, char* argv[]){
  //setup args parser
  progname = argv[0];
  ArgParser* ap = createArgumentParser(&argp);
  int result = parseArguments(ap, argc, argv);
  if(result){
    die("Error parsing arguments");
  }
  freeCommandLine();
  freeArgumentParser(ap);

  srand(rseed);
  sort_len = 1<<sort_len;

  setup();

  //create runner thread
  pthread_t tid;
  pthread_attr_t attr;
  myset_core(&attr, 0);
  mypthread_create(&tid, &attr, runner, NULL);
  pthread_attr_destroy(&attr);
  pthread_join(tid, NULL);

  //diffpattern2 (2 time taken, start sort, end sort)
  for(int i=0;i<all_types;i++){
    diffPatternN(i, 2);
  }

  //print stats (ms prints it as milliseconds)
  printStats(csv, ms);

  //NO MEMORY LEAKS!
  freeTiming();
  freeTemp();
  return SUCCESS;
}


void prepSortTest(){
  int event_num = 0;

  //do char test
  if(doChar){


    char* arr=mymalloc(sort_len*sizeof(char)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      //char max
      arr[i]=rand()&0x7f;
    }

    //setup known correct sort for correctness test
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(char));
      fast_bytecopy(corr, arr, sort_len*sizeof(char));
      qsort(corr, sort_len, sizeof(char), charCompare);
    }

    //prepsort requires sMin/sMax for slot function
    unsigned long sMin=0;
    unsigned long sMax=0x7f;

    //prepsort is basically about preparing the data
    //to better optimize timsort, we pass timsort function
    //as function ptr. Prepsort will also improve things
    //like merge sort but timsort is best
    sort_funs f;
    f.sort_fun_char=sorter_ch_tim_sort;

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    prepSort(arr, sort_len, sizeof(char), 1,
	     (const void*)(&sMax),(const void*)(&sMin),
	     charVal, charCompare, f);
    takeTime(event_num);

    //do the correctness test
    if(corr_test){
      for(int i=0;i<sort_len;i++){
	assert(arr[i]==corr[i]);
      }
      //NO MEMORY LEAKS!
      free(corr);
    }
    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do short test
  if(doShort){

    //init array
    short* arr=mymalloc(sort_len*sizeof(short)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand()&0x7fff;
    }

    //setup correctness test
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(short));
      fast_bytecopy(corr, arr, sort_len*sizeof(short));
      qsort(corr, sort_len, sizeof(short), shortCompare);
    }

    //short min/ax
    unsigned long sMin=0;
    unsigned long sMax=0x7fff;

    //sort.h timsort is best i've found.
    sort_funs f;
    f.sort_fun_short=sorter_sh_tim_sort;

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    prepSort(arr, sort_len, sizeof(short), 1,
	     (const void*)(&sMax),(const void*)(&sMin),
	     shortVal, shortCompare, f);
    takeTime(event_num);

    //do correctness test
    if(corr_test){
      for(int i=0;i<sort_len;i++){
	assert(arr[i]==corr[i]);
      }
      //NO MEMORY LEAKS!
      free(corr);
    }
    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do int test
  if(doInt){

    //init arr
    int* arr=mymalloc(sort_len*sizeof(int)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
    }

    //prepare correctness test
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(int));
      fast_bytecopy(corr, arr, sort_len*sizeof(int));
      qsort(corr, sort_len, sizeof(int), intCompare);
    }

    //int min/max for slot function
    unsigned long iMax=RAND_MAX;
    unsigned long iMin=0;

    //setup sortfun
    sort_funs f;
    f.sort_fun_int=sorter_int_tim_sort;

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    prepSort(arr, sort_len, sizeof(int), 1,
	     (const void*)(&iMax),(const void*)(&iMin),
	     intVal, intCompare, f);
    takeTime(event_num);

    //do correctness test
    if(corr_test){
      for(int i=0;i<sort_len;i++){
	assert(arr[i]==corr[i]);
      }
      //NO MEMORY LEAKS!
      free(corr);
    }
    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do long test
  if(doLong){

    //init arr
    long* arr=mymalloc(sort_len*sizeof(long)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
      arr[i]=(arr[i]<<32)|rand();
      if(rand()%2){
	arr[i]|=1<<31;
      }
    }

    //prep correctness test
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(long));
      fast_bytecopy(corr, arr, sort_len*sizeof(long));
      qsort(corr, sort_len, sizeof(long), longCompare);
    }

    //this is VERY important for
    //performance. I haven't found the best
    //yet I believe (bad long max/poor distribution
    //on slot function is main reason UL sort with
    //prepsort is poor
    unsigned long lMax=RAND_MAX;
    lMax = (lMax << 32) | RAND_MAX;
    lMax|=(1<<31);
    unsigned long lMin=0;

    //still best to use timsort
    sort_funs f;
    f.sort_fun_long=sorter_ul_tim_sort;

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    prepSort(arr, sort_len, sizeof(long), 1,
	     (const void*)(&lMax),(const void*)(&lMin),
	     longVal, longCompare, f);
    takeTime(event_num);
    if(corr_test){
      for(int i=0;i<sort_len;i++){
	assert(arr[i]==corr[i]);
      }
      //NO MEMORY LEAKS!
      free(corr);
    }
    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do str test
  if(doStr){

    //init array
    char** arr=mycalloc(sort_len, sizeof(char*)), **corr = NULL;
    if(corr_test){
      corr = (char**)mycalloc(sort_len,sizeof(char*));
    }
    for(int i =0;i<sort_len;i++){

      //rand le
      int temp_len = rand()%sb_len;
      temp_len=MAX(temp_len, 2);
      arr[i] = (char*)mycalloc(temp_len, sizeof(char));
      if(corr_test){
	corr[i] = (char*)mycalloc(temp_len, sizeof(char));
      }

      //mix upper/lower
      for(int j=0;j<temp_len-1;j++){
	if(rand()%2){
	  ((char*)getPtr(arr[i]))[j]=(rand()%26)+65;
	}else{
	  ((char*)getPtr(arr[i]))[j]=(rand()%26)+97;
	}
      }

      //prep correctness
      if(corr_test){
	fast_bytecopy(corr[i], arr[i], temp_len);
      }

      //high bits for storing len
      highBitsSet((void**)(&arr[i]), MIN(temp_len, 0x7fff));
    }

    //for real string 'zzzz...' is max
    unsigned long strMin=0;
    unsigned long strMax=0;
    unsigned long max_val = 'z';
    for(int i=0;i<MIN(sb_len, 8);i++){
      strMax |= max_val<<(i<<3);
    }

    //sort.h doesn't work on strings
    //so use this timsort function
    sort_funs f;
    f.sort_fun_generic=timsort;

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);

    //the 0 is to say this is not countable (i.e normal
    //bit comparisons wont work for sorting.
    //a good strval function is seriously important.
    prepSort(arr, sort_len, sizeof(char*),0,
	     (const void*)(&strMax),(const void*)(&strMin),
	     strVal, strCompare, f);
    takeTime(event_num);

    //check correctness
    if(corr_test){
      qsort(corr, sort_len, sizeof(char*), strCompare);
      for(int i=0;i<sort_len;i++){
	assert(!strcmp(getPtr((void*)arr[i]),corr[i]));
	//NO MEMORY LEAKS!
	free(corr[i]);
      }

      //NO MEMORY LEAKS!
      free(corr);
    }
    for(int i=0;i<sort_len;i++){
      //NO MEMORY LEAKS!
      free(getPtr((void*)arr[i]));
    }    
    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do byte test
  if(doByte){

    //init random bytes
    unsigned char* arr=mycalloc(sort_len, sb_len), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      for(int j=0;j<sb_len-1;j++){
	arr[(sb_len*i)+j]=(rand()%255);
      }
    }

    //setup correctness test
    if(corr_test){
      corr = mymalloc(sort_len*sb_len);
      fast_bytecopy(corr, arr, sort_len*sb_len);
      qsort(corr, sort_len, sb_len, byteCompare);
    }

    //for byte (as with string)
    //just use first 8 bytes to decide slot (this could
    //be adjusted as needed with different val functions
    //but for most data 8 bytes is enough to make meaningful
    //decisions about value
    unsigned long byteMin=0;
    unsigned long byteMax=0;
    if(sb_len>=8){
      byteMax=~byteMax;
    }
    else{
      byteMax=(1UL<<((sb_len<<3)))-1;
    }

    //again cant use sort.h
    sort_funs f;
    f.sort_fun_generic=timsort;

    
    takeTime(event_num);
    //again non-countable
    prepSort(arr, sort_len, sb_len ,0,
	     (const void*)(&byteMax),(const void*)(&byteMin),
	     byteVal, byteCompare, f);
    takeTime(event_num);

    //ensure correctness
    if(corr_test){
      assert(!memcmp(arr, corr, sort_len*sb_len));
      //NO MEMORY LEAKS!
      free(corr);
    }
    
    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }
}




