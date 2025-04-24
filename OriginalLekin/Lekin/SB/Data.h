/*  Data.h   February 17, 1996          last modification: 3/5/96

    Data management classes for scheduling heuristic development.
*/
#pragma once

#include "Parameter.h"
#include "Stack.h"  // general stack classes

class TMachine;
class TJob;
/* TOperation class

   This class contain basic info of each operation. There are 3 lists -- 2 for
   precedent relations (prior/post operations) & machine that able to process
   this operation. The prior/post operation list contain ptr to operation
   while the machine list contains ptr to machine. Thus, all the operation and
   machines should be created before adding to the list.

   sMC_status represents the machine status when processing this operation.
   The setup time is determined from the current machine status and the status
   needed to process the next operation. The default value is set to 'a'.

   fStartTime and fSetupTime will be determined from the sequence.
   The completion time = fStartTime+fSetupTime+fProcessTime

   Commands
   ========
   TOperation(id,p,r,d,w,description)
 - AddPrior(px)     : add prior operation
 - AddPost(px)      : add post operation
 p GetPrior(x)      : return ptr to the xth prior operation
 p GetPost(x)       : return ptr to the xth post operaton
 b RemovePrior(px)  : remove the px prior operation.
 b RemovePost(px)   : remove the px post operation.
 - AddMachine(px)   : add machine that able to process this operation
 p GetMachine(x)    : return ptr to the xth machine that able to process this
                      operation.
 b RemoveMachine(px): remove the px machine.
 - SetStatus(ch)    : set the machine status (used for setup time determination.
 - Clear()          : clear the stacks -- stkPriorOp, stkPostOp, stkMachine.

   Note: Return types: - = void, b = boolean(0/1), p = pointer, i = integer,
                       f = double, x = others
*/
class TOperation
{
    friend class TJob;

public:
    TOperationID oID;
    TStatus sMC_Status;
    TCHAR szDescription[30];

    double fProcessTime;  // standard processing time on avg machine
    double fReleaseTime;
    double fDueDate;
    double fWeight;
    double fEWeight;         // earliness weight
    double fStartTime;       // determined by sequence; does not include setup time
    double fFinishTime;      // determined by sequence; may have space between fStartTime and fFinishTime
    double fSetupTime;       // determined by sequence; occur before fStartTime
    double fActProcessTime;  // determined by sequence; may differ from fProcessTime due to machine speed

    TMachineID mID;  // determined by sequence; machine ID that do the processing
    TStackP stkPriorOp;
    TStackP stkPostOp;
    TStackP stkMachine;

    TJob* pJob;  // point back to job that contain this operation

    ///// functions /////
    TOperation();
    TOperation(TOperationID _oID,
        double _fProcessTime,
        double _fReleaseTime = 0,
        double _fDueDate = fBigM,
        double _fWeight = 1,
        double _fEWeight = 0,
        LPCTSTR _szDescription = _T(""));
    TOperation(TOperation& opX);
    TOperation& operator=(TOperation& opX);

    int IsInMCStack(int _mID);
    void AddPrior(TOperation* popPrior)
    {
        if (!stkPriorOp.Member(popPrior)) stkPriorOp.Push(popPrior);
    }
    void AddPost(TOperation* popPost)
    {
        if (!stkPostOp.Member(popPost)) stkPostOp.Push(popPost);
    }
    TOperation* GetPrior(int iLocation)
    {
        return iLocation <= stkPriorOp.Num() ? (TOperation*)stkPriorOp[iLocation] : NULL;
    }
    TOperation* GetPost(int iLocation)
    {
        return iLocation <= stkPostOp.Num() ? (TOperation*)stkPostOp[iLocation] : NULL;
    }
    int RemovePrior(TOperation* popX)
    {
        return stkPriorOp.Locate(popX) ? stkPriorOp.Delete() : 0;
    }
    int RemovePost(TOperation* popX)
    {
        return stkPostOp.Locate(popX) ? stkPostOp.Delete() : 0;
    }
    void AddMachine(TMachine* pmcX)
    {
        if (!stkMachine.Member(pmcX)) stkMachine.Push(pmcX);
    }
    TMachine* GetMachine(int iLocation)
    {
        return iLocation <= stkMachine.Num() ? (TMachine*)stkMachine[iLocation] : NULL;
    }
    int RemoveMachine(TMachine* pmcX)
    {
        return stkMachine.Locate(pmcX) ? stkMachine.Delete() : 0;
    }
    void SetStatus(TStatus sX)
    {
        sMC_Status = sX;
    }
    void Clear();
};

/* TJob class

   Job is a list of operations with precedent relationship.
   Commands
   ========
   TJob(id,release,duedate,weight,description)
 x [i]            : return the ith operation.
 p Append(xx)     : add a new operation (allocate memory) & return ptr to
                    the new operation (return 0 if duplicate ID).
 p <<xx           : Append(xx).
 - Clear()        : remove all the operations.
 b Link(x,y)      : add precedent relation; x->y
 b DLink(x,y)     : remove precedent relation between x-y
 p Get(x)         : return ptr to the xth operation
 p GetID(x)       : return ptr to operation that has ID = x, 0 if not found
 b Remove(px)     : remove px operation from the job.
 b Remove(x)      : remove the xth operation.
     == See TStackP class for other commands ==
     == DO NOT use Push(xx),Pop(),PopFirst(),Insert(x,xx),Delete(x) ==
     == Due to the lacking of memory allocation.                    ==
*/
class TJob
{
private:
    TStackP operations;

public:
    TJobID jID;
    TCHAR szDescription[30];
    double fReleaseTime;
    double fDueDate;
    double fWeight;
    double fEWeight;

    TJob(TJobID _jID = 0,
        double _fRelease = 0,
        double _fDueDate = fBigM,
        double _fWeight = 1,
        double _fEWeight = 0,
        LPCTSTR _szDescription = _T(""));
    TJob(TJob& jbX);
    ~TJob()
    {
        Clear();
    }
    virtual void Clear();
    virtual TJob& operator=(TJob& jbX);
    virtual TOperation* Append(TOperation& opX);
    TOperation* operator<<(TOperation& opX)
    {
        return Append(opX);
    }

    int Num()
    {
        return operations.Num();
    }
    void Head()
    {
        operations.Head();
    }
    TOperation* Current()
    {
        return (TOperation*)operations.Current();
    }
    TOperation* Next()
    {
        return (TOperation*)operations.Next();
    }
    int Delete(int i)
    {
        return operations.Delete(i);
    }

    TOperation* GetID(TOperationID oID);

    virtual TOperation& operator[](int iLocation)
    {
        return *(TOperation*)operations[iLocation];
    }
    TOperation* Get(int iLocation)
    {
        return (TOperation*)operations[iLocation];
    }

    int Link(TOperationID oID1, TOperationID oID2);
    int DLink(TOperationID oID1, TOperationID oID2);
    virtual int Remove(TOperation* popX);
    virtual int Remove(int iLocation);
    virtual int RemoveID(int iID)
    {
        return Remove(GetID(iID));
    }
};

/* TJobList class

   This class contains a list of jobs.
   Commands
   ========
   TJobList()
 x [i]           : return the ith job.
 p Append(jb)    : add a job to the job list and return ptr to the new job
 - <<jb          : Append(jb).
 p Get(x)        : return ptr to the job at location xth.
 p GetID(jID)    : return ptr to the job at job ID = jID.
 p Geto_oID(oID) : return ptr to the operation at operation ID=oID.
 p Getj_oID(oID) : return ptr to the job that contains operation ID=oID.
 b Remove(x)     : remove the job at location xth.
 b Remove(px)    : remove the job at px.
 b RemoveID(y)   : remove the job at job ID=y.
 - Clear()       : remove all the jobs.
*/
class TJobList : public TStackP
{
public:
    TJobList()
    {}
    TJobList(TJobList& jblX);
    TJobList& operator=(TJobList& jblX);
    TJob* Append(TJob& jbX);
    void operator<<(TJob& jbX)
    {
        Append(jbX);
    }
    TJob& operator[](int iLocation)
    {
        return *(TJob*)TStackP::operator[](iLocation);
    }
    TJob* Get(int iLocation)
    {
        return (TJob*)TStackP::operator[](iLocation);
    }
    TJob* GetID(TJobID jID);
    TOperation* Geto_oID(TOperationID oID);
    TJob* Getj_oID(TOperationID oIDI);
    int Remove(int iLocation);
    int Remove(TJob* pjb);
    int RemoveID(TJobID jID);
    void RandomProcessTime(int iFrom, int iTo);
    void RandomReleaseTime(int iFrom, int iTo);
    void RandomDueDate(double fPercentFrom, double fPercentTo);
    void RandomPriority(int iFrom, int iTo);
    void RandomStatus(TCHAR cFrom, TCHAR cTo);
    void Clear();
};

/* TSetupElemt class

   A structure for TSetupString.
*/
struct TSetupElemt
{
public:
    TStatus sCode;
    double fTime;
    TSetupElemt(TStatus _sCode = sDefault, double _fTime = 0)
    {
        sCode = _sCode;
        fTime = _fTime;
    }
    TSetupElemt(TSetupElemt& sX)
    {
        sCode = sX.sCode;
        fTime = sX.fTime;
    }
    TSetupElemt& operator=(TSetupElemt& sX)
    {
        sCode = sX.sCode;
        fTime = sX.fTime;
        return *this;
    }
};

/* TSetupString class

   This list contains setup time for a machine that current status is chCode.
   The new status and its setup time (when change machine status) is recorded
   in the list.
   Commands
   ========
   TSetupString(id,default)
 x [i]             : return the ith TSetupElemt
 f [a]             : return the setup time for changing from sCode to a,
                     "fDefault" if not found.
 - Append(x,fx)    : add the setup time, fx, for processing operation x
                     before iID.
 - Clear()         : remove all the elements
 p LocateCode(x)   : return ptr to TSetupElemt that has ID=x, 0 if not found.
 b Remove(x)       : remove operation x from the setup list.
 f Get(x)          : return the setup time for changing from sCode to x,
                     "fDefault" if not found.
*/
class TSetupString : public TStackP
{
public:
    TStatus sCode;  // machine initial status
    double fDefault;

    TSetupString(TStatus _sCode, double _fDefault = 0)
    {
        sCode = _sCode;
        fDefault = _fDefault;
    }
    TSetupString(TSetupString& susX);
    TSetupString& operator=(TSetupString& susX);
    TSetupElemt& operator[](int iLocation)
    {
        return *(TSetupElemt*)TStackP::operator[](iLocation);
    }
    double operator[](TStatus sCode)
    {
        return Get(sCode);
    }
    void Clear();
    void Append(TStatus _sCode, double _fSetupTime);
    TSetupElemt* LocateCode(TStatus _sCode);
    int Remove(TStatus _sCode);
    double Get(TStatus _sCode);
};

/* TSetup class

   This class contains setup time table of the a machine.
   Commands
   ========
   TSetup(default)
 x [i]          : return the ith TSetupString.
 - Default(fx)  : set the default setup time.
 - Clear()      : remove all the elements
 - Append(xx)   : add the TSetupString (current/new status & its setup time)
                  to the setup table.
 - Set(x,y,fx)  : set the setup time, fx, for switching from status x->y.
 p LocateCode(x): return ptr to TSetupString that current status is x.
 b Remove(x)    : remove TSetupString that current m/c status=x from the table.
 f Get(x,y)     : return the setup time from x->y, return default if not found.
*/

class TSetup : public TStackP
{
public:
    double fDefault;

    TSetup(double _fDefault = 0)
    {
        fDefault = _fDefault;
    }
    TSetup(TSetup& suX);
    TSetup& operator=(TSetup& suX);
    TSetupString& operator[](int iLocation)
    {
        return *(TSetupString*)TStackP::operator[](iLocation);
    }
    void Default(double _fDefault)
    {
        fDefault = _fDefault;
    }
    void Clear();
    void Append(TSetupString& susX);
    void Set(TCHAR chFrom, TCHAR chTo, double fSetup);
    TSetupString* LocateCode(TStatus sCode);
    int Remove(TStatus sID);
    double Get(TStatus sFrom, TStatus sTo);
};

class TWorkcenter;

/* TMachine class

   Machine information
   Commands
   ========
   TMachine(id,description,initStatus,speed,batchSize,availtime)
 - AddTask(px)      : add the operation (ptr) to the task list.
 p GetTask(x)       : return ptr to the xth operation in the Task list.
 b IsTask(xx)       : return 1 if xx is the operation that this MC can process.
 b RemoveTask(px)   : remove the px operation from the Task list.
 - Clear()          : clear the memory.
*/
class TMachine
{
public:
    TMachineID mID;
    int iBatchSize;  // batch size; 1 = normal machine
    TCHAR szDescription[30];
    double fSpeed;           // ex: 1.1 = 10%faster 0.9 = 10%slower
    TStatus sInit;           // initial machine status
    double fAvailTime;       // machine available time
    TSetup suSetup;          // setup table
    TWorkcenter* pWkCenter;  // point back to the workcenter that contain this machine
    TStackP stkTask;         // list of operations those can be processed
                             // on this machine

    TMachine(TMachineID _mID = 0,
        LPCTSTR szDescription = _T(""),
        TStatus _sInit = 'a',
        double _fSpeed = 1,
        int _iBatchSize = 1,
        double _fAvailTime = 0);
    TMachine(TMachine& mchX);
    TMachine& operator=(TMachine& mchX);

    void AddTask(TOperation* popX)
    {
        if (!stkTask.Member(popX)) stkTask.Push(popX);
    }
    TOperation* GetTask(int iLocation)
    {
        return (TOperation*)stkTask[iLocation];
    }
    void Clear()
    {
        stkTask.Clear();
        suSetup.Clear();
        mID = 0;
    }

    int IsTask(int iOp);
    int RemoveTask(TOperation* popX);
    void RandomSetup(TCHAR cFrom, TCHAR cTo, int iFrom, int iTo);
};

/* TWorkcenter class

   Commands
   ========
   TWorkcenter(id,description="")
 x [i]             : return the ith machine.
 p Append(xx)      : add a new machine (allocate memory) & return ptr to
                     the new machine (return 0 if duplicate ID).
 p <<xx            : Append(xx);
 - AddTask(px)     : add operation, px, to "stkTask".
 p GetTask(x)      : return the xth operation in the task list.
 b RemoveTask(px)  : remove operation, px, from the task list.
 b IsTask(x)       : return 1 if operation ID x is a task for the workcenter
 - Clear()         : remove all the elements
 p Get(x)          : return the pointer to the xth machine.
 p GetID(x)        : return ptr to machine that has ID = x, 0 if not found
 b Remove(px)      : remove px machine from the workcenter
 b Remove(x)       : remove the xth machine
     == See TStackP class for other commands                       ==
     == DO NOT use Push(xx),Pop(),PopFirst(),Insert(x,xx),Delete(x) ==
     == Due to the lacking of memory allocation.                    ==
*/
class TWorkcenter : public TStackP
{
public:
    TWorkcenterID wID;
    TCHAR szDescription[30];
    TStackP stkTask;
    //
    TWorkcenter(TWorkcenterID _wID = 0, LPCTSTR _szDescription = _T(""))
    {
        wID = _wID;
        _tcscpy(szDescription, _szDescription);
    }
    TWorkcenter(TWorkcenter& wkcX);
    TWorkcenter& operator=(TWorkcenter& wkcX);
    TMachine& operator[](int iLocation)
    {
        return *(TMachine*)TStackP::operator[](iLocation);
    }

    void AddTask(TOperation* popX);
    TOperation* GetTask(int iLocation);
    int RemoveTask(TOperation* popX);
    int IsTask(TOperationID oID);
    int RemoveOperation(TOperation* popX);
    void Clear();
    TMachine* Append(TMachine& mchX);
    TMachine* operator<<(TMachine& mchX)
    {
        return Append(mchX);
    }
    TMachine* GetID(TMachineID mID);
    TMachine* Get(int iLocation)
    {
        return (TMachine*)TStackP::operator[](iLocation);
    }
    int Remove(TMachine* pmchX);
    int Remove(int iLocation);
};

/* TWorkcenterList class

   Commands
   ========
   TWorkcenterList()
 x [i]            : return the ith workcenter.
 p Append(xx)     : add a new workcenter in the list.
 - <<xx           : Append(xx).
 p Get(x)         : return the ptr to TWorkcenter at location xth.
 p GetID(xx)      : return the ptr to TWorkcenter that has id = xx.
 p Getw_mID(xx)   : return the ptr to TWorkcenter that has machine id = xx.
 p Getm_mID(xx)   : return the ptr to TMachine that has id = xx.
 x Getm_oID(xx)   : return list machines (TWorkcenter) that can process operation xx.
 b Remove(x)      : remove the workcenter at location xth.
 b RemoveID(xx)   : remove the machine id=xx from the list.
 - Clear()        : remove everything.
 - ClearTask()    : remove task list from the workcenter & machines
*/

class TWorkcenterList : public TStackP
{
public:
    TWorkcenterList()
    {}
    TWorkcenterList(TWorkcenterList& wklX);
    TWorkcenterList& operator=(TWorkcenterList& wklX);
    TWorkcenter& operator[](int iLocation)
    {
        return *(TWorkcenter*)TStackP::operator[](iLocation);
    }
    TWorkcenter* Append(TWorkcenter& wkcX);
    /*
      { TWorkcenter* pwkcNew = new TWorkcenter(wkcX);
        Push(pwkcNew); return pwkcNew;
      }
    */
    void operator<<(TWorkcenter& wkcX)
    {
        Append(wkcX);
    }
    TWorkcenter* Get(int iLocation)
    {
        return (TWorkcenter*)TStackP::operator[](iLocation);
    }
    TWorkcenter* GetID(TWorkcenterID wID);
    TWorkcenter* Getw_mID(TMachineID mID);
    TMachine* Getm_mID(TMachineID mID);
    TWorkcenter& Getm_oID(TOperationID oID);
    int Remove(int iLocation);
    int RemoveID(TWorkcenterID wIDI);
    void Clear();
    void ClearTask();
};

//*** Overload ostream ***//
ostream& operator<<(ostream& os, TOperation& opX);
ostream& operator<<(ostream& os, TJob& jbX);
// will not read process machine due to no machine info available
ostream& operator<<(ostream& os, TJobList& jblX);
ostream& operator<<(ostream& os, TMachine& mchX);
ostream& operator<<(ostream& os, TWorkcenter& wkcX);
ostream& operator<<(ostream& os, TWorkcenterList& wklX);
ostream& operator<<(ostream& os, TSetup& suX);
//*** Overload istream ***//
istream& operator>>(istream& ifs, TJob& jbX);
istream& operator>>(istream& ifs, TJobList& jblX);
istream& operator>>(istream& ifs, TWorkcenter& wkcX);
istream& operator>>(istream& ifs, TWorkcenterList& wklX);

void Trace(LPSTR szName, TJob& jb);
void Trace(LPSTR szName, TJobList& jl);
void Trace(LPSTR szName, TWorkcenter& wkc);
void Trace(LPSTR szName, TWorkcenterList& wl);
