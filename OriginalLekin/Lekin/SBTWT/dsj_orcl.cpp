#include "StdAfx.h"

#include "dsj_orcl.h"

int Dsj_Oracle::Could_Fix_Disjunctives(int this_max_acceptable, int depth)
{
    if (this_max_acceptable < INT_MAX) return (TRUE);
    /*//WARNING!!!!!!!!!!!!
                    if ( depth % 2 == 0 )
                            return( TRUE ) ;
                    else
                            return( FALSE ) ;
    */
    else
        return (FALSE);
}

int Dsj_Oracle::Can_Get_Mach_Bound(int new_penalty,
    int center,
    Cluster* p_Cluster,
    Graph* p_Graph  // Position in the WHILE.
)
{
    if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND) && (new_penalty <= max_acceptable) &&
        (p_Cluster->Get_Number_Work_Centers() > 2)
        // Three or more work centers in the cluster.
        && (p_Graph->Get_N_Blank_Centers(center) == 0) && (p_Graph->Get_Number_Blanks() > 0))
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

// Get_Arcs_Center(): Returns INT_MIN if all the arcs have been assigned.
int Dsj_Oracle::Get_Arcs_Center(Class_Disjunctive_Arc* p_first_arc,
    Class_Disjunctive_Arc* p_second_arc,
    int* p_location_branch_center,
    Graph* p_Graph,
    int depth,
    Set_Work_Centers* p_Set_Centers,
    Cluster* p_Cluster)
{
    int dead_lock = FALSE;

    p_Graph->Get_Forward_Windows(&dead_lock);
    if (dead_lock == TRUE) return (INT_MIN);

#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif
    if (depth % 5 == 0) p_Graph->Get_Backward_Windows(FALSE);

    if (p_Cluster->Get_Number_Work_Centers() < p_Graph->Number_Centers_Plant())
    {
#ifdef CHECK_DEBUG
        //                                              assert( type_oracle == HEURISTIC_ORACLE ) ;
#endif
        switch (par_selection_rule)
        {
                //              case EARLY :
                //              case ESTIMATE :
            case BOUND:
                return (p_Cluster->Arcs_Max_Increase_Center(
                    p_first_arc, p_second_arc, p_location_branch_center, p_Graph, depth));
            case BOTTLE:
                return (p_Cluster->Arcs_Bottleneck_Center(
                    p_first_arc, p_second_arc, p_location_branch_center, p_Graph, depth));
        }
    }
    else
    {
        switch (par_selection_rule)
        {
                //              case EARLY :
                //              case ESTIMATE :
                //                      return( p_Cluster->Arcs_Max_Increase_Center(    p_first_arc ,
                //                      p_second_arc ,
                //                                                                                              p_location_branch_center ,  p_Graph ,depth ) ) ;
            case BOTTLE:
                return (p_Cluster->Arcs_Bottleneck_Center(
                    p_first_arc, p_second_arc, p_location_branch_center, p_Graph, depth));
            case BOUND:
                return (p_Cluster->Arcs_Max_Bound_Center(
                    p_first_arc, p_second_arc, p_location_branch_center, p_Graph, depth, p_Set_Centers));
        }
    }
    assert(2 == 1);
    return (0);
}

int Dsj_Oracle::Switch_Active_Branching(int depth, Graph* p_Graph)
{
    if (
        //              ( depth > par_number_blanks )   //0->Disjunctive;
        //                                                              //1000->Active
        //              &&
        (par_transition_speed > 0) && (p_Graph->Get_Number_Blanks() <= par_transition_speed)
        // 0->Disjunctive;
        // 1000->Active
        //              &&
        //              p_Graph->Few_Blanks_Centers( par_number_blanks )
        // TRUE if each one of the centers has par_number_blanks or less blanks in the matrix.
    )
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

void Dsj_Oracle::Recursive_Call(
    // Output parameters.
    Class_Solution_Oracle* p_optimal_solution,

    // Recursive input paramenters.
    int original_penalty,

    Class_Disjunctive_Arc* p_disjunctive_arc,
    int location_center,

    int depth,

    Set_Work_Centers** p_p_input_Set_Work_Centers,
    // Could the the parent's or a left sibling's

    int* p_bound_number)
{
    /*//DELETE!!!!!!!!*/
    int backup_par_selection_rule = par_selection_rule;

    int dead_lock;
    int lower_bound = INT_MIN;
    // It remains INT_MIN if no lower bound was calculated.
    // It becomes ) if they are calculated, but are not effective.
    // Are positive if and only if the lower bound is efective.

    // Beam_Indices<Solution_Increment> list_increments ;
    // Stores the possible increments in a sorted way.

    Class_Disjunctive_Arc first_arc, second_arc;
    int next_branching_center = NULL_WORK_CENTER;

    int could_calculate_bound = FALSE;
    // int number_children ;

    Set_Work_Centers* p_child_Set_Work_Centers = NULL;

    // Do the assigment.

    Cluster_Work_Center* p_current_center = p_Cluster->Get_Cluster_Work_Center(location_center);
    // Points to the Cluster_Work_Center representing the work center
    // being scheduled in this iteration.

    int number_arcs_list = 0;
    Arc_Backup* list_arcs = new Arc_Backup[MAX_ARCS_BACKUP];

    dead_lock = p_Graph->Add_Disjunctive_New(&number_arcs_list, list_arcs, p_disjunctive_arc->job_from,
        p_disjunctive_arc->step_from, p_disjunctive_arc->job_to, p_disjunctive_arc->step_to, TEMP);
    // Temporary arcs are added.

#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif

    // int predecessor = p_Graph->Is_Optimal_Predecessor(0) ;

    p_Graph->Get_Forward_Windows(&dead_lock);
    int new_penalty = INT_MAX;
    int index = INT_MIN;
    if (!dead_lock)
    {
        new_penalty = p_Graph->Get_Objective();
        if ((new_penalty <= max_acceptable) && Could_Fix_Disjunctives(max_acceptable, depth))
        {
            new_penalty = new_penalty +
                          //                                              p_Cluster->Fix_Disjunctions(
                          //                                              &number_arcs_list , list_arcs ,
                          p_Plant->Fix_Disjunctions(
                              &number_arcs_list, list_arcs, max_acceptable - new_penalty, p_Graph);
            //(max_acceptable - new_penalty) is maximum increase in the objective function.
        }
        // predecessor = p_Graph->Is_Optimal_Predecessor(0) ;

        if (new_penalty <= max_acceptable)
        {
            p_Graph->Get_Forward_Windows(&dead_lock);
            new_penalty = p_Graph->Get_Objective();
        }
        could_calculate_bound =
            Can_Get_Mach_Bound(new_penalty, p_current_center->Get_Id(), p_Cluster, p_Graph);

        if (could_calculate_bound)
        {
            lower_bound = this_Act_Exact_Orcl.Single_Mach_Bound(p_p_input_Set_Work_Centers,
                &p_child_Set_Work_Centers, p_optimal_solution->Get_Objective(), p_Cluster, p_Graph);
        }
        else if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND))
        {
            //                      p_child_Set_Work_Centers = *p_p_input_Set_Work_Centers ;
            p_child_Set_Work_Centers = new Set_Work_Centers();
            p_child_Set_Work_Centers->Initialize(p_Cluster->Get_Number_Work_Centers());
            p_child_Set_Work_Centers->Get_Sequence((*p_p_input_Set_Work_Centers));
        }
        if ((new_penalty <= max_acceptable)
            // If the current solution stills competitive.
            && (lower_bound <= max_acceptable)
            // This is the general upper bound for the problem.
        )
        {
            index = Get_Arcs_Center(&first_arc, &second_arc, &next_branching_center, p_Graph, depth,
                p_child_Set_Work_Centers, p_Cluster);
            // Next branching arcs.
        }
    }

    if (!dead_lock  // If there is no deadlock.
        && (new_penalty <= max_acceptable)
        // If the current solution stills competitive.
        && (lower_bound <= max_acceptable)
        // This is the general upper bound for the problem.
    )
    {
        if ((index == INT_MIN) && p_Cluster->Valid_Answer(p_Graph))
        {
            // TRUE if there is the proper number of selected arcs

            p_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
            assert(dead_lock == FALSE);
#endif

            int this_new_penalty = p_Graph->Get_Objective();
            if (this_new_penalty <= max_acceptable)
            {
                if (par_option == LOWER_BOUND)
                {
                    // SOLEX int solution_time ;
                    int solution_time;
                    // Augusto 12.8.96
                    //                                      time( &solution_time ) ;
                    //                                        solution_time = (int )(clock()) /CLOCKS_PER_SEC
                    //                                        ;
                    solution_time = (int)(clock()) / CLK_TCK;
                    printf("( %d , %d ) ; ", this_new_penalty, solution_time - first_time);
                    // first_time is a global variable.
                }

                this_Act_Heur_Orcl.Optimize(p_optimal_solution, p_Cluster, p_Graph, INT_MAX, global_K,
                    INT_MAX, INT_MIN, *p_p_input_Set_Work_Centers);
                // Obtains the solution in gantt chart form.
#ifdef CHECK_DEBUG
                assert(this_new_penalty == p_optimal_solution->Get_Objective());
#endif

                max_acceptable = this_new_penalty - 1;
            }
        }
        else if (Switch_Active_Branching(depth, p_Graph))
        {
            // Switch to the active schedule branching.

#ifdef CHECK_DEBUG
            p_Graph->Get_Forward_Windows(&dead_lock);
            int check_old_penalty = p_Graph->Get_Objective();
#endif

            //                      int this_max_acceptable = min2( current_optimal , max_acceptable ) ;
            int par_selection_rule_backup = par_selection_rule;
            // WARNING!!!!!
            par_selection_rule = BOTTLE;

            Class_Solution_Oracle switch_solution_oracle;
            p_Cluster->Initialize_Solution(&switch_solution_oracle);
            this_Act_Exact_Orcl.Optimize(&switch_solution_oracle, p_Cluster, p_Graph, INT_MAX, global_K,
                max_acceptable, INT_MIN, *p_p_input_Set_Work_Centers);

#ifdef CHECK_DEBUG
            p_Graph->Get_Forward_Windows(&dead_lock);
            int check_new_penalty = p_Graph->Get_Objective();
#endif

            if (switch_solution_oracle.Get_Objective() < p_optimal_solution->Get_Objective())
            {
                *p_optimal_solution = switch_solution_oracle;

                max_acceptable = p_optimal_solution->Get_Objective() - 1;

#ifdef CHECK_DEBUG
                p_Graph->Get_Forward_Windows(&dead_lock);
                int check_new_penalty = p_Graph->Get_Objective();
                if (check_old_penalty == check_new_penalty) assert(check_old_penalty == check_new_penalty);
                if (check_new_penalty > p_optimal_solution->Get_Objective())
                    assert(check_new_penalty <= p_optimal_solution->Get_Objective());
#endif
            }
            switch_solution_oracle.Terminate();
            par_selection_rule = par_selection_rule_backup;
        }
        else if (index != INT_MIN)
        {  // Is not the last job, keep on branching.

            // This part prepares the branching.
            int bound_number = 0;
            // Number of lower bounds calculated in the WHILE.
            // Increases when one of the recursive calls calculates a lower bound.

            if (p_optimal_solution->Get_Objective() > max_allowed)
            {
                // If the corrent optimal is smaller that or equal to an
                // allowed solution, then QUIT.

                Recursive_Call(p_optimal_solution, new_penalty, &first_arc, next_branching_center, depth + 1,
                    &p_child_Set_Work_Centers, &bound_number);
            }
            if ((p_optimal_solution->Get_Objective() > original_penalty)
                // The current solution must be non-optimal.
                && (p_optimal_solution->Get_Objective() > max_allowed)
                // If the corrent optimal is smaller that or equal to an
                // allowed solution, then QUIT.

            )
            {
                Recursive_Call(p_optimal_solution, new_penalty, &second_arc, next_branching_center, depth + 1,
                    &p_child_Set_Work_Centers, &bound_number);
            }

        }  // End branching part.
    }

    if (p_child_Set_Work_Centers != NULL)
    {
        p_child_Set_Work_Centers->Terminate();
        delete p_child_Set_Work_Centers;
    }

    // Undo the assignment.
    p_Graph->Restore_Arcs(number_arcs_list, list_arcs);
    p_Graph->Set_Objective(original_penalty);

    /*//DELETE!!!!!!!!*/
    par_selection_rule = backup_par_selection_rule;
}

// Public functions.

// void Dsj_Oracle::Initialize( Graph *p_input_Graph , Plant *p_input_Plant)
void Dsj_Oracle::Initialize(int input_type_oracle,
    // It can be either HEURISTIC_ORACLE, EXACT_ORACLE

    Plant* p_input_Plant,
    Set_Clusters* p_input_Set_Clusters)
{
    type_oracle = input_type_oracle;
#ifdef CHECK_DEBUG
    assert(type_oracle == HEURISTIC_ORACLE || type_oracle == EXACT_ORACLE);
#endif
    p_Plant = p_input_Plant;
    p_Set_Clusters = p_input_Set_Clusters;

    this_Act_Exact_Orcl.Initialize(EXACT_ORACLE, p_input_Plant, p_input_Set_Clusters);
    this_Act_Heur_Orcl.Initialize(HEURISTIC_ORACLE, p_input_Plant, p_input_Set_Clusters);
    // They do NOT allocate memory.
}

void Dsj_Oracle::Optimize(
    // Output parameter.
    Class_Solution_Oracle* p_optimal_solution,
    // Input
    Cluster* p_input_Cluster,
    Graph* p_input_Graph,
    int input_max_acceptable,
    int input_max_allowed)
{
    int lower_bound = INT_MIN;
    // It remains INT_MIN if no lower bound was calculated.
    // It becomes ) if they are calculated, but are not effective.
    // Are positive if and only if the lower bound is efective.

    // Initialization:
    p_Cluster = p_input_Cluster;
    p_Graph = p_input_Graph;
    max_acceptable = input_max_acceptable;
    max_allowed = input_max_allowed;

    p_optimal_solution->Set_Objective(INT_MAX);

    Class_Disjunctive_Arc first_arc, second_arc;
    int next_branching_center = NULL_WORK_CENTER;

    // Actual program:

    //      cutting_time = Get_Cutting_Time( ) ;
    // Calculates the time at which the backtracking will be cutted.

    p_Graph->Set_Obsolete_Forward();
    int dead_lock = FALSE;
    p_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif
    p_Graph->Get_Backward_Windows(FALSE);
    // Gets accurate release times and due times.
    // Does not requiere delay precedence constraints.

    int original_penalty = p_Graph->Get_Objective();
    // For the GOTO short cut.

    int number_arcs_list = 0;
    Arc_Backup* list_arcs = new Arc_Backup[MAX_ARCS_BACKUP];

    if (Could_Fix_Disjunctives(max_acceptable, 0))
    {
        p_Plant->Fix_Disjunctions(&number_arcs_list, list_arcs, max_acceptable - original_penalty, p_Graph);
        //(max_acceptable - new_penalty) is maximum increase in the objective function.
        p_Graph->Get_Forward_Windows(&dead_lock);
    }
#ifdef CHECK_DEBUG
    assert(type_oracle == EXACT_ORACLE);
#endif
    Set_Work_Centers* p_input_Set_Work_Centers = NULL;
    Set_Work_Centers* p_child_Set_Work_Centers = NULL;
    if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND))
        lower_bound = this_Act_Exact_Orcl.Single_Mach_Bound(
            &p_input_Set_Work_Centers, &p_child_Set_Work_Centers, INT_MAX, p_Cluster, p_Graph);

    int index = INT_MIN;
    if (lower_bound <= max_acceptable)
        index = Get_Arcs_Center(
            &first_arc, &second_arc, &next_branching_center, p_Graph, 0, p_child_Set_Work_Centers, p_Cluster);
    // Next branching arcs.

    int bound_number = 0;  // Number of lower bounds calculated in the WHILE.
                           // Increases when one of the recursive calls calculates a lower bound.

    if ((index == INT_MIN) && (lower_bound < INT_MAX) && p_Cluster->Valid_Answer(p_Graph))
    {
        // TRUE if there is the proper number of selected arcs

        p_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
        assert(dead_lock == FALSE);
#endif

        int this_new_penalty = p_Graph->Get_Objective();
        if (this_new_penalty <= max_acceptable)
        {
            this_Act_Heur_Orcl.Optimize(p_optimal_solution, p_Cluster, p_Graph, INT_MAX, global_K, INT_MAX,
                INT_MIN, p_child_Set_Work_Centers);
#ifdef CHECK_DEBUG
            assert(this_new_penalty == p_optimal_solution->Get_Objective());
#endif
        }
    }
    else if ((index != INT_MIN) && (lower_bound < INT_MAX))
    {
        Recursive_Call(p_optimal_solution, original_penalty, &first_arc, next_branching_center, 1,
            &p_child_Set_Work_Centers, &bound_number);
        if ((p_optimal_solution->Get_Objective() > original_penalty)
            // The current solution must be non-optimal.
            && (p_optimal_solution->Get_Objective() > max_allowed)
            // If the corrent optimal is smaller that or equal to an
            // allowed solution, then QUIT.
        )
        {
            Recursive_Call(p_optimal_solution, original_penalty, &second_arc, next_branching_center, 1,
                &p_child_Set_Work_Centers, &bound_number);
        }
    }  // End branching part.

    if (p_child_Set_Work_Centers != NULL)
    {
        p_child_Set_Work_Centers->Terminate();
        delete p_child_Set_Work_Centers;
    }
    p_Graph->Restore_Arcs(number_arcs_list, list_arcs);

    if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND))
    {
        p_input_Set_Work_Centers->Terminate();
        delete p_input_Set_Work_Centers;
    }

    //      if ( time_out == TRUE )
    //              p_optimal_solution->Set_Time_Out( TRUE ) ;
}
