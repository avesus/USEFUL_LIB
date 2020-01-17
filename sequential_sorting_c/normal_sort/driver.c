#include "driver.h"


//////////////////////////////////////////////////////////////////////
//This is a driver for running various sorting functions all found
//in sort.h
//sort.h only accepts types where boolean logic works so
//these functions will not work for sorting strings or byte arrays.
//The library is highly optimized. In this example driver uses
//in place mergesort and quicksort from sort.h
//There are MANY sorting algorithms available.
//build: make clean; make
//to see usage: ./driver -h
#define SUCCESS 0
#define FAILURE -1


int event_num = 0;
int num_events = 0;

//all args in driver.h
#define num_sorts 2
#define num_types 4
#define name_size 32

//for doing temp regulation
#define nthreads 1
#define AllowedTempVariance 1.1
#define timeout 60

//sleep time between runs (in addition to temp
//regulation)
#define stime 1

//multiple sorts from sort.h as well as multiple types
#define all_types ((!!doChar) + (!!doShort) + (!!doInt) + (!!doLong))
#define all_sorts ((!!doMS) + (!!doQS))

int type_bit_mask = 0;
int sort_bit_mask = 0;

#define Version "0.1"

static ArgOption args[] = {
  // Kind, 	  Method,		name,	     reqd,  variable,		 help
  { KindOption,   Integer, 		"-v", 	     0,     &verbose, 		 "Set verbosity level" },
  { KindOption,   Set, 	        	"--csv",     0,     &csv, 		 "Set verbosity level" },
  { KindOption,   Integer,              "--len",     0,     &sort_len,           "Sets power for array size (i.e size= 1<<len)" },
  { KindOption,   Integer,              "--seed",    0,     &rseed,              "Set random seed" },
  { KindOption,   Set,                  "--char",    0,     &doChar,             "do char sort test" },
  { KindOption,   Set,                  "--short",   0,     &doShort,            "do short sort test" },
  { KindOption,   Set,                  "--int",     0,     &doInt,              "do int sort test" },
  { KindOption,   Set,                  "--long",    0,     &doLong,             "do long sort test" },
  { KindOption,   Set,                  "--all",     0,     &doAll,              "do all sort tests" },
  { KindOption,   Set,                  "--qs",      0,     &doQS,               "do quicksort test" },
  { KindOption,   Set,                  "--ms",      0,     &doMS,               "do mergesort test" },
  { KindOption,   Set,                  "--as",      0,     &doAS,               "do all sort functions" },
  { KindOption,   Integer,              "--trials",  0,     &ntrials,            "number of trials to run" },
  { KindOption,   Set,                  "--regtemp", 0,     &regtemp,            "number of trials to run" },
  { KindOption,   Set,                  "--correct", 0,     &corr_test,          "test correctness" },
  { KindHelp,     Help, 	        "-h" },
  { KindEnd }
};
static ArgDefs argp = { args, "sorting", Version, NULL };

//these are for printing in time.h
char purpose_header[name_size] = "Execution Time";
char sort_headers[num_sorts][name_size] = {
  "quicksort",
  "mergesort"
};
char type_headers[num_types][name_size] = {
  "chars",
  "shorts",
  "int",
  "long",
};

//for individualizing to the run (i.e quicksort on char or
//mergesort on int)
char cat_names[num_types*num_sorts][name_size<<1] = {};

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
void setup(){
  //setting up what to run
  if(doAll){
    type_bit_mask = (1<<num_types)-1;
    doChar = 1;
    doShort = 2;
    doInt = 3;
    doLong = 4;
  }
  else{
    doChar *= 1;
    doShort *= 2;
    doInt *= 3;
    doLong *= 4;
    
    type_bit_mask |= (1<<doChar);
    type_bit_mask |= (1<<doShort);
    type_bit_mask |= (1<<doInt);
    type_bit_mask |= (1<<doLong);
    type_bit_mask >>= 1;
  }
  if(doAS){
    sort_bit_mask = (1<<num_sorts)-1;
    doQS = 1;
    doMS = 2;
  }else{
    doQS *= 1;
    doMS *= 2;
    sort_bit_mask |= (1<<doQS);
    sort_bit_mask |= (1<<doMS);
    sort_bit_mask >>= 1;
  }
  num_events = all_sorts*all_types;

  //timing setup
  int * ntimers = (int*)mycalloc(num_events, sizeof(int));
  for(int i=0;i<num_events;i++){
    ntimers[i] = ntrials<<1;
  }

  //i know this kinda sucks but for multiple events/multi
  //timers in each event this is needed
  char*** dif_headers = (char***)mycalloc(num_events, sizeof(char**));
  for(int i=0;i<num_events;i++){
    dif_headers[i] = (char**)mycalloc(1, sizeof(char*));
    dif_headers[i][0] = (char*)purpose_header;
  }
  
  char** ev_headers = (char**)mycalloc(num_events, sizeof(char*));

  //creating event for each sort/type combination
  int sort_index = 0;
  for(int i=0;i<num_sorts;i++){
    if(sort_bit_mask&(1<<i)){
      int type_index = 0;
      for(int j=0;j<num_types;j++){
	if((1<<j)&type_bit_mask){
	  int true_index = sort_index*all_types + type_index;
	  //individualizing strings
	  sprintf(cat_names[true_index], "%s %s", sort_headers[i], type_headers[j]);
	  ev_headers[true_index] = cat_names[true_index];
	  type_index++;
	}
      }
      sort_index++;
    }
  }

  //initializing the actual timing struct
  initTiming(num_events, ntimers, dif_headers, ev_headers, stdout);
  if(regtemp){
    //if regtemp initialize temp enforcer
    if(initTemp(ntrials, nthreads)){
      die("error accessing CPU temp/cores\n");
    }
    setEnforcedTemps(AllowedTempVariance, nthreads);
  }
  
  //NO MEMORY LEAKS!
  free(ntimers);
  //NO MEMORY LEAKS!
  free(dif_headers);
}

void* runner(void* args){
  for(int i=0;i<ntrials;i++){
    if(verbose){
      printf("Trial: %d\n", i);
    }
    if(doQS){
      sortTest_QS();
    }
    if(doMS){
      sortTest_MS();
    }
  }
  return NULL;
}

int main(int argc, char* argv[]){
  progname = argv[0];
  ArgParser* ap = createArgumentParser(&argp);
  int result = parseArguments(ap, argc, argv);
  if(result){
    die("Error parsing arguments");
  }
  //NO MEMORY LEAKS!
  freeCommandLine();
  //NO MEMORY LEAKS!
  freeArgumentParser(ap);
  srand(rseed);
  sort_len = 1 << sort_len;


  //this initializes timing
  setup();

  //this initializes temp regular

  
  pthread_t tid;
  pthread_attr_t attr;
  myset_core(&attr, 0);
  mypthread_create(&tid, &attr, runner, NULL);
  pthread_attr_destroy(&attr);

  //  mypthread_create(&tid, &attr, runner, NULL);
  pthread_join(tid, NULL);

  for(int i=0;i<num_events;i++){
    diffPatternN(i, 2);
  }
  printStats(csv, ms);
  //NO MEMORY LEAKS!
  freeTiming();
  //NO MEMORY LEAKS!
  freeTemp();
  return SUCCESS;
}


//all the sorting stuff is same as other 2
//just cant do byte arr or string (this has correctness
//test just like prepsort)
void sortTest_QS(){
  //byte test
  if(doChar){
    char* arr=mymalloc(sort_len*sizeof(char)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand()&0x7f;
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(char));
      fast_bytecopy(corr, arr, sort_len*sizeof(char));
      qsort(corr, sort_len, sizeof(char), charCompare);
    }
    
    waitTill();
    takeTime(event_num%num_events);
    sorter_ch_quick_sort(arr, sort_len);
    takeTime(event_num%num_events);
    
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
  
  if(doShort){
    short* arr=mymalloc(sort_len*sizeof(short)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand()&0x7fff;
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(short));
      fast_bytecopy(corr, arr, sort_len*sizeof(short));
      qsort(corr, sort_len, sizeof(short), shortCompare);
    }
    
    waitTill();
    takeTime(event_num%num_events);
    sorter_sh_quick_sort(arr, sort_len);
    takeTime(event_num%num_events);
    
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
  
  if(doInt){
    int* arr=mymalloc(sort_len*sizeof(int)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(int));
      fast_bytecopy(corr, arr, sort_len*sizeof(int));
      qsort(corr, sort_len, sizeof(int), intCompare);
    }

    waitTill();
    takeTime(event_num%num_events);
    sorter_int_quick_sort(arr, sort_len);
    takeTime(event_num%num_events);
    
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
  
  if(doLong){
    long* arr=mymalloc(sort_len*sizeof(long)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
      arr[i]=(arr[i]<<32)|rand();
      if(rand()%2){
	arr[i]|=1<<31;
      }
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(long));
      fast_bytecopy(corr, arr, sort_len*sizeof(long));
      qsort(corr, sort_len, sizeof(long), longCompare);
    }
    
    waitTill();
    takeTime(event_num%num_events);
    sorter_ul_quick_sort(arr, sort_len);
    takeTime(event_num%num_events);
    
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
}

void sortTest_MS(){
  if(doChar){
    char* arr=mymalloc(sort_len*sizeof(char)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand()&0x7f;
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(char));
      fast_bytecopy(corr, arr, sort_len*sizeof(char));
      qsort(corr, sort_len, sizeof(char), charCompare);
    }

    waitTill();
    takeTime(event_num%num_events);
    sorter_ch_merge_sort_in_place(arr, sort_len);
    takeTime(event_num%num_events);
    
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
  
  if(doShort){
    short* arr=mymalloc(sort_len*sizeof(short)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand()&0x7fff;
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(short));
      fast_bytecopy(corr, arr, sort_len*sizeof(short));
      qsort(corr, sort_len, sizeof(short), shortCompare);
    }

    waitTill();
    takeTime(event_num%num_events);
    sorter_sh_merge_sort_in_place(arr, sort_len);
    takeTime(event_num%num_events);
    
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
  
  if(doInt){
    
    int* arr=mymalloc(sort_len*sizeof(int)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(int));
      fast_bytecopy(corr, arr, sort_len*sizeof(int));
      qsort(corr, sort_len, sizeof(int), intCompare);
    }
    
    waitTill();
    takeTime(event_num%num_events);
    sorter_int_merge_sort_in_place(arr, sort_len);
    takeTime(event_num%num_events);
    
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
  
  if(doLong){
    long* arr=mymalloc(sort_len*sizeof(long)), *corr = NULL;
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
      arr[i]=(arr[i]<<32)|rand();
      if(rand()%2){
	arr[i]|=1<<31;
      }
    }
    
    if(corr_test){
      corr = mymalloc(sort_len*sizeof(long));
      fast_bytecopy(corr, arr, sort_len*sizeof(long));
      qsort(corr, sort_len, sizeof(long), longCompare);
    }
    
    waitTill();
    takeTime(event_num%num_events);
    sorter_ul_merge_sort_in_place(arr, sort_len);
    takeTime(event_num%num_events);
    
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
}
