#include "StdAfx.h"

#include "Datei.h"
#include "Def.h"
#include "Metropol.h"
#include "Neighbor.h"
#include "Start.h"

/************************************************************************/
/*                                                                      */
/* Aufgabe: Transformiere erste L”sung der KOZ-Regel in Graph.          */
/*                                                                      */
/* Parameter: -                                                         */
/*                                                                      */
/* Veraenderte globale Variablen: -                                     */
/*                                                                      */
/* Aufgerufene Funktionen: void ConstructGraph() - START.CPP            */
/*                         int FindLongestPath() - START.CPP            */
/*                         void GetResult()      - START.CPP            */
/*                                                                      */
/* Rueckgabewert: -                                                     */
/*                                                                      */
/************************************************************************/

void GetFirstGraphSchedule()
{  // Erzeuge disjunktiven Graphen.
    ConstructGraph();

    // Bestimme l„ngste Wege
    if (!FindLongestPath())
    {
        printf("Cycling occurs!!\n");
        exit(1);
    }

    // Bestimme Durchlaufzeiten
    GetResult();
    printf("\nFirst Solution: %lu\n", CANDIDATE_RESULT);

    ACT_RESULT = CANDIDATE_RESULT;
    BEST_RESULT = CANDIDATE_RESULT;

    // Speichere diese L”sung als bisher beste ab.
    SaveConfiguration();
    WriteScheduleFile();
    SaveLP();

    // Bestimme kritische Kanten
    GetCriticalEdges();
}

void s_t()
{
    for (int j = 0; j < JOBS.GetSize(); ++j) JOBS[j]->m_weight = 1;
}

void w_c()
{
    for (int j = 0; j < JOBS.GetSize(); ++j) JOBS[j]->m_due = 0;
}

void s_c()
{
    for (int j = 0; j < JOBS.GetSize(); ++j)
    {
        JOBS[j]->m_weight = 1;
        JOBS[j]->m_due = 0;
    }
}

void ChangeResultToFlowtime()
{
    int sum = 0;

    for (int j = 0; j < JOBS.GetSize(); ++j) sum += JOBS[j]->m_weight * JOBS[j]->m_release;

    printf("\nFinal Result: %d\n", BEST_RESULT - sum);
}

int main(int argc, char* argv[])
{
    ALGORITHM = PSG;
    TEMPERATURE = 250;
    ACCEPT_RATE = 0.25;
    NEIGHBORHOOD = 5;

    int i1, i2, i3;
    GetMaxValues(i1, i2, i3);

    OBJECTIVE_FUNCTION = argc > 1 ? TObjective(atoi(argv[1])) : M_S;

    switch (OBJECTIVE_FUNCTION)
    {
        case W_T:
            printf("Objective Function: Total Weighted Tardiness\n");
            break;
        case W_C:
            printf("Objective Function: Total Weighted Flow Time\n");
            break;
        case S_T:
            printf("Objective Function: Total Tardiness\n");
            break;
        case S_C:
            printf("Objective Function: Total Flow Time\n");
            break;
        case M_S:
            printf("Objective Function: Makespan\n");
            break;
        default:
            printf("Incorrect Objective Function.\n");
            exit(1);
    }

    STAT_TIME = argc > 2 ? atoi(argv[2]) : 60;

    ReadMachineFile();
    ReadJobFile();

    // Modify weights and due_dates according to the objective function
    switch (OBJECTIVE_FUNCTION)
    {
        case S_T:  // Sum of tardiness
            s_t();
            break;
        case W_C:  // Weighted completion time
            w_c();
            break;
        case S_C:  // Sum of completion time
        case M_S:  // Makespan
            s_c();
            break;
    }

    // Bestimme ersten Ablaufplan mit Hilfe der KOZ-Regel.
    GetFirstSchedule();

    // Bestimme erste L”sung in Graphenform
    GetFirstGraphSchedule();

    switch (ALGORITHM)
    {
        case 1:
            metropolis();
            break;
        case 4:
            psg();
            break;
    }

    if (OBJECTIVE_FUNCTION == W_C) ChangeResultToFlowtime();

    printf("\nProgram finished!\n");

    FreeMemory();
    return 0;
}
