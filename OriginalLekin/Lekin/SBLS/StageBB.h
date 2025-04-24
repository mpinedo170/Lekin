// StageBB.h:
// This file specifies the data structures and the functions used in the
// B&B algorithm in StageBB.cxx

#ifndef STAGE_BB_H
#define STAGE_BB_H

#include "Constants.h"

class Data;
extern void Decomposition_Heuristic(const Data&);

// info about the input jobs, all static data, get from Data
class Job
{
    int ReleaseTime;
    int* ProcessTime;
    int DueTime;
    int weight;
    int CompleteTime;

public:
    Job()
    {
        ProcessTime = new int[MAXSTAGE];
    }
    ~Job()
    {
        if (ProcessTime) delete[] ProcessTime;
    }

    friend class Graph;
    friend void Decomposition_Heuristic(const Data& I);
};

// The below structures are all about the disjunctive graph
class OpInfo
{
    int stage;
    int job;
    int Weight;
    int Release;
    int* Due;
    int Process;

public:
    OpInfo()
    {
        Due = new int[MAXJOB];
    }
    ~OpInfo()
    {
        if (Due) delete[] Due;
    }

    friend class Graph;
    friend class ParallelMachine;
    friend void Decomposition_Heuristic(const Data& I);
};

// for conjunctive arcs, from, to is index of the stages,
// for disjunctive arcs, from, to is index of the jobs.
class Arc
{
    int from;
    int to;
    int length;
    int neglength;  // -length, for the calculation of longest path

    friend class Graph;
};

// This class is only used in the calculation of longest path,
// It's used in the FIFO QUEUE which maintains the nodes which violate
// the optimality condition of d(j) <= d(i)+C(i,j) for arc (i,j)
class Node_Index
{
    int StageIndex;
    int JobIndex;

    friend class Graph;
    friend int in(int Stage, int Job, Node_Index* QUEUE, int head, int QLength);
};

class Node
{
    int StageIndx;
    int JobIndx;

    int PathLength;  // the label used in the label correcting algo. for longest path calculation

    OpInfo operation;

    Arc* DisNext;
    int ExistDisNext;
    Arc* DisPre;
    int ExistDisPre;
    Arc* ConNext;
    Arc* ConPre;

public:
    Node()
    {
        DisNext = new Arc;
        ExistDisNext = FALSE;
        DisPre = new Arc;
        ExistDisPre = FALSE;
        ConNext = new Arc;
        ConPre = new Arc;
    }
    ~Node()
    {
        if (DisNext) delete DisNext;
        if (DisPre) delete DisPre;
        if (ConNext) delete ConNext;
        if (ConPre) delete ConPre;
    }

    // for Graph::Reset() to use member of Node
    friend class Graph;
    friend class ParallelMachine;
    friend void Decomposition_Heuristic(const Data& I);  // for print out only. May delete later
};

class Source
{
    int PathLength;

    Arc** ConNext;

public:
    Source(int numj)
    {
        ConNext = new Arc*[numj];
        for (int i = 0; i < numj; i++) ConNext[i] = new Arc;
    }
    ~Source()
    {
        if (ConNext) delete[] ConNext;
    }

    friend class Graph;
};

class Sink
{
    int PathLength;

    Arc* ConPre;

public:
    Sink()
    {
        ConPre = new Arc;
    }
    ~Sink()
    {
        if (ConPre) delete ConPre;
    }

    friend class Graph;
};

class PartialSchedule;
class ParallelMachine;

class Graph
{
    int NumStages;
    int NumJobs;
    int* NumMachines;

    Job* jobs;

    Source* source;
    Node*** graph;
    Sink** sink;

    int* IsScheduled;  // This array is flag to indicate whether a stage has been scheduled or not

public:
    Graph(int, int);
    ~Graph();
    void Reset(int, int);
    void Reset(int);
    int Cal_Release_Time(int, int);
    void Cal_Due_Dates(int, int);
    void Cal_Completion_Time(int);
    void Buildup_Graph(const Data&);
    void Detect_Bottleneck_Stage();
    void Fix_A_Stage(Node** stage, PartialSchedule* schedule, int size);

    // need info. of a stage of the graph to build up the case of parallel machine scheduling
    friend ParallelMachine;
    friend void Decomposition_Heuristic(const Data&);

    // The list of stage priorities do not belong to any class, so, it's perfectly
    // fine to make it public here. It actually behaves like a global variable

    int* BottleList;
};

////////////////////////////////////////////////////////////////////////////////////
// Data Structures about the parallelmachine subproblems
///////////////////////////////////////////////////////////////////////////////////

class PartialSchedule
{
    int length;
    int tardiness;  // it is double tardiness here
    int* sequence;

    friend void Decomposition_Heuristic(const Data&);
    friend void Save(const Data&, PartialSchedule** BestSchedule);

public:
    PartialSchedule()
    {
        sequence = new int[MAXJOB];
    }

    PartialSchedule(const PartialSchedule& from)
    {
        length = from.length;
        sequence = new int[MAXJOB];
        // memcpy(sequence, from.sequence, length);
        for (int i = 0; i < length; i++) sequence[i] = from.sequence[i];
        tardiness = from.tardiness;
    }

    PartialSchedule& operator=(const PartialSchedule& from)
    {
        if (this != &from)
        {
            length = from.length;
            if (sequence) delete[] sequence;
            sequence = new int[MAXJOB];
            assert(sequence);
            // memcpy(sequence, from.sequence, length);
            for (int i = 0; i < from.length; i++) sequence[i] = from.sequence[i];
            tardiness = from.tardiness;
        }
        return *this;
    }

    ~PartialSchedule()
    {
        if (sequence) delete[] sequence;
    }

    friend void Graph::Fix_A_Stage(Node** stage, PartialSchedule* schedule, int size);
    friend class ParallelMachine;
};

// Additional Constraint Sequence.
// It can be either the forbidden sequence or on the contrary,
// the enforcing sequence
class AddConSeq
{
public:
    int front;
    int hind;
};

class BBNode
{
    int LowerBound;
    int IsIntegral;
    int fsize;          // size of ForSeq;
    AddConSeq* ForSeq;  // record the set of forbidden sequences of the
                        // this B&B node.
    int esize;          // size of EnfSeq;
    AddConSeq* EnfSeq;  // record the set of enforced sequences of the
                        // B&B node.

public:
    BBNode()
    {
        ForSeq = new AddConSeq[SIZE];
        assert(ForSeq);
        EnfSeq = new AddConSeq[SIZE];
        assert(EnfSeq);
    }

    ~BBNode()
    {
        if (ForSeq) delete[] ForSeq;
        if (EnfSeq) delete[] EnfSeq;
    }

    // for ParallelMachine::Process_A_Node;
    friend class ParallelMachine;
};

class CplexData
{
    // for the function ParallelMachine::Fill_Lp_Pars() etc
    friend class ParallelMachine;

    int mac;
    int mar;
    double* objx;
    double* rhsx;
    char* senx;
    int* matbeg;
    int* matcnt;
    int* matind;
    double* matval;
    double* bdl;
    double* bdu;
    int macsz;
    int marsz;
    int matsz;

public:
    CplexData(int col, int row)
    {
        int mat = col * row;

        objx = new double[col];
        rhsx = new double[row];
        senx = new char[row];
        matbeg = new int[col];
        matcnt = new int[col];
        matind = new int[mat];
        matval = new double[mat];
        bdl = new double[col];
        bdu = new double[col];
    }

    ~CplexData()
    {
        if (objx) delete[] objx;
        if (rhsx) delete[] rhsx;
        if (senx) delete[] senx;
        if (matbeg) delete[] matbeg;
        if (matcnt) delete[] matcnt;
        if (matind) delete[] matind;
        if (matval) delete[] matval;
        if (bdl) delete[] bdl;
        if (bdu) delete[] bdu;
    }
};

class SENode
{  // this is a node in the enumeration tree for the single machine subproblem

    int length;
    double ReducedCost;
    int* sequence;

public:
    SENode()
    {
        ReducedCost = 0.0;
        sequence = new int[MAXJOB];
    }

    SENode(const SENode& from)
    {
        sequence = new int[length = from.length];
        // memcpy(sequence, from.sequence, length);
        for (int i = 0; i < length; i++) sequence[i] = from.sequence[i];
        ReducedCost = from.ReducedCost;
    }

    SENode& operator=(const SENode& from)
    {
        if (this != &from)
        {
            delete[] sequence;
            sequence = new int[length = from.length];
            for (int i = 0; i < length; i++) sequence[i] = from.sequence[i];
            ReducedCost = from.ReducedCost;
        }
        return *this;
    }

    ~SENode()
    {
        if (sequence) delete[] sequence;
    }

    friend class ParallelMachine;
};

class ParallelMachine
{
    // Input Data
    int NumMachines;
    int NumJobs;

    OpInfo* jobs;  // the jobs here is actually the operations on this stage

    // the schedule during the local search procedure
    PartialSchedule* RestrictedSet;
    int RSetSize;  // the number of columns in the restricted set
    int objective;

    // the so far best schedule during local search
    PartialSchedule* OptSchedule;
    PartialSchedule* OptSchedule2;
    int OptSize;
    int OptSize2;
    int OptValue;   // for double summation objective
    int OptValue2;  // for summation of the maximum tardiness objective

public:
    // functions

    // the constructor of a ParallelMachine object certainly base on one stage in the disjunctive graph
    ParallelMachine(int, const Graph*);
    ~ParallelMachine();

    int Cal_Initial_Upperbound();
    int Cal_Tardiness(const int* sequence, const int length);
    void Compute_Individual_Tardiness(const int* sequence, const int length);
    int Is_Active_Schedule(const int* schedule, const int length);
    void Solve_The_Stage();
    void Reload_The_Stage();
    void Reschedule_Process();
    int Cal_Objective_Value(const int* sequence, const int length, const int* duedate);
    int Stage_Local_Search(int);
    int Stage_Local_Search_Again(int);

    int Find_Max_Tardiness_Job_Index(const int* sequence, const int length);
    int Find_Second_Tardiness_Job_Index(const int* sequence, const int length, const int jobindex);
    void Single_Machine_ATC(int* sequence, const int* JobSet, const int JobSize);
    int Single_Machine_Local_Search(int* sequence, int length);
    void Single_Machine_Local_Search_Again(int* sequence, int length, int index);

    friend void Graph::Fix_A_Stage(Node** stage, PartialSchedule* schedule, int size);
    friend void Decomposition_Heuristic(const Data&);
};

#endif
