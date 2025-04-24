#include "StdAfx.h"

#include "center.h"

// Obtains the earliest timing when an operation that has not been scheduled yet would finish.
int Cluster_Work_Center::Earliest_Finishing(Graph* p_Graph)
{
    int i;
    int earliest_finishing = INT_MAX;

    int current_completion;

    if (total_number_scheduled < number_operations)
    {
        for (i = 0; i < number_operations; i++)
        {
            current_completion = operation[i].Get_Finishing_Time();
            // Get_Finishing_Time() returns the release time plus the processing time.

            if (!operation[i].Is_Scheduled()
                // The operation has not been scheduled.
                && p_Graph->Predecessors_Are_Scheduled(id, i))
            {
                Omega_i++;

                if (current_completion < earliest_finishing)
                {
                    earliest_finishing = current_completion;
                }
            }
        }
#ifdef CHECK_DEBUG
        assert(Omega_i <= number_operations - total_number_scheduled);
#endif

        return (earliest_finishing);
    }
    else
    {
        return (INT_MAX);
    }
}

// Finds the star completion time for the active generation rule among the not scheduled and not marked.
int Cluster_Work_Center::Completion_Star_Marked(Graph* p_Graph)
{
    // completion_star = INT_MAX ;
    // position_star = NULL_LOCATION ;
    // Omega_i = 0 ;

    int current_completion;
    //      int complete_center = Complete_or_Marked( p_Graph ) ;
    if (total_number_scheduled < number_operations)
    {
        for (int i = 0; i < number_operations; i++)
        {
            if (!operation[i].Is_Scheduled()
                // The operation has not been scheduled.
                && !operation[i].is_marked && Predecessors_Are_Scheduled_or_Marked(i, p_Graph))
            {
                Omega_i++;
                current_completion = operation[i].marked_release_time + operation[i].length;
                if (
                    //                                      complete_center
                    //                                      &&
                    //                                      operation[i].Is_Complete( p_Graph )
                    //                                      &&
                    ((current_completion < completion_star) ||
                        ((current_completion == completion_star) &&
                            (operation[i].marked_release_time < operation[position_star].marked_release_time))
                        // Choose the minimum completion time with the minimum release date.
                        ))
                {
                    completion_star = current_completion;
                    position_star = i;
                }
            }
        }
        return (completion_star);
    }
    else
    {
        return (INT_MAX);
    }
}

// void Cluster_Work_Center::Calculate_Release_Times( Graph *p_Graph , int *forbidden )
void Cluster_Work_Center::Calculate_Release_Times()
{
    int current_release;

    earliest_release[0] = INT_MAX;
    earliest_release[1] = INT_MAX;
    // Should hold that earliest_release[0] <= earliest_release[1]

    position_release[0] = NULL_LOCATION;
    position_release[1] = NULL_LOCATION;

    // Obtain the two earliest completion times.
    for (int k = 0; k < number_operations; k++)
    {
        if (!operation[k].Is_Scheduled()
            // The operation has not been scheduled.
            && k != position_star
            // We know type_oracle == EXACT_ORACLE.
        )
        {
            current_release = operation[k].Get_Release_Time();
            if (current_release < earliest_release[0])
            {
                earliest_release[1] = earliest_release[0];
                earliest_release[0] = current_release;

                position_release[1] = position_release[0];
                position_release[0] = k;
            }
            else if (current_release < earliest_release[1])
            {
                earliest_release[1] = current_release;

                position_release[1] = k;
            }
        }
    }
#ifdef CHECK_DEBUG
    assert(earliest_release[0] < INT_MAX);
    assert(earliest_release[1] < INT_MAX);
#endif
}

/* xxx Antes de ser modificado por Augusto el 30.9.96
//Used by ATC_Rule::Is_Schedulable for the non-delay schedule parameter.
//Used by ATC_Rule::Value_Index.
void Cluster_Work_Center::Get_Available_Time(   int *p_available_time ,
                                                int *p_next_idle_machine ,
                                                Graph *p_Graph )
{
        int i ;
        *p_available_time = INT_MAX ;
        int this_machine_available_time ;
        int location_operation ;

        *p_next_idle_machine = -1 ;

//        if ( machine_number_scheduled[0] == 0 ) {
        if ( total_number_scheduled == 0 ) {
                                        #ifdef CHECK_DEBUG
                                                for( i = 0 ; i < number_machines ; i++ )
                                                        assert( machine_number_scheduled[i] == 0 ) ;
                                        #endif
                       //If there are no operations already scheduled.
                       //Obtain the earliest release time among the operations.

                for( i = 0 ; i < number_operations ; i++ ) {

                                //Obtains the minimal release time
                        if ( operation[ i ].marked_release_time < *p_available_time ) {
                                *p_available_time = operation[ i ].marked_release_time ;
                        }
                }
        } else {

                int there_is_empty_machine = FALSE ;
                while( !there_is_empty_machine
                        &&
                        i < number_machines
                ) {
                        if ( machine_number_scheduled[i] == 0 ) {
                                there_is_empty_machine = FALSE ;
                                *p_next_idle_machine = i ;

                                for( i = 0 ; i < number_operations ; i++ ) {

                                        //Obtains the minimal release time
                                        if (    !operation[ i ].Is_Scheduled()
                                                &&
                                                operation[ i ].marked_release_time < *p_available_time
                                        ) {
                                                *p_available_time = operation[ i ].marked_release_time ;
                                        }
                                }

                        } else {
                                location_operation = sequence[i][ machine_number_scheduled[i] - 1 ] ;
                                        //Assigns the id of the operation that was scheduled last.

                                this_machine_available_time =
                                        p_Graph->Get_Finishing_Time(    operation[ location_operation ].job ,
                                                                        operation[ location_operation ].step )
; if ( this_machine_available_time < *p_available_time ) { *p_available_time = this_machine_available_time ;
                                        *p_next_idle_machine = i ;
                                }
                        }
                }




       }
                                        #ifdef CHECK_DEBUG
                                                assert( *p_available_time < INT_MAX ) ;
                                        #endif


}
********/

// Used by ATC_Rule::Is_Schedulable for the non-delay schedule parameter.
// Used by ATC_Rule::Value_Index.
void Cluster_Work_Center::Get_Available_Time(int* p_available_time, int* p_next_idle_machine, Graph* p_Graph)
{
    int i, j;
    *p_available_time = INT_MAX;
    int this_machine_available_time;
    int location_operation;

    *p_next_idle_machine = -1;

    //        if ( machine_number_scheduled[0] == 0 ) {
    if (total_number_scheduled == 0)
    {
#ifdef CHECK_DEBUG
        for (i = 0; i < number_machines; i++) assert(machine_number_scheduled[i] == 0);
#endif
        // If there are no operations already scheduled.
        // Obtain the earliest release time among the operations.

        for (i = 0; i < number_operations; i++)
        {
            // Obtains the minimal release time
            // Augusto 01.10.96
            //                        if ( operation[ i ].marked_release_time < *p_available_time ) {
            //                                *p_available_time = operation[ i ].marked_release_time ;
            if (*(operation[i].p_release_time) < *p_available_time)
            {
                *p_available_time = *(operation[i].p_release_time);
            }
        }
        // Augusto asigna 0 a idle_machine
        *p_next_idle_machine = 0;
    }
    else
    {
        // Augusto   inicializar i con 0
        i = 0;
        int there_is_empty_machine = FALSE;
        while (!there_is_empty_machine && i < number_machines)
        {
            if (machine_number_scheduled[i] == 0)
            {
                // Augusto                                there_is_empty_machine = FALSE ;
                there_is_empty_machine = TRUE;
                *p_next_idle_machine = i;
                // Augusto cambiar variable i por j
                for (j = 0; j < number_operations; j++)
                {
                    // Obtains the minimal release time
                    if (!operation[j].Is_Scheduled() &&
                        // Augusto 2.10.96
                        //                                                operation[ j ].marked_release_time <
                        //                                                *p_available_time
                        *(operation[j].p_release_time) < *p_available_time)
                    {
                        // Augusto 2.10.96
                        //                                                *p_available_time = operation[ j
                        //                                                ].marked_release_time ;
                        *p_available_time = *(operation[j].p_release_time);
                    }
                }
            }
            else
            {
                location_operation = sequence[i][machine_number_scheduled[i] - 1];
                // Assigns the id of the operation that was scheduled last.

                this_machine_available_time = p_Graph->Get_Finishing_Time(
                    operation[location_operation].job, operation[location_operation].step);
                if (this_machine_available_time < *p_available_time)
                {
                    *p_available_time = this_machine_available_time;
                    *p_next_idle_machine = i;
                }
            }
            // Augusto sumar 1 a i
            i++;
        }
    }
#ifdef CHECK_DEBUG
    assert(*p_available_time < INT_MAX);
#endif
}

// Obtains the machine and its available time where the likely_ATC_operation would finish the earliest.
void Cluster_Work_Center::Obtain_Machine(int* p_available_time,
    int* p_next_idle_machine,
    Graph* p_Graph,
    Cluster_Operation* p_likely_ATC_operation)
{
    int i;
    int aux_available_time = INT_MAX;
    *p_available_time = INT_MAX;
    int this_machine_available_time;
    int location_last_operation;

    *p_next_idle_machine = -1;

    if (total_number_scheduled == 0)
    {
#ifdef CHECK_DEBUG
        for (i = 0; i < number_machines; i++) assert(machine_number_scheduled[i] == 0);
#endif
        // If there are no operations already scheduled.
        // Obtain the earliest release time among the operations.

        for (i = 0; i < number_operations; i++)
        {
            // Obtains the minimal release time
            if (*(operation[i].p_release_time) < *p_available_time)
            {
                *p_available_time = *(operation[i].p_release_time);
            }
        }
        *p_next_idle_machine = Get_Fastest_Machine();
    }
    else
    {
        i = 0;
        int min_finish_time = INT_MAX;
        int this_machine_new_finish_time;
        for (i = 0; i < number_machines; i++)
        {
            if (machine_number_scheduled[i] == 0)
            {
                this_machine_new_finish_time =
                    *(p_likely_ATC_operation->p_release_time) + p_likely_ATC_operation->length / speed[i];
            }
            else
            {
                location_last_operation = sequence[i][machine_number_scheduled[i] - 1];
                // Assigns the id of the operation that was scheduled last.

                this_machine_available_time = p_Graph->Get_Finishing_Time(
                    operation[location_last_operation].job, operation[location_last_operation].step);
                // Returns the finishing time of the [job,step].

                this_machine_new_finish_time =
                    max2(*(p_likely_ATC_operation->p_release_time), this_machine_available_time) +
                    p_likely_ATC_operation->length / speed[i];
            }
            if (this_machine_new_finish_time < min_finish_time)
            {
                min_finish_time = this_machine_new_finish_time;
                *p_available_time = this_machine_available_time;
                *p_next_idle_machine = i;
            }
        }
    }
#ifdef CHECK_DEBUG
    assert(*p_available_time < INT_MAX);
#endif
}

int local_par_quick = FALSE;

/*
//Filter_Rank_Operations() returns number_schedulable.
int Cluster_Work_Center::Filter_Rank_Operations(        Beam_Indices<Solution_Increment> *p_list_increments ,
                                                                                int location_branching_center
,
                                                                                                //Location of
this work center in the current cluster,
                                                                                                //not its id.

                                                                                Graph *p_Graph , ATC_Rule
*p_ATC_Rule , int type_oracle , int **updating_forbidden , int *forbidden )
{
int j ;

        int available_time ;            //Time at which the machine is available
                                                //i.e., finishing time for the machine BEFORE scheduling the
new operation.
                                                //CHECK whether accurate.

        int next_idle_machine ; //Corresponding machine.

        Get_Available_Time( &available_time , &next_idle_machine , p_Graph );
                                //We choose the machine next_idle_machine where to schedule the operation.

        if ( completion_star < INT_MAX ) {
                                                //We know the the star operation exists.

                p_ATC_Rule->Insert_Operation(   p_list_increments , &( operation[position_star] ) ,
                                                                location_branching_center ,
                                                                next_idle_machine ,  position_star ,
available_time , type_oracle ) ;

                if ( Omega_i == 1 )
                        return ( 1 ) ;  //There is only one schedulable operation.
        }

        int number_unscheduled = number_operations - total_number_scheduled ;
        int r_min ;                             //Minimum release date for the operations in machine i other
than star and j. if ( par_flag_flow_shop && ( type_oracle == EXACT_ORACLE ) && ( number_unscheduled > 2 )
                && ( local_par_quick == FALSE ) && ( completion_star < INT_MAX )
        )
                Calculate_Release_Times( ) ;
        else
                r_min = INT_MAX ;

        int C_pair ;            //Finishing time of the star operation followed by j.
        int C_swap ;            //Finishing time of the operation to be swapped with star.
        int d_pair ;                    //Maximum value for C_swap.

        Class_Node_Index *p_node ;
        for ( j = 0 ; j < number_operations ; j++ ) {
                if (    !operation[j].Is_Scheduled()
                                                //The operation has not been scheduled.
                        &&
                        ( operation[j].Get_Release_Time() < completion_star )
                        &&                      //Active schedules.
                        j != position_star
                        &&
                        p_Graph->Predecessors_Are_Scheduled( id , j )
                ) {
                        if (    !par_flag_flow_shop || ( type_oracle != EXACT_ORACLE ) ||
                                local_par_quick || ( completion_star == INT_MAX )
                        ) {
                                p_ATC_Rule->Insert_Operation(   p_list_increments , &( operation[j] ) ,
                                                                                location_branching_center ,
                                                                                next_idle_machine ,  j ,
available_time , type_oracle ) ; } else {


                                if (    ( machine_number_scheduled[0] - 1 >= 0 )
                                        &&
                                        ( forbidden[ sequence[0][ (machine_number_scheduled[0] - 1)] ] == j )
                                ) {
                                        printf("");
                                        ;
                                } else {
                                                //Obtaining the end of the slack period.
                                        if ( !operation[j].Is_Final_Operation( p_Graph ) ) {
                                                //operation[j] is not the final operation.

                                                p_node = p_Graph->Get_Node_Index_Location( id , j ) ;

                                                d_pair = p_Graph->Get_Release_Time( p_node->job , p_node->step
+ 1 ) ; } else  //operation[j] is the final operation.

                                                d_pair = operation[j].Get_Due_Date( p_Graph ) ;

                                        C_pair =        max2(    operation[position_star].Get_Finishing_Time()
, operation[j].Get_Release_Time() )
                                                        +
                                                        operation[j].length ;
                                                //Completion time of the pair (*->j).

                                        if ( C_pair <= d_pair ) {
                                                //Something can be done.

                                                if ( number_unscheduled > 2 ) {
                                                        //Otherwise r_min == INT_MAX.

                                        #ifdef CHECK_DEBUG
                                                        assert( earliest_release[0] < INT_MAX ) ;
                                                        assert( earliest_release[1] < INT_MAX ) ;
                                        #endif

                                                        if ( j == position_release[0] )
                                                                r_min = earliest_release[1] ;
                                                        else
                                                                r_min = earliest_release[0] ;
                                                }

                                                if (    ( r_min == INT_MAX )
                                                        ||
                                                        ( C_pair <= r_min )
                                                                //We know the operation j is freezable.
                                                ) {
                                                                //Complete elimination.

                                                        printf("");
                                                        ;

                                                } else {

                                                        C_swap =        max2(
operation[j].Get_Finishing_Time() , operation[position_star].Get_Release_Time() )
                                                                        +
                                                                        operation[position_star].length ;
                                                                //Completion time of the swap that is going to
be eliminated.

                                                        if ( C_swap >= C_pair )
                                                                updating_forbidden[ location_branching_center
][ j ] = position_star ;

                                                        p_ATC_Rule->Insert_Operation(   p_list_increments , &(
operation[j] ) , location_branching_center , next_idle_machine ,  j , available_time , type_oracle ) ;

//                                                      printf("");
                                                }

                                        } else {

//                                              printf(".");
                                                p_ATC_Rule->Insert_Operation(   p_list_increments , &(
operation[j] ) , location_branching_center , next_idle_machine ,  j , available_time , type_oracle ) ;
                                        }
                                }
                        }
                }
        }

        return ( p_list_increments->Number_Elements_List() ) ;
}
*/
int Cluster_Work_Center::Predecessors_Are_Scheduled_or_Marked(int a, Graph* p_Graph)
{
    int j;
#ifdef CHECK_DEBUG
    assert(a >= 0 && a < number_operations);
#endif

    for (j = 0; j < number_operations; j++)
    {
        if ((*(operation[j].p_is_scheduled) == FALSE) && !operation[j].is_marked && j != a &&
            (p_Graph->Delay(operation[j].job, operation[j].step, operation[a].job, operation[a].step) != -1))
        {
            return (FALSE);
        }
    }
    return (TRUE);
}

int Cluster_Work_Center::Filter_Rank_Operations(Beam_Indices<Solution_Increment>* p_list_increments,
    int available_time,
    Graph* p_Graph,
    ATC_Rule* p_ATC_Rule)
{
    int j;

    if (completion_star < INT_MAX)
    {
        // We know the the star operation exists.

        p_ATC_Rule->Insert_Operation(p_list_increments, &(operation[position_star]), -1, 0, position_star,
            available_time, BOUND_ORACLE);

        if (Omega_i == 1) return (1);  // There is only one schedulable operation.
    }
    for (j = 0; j < number_operations; j++)
    {
        if (!operation[j].Is_Scheduled()
            // The operation has not been scheduled.
            && !operation[j].is_marked && j != position_star &&
            (operation[j].marked_release_time < completion_star + par_slack_active) &&
            Predecessors_Are_Scheduled_or_Marked(j, p_Graph))
        {
            p_ATC_Rule->Insert_Operation(
                p_list_increments, &(operation[j]), -1, 0, j, available_time, BOUND_ORACLE);
        }
    }
    return (p_list_increments->Number_Elements_List());
}

int Cluster_Work_Center::Complete(Graph* p_Graph)
{
    for (int k = 0; k < number_operations; k++)
    {
        if (operation[k].Is_Scheduled())
            if (!operation[k].Is_Complete(p_Graph)) return (FALSE);
    }
    return (TRUE);
}

int Cluster_Work_Center::Complete_or_Marked(Graph* p_Graph)
{
    for (int k = 0; k < number_operations; k++)
    {
        if (operation[k].Is_Scheduled() || operation[k].is_marked)
            if (!operation[k].Is_Complete(p_Graph)) return (FALSE);
    }
    return (TRUE);
}

int Cluster_Work_Center::Try_Operation(int* p_new_available_time,
    int** p_max_violation,
    int current_available_time,
    int location_operation,
    Graph* p_Graph)
{
    int j, k;
    int this_violation;
    int delta = 0;
    int job_from, step_from, job_to, step_to;
    int this_delay;

    operation[location_operation].is_marked = TRUE;
    *(operation[location_operation].p_machine) = 0;
    operation[location_operation].marked_release_time =
        max2(operation[location_operation].marked_release_time, current_available_time);
    *p_new_available_time =
        operation[location_operation].marked_release_time + operation[location_operation].length;
    // Update the available time.
    // Update release times using the delayed precedence constraints:
    for (j = 0; j < number_operations; j++)
    {
        if (!operation[j].is_marked && !operation[j].Is_Scheduled())
        {
            job_from = operation[location_operation].job;
            step_from = operation[location_operation].step;
            job_to = operation[j].job;
            step_to = operation[j].step;
            operation[j].marked_release_time = max2(operation[j].marked_release_time,
                operation[location_operation].marked_release_time + operation[location_operation].length);
            // If no precedence constraints are considered.
            this_delay = p_Graph->Delay(job_from, step_from, job_to, step_to);
            if (this_delay != -1)
            {
                // Now they are considered.
                operation[j].marked_release_time = max2(operation[j].marked_release_time,
                    operation[location_operation].marked_release_time + this_delay);
            }
        }
    }

    // Calculate delta:
    int number_sinks = p_Graph->Get_Number_Jobs();
    int* completion_penalty = p_Graph->Get_Completion_Time_Penalty();
    (*p_max_violation) = new int[number_sinks];
    for (k = 0; k < number_sinks; k++) (*p_max_violation)[k] = INT_MIN;
    Class_Due_Time* this_due_time;
    for (j = 0; j < number_operations; j++)
    {
        if (!operation[j].Is_Scheduled())
        {
#ifdef CHECK_DEBUG
            assert(operation[j].marked_release_time != -1);
#endif
            this_due_time = p_Graph->Get_p_Due_Time(operation[j].job, operation[j].step);
            for (k = 0; k < number_sinks; k++)
            {
                if (this_due_time[k].Exist())
                {
                    this_violation = pos(operation[j].marked_release_time - this_due_time[k].Time());
                    if (this_violation > (*p_max_violation)[k]) (*p_max_violation)[k] = this_violation;
                }
            }
        }
    }
    for (k = 0; k < number_sinks; k++)
        if ((*p_max_violation)[k] > 0) delta = delta + (*p_max_violation)[k] * completion_penalty[k];
    return (delta);
}

// Fix_Precedences(): Function analogous to Cluster_Work_Center::Fix_Disjunctions()
int Cluster_Work_Center::Fix_Precedences(int* p_number_fixed,
    int** p_delay_from,
    int** p_delay_to,
    // list keeps a backup of the value of an arc before replacing it.

    int* max_violation,
    // Comes from Cluster_Work_Center::Try_Operation()

    int max_possible_index,
    Graph* p_Graph)

{
    int j, k, l, m;
    int this_violation;
    int operation_j_marked_release_time, operation_k_marked_release_time;
    int delta_jk, delta_kj;
    int delay_jm, delay_km;

    // Update release times using the delayed precedence constraints:
    int number_sinks = p_Graph->Get_Number_Jobs();
    int* this_max_violation = new int[number_sinks];
    int* completion_penalty = p_Graph->Get_Completion_Time_Penalty();
    Class_Due_Time* this_due_time;
    *p_number_fixed = 0;
    *p_delay_from = new int[number_operations * number_operations];
    *p_delay_to = new int[number_operations * number_operations];
    for (j = 0; j < number_operations; j++)
    {
        if (!operation[j].is_marked && !operation[j].Is_Scheduled())
        {
            for (k = j + 1; k < number_operations; k++)
            {
                if (!operation[k].is_marked && !operation[k].Is_Scheduled() &&
                    (p_Graph->Delay(
                         operation[j].job, operation[j].step, operation[k].job, operation[k].step) == -1) &&
                    (p_Graph->Delay(
                         operation[k].job, operation[k].step, operation[j].job, operation[j].step) == -1)
                    // No delayed precedence constraints.
                )
                {
                    // try j->k:
                    operation_k_marked_release_time = max2(operation[k].marked_release_time,
                        operation[j].marked_release_time + operation[j].length);

                    // Analogous to p_Graph->Estimate_Delta( )
                    delta_jk = 0;
                    if (FALSE)
                    {
                        this_due_time = p_Graph->Get_p_Due_Time(operation[k].job, operation[k].step);
                        for (l = 0; l < number_sinks; l++)
                        {
                            if (this_due_time[l].Exist())
                            {
                                this_violation =
                                    pos(operation_k_marked_release_time - this_due_time[l].Time());
                                if (this_violation > max_violation[l])
                                    delta_jk = delta_jk +
                                               (this_violation - max_violation[l]) * completion_penalty[l];
                            }
                        }
                    }
                    else
                    {
#ifdef CHECK_DEBUG
                        assert(p_Graph->Delay(operation[k].job, operation[k].step, operation[k].job,
                                   operation[k].step) == 0);
#endif

                        for (l = 0; l < number_sinks; l++) this_max_violation[l] = INT_MIN;
                        for (m = 0; m < number_operations; m++)
                        {
                            delay_km = p_Graph->Delay(
                                operation[k].job, operation[k].step, operation[m].job, operation[m].step);
                            if (delay_km != -1)
                            {
                                this_due_time = p_Graph->Get_p_Due_Time(operation[m].job, operation[m].step);
                                for (l = 0; l < number_sinks; l++)
                                {
                                    if (this_due_time[l].Exist())
                                    {
                                        this_violation = pos(operation_k_marked_release_time + delay_km -
                                                             this_due_time[l].Time());
                                        if (this_violation > this_max_violation[l])
                                            this_max_violation[l] = this_violation;
                                    }
                                }
                            }
                        }
                        for (l = 0; l < number_sinks; l++)
                            if (this_max_violation[l] > max_violation[l])
                                delta_jk = delta_jk +
                                           (this_max_violation[l] - max_violation[l]) * completion_penalty[l];
                    }

                    // try k->j:
                    operation_j_marked_release_time = max2(operation[j].marked_release_time,
                        operation[k].marked_release_time + operation[k].length);

                    // Analogous to p_Graph->Estimate_Delta( )
                    delta_kj = 0;
                    if (FALSE)
                    {
                        this_due_time = p_Graph->Get_p_Due_Time(operation[j].job, operation[j].step);
                        for (l = 0; l < number_sinks; l++)
                        {
                            if (this_due_time[l].Exist())
                            {
                                this_violation =
                                    pos(operation_j_marked_release_time - this_due_time[l].Time());
                                if (this_violation > max_violation[l])
                                    delta_kj = delta_kj +
                                               (this_violation - max_violation[l]) * completion_penalty[l];
                            }
                        }
                    }
                    else
                    {
#ifdef CHECK_DEBUG
                        assert(p_Graph->Delay(operation[j].job, operation[j].step, operation[j].job,
                                   operation[j].step) == 0);
#endif

                        for (l = 0; l < number_sinks; l++) this_max_violation[l] = INT_MIN;
                        for (m = 0; m < number_operations; m++)
                        {
                            delay_jm = p_Graph->Delay(
                                operation[j].job, operation[j].step, operation[m].job, operation[m].step);
                            if (delay_jm != -1)
                            {
                                this_due_time = p_Graph->Get_p_Due_Time(operation[m].job, operation[m].step);
                                for (l = 0; l < number_sinks; l++)
                                {
                                    if (this_due_time[l].Exist())
                                    {
                                        this_violation = pos(operation_j_marked_release_time + delay_jm -
                                                             this_due_time[l].Time());
                                        if (this_violation > this_max_violation[l])
                                            this_max_violation[l] = this_violation;
                                    }
                                }
                            }
                        }
                        for (l = 0; l < number_sinks; l++)
                            if (this_max_violation[l] > max_violation[l])
                                delta_kj = delta_kj +
                                           (this_max_violation[l] - max_violation[l]) * completion_penalty[l];
                    }

                    if ((delta_jk > max_possible_index) && (delta_kj > max_possible_index))
                    {
                        delete[] this_max_violation;
                        delete[] max_violation;
                        return (TRUE);
                        // The node is fathomed.
                    }
                    else if (delta_jk > max_possible_index)
                    {
                        (*p_delay_from)[*p_number_fixed] = k;
                        (*p_delay_to)[*p_number_fixed] = j;
                        (*p_number_fixed)++;
                        p_Graph->Set_Delay(operation[k].job, operation[k].step, operation[j].job,
                            operation[j].step, operation[k].length);
                        // Temporary delayed precedence constralong int added.
                    }
                    else if (delta_kj > max_possible_index)
                    {
                        (*p_delay_from)[*p_number_fixed] = j;
                        (*p_delay_to)[*p_number_fixed] = k;
                        (*p_number_fixed)++;
                        p_Graph->Set_Delay(operation[j].job, operation[j].step, operation[k].job,
                            operation[k].step, operation[j].length);
                        // Temporary delayed precedence constralong int added.
                    }
                }
            }
        }
    }
    delete[] this_max_violation;
    delete[] max_violation;
    return (FALSE);
    // The node is not fathomed.
}

void Cluster_Work_Center::Untry_Operation(int location_operation)
{
    operation[location_operation].Unset_Marked();
}

// void Cluster_Work_Center::Back_Up( int **p_release_times , int ***p_delay , Graph *p_Graph )
void Cluster_Work_Center::Back_Up(int** p_release_times)
{
    int j;
    *p_release_times = new int[number_operations];
    for (j = 0; j < number_operations; j++) (*p_release_times)[j] = operation[j].marked_release_time;
}

// void Cluster_Work_Center::Restore( int *release_times , int **delay , Graph *p_Graph )
// void Cluster_Work_Center::Restore_Release( int *release_times )
void Cluster_Work_Center::Restore(
    int* release_times, int number_fixed, int* delay_from, int* delay_to, Graph* p_Graph)
{
    int j;
    for (j = 0; j < number_operations; j++)
    {
        operation[j].marked_release_time = release_times[j];
    }
    delete[] release_times;

#ifdef CHECK_DEBUG
    if (number_fixed > -1) assert(delay_from != NULL && delay_to != NULL);
#endif
    for (j = 0; j < number_fixed; j++)
    {
        p_Graph->Set_Delay(operation[delay_from[j]].job, operation[delay_from[j]].step,
            operation[delay_to[j]].job, operation[delay_to[j]].step, -1);
    }
    delete[] delay_from;
    delete[] delay_to;
}

void Cluster_Work_Center::Copy_Release_Times()
{
    int j;
    for (j = 0; j < number_operations; j++)
        if (!operation[j].Is_Scheduled()) operation[j].marked_release_time = operation[j].Get_Release_Time();
}

void Cluster_Work_Center::Uncopy_Release_Times()
{
    int j;
    for (j = 0; j < number_operations; j++)
        if (!operation[j].Is_Scheduled()) operation[j].marked_release_time = -1;
}

/*                                      if ( current_penalty + delta_jk > upper_bound ) {
                                                p_Graph->Set_Delay(     operation[k].job , operation[k].step ,
                                                                                operation[j].job ,
operation[j].step , operation[k].length ) ;
//                                              (*p_delay_from)[*p_number_fixed] = k ;
//                                              (*p_delay_to)[*p_number_fixed] = j ;
//                                              (*p_number_fixed)++ ;
                                        }
                                        if ( current_penalty + delta_kj > upper_bound ) {
                                                if ( current_penalty + delta_jk > upper_bound ) {
                                                        delete[] this_max_violation ;
                                                        delete[] max_violation ;
                                                        return( TRUE ) ;
                                                                //The node is fathomed.
                                                } else {
                                                        p_Graph->Set_Delay(     operation[j].job ,
operation[j].step , operation[k].job , operation[k].step , operation[j].length ) ;
//                                                      (*p_delay_from)[*p_number_fixed] = j ;
//                                                      (*p_delay_to)[*p_number_fixed] = k ;
//                                                      (*p_number_fixed)++ ;
                                                }
                                        }
*/

/*      if ( number_fixed > -1 ) {
                                        #ifdef CHECK_DEBUG
                                                assert( delay_from != NULL && delay_to != NULL ) ;
                                        #endif
                for ( j = 0 ; j < number_fixed ; j++ ) {
                        p_Graph->Set_Delay(     operation[delay_from[j]].job , operation[delay_from[j]].step ,
                                                        operation[delay_to[j]].job ,
   operation[delay_to[j]].step , -1 ) ;
                }
                delete[] delay_from ;
                delete[] delay_to ;
        }
*/

/* Funciones que se utilizan para un cluster con varios workcenters */

// Finds the star completion time for the active generation rule among the not scheduled.
int Cluster_Work_Center::Completion_Star(Graph* p_Graph, int must_be_ready)
{
    // Notice that   completion_star = INT_MAX ;
    //              position_star = NULL_LOCATION ;
    //              Omega_i = 0 ;
    // are global variables of the object Cluster_Work_Center.

#ifdef CHECK_DEBUG
    if (machine_number_scheduled[0] - 1 >= 0)
        assert(sequence[0][(machine_number_scheduled[0] - 1)] != NULL_LOCATION);
#endif

    if (total_number_scheduled < number_operations)
    {
        int current_completion;
        for (int i = 0; i < number_operations; i++)
        {
            if (!operation[i].Is_Scheduled()
                // The operation has not been scheduled.
                // MS 27/3                        &&
                // MS 27/3                        p_Graph->Predecessors_Are_Scheduled( id , i )
                // The star operation has to have all ist predecessors scheduled,
                // otherwise we do not know when does it exactly finishes.
                // Predecessors_Are_Scheduled() returns TRUE if the immediate
                // machine predecessr is scheduled, in order to save time.
            )
            {
                Omega_i++;
                current_completion = operation[i].Get_Finishing_Time();

                // modificaciones Luis, marzo del 98
                // if (    operation[i].Is_Complete( p_Graph )
                // Returns TRUE if all the job predecessors have been scheduled.

                //         &&
                //         (       ( current_completion < completion_star )
                //                 ||
                //                 (       ( current_completion == completion_star )
                //                         &&
                //                         ( operation[i].Get_Release_Time()
                //                                 < operation[ position_star ].Get_Release_Time() )
                //                 )
                // Choose the minimum completion time with the minimum release date.
                //         )
                // ) {

                if (current_completion < completion_star)
                {
                    completion_star = current_completion;
                    position_star = i;
                }
            }
        }
        return (completion_star);
    }
    else
    {
        return (INT_MAX);
    }
}

// Returns the operation that is most likely to be scheduled next.
int Cluster_Work_Center::Get_Likely_ATC_Operation(Graph* p_Graph, ATC_Rule* p_ATC_Rule, int type_oracle)
{
    int j;
    Beam_Indices<Solution_Increment> list_increments;

    int time_earliest_finishing_machine = Get_Time_Earliest_Finishing_Machine(p_Graph);

    int number_unscheduled_operations = Get_Number_Unscheduled_Operations();

    list_increments.Initialize(
        number_unscheduled_operations, number_unscheduled_operations, par_threshold_oracle);
    // Threshold.
    for (j = 0; j < number_operations; j++)
    {
        if (!operation[j].Is_Scheduled()
            // The operation has not been scheduled.
            && (operation[j].Get_Release_Time() < completion_star)
            // MS 27/3                 &&                      //Active schedules.
            // MS 27/3                p_Graph->Predecessors_Are_Scheduled( id , j )
        )
        {
            p_ATC_Rule->Insert_Operation(
                &list_increments, &(operation[j]), 0, 0, j, time_earliest_finishing_machine, type_oracle);
        }
    }
    int likely_ATC_operation = (list_increments.Get_p_ith_Element(0))->Get_Location_Operation();
    list_increments.Terminate();
    return (likely_ATC_operation);
}

// Filter_Rank_Operations() returns number_schedulable.
int Cluster_Work_Center::Filter_Rank_Operations(Beam_Indices<Solution_Increment>* p_list_increments,
    int location_branching_center,
    // Location of this work center in the current cluster,
    // not its id.

    Graph* p_Graph,
    ATC_Rule* p_ATC_Rule,
    int type_oracle,
    int** updating_forbidden,
    int* forbidden)
{
    int j;

    int available_time;  // Time at which the machine is available
                         // i.e., finishing time for the machine BEFORE scheduling the new operation.
                         // CHECK whether accurate.

    int next_idle_machine;  // Corresponding machine.

    //        int length_longest_operation = Get_Length_Longest_Operation() ;
    // Finds the length of the longest operation not scheduled yet.

    int likely_ATC_operation = Get_Likely_ATC_Operation(p_Graph, p_ATC_Rule, type_oracle);
    // Operation that is likely to be chosen next.
    // We are not sure since the machine has not been settled.

    //        Get_Available_Time( &available_time , &next_idle_machine , p_Graph );
    //        Get_Available_Time_Length_Operation( &available_time , &next_idle_machine , p_Graph ,
    //        length_longest_operation );
    // Estimates when is the longest operation going to finish.
    // The machine where such operation would finish first is chosen.

    Obtain_Machine(&available_time, &next_idle_machine, p_Graph, &(operation[likely_ATC_operation]));
    // Estimates when is the likely_ATC_operation going to finish.
    // The machine where such operation would finish first is chosen.

#ifdef CHECK_DEBUG
    //                                              assert( completion_star == INT_MAX ) ;
#endif

    int number_unscheduled = number_operations - total_number_scheduled;

    // Inserts in the sorted list p_list_increments all the
    // operations that can be scheduled.
    for (j = 0; j < number_operations; j++)
    {
        if (!operation[j].Is_Scheduled()
            // The operation has not been scheduled.
            && (operation[j].Get_Release_Time() < completion_star)
            // MS 27/3                 &&                      //Active schedules.
            // MS 27/3                p_Graph->Predecessors_Are_Scheduled( id , j )
        )
        {
            p_ATC_Rule->Insert_Operation(p_list_increments, &(operation[j]), location_branching_center,
                next_idle_machine, j, available_time, type_oracle);
        }
    }
    return (p_list_increments->Number_Elements_List());
}
