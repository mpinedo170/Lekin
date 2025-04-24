/************************************************************************/
/*                                                                      */
/*           Implementierung des Metropolis-Algorithmuss                */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "Datei.h"
#include "Def.h"
#include "Metropol.h"
#include "Neighbor.h"
#include "Start.h"

/************************************************************************/
/*                                                                      */
/*         Hilfsfunktionen fÅr den Metropolis-Algorithmus               */
/*                                                                      */
/************************************************************************/

bool CheckNeighborhood()
{
    if (NEIGHBORHOOD != 5) exit(1);

    // Transition rÅckgÑngig machen.
    return false;
}

void AcceptCandidate()
{
    SaveLP();
    ACT_RESULT = CANDIDATE_RESULT;

    // Neuer bester Zielwert
    if (ACT_RESULT < BEST_RESULT)
    {
        BEST_RESULT = ACT_RESULT;
        // SaveConfiguration();
        printf("Large Step: %u\n", BEST_RESULT);
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Optimierung mittels Metropolis-Algorithmus                  */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: bool GetCandidateResult() - NEIGHBOR.CPP     */
/*             void SelectCandidateFromCriticalEdges() - NEIGHBOR.CPP   */
/*                         bool AcceptNewConfiguration()                */
/*                         void TurnBackChangedEdge() - NEIGHBOR.CPP    */
/*                         void GetCriticalEdges()  - NEIGHBOR.CPP      */
/*             void PrepareLocalSearch()                                */
/*             void LocalSearch()                                       */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void metropolis()
{
    ITERATION = ACCEPT = BAD_ITER = 0;
    SaveLP();

    bool bAbort = false;
    time_t t1 = time(NULL);

    while (!bAbort)
    {
        ++ITERATION;

        // WÑhle zufÑlligen Nachbarszustand
        SelectCandidateFromCriticalEdges();
        bool bOk = GetCandidateResult();

        // Wird neue Konfiguration akzeptiert?
        bool bChange = bOk ? AcceptNewConfiguration() : CheckNeighborhood();

        if (bChange)
        {  // Ja:   Aenderung akzeptieren
            AcceptCandidate();
            // Neuer Graph liegt vor -> bestimme kritische Kanten
            // GetCriticalEdges();
            GetCriticalEdgesFromHighestImpactJob();
        }
        else  // Nein: Aenderung zuruecknehmen
            TurnBackChangedEdge();

        if (ITERATION % 100 == 0)
        {
            time_t t2 = time(NULL);
            double runtime = difftime(t2, t1);

            if (runtime > double(STAT_TIME)) bAbort = true;

            // Temperatur an gewÅnschte Akzeptanzrate angleichen.
            double multi = ACCEPT == 0 || BAD_ITER == 0 ? 1.0 : ACCEPT_RATE / (double(ACCEPT) / BAD_ITER);

            TEMPERATURE *= multi;
            ACCEPT = BAD_ITER = 0;
        }
    }

    if (ALGORITHM == METROPOLIS)
    {
        PrepareLocalSearch();
        LocalSearch();
        printf("Best result: %u\n", BEST_RESULT);
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Neue Loesung akzeptieren: ja oder nein                      */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: -                                            */
/*                                                                      */
/* Rueckgabewert: true oder false                                       */
/************************************************************************/

bool AcceptNewConfiguration()
{  // Wahrscheinlichkeit einen schlechteren Zustand zu akzeptieren
    if (CANDIDATE_RESULT - ACT_RESULT <= 0) return true;  // Keine Verschlechterung -> immer akzeptieren

    ++BAD_ITER;
    double prop = exp((ACT_RESULT - CANDIDATE_RESULT) / TEMPERATURE);
    double u = TRandom::Get().Uniform();

    if (u < prop)
    {
        ++ACCEPT;
        return true;
    }

    return false;
}

/************************************************************************/
/*                                                                      */
/*                Implementierung von lokaler Suche                     */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* Aufgabe: Optimierung mittels Lokaler Suche                           */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: bool GetCandidateResult() - NEIGHBOR.CPP     */
/*             void SelectCandidateFromCriticalEdges()                  */
/*                                                     - NEIGHBOR.CPP   */
/*                         void TurnBackChangedEdge() - NEIGHBOR.CPP    */
/*                         void GetCriticalEdges() - NEIGHBOR.CPP       */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void LocalSearch()
{
    if (CRITICAL_EDGES.IsEmpty()) return;

    bool bAbort = false;

    while (!bAbort)
    {
        int nr = 0;
        // WÑhle zufÑlligen Nachbarszustand
        SelectCandidateFromCriticalEdges();

        // Suche einen Nachbarn, der zu einer Verbesserung fÅhrt.
        while (nr <= CRITICAL_EDGES.GetSize())
        {
            ++nr;
            bool bOk = GetCandidateResult();

            if (!bOk)
            {
                CheckNeighborhood();

                // Transition rÅckgÑngig machen.
                TurnBackChangedEdge();

                // NÑchsten Kandidaten bestimmen
                // Gehe dazu CRITICAL_EDGES um eins weiter.
                if (++PLACE == CRITICAL_EDGES.GetSize()) PLACE = 0;
                CANDIDATE = CRITICAL_EDGES[PLACE];
            }
            else if (CANDIDATE_RESULT >= ACT_RESULT)
            {  // Wird neue Konfiguration akzeptiert?
                // Nein

                // énderung rÅckgÑngig machen
                TurnBackChangedEdge();

                // NÑchsten Kandidaten bestimmen
                // Gehe dazu CRITICAL_EDGES um eins weiter.
                if (++PLACE == CRITICAL_EDGES.GetSize()) PLACE = 0;
                CANDIDATE = CRITICAL_EDGES[PLACE];
            }
            else
            {  // Ja
                SaveLP();
                ACT_RESULT = CANDIDATE_RESULT;

                // Neuer bester Zielwert
                if (ACT_RESULT < BEST_RESULT)
                {
                    BEST_RESULT = ACT_RESULT;
                    // SaveConfiguration();
                    printf("Small Step: %u\n", BEST_RESULT);
                }

                // Neuer Graph liegt vor -> bestimme kritische Kanten
                if (OBJECTIVE_FUNCTION != M_S)
                    GetCriticalEdges();
                else
                    GetCriticalEdgesFromHighestImpactJob();
                nr = CRITICAL_EDGES.GetSize() + 1;
            }

            if (nr == CRITICAL_EDGES.GetSize() || CRITICAL_EDGES.IsEmpty()) bAbort = true;
        }
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Vorbereitung fÅr die lokale Suche - bilde dazu die bisher   */
/*          beste gefundene Lîsung in disjunktiven Graphen ab.          */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: void ConstructGraph()    - START.CPP         */
/*                         int FindLongestPath()   - START.CPP          */
/*             void GetCriticalEdges() - NEIGHBOR.CPP                   */
/*             void ReadBestPlan()  - NEIGHBOR.CPP                      */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void PrepareLocalSearch()
{  // Bisher besten Ablaufplan als aktuellen einlesen.
    ReadBestPlan();

    FreeMemory();

    // Erzeuge disjunktiven Graphen.
    ConstructGraph();

    // Bestimme lÑngste Wege
    if (!FindLongestPath())
    {
        printf("Cycle occurs during creation of the start solution!!\n");
        exit(1);
    }

    ACT_RESULT = BEST_RESULT;

    // Bestimme kritische Kanten
    if (OBJECTIVE_FUNCTION != M_S)
        GetCriticalEdges();
    else
        GetCriticalEdgesFromHighestImpactJob();
}

/************************************************************************/
/*                                                                      */
/*                Implementierung eines eigenen Verfahrens              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* Aufgabe: Eigenes Optimierungsverfahren (sucht lokale Minima)         */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: void SaveLP()                                */
/*             void LocalSearch()                                       */
/*             void metropolis()                                        */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void psg()
{
    SaveLP();
    ITERATION = ACCEPT = BAD_ITER = 0;

    bool bAbort = false;
    int minIter = 0;
    int maxIter = 0;
    int maxLocalMin = 400;
    int localMinCounter = 0;
    int activeBest = ACT_RESULT;
    time_t t1 = time(NULL);

    while (!bAbort)
    {
        LocalSearch();
        if (ACT_RESULT < activeBest) activeBest = ACT_RESULT;

        if (ACT_RESULT > activeBest * 1.1)
        {
            minIter = 10;
            maxIter = 100;
        }

        if (ACT_RESULT <= activeBest * 1.1 || ACT_RESULT <= activeBest + 20)
        {
            minIter = 10;
            maxIter = 40;
        }

        if (ACT_RESULT == activeBest)
        {
            minIter = 5;
            maxIter = 20;
        }

        if (ACT_RESULT > activeBest)
            ++localMinCounter;
        else
            localMinCounter = 0;

        time_t t2 = time(NULL);
        double runtime = difftime(t2, t1);
        if (runtime > double(STAT_TIME)) bAbort = true;

        ++ITERATION;
        int iter = 0;
        bool bVeryLargeStep = false;

        // Very large step, falls maxLocalMin keine Verbesserung auftritt.
        if (localMinCounter >= maxLocalMin)
        {
            printf("***** Very large step *****\n");
            minIter = 1000;
            bVeryLargeStep = true;
            TEMPERATURE *= TEMPERATURE;
        }

        int metroIter = TRandom::Get().Uniform(maxIter) + minIter;

        while (iter < metroIter)
        {  // WÑhle zufÑlligen Nachbarszustand
            SelectCandidateFromCriticalEdges();
            bool bOk = GetCandidateResult();

            // Wird neue Konfiguration akzeptiert?
            bool bChange = bOk ? AcceptNewConfiguration() : CheckNeighborhood();

            if (bChange)
            {  // Ja:   Aenderung akzeptieren
                AcceptCandidate();
                // Neuer Graph liegt vor -> bestimme kritische Kanten
                GetCriticalEdgesFromHighestImpactJob();
            }
            else  // Nein: Aenderung zuruecknehmen
                TurnBackChangedEdge();

            if (ITERATION % 50 == 0 && !bVeryLargeStep)
            {
                t2 = time(NULL);
                runtime = difftime(t2, t1);
                if (runtime > STAT_TIME) bAbort = true;

                // Temperatur an gewÅnschte Akzeptanzrate angleichen.
                double multi = ACCEPT == 0 || BAD_ITER == 0 ? 1.0 : ACCEPT_RATE / (double(ACCEPT) / BAD_ITER);

                TEMPERATURE = TEMPERATURE * multi;
                ACCEPT = BAD_ITER = 0;
            }
            ++iter;
            ++ITERATION;
        }

        if (bVeryLargeStep)
        {
            bVeryLargeStep = false;
            TEMPERATURE /= 10;
            localMinCounter = 0;
            activeBest = ACT_RESULT;
        }
    }
}
