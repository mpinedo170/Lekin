/************************************************************************/
/*                                                                      */
/*     Prozeduren zur Bestimmung der Nachbarschaft eines Zustandes.     */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "Datei.h"
#include "Def.h"
#include "Neighbor.h"
#include "Start.h"

TGraphNode* NODE;
TGraphNode* U;
TGraphNode* V;
int X_U;
int F_V;

/************************************************************************/
/*                                                                      */
/* Aufgabe: Markiere und speichere Knoten, die sich auf einem lÑngsten  */
/*          Pfad befinden (nur kritische Kanten).                       */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: GRAPH[][]                             */
/*                    CRITICAL_EDGES[]                                  */
/*                NR_OF_CRITICAL_EDGES                                  */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void GetCriticalEdges()
{  // Lîsche Markierungen bei GRAPH.
    for (int i = 0; i < MACHINES.GetSize(); ++i) MACHINES[i]->ClearMark();
    CRITICAL_EDGES.RemoveAll();

    // Gehe, ausgehend von den letzten Bearbeitungen, die lÑngsten
    // Auftragswege rÅckwÑrts durch.
    for (int i = 0; i < MACHINES.GetSize(); ++i)
    {
        TMachine* pMachine = MACHINES[i];

        for (int j = 0; j < pMachine->GetOperCount(); ++j)
        {
            TGraphNode* pNode = pMachine->m_graph[j];

            // Letzte Bearbeitung und Verspaetung gegenueber
            // dem vorgeschriebenen Fertigstellungszeitpunkt
            if (pNode->m_pNodeSucc == NULL &&
                pNode->m_savedLP + pNode->m_processingTime > JOBS[pNode->m_jobID]->m_due)
            {  // Betrachte VorgÑnger des lÑngsten Weges.
                TGraphNode* pNodePredLP = pNode->m_pNodePredLP;

                while (pNodePredLP != NULL)
                {
                    if (pNodePredLP->m_machineID == pNode->m_machineID &&
                        pNodePredLP->m_jobID != pNode->m_jobID && !pNodePredLP->m_bMarked)
                    {
                        CRITICAL_EDGES.Add(pNodePredLP);
                        // Markiere Knoten als besucht.
                        pNodePredLP->m_bMarked = true;
                    }
                    pNode = pNode->m_pNodePredLP;
                    pNodePredLP = pNode->m_pNodePredLP;
                }
            }
        }
    }

    if (CRITICAL_EDGES.IsEmpty())
    {
        printf("Optimal value (no critical edges)!!\nExit Program!!\n");
        exit(0);
    }

    // Reihenfolge in CANDIDATE_EDGES[] umdrehen.
    for (int i = 0; i < CRITICAL_EDGES.GetSize() / 2; ++i)
    {
        int i2 = CRITICAL_EDGES.GetSize() - 1 - i;
        TGraphNode* pNodeTemp = CRITICAL_EDGES[i];
        CRITICAL_EDGES[i] = CRITICAL_EDGES[i2];
        CRITICAL_EDGES[i2] = pNodeTemp;
    }
}

void GetCriticalEdgesFromHighestImpactJob()
{
    CRITICAL_EDGES.RemoveAll();

    // Gehe, ausgehend von der letzten Bearbeitung den lÑngsten
    // Auftragsweg rÅckwÑrts durch.
    // Betrachte VorgÑnger des lÑngsten Weges.
    TGraphNode* pNode = LAST_NODE;
    TGraphNode* pNodePredLP = pNode->m_pNodePredLP;

    while (pNodePredLP != NULL)
    {
        if (pNodePredLP->m_machineID == pNode->m_machineID && pNodePredLP->m_jobID != pNode->m_jobID)
        {
            CRITICAL_EDGES.Add(pNodePredLP);
            // Markiere Knoten als besucht.
            pNodePredLP->m_bMarked = true;
        }
        pNode = pNode->m_pNodePredLP;
        pNodePredLP = pNode->m_pNodePredLP;
    }

    // Reihenfolge in CANDIDATE_EDGES[] umdrehen.
    for (int i = 0; i < CRITICAL_EDGES.GetSize() / 2; ++i)
    {
        int i2 = CRITICAL_EDGES.GetSize() - 1 - i;
        TGraphNode* pNodeTemp = CRITICAL_EDGES[i];
        CRITICAL_EDGES[i] = CRITICAL_EDGES[i2];
        CRITICAL_EDGES[i2] = pNodeTemp;
    }

    if (CRITICAL_EDGES.IsEmpty()) GetCriticalEdges();
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Zufaellige Auswahl einer Kante aus CRITICAL_EDGES[]         */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: TGraphNode* CANDIDATE                 */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void SelectCandidateFromCriticalEdges()
{  // Waehle zufaellig einen Knoten aus {0, ..., CRITICAL_EDGES.GetSize()-1}
    PLACE = TRandom::Get().Uniform(CRITICAL_EDGES.GetSize());
    CANDIDATE = CRITICAL_EDGES[PLACE];
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Bestimme Zielwert von CANDIDATE                             */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: int FindLongestPath() - START.CPP          */
/*                           void GetResult()       - START.CPP        */
/*                           void neighborhood_n1()                     */
/*                           void neighborhood_n2()                     */
/*                           void neighborhood_n4()                     */
/*                           void neighborhood_n5()                     */
/*                                                                      */
/* Rueckgabewert: true if OK, false if cycle                            */
/*                                                                      */
/************************************************************************/

bool GetCandidateResult()
{  // Transition entsprechend der gewÑhlten Nachbarschaftsstruktur
    switch (NEIGHBORHOOD)
    {
        case 1:
            neighborhood_n1();
            break;
        case 5:
            neighborhood_n5();
            break;
    }

    // Zielwert des Nachbarn bestimmen
    if (!FindLongestPath()) return false;

    GetResult();
    return true;
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Nachbarschaftsstruktur N1                                   */
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

void neighborhood_n1()
{  // Position von CANDIDATE in der Auftragsreihenfolge
    CANDIDATE->SwapForward();
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Transition von Nachbarschaftsstruktur N1 annulieren         */
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

void restore_n1()
{  // Position von CANDIDATE in der Auftragsreihenfolge
    CANDIDATE->SwapBack();
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Nachbarschaftsstruktur N5                                   */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: int step1()                                  */
/*                         int step2()                                  */
/*                         int step3()                                  */
/*                         int step4()                                  */
/*                         int step5()                                  */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void neighborhood_n5()
{
    NODE_2 = NODE_3 = NULL;

    int psg = step1();
    if (psg == 5) goto STEP5;

    psg = step2();
    if (psg == 5) goto STEP5;

STEP3:
    psg = step3();
    if (psg == 5) goto STEP5;

    psg = step4();
    if (psg == 3) goto STEP3;

STEP5:
    psg = step5();
}

/************************************************************************/
/*                                                                      */
/*                step1 - step2 - step3 - step4 - step5                 */
/*                                                                      */
/************************************************************************/

int step1()
{
    int pos = CANDIDATE->m_position;
    TMachine* pMachine = MACHINES[CANDIDATE->m_machineID];

    U = pMachine->m_graph[pos];
    X_U = U->m_savedLP;

    V = pMachine->m_graph[pos + 1];
    F_V = V->m_savedLP + V->m_processingTime;

    neighborhood_n1();

    // node = v
    TGraphNode* pNode = V;

    // NODE = a(v)
    NODE = pNode->m_pNodePred;
    return NODE == NULL ? 5 : 0;
}

int step2()
{  // NODE = v'
    return NODE->m_savedLP + NODE->m_processingTime < X_U ? 5 : 0;
}

int step3()
{
    int pos = NODE->m_position;
    if (pos == 0) return 0;

    // node = b(v')   NODE = v'
    TMachine* pMachine = MACHINES[NODE->m_machineID];
    TGraphNode* pNode = pMachine->m_graph[pos - 1];
    if (pNode->m_savedLP + pNode->m_processingTime < NODE->m_savedLP) return 0;

    // Vertausche node mit Nachfolgerknoten NODE
    NODE_2 = pNode;
    NODE_2->SwapForward();
    return 5;
}

int step4()
{  // node = a(v')    NODE = v'
    TGraphNode* pNode = NODE->m_pNodePred;
    if (pNode == NULL) return 5;

    NODE = pNode;
    return 3;
}

int step5()
{  // node = u   node1 = v*/
    TGraphNode* pNode = U;
    TGraphNode* pNode1 = V;

    // Nachfolger von node
    if (pNode->m_pNodeSucc == NULL) return 0;

    // node = c(u)
    pNode = pNode->m_pNodeSucc;
    if (pNode->m_savedLP >= F_V) return 0;

    // node1 = d(c(u))
    int pos = pNode->m_position;
    TMachine* pMachine = MACHINES[pNode->m_machineID];
    if (pos >= pMachine->GetOperCount() - 1) return 0;

    pNode1 = pMachine->m_graph[pos + 1];
    if (pNode1->m_savedLP != pNode->m_savedLP + pNode->m_processingTime) return 0;

    // Vertausche node mit node1
    NODE_3 = pNode;
    NODE_3->SwapForward();
    return 5;
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Transitionen von Nachbarschaftsstruktur N5 annulieren       */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: void restore_n1()                            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void restore_n5()
{
    restore_n1();

    if (NODE_2 != NULL) NODE_2->SwapBack();
    if (NODE_3 != NULL) NODE_3->SwapBack();
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: UrsprÅnglichen Graphen wieder herstellen.                   */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: restore_n1()                                 */
/*                         restore_n5()                                 */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/************************************************************************/

void TurnBackChangedEdge()
{  // RÅcknahme der Transition (abhÑngig von der Nachbarschaftsstruktur)
    switch (NEIGHBORHOOD)
    {
        case 1:
            restore_n1();
            break;
        case 5:
            restore_n5();
            break;
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Beste Lîsung (Ablaufplan) abspeichern.                      */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: true oder false                                       */
/************************************************************************/

void SaveConfiguration()
{
    for (int i = 0; i < MACHINES.GetSize(); ++i) MACHINES[i]->SaveConfiguration();
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Beste Lîsung (Ablaufplan) als aktuell betrachten.           */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: true oder false                                       */
/************************************************************************/

void ReadBestPlan()
{
    for (int i = 0; i < MACHINES.GetSize(); ++i) MACHINES[i]->ReadBestPlan();
}
