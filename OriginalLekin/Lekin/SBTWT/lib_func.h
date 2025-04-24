#ifndef LIB_FUNC_H
#define LIB_FUNC_H

#include "const.h"

// Augusto 13.8.96
// extern int max(int,int);
extern int max2(int, int);
extern int pos(int);
extern double pos(double);

// Augusto 13.8.96
// extern int min(int,int);
// extern int min(int,int,int);
// extern int min(int,int,int,int);
extern int min2(int, int);
extern int min2(int, int, int);
extern int min2(int, int, int, int);

extern FILE* open_file(char*, char*);

#endif
