/* CPM.h   February 29, 1996                  Last modification: 3/5/96

   Project graph representation

*/

#pragma once

#include "Array.h"
#include "Data.h"  // newly added
#include "Stack.h"

class TNode;
extern ostream& operator<<(ostream& os, TNode& ndX);

/* Please check TStack class when TNodeID is set to other types */

const double fBigMM = 9e20;
const TNodeID nMaxNode = 32767;

class TNode;

/* TArc class
 */
class TArc
{
public:
    TNode* pndTo;  // to node
    double fWeight;

    TArc()
    {
        pndTo = NULL;
    }
    TArc(TNode* _pndTo, double _fWeight)
    {
        pndTo = _pndTo;
        fWeight = _fWeight;
    }
};

/* TNode class

   Each node contains a set of arcs to-from the other nodes.
   Commands
   ========
   TNode(id),TNode(id,index),TNode(id,index,EST,LST,DueDate)
 f EST()          : return the earliest starting time
 f LST()          : return the latest starting time
 f DueDate()      : return the latest completion time
 x ID()           : return id
 - Clear()        : remove all the arcs
 - AddPrior(ax)   : add TArc, ax, to the prior arcs list
 - AddPrior(px,fx): add ptr px (to node) with arc weight fx to the prior arcs list
 b RemovePrior(px): remove ptr to TNode px from the prior arcs list.
 - AddPost(ax)    : add TArc, ax, to the post arcs list
 - AddPost(px,fx) : add ptr px (to node) with arc weight fx to the post arcs list
 b RemovePost(px) : remove ptr to TNode px to the post arcs list.
 p GetPriorArcPtr(x) : return ptr to prior arc at location x in the PriorArc list.
 p GetPriorNode(x)   : return ptr to node at location x in the PriorArc list.
 x GetPriorID(x)     : return the prior node ID     " ------ "
 f GetPriorWeight(x) : return the prior arc weight  " ------ "
 f GetPriorWeight(px): return the prior arc weight of node pointed by px.
 p GetPostArcPtr(x)  : return ptr to post arc at location x in the PostArc list.
 p GetPostNode(x)    : return ptr to node at location x in the PostArc list.
 x GetPostID(x)      : return the post operation ID " ------ "
 f GetPostWeight(x)  : return the post arc weight   " ------ "
 f GetPostWeight(px) : return the post arc weight of node pointed by px. Return fBigM if not found.
 b IsOnCritical()    : return 1 if fEST==fLST
*/
class TNode
{
    friend class TDGraph;
    friend ostream& operator<<(ostream& os, TNode& ndX);

public:
    TNodeID nID;
    TOperation* op;

private:
    double fIndex;
    double fEST;      // Earliest starting time
    double fLST;      // Latest starting time
    double fDueDate;  // Duedate(Latest completion)=fLST + fProcessTime + fSetup
public:
    TStackP stkPostArc;   // ptr to TArc ; TArc ptr to TNode & weight
    TStackP stkPriorArc;  // ptr to TArc

public:
    // functions
    TNode(TNodeID _nID = 0,
        double _fIndex = 0,
        double _fEST = 0,
        double _fLST = fBigM,
        double _fDueDate = fBigM);
    TNode(TNode& ndX);
    TNode& operator=(TNode& ndX);

    double EST()
    {
        return fEST;
    }
    double LST()
    {
        return fLST;
    }
    double DueDate()
    {
        return fDueDate;
    }
    TNodeID ID()
    {
        return nID;
    }

    void Clear();
    void Destroy();  // free all memory used = ~TNode()
    void AddPrior(TArc& arcX);
    void AddPrior(TNode* pndFrom, double _fWeight);
    int RemovePrior(TNode* pndRemove);
    void AddPost(TArc& arcX);
    void AddPost(TNode* pndTo, double _fWeight);
    int RemovePost(TNode* pndRemove);

    TArc* GetPriorArcPtr(int iLocation)
    {
        return (TArc*)(stkPriorArc[iLocation]);
    }
    TNode* GetPriorNode(int iLocation)
    {
        return GetPriorArcPtr(iLocation)->pndTo;
    }
    TNodeID GetPriorID(int iLocation)
    {
        return (((TArc*)stkPriorArc[iLocation])->pndTo)->nID;
    }
    double GetPriorWeight(int iLocation)
    {
        return ((TArc*)stkPriorArc[iLocation])->fWeight;
    }

    double GetPostWeight(TNode* pndToI);
    double GetPriorWeight(TNode* pndFrom);
    TNode* GetPostDummy();

    TArc* GetPostArcPtr(int iLocation)
    {
        return (TArc*)(stkPostArc[iLocation]);
    }
    TNode* GetPostNode(int iLocation)
    {
        return GetPostArcPtr(iLocation)->pndTo;
    }
    TNodeID GetPostID(int iLocation)
    {
        return (((TArc*)stkPostArc[iLocation])->pndTo)->nID;
    }
    double GetPostWeight(int iLocation)
    {
        return ((TArc*)stkPostArc[iLocation])->fWeight;
    }

    int IsDummy()
    {
        return nID > -(nMaxNode / 2) && nID < -1;
    }
    int IsSink()
    {
        return nID < -(nMaxNode / 2);
    }
    int IsOperation()
    {
        return nID > 0;
    }
    int IsOnCriticalPath()
    {
        return fEST == fLST;
    }
    ~TNode()
    {
        Clear();
    }
};

/* TDGraph class

   Scheduling graph representation. Initially, it is a di-graph. Then, the
   conjunctive arcs can be added. The main features are to determine makespan,
   lateness, earliest starting time (rj), latest starting time (dj-pj),
   due date (dj) for each node(operation). It can be used to determine active
   operations for dispatching rules. "active()" function will return id of
   operation that does not have precedent constraints but some post operations.

   TNodeID  0                    : source node
          (0,nMaxNode)          : regular nodes
          (nLastDummy,-2]       : Dummy nodes
          (-nMaxNode,nLastSink) : sink nodes

   Commands
   ========
   TDGraph(x=1) single sink node
   TDGraph(2)   multi-sink nodes
 p AddNode(x,fx)  : Add new node with nID=x (>0) & fIndex = fx and return ptr to that node.
 p AddDummy()     : Add dummy node (id<0) and return the pointer to that node
 p AddSink()      : Add sink node(s) for nodes that don't have post node
 p AddSink(x,w,d) : Add sink node after node x with weight w and duedate d.
                    Return ptr to that node.
 i NumDummy()     : Return # of dummy nodes.
 i NumSink()      : Return # of sink nodes.
 b Remove(x)      : remove node nID=x.
 b Remove(px)     : remove node that ptr px point to.
 b RemoveDummy(i) : remove dummy node at location ith.
 - Clear()        : remove all the nodes and arcs.
 p Get(x)         : return ptr to the xth node in the graph
 p GetID(x)       : return ptr to node nID=x.
 p GetSink(x)     : return ptr to xth sink node.
 b Link(x,y,fx)   : add arc from node x to node y with weight fx;
                    if x or y not exist return 0, if x & y exist replace
                    weight with max(weight,fx)
 b Link(px,py,fx) : add arc from node *px -> *py with weight fx.
 b RLink(x,y,fx)  : Link(x,y,fx) if link x->y exists, replace weight with fx
 b RLink(px,py,fx): Link(px,py,fx) " ---------------------------------- "
 b DLink(x,y)     : remove arcs between nodes x and y.
 b DLink(px,py)   : remove arcs between nodes *px and *py.
 f EST(x)         : return the earliest starting time of node id=x
 f LST(x)         : return the latest starting time of node id=x
 f DueDate(x)     : return the due date of node id=x
 f MakeSpan()     : return the max completion time of the last operation
 x ActiveList()   : return TStack (list) of node IDs those are active
                    (no precedent constraints but have some operations to perform)
 x ActiveList(xx) : return TStack that fEST<xx.
 x Active(xx)     : return node id in active list that has max index (default=release time)
                    and release time<=xx. If not found return the node id with
                    min release time. If no active node, return -1.
 p Active(i)      : return the ptr to ith stkActive node.
 b CopyEST(xx)    : copy EST from xx (TDGraph).
 b CopyLST(xx)    : copy LST and DueDate from xx.
 b CopyESTLST(xx) : copy EST, LST and DueDate from xx.
 b Sequence(x)    : remove the links from node id=x & update EST of
                    preceeding nodes (x must be in Active list).
 b Sequence(px)   :  == same == but with ptr
 b Sequence(x,f)  : similar to Sequence(x) but change the weight for the post
                    arcs to f.
 - Copy(xx)       : copy TDGraph from xx & reset EST,LST,Duedate.
 b ReCalcEST()    : recalculate EST.
 b ReCalcLST()    : recalculate LST and DueDate.
 i ReCalc()       : recalculate EST,LST,DueDate. return 0 if recalc is not necessary.
                    return 1:normal, return 2: no feasible sequence
 b Reduce(px)     : remove the node px & connect its prior nodes to its post nodes

   private:
 - UpDate(px,py)  : recalc EST for node *py.
 - UpDateR(px,py) : recalc LST & DueDate for node *px.
 - AddActive(px)  : add node *px to stkActive.
 - Debug(px,sz)   : show "sz" string & current info of object px (TDGraph).
*/
class TDGraph : public TStackP
{
private:
    TArrayP aryNode;
    TNodeID nLastSink;   // last sink node id (id>-nMaxNode);
    TNodeID nLastDummy;  // last dummy node id (id<-1)
    int iType;           // 1 = single sink node; 2 = multi-sink nodes
    int iFlag;           // 1 = need recalc;      0 = up to date
    TStackP stkSink;     // ptr to sink nodes
    TStackP stkDummy;

    void UpDate(TNode* pndFrom, TNode* pndTo);   // update fEST
    void UpDateR(TNode* pndFrom, TNode* pndTo);  // update fLST & fDueDate
    void AddActive(TNode* pndX);
    void Debug(TDGraph& dgX, LPSTR sz);

public:
    TStackP stkActive;  // ptr to nodes w/o prior arc

    TDGraph(int _iType = 1);
    TDGraph(TDGraph& dgX);
    TDGraph& operator=(TDGraph& dgX);
    ~TDGraph();
    TNode* AddNode(
        TNodeID _nID, double _fIndex = 0, double _fEST = 0, double _fLST = fBigMM, double fDueDate = fBigMM);
    TNode* AddSink(TNodeID _nID = 0, double _fWeight = 0, double fDueDate = fBigMM);
    TNode* AddDummy();

    int NumDummy()
    {
        return stkDummy.Num();
    }
    int NumSink()
    {
        return stkSink.Num();
    }

    int Remove(TNodeID _nID);
    int Remove(TNode* pndX);
    int RemoveDummy(int iLocation);
    int RemoveDummy(TNode* pndDummy);
    void Clear();
    void Destroy();
    void FindActive();  // find stkActive
    void ActiveList(TStack& stk, double fTime = fBigMM);

    TNode* GetID(TNodeID nID);
    TNode* GetSink(int iN);

    TNode* Get(int iLocation)
    {
        return (TNode*)(*this)[iLocation];
    }
    TNode* GetDummy(int iN)
    {
        return (TNode*)stkDummy[iN];
    }

    int Link(TNodeID nID1, TNodeID nID2, double fWeight);
    int Link(TNode* pndFrom, TNode* pndTo, double fWeight);
    int RLink(TNodeID nID1, TNodeID nID2, double fWeight);  // Replace Link
    int RLink(TNode* pndFrom, TNode* pndTo, double fWeight);
    int DLink(TNodeID nID1, TNodeID nID2);
    int DLink(TNode* pndFrom, TNode* pndTo);

    int ReCalcEST();
    int ReCalcLST();
    int ReCalc();  // recalc the EST, LST, DueDate
    void ReCalc2();

    double EST(TNodeID nID)
    {
        return GetID(nID)->fEST;
    }
    double LST(TNodeID nID)
    {
        return GetID(nID)->fLST;
    }
    double DueDate(TNodeID nID);
    double MakeSpan();
    TNodeID Active(double fTime = fBigMM);
    TNode* GetActive(int iLocation)
    {
        return (TNode*)stkActive[iLocation];
    }

    int CopyEST(TDGraph& dgX);
    int CopyLST(TDGraph& dgX);
    int CopyESTLST(TDGraph& dgX);
    int Sequence(TNodeID _nID);
    int Sequence(TNode* pndCurrent);
    int Sequence(TNodeID _nID, double fProcessTime);
    void Copy(TDGraph& dgX);
    int Reduce(TNode* pNode);
};

void Trace(LPCTSTR szName, TDGraph& Graph);
void Log(LPCTSTR szName, TDGraph& Graph);

//*** overload iostream ***//
ostream& operator<<(ostream& os, TNode& ndX);
ostream& operator<<(ostream& os, TDGraph& dgX);
