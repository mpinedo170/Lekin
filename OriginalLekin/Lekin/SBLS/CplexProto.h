#ifndef CPLEXPROTO
#define CPLEXPROTO
#include "cpxdefs.inc"

extern "C" CPXLPptr  loadprob (
  char * probname,
  int mac,
  int mar,
  int mae,
  int objsen,
  double *objx,
  double *rhsx,
  char *senx,
  int *matbeg,
  int *matcnt,
  int *matind,
  double *matval,
  double *bdl,
  double *bdu,
  double *rngval,
  int *nrowind,
  int *etype,
  int *enzbeg,
  int *enzcnt,
  int *enzind,
  double *enzval,
  char *dataname,
  char *objname ,
  char *rhsname,
  char *rngname,
  char *bndname,
  char **cname,
  char *cstore,
  char **rname,
  char *rstore,
  char **ename,
  char *estore,
  int macsz,
  int marsz,
  int matsz,
  int maesz,
  int enzsz,
  unsigned cstorsz,
  unsigned rstorsz,
  unsigned estorsz);

extern "C" int  lpread (
  char * probname,
  int *mac,
  int *mar,
  int *objsen,
  double **objx,
  double **rhsx,
  char **senx,
  int **matbeg,
  int **matcnt,
  int **matind,
  double **matval,
  double **bdl,
  double **bdu,
  char **objname ,
  char **rhsname,
  char ***cname,
  char **cstore,
  char ***rname,
  char **rstore,
  int *macsz,
  int *marsz,
  int *matsz,
  unsigned *cstorsz,
  unsigned *rstorsz);

extern "C" CPXLPptr  loadlp (
char * probname,
int cols,
int rows,
int objsen,
double *obj,
double *rhs,
char *sense,
int *matbeg,
int *matcnt,
int *matind,
double *matval,
double *lb,
double *ub,
double *rngval,
int cspace,
int rspace,
int nzspace);

extern "C" int addrows(
CPXLPptr lp,
int ccnt,
int rcnt,
int nzcnt,
double * rhs,
char * sense,
int * rmatbeg,
int * rmatind,
double * rmatval,
char **cname,
char **rname);

extern "C" int dualopt(
CPXLPptr lp);

extern "C" int optimize(
CPXLPptr lp);

extern "C" int lpwrite(
CPXLPptr lp,
char *filename);

extern "C" int getobjval(
CPXLPptr lp,
double * pobjval);

extern "C" int getx(
CPXLPptr lp,
double * x,
int begin,
int end);

extern "C" int setlogfile(
FILE * logfile);

extern "C" int setscr_ind(
int number);

extern "C" int getmar(
CPXLPptr lp);

extern "C" int getstat(
CPXLPptr lp);

extern "C" int getitc(
CPXLPptr lp);

extern "C" int mipoptimize(
CPXLPptr lp);

extern "C" int loadctype(
CPXLPptr lp,
char * ctype);

extern "C" int getmobjval(
CPXLPptr lp,
double * pobjval);

extern "C" int getmx(
CPXLPptr lp,
double * x,
int begin,
int end);

extern "C" int loadorder(
CPXLPptr lp,
int cnt,
int *colindex,
int *priority,
int *direction);

extern "C" void freeprob(
CPXLPptr *lp);

extern "C" int setitlim(
int newvalue,
int *ptoosmall,
int *ptoobig);

extern "C" int getpi(
CPXLPptr lp,
double *pi,
int begin,
int end);

#endif
