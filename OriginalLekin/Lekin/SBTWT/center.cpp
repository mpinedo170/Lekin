#include "StdAfx.h"

#include "center.h"

void ATC_Rule::Initialize(int input_number_due_times,
    int* input_completion_time_penalty,
    double input_step_length_average,
    double input_K)
{
    number_due_times = input_number_due_times;

    completion_time_penalty = input_completion_time_penalty;

    step_length_average = input_step_length_average;

    this_K = input_K;
}

double ATC_Rule::Value_Index(Cluster_Operation* p_operation, int available_time)
// Time the assigned machine is idle before assigning the job.
{
    // Augusto 2.10.96
    //        int this_release = max2( p_operation->Get_Release_Time() ,
    //        p_operation->Get_Marked_Release_Time() ) ;
    // Warning  Esto hay que mejorarlo
    int this_release = max2(p_operation->Get_Release_Time(), available_time);
#ifdef CHECK_DEBUG
    assert(this_K > 0);
    if (this_release < available_time) assert(this_release >= available_time);
#endif

    double partial_index = 0;
    for (int i = 0; i < number_due_times; i++)
    {
        // For every sink node the index is calculated and added.

        if (p_operation->Get_Due_Time(i)->Exist())
        {
            // If there is no path from the node to a sink, do not calculate
            // ist penalty.

            // From now is the actual ATC index calculation.
            if (step_length_average != 0)
            {
                partial_index +=
                    completion_time_penalty[i] *
                    (exp(-pos(double(p_operation->Get_Due_Time(i)->Time() - p_operation->Get_Length() +
                                     this_release - available_time)) /
                         (this_K * step_length_average)));
            }
            else
            {
                partial_index +=
                    completion_time_penalty[i] *
                    (exp(-pos(double(p_operation->Get_Due_Time(i)->Time() - p_operation->Get_Length() +
                                     this_release - available_time)) /
                         (this_K)));
            }
        }
    }
    if (p_operation->Get_Length() == 0)
        return (10000 * partial_index);
    else
        return (10000 * partial_index / p_operation->Get_Length());
    // Normal AATC rule.
}

void ATC_Rule::Insert_Operation(Beam_Indices<Solution_Increment>* p_list_increments,
    Cluster_Operation* p_operation,
    // Information of the job being tested.

    int location_work_center,
    int next_idle_machine,
    int location_operation,
    int available_time,
    // Time the assigned machine is idle before assigning the job.

    int type_oracle)
{
#ifdef CHECK_DEBUG
    assert(this_K > 0);
#endif

    double aatc_index = Value_Index(p_operation, available_time);

    /*      if ( type_oracle == EXACT_ORACLE )
                    if ( aatc_index == 0 )
                            aatc_index = FLT_MAX / 20 ;
                    else
                            aatc_index = 1  / aatc_index ;
                                            //The inverse, we choose bad operations.
    */
    if ((type_oracle != HEURISTIC_ORACLE) || (p_list_increments->Worth_Adding(aatc_index)))
    {
        Solution_Increment* p_chosen_solution_increment = new Solution_Increment(
            location_work_center, next_idle_machine, location_operation, available_time);
        p_list_increments->Add_Element(aatc_index, p_chosen_solution_increment, location_operation);
    }
}

//-------------------------------------------------------------------------------------------

Cluster_Work_Center::Cluster_Work_Center()
{
    id = NULL_WORK_CENTER;
    number_machines = NULL_NUMBER;
    number_operations = NULL_NUMBER;
    partial_num_oper = NULL_NUMBER;

    total_number_scheduled = 0;
    pred_node = NULL;
    succ_node = NULL;
};

void Cluster_Work_Center::Initialize(int input_id,
    int input_number_operations,
    int input_number_machines,
    Class_Node_Index* input_pred_node,
    Class_Node_Index* input_succ_node,
    Graph* p_Graph)
{
    int i, j;

    id = input_id;
    number_operations = input_number_operations;
    partial_num_oper = 0;
    number_machines = input_number_machines;

    // Allocating memory:
    operation = new Cluster_Operation[number_operations];

    total_number_scheduled = 0;
    machine_number_scheduled = new int[number_machines];

    // Augusto 9.10.96
    speed = new int[number_machines];

    sequence = new type_p_int[number_machines];

    pred_node = new Class_Node_Index[number_machines];
    succ_node = new Class_Node_Index[number_machines];
    // They are initialized as job = NULL_JOB; step = NULL_STEP;

    finishing_time_machine = new int[number_machines];

    for (i = 0; i < number_machines; i++)
    {
        machine_number_scheduled[i] = 0;
        // Augusto 9.10.96
        speed[i] = 0;

        // Sequence of scheduled operations in each machine.
        sequence[i] = new int[number_operations];
        for (j = 0; j < number_operations; j++) sequence[i][j] = NULL_LOCATION;

        /*                              //Predecessor node.
                        if ( input_pred_node != NULL ) {
                                pred_node[ i ] = input_pred_node[ i ] ;
                                p_Graph->Delete_Disjunctive( pred_node[ i ].job , pred_node[ i ].step ) ;
                        }
                                        //Successor node.
                        if ( input_succ_node != NULL ) {
                                succ_node[ i ] = input_succ_node[ i ] ;
                                p_Graph->Delete_Pred_Disj( succ_node[ i ].job , succ_node[ i ].step ) ;
                        }
        */
        finishing_time_machine[i] = NULL_TIME;
    }
    min_release_time = 0;
    min_machine = 0;
}

// Augusto 9.10.96
void Cluster_Work_Center::Ini_veloc(int machine, int speed_machine)
{
    speed[machine] = speed_machine;
}

void Cluster_Work_Center::Terminate()
{
    int i;

    // De-allocating memory:
    for (i = 0; i < number_machines; i++) delete[] sequence[i];
    delete[] operation;
    delete[] machine_number_scheduled;

    // Augusto 9.10.96
    delete[] speed;
    delete[] sequence;
    if (pred_node != NULL) delete[] pred_node;
    if (succ_node != NULL) delete[] succ_node;
    delete[] finishing_time_machine;
}

int Cluster_Work_Center::operator==(const Cluster_Work_Center& from)
{
    if (number_operations != from.number_operations) return (FALSE);

    for (int i = 0; i < number_operations; i++)
        if ((operation[i].job != from.operation[i].job) || (operation[i].step != from.operation[i].step))
            return (FALSE);

    return (TRUE);
}

// void Cluster_Work_Center::Copy_Graph_Plant_Information( Graph *p_Graph , Plant *p_Plant )
void Cluster_Work_Center::Add_Operation(int input_job, int input_step, Graph* p_Graph)

{
#ifdef CHECK_DEBUG
    assert(partial_num_oper < number_operations);
#endif
    operation[partial_num_oper].job = input_job;
    operation[partial_num_oper].step = input_step;

    operation[partial_num_oper].length = p_Graph->Length(input_job, input_step);
    operation[partial_num_oper].p_release_time = p_Graph->Get_p_Release_Time(input_job, input_step);
    operation[partial_num_oper].due_time = p_Graph->Get_p_Due_Time(input_job, input_step + 1);
    operation[partial_num_oper].work_center = p_Graph->Get_Work_Center(input_job, input_step);
    operation[partial_num_oper].p_machine = p_Graph->Get_p_Machine(input_job, input_step);
    operation[partial_num_oper].p_is_scheduled = p_Graph->Get_p_Is_Scheduled(input_job, input_step);

    operation[partial_num_oper].Unset_Machine_Is_Scheduled();

    /*
            if ( number_machines == 1 ) {
                    if ( pred_node[ 0 ].job != NULL_JOB )
                                    //There is a successor for the cluster work center.

                            p_Graph->Add_Disjunctive(       pred_node[0].job , pred_node[0].step , input_job ,
       input_step );

                    p_Graph->Delete_Disjunctive( input_job , input_step );
                    if ( succ_node[ 0 ].job != NULL_JOB )
                                    //There is a successor for the cluster work center.

                            p_Graph->Add_Disjunctive(       input_job , input_step , succ_node[0].job ,
       succ_node[0].step );
            }
    */
    p_Graph->Set_Location_Center(partial_num_oper, input_job, input_step);
    //      p_Graph->node_index_location[id][partial_num_oper].Set( input_job , input_step ) ;
    p_Graph->Set_Node_Index_Location(id, partial_num_oper, input_job, input_step);
    // In order to manage
    partial_num_oper++;
}

// Used in the interacion with CUISE.
void Cluster_Work_Center::Delete_Disjunctives(Graph* the_Graph)
{
    int i;
    int job_from, step_from;

    for (i = 0; i < number_operations; i++)
    {
        job_from = operation[i].job;
        step_from = operation[i].step;

        //              the_Graph->Delete_Disjunctive( job_from , step_from );
    }
}

void Cluster_Work_Center::Print(int number_jobs, Graph* p_Graph)
{
    /*
            fprintf( par_output_file ,"number of machines: %d \n", number_machines );

            for( int j = 0 ; j < number_operations ; j++){

                    fprintf( par_output_file ,"length %d, window:%d -> ",
                             operation[j].length ,
                             *(operation[j].p_release_time));

                    for( int k = 0 ; k < number_jobs ; k++ )
                            if ( operation[j].due_time[k].Exist())
                                    fprintf( par_output_file , "%d, " , operation[j].due_time[k].Time());
                            else
                                    fprintf( par_output_file ,"_, ");
                    if ( operation[j].Is_Complete( p_Graph ) == FALSE )
                            fprintf( par_output_file ," not complete ");
                    fprintf( par_output_file ,"\n");
            };
            fprintf( par_output_file ,"\n");
    */
}

// Schedule_Operation(): Returns TRUE if there is a dead lock.
int Cluster_Work_Center::Schedule_Operation(Graph* p_Graph, int scheduling_machine, int location_operation)
{
    int dead_lock;

    // Setting flags.
    operation[location_operation].Set_Machine_Is_Scheduled(scheduling_machine);

    // Recording sequence.
    sequence[scheduling_machine][machine_number_scheduled[scheduling_machine]] = location_operation;
    machine_number_scheduled[scheduling_machine]++;
    total_number_scheduled++;

    // Updating the graph.

    // Augusto 17.10.96
    //  calcular la nueva long. del arco
    //  new_length_arc = largo_actual / veloc_maquina.
    int new_length_operation = Get_Length_operation(location_operation) / speed[scheduling_machine];
    int job_to, step_to;
    //  llamar funcion que establece nueva longitud de los arcos.
    job_to = operation[location_operation].job;
    step_to = operation[location_operation].step;
    p_Graph->Set_Length_Conjuntive(job_to, step_to, new_length_operation);

    // For the cluster scheduling:
    if (machine_number_scheduled[scheduling_machine] > 1)
    {
        // IF there is a machine predecessor THEN.

        int location_from = sequence[scheduling_machine][machine_number_scheduled[scheduling_machine] - 2];
        // Obtain the location of the operation that was scheduled
        // before in the same machine of the work center.

        dead_lock = p_Graph->Insert_Arc(id, location_from, location_operation);
        // Delete and then insert .
#ifdef CHECK_DEBUG
        assert(dead_lock == FALSE);
#endif
    }
    return (FALSE);
}

int Cluster_Work_Center::Schedule_Operation(int* p_n_arcs_list,
    Arc_Backup* list,
    Graph* p_Graph,
    int scheduling_machine,
    int location_operation,
    int* p_remaining_operations,
    Class_Solution_Oracle* p_partial_solution,
    int location_work_center)
{
    int dead_lock;

    // Setting flags.
    operation[location_operation].Set_Machine_Is_Scheduled(scheduling_machine);
    // operation[] is the array of operations that should be scheduled.

    // Recording sequence.
    sequence[scheduling_machine][machine_number_scheduled[scheduling_machine]] = location_operation;
    // machine_number_scheduled[0]==2 means that there are 2 operations
    // already scheduled on machine 0.
    // sequence[0][2] is the third slot on machine 0.

    machine_number_scheduled[scheduling_machine]++;
    total_number_scheduled++;

    // Updating the graph.

    // For the cluster scheduling:
    /*
            if ( flag_temp == NO_TEMP ) {
            }
            else if ( pred_node[ scheduling_machine ].job != NULL_JOB ){
                                                    //ELSE machine_number_scheduled[ scheduling_machine ]
       == 1.

                    int pred_node_job = pred_node[ scheduling_machine ].job ;
                    int pred_node_step = pred_node[ scheduling_machine ].step ;

                    p_Graph->Delete_Disjunctive( pred_node_job , pred_node_step );
                                                    //The temporary disjuctive arcs go to al the elements in
       the cluster work center.

                    p_Graph->Add_Disjunctive( pred_node_job , pred_node_step , job_sched , step_sched );
                                                    //Adds a single arc from the cluster work center
       predecessor.
            }
    */

    // This part adds temporary arcs
    if (number_machines == 1)
    {
        // Not all the operations have been scheduled.

#ifdef CHECK_DEBUG
        assert(total_number_scheduled < number_operations);
#endif
        int i;
        /*              if ( succ_node[ 0 ].job != NULL_JOB ) {
                                p_Graph->Delete_Pred_Disj( succ_node[ 0 ].job , succ_node[ 0 ].step ) ;
                                for( i = 0 ; i < number_operations ; i++){
                                        if ( *( operation[ i ].p_is_scheduled ) == FALSE )
                                                p_Graph->Add_Disjunctive( operation[ i ].job , operation[ i
           ].step , succ_node[ 0 ].job , succ_node[ 0 ].step );
                                        //Adds an arc to the not yet scheduled operations in the machine.
                                }
                        }
        */
        if (total_number_scheduled == (number_operations - 1))
        {
            // When scheduling the operation before the last one.
            // We schedule the last operation also.

            (*p_remaining_operations)--;

            // Scheduling the last operation.
            for (i = 0; i < number_operations; i++)
            {
                if (*(operation[i].p_is_scheduled) == FALSE)
                {
                    dead_lock = p_Graph->Insert_Arc(p_n_arcs_list, list, id, location_operation, i);
                    // Adds an arc to the not yet scheduled operations in the machine.

                    // Setting flags.
                    operation[i].Set_Machine_Is_Scheduled(scheduling_machine);

                    // Recording sequence.
                    sequence[scheduling_machine][machine_number_scheduled[scheduling_machine]] = i;
                    machine_number_scheduled[scheduling_machine]++;
                    total_number_scheduled++;

                    p_partial_solution->Write(location_work_center, scheduling_machine, i);

                    if (dead_lock == TRUE) return (TRUE);
                }
            }
        }
        else
        {
            for (i = 0; i < number_operations; i++)
            {
                // Check all the operations.

                if (*(operation[i].p_is_scheduled) == FALSE)
                {
                    // If an operation has not been scheduled yet.

                    dead_lock = p_Graph->Insert_Arc(p_n_arcs_list, list, id, location_operation, i);
                    // Adds an arc to the not yet scheduled operations in the machine.
                    // p_n_arcs_list is the size of the list of arcs
                    // list is the list of arcs.
                    // id is the identification of the work center.
                    // location_operation is the operation from which the arc is fixed.
                    // i is the operation to which the arc is fixed.

                    if (dead_lock == TRUE) return (TRUE);
                }
            }
        }
    }
    else
    {  // There are many machines.

        // Augusto 17.10.96
        //  calcular la nueva long. del arco
        //  new_length_arc = largo_actual / veloc_maquina.
        int new_length_operation = Get_Length_operation(location_operation) / speed[scheduling_machine];
        int job_from, step_from, job_to, step_to;
        //  llamar funcion que establece nueva longitud de los arcos.
        job_from =
            operation[sequence[scheduling_machine][machine_number_scheduled[scheduling_machine] - 2]].job;
        step_from =
            operation[sequence[scheduling_machine][machine_number_scheduled[scheduling_machine] - 2]].step;
        job_to = operation[location_operation].job;
        step_to = operation[location_operation].step;
        p_Graph->Set_Length_Conjuntive(job_to, step_to, new_length_operation);

        if (machine_number_scheduled[scheduling_machine] > 1)
        {
            dead_lock = p_Graph->Insert_Arc(p_n_arcs_list, list, id,
                sequence[scheduling_machine][machine_number_scheduled[scheduling_machine] - 2],
                location_operation);

            // dead_lock = p_Graph->Insert_Arc( p_n_arcs_list , list , id , location_operation , i );
            // Adds an arc to the not yet scheduled operations in the machine.
            // p_n_arcs_list is the size of the list of arcs
            // list is the list of arcs.
            // id is the identification of the work center.
            // location_operation is the operation from which the arc is fixed.
            // i is the operation to which the arc is fixed.

            if (dead_lock == TRUE) return (TRUE);
        }

        //*******
    }
    return (FALSE);  // If everything goes OK, returs that there is no dead lock.
}

void Cluster_Work_Center::Unschedule_Operation(int unscheduling_machine,
    int location_operation,
    // For modifying the cluster.

    Graph* p_Graph)
// For modifying the graph.
{
    // Recording sequence.
    total_number_scheduled--;
    machine_number_scheduled[unscheduling_machine]--;

#ifdef CHECK_DEBUG
    assert(
        location_operation == sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine]]);
#endif

    // Setting flags.
    operation[location_operation].Unset_Machine_Is_Scheduled();

    sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine]] = NULL_LOCATION;
}

void Cluster_Work_Center::Unschedule_Operation(int unscheduling_machine,
    int location_operation,
    // For modifying the cluster.

    Graph* p_Graph,
    Class_Solution_Oracle* p_partial_solution,
    int location_work_center,
    int old_length_operation)
// For modifying the graph.
{
    // Augusto 17.10.96
    // Restablecer largo original de los arcos
    int job_from, step_from, job_to, step_to;
    //  llamar funcion que establece nueva longitud de los arcos.
    job_from =
        operation[sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine] - 2]].job;
    step_from =
        operation[sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine] - 2]].step;
    job_to = operation[location_operation].job;
    step_to = operation[location_operation].step;
    p_Graph->Set_Length_Conjuntive(job_to, step_to, old_length_operation);

    if (total_number_scheduled < number_operations)
    {
        // If is not the last operation.

        // Recording sequence.
        total_number_scheduled--;
        machine_number_scheduled[unscheduling_machine]--;

#ifdef CHECK_DEBUG
        assert(location_operation ==
               sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine]]);
#endif

        // Setting flags.
        operation[location_operation].Unset_Machine_Is_Scheduled();

        // Augusto 8.10.96
        p_partial_solution->Unwrite(location_work_center, unscheduling_machine, location_operation);

        sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine]] = NULL_LOCATION;
    }
    else
    {  // It is the last operation
#ifdef CHECK_DEBUG
        assert(total_number_scheduled == number_operations);
#endif
        total_number_scheduled--;
        machine_number_scheduled[unscheduling_machine]--;
        int location_last = sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine]];
        operation[location_last].Unset_Machine_Is_Scheduled();

        p_partial_solution->Unwrite(location_work_center, unscheduling_machine, location_last);

        // Augusto 3.10.96   se repite anteriormente
        //                total_number_scheduled-- ;
        //                machine_number_scheduled[ unscheduling_machine ]-- ;
        //                operation[ location_operation ].Unset_Machine_Is_Scheduled() ;
        sequence[unscheduling_machine][machine_number_scheduled[unscheduling_machine]] = NULL_LOCATION;
    }

    /*
            if ( flag_temp == NO_TEMP ) {
                                    //if flag_temp == TEMP then the recursion restore the state of the graph.

                    int location_predecessor ;
                    if ( p_Graph->Is_There_Machine_Predecessor( &location_predecessor , id ,
       location_operation ) ) {
                                    //If there is a machine predecessor.

                            p_Graph->Uninsert_Arc( id , location_predecessor , location_operation ) ;
                                    //Deletes the one arc location_predecessor->location_operation.
                    }
            }
    */
    /*
    //Updating the graph.

            if ( ( number_machines == 1 ) && ( flag_temp == TEMP ) && ( total_number_scheduled + 1 <
    number_operations ) ) {
                                                    //Is not the last operation.

                    p_Graph->Uninsert_All_Arcs( id , location_operation , NO_TEMP );
                                    //Deletes all the arc going from location_operation to the other operation
                                    //in the center.


    //              if ( succ_node[ unscheduling_machine ].job != NULL_JOB )
                                    //There is a successor for the cluster work center.

    //                      p_Graph->Add_Disjunctive(       job_unsched , step_unsched ,
    //                                                      succ_node[ unscheduling_machine ].job , succ_node[
    unscheduling_machine ].step );

            }

            int location_predecessor ;
            if ( p_Graph->Is_There_Machine_Predecessor( &location_predecessor , id , location_operation ) ) {
                                    //If there is a machine predecessor.

                    p_Graph->Uninsert_Arc( id , location_predecessor , location_operation , NO_TEMP ) ;
                                    //Deletes the one arc going from ( job_pred , step_pred ).

                    if ( ( number_machines == 1 ) && ( flag_temp == TEMP ) ) {
                            int i ;
                            for( i = 0 ; i < number_operations ; i++){
                                    if ( *( operation[ i ].p_is_scheduled ) == FALSE ) {
                                            dead_lock = p_Graph->Insert_Arc( id , location_predecessor , i ,
    NO_TEMP );
                                    }
                            }
                                    //Adds an arc to the not yet scheduled operations in the machine.
                    }
            }
    */
}

// Get_Index(): returns the index.
void Cluster_Work_Center::Get_Index(int* p_index_1,
    int* p_index_2,
    Class_Disjunctive_Arc* p_first_arc,
    Class_Disjunctive_Arc* p_second_arc,
    Graph* p_Graph)
{
    int index_j = 0;
    int index_k = 0;
    *p_index_1 = INT_MIN;
    *p_index_2 = INT_MIN;
    int max_index_1 = INT_MIN;
    int max_index_2 = INT_MIN;
    int estimated_release_j, estimated_release_k;
    int j, k;
    int best_j = -1;
    int best_k = -1;

    for (j = 0; j < number_operations; j++)
    {
        for (k = j + 1; k < number_operations; k++)
        {
            if (p_Graph->Matrix_Value(id, j, k) == BLANK)
            {
                // If j->k:
                estimated_release_k =
                    max2(operation[j].Get_Finishing_Time(), operation[k].Get_Release_Time());
                index_j = p_Graph->Estimate_Delta(operation[k].job, operation[k].step, estimated_release_k);

                // If k->j:
                estimated_release_j =
                    max2(operation[k].Get_Finishing_Time(), operation[j].Get_Release_Time());
                index_k = p_Graph->Estimate_Delta(operation[j].job, operation[j].step, estimated_release_j);

                if (index_j == 0 && index_k == 0)
                {
                    index_j = operation[j].length;
                    index_k = operation[k].length;
                }
                *p_index_1 = index_j + index_k;
                //                              *p_index_1 = abs( index_j - index_k ) ;
                *p_index_2 = min2(index_j, index_k);
                if (*p_index_1 > max_index_1)
                {
                    if (index_j >= index_k)
                    {
                        best_j = j;
                        best_k = k;
                    }
                    else
                    {
                        best_j = k;
                        best_k = j;
                    }
                    max_index_1 = *p_index_1;
                    max_index_2 = *p_index_2;
                }
                else if ((*p_index_1 == max_index_1) && (*p_index_2 > max_index_2))
                {
                    if (index_j >= index_k)
                    {
                        best_j = j;
                        best_k = k;
                    }
                    else
                    {
                        best_j = k;
                        best_k = j;
                    }
                    max_index_2 = *p_index_2;
                }
            }
        }
    }
    if (best_j > -1)
    {
#ifdef CHECK_DEBUG
        assert(best_k < number_operations);
        assert(best_k >= 0);
#endif
        (*p_first_arc).job_from = operation[best_j].job;
        (*p_first_arc).step_from = operation[best_j].step;
        (*p_first_arc).job_to = operation[best_k].job;
        (*p_first_arc).step_to = operation[best_k].step;

        (*p_second_arc).job_from = operation[best_k].job;
        (*p_second_arc).step_from = operation[best_k].step;
        (*p_second_arc).job_to = operation[best_j].job;
        (*p_second_arc).step_to = operation[best_j].step;
    }
}

// Fix_Disjunctions(): returns INT_MAX/3 if we fathom the node.
int Cluster_Work_Center::Fix_Disjunctions(int* p_n_arcs_list,
    Arc_Backup* list,
    // list keeps a backup of the value of an arc before replacing it.

    int max_possible_index,
    Graph* p_Graph)
{
    if (p_Graph->Get_N_Blank_Centers(id) == 0) return (0);

    int estimated_release_j, estimated_release_k;
    int j, k;
    int dead_lock;
    int index_j = 0;
    int index_k = 0;
    for (j = 0; j < number_operations; j++)
    {
        for (k = j + 1; k < number_operations; k++)
        {
            if (p_Graph->Matrix_Value(id, j, k) == BLANK)
            {
                // If j->k:
                estimated_release_k =
                    max2(operation[j].Get_Finishing_Time(), operation[k].Get_Release_Time());
                index_j = p_Graph->Estimate_Delta(operation[k].job, operation[k].step, estimated_release_k);
                // If k->j:
                estimated_release_j =
                    max2(operation[k].Get_Finishing_Time(), operation[j].Get_Release_Time());
                index_k = p_Graph->Estimate_Delta(operation[j].job, operation[j].step, estimated_release_j);
                if ((index_j > max_possible_index) && (index_k > max_possible_index))
                {
                    return (INT_MAX / 3);
                }
                else if (index_j > max_possible_index)
                {
                    dead_lock = p_Graph->Insert_Arc(p_n_arcs_list, list, id, k, j);
                    // Temporary arcs are added.
#ifdef CHECK_DEBUG
                    assert(dead_lock == FALSE);
#endif
                }
                else if (index_k > max_possible_index)
                {
                    dead_lock = p_Graph->Insert_Arc(p_n_arcs_list, list, id, j, k);
                    // Temporary arcs are added.
#ifdef CHECK_DEBUG
                    assert(dead_lock == FALSE);
#endif
                }
            }
        }
    }
    return (FALSE);
}
