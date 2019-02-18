// tools.h, 159

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include "types.h" // need definition of 'q_t' below

//Phase 1
void MyBzero(char *, int);
int DeQ(q_t *);
void EnQ(int, q_t *);
int MyStrlen(char *p);
void MyStrcat(char *dst, char *addendum);
int MyStrcmp(char *p, char *q, int len);
void MyStrcpy(char *dst, char *src);
void MyMemcpy(char *dst, char *src, int size);

#endif

