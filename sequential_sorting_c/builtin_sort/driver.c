#include "driver.h"

//////////////////////////////////////////////////////////////////////
//This is simply a driver for qsort native to math.h
//this exists simply as a benchmark to compare the performance of
//prepsort.c and sort.h against. Both the drivers for prepsort
//and sort.h have a --correct argument which will test that the
//sorts were performed correctly, so again this is just to get a
//feel for the performance of the other libraries.
#define SUCCESS 0
#define FAILURE -1


//this is use for properly using timing.c/.h
#define num_sorts 1
#define num_types 6
#define name_size 32
#define all_types ((!!doChar) + (!!doShort) + (!!doInt) + (!!doLong) + (!!doStr) + (!!doByte))

//this is important for temp regulation library
#define nthreads 1
#define AllowedTempVariance 1.1
#define timeout 60

//sleep after intializing data. This ensures each run
//is done on an empty cache
#define stime 1




#ifndef MAX
#define MAX(x,y) (((x) > (y) ? (x) : (y)))
#endif

#ifndef MIN
#define MIN(x,y) (((x) < (y) ? (x) : (y)))
#endif

#define Version "0.1"

//all args in driver.h
static ArgOption args[] = {
  // Kind, 	  Method,		name,	     reqd,  variable,  		 help
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
  { KindHelp,     Help, 	        "-h" },
  { KindEnd }
};
static ArgDefs argp = { args, "builtin quicksort", Version, NULL };

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
  if(regtemp){
    enforceTemps(nthreads, timeout);
  }
  sleep(stime); //this is to ensure no hot cache
}

//where whatever function(s) being tested should be called from
void* runner(void* args){
  for(int i=0;i<ntrials;i++){
    if(verbose){
      printf("Trial: %d\n", i);
    }
    //code that calls relevant function(s)
    sortTest();
  }
  return NULL;
}

//setup for timing
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

  
  if(verbose){
    printf("starting setup\n");
  }
  setup();
  if(verbose){
    printf("completed setup\n");
  }

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

void sortTest(){
  int event_num = 0;

  //do char test
  if(doChar){

    //init array to be sorted
    char* arr=mymalloc(sort_len*sizeof(char));
    for(int i =0;i<sort_len;i++){
      //char max
      arr[i]=rand()&0x7f;
    }

    //sleep to flush cache/reg temp
    waitTill();
    takeTime(event_num);
    qsort(arr, sort_len, sizeof(char), charCompare);
    takeTime(event_num);

    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do short test
  if(doShort){

    //init array
    short* arr=mymalloc(sort_len*sizeof(short));
    for(int i =0;i<sort_len;i++){
      arr[i]=rand()&0x7fff;
    }

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    qsort(arr, sort_len, sizeof(short), shortCompare);
    takeTime(event_num);

    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do int test
  if(doInt){

    //init array
    int* arr=mymalloc(sort_len*sizeof(int));
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
    }

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    qsort(arr, sort_len, sizeof(int), intCompare);
    takeTime(event_num);

    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do long test
  if(doLong){
    //init array
    long* arr=mymalloc(sort_len*sizeof(long));
    for(int i =0;i<sort_len;i++){
      arr[i]=rand();
      arr[i]=(arr[i]<<32)|rand();
      if(rand()%2){
	//do this since RAND_MAX is signed INT max
	arr[i]|=1<<31;
      }
    }

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    qsort(arr, sort_len, sizeof(long), longCompare);
    takeTime(event_num);

    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }

  //do str test
  if(doStr){

    //init array
    char** arr=mycalloc(sort_len, sizeof(char*));
    for(int i =0;i<sort_len;i++){
      int temp_len = rand()%sb_len;
      temp_len=MAX(temp_len, 2);
      arr[i] = (char*)mycalloc(temp_len, sizeof(char));

      //variable length strings/upper/lower case mixed
      for(int j=0;j<temp_len-1;j++){
	if(rand()%2){
	  //upper case
	  ((char*)getPtr(arr[i]))[j]=(rand()%26)+65;
	}else{
	  //lower case
	  ((char*)getPtr(arr[i]))[j]=(rand()%26)+97;
	}
      }
      
      //str len is important for val function
      highBitsSet((void**)(&arr[i]), MIN(temp_len, 0x7fff));
    }

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    qsort(arr, sort_len, sizeof(char*), strCompare);
    takeTime(event_num);

    //NO MEMORY LEAKS!
    for(int i=0;i<sort_len;i++){
      free(getPtr((void*)arr[i]));
    }    
    free(arr);
    event_num++;
  }

  //do byte test
  if(doByte){

    //init array
    unsigned char* arr=mycalloc(sort_len, sb_len);
    for(int i =0;i<sort_len;i++){
      for(int j=0;j<sb_len-1;j++){
	//random bytes len sb_len
	arr[(sb_len*i)+j]=(rand()%255);
      }
    }

    //flush cache/reg temp
    waitTill();
    takeTime(event_num);
    qsort(arr, sort_len, sb_len, byteCompare);
    takeTime(event_num);

    //NO MEMORY LEAKS!
    free(arr);
    event_num++;
  }
}



