#include "driver.h"


#define SUCCESS 0
#define FAILURE -1



typedef struct thread_args{
  pthread_barrier_t* start;
  pthread_barrier_t* end;
  int tid;
}thread_args;



int verbose = 0;
int rseed = 0;
int init_size = 20;
int inserts = 20, queries = 5;
int corr_test = 0;
int regtemp = 0;
int ntrials = 1;
int str_len = 1;
int csv = 0;
int min_key = 32;
int min_val = 32;
int nthreads = 1;


//this is important for temp regulation library

#define AllowedTempVariance 1.1
#define timeout 60

#define stime 1

#define Version "0.1"

static ArgOption args[] = {
  // Kind, 	  Method,		name,	     reqd,  variable,		help
  { KindOption,   Integer, 		"-v", 	     0,     &verbose, 		"Set verbosity level" },
  { KindOption,   Integer, 		"-t", 	     0,     &nthreads, 		"Set number of threads" },
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


//global for testing the hashtable
hashTable* global_table;
node* global_nodes;
int* global_queries;

//num cores on machines
#define num_cores sysconf(_SC_NPROCESSORS_ONLN)

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
  thread_args* targs = (thread_args*)args;
  int tid = targs->tid;
  pthread_barrier_t* start = targs->start;
  pthread_barrier_t* end = targs->end;
  free(args);
  for(int i=0;i<ntrials;i++){
    if(verbose){
      printf("Trial[%d]: %d\n", tid, i);
    }
    if(!tid){
      initTestVars();
    }
    waitTill();
    timingBarrierWait(start, 0, tid);
    hashtableTest(global_table,
		  global_nodes+(inserts*tid),
		  global_queries+(queries*inserts*tid),
		  tid);
    timingBarrierWait(end, 0, tid);
    if(!tid){
      freeTestVars();
    }
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
  if(max_threads<nthreads){
    die("To many threads: %d/%d\n", nthreads, max_threads);
  }
  inserts = 1<<inserts;
  if(queries){
    queries = 1<<queries;
  }
  srand(rseed);
  assert(sizeof(node_block)==cache_line_size);
  
  setup();

  pthread_barrier_t start, end;
  timingBarrierInit(&start, nthreads);
  timingBarrierInit(&end, nthreads);
  pthread_t* tid = (pthread_t*)calloc(nthreads, sizeof(pthread_t));
  for(int i=0;i<nthreads;i++){
    pthread_attr_t attr;
    thread_args* targs = (thread_args*)calloc(1, sizeof(thread_args));
    targs->tid = i;
    targs->start = &start;
    targs->end = &end;
    myset_core(&attr, i%num_cores);
    mypthread_create(&tid[i], &attr, runner, targs);
    pthread_attr_destroy(&attr);
  }
  
  for(int i=0;i<nthreads;i++){
    pthread_join(tid[i], NULL);
  }
  myFree(tid);
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




void hashtableTest(hashTable* table,
		   node* new_node,
		   int* q_index,
		   int tid){
  assert(table);
  assert(new_node);
  assert(q_index);
  if(corr_test){
    if(verbose){
      fprintf(stderr,"Correctness testing\n");
    }
    for(int i=0;i<inserts;i++){
      addNode(table, new_node[i], tid);
      int q_min = i*queries;
      int q_max = (i+1)*queries;
      for(int j=q_min;j<q_max;j++){
	findNode(table, new_node[q_index[j]], tid);
      }
      assert(findNode(table, new_node[i], tid));
      assert(deleteNode(table, new_node[i], tid));
      assert(!findNode(table, new_node[i], tid));
      addNode(table, new_node[i], tid);
    }
  }
  else{
    for(int i=0;i<inserts;i++){
      addNode(table, new_node[i], tid);
      int q_min = i*queries;
      int q_max = (i+1)*queries;
      for(int j=q_min;j<q_max;j++){
	findNode(table, new_node[q_index[j]], tid);
      }
    }
  }
}


void initTestVars(){
  global_table=initTable(init_size);
  global_nodes = calloc(inserts*nthreads, sizeof(node));
  global_queries = calloc(inserts*queries*nthreads, sizeof(int));
  for(int i =0;i<inserts*nthreads;i++){
#ifdef int_test
    global_nodes[i].key=rand();
    global_nodes[i].val=rand();
#endif
#ifdef str_test
    int len = (rand()%str_len)+min_key;
    global_nodes[i].key=setStr(len);
    global_nodes[i].key_len = len;
    
    len = (rand()%str_len)+min_val;
    global_nodes[i].val=setStr(len);
    global_nodes[i].val_len = len;
#endif
#ifdef str_hblen_test
    int len = (rand()%str_len)+min_key;
    global_nodes[i].key=setStr(len);
    setKeyLen(global_nodes[i].key, len);

    len = (rand()%str_len)+min_val;
    global_nodes[i].val=setStr(len);
#endif
#ifdef str_nolen_test
    int len = (rand()%str_len)+min_key;
    global_nodes[i].key=setStr(len);
    
    len = (rand()%str_len)+min_val;
    global_nodes[i].val=setStr(len);
#endif
  }
  
  for(int i=0;i<inserts*queries*nthreads;i++){
    global_queries[i]=rand()%inserts;
  }
}

void freeTestVars(){
#if defined(str_test) || defined(str_nolen_test) || defined(str_hblen_test)
  for(int i=0;i<inserts*nthreads;i++){
    myFree(highBitsGetPtr(global_nodes[i].key));
    myFree(highBitsGetPtr(global_nodes[i].val));
  }
#endif
  myFree(global_nodes);
  myFree(global_queries);
  freeTable(global_table);
}
