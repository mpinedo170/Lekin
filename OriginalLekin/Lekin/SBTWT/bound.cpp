#include "StdAfx.h"

#include "act_orcl.h"

// int Act_Oracle::Could_Calculate_Lower_Bound: returns TRUE if a lower bound could be calculated.
int Act_Oracle::Could_Calculate_Lower_Bound(int new_penalty,
    int current_optimal,
    Cluster* p_Cluster,
    Set_Work_Centers* p_in_Set_Work_Centers,
    int bound_number)  // Position in the WHILE.
{
    if ((type_oracle != HEURISTIC_ORACLE)
        // If performing complete enumeration.
        && (new_penalty < current_optimal) && (new_penalty <= max_acceptable) &&
        (p_Cluster->Get_Number_Unscheduled_Operations() > 20)
        // otherwise it pays to continue the backtracking.
        //              &&
        //              ( p_Cluster->Get_Number_Unscheduled_Operations() % 5 == 0 )
        // otherwise it pays to continue the backtracking.
        && (p_Cluster->Get_Number_Work_Centers() > 2)
        // Three or more work centers in the cluster.
        && ((bound_number == 0)
               // Is the left most sibling.
               || p_in_Set_Work_Centers->Some_Machine_Effective()
               // Some left sibling was effective.
               ))
    {
        if (par_selection_rule == 0)
            // Earliest completion time rule.

            if (p_Cluster->Get_Number_Scheduled_Operations() > 13)
                return (TRUE);
            else
                return (FALSE);
        else if (p_Cluster->Get_Number_Scheduled_Operations() > 1)
            return (TRUE);
        else
            return (FALSE);
    }
    else
    {
        return (FALSE);
    }
}

int Act_Oracle::Can_Get_Mach_Bound(int new_penalty,
    int fathom,
    int center,
    Cluster* p_Cluster,
    Graph* p_Graph  // Position in the WHILE.
)
{
    if ((type_oracle == EXACT_ORACLE) && (par_selection_rule == BOUND) && (new_penalty <= max_acceptable) &&
        !fathom &&
        (p_Cluster->Get_Number_Work_Centers() > 2)
        // Three or more work centers in the cluster.
        && (p_Cluster->Get_Number_Unscheduled_Work_Center(center) == 0)
        // Finishing to schedule a machine.
        && (p_Cluster->Get_Number_Unscheduled_Operations() > 0))
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

int Act_Oracle::Get_Number_Unscheduled(int* location, int number_centers, Cluster* p_input_Cluster)
{
    int i;
    Cluster_Work_Center* p_cluster_work_center_next;
    int number_unscheduled = 0;

    for (i = 0; i < number_centers; i++)
    {
        p_cluster_work_center_next = p_input_Cluster->Get_Cluster_Work_Center(location[i]);

        number_unscheduled =
            number_unscheduled + p_cluster_work_center_next->Get_Number_Unscheduled_Operations();
    }
    return (number_unscheduled);
}

// Get_Bound_N(): gets a lower bound from any number of machines.
int Act_Oracle::Get_Bound_N(int* p_time_out,
    int* location,  // Contains the location of the cluster
                    // work centers in the cluster.
    int number_centers,
    int this_max_acceptable,
    int this_max_allowed,
    Cluster* p_input_Cluster,
    Graph* p_Graph)

{
    int i;
    Class_Solution_Oracle optimal_solution_graph, optimal_solution_delayed;
    Cluster this_Cluster;
    Act_Oracle this_Oracle_Delay;

    this_Oracle_Delay.Initialize(BOUND_ORACLE, p_Plant, p_Set_Clusters);

    // Creating the cluster.
    this_Cluster.Initialize(0, number_centers, p_Graph);
    //(id , Number of work centers).
    for (i = 0; i < number_centers; i++)
        this_Cluster.Assign_p_Work_Center(p_input_Cluster->Get_Cluster_Work_Center(location[i]));

    // Creating the solution.
    this_Cluster.Initialize_Solution(&optimal_solution_delayed);

    this_Oracle_Delay.Optimize(&optimal_solution_delayed, &this_Cluster, p_Graph, INT_MAX, global_K,
        this_max_acceptable, this_max_allowed);
    optimal_solution_delayed.Terminate();
    this_Cluster.Terminate();

    *p_time_out = optimal_solution_delayed.Get_Time_Out();
    return (optimal_solution_delayed.Get_Objective());
}

int Act_Oracle::Single_Mach_Bound(Set_Work_Centers** p_p_in_Set_Work_Centers,
    Set_Work_Centers** p_p_out_child_Set_Work_Centers,
    int best_feasible_value,
    Cluster* p_input_Cluster,
    Graph* p_Graph)

{
#ifdef CHECK_DEBUG
    assert(global_K != -1);
    assert(p_Graph->Is_Updated_Forward());
#endif

    int best_location_one[1];
    int number_unscheduled_operations;
    int i;
    int lower_bound, previous_lower_bound, time_out;  // Output from Get_Bound_N().
    int this_center_id;

    int is_effective = FALSE;

    Set_Work_Centers* p_out_sibling_Set_Work_Centers;
    // Will swap with *p_p_in_Set_Work_Centers if there
    // a new sequence.

    int this_max_acceptable;  // It continuos with a current solution if it is equal to
                              // or less than this_max_acceptable.

    int this_max_allowed;  // It quits the recursion if the solution is smaller than or
                           // equal to this_max_allowed.

    int number_work_centers_cluster = p_input_Cluster->Get_Number_Work_Centers();

#ifdef CHECK_DEBUG
    int par_scheduling_operation_backup = par_scheduling_operation;
    par_scheduling_operation = FALSE;
    // Backing up.

    assert(p_Graph->Is_Updated_Forward());
#endif

    p_Graph->Increase_Counter();  // So Set_Obsolete_Backward() runs OK.
    p_Graph->Set_Obsolete_Backward();
    p_Graph->Get_Backward_Windows(TRUE);
    p_Graph->Get_Objective();

    if (best_feasible_value == INT_MAX)
    {
        // A solution better than the initial upper bound has not
        // reached (yet).

        this_max_acceptable = par_upper_bound;
        // The external upper bound is not acceptable.
        // A solution less than the uppar bound is searched.

        this_max_allowed = this_max_acceptable + 1;
    }
    else
    {
        this_max_acceptable = best_feasible_value;
        // The external upper bound is not acceptable.
        // A solution less than the upper bound is searched.

        this_max_allowed = this_max_acceptable + 1;
    }

    // This loop is the one machine lower bound.
    if ((*p_p_in_Set_Work_Centers) == NULL)
    {
        (*p_p_in_Set_Work_Centers) = new Set_Work_Centers();
        (*p_p_in_Set_Work_Centers)->Initialize(number_work_centers_cluster);
    }

    p_out_sibling_Set_Work_Centers = new Set_Work_Centers();
    p_out_sibling_Set_Work_Centers->Initialize(number_work_centers_cluster);
    // For recording the new sequence.

    for (i = 0; i < number_work_centers_cluster; i++)
    {
        // Perform the TWO machines lower bound.

        // Finding the best locations.
        (*p_p_in_Set_Work_Centers)
            ->Get_Highest_Locations_One(
                best_location_one, &previous_lower_bound, &number_unscheduled_operations, i);
        if (!is_effective)
        {
            // There is no effective lower bound yet.

            number_unscheduled_operations = Get_Number_Unscheduled(best_location_one, 1, p_input_Cluster);

            this_center_id = (p_input_Cluster->Get_Cluster_Work_Center(best_location_one[0]))->Get_Id();

            if (p_Graph->Get_N_Blank_Centers(this_center_id) > 0)
            {
                // Only few operations.

                lower_bound = Get_Bound_N(
                    &time_out, best_location_one, 1, this_max_acceptable, INT_MIN, p_input_Cluster, p_Graph);

                if ((lower_bound > this_max_acceptable) && (time_out == FALSE))
                {
                    // If the lower bound is effective:

                    is_effective = TRUE;

                    p_out_sibling_Set_Work_Centers->Set_N_Effective(best_location_one, 1);
                    // The sibling will get the new sequence,
                    // but knowing best_location_one[0] was effective.

                    p_out_sibling_Set_Work_Centers->Add(best_location_one[0], this_max_acceptable,
                        number_unscheduled_operations, best_location_one[0]);
                }
                else if (time_out == FALSE)
                {
                    // It finds a solution bellow the lower bound.

                    p_out_sibling_Set_Work_Centers->Add(best_location_one[0], lower_bound,
                        number_unscheduled_operations, best_location_one[0]);
                }
                else
                {
                    // It does not have time to find a solution.

                    p_out_sibling_Set_Work_Centers->Add(best_location_one[0], previous_lower_bound,
                        number_unscheduled_operations, best_location_one[0]);
                }
            }
            else
            {
                p_out_sibling_Set_Work_Centers->Add(best_location_one[0], previous_lower_bound,
                    number_unscheduled_operations, best_location_one[0]);
                // previous_lower_bound = -1 if not processed
            }
        }
        else
        {
            p_out_sibling_Set_Work_Centers->Add(best_location_one[0], previous_lower_bound,
                number_unscheduled_operations, best_location_one[0]);
        }
    }

    // If all the machines were recalculated:
    (*p_p_in_Set_Work_Centers)->Terminate();
    delete (*p_p_in_Set_Work_Centers);
    (*p_p_in_Set_Work_Centers) = p_out_sibling_Set_Work_Centers;
    // There is a new sequence, so the previous one
    // is deleted and replaced by p_out_sibling_Set_Work_Centers.

    if (is_effective)
    {
#ifdef CHECK_DEBUG
        par_scheduling_operation = par_scheduling_operation_backup;
#endif
        return (INT_MAX);
    }
    else
    {
        (*p_p_out_child_Set_Work_Centers) = new Set_Work_Centers();
        (*p_p_out_child_Set_Work_Centers)->Initialize(number_work_centers_cluster);
        (*p_p_out_child_Set_Work_Centers)->Get_Sequence((*p_p_in_Set_Work_Centers));
        // Copies everything EXCEPT that some lower bound has been effective

#ifdef CHECK_DEBUG
        par_scheduling_operation = par_scheduling_operation_backup;
#endif
        return (0);
    }
}

int Act_Oracle::Get_Lower_Bound(Set_Work_Centers** p_p_in_Set_Work_Centers,
    Set_Work_Centers** p_p_out_child_Set_Work_Centers,
    int bound_number,  // Position in the WHILE.
    int number_children,
    //                                              int branching_center ,
    int best_feasible_value,
    Cluster* p_input_Cluster,
    Graph* p_Graph)

{
#ifdef CHECK_DEBUG
    assert(global_K != -1);
#endif

    int best_location_one[1], best_location_two[2], best_location_three[3], best_location_four[4],
        best_location_five[5];
    int total_value;
    int number_unscheduled_operations;
    int i;
    int lower_bound, previous_lower_bound, time_out;  // Output from Get_Bound_N().
    int this_center_id;

    int is_effective = FALSE;

    Set_Work_Centers* p_out_sibling_Set_Work_Centers;
    // Will swap with *p_p_in_Set_Work_Centers if there
    // a new sequence.

    int this_max_acceptable;  // It continuos with a current solution if it is equal to
                              // or less than this_max_acceptable.

    int this_max_allowed;  // It quits the recursion if the solution is smaller than or
                           // equal to this_max_allowed.

    int max_number_bounds_one = number_children + par_max_number_bounds_one;
    int max_number_bounds_two = number_children + par_max_number_bounds_two;
    int max_number_bounds_three = number_children + par_max_number_bounds_three;
    int max_number_bounds_four = number_children + par_max_number_bounds_four;
    int max_number_bounds_five = number_children + par_max_number_bounds_five;

    int number_work_centers_cluster = p_input_Cluster->Get_Number_Work_Centers();

#ifdef CHECK_DEBUG
    int par_scheduling_operation_backup = par_scheduling_operation;
    par_scheduling_operation = FALSE;
    // Backing up.
#endif

    if (best_feasible_value == INT_MAX)
    {
        // A solution better than the initial upper bound has not
        // reached (yet).

        this_max_acceptable = par_upper_bound;
        // The external upper bound is not acceptable.
        // A solution less than the upper bound is searched.

        this_max_allowed = this_max_acceptable + 1;
    }
    else
    {
        this_max_acceptable = best_feasible_value;
        // The external upper bound is not acceptable.
        // A solution less than the upper bound is searched.

        this_max_allowed = this_max_acceptable + 1;
    }

    // This loop is the one machine lower bound.

    if (bound_number == 0)
    {  // If we are calculating all the machines

        if ((*p_p_in_Set_Work_Centers) == NULL)
        {
            (*p_p_in_Set_Work_Centers) = new Set_Work_Centers();
            (*p_p_in_Set_Work_Centers)->Initialize(number_work_centers_cluster);
        }

        p_out_sibling_Set_Work_Centers = new Set_Work_Centers();
        p_out_sibling_Set_Work_Centers->Initialize(number_work_centers_cluster);
        // For recording the new sequence.

        int number_bounds_one =
            (*p_p_in_Set_Work_Centers)->Get_Number_Bounds_One(bound_number, max_number_bounds_one);

        //              for ( i = 0 ; i < number_work_centers_cluster ; i++ ){
        for (i = 0; i < number_bounds_one; i++)
        {
            // Perform the TWO machines lower bound.

            // Finding the best locations.
            (*p_p_in_Set_Work_Centers)
                ->Get_Highest_Locations_One(
                    best_location_one, &previous_lower_bound, &number_unscheduled_operations, i);
            if (!is_effective)
            {
                // There is no effective lower bound yet.

                number_unscheduled_operations = Get_Number_Unscheduled(best_location_one, 1, p_input_Cluster);

                this_center_id = (p_input_Cluster->Get_Cluster_Work_Center(best_location_one[0]))->Get_Id();

                if ((p_Graph->Get_N_Blank_Centers(this_center_id) > 0) &&
                    (number_unscheduled_operations <= par_max_number_unscheduled_one) &&
                    (number_unscheduled_operations > 0))
                {
                    // Only few operations.

                    lower_bound = Get_Bound_N(&time_out, best_location_one, 1, this_max_acceptable,
                        this_max_allowed, p_input_Cluster, p_Graph);

                    if ((lower_bound > this_max_acceptable) && (time_out == FALSE))
                    {
                        // If the lower bound is effective:

                        is_effective = TRUE;

                        p_out_sibling_Set_Work_Centers->Set_N_Effective(best_location_one, 1);
                        // The sibling will get the new sequence,
                        // but knowing best_location_one[0] was effective.

                        //                                              fprintf( par_output_file ,"(%d)]",
                        //                                              best_location_one[0]);

                        p_out_sibling_Set_Work_Centers->Add(best_location_one[0], this_max_acceptable,
                            number_unscheduled_operations, best_location_one[0]);
                    }
                    else if (time_out == FALSE)
                    {
                        // It finds a solution bellow the lower bound.

                        p_out_sibling_Set_Work_Centers->Add(best_location_one[0], lower_bound,
                            number_unscheduled_operations, best_location_one[0]);
                    }
                    else
                    {
                        // It does not have time to find a solution.

                        p_out_sibling_Set_Work_Centers->Add(best_location_one[0], previous_lower_bound,
                            number_unscheduled_operations, best_location_one[0]);
                    }
                }
                else
                {
                    p_out_sibling_Set_Work_Centers->Add(best_location_one[0], previous_lower_bound,
                        number_unscheduled_operations, best_location_one[0]);
                    // previous_lower_bound = -1 if not processed
                }
            }
            else
            {
                p_out_sibling_Set_Work_Centers->Add(best_location_one[0], previous_lower_bound,
                    number_unscheduled_operations, best_location_one[0]);
            }
        }

        // If all the machines were recalculated:
        (*p_p_in_Set_Work_Centers)->Terminate();
        delete (*p_p_in_Set_Work_Centers);
        (*p_p_in_Set_Work_Centers) = p_out_sibling_Set_Work_Centers;
        // There is a new sequence, so the previous one
        // is deleted and replaced by p_out_sibling_Set_Work_Centers.

        if (is_effective)
        {
#ifdef CHECK_DEBUG
            par_scheduling_operation = par_scheduling_operation_backup;
#endif
            return (INT_MAX);
        }
    }
    else if ((*p_p_in_Set_Work_Centers)->N_Machine_Effective(1))
    {
        (*p_p_in_Set_Work_Centers)->Get_N_Effective(best_location_one, 1);

        // We know it has only a few operations.
        lower_bound = Get_Bound_N(
            &time_out, best_location_one, 1, this_max_acceptable, this_max_allowed, p_input_Cluster, p_Graph);

        if ((lower_bound > this_max_acceptable) & (time_out == FALSE))
        {
            // If the lower bound is effective:

            (*p_p_in_Set_Work_Centers)->Set_N_Effective(best_location_one, 1);
            // The sibling will get the same sequence,
            // but knowing best_location_one[0] was effective.

#ifdef CHECK_DEBUG
            //                      fprintf( par_output_file ,"(%d)]", best_location_one[0]);
            par_scheduling_operation = par_scheduling_operation_backup;
#endif
            return (lower_bound);
        }
    }

    int number_bounds_two =
        (*p_p_in_Set_Work_Centers)->Get_Number_Bounds_Two(bound_number, max_number_bounds_two);

    for (i = 0; i < number_bounds_two; i++)
    {
        // Perform the TWO machines lower bound.

        // Finding the best locations.

        (*p_p_in_Set_Work_Centers)->Get_Highest_Locations_Two(best_location_two, &total_value, i);

        number_unscheduled_operations = Get_Number_Unscheduled(best_location_two, 2, p_input_Cluster);

        if ((number_unscheduled_operations <= par_max_number_unscheduled_two) &&
            (number_unscheduled_operations > 0))
        {
            lower_bound = Get_Bound_N(&time_out, best_location_two, 2, this_max_acceptable, this_max_allowed,
                p_input_Cluster, p_Graph);

            if ((lower_bound > this_max_acceptable) & (time_out == FALSE))
            {
                // If the lower bound is effective:

                (*p_p_in_Set_Work_Centers)->Set_N_Effective(best_location_two, 2);
                // The sibling will get the same sequence,
                // but knowing best_location_two was effective.

#ifdef CHECK_DEBUG
                //                              fprintf( par_output_file ,"(%d%d)]", best_location_two[0] ,
                //                              best_location_two[1] );
                par_scheduling_operation = par_scheduling_operation_backup;
#endif
                return (lower_bound);
            }
        }
    }

    int number_bounds_three =
        (*p_p_in_Set_Work_Centers)->Get_Number_Bounds_Three(bound_number, max_number_bounds_three);

    for (i = 0; i < number_bounds_three; i++)
    {
        // Perform the THREE machines lower bound.

        // Finding the best locations.

        (*p_p_in_Set_Work_Centers)->Get_Highest_Locations_Three(best_location_three, &total_value, i);

        number_unscheduled_operations = Get_Number_Unscheduled(best_location_three, 3, p_input_Cluster);

        if ((number_unscheduled_operations <= par_max_number_unscheduled_three) &&
            (number_unscheduled_operations > 0)
            //                      &&
            //                      ( total_value > this_max_acceptable * par_lower_bound * 3 )
        )
        {
            //                      printf( "3" );

            lower_bound = Get_Bound_N(&time_out, best_location_three, 3, this_max_acceptable,
                this_max_allowed, p_input_Cluster, p_Graph);

            if ((lower_bound > this_max_acceptable) & (time_out == FALSE))
            {
                // If the lower bound is effective:

                (*p_p_in_Set_Work_Centers)->Set_N_Effective(best_location_three, 3);
                // The sibling will get the same sequence,
                // but knowing best_location_three was effective.

#ifdef CHECK_DEBUG
                //                              fprintf( par_output_file ,"(%d%d%d)]",  best_location_three[0]
                //                              ,
                //                                                                                      best_location_three[1]
                //                                                                                      ,
                //                                                                                      best_location_three[2]);
                par_scheduling_operation = par_scheduling_operation_backup;
                //                              printf( "+" );
#endif
                return (lower_bound);
                //                      } else {
                //                              printf( "-" );
            }
        }
    }

    int number_bounds_four =
        (*p_p_in_Set_Work_Centers)->Get_Number_Bounds_Four(bound_number, max_number_bounds_four);

    for (i = 0; i < number_bounds_four; i++)
    {
        // Perform the FOUR machines lower bound.

        // Finding the best locations.

        (*p_p_in_Set_Work_Centers)->Get_Highest_Locations_Four(best_location_four, &total_value, i);

        number_unscheduled_operations = Get_Number_Unscheduled(best_location_four, 4, p_input_Cluster);

        if ((number_unscheduled_operations <= par_max_number_unscheduled_four) &&
            (number_unscheduled_operations > 0)
            //                      &&
            //                      ( total_value > this_max_acceptable * par_lower_bound * 4 )
        )
        {
            //                      printf( "4" );

            lower_bound = Get_Bound_N(&time_out, best_location_four, 4, this_max_acceptable, this_max_allowed,
                p_input_Cluster, p_Graph);

            if ((lower_bound > this_max_acceptable) & (time_out == FALSE))
            {
                // If the lower bound is effective:

                (*p_p_in_Set_Work_Centers)->Set_N_Effective(best_location_four, 4);
                // The sibling will get the same sequence,
                // but knowing best_location_four was effective.

#ifdef CHECK_DEBUG
                //                              fprintf( par_output_file ,"(%d%d%d%d)]", best_location_four[0]
                //                              ,
                //                                                                                      best_location_four[1]
                //                                                                                      ,
                //                                                                                      best_location_four[2]
                //                                                                                      ,
                //                                                                                      best_location_four[3]);
                par_scheduling_operation = par_scheduling_operation_backup;
                //                              printf( "+" );
#endif
                return (lower_bound);
                //                      } else {
                //                              printf( "-" );
            }
        }
    }

    int number_bounds_five =
        (*p_p_in_Set_Work_Centers)->Get_Number_Bounds_Five(bound_number, max_number_bounds_five);

    for (i = 0; i < number_bounds_five; i++)
    {
        // Perform the FIVE machines lower bound.

        // Finding the best locations.

        (*p_p_in_Set_Work_Centers)->Get_Highest_Locations_Five(best_location_five, &total_value, i);

        number_unscheduled_operations = Get_Number_Unscheduled(best_location_five, 5, p_input_Cluster);

        if ((number_unscheduled_operations <= par_max_number_unscheduled_five) &&
            (number_unscheduled_operations > 0) && (total_value > this_max_acceptable * par_lower_bound * 5))
        {
            //                      printf( "5" );

            lower_bound = Get_Bound_N(&time_out, best_location_five, 5, this_max_acceptable, this_max_allowed,
                p_input_Cluster, p_Graph);

            if ((lower_bound > this_max_acceptable) & (time_out == FALSE))
            {
                // If the lower bound is effective:

                (*p_p_in_Set_Work_Centers)->Set_N_Effective(best_location_five, 5);
                // The sibling will get the same sequence,
                // but knowing best_location_five was effective.

#ifdef CHECK_DEBUG
                //                              fprintf( par_output_file ,"(%d%d%d%d%d)]",
                //                              best_location_five[0] ,
                //                                                                                      best_location_five[1]
                //                                                                                      ,
                //                                                                                      best_location_five[2]
                //                                                                                      ,
                //                                                                                      best_location_five[3]
                //                                                                                      ,
                //                                                                                      best_location_five[4]);
                par_scheduling_operation = par_scheduling_operation_backup;
                //                              printf( "+" );
#endif
                return (lower_bound);
                //                      } else {
                //                              printf( "-" );
            }
        }
    }

    (*p_p_out_child_Set_Work_Centers) = new Set_Work_Centers();
    (*p_p_out_child_Set_Work_Centers)->Initialize(number_work_centers_cluster);
    (*p_p_out_child_Set_Work_Centers)->Get_Sequence((*p_p_in_Set_Work_Centers));
    // Copies everything EXCEPT that some lower bound has been effective

#ifdef CHECK_DEBUG
    //      fprintf( par_output_file ,"*]" );

    par_scheduling_operation = par_scheduling_operation_backup;
#endif
    return (0);
}

// int par_sequence[6] ;

/*      par_sequence[ 6 - remaining_jobs - 1 ] =  p_solution_increment->location_operation ;
        if (            par_sequence[0] == 0
                &&      par_sequence[1] == 1
//              &&      par_sequence[2] == 4
//              &&      par_sequence[3] == 2
//              &&      par_sequence[4] == 3
//              &&      par_sequence[5] == 5
                &&      6 - remaining_jobs - 1 == 1
        )
                printf("") ;
*/
