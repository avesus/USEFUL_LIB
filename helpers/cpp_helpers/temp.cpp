#include "temp.h"


extern int verbose;

#define VERSION "0.1"

#define PATHLEN 256
// path to directory with core files
static  char temp_path[PATHLEN]="/sys/devices/platform/coretemp.0/";
// the first number used to find the file with a core
static  int core_offset=0;	
// number of physical cores in system
static  int num_cores=0;

//get num cores (called in initTemp)
static int getCores(void);

//set path for temp_path (called in initTemp)
static int setPath(void);


void
printNdouble(FILE* f, const char* prompt, int n, double* ds) {
  fputs(prompt, f);
  for (int i=0; i<n; i++) fprintf(f, "\t%lf", ds[i]);
  fputs("\n", f);
}

//initializes variables. Gets number of cores, sets variables for finding tempature files
//and mallocs arrays for storing information
int
initTemp(int trials, int num_threads) {
  if (!getCores()) {
    printf("Couldnt find cores on your machine\n");
    return -1;
  }
  if (setPath()) {
    return -1;
    }
  return 0;
}

// set number of physical cores (not hyperthreading count).  
// Returns 1 on success (and sets `num_cores`), 0 on failure
//
// Depends on `/proc/cpuinfo` to be available
// cant use number of processors because hyperthread (on my machine at
// least 8 processors 4 cores) also seems like processors 4-7 are on
// core procNum-4 so only first 4 threads will need to monitor core
// temp.
static int 
getCores(void) {
  FILE* fp= myfopen("/proc/cpuinfo","r");
  char buf[32]="";
  while (fgets(buf, 32,fp)) {
    if (!strncmp(buf,"cpu cores",9)) {
      num_cores=atoi(buf+12);
      fclose(fp);
      return 1;
    }
  }
  fclose(fp);
  return 0;
}


//Finds the files that contain temp information for each core (does
//this by finding the path starting at
///sys/devices/platform/coretemp.0/ then trying to find hwmon* until
//its reached reach dir that containts temp*_type files. Then finds
//the temp*_type files that correspond to cores and ensures it can
//find all cores for later. Will print error and return -1 if it cant
//find the temperature file for each core (0 returned on success)
//there is a goto in there so you might want to delete the entire
//file?

static int 
setPath(void) {
  DIR* d=NULL;
  int cores=0;
  int success=0;
  struct dirent *dir;
 startDir:;
  //open dir and read its contents
  d=opendir(temp_path);
  if (d) {
    while ((dir=readdir(d)) != NULL) {
      // check to see if hwmon is in this directory
      if (!strncmp("hwmon",dir->d_name,5)) {
	strcat(temp_path,dir->d_name);
	strcat(temp_path,"/");
	closedir(d);

	//SORRY
	goto startDir;
      }
    }
    closedir(d);
  }else{
    die("path error finding temp files\n");
  }


  //once through all the hwmon* get offset into temp*_label files
  //basically temp0_label does not exist on my computer and
  //temp1_label is not related to a core starts on my comp for
  //temp2_label (might be different on yours) so I read through
  //temp%d_label until matches with Core* then continue reading until
  //file does not exist (i.e on 4 core machine would read until
  //temp5_label. Once file does not exist but after succesful reads
  //(so could start at temp50_label) will exist while(1) loop.
  char checkPath[PATHLEN];
  int max=0;
  while (1) {
    sprintf(checkPath,"%stemp%d_label",temp_path,max);
    if (verbose) {
      printf("Checking: %s\n", checkPath);
    }
    //check if file exists
    if (access(checkPath,F_OK)!=-1) {
      success=1;
      FILE* fp=myfopen(checkPath,"r");
	char buf[32]="";
	if (fgets(buf,32,fp)!=NULL) {
	  if (!strncmp(buf,"Core",4)) {
	    cores+=1<<atoi(buf+4);
	  }
	}
	fclose(fp);
    }
    else{
      //core offset incremented so later when trying to do core 3 can
      //do threadNum%num_cores+core_offset as %d in temp%d_input
      core_offset++;

      //success is set to 1 after the temp%d_label file corresponding
      //to a core was found next time a file does not exist will break
      //from while(1)
      if (success) {
	break;
      }
    }
    //If didnt find as many
    //files as cores on machine will return -1. Returns 0 on success.
    max++;

  }
  if (cores != ((1<<num_cores)-1)) {
    printf("Couldnt find all cores %x vs %x\n", cores,((1<<num_cores)-1));
    return -1;
  }
  return 0;
}

double
getTempFromCore(int coreid) {
  char path[PATHLEN];
  double temp = 0;
  
  sprintf(path,"%s/temp%d_input", temp_path, coreid);
  FILE* fp=myfopen(path, "r");
    int x = fscanf(fp,"%lf", &temp);
    if (x != 1) {
      fprintf(stderr,"Failed to read temperature from [%s]\n", path);
    }
    fclose(fp);
  return temp/1000.0;
}

//reads temps for cores associated with `num_threads` threads starting at
//thread `index`.  store result in dest[index]
void 
doTemps(int index, double* dest, int num_threads) {
  //fprintf(stderr, "doTemp(%d,%p,%d)\n", index, dest, num_threads);
  int iStart=index;
  int iEnd=index+num_threads;
  long long int gotit = 0;
  /* if running on more threads need better way to avoid duplicate queries */
  assert(num_threads <= (8*sizeof(unsigned long long)));
  
  for (int i = iStart; i < iEnd; i++) {
    int coreIndex = i%num_cores;
    double temp = getTempFromCore(coreIndex+core_offset);
    dest[i] = temp;
  }
}


extern int nthreads;


static double* enforcedTemps=NULL;
static double* nowTemps=NULL;

static double deltaEnforcedTemp;

static void
getTemps(double* buffer, int nthreads) {
  unsigned long long gotit = 0;
  /* if running on more threads need better way to avoid duplicate queries */
  assert(nthreads <= (8*sizeof(unsigned long long))); 

  for (int i=0; i<nthreads; i++) {
    int idx = i%num_cores;
    if (gotit & (1 << idx)) continue;
    gotit |= (1 << idx);
    buffer[i] = getTempFromCore(idx+core_offset);
  }
}

void
setEnforcedTemps(double delta, int nthreads) 
{
  enforcedTemps = (double*)mycalloc(nthreads, sizeof(double));
  nowTemps = (double*)mycalloc(nthreads, sizeof(double));
  deltaEnforcedTemp = delta;
  getTemps(enforcedTemps, nthreads);
  if(verbose){
    printNdouble(stderr, "Enforcing temp:", nthreads, enforcedTemps);
  }
}

void freeTemp(){
  if(enforcedTemps){
    free(enforcedTemps);
  }
  if(nowTemps){
    free(nowTemps);
  }
}

#define MaxSleepBeforeExit 1000

static int showenforcedtemps = 1;

// stays in while (1) with a sleep(1) inside until the current
// temperature is less than 1.1*StartingTemp for a given core.  Wait a
// maximum of `maxWait` seconds before terminating program
void 
enforceTemps(int num_threads, int maxWait) 
{
  int loopNum=maxWait;
  while (1) {
    int cont=0;
    if (loopNum > 0) {
      //sleep here to let cool off between loops, can adjust this to
      //fit your needs
      sleep(1);
    }
    // get current temperatures
    getTemps(nowTemps, num_threads);
    for (int t=0; t<num_threads; t++) {
      int i = t%num_cores;
      if (verbose) {
	printf("%d: core %d -> start %f vs cur %f\n", loopNum, i, enforcedTemps[i], nowTemps[i]);
      }
      //compare, if cont is set will redo loop, else will break
      if (nowTemps[i] > (deltaEnforcedTemp * enforcedTemps[i])) {
	cont=1;
      }
    }
    loopNum--;
    if (!cont) {
      // all cores are cool enough

      char buffer[64];
      if(verbose){
	sprintf(buffer, "After enforcing temp(%7.3lf):", (double)(maxWait - loopNum));
	printNdouble(stderr, buffer, num_threads, nowTemps);
      }
      break;
    }
    if (loopNum <= 0) {
      fprintf(stderr, 
	      "Exceeded %d loops without reaching cool down temperature\n", maxWait);
      for (int i=0; i<num_cores; i++) {
	fprintf(stderr, "\tcore %d: %lf isn't lower than %lf*%lf\n", i, nowTemps[i], deltaEnforcedTemp, enforcedTemps[i]);
      }
      exit(-1);
    }
  }
}
