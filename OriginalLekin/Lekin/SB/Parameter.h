/* Parametr.h

   Definitions for general parameters.
*/

#pragma once

//***** Location of other header files *****//

#include "Stack.h"

#undef DEBUG
#define DEBUG 1

typedef int TOperationID;
typedef int TJobID;
typedef int TMachineID;
typedef int TWorkcenterID;
typedef int TNodeID;
typedef TCHAR TStatus;

const TStatus sDefault = 'a';
const double fBigM = 999999;
const double fSmall = 0.00001;
const double pi = 3.1415926535;
const int iMaxJob = 200;
const int iMaxMachine = 1000;

const TCHAR szFileExtJob[] = _T(".job");
const TCHAR szFileExtWkc[] = _T(".wkt");
const TCHAR szFileExtSeq[] = _T(".seq");
const TCHAR szFileExtAvl[] = _T(".avl");
const TCHAR szFileDefault[] = _T("data");

/***** global functions *****
 - dtoXa(sx,d1,i1,i2) : convert d1(double) to TCHAR with size i1 and i2 digits.
*/
int equal(double f1, double f2);
void InitRandom();
double Random();
int Random(int x);
int Random(int iFrom, int iTo);
double Random(double fFrom, double fTo);
double Random(double fFrom, double fTo, int idigit);
void ltoa(long lx, LPSTR szTemp);
void mIDtoa(LPSTR szInt5, TMachineID mID, int iN);
void jIDtoa(LPSTR szInt5, TJobID mID, int iN);
void oIDtoa(LPSTR szInt5, TOperationID mID, int iN);
void wIDtoa(LPSTR szInt5, TWorkcenterID wID, int iN);
void dtoXa(LPSTR szTemp, double ll, int nn, int dd);
int strcount(LPTSTR st, TCHAR lookfor);
void strextc(LPTSTR result, LPTSTR st, TCHAR lookfor);
int strextc_int(LPTSTR st, TCHAR lookfor);
TJobID strextc_JobID(LPTSTR st, TCHAR lookfor);
TMachineID strextc_MachineID(LPTSTR st, TCHAR lookfor);
TOperationID strextc_OperationID(LPTSTR st, TCHAR lookfor);
CString std_filename(LPCTSTR szName, LPCTSTR szExt);  // add szExt to szName if no extension
void roundoff(double& fX, int iDigit);
void wait();

class TStopWatch
{
private:
    time_t tStart;
    time_t tComplete;

public:
    void Start()
    {
        time(&tStart);
    }
    void Stop()
    {
        time(&tComplete);
    }
    int Time()
    {
        return (int)(tComplete - tStart);
    }
};

class TTool;
class TShop;
class TMachine;
class TJob;
class TOperation;
class TMcAvail;
class TDGraph;
class TSequence;
class TRuleReturn;
class TNode;

typedef double (*TObjFunc)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight);
typedef double (*TSeqFunc)(TTool& WkTool);
typedef void (*TOptFunc)(TShop& Shop, TStackP& WkcSchedList, TSeqFunc OptSeq, TObjFunc Obj);
typedef TRuleReturn* (*TIndexFunc)(
    TShop* pShop, TMcAvail& MCAvail, TJob& jbX, TDGraph& Graph, double fK1, double fK2);
