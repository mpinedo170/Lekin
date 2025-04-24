#include "StdAfx.h"

#include "graph.h"

void Graph::Reset()
{
    updated_forward = FALSE;
    updated_backward = FALSE;
    objective_updated = FALSE;

    counter_TRUE = 1;
    counter_FALSE = 0;
    counter_NOT_VISITED = 0;

    flag_reset = FALSE;

    number_disjunctives = 0;
}

void Graph::operator=(const Graph& from)
{
    int i, j;

    Reset();

    total_num_jobs = from.total_num_jobs;
    number_due_times = from.number_due_times;

    // Allocating memory:
    graph_Node = new type_p_Node[total_num_jobs];
    graph_Job_Info = new Job_Info[total_num_jobs];
    completion_time_penalty = new int[total_num_jobs];
    number_operations_job = new int[total_num_jobs];

    for (i = 0; i < total_num_jobs; i++)
    {
        // Copying the static information.
        completion_time_penalty[i] = from.completion_time_penalty[i];
        number_operations_job[i] = from.number_operations_job[i];

        graph_Job_Info[i] = from.graph_Job_Info[i];
    }
    for (i = 0; i < total_num_jobs; i++)
    {
        // Allocating memory:
        graph_Node[i] = new Node[graph_Job_Info[i].number_steps + 3];

        for (j = 0; j < graph_Job_Info[i].number_steps + 3; j++)
        {
            // Allocating memory for the due dates :
            graph_Node[i][j].Initialize(i, j, number_due_times, total_num_jobs, number_operations_job);
            graph_Node[i][j] = from.graph_Node[i][j];
        }
    }

    //      number_disjunctives = from.number_disjunctives ;

    Create_Matrix(from.number_centers_plant, from.number_operations_center);
    number_centers_plant = from.number_centers_plant;
    for (i = 0; i < number_centers_plant; i++)
    {
        number_operations_center[i] = from.number_operations_center[i];
        for (j = 0; j < number_operations_center[i]; j++)
            node_index_location[i][j] = from.node_index_location[i][j];
    }
}

int Graph::operator==(const Graph& from)
{
    int i, j, k;

    if ((objective != from.objective) || (makespan != from.makespan) ||
        (total_weighted_tardiness != from.total_weighted_tardiness) ||
        (total_weighted_completion != from.total_weighted_completion) ||
        (number_disjunctives != from.number_disjunctives))
        return (FALSE);

    for (i = 0; i < total_num_jobs; i++)
    {
        if (graph_Job_Info[i].completion_time != from.graph_Job_Info[i].completion_time) return (FALSE);
    }

    for (i = 0; i < total_num_jobs; i++)
    {
        for (j = 0; j < graph_Job_Info[i].number_steps + 3; j++)
        {
            if (!(graph_Node[i][j] == from.graph_Node[i][j])) return (FALSE);

            for (k = 0; k < number_due_times; k++)
                if (!(graph_Node[i][j].due_time[k] == from.graph_Node[i][j].due_time[k])) return (FALSE);
        }
    }
    return (TRUE);
}

void Graph::Add_Source_Sink()
{
    int i, sink_step;
    int dead_lock = FALSE;
    int makespan;

    for (i = 0; i < total_num_jobs; i++)
    {
        // The following is for the source links.
        graph_Node[i][0].release_time = 0;
        // assuming all release times are zero.
        // The release time is managed by the arc.

        Add_Conjunctive(i, 0, graph_Job_Info[i].release_time, i, 1);
        // connection between the source-step and
        // the first  step.

        // The following is for the sink links.
        sink_step = graph_Job_Info[i].number_steps + 2;
        // graph_Node[i][sink_step].array_sucessor[i] = 1;
        // every final node preceeds itself.
        // This is for the calculation of the penalty.

        Add_Conjunctive(i, (sink_step - 1), 0, i, sink_step);
        // connection between the sink-step and
        // the last step.
    }

    Get_Forward_Windows(&dead_lock);

    switch (par_objective)
    {
        case MAKESPAN:
            makespan = Get_Makespan();
            for (i = 0; i < total_num_jobs; i++)
            {
                sink_step = graph_Job_Info[i].number_steps + 2;
                graph_Node[i][sink_step].due_time[0].Set(makespan);
            }
            break;

        case TARDINESS:
            for (i = 0; i < total_num_jobs; i++)
            {
                sink_step = graph_Job_Info[i].number_steps + 2;
                graph_Node[i][sink_step].due_time[i].Set(
                    max2(graph_Job_Info[i].due_date, graph_Job_Info[i].completion_time));
                // the due time of the sink
                // is the completion time.
                // This is when minimizing completion time.
            }
            break;

        case COMPLETION:
            for (i = 0; i < total_num_jobs; i++)
            {
                sink_step = graph_Job_Info[i].number_steps + 2;
                graph_Node[i][sink_step].due_time[i].Set(graph_Job_Info[i].completion_time);
                // the due time of the sink
                // is the completion time.
                // This is when minimizing tardiness
            }
            break;
    }
}

void Graph::Initialize()
{
    Reset();

    Add_Source_Sink();
    Set_Min_Release();
    // Sets the minimum time an operation can be started.
}

// Create_Matrix() ; Creates the matrix containing the status SELECTED, FORBIDDEN, REDUNDANT, BLANK or
// DIAGONAL.
void Graph::Create_Matrix(int input_number_centers_plant, int* input_number_operations_center)
{
    int i, j, k;

    number_centers_plant = input_number_centers_plant;
    number_blanks_graph = 0;
    number_operations_center = new int[number_centers_plant];
    number_blanks_center = new int[number_centers_plant];
    node_index_location = new type_p_Class_Node_Index[number_centers_plant];
    num_disj_center_oper = new type_p_p_int[number_centers_plant];
    matrix = new type_p_p_int[number_centers_plant];
    for (i = 0; i < number_centers_plant; i++)
    {
        number_operations_center[i] = input_number_operations_center[i];
        number_blanks_center[i] = number_operations_center[i] * (number_operations_center[i] - 1);
        number_blanks_graph = number_blanks_graph + number_blanks_center[i];
        node_index_location[i] = new Class_Node_Index[number_operations_center[i]];
        num_disj_center_oper[i] = new type_p_int[number_operations_center[i]];
        matrix[i] = new type_p_int[number_operations_center[i]];
        for (j = 0; j < number_operations_center[i]; j++)
        {
            num_disj_center_oper[i][j] = new int[2];
            // FORWARD and BACKWARD.

            matrix[i][j] = new int[number_operations_center[i]];

            for (k = 0; k < 2; k++)
            {
                num_disj_center_oper[i][j][k] = 0;
            }
            for (k = 0; k < number_operations_center[i]; k++)
            {
                matrix[i][j][k] = BLANK;
            }
            matrix[i][j][j] = DIAGONAL;
        }
    }
}

/*
void Graph::Back_Up_Center_Matrix(      int ***p_back_up_matrix , int *p_n_blanks_center , int
*p_n_blanks_graph , int ***p_back_up_num_disj , int *p_n_disjunctives_graph , int work_center )
{
int j , k ;

        (*p_back_up_matrix) = new type_p_int[ number_operations_center[work_center] ] ;
        (*p_back_up_num_disj) = new type_p_int[ number_operations_center[work_center] ] ;
        for( j = 0; j < number_operations_center[work_center] ; j++ ){
                (*p_back_up_matrix)[j] = new int[ number_operations_center[work_center] ] ;
                for( k = 0; k < number_operations_center[work_center] ; k++ ){
                        (*p_back_up_matrix)[j][k] = matrix[work_center][j][k] ;
                }
                (*p_back_up_num_disj)[j] = new int[ 2 ] ;
                (*p_back_up_num_disj)[j][FORWARD] = num_disj_center_oper[work_center][j][FORWARD] ;
                (*p_back_up_num_disj)[j][BACKWARD] = num_disj_center_oper[work_center][j][BACKWARD] ;
        }
        *p_n_blanks_center = number_blanks_center[work_center] ;
        *p_n_blanks_graph = number_blanks_graph ;
        *p_n_disjunctives_graph = number_disjunctives ;
}


void Graph::Back_Up_Graph_Matrix(       int ****p_p_back_up_matrix ,
                                                        int **p_p_n_blanks_center , int *p_n_blanks_graph ,
                                                        int ****p_p_back_up_num_disj , int
*p_n_disjunctives_graph )
{
        (*p_p_back_up_matrix) = new type_p_p_int[number_centers_plant] ;
        (*p_p_n_blanks_center) = new int[number_centers_plant] ;
        (*p_p_back_up_num_disj) = new type_p_p_int[number_centers_plant] ;
        for( int i = 0 ; i < number_centers_plant ; i++ )
                Back_Up_Center_Matrix(  &((*p_p_back_up_matrix)[i]) , &((*p_p_n_blanks_center)[i]) ,
p_n_blanks_graph ,
                                                &((*p_p_back_up_num_disj)[i]) , p_n_disjunctives_graph , i ) ;
}


void Graph::Check_Back_Up_Graph_Matrix( )
{
int i , j , k;
int n_blanks = 0 ;
int n_blanks_i ;
        for( i = 0; i < number_centers_plant ; i++ ) {
                n_blanks_i = 0 ;
                for( j = 0; j < number_operations_center[i] ; j++ ) {
                        for( k = 0; k < number_operations_center[i] ; k++ ) {
                                if ( matrix[i][j][k] == BLANK )
                                        n_blanks_i++ ;
                        }
                }
                if( n_blanks_i != number_blanks_center[i] )
                        assert( n_blanks_i == number_blanks_center[i] ) ;
                n_blanks = n_blanks + n_blanks_i ;
        }
        assert( n_blanks == number_blanks_graph ) ;
}


void Graph::Restore_Center_Matrix(      int **back_up_matrix , int n_blanks_center , int n_blanks_graph ,
                                                int **back_up_num_disj , int input_number_disjunctives ,
                                                int work_center )
{
int j , k ;
                                        #ifdef CHECK_DEBUG
                                                assert( work_center < number_centers_plant ) ;
                                        #endif

        for( j = 0; j < number_operations_center[work_center] ; j++ ){
                for( k = 0; k < number_operations_center[work_center] ; k++ ){
                        matrix[work_center][j][k] = back_up_matrix[j][k] ;
                }
                delete[] back_up_matrix[j] ;

                num_disj_center_oper[work_center][j][FORWARD] = back_up_num_disj[j][FORWARD] ;
                num_disj_center_oper[work_center][j][BACKWARD] = back_up_num_disj[j][BACKWARD] ;
                delete[] back_up_num_disj[j] ;
        }
        delete[] back_up_matrix ;
        delete[] back_up_num_disj ;
        number_blanks_center[work_center] = n_blanks_center ;
        number_blanks_graph = n_blanks_graph ;
        number_disjunctives = input_number_disjunctives ;
}
*/

void Graph::Restore_Arcs(int number_arcs_list, Arc_Backup* list)
{
    for (int i = number_arcs_list - 1; i >= 0; i--)
    {
        if (list[i].value == BLANK)
        {
            if (matrix[list[i].center][list[i].from][list[i].to] == SELECTED)
            {
                num_disj_center_oper[list[i].center][list[i].from][FORWARD]--;
                num_disj_center_oper[list[i].center][list[i].to][BACKWARD]--;
            }
            number_blanks_center[list[i].center]++;
            number_blanks_graph++;
        }
        else if (list[i].value == SELECTED)
        {
#ifdef CHECK_DEBUG
            assert(matrix[list[i].center][list[i].from][list[i].to] == REDUNDANT);
#endif
            num_disj_center_oper[list[i].center][list[i].from][FORWARD]++;
            num_disj_center_oper[list[i].center][list[i].to][BACKWARD]++;
            number_disjunctives++;
        }
        if (matrix[list[i].center][list[i].from][list[i].to] == SELECTED)
        {
#ifdef CHECK_DEBUG
            assert(list[i].value == REDUNDANT || list[i].value == BLANK);
#endif
            number_disjunctives--;
        }
        matrix[list[i].center][list[i].from][list[i].to] = list[i].value;
    }
    delete[] list;
}

// Used in the reoptimization. It deletes all the arcs of a center.
void Graph::Delete_Center_Matrix(int work_center)
{
    int j, k;
#ifdef CHECK_DEBUG
    assert(work_center < number_centers_plant);
    int this_number_disjunctives = 0;
    int this_number_blanks = 0;
    int initial_disjunctives = number_disjunctives;
#endif

    for (j = 0; j < number_operations_center[work_center]; j++)
    {
        for (k = 0; k < number_operations_center[work_center]; k++)
        {
#ifdef CHECK_DEBUG
            if (matrix[work_center][j][k] == SELECTED) this_number_disjunctives++;
            if (matrix[work_center][j][k] == BLANK) this_number_blanks++;
            assert(matrix[work_center][j][k] != BLK_FIRST);
#endif

            matrix[work_center][j][k] = BLANK;
        }
        matrix[work_center][j][j] = DIAGONAL;

        number_disjunctives = number_disjunctives - num_disj_center_oper[work_center][j][FORWARD];
        num_disj_center_oper[work_center][j][FORWARD] = 0;
        num_disj_center_oper[work_center][j][BACKWARD] = 0;
    }

    int number_entries = number_operations_center[work_center] * (number_operations_center[work_center] - 1);

#ifdef CHECK_DEBUG
    assert(0 <= number_disjunctives);
    assert(this_number_blanks <= number_entries);
    assert(this_number_blanks == number_blanks_center[work_center]);
    assert(number_disjunctives == initial_disjunctives - this_number_disjunctives);
#endif

    number_blanks_graph = number_blanks_graph + (number_entries - number_blanks_center[work_center]);
    number_blanks_center[work_center] = number_entries;
}

/*
void Graph::Restore_Graph_Matrix(       int ***p_back_up_matrix , int *p_n_blanks_center , int n_blanks_graph
, int ***p_back_up_num_disj , int n_disjunctives_graph  )
{
        for( int i = 0 ; i < number_centers_plant ; i++ )
                Restore_Center_Matrix(  p_back_up_matrix[i] , p_n_blanks_center[i] , n_blanks_graph ,
                                                        p_back_up_num_disj[i] , n_disjunctives_graph , i ) ;
        delete[] p_back_up_matrix ;
        delete[] p_n_blanks_center ;
        delete[] p_back_up_num_disj ;
}
*/

void Graph::Terminate()
{
    int i, j;

    // De-allocating memory:
    for (i = 0; i < total_num_jobs; i++)
    {
        if (graph_Node[i] != NULL)
        {
            for (j = 0; j < (graph_Job_Info[i].number_steps + 3); j++)
            {
                graph_Node[i][j].Terminate();
            }
            delete[] graph_Node[i];
        }
    }
    if (graph_Node != NULL) delete[] graph_Node;

    if (graph_Job_Info != NULL) delete[] graph_Job_Info;
    if (completion_time_penalty != NULL) delete[] completion_time_penalty;
    if (number_operations_job != NULL) delete[] number_operations_job;

    for (i = 0; i < number_centers_plant; i++)
    {
        for (j = 0; j < number_operations_center[i]; j++)
        {
            delete[] num_disj_center_oper[i][j];
            delete[] matrix[i][j];
        }
        delete[] node_index_location[i];
        delete[] num_disj_center_oper[i];
        delete[] matrix[i];
    }
    delete[] number_operations_center;
    delete[] number_blanks_center;
    delete[] node_index_location;
    delete[] num_disj_center_oper;
    delete[] matrix;
}
/*
                Add_Conjunctive(i , 0 , graph_Job_Info[i].release_time , i , 1 );
                                //connection between the source-step and
                                //the first  step.

                //The following is for the sink links.
                sink_step = graph_Job_Info[i].number_steps + 2;
                                //graph_Node[i][sink_step].array_sucessor[i] = 1;
                                //every final node preceeds itself.
                                //This is for the calculation of the penalty.

                Add_Conjunctive(i , (sink_step - 1) , 0 , i , sink_step);
                                //connection between the sink-step and
                                //the last step.

*/

void Graph::Add_Conjunctive(int job_from, int step_from, int length, int job_to, int step_to)
{
    updated_forward = FALSE;
    updated_backward = FALSE;
    objective_updated = FALSE;

    graph_Node[job_from][step_from].conjunctive[FORWARD].Create(job_to, step_to, length);

    graph_Node[job_to][step_to].conjunctive[BACKWARD].Create(job_from, step_from, -length);
    // The length is negative so when running backward
    // it can be added.
}

// Augusto 17.10.96
//   Modifica la longitud de un arco conjunctivo
void Graph::Set_Length_Conjuntive(int job_to, int step_to, int new_length)
{
    //  Consultar
    //        updated_forward = FALSE ;
    //        updated_backward = FALSE ;
    //        objective_updated = FALSE ;

    graph_Node[job_to][step_to].conjunctive[FORWARD].Modify(job_to, step_to + 1, new_length);

    graph_Node[job_to][step_to + 1].conjunctive[BACKWARD].Modify(job_to, step_to, -new_length);
    // The length is negative so when running backward
    // it can be added.
}

// Returns TRUE if the node has a conjunctive arc.
int Graph::There_Is_Conj_Node(Node** p_p_conj_Node, Node* p_current_Node, int direction)
{
    int next_job, its_step;

    if (p_current_Node->conjunctive[direction].Exist() == FALSE)
    {
        *p_p_conj_Node = NULL;
        return (FALSE);
    }
    else
    {
        next_job = p_current_Node->conjunctive[direction].Job();
        its_step = p_current_Node->conjunctive[direction].Step();
        *p_p_conj_Node = &(graph_Node[next_job][its_step]);
        return (TRUE);
    }
}

int Graph::Add_Disjunctive_New(
    int* p_n_arcs_list, Arc_Backup* list, int job_from, int step_from, int job_to, int step_to, int flag_temp)
{
#ifdef CHECK_DEBUG
    assert(job_from >= 0 && job_from < total_num_jobs);
    assert(job_to >= 0 && job_to < total_num_jobs);
    assert(step_from >= 1 && step_from <= graph_Job_Info[job_from].number_steps);
    assert(step_to >= 1 && step_to <= graph_Job_Info[job_to].number_steps);
#endif

    return (Insert_Arc(p_n_arcs_list, list, graph_Node[job_from][step_from].work_center,
        graph_Node[job_from][step_from].location_center, graph_Node[job_to][step_to].location_center));
}

// Insert_Arc() : inserts a->b
int Graph::Insert_Arc(int center, int a, int b)
{
#ifdef CHECK_DEBUG
    assert(a != b);
    assert(a >= 0 && a < number_operations_center[center]);
    assert(b >= 0 && b < number_operations_center[center]);
    assert(matrix[center][a][b] == BLANK);
#endif

    // Otherwise insert the arc.
    updated_forward = FALSE;
    updated_backward = FALSE;
    objective_updated = FALSE;

    matrix[center][a][b] = SELECTED;
    matrix[center][b][a] = FORBIDDEN;

    (num_disj_center_oper[center][a][FORWARD])++;
    (num_disj_center_oper[center][b][BACKWARD])++;
    number_disjunctives++;

    number_blanks_center[center] = number_blanks_center[center] - 2;
    number_blanks_graph = number_blanks_graph - 2;

    return (FALSE);
    // No dead_lock.
}

// Insert_Arc() : inserts a->b
int Graph::Insert_Arc(int* p_n_arcs_list,
    Arc_Backup* list,
    // list keeps a backup of the value of an arc before replacing it.

    int center,
    int a,
    int b)
{
#ifdef CHECK_DEBUG
    assert(a != b);
    assert(a >= 0 && a < number_operations_center[center]);
    assert(b >= 0 && b < number_operations_center[center]);
#endif

    if (matrix[center][a][b] == FORBIDDEN) return (TRUE);
    // There is a dead_lock.

    if (matrix[center][a][b] != BLANK) return (FALSE);
    // There is nothing to do since the arc is either SELECTED or REDUNDANT.

    // Otherwise insert the arc.
    updated_forward = FALSE;
    updated_backward = FALSE;
    objective_updated = FALSE;

    //      int flag = matrix[center][a][b] ;

    // There is insertion of temporary arcs:

    int j, k;
    for (j = 0; j < number_operations_center[center]; j++)
    {
        // Predecessors of job a.

        if ((matrix[center][j][a] == SELECTED) || (matrix[center][j][a] == REDUNDANT) ||
            (matrix[center][j][a] == DIAGONAL)
            // j may take the value a.
        )
        {
            // There is a precedence j->a.

            for (k = 0; k < number_operations_center[center]; k++)
            {
                // Successors of job b.

                if ((matrix[center][b][k] == SELECTED) || (matrix[center][b][k] == REDUNDANT) ||
                    (matrix[center][b][k] == DIAGONAL)
                    // k may take the value b.
                )
                {
                    // There is a precedence b->k.

#ifdef CHECK_DEBUG
                    assert(matrix[center][j][k] != FORBIDDEN);
#endif

                    if (j != a || k != b)
                    {
                        if (matrix[center][j][k] == SELECTED)
                        {
                            // The arc a->k becomes redundant because of j->a->b->k.

                            list[*p_n_arcs_list].Insert(center, j, k, SELECTED);
                            (*p_n_arcs_list)++;

                            matrix[center][j][k] = REDUNDANT;

                            (num_disj_center_oper[center][j][FORWARD])--;
                            (num_disj_center_oper[center][k][BACKWARD])--;
                            number_disjunctives--;

#ifdef CHECK_DEBUG
                            assert(num_disj_center_oper[center][j][FORWARD] >= 0);
                            assert(num_disj_center_oper[center][k][BACKWARD] >= 0);
#endif
                        }
                        else if (matrix[center][j][k] == BLANK)
                        {
                            list[*p_n_arcs_list].Insert(center, j, k, BLANK);
                            (*p_n_arcs_list)++;
                            list[*p_n_arcs_list].Insert(center, k, j, BLANK);
                            (*p_n_arcs_list)++;

                            matrix[center][j][k] = REDUNDANT;
                            matrix[center][k][j] = FORBIDDEN;
                            number_blanks_center[center] = number_blanks_center[center] - 2;
                            number_blanks_graph = number_blanks_graph - 2;
                        }
                    }
                }
            }
        }
    }

    // After the TEMP part in order to overwrite matrix[center][a][b] = REDUNDANT ;
#ifdef CHECK_DEBUG
    assert(matrix[center][a][b] == BLANK);
#endif
    list[*p_n_arcs_list].Insert(center, a, b, matrix[center][a][b]);
    (*p_n_arcs_list)++;
    list[*p_n_arcs_list].Insert(center, b, a, matrix[center][b][a]);
    (*p_n_arcs_list)++;

#ifdef CHECK_DEBUG
    assert(*p_n_arcs_list < MAX_ARCS_BACKUP);
#endif

    matrix[center][a][b] = SELECTED;
    matrix[center][b][a] = FORBIDDEN;

    number_blanks_center[center] = number_blanks_center[center] - 2;
    number_blanks_graph = number_blanks_graph - 2;

    (num_disj_center_oper[center][a][FORWARD])++;
    (num_disj_center_oper[center][b][BACKWARD])++;
    number_disjunctives++;

    return (FALSE);
    // No dead_lock.
}

// Predecessors_Are_Scheduled(): returns TRUE if the immediate machine predecessr is scheduled.
int Graph::Predecessors_Are_Scheduled(int center, int a)
{
    int j;
#ifdef CHECK_DEBUG
    assert(a >= 0 && a < number_operations_center[center]);
#endif

    if (num_disj_center_oper[center][a][BACKWARD] == 0)
    {
        // num_disj_center_oper[a][b][BACKWARD]==2 meas that opeation b in center
        // a has 2 arcs pointing to it.
        return (TRUE);
    }
    else
    {
        for (j = 0; j < number_operations_center[center]; j++)
        {
            if (matrix[center][j][a] == SELECTED)
            {
                if (!graph_Node[node_index_location[center][j].job][node_index_location[center][j].step]
                         .is_scheduled)
                    return (FALSE);
            }
        }
        return (TRUE);
    }
}

int Graph::Number_Predecesors(Node* p_Node, int direction)
{
    if (p_Node->work_center != NULL_WORK_CENTER)
        return (p_Node->conjunctive[!direction].Exist() +
                num_disj_center_oper[p_Node->work_center][p_Node->location_center][!direction]);
    else
        return (p_Node->conjunctive[!direction].Exist());
}

void Graph::Add_New_Disjunctives(Class_List_Disjunctives* p_list_disj)
{
    int i, number_disjunctives_list;
    int dead_lock;
    int job_from, step_from, job_to, step_to;

    number_disjunctives_list = p_list_disj->Get_Number_Disjunctives();
    for (i = 0; i < number_disjunctives_list; i++)
    {
        //              dead_lock = Add_Disjunctive_New(        p_list_disj->Get_Nth_Job_From(i) ,
        //                                                                      p_list_disj->Get_Nth_Step_From(i)
        //                                                                      ,
        //                                                                      p_list_disj->Get_Nth_Job_To(i)
        //                                                                      ,
        //                                                                      p_list_disj->Get_Nth_Step_To(i)
        //                                                                      );
        job_from = p_list_disj->Get_Nth_Job_From(i);
        step_from = p_list_disj->Get_Nth_Step_From(i);
        job_to = p_list_disj->Get_Nth_Job_To(i);
        step_to = p_list_disj->Get_Nth_Step_To(i);
        dead_lock = Insert_Arc(graph_Node[job_from][step_from].work_center,
            graph_Node[job_from][step_from].location_center, graph_Node[job_to][step_to].location_center);

#ifdef CHECK_DEBUG
        assert(dead_lock == FALSE);
#endif
    }
}

void Graph::Initialize_Solution(Solution_Graph* p_Solution_Graph)
{
    /*
    int *number_steps = new int[ total_num_jobs ] ;


                      for( int i = 0 ; i < total_num_jobs ; i++ ) {
                                             number_steps[i] = graph_Job_Info[i].number_steps ;
                      }
                      p_Solution_Graph->Initialize( total_num_jobs , number_steps ) ;
                      delete[] number_steps ;
    */
    p_Solution_Graph->Initialize(total_num_jobs, number_operations_job);
}

void Graph::Set_Blank_First(Solution_Graph* p_Solution_Graph)
{
    int i;
    int center;
    int job_from, step_from, job_to, step_to;
    int a, b;

    int number_disjunctives_solution = p_Solution_Graph->Get_Number_Disjunctives();
    for (i = 0; i < min2(0, number_disjunctives_solution); i++)
    {
        job_from = p_Solution_Graph->Get_Nth_Job_From(i);
        step_from = p_Solution_Graph->Get_Nth_Step_From(i);
        job_to = p_Solution_Graph->Get_Nth_Job_To(i);
        step_to = p_Solution_Graph->Get_Nth_Step_To(i);
        center = graph_Node[job_from][step_from].work_center;
#ifdef CHECK_DEBUG
        assert(center == graph_Node[job_to][step_to].work_center);
#endif
        a = graph_Node[job_from][step_from].location_center;
        b = graph_Node[job_to][step_to].location_center;
        matrix[center][a][b] = BLK_FIRST;
        matrix[center][b][a] = BLK_FIRST;
    }
}

void Graph::Write_Solution(Solution_Graph* p_Solution_Graph)
{
    int i, j, k;
    int center;

    p_Solution_Graph->Delete();

#ifdef CHECK_DEBUG
    int n_forward = 0;
    int n_backward = 0;
    for (i = 0; i < number_centers_plant; i++)
    {
        for (j = 0; j < number_operations_center[i]; j++)
        {
            n_forward = n_forward + num_disj_center_oper[i][j][FORWARD];
            n_backward = n_backward + num_disj_center_oper[i][j][BACKWARD];
        }
    }
    assert(n_forward == number_disjunctives);
    assert(n_backward == number_disjunctives);
#endif
    p_Solution_Graph->Prepare_Write(number_disjunctives);
    p_Solution_Graph->Set_Objective(objective);

    for (i = 0; i < total_num_jobs; i++)
    {
        p_Solution_Graph->Write_Completion_Time(i, graph_Job_Info[i].completion_time);
        for (j = 1; j <= graph_Job_Info[i].number_steps; j++)
        {
            p_Solution_Graph->Set_Machine(graph_Node[i][j].machine, i, j);
            center = graph_Node[i][j].work_center;
            for (k = 0; k < number_operations_center[center]; k++)
            {
                if (matrix[center][graph_Node[i][j].location_center][k] == SELECTED)
                {
                    p_Solution_Graph->Add_Arc(graph_Node[i][j].position.job, graph_Node[i][j].position.step,
                        node_index_location[center][k].job, node_index_location[center][k].step);
                }
            }
        }
    }
#ifdef CHECK_DEBUG
    assert(p_Solution_Graph->Get_Number_Disjunctives() == number_disjunctives);
#endif
}

void Graph::Read_Machines(Solution_Graph* p_Solution_Graph)
{
    int i, j;

    for (i = 0; i < total_num_jobs; i++)
        for (j = 1; j <= graph_Job_Info[i].number_steps; j++)
        {
            graph_Node[i][j].machine = p_Solution_Graph->Get_Machine(i, j);
            graph_Node[i][j].is_scheduled = TRUE;
        }
}

void Graph::Read_Arcs(Solution_Graph* p_Solution_Graph)
{
    int i;
    int dead_lock;
    int job_from, step_from, job_to, step_to;

    int number_disjunctives_solution = p_Solution_Graph->Get_Number_Disjunctives();
    for (i = 0; i < number_disjunctives_solution; i++)
    {
        //              dead_lock = Add_Disjunctive_New(        p_Solution_Graph->Get_Nth_Job_From(i) ,
        //                                                                      p_Solution_Graph->Get_Nth_Step_From(i)
        //                                                                      ,
        //                                                                      p_Solution_Graph->Get_Nth_Job_To(i)
        //                                                                      ,
        //                                                                      p_Solution_Graph->Get_Nth_Step_To(i)
        //                                                                      , TEMP );

        job_from = p_Solution_Graph->Get_Nth_Job_From(i);
        step_from = p_Solution_Graph->Get_Nth_Step_From(i);
        job_to = p_Solution_Graph->Get_Nth_Job_To(i);
        step_to = p_Solution_Graph->Get_Nth_Step_To(i);
        dead_lock = Insert_Arc(graph_Node[job_from][step_from].work_center,
            graph_Node[job_from][step_from].location_center, graph_Node[job_to][step_to].location_center);

#ifdef CHECK_DEBUG
        assert(dead_lock == FALSE);
#endif
    }
}

void Graph::Show_Gantt_Chart()
{
    int dead_lock = FALSE;

    Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif
    /*
    for( int i = 0 ; i < total_num_jobs ; i++ ) {
            for( int j = 1 ; j <= graph_Job_Info[i].number_steps ; j++ ) {
                    fprintf( par_output_file ,"job %d, step %d, work center %d, machine %d, start %d, length
    %d ; ", i,j, graph_Node[i][j].work_center, graph_Node[i][j].machine, graph_Node[i][j].release_time,
                            graph_Node[i][j].conjunctive[FORWARD].Length());
                    fprintf( par_output_file ,"\n");
            };
    };
    fprintf( par_output_file ,"\n");
    */
}

int Graph::Few_Blanks_Centers(int threshold)
{
    for (int i = 0; i < number_centers_plant; i++)
    {
        if (number_blanks_center[i] > threshold)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}
int Graph::Max_Branks_Center()
{
    int max_blanks = INT_MIN;

    for (int i = 0; i < number_centers_plant; i++)
    {
        if (number_blanks_center[i] > max_blanks)
        {
            max_blanks = number_blanks_center[i];
        }
    }
#ifdef CHECK_DEBUG
    assert(max_blanks > 1);
#endif
    return (max_blanks);
}

// Print_Bottleneck_Index(): Implemented for rectangular problems.
double Graph::Print_Bottleneck_Index()
{
    int i, k;

    int number_steps = graph_Job_Info[0].number_steps;
    for (i = 1; i < total_num_jobs; i++)
        assert(number_steps == graph_Job_Info[i].number_steps ||
               number_steps == graph_Job_Info[i].number_steps + 1 ||
               number_steps == graph_Job_Info[i].number_steps - 1);

    int** number_visits = new type_p_int[number_centers_plant];
    // number_visits[2][4] = 5 means that in position 2 machine 4 is 5 times

    for (i = 0; i < number_centers_plant; i++)
    {
        number_visits[i] = new int[number_steps + 1];
        for (k = 0; k <= number_steps; k++) number_visits[i][k] = 0;
    }
    for (i = 0; i < total_num_jobs; i++)
        for (k = 1; k <= graph_Job_Info[i].number_steps; k++)
            number_visits[graph_Node[i][k].work_center][k]++;

    double sum = 0;
    for (i = 0; i < number_centers_plant; i++)
        for (k = 1; k <= number_steps; k++)
            sum += pos(number_visits[i][k] - 1) / double(number_centers_plant - 1);

    //      printf("%lf ; ",  sum / number_centers_plant ) ;
    for (i = 0; i < number_centers_plant; i++) delete[] number_visits[i];
    delete[] number_visits;
    return (sum / number_centers_plant);
}

double Graph::Print_Route_Index()
{
    int i, k;

    int number_steps = graph_Job_Info[0].number_steps;
    int** number_transitions = new type_p_int[number_centers_plant];
    for (i = 0; i < number_centers_plant; i++)
    {
        number_transitions[i] = new int[number_centers_plant];
        for (k = 0; k < number_centers_plant; k++) number_transitions[i][k] = 0;
    }
    for (i = 0; i < total_num_jobs; i++)
        for (k = 1; k < graph_Job_Info[i].number_steps; k++)
            number_transitions[graph_Node[i][k].work_center][graph_Node[i][k + 1].work_center]++;

    double sum = 0;
    for (i = 0; i < number_centers_plant; i++)
        for (k = 0; k < number_centers_plant; k++)
            sum += pos(number_transitions[i][k] - 1) / double(number_steps - 1);

    //      printf("%lf ; ",  sum / number_centers_plant ) ;
    for (i = 0; i < number_centers_plant; i++) delete[] number_transitions[i];
    delete[] number_transitions;
    return (sum / (number_centers_plant - 1));
}
/*
double Graph::Print_Route_Index()
{
int i , k ;
int n_i_p ;
int total_number_operations = 0 ;
double phi_i ;
double sum_n_phi = 0 ;

        int **number_transitions = new type_p_int[ number_centers_plant ] ;
        for( i = 0 ; i < number_centers_plant ; i++ ) {
                number_transitions[i] = new int[ number_centers_plant ] ;
                for( k = 0 ; k < number_centers_plant ; k++ )
                        number_transitions[i][k] = 0 ;
        }
        for( i = 0 ; i < total_num_jobs ; i++ ) {
                for( k = 1 ; k < graph_Job_Info[i].number_steps ; k++ ) {
                                        #ifdef CHECK_DEBUG
                                                assert( graph_Node[i][k].work_center >= 0 ) ;
                                                assert( graph_Node[i][k].work_center < number_centers_plant )
; assert( graph_Node[i][k+1].work_center >= 0 ) ; assert( graph_Node[i][k+1].work_center <
number_centers_plant ) ; #endif
                        number_transitions[graph_Node[i][k].work_center][graph_Node[i][k+1].work_center]++;
                };
        };
        for( i = 0 ; i < number_centers_plant ; i++ ) {
                n_i_p = 0 ;
                phi_i = 0 ;
                for( k = 0 ; k < number_centers_plant ; k++ )
                        n_i_p = n_i_p + pos( number_transitions[i][k] - 1 ) ;
                if ( n_i_p == 0 )
                        phi_i = 1 ;
                else
                        for( k = 0 ; k < number_centers_plant ; k++ )
//                              phi_i += double(pos( number_transitions[i][k] - 1 ) / n_i_p) ;
                                phi_i += double(pos( number_transitions[i][k]-1) /
(number_operations_center[i]-1)) ; sum_n_phi = sum_n_phi + number_operations_center[i] * phi_i ;
                total_number_operations = total_number_operations + number_operations_center[i] ;
        }
//      printf("%lf ; ",  sum_n_phi / total_number_operations ) ;
        for( i = 0 ; i < number_centers_plant ; i++ )
                delete[] number_transitions[i] ;
        delete[] number_transitions ;
        return( sum_n_phi / total_number_operations ) ;
}
*/
int Graph::Is_Optimal_Predecessor()
{
    static int forbidden[6][6][6] = {{{0, 1, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 1},
                                         {0, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0}, {1, 1, 0, 1, 1, 0}},
        {{0, 0, 0, 1, 0, 0}, {1, 0, 0, 1, 1, 0}, {1, 1, 0, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0},
            {1, 1, 1, 1, 1, 0}},
        {{0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 1}, {1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 0, 1},
            {1, 1, 0, 1, 0, 0}},
        {{0, 1, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0}, {0, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 0, 0},
            {1, 1, 1, 1, 1, 0}},
        {{0, 1, 0, 0, 0, 1}, {0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 1}, {1, 1, 0, 0, 0, 1}, {1, 1, 1, 1, 0, 1},
            {0, 1, 0, 0, 0, 0}},
        {{0, 1, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 1}, {0, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0},
            {1, 1, 0, 1, 1, 0}}};

    int i, j, k;
    for (i = 0; i < number_centers_plant; i++)
    {
        for (j = 0; j < number_operations_center[i]; j++)
        {
            for (k = 0; k < number_operations_center[i]; k++)
            {
                if ((forbidden[i][j][k] == 1) &&
                    ((matrix[i][j][k] == SELECTED) || (matrix[i][j][k] == REDUNDANT)))
                    return (FALSE);
            }
        }
    }
    return (TRUE);
}

void Graph::FPrint_Matrix()
{
    int i, j, k;
    FILE* this_file = fopen("matrix", "w");

    fprintf(this_file, "{");
    for (i = 0; i < number_centers_plant; i++)
    {
        fprintf(this_file, "{");
        for (j = 0; j < number_operations_center[i]; j++)
        {
            fprintf(this_file, "{");
            for (k = 0; k < number_operations_center[i] - 1; k++)
            {
                if (matrix[i][j][k] == FORBIDDEN)
                    fprintf(this_file, " 1 ,");
                else
                    fprintf(this_file, " 0 ,");
            }
            if (matrix[i][j][k] == FORBIDDEN)
                fprintf(this_file, " 1 } ,\n");
            else
                fprintf(this_file, " 0 } ,\n");
        }

        fprintf(this_file, "} ,\n");
    }
    fprintf(this_file, "} ,\n");
    fclose(this_file);
}

// Valid_Answer_Cent(): checks whether a the solution of a center is a tour.
int Graph::Valid_Answer_Cent(int center_id)
{
    int i;
    int number_selected;
    if (number_blanks_center[center_id] == 0)
    {
        // There are no blnaks so the matrix was filled  step by step.

#ifdef CHECK_DEBUG
        number_selected = 0;
        for (i = 0; i < number_operations_center[center_id]; i++)
        {
            assert(num_disj_center_oper[center_id][i][FORWARD] <= 1);
            number_selected = number_selected + num_disj_center_oper[center_id][i][FORWARD];
        }
        assert(number_selected < number_operations_center[center_id]);
        assert(number_selected == (number_operations_center[center_id] - 1));
#endif

        return (TRUE);
    }
    else
    {  // The answer was given externally.
        number_selected = 0;
        for (i = 0; i < number_operations_center[center_id]; i++)
        {
            if (num_disj_center_oper[center_id][i][FORWARD] > 1)
                // No node can have to outgoing arcs.

                return (FALSE);
            number_selected = number_selected + num_disj_center_oper[center_id][i][FORWARD];
        }
#ifdef CHECK_DEBUG
        assert(number_selected < number_operations_center[center_id]);
#endif
        if (number_selected == (number_operations_center[center_id] - 1))
            return (TRUE);
        else
            return (FALSE);
    }
}

int* Graph::Backup_Change_Due_Dates()
{
    int* backup = new int[total_num_jobs];

    for (int i = 0; i < total_num_jobs; i++)
    {
        backup[i] = graph_Job_Info[i].due_date;
        graph_Job_Info[i].due_date = (int)(0.9 * graph_Job_Info[i].due_date);
    }
    return (backup);
}

void Graph::Restore_Due_Dates(int* backup)
{
    for (int i = 0; i < total_num_jobs; i++)
    {
        graph_Job_Info[i].due_date = backup[i];
    }
    delete[] backup;
}
