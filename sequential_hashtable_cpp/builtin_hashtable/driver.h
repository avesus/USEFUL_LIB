#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <iterator>
#include <map>
#include <unordered_map>
#include "helpers/locks.h"
#include "helpers/temp.h"
#include "helpers/opt.h"
#include "helpers/util.h"
#include "helpers/arg.h"
#include "helpers/bits.h"
#include "helpers/timing.h"
#include "test_config.h"


//////////////////////////////////////////////////////////////////////
//config for type hashtable will use
#ifdef int_test
//int type key
typedef struct ent{
  int key;
  int val;
}ent;
typedef ent node;
#endif

//using cpp builtin no good way to do non str type
#ifdef str_test
//str type key
typedef struct str_node{
  std::string key;
  std::string val;
}str_node;
typedef str_node node;
#endif

void hashtableTest();






