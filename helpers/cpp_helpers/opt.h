#ifndef _MY_OPT_H_
#define _MY_OPT_H_

#include <stdlib.h>
#include <stdio.h>
#include "config.h"

int fast_bytecmp_u(const void* a, const void* b, size_t len);
int fast_bytecmp(const void* a, const void* b, size_t len);
void fast_bytecopy(void* dst, void* src, size_t len);
void fast_memset(void* loc, size_t val, size_t len);

int ff1_asm(int x);
int ff0_asm(int x);
int fl1_asm(int x);
int fl0_asm(int x);

#endif
