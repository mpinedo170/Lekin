#pragma once

#include "CPM.h"
#include "Heu.h"
#include "Sequence.h"

enum index
{
    DueDate = 0,
    ReleaseTime,
    ProcessTime,
    Weight
};

/* TTool
  COMMANDS
  ========
  TTool(
  TTool(*pShop,*pWkCenter,&DGraph,(*ObjFnc));
 i NumMachine()       : return number of machines in workcenter
 i BatchSize(iMC=1)   : return the batch size for machine iMC
 x &Sequence(Rule)    : return TSequence using various rules
 d Obj()              : return the objective value according to (*ObjFnc)
 - UpdateLink(nID,pm) : update the arc weight of the node nID with the processing time determined from machine
 *pm. call this function at the end of sequence (after Link()).
 - UpdateLink(pn,fx,fy): update the arc weight of the node *pn with fx-fy (previous processing time - new).
                            call this function at the end of sequence (after Link()).
 - Link(n1,n2,pmc)    : link node n1->n2 in SGraph on *pmc machine
 - Link(pn1,pn2,pmc)  : link node *pnd1->*pnd2 in SGraph on *pmc machine
 - Link(n1,n2)        : link node n1->n2 in SGraph on the first machine
 - Link(&sqX)         :
 - Append(&sqX)       : add the sequence sqX to the end of Seq
 - Append(&sqlX)      : add the sequences in sqlX to the end of Seq
 - Insert(&sqX)       : insert the sequence sqX to Seq
 - Insert(&sqlX)      : insert the sequences in sqlX to Seq
 - ClearSequence()    : clear the current sequence

 - PriorityRule(rule) : call the priority rule
 - ParameterATC()     : set fK1/fK2 parameter for ATC rule
 - ParameterTER()     : set fK1/fK2 parameter for TER rule
 - ATC()
 - TER()
 b IsCriticalOperation(x): return 1 if x is a critical operation (on critical path
                           & has a critical link to a sink
 - Critical(ol)       : put all the critical operations in ol
 - CriticalAssoWith(pn,ol): put all the critical operations associate with pn to ol
                            all critical op in ol are on the same critical path with pn
 - Idle(ol)           : put all the operations on critical path that start after
                        the machine is idle
 - IdleAssoWith(pn,ol): put all the idle operations associate with pn to ol
 b Valid()            : return 0 if the sequence is not valid

   convert org oID -> new oID starting from 1
               jID -> new jID starting from 1
*/

class TTool;
class TShop;

class TTool
{
protected:
    void UpdateLink(TNode* pNode, double fPTime, double fOldPTime);
    void UpdateLink(TNodeID nID, TMachine* pMachine);
    void Link(TNodeID nID1, TNodeID nID2, TMachine* pMachine);
    void Link(TNode* pNd1, TNode* pNd2, TMachine* pMachine);  // add and update links for single machine
    void Link(TNodeID nID1, TNodeID nID2);                    // batch machine use BatchLink(i,j)
    void Link(TSequence& sqX);
    void CancelLink(TNode* pNd1, TNode* pNd2);
    void CancelLink(TNodeID nID1, TNodeID nID2);
    // void DLink(TNode *pNode);

public:
    TShop* pShop;            // for referring back
    TWorkcenter* pWkCenter;  // workcenter that is in scheduling
    TSequenceList Seq;
    TDGraph DGraph;  // sequencing graph -- sink nodes remain the same as in pShop->DGraph
    TDGraph SGraph;
    TJob BatchInfo;
    double fK1, fK2;  // parameters for indexing rules
    TObjFunc ObjFnc;

    TTool()
    {
        ObjFnc = ::Cmax;
    }
    TTool(TShop* _pShop, TWorkcenter* _pWkCenter, TDGraph* _pGraph, TObjFunc _Obj = Cmax);
    TTool(TTool& Tl);
    TTool& operator=(TTool& Tl);
    ~TTool(){};
    int NumOp();
    int NumJob()
    {
        return DGraph.NumSink();
    }
    // void    SetSize(int iNumOp,int iNumJob);
    // void    ClearArray();
    void SetData(TShop* pShop, TWorkcenter* pWkc, TDGraph* pGrh);
    void SetObj(TObjFunc _ObjFnc)
    {
        ObjFnc = _ObjFnc;
    }
    void SetWorkcenter(TWorkcenter* pWkcI)
    {
        pWkCenter = pWkcI;
    }
    TStack& PriorTo(TOperationID oIndex);
    int IsActive(TOperationID oIndex);
    TStack& Post(TOperationID oIndex);
    TOperation* GetOp(TOperationID oIndex);  // return ptr to the operation

    int NumMachine()
    {
        return pWkCenter->Num();
    };  // return number of machines in workcenter
        // TSequence &Sequence(int iRule=0); // not use
    double Obj();
    void Append(TSequence& sqX);
    void Append(TSequenceList& sqlX);
    void Insert(TSequence& sqX);
    void Insert(TSequenceList& sqlX);
    void ClearSequence()
    {
        SGraph = DGraph;
        Seq.Clear();
        BatchInfo.Clear();
    };
    int BatchSize(int iMachine = 0);  // return the batch size for machine iMachine
    double BGetProcessTime(int bID);
    double BGetProcessTime(TNode* pNode);
    double BGetReleaseTime(int bID);
    double BGetReleaseTime(TNode* pNode);
    TStatus BGetStatus(int bID);
    TStatus BGetStatus(TNode* pNode);
    void BSetActProcessTime(TNode* pNode, double fPTime, int mID);
    void BSetSetupTime(TNode* pNode, double fTime);
    void BatchLink(int bID1, int bID2);
    void Swap(int iLoc1, int iLoc2);  // swap 2 locations in the sequence
    void Move(int iFrom, int iTo);
    void MoveBack(int iFrom, int iTo);
    int IsFeasible();  // check feasibility by look at EST == -fBigM;
    void GetSequence(TSequenceList& sqList)
    {
        sqList = Seq;
    }

    virtual void PriorityRule(TIndexFunc pIndex);
    virtual int PriorityRule(TIndexFunc pIndex, TSequenceList& SeqList);
    virtual int PriorityRule(TIndexFunc pIndex, int, int iLoop);
    virtual int PriorityRule(TIndexFunc pIndex, TSequenceList& SeqList, int iLoop);

    virtual void ParameterATC();  // determine the parameters
    virtual void ParameterTER();
    void ATC(double fK1I = 0, double fK2I = 0);
    void TER(double fK1I = 0, double fK2I = 0);
    int IsCriticalOperation(int nID);
    void Critical(TStack& opList);  // return the critical operations
    void CriticalAssoWith(TNode* pNode, TStack& opList);
    void Idle(TStack& opList);          // return the operations on critical path that follow the idle
    TNode* EndCluster(TNode* pNode);    // return the last operation in the cluster
    TNode* BeginCluster(TNode* pNode);  // return the first operation in the cluster
    void IdleAssoWith(TNode* pNode, TStack& opList);
    int Valid();
    int ReCalc(int fl = 0);  // if fl=1, force recalc. Return 0 if infeasible
    int Batch(TStack& oIDList, TMachine* pMC);
};

/* T1MC_Center

   Single machine sequencing with batch size = 1.
   machine speed (fSpeed) must = 100
 x Random()       : generate random sequence.
 x Schrage()      : return sequence
 x SC()           : Shifting Cluster, return sequence in Seq
 x Cluster()      : return the next cluster
 - Neighbor(ix,iy): generate neighbor sequence from current sequence
                    the interchange will be in [ix,iy] of the sequence
 - SA(ix,iy,t,(*c),n) : Simulated Annealing --
 - Move(x,y)          : move operation x to follow operation y
 - MoveBack(x,y)      : move operation x to preceed y

*/

class T1MC_Center : public TTool
{
public:
    TMachine* pMC;
    TSequence* pSeq;  // temporary storage

    T1MC_Center(){};
    T1MC_Center(TShop* pShopI, TWorkcenter* pWkcI, TMachine* pMCI, TDGraph* pGraphI, TObjFunc ObjI = Cmax);
    T1MC_Center(TTool& Tl);
    T1MC_Center& operator=(TTool& Tl);
    T1MC_Center(T1MC_Center& Tl);
    T1MC_Center& operator=(T1MC_Center& Tl);
    void SetData(TShop* pShopI, TWorkcenter* pWkcI, TMachine* pMCI, TDGraph* pGrhI);
    void SetData(TShop* pShopI, TWorkcenter* pWkcI, TDGraph* pGrhI);
    void SetMachine(TMachine* pMCI)
    {
        pMC = pMCI;
    };
    void SetSequence(TSequence& sq);
    int IsMove(TNode* pNode, TNode* pCrit);
    int IsMoveBack(TNode* pNode, TNode* pCrit);
    int CPI();
    int Local(int iLoop);
    void GetSequence(TSequence& Sq)
    {
        Sq = *(Seq.Get(1));
    };
    void GetSequence(TSequenceList& sqList)
    {
        sqList = Seq;
    };
    void PriorityRule(TIndexFunc pIndex);
    TSequence& Random();
    TSequence& TER();

    /**** Not use **** save in "notuse.cpp"
    TSequenceList &Cluster();
    TSequence     &Schrage();
    TSequence     &SC(); //shifting cluster
    void           AddBest(TSequence &sqX);
    void           Neighbor(TSequence &LastCluster);
    TSequence     &SA(TSequence &sqX);
    ********/

    // void Move(int iFromID,int iToID); // move pNodeFrom to follow pNodeTo
    // void MoveBack(int iFromID,int iToID); // move pNodeFrom to preceed pNodeTo
};

/* TMMC_Center class

   This class is similar to T1MC_Center but with parallel machines.
   Commands
   ========
   TMMC_Center(ptr_to_workcenter)
   EDD()         : return the sequence list using EDD rule.
   SPT()         : return the sequence list using SPT rule.
*/
class TMMC_Center : public TTool
{
public:
    TMMC_Center() : TTool(){};
    TMMC_Center(TTool& tl) : TTool(tl){};
    TMMC_Center(TMMC_Center& tl);
    TMMC_Center& operator=(TTool& tl);
    TMMC_Center& operator=(TMMC_Center& tl);

    void MyHeuristic();
    void BeamSearch(int iBands);
    void BeamSearch(int iBands, int iLoop);
    void BranchNBound();
    void BranchNBound2();
};

/* TBMC_Center class

   This class is used for solving single batch machine workcenter.
   Commands
   ========
   TBMC_Center(ptr_to_machine)
   EDD()
   SPT()
*/
class TBMC_Center : public T1MC_Center
{
public:
    TBMC_Center() : T1MC_Center(){};
    TBMC_Center(TTool& tl) : T1MC_Center(tl){};
    TBMC_Center(TBMC_Center& tl);
    TBMC_Center& operator=(TTool& tl);
    TBMC_Center& operator=(TBMC_Center& tl);
    void PriorityRule(TIndexFunc pIndex);
    void PriorityRuleS(TIndexFunc pIndex);
    void Batching();  // rebatch the current sequence
    void Batching(TSequence& sq);
    void TER();
    void CPI();
    void CPI_Setup();
};

/* cMBM_Center class

   This class is similar to cBCenter but with paralel batch machines.
   Commands
   ========
   cMBM_Center(ptr_to_workcenter)
   EDD()
   SPT()
*/

// p SA(initSeq,Obj,Neighbor,Cooling,step,n)
TStack* SA(TStack* pInitSeq,
    double (*Obj)(TStack* pSeq),
    double* (*Neightbor)(TStack* pSeq),
    double (*Cooling)(double fT, int iIter),
    int iStep,
    int n);

ostream& operator<<(ostream& os, TTool& tlX);
void Trace(LPCTSTR szName, TTool& tl);
