/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                      N E I G H B O R . C P P                         */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "def.h"
#include "graph.h"
#include "neighbor.h"

struct graph_node* NODE;
struct graph_node *U, *V;
int X_U, F_V;
long COUNT1 = 0;
long COUNT21 = 0;
long COUNT22 = 0;
long COUNT31 = 0;
long COUNT32 = 0;

extern double Random();

void get_critical_edges()
{
    int i, stage_nr, j_nr;
    struct graph_node *pred_lp, *node;
    int save_critical_edges[MAX_STAGES * MAX_JOB_NR][2], lp;

    /* Lösche Markierungen bei GRAPH. */
    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++) GRAPH[stage_nr][j_nr]->Mark = NO;

    NR_OF_CRITICAL_EDGES = 0;

    /* Gehe, ausgehend von den letzten Bearbeitungen, die längsten */
    /* Auftragswege rückwärts durch.                               */
    for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
    {
        lp = GRAPH[STAGES][j_nr]->Longest_Path + GRAPH[STAGES][j_nr]->Processing_Time;
        /* Letzte Bearbeitung und Verspaetung gegenueber */
        /* dem vorgeschriebenen Fertigstellungszeitpunkt */
        if (lp > JOBS[j_nr].Due_Date)
        {
            /* Betrachte Vorgänger des längsten Weges. */
            node = GRAPH[STAGES][j_nr];
            pred_lp = NULL;
            pred_lp = GRAPH[STAGES][j_nr]->Pred_Operation_LP;

            while (pred_lp != NULL)
            {
                if (((pred_lp->Machine_Nr == node->Machine_Nr) && (pred_lp->Job_Nr != node->Job_Nr)) &&
                    (pred_lp->Mark == NO))
                {
                    /* Memorize stage- and jobnumber */
                    CRITICAL_EDGES[NR_OF_CRITICAL_EDGES][0] = pred_lp->Stage_Nr;
                    CRITICAL_EDGES[NR_OF_CRITICAL_EDGES][1] = pred_lp->Job_Nr;

                    NR_OF_CRITICAL_EDGES++;
                    /* Markiere Knoten als besucht. */
                    pred_lp->Mark = YES;
                }
                node = node->Pred_Operation_LP;
                pred_lp = node->Pred_Operation_LP;
            }
        }
    }

    if (NR_OF_CRITICAL_EDGES == 0)
    {
        // printf("\nNR_OF_CRITICAL_EDGES = 0!! Stop programm!!");
        // control ();
        // exit (0);
        return;
    }

    /* Reihenfolge in CANDIDATE_EDGES [] umdrehen. */
    for (i = 0; i < NR_OF_CRITICAL_EDGES; i++)
    {
        save_critical_edges[i][0] = CRITICAL_EDGES[(NR_OF_CRITICAL_EDGES - 1) - i][0];
        save_critical_edges[i][1] = CRITICAL_EDGES[(NR_OF_CRITICAL_EDGES - 1) - i][1];
    }
    for (i = 0; i < NR_OF_CRITICAL_EDGES; i++)
    {
        CRITICAL_EDGES[i][0] = save_critical_edges[i][0];
        CRITICAL_EDGES[i][1] = save_critical_edges[i][1];
    }
}

void get_critical_edges_from_highest_impact_job()
{
    int i, j_nr;
    struct graph_node *pred_lp, *node;
    int save_critical_edges[MAX_STAGES * MAX_JOB_NR][2];

    NR_OF_CRITICAL_EDGES = 0;

    /* Gehe, ausgehend von der letzten Bearbeitung den längsten */
    /* Auftragsweg rückwärts durch.                             */
    /* Betrachte Vorgänger des längsten Weges.                  */
    // node = LAST_NODE;
    j_nr = LAST_NODE->Job_Nr;
    node = GRAPH[STAGES][j_nr];
    pred_lp = NULL;
    pred_lp = GRAPH[STAGES][j_nr]->Pred_Operation_LP;

    while (pred_lp != NULL)
    {
        if ((pred_lp->Machine_Nr == node->Machine_Nr) && (pred_lp->Job_Nr != node->Job_Nr))
        {
            /* Memorize stage- and jobnumber */
            CRITICAL_EDGES[NR_OF_CRITICAL_EDGES][0] = pred_lp->Stage_Nr;
            CRITICAL_EDGES[NR_OF_CRITICAL_EDGES][1] = pred_lp->Job_Nr;

            NR_OF_CRITICAL_EDGES++;
        }
        node = node->Pred_Operation_LP;
        pred_lp = node->Pred_Operation_LP;
    }

    /* Reihenfolge in CANDIDATE_EDGES [] umdrehen. */
    for (i = 0; i < NR_OF_CRITICAL_EDGES; i++)
    {
        save_critical_edges[i][0] = CRITICAL_EDGES[(NR_OF_CRITICAL_EDGES - 1) - i][0];
        save_critical_edges[i][1] = CRITICAL_EDGES[(NR_OF_CRITICAL_EDGES - 1) - i][1];
    }
    for (i = 0; i < NR_OF_CRITICAL_EDGES; i++)
    {
        CRITICAL_EDGES[i][0] = save_critical_edges[i][0];
        CRITICAL_EDGES[i][1] = save_critical_edges[i][1];
    }

    /* It is possible, that with the highest impact job the number of */
    /* critcal edges is zero.                                         */
    if (NR_OF_CRITICAL_EDGES == 0) get_critical_edges();
}

void select_candidate_from_critical_edges()
{
    double u;
    int stage_nr, j_nr;

    /* Weise u eine Zufallszahl zwischen 0 und 1 zu */
    u = Random();
    // u = rand()/RAND_MAX;

    /* Waehle zufaellig einen Knoten aus {0, ..., NR_OF_CRITICAL_EDGES-1} */
    PLACE = int(NR_OF_CRITICAL_EDGES * u);

    if (PLACE == NR_OF_CRITICAL_EDGES) PLACE = NR_OF_CRITICAL_EDGES - 1;

    stage_nr = CRITICAL_EDGES[PLACE][0];
    j_nr = CRITICAL_EDGES[PLACE][1];

    CANDIDATE = GRAPH[stage_nr][j_nr]; /* which node in GRAPH is CANDIDATE */
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Bestimme Zielwert von CANDIDATE                             */
/*                                                                      */
/************************************************************************/

int get_candidate_result()
{
    /* Transition entsprechend der gewählten Nachbarschaftsstruktur */
    switch (NEIGHBORHOOD)
    {
        case 1:
            neighborhood_n1();
            break;
        case 5:
            neighborhood_n5();
            break;
    }

    /* Zielwert des Nachbarn bestimmen */
    if (find_longest_path() == CYCLE)
        return (CYCLE);
    else
        get_result();

    return (OK);
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Nachbarschaftsstruktur N1                                   */
/*                                                                      */
/************************************************************************/

void neighborhood_n1()
{
    int j_nr, stage_nr, j_a, j_c, j_d;

    /* which job on which stage */
    j_nr = CANDIDATE->Job_Nr;
    stage_nr = CANDIDATE->Stage_Nr;

    /* a(?) - b - c - d(?)    ---->    a(?) - c - b - d(?) */
    j_a = GRAPH[stage_nr][j_nr]->Pred_Job;
    j_c = GRAPH[stage_nr][j_nr]->Succ_Job;
    j_d = GRAPH[stage_nr][j_c]->Succ_Job;

    if (j_a != 0) GRAPH[stage_nr][j_a]->Succ_Job = j_c;
    if (j_d != 0) GRAPH[stage_nr][j_d]->Pred_Job = j_nr;
    GRAPH[stage_nr][j_nr]->Pred_Job = j_c;
    GRAPH[stage_nr][j_nr]->Succ_Job = j_d; /* can be 0 */
    GRAPH[stage_nr][j_c]->Pred_Job = j_a;  /* can be 0 */
    GRAPH[stage_nr][j_c]->Succ_Job = j_nr;

    COUNT1++;
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Transition von Nachbarschaftsstruktur N1 annulieren         */
/*                                                                      */
/************************************************************************/

void restore_n1()
{
    int j_nr, stage_nr, j_a, j_c, j_d;

    /* which job on which stage */
    j_nr = CANDIDATE->Job_Nr;
    stage_nr = CANDIDATE->Stage_Nr;

    /* a(?) - c - b - d(?)    ---->    a(?) - b - c - d(?) */
    j_c = GRAPH[stage_nr][j_nr]->Pred_Job;
    j_a = GRAPH[stage_nr][j_c]->Pred_Job;
    j_d = GRAPH[stage_nr][j_nr]->Succ_Job;

    if (j_a != 0) GRAPH[stage_nr][j_a]->Succ_Job = j_nr;
    if (j_d != 0) GRAPH[stage_nr][j_d]->Pred_Job = j_c;
    GRAPH[stage_nr][j_nr]->Pred_Job = j_a; /* can be 0 */
    GRAPH[stage_nr][j_nr]->Succ_Job = j_c;
    GRAPH[stage_nr][j_c]->Pred_Job = j_nr;
    GRAPH[stage_nr][j_c]->Succ_Job = j_d; /* can be 0 */
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Nachbarschaftsstruktur N5                                   */
/*                                                                      */
/************************************************************************/

void neighborhood_n5()
{
    int psg;

    MACHINE_2 = 0;
    MACHINE_3 = 0;

    psg = step1();
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
    int stage_nr, j_nr, succ_j;

    j_nr = CANDIDATE->Job_Nr;
    stage_nr = CANDIDATE->Stage_Nr;
    succ_j = GRAPH[stage_nr][j_nr]->Succ_Job;

    U = GRAPH[stage_nr][j_nr];
    X_U = get_lp(U);

    V = GRAPH[stage_nr][succ_j];
    F_V = get_lp(V) + V->Processing_Time;

    neighborhood_n1();

    /* NODE = a(v) */
    NODE = NULL;
    if (stage_nr > 1) NODE = GRAPH[stage_nr - 1][succ_j];

    return NODE == NULL ? 5 : 0;
}

int step2()
{
    /* NODE = v' */
    if ((get_lp(NODE) + NODE->Processing_Time) < X_U)
        return (5);
    else
        return (0);
}

int step3()
{
    int j_nr, stage_nr, pred_j, j_a, j_c, j_d;
    struct graph_node* node;

    j_nr = NODE->Job_Nr;
    stage_nr = NODE->Stage_Nr;
    pred_j = GRAPH[stage_nr][j_nr]->Pred_Job;
    if (pred_j == 0) return (0);

    /* node = b(v')   NODE = v' */
    node = GRAPH[stage_nr][pred_j];
    if ((get_lp(node) + node->Processing_Time) < get_lp(NODE)) return (0);

    /* Vertausche node mit Nachfolgerknoten NODE */
    /* which job on which stage */
    j_nr = node->Job_Nr;
    stage_nr = node->Stage_Nr;

    /* a(?) - b(node) - c(NODE) - d(?)    ---->    a(?) - c - b - d(?) */
    j_a = GRAPH[stage_nr][j_nr]->Pred_Job;
    j_c = GRAPH[stage_nr][j_nr]->Succ_Job;
    j_d = GRAPH[stage_nr][j_c]->Succ_Job;

    if (j_a != 0) GRAPH[stage_nr][j_a]->Succ_Job = j_c;
    if (j_d != 0) GRAPH[stage_nr][j_d]->Pred_Job = j_nr;

    GRAPH[stage_nr][j_nr]->Pred_Job = j_c;
    GRAPH[stage_nr][j_nr]->Succ_Job = j_d; /* can be 0 */
    GRAPH[stage_nr][j_c]->Pred_Job = j_a;  /* can be 0 */
    GRAPH[stage_nr][j_c]->Succ_Job = j_nr;

    POSITION_2 = j_nr;
    MACHINE_2 = stage_nr;

    COUNT21++;

    return (5);
}

int step4()
{
    int stage_nr, j_nr;
    struct graph_node* node;

    /* node = a(v')    NODE = v' */
    node = NULL;
    j_nr = NODE->Job_Nr;
    stage_nr = NODE->Stage_Nr;
    if (stage_nr > 1) node = GRAPH[stage_nr - 1][j_nr];

    if (node == NULL) return (5);

    NODE = node;
    return (3);
}

int step5()
{
    int stage_nr, j_nr, succ_j, j_a, j_c, j_d;
    struct graph_node *node, *node1;

    /* node = u   node1 = v*/
    node = U;
    node1 = V;

    j_nr = node->Job_Nr;
    stage_nr = node->Stage_Nr;

    /* Nachfolger von node */
    if (stage_nr == STAGES) return (0);

    /* node = c(u) */
    node = GRAPH[stage_nr + 1][j_nr];
    if (get_lp(node) >= F_V) return (0);

    /* node1 = d(c(u)) */
    j_nr = node->Job_Nr;
    stage_nr = node->Stage_Nr;
    succ_j = GRAPH[stage_nr][j_nr]->Succ_Job;
    if (succ_j == 0) return (0);
    node1 = GRAPH[stage_nr][succ_j];
    if (get_lp(node1) != (get_lp(node) + node->Processing_Time)) return (0);

    /* Vertausche node mit node1 */
    j_nr = node->Job_Nr;
    stage_nr = node->Stage_Nr;

    /* a(?) - b(node) - c(node1) - d(?)    ---->    a(?) - c - b - d(?) */
    j_a = GRAPH[stage_nr][j_nr]->Pred_Job;
    j_c = GRAPH[stage_nr][j_nr]->Succ_Job;
    j_d = GRAPH[stage_nr][j_c]->Succ_Job;

    if (j_a != 0) GRAPH[stage_nr][j_a]->Succ_Job = j_c;
    if (j_d != 0) GRAPH[stage_nr][j_d]->Pred_Job = j_nr;
    GRAPH[stage_nr][j_nr]->Pred_Job = j_c;
    GRAPH[stage_nr][j_nr]->Succ_Job = j_d; /* can be 0 */
    GRAPH[stage_nr][j_c]->Pred_Job = j_a;  /* can be 0 */
    GRAPH[stage_nr][j_c]->Succ_Job = j_nr;

    POSITION_3 = j_nr;
    MACHINE_3 = stage_nr;

    COUNT31++;

    return (5);
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Transitionen von Nachbarschaftsstruktur N5 annulieren       */
/*                                                                      */
/************************************************************************/

void restore_n5()
{
    int stage_nr, j_nr, j_a, j_c, j_d;

    restore_n1();

    stage_nr = MACHINE_2;

    if (stage_nr != 0)
    {
        j_nr = POSITION_2;

        /* a(?) - c - b - d(?)    ---->    a(?) - b - c - d(?) */
        j_c = GRAPH[stage_nr][j_nr]->Pred_Job;
        j_a = GRAPH[stage_nr][j_c]->Pred_Job;
        j_d = GRAPH[stage_nr][j_nr]->Succ_Job;

        if (j_a != 0) GRAPH[stage_nr][j_a]->Succ_Job = j_nr;
        if (j_d != 0) GRAPH[stage_nr][j_d]->Pred_Job = j_c;
        GRAPH[stage_nr][j_nr]->Pred_Job = j_a; /* can be 0 */
        GRAPH[stage_nr][j_nr]->Succ_Job = j_c;
        GRAPH[stage_nr][j_c]->Pred_Job = j_nr;
        GRAPH[stage_nr][j_c]->Succ_Job = j_d; /* can be 0 */

        COUNT22++;
    }

    stage_nr = MACHINE_3;

    if (stage_nr != 0)
    {
        j_nr = POSITION_3;

        /* a(?) - c - b - d(?)    ---->    a(?) - b - c - d(?) */
        j_c = GRAPH[stage_nr][j_nr]->Pred_Job;
        j_a = GRAPH[stage_nr][j_c]->Pred_Job;
        j_d = GRAPH[stage_nr][j_nr]->Succ_Job;

        if (j_a != 0) GRAPH[stage_nr][j_a]->Succ_Job = j_nr;
        if (j_d != 0) GRAPH[stage_nr][j_d]->Pred_Job = j_c;
        GRAPH[stage_nr][j_nr]->Pred_Job = j_a; /* can be 0 */
        GRAPH[stage_nr][j_nr]->Succ_Job = j_c;
        GRAPH[stage_nr][j_c]->Pred_Job = j_nr;
        GRAPH[stage_nr][j_c]->Succ_Job = j_d; /* can be 0 */

        COUNT32++;
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Ursprünglichen Graphen wieder herstellen.                   */
/*                                                                      */
/************************************************************************/

void turn_back_changed_edge()
{
    /* Rücknahme der Transition (abhängig von der Nachbarschaftsstruktur) */
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
