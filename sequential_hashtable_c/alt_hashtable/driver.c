#include "driver.h"


#define SUCCESS 0
#define FAILURE -1


int verbose = 0;
int rseed = 0;
int init_size = 5;
int inserts = 20, queries = 5;
int corr_test = 0;
int regtemp = 0;
int ntrials = 1;
int str_len = 1;
int csv = 0;
int min_key = 32;
int min_val = 32;


//this is important for temp regulation library
#define nthreads 1
#define AllowedTempVariance 1.1
#define timeout 60

#define stime 1

#define Version "0.1"

static ArgOption args[] = {
  // Kind, 	  Method,		name,	     reqd,  variable,		help
  { KindOption,   Integer, 		"-v", 	     0,     &verbose, 		"Set verbosity level" },
  { KindOption,   Integer, 		"--isize",   0,     &init_size,		"Initial size for hashtable (as exponent)" },
  { KindOption,   Integer, 		"--inserts", 0,     &inserts, 		"Sets number of inserts (as exponent)" },
  { KindOption,   Integer, 		"--queries", 0,     &queries, 		"Sets number of queries per insert (as exponent)" },
  { KindOption,   Integer, 		"--seed",    0,     &rseed, 		"random seed" },
  { KindOption,   Integer, 		"--trials",  0,     &ntrials, 		"num trials" },
  { KindOption,   Integer, 		"--minkey",  0,     &min_key, 		"strlen bound (will be rand()%str_len+min_key/val" },
  { KindOption,   Integer, 		"--minval",  0,     &min_val, 		"min key len" },
  { KindOption,   Integer, 		"--strlen",  0,     &str_len, 		"min val len" },
  { KindOption,   Set,  		"--regtemp", 0,     &regtemp, 		"turn on temp regulation" },
  { KindOption,   Set,  		"--correct", 0,     &corr_test,		"turn on correctness test" },
  { KindOption,   Set,  		"--csv",     0,     &csv,		"turn on correctness test" },

  { KindHelp,     Help, 	"-h" },
  { KindEnd }
};
static ArgDefs argp = { args, "hashtable driver", Version, NULL };

#define name_size 32
char purpose_header[name_size] = {"Execution Time"};
char table_header[name_size] = {"Hashtable Alt"};


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
    hashtableTest();
  }
  return NULL;
}



//setup for timing
void setup(){
  //timing setup
  int * ntimers = (int*)mycalloc(1, sizeof(int));
  ntimers[0] = ntrials<<1;

  //i know this kinda sucks but for multiple events/multi
  //timers in each event this is needed
  char*** dif_headers = (char***)mycalloc(1, sizeof(char**));
  dif_headers[0] = (char**)mycalloc(1, sizeof(char*));
  dif_headers[0][0] = (char*)purpose_header;
  
  char** ev_headers = (char**)mycalloc(1, sizeof(char*));
  ev_headers[0] = (char*)table_header;

  //initializing the actual timing struct
  initTiming(1, ntimers, dif_headers, ev_headers, stdout);
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
  progname = argv[0];
  ArgParser* ap = createArgumentParser(&argp);
  int result = parseArguments(ap, argc, argv);
  if(result){
    die("Error parsing arguments");
  }
  freeCommandLine();
  freeArgumentParser(ap);

  inserts = 1<<inserts;
  if(queries){
    queries = 1<<queries;
  }
  srand(rseed);
  assert(sizeof(node_block)==cache_line_size);
  
  setup();

  pthread_t tid;
  pthread_attr_t attr;
  myset_core(&attr, 0);
  mypthread_create(&tid, &attr, runner, NULL);
  pthread_attr_destroy(&attr);
  pthread_join(tid, NULL);
  
  diffPatternN(0, 2);
  printStats(csv, ms);

  //NO MEMORY LEAKS!
  freeTiming();
  freeTemp();

  return SUCCESS;
}


char* setStr(int bound){
  char* s=calloc(bound+1, sizeof(char));
  for(int i =0;i<bound;i++){
    s[i]=(rand()%26)+65;
  }
  return s;
}


void hashtableTest(){
  hashTable* table=initTable(init_size);
  node* new_node = calloc(inserts, sizeof(node));
  int* q_index = calloc(inserts*queries, sizeof(int));
  for(int i =0;i<inserts;i++){
#ifdef int_test
    new_node[i].key=rand();
    new_node[i].val=rand();
#endif
#ifdef str_test
    int len = (rand()%str_len)+min_key;
    new_node[i].key=setStr(len);
    new_node[i].key_len = len;
    
    len = (rand()%str_len)+min_val;
    new_node[i].val=setStr(len);
    new_node[i].val_len = len;
#endif
#ifdef str_hblen_test
    int len = (rand()%str_len)+min_key;
    new_node[i].key=setStr(len);
    setKeyLen(new_node[i].key, len);

    len = (rand()%str_len)+min_val;
    new_node[i].val=setStr(len);
#endif
#ifdef str_nolen_test
    int len = (rand()%str_len)+min_key;
    new_node[i].key=setStr(len);
    
    len = (rand()%str_len)+min_val;
    new_node[i].val=setStr(len);
#endif
  }
  
  for(int i=0;i<inserts*queries;i++){
    q_index[i]=rand()%inserts;
  }
  
  if(corr_test){
    if(verbose){
      fprintf(stderr,"Correctness testing\n");
    }
    takeTime(0);
    for(int i=0;i<inserts;i++){
      addNode(table, new_node[i]);
      int q_min = i*queries;
      int q_max = (i+1)*queries;
      for(int j=q_min;j<q_max;j++){
	findNode(table, new_node[q_index[j]]);
      }
      assert(findNode(table, new_node[i]));
      assert(deleteNode(table, new_node[i]));
      assert(!findNode(table, new_node[i]));
      addNode(table, new_node[i]);
    }
    takeTime(0);
  }
  else{
    waitTill();
    takeTime(0);
    for(int i=0;i<inserts;i++){
      addNode(table, new_node[i]);
      int q_min = i*queries;
      int q_max = (i+1)*queries;
      for(int j=q_min;j<q_max;j++){
	findNode(table, new_node[q_index[j]]);
      }
    }
    takeTime(0);
  }
#if defined(str_test) || defined(str_nolen_test) || defined(str_hblen_test)
  for(int i=0;i<inserts;i++){
    free(getPtr(new_node[i].key));
    free(getPtr(new_node[i].val));
  }
#endif
  free(new_node);
  free(q_index);
  freeTable(table);
}
