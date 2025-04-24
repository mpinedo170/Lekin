/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                         L _ S E A R C H . C P P                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "def.h"
#include "graph.h"
#include "metropol.h"
#include "neighbor.h"

void read_in_new_schedule_from_sb()
{
    int s_nr, m_nr, max_m_nr, j_nr, ops_on_machines, job;

    /* Need to actualize Act_Job_Order [], WORKLOAD [] [] and */
    /* OPERATIONS_ON_MACHINES [] []                            */
    for (s_nr = 1; s_nr <= STAGES; s_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[s_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++) OPERATIONS_ON_MACHINES[s_nr][m_nr] = 0;
    }

    for (s_nr = 1; s_nr <= STAGES; s_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[s_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
            for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
                if (MACHINES[s_nr][m_nr].Best_Job_Order[j_nr] != 0)
                    OPERATIONS_ON_MACHINES[s_nr][m_nr]++;
                else
                    break;
    }

    for (s_nr = 1; s_nr <= STAGES; s_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[s_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
        {
            ops_on_machines = OPERATIONS_ON_MACHINES[s_nr][m_nr];
            for (j_nr = 1; j_nr <= ops_on_machines; j_nr++)
            {
                MACHINES[s_nr][m_nr].Act_Job_Order[j_nr] = MACHINES[s_nr][m_nr].Best_Job_Order[j_nr];

                job = MACHINES[s_nr][m_nr].Best_Job_Order[j_nr];

                WORKLOAD[s_nr][job] = m_nr;
            }
        }
    }
}

void read_in_new_schedule_from_sb_partial(int stage)
{
    int s_nr, m_nr, max_m_nr, j_nr, ops_on_machines, job;

    /* Need to actualize Act_Job_Order [], WORKLOAD [] [] and */
    /* OPERATIONS_ON_MACHINES [] []                            */
    for (s_nr = 1; s_nr <= STAGES; s_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[s_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++) OPERATIONS_ON_MACHINES[s_nr][m_nr] = 0;
    }

    for (s_nr = 1; s_nr <= STAGES; s_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[s_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
            if (s_nr != stage)
            {
                for (j_nr = 1; j_nr <= JOB_NR; j_nr++)
                    if (MACHINES[s_nr][m_nr].Best_Job_Order[j_nr] != 0)
                        OPERATIONS_ON_MACHINES[s_nr][m_nr]++;
                    else
                        break;
            }
            else
                OPERATIONS_ON_MACHINES[s_nr][m_nr] = 0;
    }

    for (s_nr = 1; s_nr <= STAGES; s_nr++)
    {
        max_m_nr = MACHINES_ON_STAGE[s_nr];
        for (m_nr = 1; m_nr <= max_m_nr; m_nr++)
        {
            ops_on_machines = OPERATIONS_ON_MACHINES[s_nr][m_nr];
            for (j_nr = 1; j_nr <= ops_on_machines; j_nr++)
            {
                MACHINES[s_nr][m_nr].Act_Job_Order[j_nr] = MACHINES[s_nr][m_nr].Best_Job_Order[j_nr];

                job = MACHINES[s_nr][m_nr].Best_Job_Order[j_nr];

                WORKLOAD[s_nr][job] = m_nr;
            }
        }
    }
}

/* Get first solution with first workload */
void read_in_sb_solution()
{
    // if (JOB_NR == 1)
    //  {printf("\nOnly one job!! Stop Programm!!!\n"); exit(0);}

    if (JOB_NR == 1)
    {
        printf("\nOnly one job!! Stop Programm!!!\n");
        return;
    }

    read_in_new_schedule_from_sb();

    construct_graph();

    init_longest_path();

    find_longest_path();

    get_result();

    ACT_RESULT = CANDIDATE_RESULT;
    BEST_RESULT = CANDIDATE_RESULT;

    save_lp();

    get_critical_edges();
}

void read_in_sb_solution_partial(int stage)
{
    if (JOB_NR == 1)
    {
        printf("\nOnly one job!! Stop Programm!!!\n");
        return;
    }

    read_in_new_schedule_from_sb_partial(stage);

    construct_graph();

    init_longest_path();

    find_longest_path();

    get_result();

    ACT_RESULT = CANDIDATE_RESULT;
    BEST_RESULT = CANDIDATE_RESULT;

    save_lp();

    get_critical_edges();
}

/*
void read_in_new_solution_from_sb ()
{
  delete_act_job_order_in_graph ();

  read_best_job_order_in_graph ();

  init_longest_path ();

  find_longest_path ();
}
*/

extern int NR_OF_CRITICAL_EDGES;

void sb_ls()
{
    TEMPERATURE = 150;
    ACCEPT_RATE = 0.25;
    NEIGHBORHOOD = 5;
    ITERATION = 100;  // 20;
    // RAND_MAX = long(pow(2,31)-1);

    ACCEPT = BAD_ITER = 0; /* verify acceptance rate from Metropolis */

    // randomize();
    srand(int(time(NULL)));

    read_in_sb_solution();

    // printf("solution give to  lsrw(): %d\n", BEST_RESULT);

    if (NR_OF_CRITICAL_EDGES > 0)  // newly added line
        lsrw();

    // printf("solution get from lsrw(): %d\n", BEST_RESULT);
    printf("* ");

    free_memory();

    // if (NR_OF_CRITICAL_EDGES == 0) return (1);
    // else return (0);
}

void sb_ls_partial(int stage)
{
    TEMPERATURE = 150;
    ACCEPT_RATE = 0.25;
    NEIGHBORHOOD = 5;
    ITERATION = 100;

    ACCEPT = BAD_ITER = 0; /* verify acceptance rate from Metropolis */

    srand(int(time(NULL)));

    read_in_sb_solution();
    read_in_sb_solution_partial(stage);

    if (NR_OF_CRITICAL_EDGES > 0)  // newly added line
        lsrw();

    free_memory();
}
