#include "StdAfx.h"

#include "input_file.h"
#include "optimize.h"
#include "paramet.h"
#include "sol_grph.h"

void Print_Final_Result(
    Solution_Graph* p_Solution_Graph, Graph* p_empty_Graph, Plant* p_Plant, Set_Clusters* p_Set_Clusters)
{
    int dead_lock = FALSE;
    int objective;

    if (p_Solution_Graph->Get_Objective() == INT_MAX)
    {
        objective = -1;
    }
    else
    {
        p_empty_Graph->Read_Machines(p_Solution_Graph);
        p_empty_Graph->Read_Arcs(p_Solution_Graph);
        // The empty graph is not empty any more.

        p_empty_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
        assert(dead_lock == FALSE);
#endif
        objective = p_empty_Graph->Get_Objective();
        // Agregado por Luis Peña '97

        // Sistema con control de operaciones
        /*
        #ifdef V_UNIX
                        char *file=concat1("INPUT/gantt/ganttluis",contador);
        #endif
        #ifndef V_UNIX
                        char *file=concat1("INPUT\\gantt\\ganttluis",contador);
        #endif
        */

        // Sistema con Ventanas
        /*
                        char *s[]={"0","1","2","3","4","5","6","7","8","9"};
                        char *file,*name="Input\\gantt\\gantt";
                        file = (char *) malloc (_tcslen(name)+_tcslen(s[contador]));
                        sprintf(file,"%s%s",name,s[contador]);
        */

        // sistema iterativo
        char* file = "gantt";
        p_Plant->Show_Gantt_Chart(file, p_empty_Graph);
        // Writes the input for CUISE.

        contador++;
    }
    switch (par_objective)
    {
        case MAKESPAN:
            // fprintf( par_output_file ,"MAKESPAN = %d ", objective );
            printf("\nMAKESPAN=%d", objective);
            break;

        case TARDINESS:
            // fprintf( par_output_file ,"TOTAL WEIGHTED TARDINESS = %d ", objective );
            printf("\nTWT=%d", objective);
            break;

        case COMPLETION:
            // fprintf( par_output_file ,"TOTAL WEIGHTED COMPLETION = %d ", objective );
            printf("\nTW COMPLETION=%d", objective);
            break;
    }
    if (objective == par_upper_bound)
        printf("* ");
    else
        printf(" ");

    // SOLEX int solution_time ;
    int solution_time;
    // Augusto 12.8.96
    //        time( &solution_time ) ;
    //                solution_time = (int )(clock()) /CLOCKS_PER_SEC  ;
    solution_time = (int)(clock()) / CLK_TCK;

    printf("\nTIME ( %d )  %d\n", hitting_time, solution_time - first_time);

    //      if ( p_Set_Clusters->Number_Clusters() != 1 )
    //              p_Set_Clusters->FPrint_Optimal_Sequence() ;
    // fclose( par_output_file );
}

int solve_instance(Graph** p_p_optimal_Graph,
    // It will store the optimal solution so far.

    int print_solution,
    Graph* p_partial_Graph,
    Plant* p_Plant,
    Set_Clusters* p_Set_Clusters)
{
    Solution_Graph this_Solution_Graph;  // Stores the optimal set of disjunctive arcs.
    Optimizer main_Optimizer;
    Local main_Local;

    // Augusto 12.8.96
    //        time( &first_time ) ;
    //        first_time = (int )(clock()) /CLOCKS_PER_SEC  ;
    first_time = (int)(clock()) / CLK_TCK;

    p_Set_Clusters->Initialize(p_partial_Graph, p_Plant);
    p_partial_Graph->Initialize_Solution(&this_Solution_Graph);
    main_Local.Initialize(p_Plant, p_Set_Clusters);
    main_Optimizer.Initialize(p_Plant, p_Set_Clusters);
    // It does NOT allocate memory.

    main_Optimizer.Optimize(&this_Solution_Graph, p_partial_Graph, &main_Local);
    int result = this_Solution_Graph.Get_Objective();
    if (par_option == HEURISTIC || par_option == LOCAL_SEARCH)
    {
        //              p_Set_Clusters->FPrint_Optimal_Sequence() ;
        p_Set_Clusters->Copy_Optimal_Sequence();
        // Copy to the global variable par_optimal_sequence[].

        //              int given_sequence[8] = {1,0,6,2,7,4,5,3} ;
        //              p_Set_Clusters->Copy_Given_Sequence( given_sequence ) ;
    }
    //      int solution_time ;
    //      time( &solution_time ) ;
    //      printf("%d", solution_time - first_time ) ;
    if (print_solution)
    {
        *p_p_optimal_Graph = new Graph;
        (**p_p_optimal_Graph) = *p_partial_Graph;
        // Allocates memory for **p_p_optimal_Graph and
        // copies only the static info and the release dates.

        Print_Final_Result(&this_Solution_Graph, *p_p_optimal_Graph, p_Plant, p_Set_Clusters);
        // Writes the optimal solution in the graph.
    }

    this_Solution_Graph.Terminate();
    p_Set_Clusters->Terminate();

    return (result);
}

void dec_input(FILE* statistics_file,
    Graph** p_p_optimal_Graph,  // It will store the optimal solution so far.
    Plant** p_p_main_Plant,     // Stores the data of work centers and clusters.
    int version)
{
    Graph partial_Graph;             // This graph will be changing constantlly.
    Set_Clusters this_Set_Clusters;  // Clusters of operations.
                                     // Keeps the sequence in which the clusters have been scheduled.

    Parameters main_Parameters;
    Optimizer this_Optimizer;

    int par_option_backup;  // These for option LOWER_BOUND, 5 and 6.

    int result_1, result_2, result_3, result_4;
    // These for option 5 and 6.

    FILE* all_param_output_file;  // These for option 7.

    int best_all;
    int new_result;
    int i, j, k;

    *p_p_main_Plant = new Plant;

    // BEGIN:

    main_Parameters.Read_Other();

    main_Parameters.Read_Static_Data(&partial_Graph, *p_p_main_Plant, version);
    // Allocates memory for both partial_Graph and for **p_p_main_Plant.
    partial_Graph.Initialize();

    // par_output_file = open_file( par_name_output_file, "w" ) ;

    switch (par_option)
    {
        case HEURISTIC:     // Optimization
        case LOCAL_SEARCH:  //

            main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            // Allocates memory only for this_Set_Clusters.

            printf("|%d,", par_max_branches_main);
            if (par_threshold_main > 0) printf("%1.3f,", par_threshold_main);
            if (par_threshold_oracle == 0)
                printf("%d,%d", par_reoptimization, par_max_branches_oracle);
            else
                printf("%d,%d,%1.3f", par_reoptimization, par_max_branches_oracle, par_threshold_oracle);
            if (par_slack_active > 0)
                printf(",%d|", par_slack_active);
            else
                printf("|");

            solve_instance(p_p_optimal_Graph, TRUE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            // Dealocates this_Set_Clusters.

            delete[] par_optimal_sequence;

            //              if ( par_option == LOCAL_SEARCH ) {
            //                      main_Local.Shuffle( &this_Solution_Graph , &partial_Graph ) ;
            //                      main_Local.Run( &this_Solution_Graph , *p_p_optimal_Graph ,
            //                      *p_p_main_Plant ) ;
            //              }

            break;

        case LOWER_BOUND:  // Lower Bound

            main_Parameters.Read_Lower_Bound_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            printf("|%d,%d|", par_transition_speed, par_selection_rule);
            assert(par_upper_bound >= 0);
            this_Set_Clusters.Terminate();

            if ((par_selection_rule == BOTTLE) ||
                ((par_transition_speed < 1000) && (par_transition_speed > 0)))
            {
                //                      if ( ( par_flag_flow_shop == TRUE ) && ( par_transition_speed == 1000
                //                      ) )
                //                              (*p_p_main_Plant)->Copy_Flow_Shop_Sequence() ;
                //                      else {
                par_option_backup = par_option;
                par_option = HEURISTIC;
                main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);

                par_max_branches_main = 1;
                par_reoptimization = INT_MAX;
                par_max_branches_oracle = INT_MAX;

                solve_instance(p_p_optimal_Graph, FALSE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);

                par_option = par_option_backup;
                //                      }
            }
            main_Parameters.Read_Lower_Bound_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            //              if ( par_flag_flow_shop == TRUE )
            //                      par_selection_rule = BOTTLE ;

            solve_instance(p_p_optimal_Graph, TRUE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            delete[] par_optimal_sequence;
            break;

        case 5:  // Solving Job Shop.
        case 6:  // Solving Flow Shop.

            par_option_backup = par_option;
            // SOLEX int start_time , finish_time ;
            int start_time, finish_time;

            par_option = HEURISTIC;

            fprintf(statistics_file, "%s  ", par_name_file_static_data);
            main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            par_max_branches_main = 2;
            par_reoptimization = INT_MAX;
            par_max_branches_oracle = 2;

            // Augusto 12.8.96
            //             time( &start_time ) ;
            //               start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            start_time = (int)(clock()) / CLK_TCK;
            result_1 =
                solve_instance(p_p_optimal_Graph, FALSE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            delete[] par_optimal_sequence;

            // Augusto 12.8.96
            //              time( &finish_time ) ;
            //                                       finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            finish_time = (int)(clock()) / CLK_TCK;
            fprintf(statistics_file, "%d  %d              ", result_1, finish_time - start_time);

            main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);

            par_max_branches_main = 2;
            par_reoptimization = INT_MAX;
            par_max_branches_oracle = 3;

            // Augusto 12.8.96
            //              time( &start_time ) ;
            //                start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            start_time = (int)(clock()) / CLK_TCK;
            result_2 =
                solve_instance(p_p_optimal_Graph, FALSE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            delete[] par_optimal_sequence;
            // Augusto 12.8.96
            //              time( &finish_time ) ;
            //                finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            finish_time = (int)(clock()) / CLK_TCK;
            fprintf(statistics_file, "%d  %d              ", result_2, finish_time - start_time);

            main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            /*//DELETE!!!!!!!!!*/
            par_max_branches_main = 3;
            par_reoptimization = INT_MAX;
            par_max_branches_oracle = 2;

            // Augusto 12.8.96
            //              time( &start_time ) ;
            //                                       start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            start_time = (int)(clock()) / CLK_TCK;
            result_3 =
                solve_instance(p_p_optimal_Graph, FALSE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            delete[] par_optimal_sequence;
            // Augusto 12.8.96
            //              time( &finish_time ) ;
            //                finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            finish_time = (int)(clock()) / CLK_TCK;
            fprintf(statistics_file, "%d  %d              ", result_3, finish_time - start_time);

            main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);

            par_max_branches_main = 1;
            par_reoptimization = INT_MAX;
            par_max_branches_oracle = INT_MAX;

            // Augusto 12.8.96
            //              time( &start_time ) ;
            //                                       start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            start_time = (int)(clock()) / CLK_TCK;
            result_4 =
                solve_instance(p_p_optimal_Graph, TRUE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            delete[] par_optimal_sequence;

            if (min2(result_1, result_2, result_3, result_4) == 0)
            {
                fprintf(statistics_file, "0           0       0       0       0               0\n");
            }
            else
            {
                (*p_p_optimal_Graph)->Terminate();
                delete (*p_p_optimal_Graph);

                par_upper_bound = min2(result_1, result_2, result_3, result_4) - 1;

                // This part is to find the "magic sequence":
                par_option = LOWER_BOUND;

                // B&B part:
                main_Parameters.Read_Lower_Bound_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
                // Allocates memory only for this_Set_Clusters.

                par_transition_speed = 0;
                par_selection_rule = 2;

                // Augusto 12.8.96
                //                      time( &finish_time ) ;
                //                        finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                finish_time = (int)(clock()) / CLK_TCK;
                int result_BB = solve_instance(
                    p_p_optimal_Graph, TRUE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);

                if (result_BB == 0)
                {
                    fprintf(statistics_file, "0           0       0       0       0               0\n");
                    printf("value: 0         ");
                }
                else
                {
                    //                              (*p_p_optimal_Graph)->Terminate() ;
                    //                              delete (*p_p_optimal_Graph) ;

                    if (result_BB == INT_MAX)
                    {
                        // Augusto 12.8.96
                        //                                      time( &finish_time ) ;
                        //                                                                                                       finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                        finish_time = (int)(clock()) / CLK_TCK;
                        fprintf(statistics_file, "%d\n", min2(result_1, result_2, result_3, result_4));
                        printf("value: %d        ", min2(result_1, result_2, result_3, result_4));
                    }
                    else
                    {
                        printf("value: %d        ", result_BB);
                        // DELETE!!!!!!!
                        fprintf(statistics_file, "%d\n", result_BB);

                        /*                                      int print_result_BB = result_BB ;
                                                                                                                                 par_upper_bound = result_BB - 1 ;

                                                                                                                                 par_option = HEURISTIC ;

                                                                                                                                 main_Parameters.Read_Heuristic_Data( &partial_Graph , *p_p_main_Plant , &this_Set_Clusters );
                                                                                                                                 par_max_branches_main = 1 ;
                                                                                                                                 par_reoptimization = INT_MAX ;
                                                                                                                                 par_max_branches_oracle = INT_MAX ;
                        // Augusto 12.8.96
                        //                                      time( &start_time ) ;
                                                                                                                                 start_time = (int )(clock()) /CLOCKS_PER_SEC  ;

                                                                                                                                 solve_instance( p_p_optimal_Graph , FALSE ,
                                                                                                                                                                                                 &partial_Graph , *p_p_main_Plant , &this_Set_Clusters ) ;
                                                                                                                                                        //Obtain again the magic sequence, only for timing purposes.

                                                                                                                                 delete[] par_optimal_sequence ;
                                                                                                                                 par_option = LOWER_BOUND ;

                                                                                                                                 main_Parameters.Read_Lower_Bound_Data( &partial_Graph , *p_p_main_Plant , &this_Set_Clusters );
                                                                                                                                 par_transition_speed = 0 ;
                                                                                                                                 par_selection_rule = 2 ;

                                                                                                                                 result_BB = solve_instance(     p_p_optimal_Graph , FALSE ,
                                                                                                                                                                                                                                                                 &partial_Graph , *p_p_main_Plant , &this_Set_Clusters ) ;
                                                                                                                                 delete[] par_optimal_sequence ;
                        // Augusto 12.8.96
                        //                                      time( &finish_time ) ;
                                                                                                                                 finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                                 fprintf( statistics_file , "%d          %d      ", print_result_BB , finish_time - start_time ) ;
                                                                                                                                 assert( result_BB == INT_MAX ) ;
                        */
                    }

                    /*                              if ( par_option == 5 ) {
                                                                                                                             main_Parameters.Read_Lower_Bound_Data( &partial_Graph , *p_p_main_Plant , &this_Set_Clusters );
                                                                                                                             par_transition_speed = 1000 ;
                                                                                                                             par_selection_rule = 2 ;
                    // Augusto 12.8.96
                    //                                      time( &start_time ) ;
                                                                                                                             start_time = (int )(clock()) /CLOCKS_PER_SEC  ;

                                                                                                                             solve_instance( p_p_optimal_Graph , FALSE , &partial_Graph , *p_p_main_Plant , &this_Set_Clusters ) ;
                    // Augusto 12.8.96
                    //                                      time( &finish_time ) ;
                                                                                                                             finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             fprintf( statistics_file , "%d  ", finish_time - start_time );
                                                                                                                             assert( result_BB == INT_MAX ) ;

                                                                                                                             main_Parameters.Read_Lower_Bound_Data( &partial_Graph , *p_p_main_Plant , &this_Set_Clusters );
                                                                                                                             par_transition_speed = 0 ;
                                                                                                                             par_selection_rule = 3 ;

                    // Augusto 12.8.96
                    //                                      time( &start_time ) ;
                                                                                                                             start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             solve_instance( p_p_optimal_Graph , FALSE , &partial_Graph , *p_p_main_Plant , &this_Set_Clusters ) ;
                    // Augusto 12.8.96
                    //                                      time( &finish_time ) ;
                                                                                                                             finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             fprintf( statistics_file , "%d  ", finish_time - start_time );
                                                                                                                             assert( result_BB == INT_MAX ) ;

                                                                                                                             main_Parameters.Read_Lower_Bound_Data( &partial_Graph , *p_p_main_Plant , &this_Set_Clusters );
                                                                                                                             par_transition_speed = 1000 ;
                                                                                                                             par_selection_rule = 3 ;

                    // Augusto 12.8.96
                    //                                      time( &start_time ) ;
                                                                                                                             start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             solve_instance( p_p_optimal_Graph , TRUE , &partial_Graph , *p_p_main_Plant , &this_Set_Clusters ) ;
                    // Augusto 12.8.96
                    //                                      time( &finish_time ) ;
                                                                                                                             finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             fprintf( statistics_file , "%d          ", finish_time - start_time );
                                                                                                                             assert( result_BB == INT_MAX ) ;

                                                                                                      } else {
                                                                                                                             main_Parameters.Read_Lower_Bound_Data( &partial_Graph , *p_p_main_Plant , &this_Set_Clusters );
                                                                                                                             (*p_p_main_Plant)->Copy_Flow_Shop_Sequence() ;
                                                                                                                             par_transition_speed = 1000 ;
                                                                                                                             par_selection_rule = 2 ;

                    // Augusto 12.8.96
                    //                                      time( &start_time ) ;
                                                                                                                             start_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             solve_instance( p_p_optimal_Graph , TRUE , &partial_Graph , *p_p_main_Plant , &this_Set_Clusters ) ;
                    // Augusto 12.8.96
                    //                                      time( &finish_time ) ;
                                                                                                                             finish_time = (int )(clock()) /CLOCKS_PER_SEC  ;
                                                                                                                             fprintf( statistics_file , "%d  ", finish_time - start_time );
                                                                                                                             assert( result_BB == INT_MAX ) ;
                                                                                                      }
                    */
                }
            }
            /*              main_Parameters.Read_Heuristic_Data( &partial_Graph , *p_p_main_Plant ,
               &this_Set_Clusters ); par_transition_speed = 0 ; par_selection_rule = 2 ;

                                                     this_Optimizer.Initialize( *p_p_main_Plant ,
               &this_Set_Clusters ) ; fprintf( statistics_file , "%d\n" , this_Optimizer.Single_Lower_Bound(
               &partial_Graph ) ) ; this_Set_Clusters.Terminate() ;
            */
            par_option = par_option_backup;
            break;

        case 7:  //

            par_option_backup = par_option;
            par_option = HEURISTIC;
            all_param_output_file = open_file("all_par", "w");

            best_all = INT_MAX;

            for (i = 1; i < 5; i++)
            {
                for (j = 0; j < 3; j++)
                {
                    for (k = 0; k < 4; k++)
                    {
                        main_Parameters.Read_Heuristic_Data(
                            &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
                        // Allocates memory only for this_Set_Clusters.

                        /*//DELETE!!!!!!!*/
                        par_max_branches_main = 2;
                        par_reoptimization = 33;
                        par_max_branches_oracle = 3;
                        par_slack_active = 10 * i;
                        par_threshold_main = 0.1 * j;
                        par_threshold_oracle = 0.005 * k;

                        printf("|%d,", par_max_branches_main);
                        if (par_threshold_main > 0) printf("%1.3f,", par_threshold_main);
                        if (par_threshold_oracle == 0)
                            printf(
                                "%d,%d,%d|", par_reoptimization, par_max_branches_oracle, par_slack_active);
                        else
                            printf("%d,%d,%1.3f,%d|", par_reoptimization, par_max_branches_oracle,
                                par_threshold_oracle, par_slack_active);

                        fprintf(all_param_output_file, "|%d,", par_max_branches_main);
                        fprintf(all_param_output_file, "%1.3f,", par_threshold_main);
                        fprintf(all_param_output_file, "%d,%d,%1.3f,%d|", par_reoptimization,
                            par_max_branches_oracle, par_threshold_oracle, par_slack_active);

                        new_result = solve_instance(
                            p_p_optimal_Graph, FALSE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
                        // Dealocates this_Set_Clusters.

                        delete[] par_optimal_sequence;
                        fprintf(all_param_output_file, "%d\n", new_result);
                        FILE* best_output_file = open_file("best", "w");
                        if (new_result < best_all) best_all = new_result;

                        fprintf(best_output_file, "%d,", best_all);
                        fclose(best_output_file);
                    }
                }
            }
            fclose(all_param_output_file);

            par_max_branches_main = 1;
            main_Parameters.Read_Heuristic_Data(&partial_Graph, *p_p_main_Plant, &this_Set_Clusters);
            // Allocates memory only for this_Set_Clusters.

            solve_instance(p_p_optimal_Graph, TRUE, &partial_Graph, *p_p_main_Plant, &this_Set_Clusters);

            delete[] par_optimal_sequence;
            par_option = par_option_backup;
            break;
    }
    partial_Graph.Terminate();
}
