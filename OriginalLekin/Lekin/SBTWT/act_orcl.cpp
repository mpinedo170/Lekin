#include "StdAfx.h"

#include "act_orcl.h"

// SOLEX int Act_Oracle::Get_Cutting_Time( int max_branches_oracle )
int Act_Oracle::Get_Cutting_Time(int max_branches_oracle)
{
    // int current_time ; //SOLEX

    time_out = FALSE;

    if (type_oracle != EXACT_ORACLE)
    {
        return (first_time + (int)5);

        /*              switch (max_branches_oracle){
                        case 0 :
                        case 1 :
                                return ( current_time + (clock_t)(
           p_Cluster->Get_Number_Unscheduled_Operations() / 10 + 2 ) ) ;

                        case 2 :
                                return ( current_time + (clock_t)(
           p_Cluster->Get_Number_Unscheduled_Operations() / 10 + 4 ) ) ;

                        case 3 :
                                return ( current_time + (clock_t)( 2 *
           p_Cluster->Get_Number_Unscheduled_Operations() / 10 + 1 ) ) ;

                        case INT_MAX :                  //Complete enumeration.
                                return ( LONG_MAX ) ;
                        default :
                                return( current_time + (clock_t) 8 ) ;
                        }
        */
    }
    else
    {
        return (LONG_MAX);
    }
}

int Act_Oracle::Time_Out()
{
    return (FALSE);
    int current_time;  // SOLEX

#ifdef CHECK_DEBUG
    assert(time_out == FALSE);
#endif

    if (type_oracle != EXACT_ORACLE)
    {
        // Augusto 12.8.96
        //             time( &current_time ) ;
        //                current_time = (clock_t)(clock())/CLOCKS_PER_SEC ;
        current_time = (clock_t)(clock()) / CLK_TCK;

        if (current_time >= cutting_time)
        {
            time_out = TRUE;
            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    else
    {
        return (FALSE);
    }
}

int Act_Oracle::Could_Fix_Disjunctives(int this_max_acceptable, int remaining_jobs)
{
    if ((this_max_acceptable < INT_MAX) && (remaining_jobs > 1))
        return (TRUE);
    else
        return (FALSE);

    if (remaining_jobs % 2 == 0)
        return (TRUE);
    else
        return (FALSE);
}

// Get_Children_Plus(): Returns the number of children, or -1 if there are no children.
int Act_Oracle::Get_Children_Plus(Beam_Indices<Solution_Increment>* p_list_increments,
    // Storage possible increments.

    int* p_location_branching_center,
    // Location int the cluster, not the id.

    int** p_backup,

    int max_branches_oracle,
    Set_Work_Centers* p_Set_Centers)
{
    // int location_branching_center ;
    // Location int the cluster of the next idle work center.
    int** forbidden = NULL;
    int* past = NULL;

    if (type_oracle == HEURISTIC_ORACLE)
    {
        if (p_Cluster->Get_Number_Work_Centers() == p_Plant->Get_Number_Work_Centers())
            *p_location_branching_center = p_Cluster->Earliest_Completion_Work_Center(p_Graph);

        // Choose the center where the branching will be performed.
        // We choose the cluster that has the center that finishes the earliest.
        else
            *p_location_branching_center = p_Cluster->Earliest_Finishing_Work_Center(p_Graph);

        // Selects the next center where to brach as the ealiest available.
        // It also abtains the star operations.
    }
    else if (type_oracle == EXACT_ORACLE)
    {
        switch (par_selection_rule)
        {
            case BOTTLE:
                *p_location_branching_center = p_Cluster->Bottleneck_Work_Center(p_Graph);
                //                                      p_Cluster->Bottleneck_Work_Center( p_Graph , forbidden
                //                                      ) ;
                break;
            case BOUND:
                *p_location_branching_center = p_Cluster->Highest_Bound_Center(p_Graph, p_Set_Centers);
                //                                      p_Cluster->Highest_Bound_Center( p_Graph , forbidden ,
                //                                      p_Set_Centers ) ;
                break;
        }
    }
    else
    {  // type_oracle == BOUND_ORACLE
        *p_location_branching_center = p_Cluster->Earliest_Finishing_Work_Center(p_Graph);
    }
    if (*p_location_branching_center == NULL_WORK_CENTER) return (-1);
        // There are -1 children.

#ifdef CHECK_DEBUG
    assert(*p_location_branching_center < p_Cluster->Get_Number_Work_Centers());
#endif

    Cluster_Work_Center* p_cluster_center_next =
        p_Cluster->Get_Cluster_Work_Center(*p_location_branching_center);
    // Points to the Cluster_Work_Center representing the work center
    // to be scheduled int the next iteration.

    p_list_increments->Initialize(p_cluster_center_next->Get_Number_Unscheduled_Operations(),
        // Number of elements.
        p_cluster_center_next->Get_Number_Unscheduled_Operations(),
        // Maximum branches.
        par_threshold_oracle);
    // Threshold.
    // This list of increments is for the next idle work center int location

    int number_schedulable;
    if (type_oracle == EXACT_ORACLE)
    {
        p_cluster_center_next->Clean_Star();
        // Deletes star operation.

        forbidden = this_Forbidden_Set.Get_Forbidden();
        // forbidden is the data accumulated int the past.

        // DO NOT DELETE!!!!!!!!
        /*              if ( par_flag_flow_shop ) {

                                                #ifdef CHECK_DEBUG
                                                        assert( par_selection_rule == BOTTLE ) ;
                                                #endif

                                p_cluster_center_next->Completion_Star( p_Graph , forbidden[
           p_cluster_center_next->Get_Id() ] ) ;
                                        //Finds the star completion time and operation for the active
           generation rule
                                        //among the not scheduled.
                        }
        */
        past = this_Forbidden_Set.Back_Up(p_Cluster, *p_location_branching_center);
        // Has the ald data for center *p_location_branching_center.

        number_schedulable = p_cluster_center_next->Filter_Rank_Operations(p_list_increments,
            *p_location_branching_center, p_Graph, &this_ATC_Rule, type_oracle, forbidden, past);
        delete[] past;

        *p_backup = this_Forbidden_Set.Back_Up(p_Cluster, *p_location_branching_center);
        // Has the new data for center *p_location_branching_center.
    }
    else
    {  // Heuristic:
        if (type_oracle == BOUND_ORACLE) p_cluster_center_next->Clean_Star();
        // If BOUND_ORACLE there is no star operation.

        number_schedulable = p_cluster_center_next->Filter_Rank_Operations(p_list_increments,
            *p_location_branching_center, p_Graph, &this_ATC_Rule, type_oracle, forbidden, past);
        // Ranks the operations and inserts them in p_list_increments.
        // If HEURISTIC_ORACLE the star operation was already obtained.
    }
    return (min2(max_branches_oracle, number_schedulable));
    // IMPROVE this part.
}

// This recursive call is for two or more machines in the center.
// Steps of the recursive call:
// Write the solution.
// Modify the graph.
// Ranks the remaining operations.
// While{ next operation }
void Act_Oracle::Recursive_Call(
    // Output parameters.
    Class_Solution_Oracle* p_optimal_solution,

    // Recursive input paramenters.
    Class_Solution_Oracle* p_partial_solution,

    Solution_Increment* p_solution_increment,
    // The increment in the schedule corresponds to the
    // operation to be scheduled in this recursive call.

    int remaining_jobs,

    int max_branches_oracle,

    Set_Work_Centers** p_p_input_Set_Work_Centers,
    // Could the the paret's or a left sibling's

    int* p_bound_number)
{
    int dead_lock;
    int lower_bound = INT_MIN;
    // It remains INT_MIN if no lower bound was calculated.
    // It becomes ) if they are calculated, but are not effective.
    // Are positive if and only if the lower bound is efective.

    Beam_Indices<Solution_Increment> list_increments;
    // Stores the possible increments in a sorted way.

    // int could_calculate_bound = FALSE ;
    int number_children = -1;

    Set_Work_Centers* p_child_Set_Work_Centers = NULL;

    int next_branching_center = NULL_WORK_CENTER;
    int* backup;  // Data for the next_branching_center.

    // Do the assigment.

    int original_penalty = p_partial_solution->Get_Objective();
    // It allows to undo everything done int this call.
    // original_penalty is the objective function which is being minimized.

    p_partial_solution->Write(p_solution_increment->location_work_center, p_solution_increment->machine,
        p_solution_increment->location_operation);
    // location_operation is the position or id of the operation in the cluster.
    // The operations in a cluster are numbered from 0,1,2...
    // If location_operation==2 the the operation is the third operation in the cluster.
    // The increment is added to the partial solution.

    Cluster_Work_Center* p_cluster_work_center_current =
        p_Cluster->Get_Cluster_Work_Center(p_solution_increment->location_work_center);
    // Points to the Cluster_Work_Center representing the work center
    // being scheduled int this iteration.

    int number_arcs_list = 0;
    Arc_Backup* list_arcs = new Arc_Backup[MAX_ARCS_BACKUP];
    // Each recursive call fixes a number of disjunctive arcs.
    // This list is to remember those disjunctive arcs in
    // order to return to the original situation once we get
    // out of the recursive call.

    // Augusto 17.10.96
    //        save_length = Get_operation_length(location_operation, location_work_center)
    //        Get_Length_operation( int id_operation )
    int save_length =
        (*p_cluster_work_center_current).Get_Length_operation(p_solution_increment->location_operation);
    //        int new_lenght_operation = save_length /
    //        (*p_cluster_work_center_current).speed[p_solution_increment->machine];

    dead_lock = p_cluster_work_center_current->Schedule_Operation(&number_arcs_list, list_arcs, p_Graph,
        p_solution_increment->machine, p_solution_increment->location_operation, &remaining_jobs,
        p_partial_solution, p_solution_increment->location_work_center);
    // Temporary arcs are added.
    // Modifying the graph.
    // To be modified for parallel machines.

#ifdef CHECK_DEBUG
    if (dead_lock != FALSE) assert(dead_lock == FALSE);
#endif

        // int predecessor = p_Graph->Is_Optimal_Predecessor(0) ;

#ifdef ANIMATION_DEBUG
    if (par_scheduling_operation == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        // Augusto 14.8.96
        //                                                p_Plant->Show_Gantt_Chart( "../DEC/gantt" , p_Graph
        //                                                );
        p_Plant->Show_Gantt_Chart("gantt", p_Graph);
        printf("SCHEDULE new operation\n");
        char in[1];
        gets(in);
    }
#endif

    p_Graph->Get_Forward_Windows(&dead_lock);
    // Not done if dead_lock == TRUE ;

    int new_penalty = p_Graph->Get_Objective();

    int fathom = FALSE;
    int could_fix_disjunctives = Could_Fix_Disjunctives(max_acceptable, remaining_jobs);

    // The following IF is not required by the heuristic if there are parallel machines.
    if (!dead_lock && (new_penalty <= max_acceptable) /* && Not_Parallel_Machines()*/)
    {
        // A dead_lock is a cycle in the graph produced by an arc
        // just inserted. Dead lock solutions are not feasible.

        p_partial_solution->Set_Objective(new_penalty);
        if (could_fix_disjunctives && (new_penalty <= max_acceptable))
        {
            if (type_oracle == EXACT_ORACLE)
                fathom = p_Plant->Fix_Disjunctions(
                    &number_arcs_list, list_arcs, max_acceptable - new_penalty, p_Graph);
            // Fix_Disjunctions() selects disjunctive arcs.
            // Tries an arc in one direction and the other,
            // and obtains the increment in the objective function.
            // fathom==TRUE if this branch should be eliminated.

            else
                fathom = p_Cluster->Fix_Disjunctions(
                    &number_arcs_list, list_arcs, max_acceptable - new_penalty, p_Graph);
            //(max_acceptable - new_penalty) is maximum increase int the objective function.
        }
        if (!fathom)
        {
            p_Graph->Get_Forward_Windows(&dead_lock);
            new_penalty = p_Graph->Get_Objective();
        }
        //                                              predecessor = p_Graph->Is_Optimal_Predecessor() ;

        //              could_calculate_bound = Can_Get_Mach_Bound(     new_penalty , fathom ,
        //                                                                              p_cluster_work_center_current->Get_Id()
        //                                                                              , p_Cluster , p_Graph
        //                                                                              ) ;

        if (Can_Get_Mach_Bound(
                new_penalty, fathom, p_cluster_work_center_current->Get_Id(), p_Cluster, p_Graph))
        {
            lower_bound = Single_Mach_Bound(p_p_input_Set_Work_Centers, &p_child_Set_Work_Centers,
                p_optimal_solution->Get_Objective(), p_Cluster, p_Graph);

            //                      number_children = Get_Children_Plus(    &list_increments ,
            //                      &next_branching_center ,
            //                                                                              &backup ,
            //                                                                              max_branches_oracle
            //                                                                              ,
            //                                                                              *p_p_input_Set_Work_Centers
            //                                                                              );
            //                                              //Actual number of branches.
            //
            //                      if ( number_children == -1 ) {
            //                                      #ifdef CHECK_DEBUG
            //                                              assert( could_fix_disjunctives == TRUE ) ;
            //                                      #endif
            //                              dead_lock = TRUE ;
            //                      }
        }
        else if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND))
        {
            p_child_Set_Work_Centers = new Set_Work_Centers();
            p_child_Set_Work_Centers->Initialize(p_Cluster->Get_Number_Work_Centers());
            p_child_Set_Work_Centers->Get_Sequence((*p_p_input_Set_Work_Centers));
        }
    }

    if (!dead_lock  // If there is no deadlock.
        &&
    // DELETE!!!!!!!!!
#ifndef ANIMATION_DEBUG
#ifndef CHECK_DEBUG
        !Time_Out()
        // Its not too long
        &&
#endif
#endif

        (new_penalty <= max_acceptable)
        // If the current solution stills competitive.
        && (lower_bound <= max_acceptable))
    {
        if (remaining_jobs == 0)
        {  // If this is the last job.
           // Notice that we know it is the best solution so far,
           // since the IF above.

            max_acceptable = new_penalty - 1;
            // max_acceptable is the general upper bound for the problem.

            *p_optimal_solution = *p_partial_solution;
            // Copy the output.

            if ((p_Set_Clusters->Number_Clusters() == 1) && (max_branches_oracle > 1)
                /* && ( type_oracle == EXACT_ORACLE )*/
            )
            {
                // if exhoustive search then:

                // SOLEX int solution_time ;
                int solution_time;
                // Augusto 12.8.96
                //                             time( &solution_time ) ;
                //                                solution_time = (clock_t)(clock())/CLOCKS_PER_SEC ;
                // Lineas agregadas por RENE
                solution_time = (clock_t)(clock()) / CLK_TCK;
                // Fin Lineas agregadas por RENE

                // time(&solution_time) escribe en la variable solution_time el numero de segundos
                // que han pasado desde cierto instante. Tipicamente, 8739284723.
                printf("{%d,%d}\n", p_optimal_solution->Get_Objective(), solution_time - first_time);
                // first_time is a global variable.
            }
        }
        else
        {  // Is not the last job, keep on branching.

            // This part prepares the branching.

            //                      if ( !could_calculate_bound ) {
            //                                              //Otherwise it was already calculated.

            number_children = Get_Children_Plus(&list_increments, &next_branching_center, &backup,
                max_branches_oracle, *p_p_input_Set_Work_Centers);
            // Ranks the remaining operations.
            // Actual number of branches.
            // Get_Children_Plus() ranks all the operations that could
            // be scheduled next.
            // To be modified for parallel machines.

            //                                      #ifdef CHECK_DEBUG
            //                                              if ( number_children == -1 ) assert(
            //                                              could_fix_disjunctives == TRUE ) ;
            //                                      #endif
            //                      }

            // max_branches_oracle = m ;

            int i = 0;
            double index;
            int chosen_operation;
            Solution_Increment* p_chosen_solution_increment;

            int bound_number = 0;
            // Number of lower bounds calculated int the WHILE.
            // Increases when one of the recursive calls calculates a lower bound.

            while ((i < number_children) &&
                   time_out == FALSE
                   // Branch as many times as needed if a feasible solution has not been reached,
                   // as long as there is time.
                   && (p_optimal_solution->Get_Objective() > original_penalty)
                   // The current solution must be non-optimal.
                   && (p_optimal_solution->Get_Objective() > max_allowed)
                // If the corrent optimal is smaller that or equal to an
                // allowed solution, then QUIT.

            )
            {
                i++;

                list_increments.Next_Best(&index, &chosen_operation, &p_chosen_solution_increment);

#ifdef CHECK_DEBUG
                assert(chosen_operation == p_chosen_solution_increment->location_operation);
                assert(next_branching_center != NULL_WORK_CENTER);
#endif

                if (type_oracle == EXACT_ORACLE)
                    this_Forbidden_Set.Recover_Data(backup, p_Cluster, next_branching_center);
                // The information for the branching center is recovered since
                // it might be overwritten int succesive recursive calls.

                Recursive_Call(p_optimal_solution, p_partial_solution, p_chosen_solution_increment,
                    remaining_jobs - 1, max_branches_oracle, &p_child_Set_Work_Centers, &bound_number);
            }

            //                      if ( ( !could_calculate_bound ) && ( number_children != -1 ) )
            //
            if (number_children != -1)
            {
                list_increments.Terminate();
                if (type_oracle == EXACT_ORACLE) delete[] backup;
                // Deallocate memory.
            }
        }  // End branching part.
    }

    //      if (    ( next_branching_center != NULL_WORK_CENTER ) &&
    //              ( type_oracle == EXACT_ORACLE ) &&
    //              ( number_children != -1 )
    //      )
    //              delete[]  backup ;

    if (p_child_Set_Work_Centers != NULL)
    {
        p_child_Set_Work_Centers->Terminate();
        delete p_child_Set_Work_Centers;
    }
    //      if ( ( could_calculate_bound ) && ( number_children != -1 ) )
    //              list_increments.Terminate();

    // Undo the assignment.
    p_cluster_work_center_current->Unschedule_Operation(p_solution_increment->machine,
        p_solution_increment->location_operation, p_Graph, p_partial_solution,
        p_solution_increment->location_work_center, save_length);

    p_Graph->Restore_Arcs(number_arcs_list, list_arcs);
    p_Graph->Set_Objective(original_penalty);

    p_partial_solution->Set_Objective(original_penalty);

    // Augusto 30.9.96 aprox
    //        p_partial_solution->Unwrite(    p_solution_increment->location_work_center ,
    //                                                        p_solution_increment->machine,
    //                                                        p_solution_increment->location_operation );

#ifdef ANIMATION_DEBUG
    if (par_scheduling_operation == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        // Augusto 14.8.96
        //                                                p_Plant->Show_Gantt_Chart( "../DEC/gantt" , p_Graph
        //                                                );
        p_Plant->Show_Gantt_Chart("gantt", p_Graph);
        printf("UNSCHEDULE new operation\n");
        char in[1];
        gets(in);
    }
#endif
}

// Public functions.

// void Act_Oracle::Initialize( Graph *p_input_Graph , Plant *p_input_Plant)
void Act_Oracle::Initialize(int input_type_oracle,
    // It can be either HEURISTIC_ORACLE, EXACT_ORACLE , BOUND_ORACLE

    Plant* p_input_Plant,
    Set_Clusters* p_input_Set_Clusters)
{
    type_oracle = input_type_oracle;
    p_Plant = p_input_Plant;
    p_Set_Clusters = p_input_Set_Clusters;
}

// Optimize():Optimizes the cluster *p_input_Cluster and places the solution in *p_optimal_solution.
// It is used when there are many machines.
void Act_Oracle::Optimize(
    // Output parameter.
    Class_Solution_Oracle* p_optimal_solution,
    // Input
    Cluster* p_input_Cluster,
    Graph* p_input_Graph,
    int max_branches_oracle,
    double input_K,
    int input_max_acceptable,
    int input_max_allowed,
    Set_Work_Centers* p_Set_Work_Centers)
{
    int dead_lock = FALSE;
    int lower_bound = INT_MIN;
    Beam_Indices<Solution_Increment> list_increments;
    // Keeps track of the posible increments of the partial solution.

#ifdef FLOW_DEBUG
    fprintf(par_output_file, "optimizing cluster %d\n", cluster_id);
#endif
    // Initialization:
    p_Cluster = p_input_Cluster;
    p_Graph = p_input_Graph;
    max_acceptable = input_max_acceptable;
    max_allowed = input_max_allowed;

    p_optimal_solution->Set_Objective(INT_MAX);

    this_ATC_Rule.Initialize(p_Graph->Get_Number_Due_Times(), p_Graph->Get_Completion_Time_Penalty(),
        p_Cluster->Get_Step_Length_Average(), input_K);
    // Ranks the job steps by the ATC rule.

    int next_branching_center = NULL_WORK_CENTER;
    int* backup;

    if ((type_oracle == EXACT_ORACLE) || (type_oracle == HEURISTIC_ORACLE))
        this_Forbidden_Set.Initialize(p_Cluster);
    // Allocates memory

    // Actual program:

    cutting_time = Get_Cutting_Time(max_branches_oracle);
    // Calculates the time at which the backtracking will be cutted.

    // int predecessor = p_Graph->Is_Optimal_Predecessor(0) ;

    p_Graph->Set_Obsolete_Forward();
    p_Graph->Get_Forward_Windows(&dead_lock);
    p_Graph->Get_Backward_Windows(FALSE);
    // Gets accurate release times and due times.

    Class_Solution_Oracle partial_solution;
    // Keeps track of the partial solution.

    p_Cluster->Initialize_Solution(&partial_solution);
    partial_solution.Set_Objective(p_Graph->Get_Objective());

    int original_penalty = partial_solution.Get_Objective();
    // For the GOTO short cut.

    int number_arcs_list = 0;
    Arc_Backup* list_arcs = new Arc_Backup[MAX_ARCS_BACKUP];
    // Each recursive call fixes a number of disjunctive arcs.
    // This list is to remember those disjunctive arcs in
    // order to return to the original situation once we get
    // out of the recursive call.

    if (Could_Fix_Disjunctives(max_acceptable, p_Cluster->Get_Number_Unscheduled_Operations()))
    {
        if (type_oracle == EXACT_ORACLE)
            // opcion B&B.
            p_Plant->Fix_Disjunctions(
                &number_arcs_list, list_arcs, max_acceptable - original_penalty, p_Graph);

        // We are fixing arcs in the graph.
        // It is recorded in list_arcs, so after quitting the recursive call
        // everything is restored.
        else
            // opcion heuristica.
            p_Cluster->Fix_Disjunctions(
                &number_arcs_list, list_arcs, max_acceptable - original_penalty, p_Graph);
        //(max_acceptable - new_penalty) is maximum increase int the objective function.
        p_Graph->Get_Forward_Windows(&dead_lock);
    }
    // predecessor = p_Graph->Is_Optimal_Predecessor(0) ;

    Set_Work_Centers* p_input_Set_Work_Centers = NULL;
    Set_Work_Centers* p_child_Set_Work_Centers = NULL;
    if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND))
        lower_bound = Single_Mach_Bound(&p_input_Set_Work_Centers, &p_child_Set_Work_Centers,
            INT_MAX,  // p_optimal_solution->Get_Objective( ) ,
            p_Cluster, p_Graph);

    int number_children = Get_Children_Plus(
        &list_increments, &next_branching_center, &backup, max_branches_oracle, p_child_Set_Work_Centers);
    // number_children is the actual number of branches.
    // Number of recursive calls from this call.
    // The children are stored in list_increments.
    // Each element in list_increment is an operation that could be scheduled next.

    int i = 0;
    double index;
    int chosen_operation;
    Solution_Increment* p_chosen_solution_increment;
    // Contains the solution to be simulated in the recursive call.
    int bound_number = 0;
    // Number of lower bounds calculated int the WHILE.
    // Increases when one of the recursive calls calculates a lower bound.

    while ((i < number_children) &&
           time_out == FALSE
           // Branch as many times as needed if a feasible solution has not been reached,
           // as long as there is time.
           && (p_optimal_solution->Get_Objective() > original_penalty)
           // The current solution must be non-optimal.
           && (p_optimal_solution->Get_Objective() > max_allowed)
           // If the corrent optimal is smaller that or equal to an
           // allowed solution, then QUIT.
           && (lower_bound <= max_acceptable)
        // This is the general upper bound for the problem.
    )
    {
        i++;

        list_increments.Next_Best(&index, &chosen_operation, &p_chosen_solution_increment);
        // This function marks the job as used, so it can be used again.
        // list_increments is a sorted list by the quality of the operations
        // to be scheduled next. The first branch is the most likely to be the best.

        if (type_oracle == EXACT_ORACLE)
            this_Forbidden_Set.Recover_Data(backup, p_Cluster, next_branching_center);
        // The information for the branching center is recovered since
        // it might be overwritten int succesive recursive calls.

        Recursive_Call(p_optimal_solution, &partial_solution, p_chosen_solution_increment,
            p_Cluster->Get_Number_Unscheduled_Operations() - 1, max_branches_oracle,
            &p_child_Set_Work_Centers,  // Pointer to a set of machines
            &bound_number               //.

        );
    }
    // WARNING!!!!!!!!!
    p_Graph->Set_Obsolete_Forward();
    p_Graph->Get_Forward_Windows(&dead_lock);
    p_Graph->Get_Backward_Windows(TRUE);
    p_Graph->Get_Objective();

    if ((next_branching_center != NULL_WORK_CENTER) && (type_oracle == EXACT_ORACLE) &&
        (number_children != -1))
        delete[] backup;

    if (p_child_Set_Work_Centers != NULL)
    {
        p_child_Set_Work_Centers->Terminate();
        delete p_child_Set_Work_Centers;
    }
    partial_solution.Terminate();

    if (number_children != -1) list_increments.Terminate();

    p_Graph->Restore_Arcs(number_arcs_list, list_arcs);
    if (time_out == TRUE) p_optimal_solution->Set_Time_Out(TRUE);

    if ((type_oracle == EXACT_ORACLE) || (type_oracle == HEURISTIC_ORACLE)) this_Forbidden_Set.Terminate();

    if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND))
    {
        p_input_Set_Work_Centers->Terminate();
        delete p_input_Set_Work_Centers;
    }
}

//--------------------------------------------------------------------

// This recursive call is for only one machine in the center.
void Act_Oracle::Recursive_Call(int* p_optimal_delta,
    int current_delta,

    Class_Solution_Oracle* p_optimal_solution,
    Class_Solution_Oracle* p_partial_solution,

    int location_operation,
    int remaining_jobs,
    int available_time,
    int max_branches_oracle,
    Cluster_Work_Center* p_center)
{
    // Do the assigment.

    p_partial_solution->Write(0, 0, location_operation);
    // location_work_center , machine , location_operation.

    int* p_backup_release_times;
    p_center->Back_Up(&p_backup_release_times);

    int new_available_time;
    int* max_violation;
    int new_delta = p_center->Try_Operation(
        &new_available_time, &max_violation, available_time, location_operation, p_Graph);
    // Temporary arcs are added.

    int number_fixed = -1;
    int* delay_from = NULL;
    int* delay_to = NULL;
    int fathom = FALSE;
    if ((max_acceptable < INT_MAX / 2) && (new_delta <= max_acceptable))
    {
        fathom = p_center->Fix_Precedences(&number_fixed, &delay_from, &delay_to, max_violation,
            max_acceptable - new_delta,
            // Maximum index.
            p_Graph);
    }
    else
    {
        delete[] max_violation;
    }
    if ((new_delta <= max_acceptable)
        // If the current solution stills competitive.
        && !fathom)
    {
        if (remaining_jobs == 0)
        {
            // If this is the last job.
            // Notice that we know it is the best solution so far,
            // since the IF above.

            max_acceptable = new_delta - 1;
            // max_acceptable is the general upper bound for the problem.

            *p_optimal_delta = new_delta;
            *p_optimal_solution = *p_partial_solution;
            // Copy the output.
        }
        else
        {  // Is not the last job, keep on branching.

            // This part prepares the branching.

            Beam_Indices<Solution_Increment> list_increments;
            // Stores the possible increments int a sorted way.

            list_increments.Initialize(remaining_jobs, remaining_jobs, 0);
            p_center->Clean_Star();
            //                      if ( type_oracle == HEURISTIC_ORACLE )
            p_center->Completion_Star_Marked(p_Graph);
            // Finds the star completion time and operation for the active generation rule
            // among the not scheduled and not marked.

            int number_children = p_center->Filter_Rank_Operations(
                &list_increments, new_available_time, p_Graph, &this_ATC_Rule);
            number_children = min2(number_children, max_branches_oracle);
            // Actual number of branches.

            int i = 0;
            double index;
            int chosen_operation;
            Solution_Increment* p_chosen_solution_increment;

            while ((i < number_children) &&
                   (*p_optimal_delta > current_delta)
                   // The current solution must be non-optimal.
                   && (*p_optimal_delta > max_allowed)
                // If the corrent optimal is smaller that or equal to an
                // allowed solution, then QUIT.
            )
            {
                i++;

                list_increments.Next_Best(&index, &chosen_operation, &p_chosen_solution_increment);

#ifdef CHECK_DEBUG
                assert(chosen_operation == p_chosen_solution_increment->location_operation);
#endif

                Recursive_Call(p_optimal_delta, new_delta, p_optimal_solution, p_partial_solution,
                    chosen_operation, remaining_jobs - 1, new_available_time, max_branches_oracle, p_center);
            }

            list_increments.Terminate();
            // Deallocate memory.

        }  // End branching part.
    }

    // Undo the assignment.
    p_center->Untry_Operation(location_operation);
    p_center->Restore(p_backup_release_times, number_fixed, delay_from, delay_to, p_Graph);

    p_partial_solution->Unwrite(0, 0, location_operation);
    // location_work_center , machine , location_operation.
}

void Act_Oracle::Optimize(Class_Solution_Oracle* p_optimal_solution,

    Cluster* p_input_Cluster,
    Graph* p_input_Graph,
    int max_branches_oracle,
    double input_K,
    int input_max_acceptable,
    int input_max_allowed)
{
    // Initialization:
    p_Graph = p_input_Graph;

    int optimal_delta = INT_MAX / 2;

    // Luis Peña abril 1998
    int res_branches_oracle = max_branches_oracle;

    this_ATC_Rule.Initialize(p_Graph->Get_Number_Due_Times(), p_Graph->Get_Completion_Time_Penalty(),
        p_input_Cluster->Get_Step_Length_Average(), input_K);
    // Ranks the job steps by the ATC rule.

#ifdef CHECK_DEBUG
    assert(p_input_Cluster->Get_Number_Work_Centers() == 1);
#endif
    Cluster_Work_Center* p_center = p_input_Cluster->Get_Cluster_Work_Center(0);
    // This method works only with one center clusters.

    // Luis Peña Abril 1998

    int num_op = p_center->Get_Number_Operations();

    if (num_op >= 20)
        max_branches_oracle = 1;
    else if (num_op >= 10)
    {
        int max_branch_arr[] = {5, 5, 5, 4, 3, 2, 2, 2, 2, 1};
        // 10  11  12  13  14  15  16  17  18  19
        max_branches_oracle = max_branch_arr[num_op - 10];
    }

    /*
            if(p_center->Get_Number_Operations() > 14 && p_center->Get_Number_Operations()<=20){
                    res_branches_oracle= max_branches_oracle;
                    max_branches_oracle=5;
            }
            if(p_center->Get_Number_Operations()>20){
                    res_branches_oracle= max_branches_oracle;
                    max_branches_oracle=1;
            }
    */
    // Actual program:
#ifdef CHECK_DEBUG
    if (p_Graph->Is_Updated() == FALSE) assert(p_Graph->Is_Updated());
#endif

    int original_penalty = p_Graph->Get_Objective();
    // For the GOTO short cut.

    max_acceptable = input_max_acceptable - original_penalty;
    // Is the delta from the original value.

    if (input_max_allowed > INT_MIN) max_allowed = input_max_allowed - original_penalty;
    // Convert maximum values to maximum deltas.
    else
        max_allowed = input_max_allowed;
    Class_Solution_Oracle partial_solution;
    // Keeps track of the partial solution.

    p_input_Cluster->Initialize_Solution(&partial_solution);

    Beam_Indices<Solution_Increment> list_increments;
    // Keeps track of the posible increments of the partial solution.

    int remaining_jobs = p_center->Get_Number_Unscheduled_Operations();
    list_increments.Initialize(remaining_jobs, remaining_jobs, 0);
    int available_time, next_idle_machine;

    p_center->Copy_Release_Times();
    p_center->Get_Available_Time(&available_time, &next_idle_machine, p_Graph);
    p_center->Clean_Star();
    //      if ( type_oracle == HEURISTIC_ORACLE )
    p_center->Completion_Star_Marked(p_Graph);
    // Finds the star completion time and operation for the active generation rule
    // among the not scheduled and not marked.

    int number_children =
        p_center->Filter_Rank_Operations(&list_increments, available_time, p_Graph, &this_ATC_Rule);
    number_children = min2(number_children, max_branches_oracle);
    // Actual number of branches.

#ifdef CHECK_DEBUG
    assert(number_children > 0);
#endif
    int i = 0;
    double index;
    int chosen_operation;
    Solution_Increment* p_chosen_solution_increment;
    // Contains the solution to be simulated int the recursive call.

    while ((i < number_children) &&
           (optimal_delta > 0)
           // The current solution must be non-optimal.
           && (optimal_delta > max_allowed)
        // If the corrent optimal is smaller that or equal to an
        // allowed solution, then QUIT.
    )
    {
        i++;

        list_increments.Next_Best(&index, &chosen_operation, &p_chosen_solution_increment);
        // This function marks the job as used, so it can be used again.

        Recursive_Call(&optimal_delta, 0,  // Current delta is zero.
            p_optimal_solution, &partial_solution, chosen_operation, remaining_jobs - 1, available_time,
            max_branches_oracle, p_center);
    }
    list_increments.Terminate();
    partial_solution.Terminate();
    if (optimal_delta <= input_max_acceptable - original_penalty)
        p_optimal_solution->Set_Objective(original_penalty + optimal_delta);
    else
        p_optimal_solution->Set_Objective(INT_MAX);

    // Luis Peña Abril 1998
    if (res_branches_oracle != 0) max_branches_oracle = res_branches_oracle;
}
