#pragma once

#define INCLUDE_AFLIB
#define INCLUDE_AFLIB_MATH
#define USE_AFLIB_NAMESPACES
#include "AFLib.h"

/*****************            Strukturen                *****************/

// Knoten eines Netzarbeitplanes
struct TNetWorkingPlanNode
{
    int m_machineID;
    int m_processingTime;

    TNetWorkingPlanNode();
};

// Auftragsinformationen
struct TJob
{
    int m_release;
    int m_due;
    int m_weight;
    CArray<TNetWorkingPlanNode> m_plan;

    TJob();

    int GetOperCount() const
    {
        return m_plan.GetSize();
    }
};

extern CSmartArray<TJob> JOBS;

struct TOper
{
    int m_jobID;
    int m_nodeID;
    int m_machineID;
    int m_processingTime;
    bool m_bLast;

    TOper();
    TOper(int jobID, int nodeID);

    bool IsEmpty() const
    {
        return m_jobID == -1;
    }
    double GetGLKey() const
    {
        return double(m_processingTime) / JOBS[m_jobID]->m_weight;
    }
    bool operator==(const TOper& oper)
    {
        return m_jobID == oper.m_jobID && m_nodeID == oper.m_nodeID;
    }
    bool operator!=(const TOper& oper)
    {
        return !(*this == oper);
    }
};

// Struktur eines Knoten des erweiterten disjunktiven Graphen.
struct TGraphNode : public TOper
{
    int m_position;
    int m_longestPath;
    int m_savedLP;
    int m_inputCounter;
    bool m_bMarked;
    TGraphNode* m_pNodePred;
    TGraphNode* m_pNodeSucc;
    TGraphNode* m_pNodePredLP;

    TGraphNode(const TOper& oper, int position);
    void Swap(int delta);

    void SwapForward()
    {
        Swap(1);
    }
    void SwapBack()
    {
        Swap(-1);
    }
};

// Maschineninformationen
struct TMachine
{
    int m_release;
    CSmartArray<TGraphNode> m_graph;

    CArray<TOper> m_bestJobOrder;
    CArray<TOper> m_activeJobOrder;

    TMachine();
    ~TMachine();

    void ClearMark();
    void SaveConfiguration();
    void ReadBestPlan();

    int GetOperCount()
    {
        return m_activeJobOrder.GetSize();
    }
};

extern CSmartArray<TMachine> MACHINES;

enum TAlgorithm
{
    METROPOLIS = 1,
    PSG = 4
};

enum TObjective
{
    M_S = 1,
    S_C = 4,
    S_T = 5,
    W_C = 6,
    W_T = 7
};

extern TGraphNode* LAST_NODE;

/******************        Globale Variablen      *****************/

// Knoten, die sich auf einem lÑngsten Weg befinden.
extern CSmartArray<TGraphNode> CRITICAL_EDGES;

// GewÑhlte kritische Kante (Anfangsknoten)
extern TGraphNode* CANDIDATE;

// Platz der gewÑhlten kritischen Kante in CRITICAL_EDGES[]
extern int PLACE;

// Zielwert
extern int ACT_RESULT;
extern int CANDIDATE_RESULT;
extern int BEST_RESULT;

// Temperatur
extern double TEMPERATURE;

// Vorgeschlagene Verschlechterungen
extern int BAD_ITER;

// Akzeptierte Verschlechterungen
extern int ACCEPT;

// Vorgegebene Verschlechterungsrate
extern double ACCEPT_RATE;

// Iterationsanzahl
extern int ITERATION;

// Zeitvorgabe fÅr statisches Problem
extern int STAT_TIME;

// Nachbarschaftsstruktur
extern int NEIGHBORHOOD;

// Nachbarschaftsstruktur N5
extern TGraphNode* NODE_2;
extern TGraphNode* NODE_3;

// Welches Lîsungsverfahren
extern TAlgorithm ALGORITHM;

// Which objective function
extern TObjective OBJECTIVE_FUNCTION;

/******************          Definitionen     *****************/

void GetMaxValues(int& MaxJob, int& MaxWkc, int& MaxMch);
