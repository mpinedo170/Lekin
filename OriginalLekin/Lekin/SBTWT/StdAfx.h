#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <time.h>

#ifdef V_UNIX
#include <time.h>
#ifndef CLK_TCK
#include <unistd.h>
#define CLK_TCK _SC_CLK_TCK
#endif
#endif
