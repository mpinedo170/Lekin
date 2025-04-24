#include "StdAfx.h"

#include "local.h"

double Local::Choose_Parameter_K(int bottleneck_work_center, Act_Oracle* p_Act_Oracle, Graph* p_Graph)
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
    /*      if (    ( par_transition_speed == 0 ) &&
                    ( par_max_number_unscheduled_one == 0 ) &&
                    ( par_option == LOWER_BOUND ) )
                                                    //Disjunctive branching and no lower bounds.
                    return( 0.1 ) ;
    */
    for (i = 0; i < 14; i++)
    {
        p_optimal_solution = new Class_Solution_Oracle();
        p_Cluster->Initialize_Solution(p_optimal_solution);

        if (p_Cluster->Get_Number_Work_Centers() == 1)
            p_Act_Oracle->Optimize(p_optimal_solution, p_Cluster, p_Graph, 1, posible_k[i], INT_MAX, INT_MIN);
        else
            p_Act_Oracle->Optimize(
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
    return (best_K);
}

void Local::Initialize(
    // Input parameters:
    //                      Graph *p_input_Graph ,
    Plant* p_input_Plant,
    Set_Clusters* p_input_Set_Clusters)
{
    // External data structures:

    p_Plant = p_input_Plant;
    p_Set_Clusters = p_input_Set_Clusters;

    this_Oracle_Local.Initialize(EXACT_ORACLE, p_input_Plant, p_input_Set_Clusters);
}

int Local::Reoptimize(Graph* p_Graph,
    Cluster* p_Cluster,
    Act_Oracle* p_Act_Oracle,
    Dsj_Oracle* p_Dsj_Oracle,
    int* p_old_objective_value)

{
    int dead_lock = FALSE;

    Class_Solution_Oracle new_solution, deleted_solution;
    Solution_Graph this_Solution_Graph;  // Stores the optimal set of disjunctive arcs.
    int change = FALSE;                  //= TRUE if there is a modification in the graph.

    // BEGIN:

    // Allocate memory.
    p_Cluster->Initialize_Solution(&new_solution);
    p_Cluster->Copy_Solution(&deleted_solution);
    // Kept as a backup in case the reoptimization does not work.

    p_Cluster->Unschedule_Solution(p_Graph);
    p_Cluster->Delete_Graph_Matrix(p_Graph);

#ifdef ANIMATION_DEBUG
    if (par_animation_reoptimization == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Plant->Show_Gantt_Chart("gantt", p_Graph);
        printf("DELETE rescheduling arcs\n");
        char in[1];
        gets(in);
    }
#endif
#ifdef PRINT_DEBUG
    if (par_print == 1) p_Cluster->Print(p_Graph->Get_Number_Jobs(), p_Graph);
#endif

    //      p_Dsj_Oracle->Optimize(         &new_solution , p_Cluster , p_Graph ,
    //                                              par_max_branches_oracle , *p_old_objective_value - 1  ,
    //                                              INT_MIN ) ;
    // max_acceptable is *p_old_objective_value - 1, so any solution must
    // better than the old value.

    if (p_Cluster->Get_Number_Work_Centers() == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Graph->Get_Backward_Windows(TRUE);
        p_Graph->Get_Objective();

        p_Act_Oracle->Optimize(&new_solution, p_Cluster, p_Graph, par_max_branches_oracle, global_K,
            *p_old_objective_value - 1, INT_MIN);
    }
    else
        //              p_Act_Oracle->Optimize(         &new_solution , p_Cluster , p_Graph ,
        //              par_max_branches_oracle ,
        p_Act_Oracle->Optimize(
            &new_solution, p_Cluster, p_Graph, INT_MAX, global_K, *p_old_objective_value - 1, INT_MIN, NULL);

    /*      p_Dsj_Oracle->Optimize(         &new_solution , p_Cluster , p_Graph ,
                                                    par_max_branches_oracle , *p_old_objective_value - 1  ,
    INT_MIN ) ;
                                            //max_acceptable is *p_old_objective_value - 1, so any solution
    must
                                            //better than the old value.

            new_solution.Terminate() ;
            p_Cluster->Initialize_Solution( &new_solution ) ;

            p_Act_Oracle->Optimize(         &new_solution , p_Cluster , p_Graph ,
    //                                              par_max_branches_oracle , global_K ,
    *p_old_objective_value - 1  , INT_MIN , NULL ) ; INT_MAX , global_K , *p_old_objective_value - 1  ,
    INT_MIN , NULL ) ;

            if ( objective_fix_arcs != new_solution.Get_Objective() )
                    printf("") ;
            assert( objective_fix_arcs == new_solution.Get_Objective() ) ;
    */
    // WARNING! it could be a "<=".
    if (new_solution.Get_Objective() < *p_old_objective_value)
    {
#ifdef PRINT_DEBUG
        if (par_print == 1) new_solution.Print();
#endif

        *p_old_objective_value = new_solution.Get_Objective();
        p_Cluster->Schedule_Solution(p_Graph, &new_solution);
        change = TRUE;
    }
    else
    {
        // The new objective is worse or equal, so we recall the deleted data.

        p_Cluster->Schedule_Solution(p_Graph, &deleted_solution);
    }

#ifdef ANIMATION_DEBUG
    if (par_animation_reoptimization == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Plant->Show_Gantt_Chart("gantt", p_Graph);
        printf("ADD rescheduling arcs\n");
        char in[1];
        gets(in);
    }
#endif

    // De-allocate memory.
    deleted_solution.Terminate();
    new_solution.Terminate();

    return (change);
}

/*
void Local::Recursive_Call(     int input_job , int input_step , int center_id ,
                                        int remaining_centers , int current_sigma ,
                                        Cluster *p_Cluster , Graph *p_Graph )
{
int size_center = 0 ;   //There is at least one operation: ( input_job , input_step ).

Class_Node_Index *node_center = new Class_Node_Index[MAX_OPERATIONS_WORK_CENTER] ;
                                //The operations to be added to the center are stored here.

        int job = input_job ;
        int step = input_step ;
        while ( p_Graph->Mach_Pred_Is_Adjacent( &job , &step ) )
                                //If there is an adjacent machine predecessor, variables
                                //job and step take the value.
                ;
        Class_Node_Index *p_pred_node ;
        int mach_pred_job , mach_pred_step ;
        if ( p_Graph->There_Is_Mach_Pred( &mach_pred_job , &mach_pred_step , job , step ) )
                                //Obtain the machine predecessor for the center.

                p_pred_node = new Class_Node_Index( mach_pred_job , mach_pred_step ) ;
        else
                p_pred_node = NULL;

        node_center[ size_center ].job = job ;
        node_center[ size_center ].step = step ;
        size_center++ ;
        while ( p_Graph->Mach_Succ_Is_Adjacent( &job , &step ) ) {
                node_center[ size_center ].job = job ;
                node_center[ size_center ].step = step ;
                size_center++ ;
        }
                                        #ifdef CHECK_DEBUG
                                                assert( size_center <= MAX_OPERATIONS_WORK_CENTER ) ;
                                        #endif

        Class_Node_Index *p_succ_node ;
        int mach_succ_job , mach_succ_step ;
        if ( p_Graph->There_Is_Mach_Succ( &mach_succ_job , &mach_succ_step , job , step ) )
                p_succ_node = new Class_Node_Index( mach_succ_job , mach_succ_step ) ;
        else
                p_succ_node = NULL;


        Cluster_Work_Center *current_center = new Cluster_Work_Center ;
        current_center->Initialize(     center_id ,
                                                size_center ,
                                                1 ,                     //Number of machines.
                                                p_pred_node , p_succ_node ,
                                                p_Graph );

//Adding the operations to the center.
        for ( int i = 0 ; i < size_center ; i++ )
                current_center->Add_Operation( node_center[i].job , node_center[i].step , p_Graph ) ;

//
        p_Cluster->Assign_p_Work_Center( current_center ) ;
        int job_pred_center_id ;
        if (    remaining_centers > 1
                &&
                p_Graph->Job_Pred_Is_Adjacent( &job_pred_center_id , input_job , input_step , current_sigma  )
        )

                Recursive_Call(         input_job , input_step - 1 , job_pred_center_id ,
                                        remaining_centers - 1 , pos( current_sigma - DELTA_SIGMA ) ,
                                        p_Cluster , p_Graph ) ;
}


void Local::Get_Cluster(        int input_job , int input_step , Cluster *p_Cluster , Graph *p_Graph )
{
int size_cluster = 1 ;  //There is at least one operation: ( input_job , input_step ).

                  int job_pred_center_id ;
        int current_step = input_step ;
        int sigma = par_sigma ;
                                        #ifdef CHECK_DEBUG
                                                assert( 0 <= par_sigma ) ;
                                                assert( 1 <= par_max_number_centers ) ;
                                        #endif
        while (         size_cluster < par_max_number_centers
                        &&
                        p_Graph->Job_Pred_Is_Adjacent( &job_pred_center_id , input_job , current_step , sigma
) ) {
                                //If there is an adjacent machine predecessor, variables
                                //job and step take the value.

                sigma = pos( par_sigma - DELTA_SIGMA*size_cluster ) ;

                size_cluster++ ;
                                //Counting the number of centers in the cluster.
                current_step-- ;
        }
        p_Cluster->Initialize( 0 , size_cluster , p_Graph ) ;
        p_Cluster->Set_Original( input_job , input_step ) ;

        Recursive_Call( input_job , input_step , job_pred_center_id ,
                                size_cluster , par_sigma ,
                                p_Cluster , p_Graph ) ;


}

void Local::Rank_Operations( Beam_Indices<Class_Node_Index> *p_list_ranking_nodes , Graph *p_Graph )
{
int i , j ;
Class_Node_Index *p_Ranking_Operation ;

        int *completion_penalty = p_Graph->Get_Completion_Time_Penalty() ;

        p_Graph->Update_Nodes( ) ;

//      int tardiness ;
        int lateness ;
        int slack ;

        int number_jobs = p_Graph->Get_Number_Jobs() ;
        int number_steps ;
        for( i = 0 ; i < number_jobs ; i++ ) {
                number_steps = p_Graph->Get_Number_Steps_Job(i) ;
                for( j = 1 ; j <= number_steps ; j++ ) {
                        slack = p_Graph->Slack_Node( i , j ) ;
                        if ( slack > 0 ) {
                                p_Ranking_Operation = new Class_Node_Index( i , j ) ;
//                              tardiness = p_Graph->Tardiness_Node( i , j ) ;
                                lateness = 10 * p_Graph->Lateness_Node( i , j ) / completion_penalty[ i ] ;

                                        #ifdef CHECK_DEBUG
                                                assert( lateness <= 0 ) ;
                                        #endif

                                p_list_ranking_nodes->Add_Element( 1000-lateness , p_Ranking_Operation , 0 );
                        }
                }
        }
}

int Local::Get_Cluster_Index( Cluster *p_Cluster , int cluster_objective , int original_objective )
{
                                        #ifdef CHECK_DEBUG
                                                assert( cluster_objective <= original_objective ) ;
                                        #endif
        if ( cluster_objective == original_objective )
                return( 0 ) ;   //To reschedule this cluster will not help.
        if ( p_Cluster->Get_Number_Operations() > 20 )
                return( 0 ) ;   //To many operations.

        int index = ( original_objective - cluster_objective )
                        +
                        p_Cluster->Get_Number_Work_Centers() * 10
                        -
                        p_Cluster->Get_Number_Operations() ;

                                        #ifdef CHECK_DEBUG
                                                assert( index > 0 ) ;
                                        #endif
        return( index ) ;
}

void Local::Rank_Clusters( Beam_Indices<Cluster> *p_list_clusters ,
                                        Solution_Graph *p_Solution_Graph ,
                                        Graph *p_Graph )
{
Beam_Indices<Class_Node_Index> list_ranked_operations ;
Class_Node_Index *p_Ranked_Operation , *p_Cluster_Operation ;
int cluster_index ;
int dead_lock = FALSE ;
Cluster *p_Cluster ;
int cluster_objective ;

        list_ranked_operations.Initialize(      100 , 0 , 0.0 ) ;
                                                //Maximum 20 elements.

//      p_Graph->Read_Solution( p_Solution_Graph ) ;
        p_Graph->Read_Machines( p_Solution_Graph ) ;
        p_Graph->Read_Arcs( p_Solution_Graph ) ;
//      p_Graph->Set_Scheduled() ;
        p_Graph->Get_Forward_Windows( &dead_lock , NULL );
                                        #ifdef CHECK_DEBUG
                                                assert( dead_lock == FALSE ) ;
                                        #endif
        int original_objective = p_Graph->Get_Objective( );

                                        #ifdef ANIMATION_DEBUG
                                        if (par_scheduling_operation == 1) {
                                                p_Graph->Get_Forward_Windows( &dead_lock );
                                                p_Plant->Show_Gantt_Chart( "gantt" , p_Graph );
                                                printf("CURRENT schedule\n") ;
                                                char in[1] ;
                                                gets(in);
                                        }
                                        #endif

        p_Graph->Get_Backward_Windows( );
        Rank_Operations( &list_ranked_operations , p_Graph ) ;

        int number_elements = list_ranked_operations.Number_Elements_List() ;
        for( int i = 0 ; i < number_elements ; i++ ) {
                p_Ranked_Operation = list_ranked_operations.Get_p_ith_Element( i ) ;
                p_Cluster = new Cluster ;
                Get_Cluster(    p_Ranked_Operation->job , p_Ranked_Operation->step ,
                                        p_Cluster , p_Graph ) ;
                                                //Cet_Cluster allocates memory.

                                        #ifdef ANIMATION_DEBUG
                                        if (par_scheduling_operation == 1) {
                                                p_Graph->Get_Forward_Windows( &dead_lock );
                                                p_Plant->Show_Gantt_Chart( "gantt" , p_Graph );
                                                printf("CURRENT schedule\n") ;
                                                char in[1] ;
                                                gets(in);
                                        }
                                        #endif

                p_Graph->Get_Forward_Windows( &dead_lock );
                                        #ifdef CHECK_DEBUG
                                                assert( dead_lock == FALSE ) ;
                                        #endif
                cluster_objective = p_Graph->Get_Objective( );
                cluster_index = Get_Cluster_Index( p_Cluster , cluster_objective , original_objective ) ;

                if (    ( cluster_index != 0 )
                        &&
                        ( p_list_clusters->Is_Member( double(cluster_index), p_Cluster ) == FALSE )
                )
                        p_list_clusters->Add_Element( double(cluster_index) , p_Cluster , 0 );

                p_Graph->Delete_All_Disjunctives( ) ;
                p_Graph->Read_Solution( p_Solution_Graph ) ;
                p_Graph->Get_Forward_Windows( &dead_lock );
                                        #ifdef CHECK_DEBUG
                                                assert( dead_lock == FALSE ) ;
                                        #endif
        }
        list_ranked_operations.Terminate() ;
}

void Local::Search_Neighborhood(        Beam_Indices<Cluster> *p_list_clusters ,
                                                        Solution_Graph *p_Solution_Graph ,
                                                        Graph *p_Graph )
{
Class_Node_Index *p_Cluster_Operation ;
Class_Solution_Oracle new_Solution_Oracle ;
int dead_lock = FALSE ;
Cluster this_Cluster ;

        int current_objective = p_Graph->Get_Objective() ;

        for( int i = 0 ; i < p_list_clusters->Number_Elements_List() ; i++ ) {
                p_Cluster_Operation = ( p_list_clusters->Get_p_ith_Element( i ) )->Get_p_Original() ;
                Get_Cluster(    p_Cluster_Operation->job , p_Cluster_Operation->step ,
                                        &this_Cluster , p_Graph ) ;

                                        #ifdef ANIMATION_DEBUG
                                        if (par_scheduling_operation == 1) {
                                                p_Graph->Get_Forward_Windows( &dead_lock );
                                                p_Plant->Show_Gantt_Chart( "gantt" , p_Graph );
                                                printf("CURRENT schedule\n") ;
                                                char in[1] ;
                                                gets(in);
                                        }
                                        #endif

                this_Cluster.Initialize_Solution( &new_Solution_Oracle ) ;

                this_Oracle_Local.Optimize(     &new_Solution_Oracle ,
                                                &this_Cluster , p_Graph ,
                                                par_max_branches_oracle , global_K , current_objective - 1 ,
INT_MIN , NULL ) ;

                if ( new_Solution_Oracle.Get_Objective() < current_objective ) {
                        current_objective = new_Solution_Oracle.Get_Objective() ;
                        printf("%d,", current_objective );
                }

                new_Solution_Oracle.Terminate() ;
                this_Cluster.Terminate() ;

                p_Graph->Delete_All_Disjunctives( ) ;
                p_Graph->Read_Solution( p_Solution_Graph ) ;
                p_Graph->Get_Forward_Windows( &dead_lock );
                                        #ifdef CHECK_DEBUG
                                                assert( dead_lock == FALSE ) ;
                                        #endif

        }
}

void Local::Run( Solution_Graph *p_Solution_Graph ,
                        Graph *p_Graph ,                //It is empty.
                        Plant *p_Plant )
{

Beam_Indices<Cluster> list_clusters ;

        list_clusters.Initialize( par_size_cluster_list , 0 , 0.0 ) ;
        Rank_Clusters( &list_clusters , p_Solution_Graph , p_Graph ) ;

//      par_scheduling_operation = 1 ;

                                        #ifdef ANIMATION_DEBUG
                                        if (par_scheduling_operation == 1) {
                                                int dead_lock = FALSE ;
                                                p_Graph->Get_Forward_Windows( &dead_lock );
                                                p_Plant->Show_Gantt_Chart( "gantt" , p_Graph );
                                                printf("CURRENT schedule\n") ;
                                                char in[1] ;
                                                gets(in);
                                        }
                                        #endif

        Search_Neighborhood( &list_clusters , p_Solution_Graph , p_Graph ) ;
        list_clusters.Terminate() ;
}
*/
void Local::Shuffle(Solution_Graph* p_Solution_Graph, Graph* p_Graph)
{
    int i;
    Cluster this_Cluster, *p_Cluster;
    Act_Oracle this_Act_Heur_Orcl;
    Dsj_Oracle this_Disj_Heur_Orcl;
    int dead_lock = FALSE;
    Class_Solution_Oracle better_solution_oracle;

    this_Act_Heur_Orcl.Initialize(HEURISTIC_ORACLE, p_Plant, p_Set_Clusters);
    this_Disj_Heur_Orcl.Initialize(HEURISTIC_ORACLE, p_Plant, p_Set_Clusters);

    p_Graph->Read_Arcs(p_Solution_Graph);
    for (i = 0; i < p_Set_Clusters->Number_Clusters(); i++)
    {
        p_Cluster = p_Set_Clusters->Get_Cluster(i);
        p_Cluster->Initialize_Solution(&better_solution_oracle);
        this_Act_Heur_Orcl.Optimize(
            &better_solution_oracle, p_Cluster, p_Graph, INT_MAX, global_K, INT_MAX, INT_MIN, NULL);
        p_Cluster->Schedule_Solution(p_Graph, &better_solution_oracle);
        better_solution_oracle.Terminate();
    }
    p_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif
    int current_objective = p_Graph->Get_Objective();
    int original_objective = current_objective;

#ifdef CHECK_DEBUG
    assert(original_objective == p_Solution_Graph->Get_Objective());
#endif
#ifdef ANIMATION_DEBUG
    if (par_scheduling_operation == 1)
    {
        p_Graph->Get_Forward_Windows(&dead_lock);
        p_Plant->Show_Gantt_Chart("gantt", p_Graph);
        printf("CURRENT schedule\n");
        char in[1];
        gets(in);
    }
#endif

    par_transition_speed = 0;
    par_selection_rule = BOTTLE;
    int par_size_shuffle = min2(5, p_Graph->Number_Centers_Plant());
    i = 0;
    int flag_stop = FALSE;
    while (((i == 0) || (i + par_size_shuffle < p_Graph->Number_Centers_Plant())) && (flag_stop == FALSE))
    {
        p_Set_Clusters->Create_Cluster(
            &this_Cluster, p_Set_Clusters->Get_Optimal_Sequence(), i, par_size_shuffle, p_Graph);

        Reoptimize(p_Graph, &this_Cluster, &this_Act_Heur_Orcl, &this_Disj_Heur_Orcl, &current_objective);
        this_Cluster.Terminate();
        i++;
        if (current_objective < original_objective)
        {
            // If the current solution stills competitive.

            p_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
            assert(dead_lock == FALSE);
#endif

            p_Graph->Set_Objective(current_objective);

            p_Graph->Write_Solution(p_Solution_Graph);
            // Deletes, Prepares and Copies the output.

            int solution_time;
            // Augusto 12.8.96
            //                      time( &solution_time ) ;
            //                        solution_time = (int )(clock()) /CLOCKS_PER_SEC  ;
            solution_time = (int)(clock()) / CLK_TCK;

            printf("\n((%d,%d))\n", p_Graph->Get_Objective(), solution_time - first_time);
            original_objective = current_objective;
        }
        else
        {
            flag_stop = TRUE;
        }
    }
}
