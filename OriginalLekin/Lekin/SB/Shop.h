#pragma once

#include "CPM.h"
#include "Data.h"
#include "Parameter.h"
#include "Period.h"
#include "Sequence.h"

/* indexing functions

   These global functions are used in TShop::PriorityRule().
   They assign the sorting index according to the function
   name. After calling Sort(), the min index value can be
   taken by calling Get(1).

*/

class TMcAvail
{
public:
    int iLastMC;
    TMachineID mID[iMaxMachine];
    TMachine* pMC[iMaxMachine];
    int iLastOp[iMaxMachine];  // last operation on machince mID[i]
    double fTime[iMaxMachine];
    double fData1, fData2;  // user data in indexing rule

    TMcAvail();
    ~TMcAvail();
    TMachine* GetMC(int iLoc)
    {
        return pMC[iLoc];
    }
    int Num()
    {
        return iLastMC;
    }

    void Set(TMachineID mIDX, TMachine* pMCI, double fTimeX, int iLastOp);
    double Get(TMachineID mIDX);
    int IsMember(int mIDI);
    int LastOp(TMachineID mIDX);
};

class TRuleReturn
{
public:
    TOperation* op;
    TMachine* mc;
    double fIndex;
    int iDataReCalc;  // need recalculation?

    TRuleReturn()
    {
        iDataReCalc = 0;
    }
};

/* TShop class

   This class is the main class.
   DGraph  = Disjunctive graph
   SGraph  = DGraph + sequences

   Commands
   ========
   TShop();
 p Append(jx)        : append job jx & add links in DGraph & SGraph.
 p Append(wx)        : append workcenter wx.
 p Append(sx)        : append sequence sx.
 - Append(slx)       : append sequence list slx.
 - Append(pdx)       : append period pdx to holiday or week
 - Remove(pj)
 - Remove(pw)
 - Remove(sx)        : remove sequence sx & links.
 - Remove(slx)       : remove sequence list slx.
 - RemoveSeq(pw)     : remove the sequence for workcenter *pw
 x GetJob(iS)        : return ptr to job that its sink node is 'iS'th location in stkSink.
                       or node ID is iS.
 x GetWkc(wID)       : return ptr to workcenter that has ID = wID.
 x GetMC(mID)        : return ptr to machine that has ID = mID.
 - Assign(ox,wx)     : assign operation ox to workcenter wx.
 b DAssign(ox,mx)    : machine mx cannot process operation ox.
 b LoadWk(is)        : load workcenter information from input stream.
 b LoadAvl(is)       : load shop availability
 b LoadJob(is)       : load job information form file input stream; return 1/0.
 b LoadSeq(is)       : load sequence from input stream; return 1/0;
 b LoadSeq(sx)       : sync sequence sx to the shop; return 1/0.
 b SaveAll(sz)       : save job, workcenter, & sequence in file "sz".
 - Clear()           : clear all.
 - ClearSequence()   : clear the current sequence.
 i NumMachine()      : return number of machines
 i NumSequence()     : return number of sequence

 b ReCalc()          : recalculate the operation starting time. return 0:infeasible seq
 b ReCalc(1)         : force recalc.
 f JobCompletion(jID): return the completion time of job id=jID.
 f AvgSetup(pwk)     : return average setup time on workstaion *pWk
 f AvgSetup()        : return avg setup for the shop
 f AvgProcessTime(pwk): return average processing time on workcenter *pWk
 f AvgProcessTime()   : return avg processing time for the shop
 x OperationOnWkc(wID): return a list of operations (TJob) that need
                        processing on workcenter wID (with precedent
                        constraints). [r=EST, d=LST+p]
 x GraphOnWkc(p)     : return a digraph for the particular workcenter
 - AddSeq(oID,mID)   : Assign oID to Seq. Update SGraph.
 b Valid()           : return 1 if the current sequence is valid
 b IsValid(sq)       : return 0 if the sequence sq is not appropriate for this shop.
 b IsValid(sql)      : return 0 if the sequence list sql is not appropriate for this shop.
 b IsComplete()      : return 0 if any operation has not assigned the machine.
 - ReGraph()         : Re-create DGraph when operation/job changes
 - PriorityRule(pFx) : apply static priority rule (pre-assign weights).
 - SB(pFx,n)         : sequence the jobs using SB technique. pFx is a function
                       ptr performing workcenter squencing. Local
           reoptimization will be done for n times.
 f Cmax()            : return the maximum weighted completion time
 f Lmax()           : return the maximum weighted lateness
 f WTmax()           : return the maximum weighted tardiness
 f WC()              : return the weighted completion time
 f WT()              : return the weighted tardiness
 f WU()              : return the weighted number of late jobs.

  - SB(obj,bottle,opt,local,final): SB

 - ATC()
 - EDD1()            :
 - EDD2()            :
 - SPT()
 - LPT()
 - FCFS()
 - SB1()             :
 - SB2()

   ******** Private
 - UpdateLink(nd,fx,fy): update the links from node nd according to the new processing time fx (old processing
 time = fy). b AddLink(jx)       : add job jx to the DGraph & SGraph. b AddLink(mid,n1,n2): add link (n1->n2)
 in SGraph. If there is setup time, modify the post arc weight of n1. b AddLink(n1,n2,fx,fy) : add link
 (n1->n2) in SGraph with weight fx.
           (&modify the post arc 'n1' weight to fy)
 - AddLink(sq)       : add links to SGraph according to sequence sq.
 i DLink(pn1,pn2,fx) : remove a link (n1->n2) in SGraph & modify the post arc
                       'n1' weight to fx.
 i DLink(n1,n2)      : remove the link (n1->n2) on a machine & modify the arc
                       link to fProcessTime.
 - DLink(sq)         : remove the links from SGraph according to sq.
 - UpdateOperation() : copy the EST&DueDate from SGraph to operations.
 - UpdateProcessTime(): determine the actual processing time
 - FindPrecedent(jbX,oID,pC,stkX) : find precedent relations of nodes in stkX
                       and node 'oID'. If found, add a link in jbX.
                       It is a recursive function working at node 'pC'.

*/

class TShop
{
    friend class TTool;

protected:
    TStackP SinkJob;  // array of ptr point to job according to sink nodes in DGraph
    TDGraph DGraph;

public:
    TDGraph SGraph;
    TJobList Job;
    TWorkcenterList WkCenter;
    TSequenceList Seq;
    TAvailability Avail;
    double fK1, fK2;
    TJob BatchInfo;

protected:
    void UpdateLink(TNode* pNd, double fTime, double fOldTime);
    void UpdateLink(TNodeID nIDx, double fTime, double fOldTime);
    int AddLink(TSequence& sqX);
    int AddLink(TJob& jbX);
    int AddLink(TMachineID mID, TNodeID nID1, TNodeID nID2);
    int AddLink(TMachine* pMachine, TNode* pNode1, TNode* pNode2);
    int AddLink(TNode* pNd1, TNode* pNd2, double fLength, double fProcessTime);
    int AddLink(TNodeID nID1, TNodeID nID2, double fLength, double fProcessTime);
    int DLink(TNode* pNode1, TNode* pNode2, double fProcessTime);
    int DLink(TNodeID nID1, TNodeID nID2);
    void DLink(TSequence& sqX);
    void UpdateOperation();
    void UpdateProcessTime();
    void FindPrecedent(TJob& jbX, TOperationID oID, TOperation* popCurrent, TStackP stkX);

public:
    TShop();
    TShop(TShop& sh);
    TShop& operator=(TShop& sh);
    void Clear();
    void ClearJob();
    void ClearSequence();
    TJob* Append(TJob& jbX);
    TWorkcenter* Append(TWorkcenter& wkX);
    TSequence* Append(TSequence& sqX);
    void Append(TPeriod& prdX);
    void Append(TSequenceList& sqlX);
    TJob* AppendAutoID(TJob& jbX);
    void Remove(TJob* pjb);
    void Remove(TMachine* pmc);
    void Remove(TWorkcenter* pwk);
    void Remove(TSequence* psqX);
    void Remove(TSequenceList* psqlX);
    void RemoveSeq(TWorkcenter* pWk);
    TJob* GetJob(int iSink);
    TWorkcenter* GetWkc(TWorkcenterID wID);
    TMachine* GetMC(TMachineID mID);
    void GetSequence(TWorkcenter* pWk, TSequenceList& sqList);
    int NumMachine();
    int NumSequence();
    void Assign(TOperationID oID, TWorkcenterID wID);
    int DAssign_wk(TOperationID oIDI, TWorkcenterID wIDI);
    int DAssign_mc(TOperationID oIDI, TMachineID mIDI);
    int LoadWk(istream& is);
    int LoadAvl(istream& is);
    int LoadJob(istream& is);
    int LoadSeq(istream& is);
    int SaveAll(LPCTSTR szName);
    double AvgSetup(TWorkcenter* pWkCenter);
    double AvgSetup();
    double AvgProcessTime(TWorkcenter* pWkCenter);
    double AvgProcessTime();
    TJob& OperationOnWkc(TWorkcenterID wIDI);
    void GraphOnWkc(TWorkcenter* pWkCenter, TDGraph& Graph);
    int IsValid();
    int IsValid(TSequence& sq);
    int IsValid(TSequenceList& sql);
    int IsComplete();
    void AdjustHoliday();
    void ReGraph();
    void RandomStatusSingle(TCHAR cFrom, TCHAR cTo);
    void RandomStatusParallel(TCHAR cFrom, TCHAR cTo);
    void RandomStatusBatch(TCHAR cFrom, TCHAR cTo, int iFrom, int iTo);
    void SetStatusBatch(TCHAR cFrom, int iProcessTime);
    // batch //
    int Batch(TStack& oIDList, TMachine* pMC);
    int BatchAdd(int iBatch, int nID);
    double BGetProcessTime(int bID);
    double BGetProcessTime(TNode* pNode);
    double BGetReleaseTime(int bID);
    double BGetReleaseTime(TNode* pNode);
    void BSetProcessTime(TNode* pDummy, double fP);
    void BSetSetupTime(TNode* pNode, double fTime);
    TStatus BGetStatus(int bID);
    TStatus BGetStatus(TNode* pNode);
    void BatchLink(int bID1, int bID2);
    //
    void PriorityRule(TIndexFunc pfn);
    void PriorityRule2(TIndexFunc pfn);
    void SB(TSequenceList& (*WkcSeq)(TTool& WkTool),
        double (*Obj)(int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight),
        int iLocal);
    //*******
    int ReCalc(int fl = 0);
    double JobCompletion(TJobID jIDI);
    double Cmax();
    double Lmax();
    double WLmax();
    double WTmax();
    double WC();
    double WFlow();
    double WT();
    double WTE();
    double WU();
    double U();
    double Objective(TObjFunc Obj);

    //*******
    void ATC(double fK1I = 0, double fK2I = 0);
    void EDD_J();
    void EDD_O();
    void SPT();
    void LPT();
    void FCFS();
    void TER(double fK1I = 0, double fK2I = 0);
    void SB(double (*Obj)(
                int iNum, double* fDue, double* fComplete, double* fWeight, double* fEWeight), /* objective */
        double (*BNSeq)(TTool& WkTool),  /* bottleneck determination */
        double (*OptSeq)(TTool& WkTool), /* optimizing bottleneck */
        void (*LocalOpt)(TShop& Shop,
            TStackP& WkcSchedList,
            double (*OptSeq)(TTool& WkTool),
            double (*Obj)(int iNum,
                double* fDue,
                double* fComplete,
                double* fWeight,
                double* fEWeight)), /* Local improvement */
        void (*FinalOpt)(TShop& Shop,
            TStackP& WkcSchedList,
            double (*OptSeq)(TTool& WkTool),
            double (*Obj)(int iNum,
                double* fDue,
                double* fComplete,
                double* fWeight,
                double* fEWeight))); /* Final improvement */
    void SB1();
    void SB2();
};

//*** overload iostream ***//
ostream& operator<<(ostream& os, TShop& spX);

void Trace(LPSTR szName, TShop& sh);
