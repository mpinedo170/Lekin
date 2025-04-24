/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                            G R A P H . C P P                         */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/*                   Erzeugung eines ersten Ablaufplanes                */
/*                     mit Hilfe von Prioritätsregeln.                  */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "def.h"
#include "graph.h"

void construct_graph()
{
    int stage_nr, m_nr, max_m_nr, j, j_succ, j_pred, j_nr, max_j_nr;
    int ident = 0;
    struct graph_node* node;

    /*** Construct basic structure without sequences ***/
    for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
        for (stage_nr = 1; stage_nr <= STAGES; stage_nr++) /* Create graph_node element */
            if (NULL == (node = (struct graph_node*)malloc(sizeof(struct graph_node))))
            {
                printf("Not enough memory!!\n\nStop Program!!\n\n");
                printf("Function:  void construct_graph ()\n");
                printf("Module:    GRAPH.CPP\n");
                exit(0);
            }
            else
            {
                node->Job_Nr = j_nr;
                node->Stage_Nr = stage_nr;
                node->Machine_Nr = WORKLOAD[stage_nr][j_nr];
                node->Processing_Time = JOBS[j_nr].Processing_Times[stage_nr];
                node->Mark = NO;
                node->Ident = ident++;
                node->Pred_Job = 0;
                node->Succ_Job = 0;
                GRAPH[stage_nr][j_nr] = node;
            }

    /*** Include sequencing ***/
    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[stage_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
        {
            max_j_nr = OPERATIONS_ON_MACHINES[stage_nr][m_nr];
            if (max_j_nr > 1)
            {
                /* First job on machine */
                j = MACHINES[stage_nr][m_nr].Act_Job_Order[1];
                j_succ = MACHINES[stage_nr][m_nr].Act_Job_Order[2];
                GRAPH[stage_nr][j]->Pred_Job = 0;
                GRAPH[stage_nr][j]->Succ_Job = j_succ;

                for (j_nr = 2; j_nr < max_j_nr; j_nr++)
                {
                    j = MACHINES[stage_nr][m_nr].Act_Job_Order[j_nr];          /* job */
                    j_pred = MACHINES[stage_nr][m_nr].Act_Job_Order[j_nr - 1]; /* pred */
                    j_succ = MACHINES[stage_nr][m_nr].Act_Job_Order[j_nr + 1]; /* succ */
                    GRAPH[stage_nr][j]->Pred_Job = j_pred;
                    GRAPH[stage_nr][j]->Succ_Job = j_succ;
                }

                /* Last job on machine */
                j = MACHINES[stage_nr][m_nr].Act_Job_Order[max_j_nr];
                j_pred = MACHINES[stage_nr][m_nr].Act_Job_Order[max_j_nr - 1];
                GRAPH[stage_nr][j]->Pred_Job = j_pred;
                GRAPH[stage_nr][j]->Succ_Job = 0;
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

struct Node
{
    struct graph_node* operation;
    struct Node* next_node;
};

struct Node* First_Node;
struct Node* Last_Node;

void get_node_memory(struct graph_node* node)
{
    struct Node* new_node;

    if (NULL == (new_node = (struct Node*)malloc(sizeof(struct Node))))
    {
        printf("Not enough memory!!\n\nStop Program!!\n\n");
        printf("Function: void get_node_memory ()\n");
        printf("Module:   GRAPH.CPP\n");
        exit(0);
    }
    else
    {
        new_node->operation = node;
        new_node->next_node = NULL;
    }
    if (First_Node == NULL) First_Node = new_node;
    if (Last_Node != NULL) Last_Node->next_node = new_node;

    Last_Node = new_node;
}

void delete_First_Node()
{
    struct Node* kill;

    kill = First_Node;
    /* Eventuell Last_Node aktualisieren */
    if (First_Node == Last_Node) Last_Node = NULL;
    First_Node = First_Node->next_node;
    free(kill);
}

void init_longest_path()
{
    int stage_nr, j_nr;

    First_Node = NULL;
    Last_Node = NULL;

    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
        {
            GRAPH[stage_nr][j_nr]->Pred_Operation_LP = NULL;
            GRAPH[stage_nr][j_nr]->Input_Counter = 0;

            /* "Release_Dates" ebenfalls hier berücksichtigen. */
            GRAPH[stage_nr][j_nr]->Longest_Path = JOBS[j_nr].Release_Date;
        }
}

int find_longest_path()
{
    int stage_nr, j_nr, succ_job, count;
    struct graph_node *succ, *node;

    /* Initialisierung (count = Anzahl der Operationen) */
    init_longest_path();
    count = STAGES * JOB_NR;

    /* Bestimme fuer jeden Knoten den Eingangsgrad       */
    /* Der Eingang vom Startknoten wird vernachlaessigt. */
    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
        {
            /* Nachfolgemaschine */
            if (stage_nr < STAGES)
            {
                succ = GRAPH[stage_nr + 1][j_nr];
                succ->Input_Counter++;
            }
            /* Nachfolgeauftrag */
            succ_job = GRAPH[stage_nr][j_nr]->Succ_Job;
            if (succ_job != 0) GRAPH[stage_nr][succ_job]->Input_Counter++;
        }

    /* First_Node ist der Beginn einer Liste, in der alle Knoten stehen,  */
    /* die Eingangsgrad Null haben.                                       */
    for (stage_nr = 1; stage_nr < STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
            if (GRAPH[stage_nr][j_nr]->Input_Counter == 0) get_node_memory(GRAPH[stage_nr][j_nr]);

    /* Hauptteil der Berechnung der laengsten Wege beginnt jetzt. */
    while (First_Node != NULL)
    {
        /* Test auf Zyklus */
        count--;
        if (count < 0) return (CYCLE);

        node = First_Node->operation;
        delete_First_Node();

        stage_nr = node->Stage_Nr; /* which stage */
        j_nr = node->Job_Nr;       /* which job */

        /* next operation for job j_nr */
        succ = NULL;
        if (stage_nr < STAGES) succ = GRAPH[stage_nr + 1][j_nr];

        if (succ != NULL)
        {
            succ->Input_Counter--;

            if (succ->Input_Counter == 0) get_node_memory(succ);

            /* Laengen vergleichen */
            if ((node->Longest_Path + node->Processing_Time) > succ->Longest_Path)
            {
                succ->Pred_Operation_LP = node;
                succ->Longest_Path = node->Longest_Path + node->Processing_Time;
            }
        }

        /* Is there a successor Operation on the machine? */
        succ_job = GRAPH[stage_nr][j_nr]->Succ_Job;
        if (succ_job != 0)
        {
            GRAPH[stage_nr][succ_job]->Input_Counter--;
            if (GRAPH[stage_nr][succ_job]->Input_Counter == 0) get_node_memory(GRAPH[stage_nr][succ_job]);

            /* Laengen vergleichen */
            if ((node->Longest_Path + (node->Processing_Time)) > GRAPH[stage_nr][succ_job]->Longest_Path)
            {
                GRAPH[stage_nr][succ_job]->Pred_Operation_LP = node;
                GRAPH[stage_nr][succ_job]->Longest_Path = node->Longest_Path + node->Processing_Time;
            }
        }
    }

    return count > 0 ? CYCLE : OK;
}

void save_lp()
{
    int stage_nr, j_nr, ident;

    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
        {
            ident = GRAPH[stage_nr][j_nr]->Ident;
            SAVE_LP[ident] = GRAPH[stage_nr][j_nr]->Longest_Path;
        }
}

int get_lp(struct graph_node* node)
{
    int ident;

    ident = node->Ident;
    return (SAVE_LP[ident]);
}

void get_result()
{
    int j_nr, completion_time, max_cost, cost;

    CANDIDATE_RESULT = 0;
    max_cost = -1;

    for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
    {
        completion_time = GRAPH[STAGES][j_nr]->Longest_Path + GRAPH[STAGES][j_nr]->Processing_Time;

        if (completion_time > JOBS[j_nr].Due_Date)
        {
            cost = JOBS[j_nr].Weight * (completion_time - JOBS[j_nr].Due_Date);
            if (cost > max_cost)
            {
                max_cost = cost;
                LAST_NODE = GRAPH[STAGES][j_nr];
            }

            CANDIDATE_RESULT =
                CANDIDATE_RESULT + (JOBS[j_nr].Weight * (completion_time - JOBS[j_nr].Due_Date));
        }
    }

    if (CANDIDATE_RESULT == 0) printf("\nObjective value = 0 (=optimal value)!!");
}

void free_memory()
{
    int stage_nr, j_nr;

    for (stage_nr = 1; stage_nr < STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++) free(GRAPH[stage_nr][j_nr]);
}

void save_actual_schedule()
{
    int stage_nr, m_nr, max_m_nr, j_nr, succ_job, pos;

    /*** Reset ***/
    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[stage_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
            for (pos = 1; pos <= JOB_NR; pos++) MACHINES[stage_nr][m_nr].Best_Job_Order[pos] = 0;
    }

    /*** Load best sequence ***/
    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++) /* found first job on a machine m_nr */
            if (GRAPH[stage_nr][j_nr]->Pred_Job == 0)
            {
                pos = 1;
                m_nr = GRAPH[stage_nr][j_nr]->Machine_Nr;
                MACHINES[stage_nr][m_nr].Best_Job_Order[pos] = j_nr;
                succ_job = GRAPH[stage_nr][j_nr]->Succ_Job;
                /* continue the following jobs on the same machine */
                while (succ_job != 0)
                {
                    pos++;
                    MACHINES[stage_nr][m_nr].Best_Job_Order[pos] = succ_job;
                    succ_job = GRAPH[stage_nr][succ_job]->Succ_Job;
                }
            }
}

void delete_act_job_order_in_graph()
{
    int stage_nr, j_nr;

    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
        for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
        {
            GRAPH[stage_nr][j_nr]->Pred_Job = 0;
            GRAPH[stage_nr][j_nr]->Succ_Job = 0;
        }
}

void read_best_job_order_in_graph()
{
    int stage_nr, m_nr, max_m_nr, j, j_pred, j_succ, j_nr, max_j_nr;

    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[stage_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
        {
            max_j_nr = OPERATIONS_ON_MACHINES[stage_nr][m_nr];
            if (max_j_nr > 1)
            {
                /* First job on machine */
                j = MACHINES[stage_nr][m_nr].Best_Job_Order[1];
                j_succ = MACHINES[stage_nr][m_nr].Best_Job_Order[2];
                GRAPH[stage_nr][j]->Pred_Job = 0;
                GRAPH[stage_nr][j]->Succ_Job = j_succ;

                for (j_nr = 2; j_nr < max_j_nr; j_nr++)
                {
                    j = MACHINES[stage_nr][m_nr].Best_Job_Order[j_nr];          /* job */
                    j_pred = MACHINES[stage_nr][m_nr].Best_Job_Order[j_nr - 1]; /* pred */
                    j_succ = MACHINES[stage_nr][m_nr].Best_Job_Order[j_nr + 1]; /* succ */
                    GRAPH[stage_nr][j]->Pred_Job = j_pred;
                    GRAPH[stage_nr][j]->Succ_Job = j_succ;
                }

                /* Last job on machine */
                j = MACHINES[stage_nr][m_nr].Best_Job_Order[max_j_nr];
                j_pred = MACHINES[stage_nr][m_nr].Best_Job_Order[max_j_nr - 1];
                GRAPH[stage_nr][j]->Pred_Job = j_pred;
                GRAPH[stage_nr][j]->Succ_Job = 0;
            }
            if (max_j_nr == 1)
            {
                j = MACHINES[stage_nr][m_nr].Best_Job_Order[1];
                GRAPH[stage_nr][j]->Pred_Job = 0;
                GRAPH[stage_nr][j]->Succ_Job = 0;
            }
        }
    }
}

void read_act_job_order_in_graph()
{
    int stage_nr, m_nr, max_m_nr, j, j_pred, j_succ, j_nr, max_j_nr;

    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[stage_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
        {
            max_j_nr = OPERATIONS_ON_MACHINES[stage_nr][m_nr];
            if (max_j_nr > 1)
            {
                /* First job on machine */
                j = MACHINES[stage_nr][m_nr].Act_Job_Order[1];
                j_succ = MACHINES[stage_nr][m_nr].Act_Job_Order[2];
                GRAPH[stage_nr][j]->Pred_Job = 0;
                GRAPH[stage_nr][j]->Succ_Job = j_succ;

                for (j_nr = 2; j_nr < max_j_nr; j_nr++)
                {
                    j = MACHINES[stage_nr][m_nr].Act_Job_Order[j_nr];          /* job */
                    j_pred = MACHINES[stage_nr][m_nr].Act_Job_Order[j_nr - 1]; /* pred */
                    j_succ = MACHINES[stage_nr][m_nr].Act_Job_Order[j_nr + 1]; /* succ */
                    GRAPH[stage_nr][j]->Pred_Job = j_pred;
                    GRAPH[stage_nr][j]->Succ_Job = j_succ;
                }

                /* Last job on machine */
                j = MACHINES[stage_nr][m_nr].Act_Job_Order[max_j_nr];
                j_pred = MACHINES[stage_nr][m_nr].Act_Job_Order[max_j_nr - 1];
                GRAPH[stage_nr][j]->Pred_Job = j_pred;
                GRAPH[stage_nr][j]->Succ_Job = 0;
            }
            if (max_j_nr == 1)
            {
                j = MACHINES[stage_nr][m_nr].Act_Job_Order[1];
                GRAPH[stage_nr][j]->Pred_Job = 0;
                GRAPH[stage_nr][j]->Succ_Job = 0;
            }
        }
    }
}

void control2()
{
    int stage_nr, max_m_nr, m_nr, j_nr, j, count, lp;
    struct graph_node* node;

    for (stage_nr = 1; stage_nr <= STAGES; stage_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[stage_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
        {
            count = 0;
            for (j = 1; j <= JOB_NR; j++)
                if ((GRAPH[stage_nr][j]->Pred_Job == 0) && (WORKLOAD[stage_nr][j] == m_nr))
                {
                    node = GRAPH[stage_nr][j];
                    j_nr = node->Job_Nr;
                    while (j_nr != 0)
                    {
                        lp = node->Longest_Path + node->Processing_Time;
                        j_nr = node->Succ_Job;
                        node = GRAPH[stage_nr][j_nr];
                        if (j_nr != 0)
                            if (lp > (node->Longest_Path + node->Processing_Time))
                            {
                                // control();
                                printf("\nLaengenproblem! (S: %d   Job: %d", stage_nr, j_nr);
                                exit(0);
                            }
                        count++;
                    }
                }

            if (count != OPERATIONS_ON_MACHINES[stage_nr][m_nr])
            {
                // control ();
                printf("\nKnotenanzahlproblem! (S: %d  M: %d)", stage_nr, m_nr);
                exit(0);
            }
        }
    }
}
