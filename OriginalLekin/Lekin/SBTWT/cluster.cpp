#include "StdAfx.h"

#include "cluster.h"

Cluster::Cluster()
{
    id = NULL_CLUSTER;
    number_work_centers = NULL_NUMBER;
    number_operations = 0;
    is_scheduled = FALSE;
    step_length_average = -1;
    p_work_center = NULL;
    current_position = 0;
    number_jobs = -1;
};

// void Cluster::Initialize( int input_id , int input_number_work_centers , int number_work_centers_graph  )
void Cluster::Initialize(int input_id, int input_number_work_centers, Graph* p_Graph)
{
    id = input_id;
    number_work_centers = input_number_work_centers;
    number_operations = 0;

    // Allocating memory:
    p_work_center = new type_p_Cluster_Work_Center[number_work_centers];
    //      rank_work_centers = NULL ;
    current_position = 0;

    number_jobs = p_Graph->Get_Number_Jobs();
    //      operation = new type_p_int[ number_jobs ] ;
    //      for( i = 0 ; i < number_jobs ; i++ ) {
    //              operation[ i ] = new int[ p_Graph->Get_Number_Steps_Job(i) + 1 ] ;
    //              for( j = 1 ; j < (p_Graph->Get_Number_Steps_Job(i) + 1) ; j++ ) {
    //                      operation[ i ][ j ] = CHECK_FALSE ;
    //              }
    //      }
}

void Cluster::Calculate_Step_Length_Average()
{
    int i, j;
    int sum_length = 0;

    // Calculate step length average
    for (i = 0; i < number_work_centers; i++)
        for (j = 0; j < p_work_center[i]->number_operations; j++)
            sum_length = sum_length + p_work_center[i]->Get_Operation(j)->Get_Length();
    // Luis Peña 22 de Septiembre de 1997
    if (number_operations != 0)
        step_length_average = (sum_length / number_operations);
    else
        step_length_average = sum_length;
};

/*
void Cluster::Terminate_Set( )
{
int i ;
        if ( rank_work_centers != NULL )
                delete[] rank_work_centers ;

//      for( i = 0 ; i < number_jobs ; i++ )
//              delete [] operation[ i ] ;
//      delete [] operation ;

        delete[] p_work_center ;

                                        #ifdef CHECK_DEBUG
                                                assert( current_position == number_work_centers ) ;
                                                        //Otherwise some position was not filled.
                                        #endif
}
*/

void Cluster::Terminate()
{
    /*
            int i ;
            for( i = 0 ; i < number_jobs ; i++ )
                    delete [] operation[ i ] ;
            delete [] operation ;

            for ( i = 0 ; i < number_work_centers ; i++ )
                    p_work_center[i]->Terminate() ;
            delete[] p_work_center ;
    */
#ifdef CHECK_DEBUG
    assert(current_position == number_work_centers);
    // Otherwise some position was not filled.
#endif
}

int Cluster::operator==(const Cluster& from)
{
    int i;

    if ((number_work_centers != from.number_work_centers) || (number_operations != from.number_operations))
        return (FALSE);

    for (i = 0; i < number_work_centers; i++)
        if ((*(p_work_center[i]) == *(from.p_work_center[i])) == FALSE) return (FALSE);

    return (TRUE);
}

void Cluster::Assign_p_Work_Center(Cluster_Work_Center* p_Cluster_Work_Center)
{
    int i;

    p_work_center[current_position] = p_Cluster_Work_Center;

    if (current_position == 0) number_machines = 0;
    if (number_work_centers == 1)
    {
        number_operations = number_operations + p_Cluster_Work_Center->number_operations;
        for (i = 0; i < number_work_centers; i++)
        {
            number_machines = number_machines + p_work_center[i]->number_machines;
        }
    }
    else
    {
        number_operations = number_operations + p_Cluster_Work_Center->number_operations;
        number_machines = number_machines + p_work_center[current_position]->number_machines;
    }
    current_position++;
}

void Cluster::Initialize_Solution(Class_Solution_Oracle* p_solution)
{
    int i, j, k;

    p_solution->value = INT_MAX;
    p_solution->time_out = FALSE;
    p_solution->cluster_id = id;
    p_solution->number_work_centers = number_work_centers;
    p_solution->number_machines_work_center = new int[number_work_centers];
    p_solution->length_solution = new type_p_int[number_work_centers];
    p_solution->solution = new type_p_p_int[number_work_centers];
    // There is a typedef int* type_p_long int and typedef int** type_p_p_long int in Data_Structures since
    // ANSI C++ forbids array dimensions with parenthesized type.

    for (i = 0; i < number_work_centers; i++)
    {
        p_solution->number_machines_work_center[i] = p_work_center[i]->number_machines;

        p_solution->length_solution[i] = new int[p_work_center[i]->number_machines];
        p_solution->solution[i] = new type_p_int[p_work_center[i]->number_machines];

        for (j = 0; j < p_work_center[i]->number_machines; j++)
        {
            p_solution->length_solution[i][j] = 0;
            p_solution->solution[i][j] = new int[p_work_center[i]->number_operations];

            for (k = 0; k < p_work_center[i]->number_operations; k++)
            {
                p_solution->solution[i][j][k] = NULL_JOB;
            }
        }
    }
}

// Copy_Solution( Class_Solution_Oracle *p_solution ): makes a copy of the curent solution of the cluster.
void Cluster::Copy_Solution(Class_Solution_Oracle* p_solution)
{
    int i, j, k;

    p_solution->value = 0;
    p_solution->cluster_id = id;
    p_solution->number_work_centers = number_work_centers;
    p_solution->number_machines_work_center = new int[number_work_centers];
    p_solution->length_solution = new type_p_int[number_work_centers];
    p_solution->solution = new type_p_p_int[number_work_centers];
    // There is a typedef int* type_p_long int and typedef int** type_p_p_long int in Data_Structures since
    // ANSI C++ forbids array dimensions with parenthesized type.

    for (i = 0; i < number_work_centers; i++)
    {
        p_solution->number_machines_work_center[i] = p_work_center[i]->number_machines;

        p_solution->length_solution[i] = new int[p_work_center[i]->number_machines];
        p_solution->solution[i] = new type_p_int[p_work_center[i]->number_machines];

        for (j = 0; j < p_work_center[i]->number_machines; j++)
        {
            p_solution->length_solution[i][j] = p_work_center[i]->machine_number_scheduled[j];
            p_solution->solution[i][j] = new int[p_work_center[i]->machine_number_scheduled[j]];

            for (k = 0; k < p_work_center[i]->machine_number_scheduled[j]; k++)
            {
                p_solution->solution[i][j][k] = p_work_center[i]->sequence[j][k];
            }
        }
    }
}

void Cluster::Print(int number_jobs, Graph* p_Graph)
{
    int i;  // counters.

    // fprintf( par_output_file ,"cluster id: %d ;", id );
    for (i = 0; i < number_work_centers; i++)
    {
        // Choose the work center in the cluster.
        p_work_center[i]->Print(number_jobs, p_Graph);
    }
}

void Cluster::Schedule_Solution(Graph* p_Graph, Class_Solution_Oracle* p_input_Solution_Oracle)
{
    int i, j, k;
#ifdef CHECK_DEBUG
    assert(p_input_Solution_Oracle->cluster_id == id);
#endif

    for (i = 0; i < number_work_centers; i++)
    {
        for (j = 0; j < p_work_center[i]->number_machines; j++)
        {
            for (k = 0; k < p_input_Solution_Oracle->length_solution[i][j]; k++)
            {
                p_work_center[i]->Schedule_Operation(p_Graph, j, p_input_Solution_Oracle->solution[i][j][k]);
                // We do not update the graph.
            }
        }
    }
}

// void Cluster::Unschedule_Solution( Graph *p_Graph , int flag_temp )
void Cluster::Unschedule_Solution(Graph* p_Graph)
{
    int i, j, k;

    for (i = 0; i < number_work_centers; i++)
    {
        for (j = 0; j < p_work_center[i]->number_machines; j++)
        {
            for (k = p_work_center[i]->machine_number_scheduled[j] - 1; k >= 0; k--)
            {
                p_work_center[i]->Unschedule_Operation(j, p_work_center[i]->sequence[j][k], p_Graph);
                // We do not update the graph.
            }
        }
    }
}

int Cluster::Get_Number_Scheduled_Operations()
{
    int i;
    int number_scheduled = 0;

    for (i = 0; i < number_work_centers; i++)
    {
        number_scheduled = number_scheduled + p_work_center[i]->total_number_scheduled;
    }
    return (number_scheduled);
}

int Cluster::Get_Number_Unscheduled_Operations()
{
    int i;
    int number_unscheduled = 0;

    for (i = 0; i < number_work_centers; i++)
    {
        number_unscheduled = number_unscheduled + p_work_center[i]->Get_Number_Unscheduled_Operations();
    }
    return (number_unscheduled);
}
//--------------------------------------------------------------------------------------------
// For the HEURISTIC_ORACLE
int Cluster::Earliest_Finishing_Work_Center(Graph* p_Graph)
{
    int i;
    int center_finishing_time;  // Earliest finishing time of the current work center.
    int min_finishing_time = INT_MAX;
    int location_work_center;

    for (i = 0; i < number_work_centers; i++)
    {
        p_work_center[i]->Clean_Star();
        center_finishing_time = p_work_center[i]->Earliest_Finishing(p_Graph);
        // For type_oracle == HEURISTIC_ORACLE,
        // the star operation does not have to be complete.

        if (center_finishing_time < min_finishing_time)
        {
            location_work_center = i;
            min_finishing_time = center_finishing_time;
        }
        // Lpm Cambio en el programa 17 de octubre de 1997

        //     else{
        //             min_finishing_time = 0;
        //             location_work_center = -1;
        //     }
    }
#ifdef CHECK_DEBUG
    assert(min_finishing_time < INT_MAX);
#endif

    //      if ( p_work_center[ location_work_center ]->There_Is_Star( p_Graph ) )
    //              p_work_center[ location_work_center ]->Calculate_Release_Times( p_Graph ) ;

    return (location_work_center);
}

// Earliest_Completion_Work_Center(): returns the id of the center in the cluster that has
// the earliest completion time.
// WARNING!!!!!!!!!
int Cluster::Earliest_Completion_Work_Center(Graph* p_Graph)
{
    int i;
    int center_completion_star;  // Earliest completion time of the current work center.
    int min_completion_star = INT_MAX;
    int location_work_center = NULL_WORK_CENTER;

    for (i = 0; i < number_work_centers; i++)
    {
        //              center_completion_star = p_work_center[i]->Completion_Star( p_Graph , forbidden[i] ,
        //              TRUE ) ;
        p_work_center[i]->Clean_Star();
        // Inicializar.

        center_completion_star = p_work_center[i]->Completion_Star(p_Graph, TRUE);
        // Find the star operation.

        if (center_completion_star < min_completion_star)
        {
            location_work_center = i;
            min_completion_star = center_completion_star;
        }
    }
#ifdef CHECK_DEBUG
    if (min_completion_star == INT_MAX) assert(min_completion_star < INT_MAX);
#endif

    return (location_work_center);
}
// Cluster::Bottleneck_Work_Center(): returns the location of the branching center.
// int Cluster::Bottleneck_Work_Center( Graph *p_Graph , int **forbidden )
int Cluster::Bottleneck_Work_Center(Graph* p_Graph)
{
    int i, j;

    int number_centers_plant = p_Graph->Number_Centers_Plant();
    // Size of the par_optimal_sequence[] sequence.
#ifdef CHECK_DEBUG
    assert(number_work_centers <= number_centers_plant);
#endif
    for (i = 0; i < number_centers_plant; i++)
    {
#ifdef CHECK_DEBUG
        assert(par_optimal_sequence[i] < number_centers_plant);
        assert(par_optimal_sequence[i] >= 0);
#endif
        for (j = 0; j < number_work_centers; j++)
        {
            if (p_work_center[j]->Get_Id() == par_optimal_sequence[i])
            {
                if (p_work_center[j]->Get_Number_Unscheduled_Operations() > 0) return (j);
            }
        }
    }
    return (NULL_WORK_CENTER);
}

// Cluster::Highest_Bound_Center(): returns the location of the branching center.
// int Cluster::Highest_Bound_Center(    Graph *p_Graph , int **forbidden ,
int Cluster::Highest_Bound_Center(Graph* p_Graph, Set_Work_Centers* p_input_Set_Work_Centers)
{
#ifdef CHECK_DEBUG
    assert(p_input_Set_Work_Centers != NULL);
#endif
    int center;
    for (int i = 0; i < number_work_centers; i++)
    {
        center = p_input_Set_Work_Centers->Highest_Bound_Center(i);
#ifdef CHECK_DEBUG
        assert(center >= 0);
#endif

        if (p_work_center[center]->Get_Number_Unscheduled_Operations() > 0) return (center);
    }
    return (NULL_WORK_CENTER);
}

// Arcs_Max_Increase_Center(): returns INT_MAX if is a fixed disjunction.
int Cluster::Arcs_Max_Increase_Center(Class_Disjunctive_Arc* p_first_arc,
    Class_Disjunctive_Arc* p_second_arc,
    int* p_location_branch_center,
    Graph* p_Graph,
    int depth)
{
    int index_1, index_2;
    int max_index_1 = INT_MIN;
    int max_index_2 = INT_MIN;
    Class_Disjunctive_Arc this_first_arc, this_second_arc;

    /*      if ( par_selection_rule == TURN ) {
                    for ( int i = 0 ; i < number_work_centers ; i++ ){
                            p_work_center[i]->Get_Index( &index_1 , &index_2 , &this_first_arc,
    &this_second_arc , p_Graph ) ; #ifdef CHECK_DEBUG assert( index_1 < INT_MAX ) ; #endif

    //                      if ( p_Graph->Get_N_Blank_Centers(i) > par_number_blanks ) {
                            if ( index_1 > max_index_1 ) {
                                    (*p_first_arc) = this_first_arc ;
                                    (*p_second_arc) = this_second_arc ;
                                    max_index_1 = index_1 ;
                                    max_index_2 = index_2 ;
                                    *p_location_branch_center = i ;
                            } else if ( ( index_1 == max_index_1 ) && ( index_2 > max_index_2 ) ) {
                                    (*p_first_arc) = this_first_arc ;
                                    (*p_second_arc) = this_second_arc ;
                                    max_index_2 = index_2 ;
                                    *p_location_branch_center = i ;
                            }
                    }
                    if ( max_index_1 > INT_MIN )
                            return( max_index_1 ) ;
            }
    */
    for (int i = 0; i < number_work_centers; i++)
    {
        p_work_center[i]->Get_Index(&index_1, &index_2, &this_first_arc, &this_second_arc, p_Graph);
#ifdef CHECK_DEBUG
        assert(index_1 < INT_MAX);
#endif

        if (index_1 > max_index_1)
        {
            (*p_first_arc) = this_first_arc;
            (*p_second_arc) = this_second_arc;
            max_index_1 = index_1;
            max_index_2 = index_2;
            *p_location_branch_center = i;
        }
        else if ((index_1 == max_index_1) && (index_2 > max_index_2))
        {
            (*p_first_arc) = this_first_arc;
            (*p_second_arc) = this_second_arc;
            max_index_2 = index_2;
            *p_location_branch_center = i;
        }
    }
    return (max_index_1);
    // returns INT_MIN if all the arcs have been assigned.
}

// Arcs_Bottleneck_Center(): returns INT_MAX if is a fixed disjunction.
int Cluster::Arcs_Bottleneck_Center(Class_Disjunctive_Arc* p_first_arc,
    Class_Disjunctive_Arc* p_second_arc,
    int* p_location_branch_center,
    Graph* p_Graph,
    int depth)
{
    int i, j;
    int index_1, index_2;

    int number_centers_plant = p_Graph->Number_Centers_Plant();
    // Size of the par_optimal_sequence[] sequence.
#ifdef CHECK_DEBUG
    assert(number_work_centers <= number_centers_plant);
#endif

    par_selection_rule = BOTTLE;

    // Round 1:
    for (i = 0; i < number_centers_plant; i++)
    {
#ifdef CHECK_DEBUG
        assert(par_optimal_sequence[i] < number_centers_plant);
        assert(par_optimal_sequence[i] >= 0);
#endif
        for (j = 0; j < number_work_centers; j++)
        {
            if (p_work_center[j]->Get_Id() == par_optimal_sequence[i])
            {
                if (p_Graph->Get_N_Blank_Centers(par_optimal_sequence[i]) > 0)
                {
                    p_work_center[j]->Get_Index(&index_1, &index_2, p_first_arc, p_second_arc, p_Graph);
                    *p_location_branch_center = j;
                    return (index_1);
                }
            }
        }
    }
    return (INT_MIN);
    // returns INT_MIN if all the arcs have been assigned.
}

// Arcs_Max_Bound_Center(): returns INT_MAX if is a fixed disjunction.
int Cluster::Arcs_Max_Bound_Center(Class_Disjunctive_Arc* p_first_arc,
    Class_Disjunctive_Arc* p_second_arc,
    int* p_location_branch_center,
    Graph* p_Graph,
    int depth,
    Set_Work_Centers* p_Set_Centers)
{
    int index_1, index_2;

    if (p_Set_Centers == NULL)
        return (
            Arcs_Max_Increase_Center(p_first_arc, p_second_arc, p_location_branch_center, p_Graph, depth));
    int center;
    /*      if ( par_selection_rule == TURN ) {
                    for ( int i = 0 ; i < number_work_centers ; i++ ) {
                            center = p_Set_Centers->Highest_Bound_Center( i ) ;
                            if ( center == -1 )
                                    return(Arcs_Max_Increase_Center(        p_first_arc , p_second_arc ,
                                                                                            p_location_branch_center
       , p_Graph , depth ) ) ; if ( p_Graph->Get_N_Blank_Centers( center ) > 0 ) { p_work_center[ center
       ]->Get_Index(     &index_1 , &index_2 , p_first_arc, p_second_arc , 0 , p_Graph ) ;
                                    *p_location_branch_center = center ;
                                    return( index_1 ) ;
                            }
                    }
            }
    */
    for (int i = 0; i < number_work_centers; i++)
    {
        center = p_Set_Centers->Highest_Bound_Center(i);
        if (center == -1)
            return (Arcs_Max_Increase_Center(
                p_first_arc, p_second_arc, p_location_branch_center, p_Graph, depth));
        if (p_Graph->Get_N_Blank_Centers(center) > 0)
        {
            p_work_center[center]->Get_Index(&index_1, &index_2, p_first_arc, p_second_arc, p_Graph);
            *p_location_branch_center = center;
            return (index_1);
        }
    }
    return (INT_MIN);
    // returns INT_MIN if all the arcs have been assigned.
}

int Cluster::Fix_Disjunctions(int* p_n_arcs_list, Arc_Backup* list, int max_possible_increase, Graph* p_Graph)
{
    int dead_lock = FALSE;
    int delta = 0;

    // First to update the graph.
    p_Graph->Get_Forward_Windows(&dead_lock);
    // Obtains all the release dates.

#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif

    p_Graph->Get_Backward_Windows(FALSE);
    // Obtains all the due dates.
    // FALSE means that it does not requiere delayed precedence constraints.

    for (int i = 0; i < number_work_centers; i++)
    {
        if ((p_Graph->Valid_Answer_Cent(p_work_center[i]->Get_Id()) == FALSE)
            // TRUE if the selection of arcs is a tour.
            // A tour visits all the nones once with no cycle.
            &&
            //                        ( p_work_center[i]->Get_Number_Machines() == 1 )
            (p_work_center[i]->number_machines == 1))
        {
            delta = p_work_center[i]->Fix_Disjunctions(p_n_arcs_list, list, max_possible_increase, p_Graph);
        }
        if (delta > 0) return (delta);
    }
    return (0);
}

/*
void Cluster::Back_Up_Matrix(   int ****p_p_back_up_matrix ,
                                                int **p_p_n_blanks_center , int *p_n_blanks_graph ,
                                                int ****p_p_back_up_num_disj , int *p_n_disjunctives_graph ,
                                                Graph *p_Graph )
{
        (*p_p_back_up_matrix) = new type_p_p_int[number_work_centers] ;
        (*p_p_n_blanks_center) = new int[number_work_centers] ;
        (*p_p_back_up_num_disj) = new type_p_p_int[number_work_centers] ;
        for( int i = 0 ; i < number_work_centers ; i++ )
                p_Graph->Back_Up_Center_Matrix(         &((*p_p_back_up_matrix)[i]) ,
&((*p_p_n_blanks_center)[i]) , p_n_blanks_graph ,
                                                                        &((*p_p_back_up_num_disj)[i]) ,
p_n_disjunctives_graph , p_work_center[ i ]->Get_Id() ) ;
}

void Cluster::Restore_Graph_Matrix(     int ***p_back_up_matrix , int *p_n_blanks_center , int n_blanks_graph
, int ***p_back_up_num_disj , int n_disjunctives_graph , Graph *p_Graph )
{
        for( int i = 0 ; i < number_work_centers ; i++ )
                p_Graph->Restore_Center_Matrix(         p_back_up_matrix[i] , p_n_blanks_center[i] ,
n_blanks_graph , p_back_up_num_disj[i] , n_disjunctives_graph , p_work_center[i]->Get_Id() ) ; delete[]
p_back_up_matrix ; delete[] p_n_blanks_center ; delete[] p_back_up_num_disj ;
}
*/

void Cluster::Delete_Graph_Matrix(Graph* p_Graph)
{
    for (int i = 0; i < number_work_centers; i++) p_Graph->Delete_Center_Matrix(p_work_center[i]->Get_Id());
    p_Graph->Set_Obsolete_Forward();
}

// TRUE if the selection of arcs is a tour.
int Cluster::Valid_Answer(Graph* p_Graph)
{
    for (int i = 0; i < number_work_centers; i++)
    {
        if (p_Graph->Valid_Answer_Cent(p_work_center[i]->Get_Id()) == FALSE)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}
