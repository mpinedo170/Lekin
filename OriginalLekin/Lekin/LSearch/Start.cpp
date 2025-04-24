/************************************************************************/
/*                                                                      */
/*                   Erzeugung eines ersten Ablaufplanes                */
/*                     mit Hilfe von PrioritÑtsregeln.                  */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "Datei.h"
#include "Def.h"
#include "Neighbor.h"
#include "Start.h"

/************************************************************************/
/*                                                                      */
/*  Im folgenden werden einige Funktionen angegeben, die zur Erstellung */
/*  und zum Loeschen einer einfachen Liste benoetigt werden.            */
/*                                                                      */
/************************************************************************/

TMachineHeap MC_HEAP;
CSmartArray<TMachineInfo> MC_ARRAY;

void AddOper(int systemTime, const TOper& oper, int release)
{
    MC_ARRAY[oper.m_machineID]->AddOper(systemTime, oper, release);
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Erzeuge ersten Ablaufplan (mit Hilfe der KOZ-Regel).        */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: MACHINES                              */
/*                                                                      */
/* Aufgerufene Funktionen: void GetKozOperation()                       */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void GetFirstSchedule()
{
    int systemTime = 0;
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        MACHINES[i]->m_activeJobOrder.RemoveAll();
        new TMachineInfo;
    }

    // Erste Warteschlangen erstellen.
    for (int jobID = 0; jobID < JOBS.GetSize(); ++jobID)
    {
        const TJob* pJob = JOBS[jobID];
        AddOper(systemTime, TOper(jobID, 0), JOBS[jobID]->m_release);
    }

    // Alle Warteschlangen abarbeiten.
    while (true)
    {
        ASSERT(MC_HEAP[0]->m_timeToChange >= systemTime);
        systemTime = MC_HEAP[0]->m_timeToChange;
        if (systemTime == INT_MAX) break;

        TMachineInfo* pMi = const_cast<TMachineInfo*>(MC_HEAP[0]);
        TOper oper = pMi->Change();
        if (oper.IsEmpty()) continue;

        // Ablaufplan aktualisieren.
        MACHINES[oper.m_machineID]->m_activeJobOrder.Add(oper);

        // Nachfolgebearbeitung in Warteschlange der Nachfolgemaschine einfÅgen.
        if (oper.m_bLast) continue;

        TOper nextOper(oper.m_jobID, oper.m_nodeID + 1);
        AddOper(systemTime, nextOper, systemTime + oper.m_processingTime);
    }

    MC_HEAP.RemoveAll();
    MC_ARRAY.DestroyAll();
}

/************************************************************************/
/*                                                                      */
/*            Erzeugung des erweiterten disjunktiven Graphen            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*          - Kenne Ablaufplan (s.o.)                                   */
/*            - Kenne Maschinenfolge mit JOBS[position].Next_node ...   */
/*            - Benîtige noch die entsprechenden Bearbeitungszeiten,    */
/*            VorgÑnger- und Nachfolgermaschine                         */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* Aufgabe: Erzeuge disjunktiven Graphen.                               */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: GRAPH                                 */
/*                                                                      */
/* Aufgerufene Funktionen:                                              */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void ConstructGraph()
{  // Trage zunÑchst die Auftragsreihenfolgen ab.
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];
        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {  // Erzeuge graph_node Element
            TGraphNode* pNode = new TGraphNode(pMachine->m_activeJobOrder[j], j);
            pMachine->m_graph.Add(pNode);
        }
    }

    // Nachfolgebearbeitungen
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];
        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {
            const TOper& oper = pMachine->m_activeJobOrder[j];
            TJob* pJob = JOBS[oper.m_jobID];

            // NÑchste Bearbeitung des Auftrages.
            if (oper.m_bLast) continue;

            TGraphNode* pNode = pMachine->m_graph[j];
            TOper nextOper(oper.m_jobID, oper.m_nodeID + 1);
            TMachine* pMachineNext = MACHINES[nextOper.m_machineID];

            // Benîtige nun entsprechenden Knoten der nÑchsten
            // Bearbeitung in GRAPH.
            for (int h = 0; h < pMachineNext->GetOperCount(); ++h)
                if (*pMachineNext->m_graph[h] == nextOper)
                {
                    pNode->m_pNodeSucc = pMachineNext->m_graph[h];
                    pMachineNext->m_graph[h]->m_pNodePred = pNode;
                    break;
                }
        }
    }
}

/************************************************************************/
/*                                                                      */
/*  Im folgenden werden einige Funktionen angegeben die zur Erstellung  */
/*  und zum Loeschen einer einfachen Liste benoetigt werden.            */
/*                                                                      */
/************************************************************************/

namespace {
CSmartArray<TGraphNode> LPQueue;
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: LÑngste Wege Algorithmus initialisieren.                    */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: GRAPH[][]                             */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: Anzahl der Operationen                                */
/*                                                                      */
/************************************************************************/

int InitLongestPath()
{
    int count = 0;
    LPQueue.RemoveAll();

    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];
        count += pMachine->GetOperCount();

        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {  // Maschinenausfallzeiten hier berÅcksichtigen.
            TGraphNode* pNode = pMachine->m_graph[j];
            TJob* pJob = JOBS[pNode->m_jobID];

            pNode->m_longestPath = 0;
            pNode->m_pNodePredLP = NULL;
            pNode->m_inputCounter = 0;

            // "Auftragsausfallzeiten" ebenfalls hier berÅcksichtigen.
            if (pNode->m_longestPath < pJob->m_release) pNode->m_longestPath = pJob->m_release;
            if (pNode->m_longestPath < pMachine->m_release) pNode->m_longestPath = pMachine->m_release;
        }
    }

    return count;
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Findet die laengsten Wege und schreibt sie in die globale   */
/*          Variable GRAPH[][]. Fuer jeden Knoten wird jeweils der      */
/*          Vorgaengerknoten und der laengste Weg bis zum Knoten ange-  */
/*          geben.                                                      */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Aufgerufene Funktionen: InsertNode()                                 */
/*                         RemoveNode()                                 */
/*                         InitLongestPath()                            */
/*                                                                      */
/* Rueckgabewert: true if OK, false if cycle                            */
/*                                                                      */
/************************************************************************/

bool FindLongestPath()
{  // Initialisierung (count = Anzahl der Operationen)
    int count = InitLongestPath();

    // Bestimme fuer jeden Knoten den Eingangsgrad
    // Der Eingang vom Startknoten wird vernachlaessigt.
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];

        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {  // Nachfolgemaschine
            TGraphNode* pNode = pMachine->m_graph[j];
            TGraphNode* pNodeSucc = pNode->m_pNodeSucc;

            // Nachfolgeauftrag
            if (j > 0) ++(pNode->m_inputCounter);

            if (pNodeSucc != NULL) ++(pNodeSucc->m_inputCounter);
        }
    }

    // First_Node ist der Beginn einer Liste, in der alle Knoten stehen,
    // die Eingangsgrad Null haben.
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];
        TGraphNode* pNode = pMachine->m_graph[0];

        if (pNode->m_inputCounter == 0) LPQueue.Add(pNode);
    }

    // Hauptteil der Berechnung der laengsten Wege beginnt jetzt.
    while (!LPQueue.IsEmpty())
    {  // Test auf Zyklus
        count--;
        if (count < 0) return false;

        int index = LPQueue.GetSize() - 1;
        TGraphNode* pNode = LPQueue[index];
        LPQueue.RemoveAt(index);

        int release = pNode->m_longestPath + pNode->m_processingTime;
        TGraphNode* pNodeSucc = pNode->m_pNodeSucc;

        if (pNodeSucc != NULL)
        {
            if (--pNodeSucc->m_inputCounter == 0) LPQueue.Add(pNodeSucc);

            // Laengen vergleichen
            if (pNodeSucc->m_longestPath < release)
            {
                pNodeSucc->m_pNodePredLP = pNode;
                pNodeSucc->m_longestPath = release;
            }
        }

        int machineID = pNode->m_machineID;
        int position = pNode->m_position;
        TMachine* pMachine = MACHINES[machineID];

        if (position >= pMachine->GetOperCount() - 1) continue;

        TGraphNode* pNodeNext = pMachine->m_graph[position + 1];
        if (--pNodeNext->m_inputCounter == 0) LPQueue.Add(pNodeNext);

        // Laengen vergleichen
        if (pNodeNext->m_longestPath < release)
        {
            pNodeNext->m_pNodePredLP = pNode;
            pNodeNext->m_longestPath = release;
        }
    }

    return count == 0;
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Speichere die lÑngsten Wege ab.                             */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: SAVE_LP[]                             */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void SaveLP()
{
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];
        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {
            TGraphNode* pNode = pMachine->m_graph[j];
            pNode->m_savedLP = pNode->m_longestPath;
        }
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Berechne Kosten.                                            */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void GetResult()
{
    CANDIDATE_RESULT = 0;

    int maxCost = -1;
    int sumCompletionTime = 0;

    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];

        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {
            TGraphNode* pNode = pMachine->m_graph[j];

            // Letzte Bearbeitung
            if (pNode->m_pNodeSucc == NULL)
            {
                int completionTime = pNode->m_longestPath + pNode->m_processingTime;
                int jobID = pNode->m_jobID;
                const TJob* pJob = JOBS[jobID];

                sumCompletionTime += completionTime;
                if (completionTime > pJob->m_due)
                {
                    int cost = pJob->m_weight * (completionTime - pJob->m_due);
                    CANDIDATE_RESULT += cost;

                    if (cost > maxCost)
                    {
                        maxCost = cost;
                        LAST_NODE = pNode;
                    }
                }
            }
        }
    }

    if (OBJECTIVE_FUNCTION == M_S) CANDIDATE_RESULT = maxCost;

    if (CANDIDATE_RESULT < BEST_RESULT || CANDIDATE_RESULT == 0)
    {
        SaveConfiguration();
        WriteScheduleFile();

        if (CANDIDATE_RESULT == 0)
        {
            printf("Optimal value (=0)!!\nExit Program!!\n");
            exit(0);
        }
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Speicher freigeben                                          */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void FreeMemory()
{
    JOBS.DestroyAll();
    MACHINES.DestroyAll();
}
