#include "StdAfx.h"

#include "optimize.h"

int Optimizer::Get_Cutting_Time(int max_branches_oracle)
{
    // int current_time ;

    //      time_out = FALSE ;

    // DELETE!!!!!
    if (FALSE)
    {
        return (first_time + (int)300);
    }
    else
    {
        return (LONG_MAX);
    }
}

int Optimizer::Time_Out()
{
    int current_time;

    return (FALSE);

#ifdef CHECK_DEBUG
    //                                              assert( time_out == FALSE ) ;
#endif

    // Augusto 12.8.96
    //      time( &current_time ) ;
    //        current_time = (int )(clock()) /CLOCKS_PER_SEC  ;
    current_time = (int)(clock()) / CLK_TCK;
    if (current_time >= cutting_time)
    {
        //              time_out = TRUE ;
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}

// Optimizer::Choose_Parameter_K: Chooses the best parameter K and B for the ATC rule by performing a spot
// check on the subproblems.
double Optimizer::Choose_Parameter_K(Graph* p_Graph, int bottleneck_work_center)
{
    int i;
    static double posible_k[14] = {
        0.00001, 0.001, 0.01, 0.03, 0.06, 0.1, 0.2, 0.3, 0.5, 0.8, 1, 5, 99, 99999};

    Cluster* p_Cluster = p_Set_Clusters->Get_Cluster(bottleneck_work_center);
    // We choose arbitraraly the first cluster.

    Class_Solution_Oracle* p_optimal_solution;
    int best_value = INT_MAX;
    double best_K;

#ifdef CHECK_DEBUG
    int par_scheduling_operation_backup = par_scheduling_operation;
    par_scheduling_operation = FALSE;
    // Backing up.
#endif

    if (par_K != -1)
    {
#ifdef CHECK_DEBUG
        par_scheduling_operation = par_scheduling_operation_backup;
#endif
        return (par_K);
    }
    for (i = 0; i < 14; i++)
    {
        p_optimal_solution = new Class_Solution_Oracle();
        p_Cluster->Initialize_Solution(p_optimal_solution);

        //                       if ( p_Cluster->Get_Number_Work_Centers() == 1 )
        if (p_Cluster->Get_Number_Machines() == 1)
            this_Act_Heur_Orcl.Optimize(
                p_optimal_solution, p_Cluster, p_Graph, 1, posible_k[i], INT_MAX, INT_MIN);
        else
            this_Act_Heur_Orcl.Optimize(
                p_optimal_solution, p_Cluster, p_Graph, 1, posible_k[i], INT_MAX, INT_MIN, NULL);
        // There is no branching when ranking the clusters.

        //              printf("(%1.3f : %d ) " , posible_k[ i ] , p_optimal_solution->Get_Objective() ) ;

        if (p_optimal_solution->Get_Objective() < best_value)
        {
            best_value = p_optimal_solution->Get_Objective();
            best_K = posible_k[i];
        }
        p_optimal_solution->Terminate();
        delete p_optimal_solution;
    }

#ifdef CHECK_DEBUG
    par_scheduling_operation = par_scheduling_operation_backup;
#endif
    //      printf("best K : %1.3f \n" , best_K ) ;
    p_Graph->Set_Obsolete_Forward();
    return (best_K);
}

// Rank_Clusters(): It decides which cluster to schedule next.
// The cluster with the highest ojective function is the most bottleneck one,
// and therefore should be schedule next.
// Cluster is any set of operations to be scheduled.
// In this case, the set corresponds to the operations that are assigned to a specific work center.
void Optimizer::Rank_Clusters(
    // Output parameters.
    int* p_number_schedulable,
    // Number of clusters that qualify to be scheduled.

    Beam_Indices<Class_Solution_Oracle>* p_list_solutions,
    // List with the solutions for each cluster.

    // Static input paramenters.
    Graph* p_Graph,

    int this_aperture)
{
    double temp_K;
    int delta;
    Class_Solution_Oracle* p_optimal_solution;
    Cluster* p_Cluster;  // Pointer to cluster being reoptimized.
    int dead_lock = FALSE;

#ifdef CHECK_DEBUG
    int possible_schedulable = 0;
#endif

#ifdef PRINT_DEBUG
    if (par_print == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Graph->Get_Backward_Windows(TRUE);
        p_Set_Clusters->Print_Unscheduled(p_Graph->Get_Number_Jobs(), p_Graph);
    }
#endif

#ifdef CHECK_DEBUG
    int par_scheduling_operation_backup = par_scheduling_operation;
    par_scheduling_operation = FALSE;
    // Backing up.
#endif

    if (global_K == -1)
        temp_K = 0.1;
    else
        temp_K = global_K;

        // Augusto 2.10.96
#ifdef CHECK_DEBUG
    par_scheduling_operation = par_scheduling_operation_backup;
#endif
    // hasta aqui

#ifdef CHECK_DEBUG
    assert(p_Graph->Is_Updated());
#endif
    int initial_objective = p_Graph->Get_Objective();

    for (int i = 0; i < p_Set_Clusters->number_clusters; i++)
    {
        // Calculates the indices of the remaining jobs.

        p_Cluster = p_Set_Clusters->Get_Cluster(i);
        // Returns the pointer to the ith cluster.

        if (p_Cluster->is_scheduled == FALSE)
        {
            p_optimal_solution = new Class_Solution_Oracle();
            // Class_Solution_Oracle es una solucion producida por un oracle.
            // El oracle es el objeto que encuentra una solucion para un cluster.
            // Cada oracle corrsponde a un metodo.

            p_Cluster->Initialize_Solution(p_optimal_solution);
            if (par_max_branches_main > 1)
            {
                // Apertura del arbol principal.

                //                                if ( p_Cluster->Get_Number_Work_Centers() == 1 )
                if (p_Cluster->Get_Number_Machines() == 1)
                    this_Act_Heur_Orcl.Optimize(
                        p_optimal_solution, p_Cluster, p_Graph, this_aperture, temp_K, INT_MAX, INT_MIN);
                // Resolver independientemente.
                // this_aperture is the aperture of the oracle tree.
                else
                    this_Act_Heur_Orcl.Optimize(
                        p_optimal_solution, p_Cluster, p_Graph, 1, temp_K, INT_MAX, INT_MIN, NULL);
                // Solve in the graph.
                // The tree as aperture 1.
            }
            else
            {
                // There is an actual tree for looking the sequence os scheduling machines.
                //                                if ( p_Cluster->Get_Number_Work_Centers() == 1 )
                if (p_Cluster->Get_Number_Machines() == 1)
                    this_Act_Heur_Orcl.Optimize(p_optimal_solution, p_Cluster, p_Graph,
                        min2(this_aperture, par_max_branches_oracle),
                        //                                                                        max2(
                        //                                                                        this_aperture
                        //                                                                        ,
                        //                                                                        par_max_branches_oracle
                        //                                                                        ) ,
                        temp_K, INT_MAX, INT_MIN);
                // This is independent.
                else
                    this_Act_Heur_Orcl.Optimize(p_optimal_solution, p_Cluster, p_Graph,
                        min2(this_aperture, par_max_branches_oracle), temp_K, INT_MAX, INT_MIN, NULL);
                // This is on tyhe graph.
            }
            // WARNING!!!!!!
            delta = pos(p_optimal_solution->Get_Objective() - initial_objective);
            // How much does the objective value grow because we schedule the center.

            if (p_list_solutions->Worth_Adding(double(delta)))
            {
                // Checks whether it is convenient to add the center to the list.

                p_list_solutions->Add_Element(double(delta), p_optimal_solution, p_Cluster->Get_Id());

#ifdef CHECK_DEBUG
                possible_schedulable++;
#endif

#ifdef PRINT_DEBUG
                if (par_print == 1)
                {
                    fprintf(par_output_file, "adding: delta = %d \n", delta);
                    p_optimal_solution->Print();
                }
#endif
            }
            else
            {
#ifdef PRINT_DEBUG
                if (par_print == 1)
                {
                    fprintf(par_output_file, "not adding: delta = %d \n", delta);
                    p_optimal_solution->Print();
                }
#endif

                p_optimal_solution->Terminate();
                delete p_optimal_solution;
            }
        }
    }
    p_list_solutions->Get_Number_Schedulable(p_number_schedulable);

#ifdef CHECK_DEBUG
    assert((*p_number_schedulable != 0) && (*p_number_schedulable <= possible_schedulable));
#endif
#ifdef CHECK_DEBUG
    par_scheduling_operation = par_scheduling_operation_backup;
#endif
}

void Optimizer::Re_Rank_Clusters(
    // Output parameters.
    int* p_number_schedulable,
    // Number of clusters that qualify to be scheduled.

    Beam_Indices<Class_Solution_Oracle>* p_list_solutions,
    // List with the solutions for each cluster.

    // Static input paramenters.
    Graph* p_Graph)
{
    p_list_solutions->Terminate();
    p_list_solutions->Initialize(
        p_Set_Clusters->Number_Clusters(), par_max_branches_main, par_threshold_main);

    int* backup = p_Graph->Backup_Change_Due_Dates();
    // Alocates memory with the original due dates, and makes them tighter.

    int dead_lock = FALSE;
    p_Graph->Set_Obsolete_Forward();
    p_Graph->Get_Forward_Windows(&dead_lock);
    p_Graph->Get_Backward_Windows(TRUE);
    p_Graph->Get_Objective();

    Rank_Clusters(p_number_schedulable, p_list_solutions, p_Graph, 1);

    p_Graph->Restore_Due_Dates(backup);
}

int Optimizer::Single_Lower_Bound(Graph* p_Graph)
{
    Class_Solution_Oracle *p_optimal_solution, *p_best_solution_oracle;
    Beam_Indices<Class_Solution_Oracle> list_solutions;
    Cluster* p_Cluster;  // Pointer to cluster being reoptimized.
    int dead_lock = FALSE;
    double single_lower_bound;
    int next_cluster_id;

    list_solutions.Initialize(
        p_Set_Clusters->Number_Clusters(), p_Set_Clusters->Number_Clusters(), par_threshold_main);

    p_Graph->Get_Forward_Windows(&dead_lock);
    p_Graph->Get_Backward_Windows(TRUE);
    p_Graph->Get_Objective();

#ifdef CHECK_DEBUG
    assert(p_Graph->Is_Updated());
#endif
    for (int i = 0; i < p_Set_Clusters->number_clusters; i++)
    {
        // Calculates the indices of the remaining jobs.

        p_Cluster = p_Set_Clusters->Get_Cluster(i);
        // Returns the pointer to the ith cluster.

        p_optimal_solution = new Class_Solution_Oracle();
        p_Cluster->Initialize_Solution(p_optimal_solution);

        double backup_global_K = global_K;
        global_K = 0.1;
        this_Disj_Exact_Orcl.Optimize(p_optimal_solution, p_Cluster, p_Graph, par_upper_bound + 1, INT_MIN);
        global_K = backup_global_K;

        list_solutions.Add_Element(
            double(p_optimal_solution->Get_Objective()), p_optimal_solution, p_Cluster->Get_Id());
    }
    list_solutions.Next_Best(&single_lower_bound, &next_cluster_id, &p_best_solution_oracle);
    return ((int)single_lower_bound);
}

int par_size_shuffle = 1;

void Optimizer::Reoptimize_Plus(Graph* p_Graph, int* p_current_solution)
{
    int i;
    int change = FALSE;  //= TRUE if there is a modification in the graph.
    Cluster this_Cluster;
    //(id , Number of work centers).

#ifdef PRINT_DEBUG
    if (par_print == 1)
    {
        fprintf(par_output_file, "                       STARTING REOPTIMIZATION PLUS\n ");
    }
#endif

    int number_reoptimizable = min2(par_reoptimization, p_Set_Clusters->Number_Scheduled());
    int number_shuffles = number_reoptimizable / par_size_shuffle;

    for (i = 0; i < number_shuffles - 1; i++)
    {
        // For each already scheduled cluster.

        p_Set_Clusters->Create_Cluster(
            &this_Cluster, p_Set_Clusters->Get_Sequence(), i * par_size_shuffle, par_size_shuffle, p_Graph);

        if (p_Local->Reoptimize(p_Graph, &this_Cluster, &this_Act_Heur_Orcl, &this_Disj_Heur_Orcl,
                p_current_solution) == TRUE)
        {
            change = TRUE;
        }
        this_Cluster.Terminate();
    }

    if ((change == TRUE) || ((number_shuffles == 1) && (par_size_shuffle > 1)))
    {
        // Reoptimize the last one only if there has been any change.

        //              p_Cluster = p_Set_Clusters->Nth_Scheduled_Cluster( number_reoptimizable - 1 ) ;
        p_Set_Clusters->Create_Cluster(&this_Cluster, p_Set_Clusters->Get_Sequence(),
            (number_shuffles - 1) * par_size_shuffle, par_size_shuffle, p_Graph);

        p_Local->Reoptimize(
            p_Graph, &this_Cluster, &this_Act_Heur_Orcl, &this_Disj_Heur_Orcl, p_current_solution);
        this_Cluster.Terminate();
    }
#ifdef PRINT_DEBUG
    if (par_print == 1)
    {
        fprintf(par_output_file, "                       FINISHING REOPTIMIZATION PLUS\n\n ");
    }
#endif
}

void Optimizer::Reoptimize_All(Graph* p_Graph, int* p_current_solution)
{
    int change = FALSE;  //= TRUE if there is a modification in the graph.
    Cluster this_Cluster;
    //(id , Number of work centers).

#ifdef PRINT_DEBUG
    if (par_print == 1)
    {
        fprintf(par_output_file, "                       STARTING REOPTIMIZATION PLUS\n ");
    }
#endif

    p_Set_Clusters->Create_Cluster(
        &this_Cluster, p_Set_Clusters->Get_Sequence(), 0, p_Set_Clusters->Number_Scheduled(), p_Graph);
    p_Local->Reoptimize(
        p_Graph, &this_Cluster, &this_Act_Heur_Orcl, &this_Disj_Heur_Orcl, p_current_solution);
    this_Cluster.Terminate();
}

// Recursive call for the main recursion, the one that selects the order on which the differetn machines are
// scheduled. Main stages: i) improve the solution of the cluster that is going to be scheduled. ii) schedules
// the cluster with such better solution. iii) reoptimizas the clusters that have been already scheduled. iv)
// ranks the clusters that have not been scheduled.
void Optimizer::Recursive_Call(
    // Output parameters.
    Solution_Graph* p_Best_Solution_Graph,
    // Recursive input paramenters.
    Graph* p_Graph,
    int chosen_cluster_id)

{
    int i;
    int number_branches;
    int number_schedulable;
    double index;  // Dummy variable.

    int next_cluster_id;  // For Mark and Unmark.

    Class_Solution_Oracle* p_next_solution_oracle;
    // We don't ask for memory for it.
    // Argument for the next recursive call.

    Beam_Indices<Class_Solution_Oracle> list_solutions;

    int dead_lock = FALSE;
    // int oracle_upper_bound ;
    int current_value;

    Cluster* p_Cluster = p_Set_Clusters->Get_Cluster(chosen_cluster_id);
    // Pointer to the cluster that is being assigned.

    // Do the assigment.

    Class_Solution_Oracle better_solution_oracle;
    // Contains an improved solution for the cliuster to be scheduled next.
    p_Cluster->Initialize_Solution(&better_solution_oracle);
    // Allocates memory for such solution.

    int this_max_acceptable;
    if (par_reoptimization > 0)
        this_max_acceptable = INT_MAX;
    else
        this_max_acceptable = p_Best_Solution_Graph->Get_Objective();

    if ((par_option == HEURISTIC) || (par_option == LOCAL_SEARCH))
    {
        //                if ( p_Cluster->Get_Number_Work_Centers() == 1 ) {
        if (p_Cluster->Get_Number_Machines() == 1)
        {
            p_Graph->Get_Forward_Windows(&dead_lock);
            p_Graph->Get_Backward_Windows(TRUE);
            p_Graph->Get_Objective();
            this_Act_Heur_Orcl.Optimize(&better_solution_oracle, p_Cluster, p_Graph, par_max_branches_oracle,
                global_K, this_max_acceptable, INT_MIN);
            // Obtains a better solution.
        }
        else
        {
            this_Act_Heur_Orcl.Optimize(&better_solution_oracle, p_Cluster, p_Graph, par_max_branches_oracle,
                global_K, this_max_acceptable, INT_MIN, NULL);
            // Obtains a better solution.
            // The aperture of the tree is par_max_branches_oracle.
        }
        current_value = better_solution_oracle.Get_Objective();
    }
    else if (par_option == LOWER_BOUND)
    {
        if (par_transition_speed > p_Graph->Get_Number_Blanks() - 2)
            // Active branching.
            this_Act_Exact_Orcl.Optimize(&better_solution_oracle, p_Cluster, p_Graph, INT_MAX, global_K,
                par_upper_bound, INT_MIN, NULL);
        else
            this_Disj_Exact_Orcl.Optimize(
                &better_solution_oracle, p_Cluster, p_Graph, par_upper_bound, INT_MIN);
        current_value = better_solution_oracle.Get_Objective();
    }
    if (current_value < INT_MAX)
    {
        // If there is an actual solution:

        p_Cluster->Schedule_Solution(p_Graph, &better_solution_oracle);
        // Once the solution was founded, we schedule it.

        p_Set_Clusters->Mark_Scheduled(chosen_cluster_id);
        // The cluster is marked as scheduled, and is written
        // in the place of the sequence of scheduled clusters.

#ifdef ANIMATION_DEBUG
        if (par_scheduling_machine == 1)
        {
            p_Graph->Get_Forward_Windows(&dead_lock);
            // Augusto 14.8.96
            // Luis Peña M.                                 p_Plant->Show_Gantt_Chart( "../DEC/gantt" ,
            // p_Graph );
            p_Plant->Show_Gantt_Chart("gantt", p_Graph);

            printf("UNSCHEDULE new machine\n");
            char in[1];
            gets(in);
        }
#endif

        // Reoptimize.
        if ((par_reoptimization > 0) && (p_Set_Clusters->Number_Scheduled() > 1))
        {
            Reoptimize_Plus(p_Graph, &current_value);
        }
        // Start recursion.
        // WARNING!!!!!!!!!!
        //                p_Graph->Set_Obsolete_Forward();
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Graph->Get_Backward_Windows(TRUE);
        p_Graph->Get_Objective();

        printf("* ");
        assert(dead_lock != TRUE);

#ifdef CHECK_DEBUG
        assert(p_Graph->Get_Objective() == current_value);
#endif
        if (current_value < p_Best_Solution_Graph->Get_Objective())
        {
            // If the current solution stills competitive.

            if (p_Set_Clusters->Number_Remaining() == 0)
            {
                // If this is the last cluster to be scheduled.
                // Notice that we know it is the best solution so far,
                // since the IF above.

                //                              p_Graph->Get_Forward_Windows( &dead_lock );
#ifdef CHECK_DEBUG
                assert(p_Graph->Is_Updated());
                assert(dead_lock == FALSE);
#endif

                p_Graph->Set_Objective(current_value);

                p_Graph->Write_Solution(p_Best_Solution_Graph);
                // Deletes, Prepares and Copies the output.

                p_Set_Clusters->Write_Optimal_Sequence();
                if (p_Set_Clusters->Number_Clusters() != 1)
                {
                    int solution_time;
                    // Augusto 12.8.96
                    //                                       time( &solution_time ) ;
                    //                                       solution_time = (int )(clock()) /CLOCKS_PER_SEC ;
                    solution_time = (int)(clock()) / CLK_TCK;
                    printf("\n(%d,%d)\n", p_Graph->Get_Objective(), solution_time - first_time);

                    //                                       p_Set_Clusters->Print_Sequence() ;

                    hitting_time = solution_time - first_time;
                    // first_time is a global variable.
                }

                //                              if ( p_Graph->Get_Objective() == par_upper_bound ) {
                //                                      p_Graph->Set_Objective( 0 ) ;
                //                              }
#ifdef FLOW_DEBUG
                fprintf(par_output_file, "optimal solution MAIN\n ");
#endif
            }
            else
            {  // Is not the last job, keep on branching.

                // This part prepares the branching.

                list_solutions.Initialize(
                    p_Set_Clusters->Number_Remaining(), par_max_branches_main, par_threshold_main);

                Rank_Clusters(&number_schedulable, &list_solutions, p_Graph, 1);
                // Ranks the work centers using the ATC rule.

                number_branches = min2(par_max_branches_main, number_schedulable);
                // IMPROVE this part.
                // par_max_branches_main is a global variable.
                i = 0;
                while ((i < number_branches) && !Time_Out() &&
                       (highest_machine_bound < p_Best_Solution_Graph->Get_Objective()))
                {
                    // Branch as many times as needed.

                    i++;
                    list_solutions.Next_Best(&index, &next_cluster_id, &p_next_solution_oracle);
                    // This function marks the element as used, so it can be used again.

#ifdef CHECK_DEBUG
                    assert(next_cluster_id == p_next_solution_oracle->Get_Cluster_Id());
#endif

                    Recursive_Call(p_Best_Solution_Graph, p_Graph, next_cluster_id);
                }
                list_solutions.Terminate();
            }  // End branching part.
        }      // If the solution is not competitive, we quit.

        // Undo the assignment.

        p_Cluster->Unschedule_Solution(p_Graph);
        p_Cluster->Delete_Graph_Matrix(p_Graph);
        // There is no back up and restore because the reoptimization
        // may change the graph.

        p_Graph->Set_Obsolete_Forward();

        p_Set_Clusters->Unmark_Scheduled(chosen_cluster_id);

#ifdef ANIMATION_DEBUG
        if (par_scheduling_machine == 1)
        {
            p_Graph->Get_Forward_Windows(&dead_lock);
            // Augusto 14.8.96
            // Luis Peña M.                                 p_Plant->Show_Gantt_Chart( "../DEC/gantt" ,
            // p_Graph );
            p_Plant->Show_Gantt_Chart("gantt", p_Graph);

            printf("UNSCHEDULE new machine\n");
            char in[1];
            gets(in);
        }
#endif
    }

    better_solution_oracle.Terminate();
}

void Optimizer::Initialize(
    // Input parameters:
    Plant* p_input_Plant,
    Set_Clusters* p_input_Set_Clusters)
{
    // External data structures:

    p_Plant = p_input_Plant;
    p_Set_Clusters = p_input_Set_Clusters;

    // Recall there is a Class_List_Disjunctives list_new_disjunctives ;

    this_Act_Exact_Orcl.Initialize(EXACT_ORACLE, p_input_Plant, p_input_Set_Clusters);
    this_Act_Heur_Orcl.Initialize(HEURISTIC_ORACLE, p_input_Plant, p_input_Set_Clusters);
    this_Disj_Exact_Orcl.Initialize(EXACT_ORACLE, p_input_Plant, p_input_Set_Clusters);
    this_Disj_Heur_Orcl.Initialize(HEURISTIC_ORACLE, p_input_Plant, p_input_Set_Clusters);
    // Optimizer of each cluster.
    // It does NOT allocate memory.
}

// Optimize(): generates a solution for the entire job shop.
void Optimizer::Optimize(
    // Output parameter:
    Solution_Graph* p_Best_Solution_Graph,
    // Input parameters:
    Graph* p_Graph,
    Local* p_input_Local)
{
    int i;
    int next_cluster_id;
    int number_schedulable = 0;
    int number_branches;
    double index;  // Dummy variable.
    int dead_lock = FALSE;

    Class_Solution_Oracle* p_next_solution_oracle;
    // Argument for the next recursive call.

    Beam_Indices<class Class_Solution_Oracle> main_list_solutions;
    // Keeps track of the indices for the main.

    // From here the actual program.
    p_Local = p_input_Local;

    cutting_time = Get_Cutting_Time(0);
    // Calculates the time at which the backtracking will be cutted.

    highest_machine_bound = INT_MIN;  // Highest one machine lower bound.
                                      // If equal to the optimal solution, then we quit.

    if (p_Set_Clusters->Number_Clusters() > 1)
    {
        main_list_solutions.Initialize(
            p_Set_Clusters->Number_Clusters(), par_max_branches_main, par_threshold_main);
        // Updates the graph.
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Graph->Get_Backward_Windows(TRUE);
        p_Graph->Get_Objective();

        Rank_Clusters(&number_schedulable, &main_list_solutions, p_Graph, 1);
        // Solves each subproblem,
        // ands ranks them according to their objective function.
        // 2 is the aperture.

        highest_machine_bound = (int)((main_list_solutions.Get_p_ith_Element(0))->Get_Objective());

        if ((par_max_branches_main == 1) &&
            //                      ( (main_list_solutions.Get_p_ith_Element(0))->Get_Objective() == 0 )
            (highest_machine_bound == 0))
            Re_Rank_Clusters(&number_schedulable, &main_list_solutions, p_Graph);
        // If all the deltas (difference beteen the original objective function and theç
        // value due to the scheduling of the center) are zero,
        // then we make the due date tighter (mas apretadas),
        // and re rank in order toi find the bottleneck center.

        number_branches = min2(par_max_branches_main, number_schedulable);
        i = 0;
        while ((i < number_branches) && !Time_Out() &&
               (highest_machine_bound < p_Best_Solution_Graph->Get_Objective()))
        {
            // Schedule each work center.

            i++;
            main_list_solutions.Next_Best(&index, &next_cluster_id, &p_next_solution_oracle);
            // This function marks the work center as used, so it can be used again.

            p_Graph->Get_Forward_Windows(&dead_lock);
            p_Graph->Get_Backward_Windows(TRUE);
            p_Graph->Get_Objective();

            global_K = Choose_Parameter_K(p_Graph, next_cluster_id);
            // Chooses the best parameter K for the ATC rule by performing a spot check
            // on the subproblems.
            // main_list_solutions.Highest_Id( ) return the most bottleneck cluster.

            Recursive_Call(p_Best_Solution_Graph, p_Graph, next_cluster_id);
            // The recursive call recieves the name of the cluster to be scheduled.
            // It has not been scheduled yet.
        }
        main_list_solutions.Terminate();
    }
    else
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Graph->Get_Backward_Windows(TRUE);
        p_Graph->Get_Objective();

        if (par_option != LOWER_BOUND || par_selection_rule != BOTTLE)
            global_K = Choose_Parameter_K(p_Graph, 0);
        // Chooses the best parameter K for the ATC rule by performing a spot check
        // on the subproblems.
        // main_list_solutions.Highest_Id( ) return the most bottleneck cluster.

        Recursive_Call(p_Best_Solution_Graph, p_Graph, 0);
    }
}
