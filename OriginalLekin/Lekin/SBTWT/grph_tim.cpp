#include "StdAfx.h"

#include "graph.h"

int this_par_delay_constraints;  // TRUE if calculating delayed precedence constraints.

// Sets the minimum time an operation can be started.
void Graph::Set_Min_Release()
{
    int i, j;

    for (i = 0; i < total_num_jobs; i++)
        for (j = 1; j < (graph_Job_Info[i].number_steps + 1); j++)
            graph_Node[i][j].min_release = graph_Node[i][j].release_time;
}

// Set_Time_Window(): sets release time and due times when the node has one predecessor.
void Graph::Set_Time_Window(Node* p_scanned_Node, Node* p_visitor_Node, int length_arc, int direction)
{
    int i, j;

    if (direction == FORWARD)
    {
        p_scanned_Node->release_time = p_visitor_Node->release_time + length_arc;

        /*              if (    ( ( p_visitor_Node->is_scheduled != TRUE ) && ( p_visitor_Node->position.step
           > 0 ) )
                                ||
                                ( p_visitor_Node->is_movable == counter_TRUE )
                                ||
                                ( p_scanned_Node->is_scheduled != TRUE )
                        )
                                p_scanned_Node->is_movable = counter_TRUE ;
        */
    }
    else
    {
        for (i = 0; i < number_due_times; i++)
        {
            if (p_visitor_Node->due_time[i].Exist())
            {
                p_scanned_Node->due_time[i].Set(p_visitor_Node->due_time[i].Time() + length_arc);
            }
            else
            {
                p_scanned_Node->due_time[i].Delete();
            }
        }
        if (this_par_delay_constraints &&
            (p_visitor_Node->position.step <= number_operations_job[p_visitor_Node->position.job] + 1) &&
            (p_scanned_Node->position.step >= 1))
        {  // If this is an actual operation
            for (i = 0; i < total_num_jobs; i++)
            {
                for (j = 1; j <= number_operations_job[i]; j++)
                {
                    if (p_visitor_Node->delay[i][j] != -1)
                    {
                        p_scanned_Node->delay[i][j] = p_visitor_Node->delay[i][j] - length_arc;
#ifdef CHECK_DEBUG
                        assert(p_scanned_Node->delay[i][j] >= 0);
#endif
                        // Going backwards the length is negative.
                    }
                    else
                    {
                        p_scanned_Node->delay[i][j] = -1;
                    }
                }
            }
            p_scanned_Node->delay[p_scanned_Node->position.job][p_scanned_Node->position.step] = 0;
        }
    }
}

// Choose_Set_Time_Window(): sets release time and due times when the node has two predecessors.
void Graph::Choose_Set_Time_Window(Node* p_scanned_Node, Node* p_visitor_Node, int length_arc, int direction)
{
    int i, j;
    int candidate;

    if (direction == FORWARD)
    {
        candidate = p_visitor_Node->release_time + length_arc;
        p_scanned_Node->release_time = max2(p_scanned_Node->release_time, candidate);

        /*              if (    ( ( p_visitor_Node->is_scheduled != TRUE ) && ( p_visitor_Node->position.step
           > 0 ) )
                                ||
                                ( p_visitor_Node->is_movable == counter_TRUE )
                                ||
                                ( p_scanned_Node->is_scheduled != TRUE )
                        )
                                p_scanned_Node->is_movable = counter_TRUE ;
        */
    }
    else
    {
        for (i = 0; i < number_due_times; i++)
        {
            if (p_visitor_Node->due_time[i].Exist())
            {
                candidate = p_visitor_Node->due_time[i].Time() + length_arc;
                // We add the length_arc because it is negative going backwards.
                if (p_scanned_Node->due_time[i].Exist())
                    p_scanned_Node->due_time[i].Set(min2(p_scanned_Node->due_time[i].Time(), candidate));
                else
                    p_scanned_Node->due_time[i].Set(candidate);
            }
        }
        if (this_par_delay_constraints &&
            (p_visitor_Node->position.step <= number_operations_job[p_visitor_Node->position.job] + 1) &&
            (p_scanned_Node->position.step >= 1))
        {  // If this is an actual operation

            for (i = 0; i < total_num_jobs; i++)
            {
                for (j = 1; j <= number_operations_job[i]; j++)
                {
                    if (p_visitor_Node->delay[i][j] != -1)
                    {
                        candidate = p_visitor_Node->delay[i][j] - length_arc;
                        // We substract the length_arc because it is negative going backwards.
                        if (p_scanned_Node->delay[i][j] != -1)
                        {
                            p_scanned_Node->delay[i][j] = max2(p_scanned_Node->delay[i][j], candidate);
                        }
                        else
                        {
                            p_scanned_Node->delay[i][j] = candidate;
                        }
                    }
                }
            }
            p_scanned_Node->delay[p_scanned_Node->position.job][p_scanned_Node->position.step] = 0;
        }
    }
}

void Graph::Scan_Node(Node* p_scanned_Node, Node* p_visitor_Node, int length_arc, int direction)
{
    if (Number_Predecesors(p_scanned_Node, direction) == 1)
    {
        // Only one predecessor.

        Set_Time_Window(p_scanned_Node, p_visitor_Node, length_arc, direction);
        p_scanned_Node->cleared_status[direction] = counter_TRUE;
    }
    else if (Number_Predecesors(p_scanned_Node, direction) >= 2)
    {
        // Two or more predecessors.

        if (p_scanned_Node->visit_status[direction] <= counter_NOT_VISITED)
        {
            // Not visited yet.

            Set_Time_Window(p_scanned_Node, p_visitor_Node, length_arc, direction);
            p_scanned_Node->visit_status[direction] = counter_NOT_VISITED + 1;
            // Visited once.
        }
        else if (p_scanned_Node->visit_status[direction] > counter_NOT_VISITED)
        {
            // VIsited at least once.

            Choose_Set_Time_Window(p_scanned_Node, p_visitor_Node, length_arc, direction);
            p_scanned_Node->visit_status[direction]++;

            if (p_scanned_Node->visit_status[direction] ==
                counter_NOT_VISITED +
                    Number_Predecesors(p_scanned_Node, direction))  // Visited by all its predecessors.

                p_scanned_Node->cleared_status[direction] = counter_TRUE;
        }
    }
}

// It writes the starting time for the time window. It can run either FORWARD
// or BACKWARD. When is forward it writes the actual starting time of the window. When it is
// backward is writes the ending time of the window.

void Graph::Get_Start_Windows(int* p_dead_lock, Stack<Node>* p_stack_Nodes, int direction)
{
    int i;
    Node *p_current_Node, *p_next_Node;
    int next_job, its_step;

    while (p_stack_Nodes->Empty() != 1)
    {
        p_current_Node = p_stack_Nodes->Pop();
        // Take the top of the stack

        p_current_Node->poped[direction] = counter_TRUE;
        // It has been poped out of the stack and therefore it
        // cannot be scanned unless there is a circuit.

        // Conjunctive successor.
        if (There_Is_Conj_Node(&p_next_Node, p_current_Node, direction))
        {
#ifdef CHECK_DEBUG
            assert(p_next_Node->poped[direction] < counter_TRUE);
#endif

            Scan_Node(
                p_next_Node, p_current_Node, p_current_Node->conjunctive[direction].Length(), direction);

            if (p_next_Node->cleared_status[direction] == counter_TRUE) p_stack_Nodes->Push(p_next_Node);
        }

        // Disjunctive successors.
        int center = p_current_Node->work_center;
        if (center != NULL_WORK_CENTER)
        {
            if (direction == FORWARD)
            {
                for (i = 0; i < number_operations_center[center]; i++)
                {
                    if (matrix[center][p_current_Node->location_center][i] == SELECTED)
                    {
                        next_job = node_index_location[center][i].job;
                        its_step = node_index_location[center][i].step;

                        Scan_Node(&(graph_Node[next_job][its_step]), p_current_Node,
                            p_current_Node->conjunctive[FORWARD].Length(), direction);

                        if (graph_Node[next_job][its_step].cleared_status[direction] == counter_TRUE)
                            p_stack_Nodes->Push(&(graph_Node[next_job][its_step]));
                        // since it scans first the conjunctive, if there is a
                        // disjunctive it is going to Push it last in the
                        // stack and therefore Pop it first.
                    }
                }
            }
            else
            {
                for (i = 0; i < number_operations_center[center]; i++)
                {
                    if (matrix[center][i][p_current_Node->location_center] == SELECTED)
                    {
                        next_job = node_index_location[center][i].job;
                        its_step = node_index_location[center][i].step;

                        Scan_Node(&(graph_Node[next_job][its_step]), p_current_Node,
                            -graph_Node[next_job][its_step].conjunctive[FORWARD].Length(), direction);

                        if (graph_Node[next_job][its_step].cleared_status[direction] == counter_TRUE)
                            p_stack_Nodes->Push(&(graph_Node[next_job][its_step]));
                        // since it scans first the conjunctive, if there is a
                        // disjunctive it is going to Push it last in the
                        // stack and therefore Pop it first.
                    }
                }
            }
        }
    }
    *p_dead_lock = FALSE;
    // Just quit.
}

void Graph::Initialize_Forward()
{
    int i, j;

    //      if ( counter_FALSE >= 30 - 5 ) {
    if (counter_FALSE >= INT_MAX - 2 * total_num_jobs - 5)
    {
        // Each node is initialized.

        counter_TRUE = 1;
        counter_FALSE = 0;
        counter_NOT_VISITED = 0;

        for (i = 0; i < total_num_jobs; i++)
        {
            for (j = 0; j < graph_Job_Info[i].number_steps + 3; j++)
            {
                graph_Node[i][j].visit_status[FORWARD] = counter_NOT_VISITED;
                graph_Node[i][j].cleared_status[FORWARD] = counter_FALSE;
                graph_Node[i][j].poped[FORWARD] = counter_FALSE;
                // No node has been poped yet.
            }
        }
        flag_reset = TRUE;
    }
    else
    {  // All the TRUE values become FALSE.

        counter_TRUE = counter_TRUE + total_num_jobs + 2;
        counter_FALSE = counter_FALSE + total_num_jobs + 2;
        counter_NOT_VISITED = counter_NOT_VISITED + total_num_jobs + 2;
    }
}
void Graph::Increase_Counter()
{
    counter_TRUE = counter_TRUE + total_num_jobs + 2;
    counter_FALSE = counter_FALSE + total_num_jobs + 2;
    counter_NOT_VISITED = counter_NOT_VISITED + total_num_jobs + 2;
}

// void Graph::Get_Forward_Windows( int *p_dead_lock , int **input_operation_cluster )
void Graph::Get_Forward_Windows(int* p_dead_lock)
{
    int i, number_steps;
    // for the Depth First Search

#ifdef CHECK_DEBUG
    assert((*p_dead_lock == FALSE) || (*p_dead_lock == TRUE));
#endif

    if ((updated_forward != TRUE) && (*p_dead_lock == FALSE))
    {
        updated_forward = TRUE;
        updated_backward = FALSE;
        objective_updated = FALSE;

        Stack<Node> stack_Nodes;

        Initialize_Forward();

        for (i = 0; i < total_num_jobs; i++)
        {
            stack_Nodes.Push(&(graph_Node[i][0]));
            // Push the forward source steps for every job.
        }

        Get_Start_Windows(p_dead_lock, &stack_Nodes, FORWARD);

        for (i = 0; i < total_num_jobs; i++)
        {
            number_steps = graph_Job_Info[i].number_steps;
            if (graph_Node[i][number_steps + 1].cleared_status[FORWARD] == counter_TRUE)
            {
                graph_Job_Info[i].completion_time = graph_Node[i][number_steps + 1].release_time;
            }
            else
            {
                // That is, it has not been scanned, there was a dead lock.
                *p_dead_lock = TRUE;
            }
        }
    }
}

void Graph::Initialize_Backward()
{
    int i, j;
    int sink_step;
    int makespan;

    if (flag_reset == TRUE)
    {
        flag_reset = FALSE;

        for (i = 0; i < total_num_jobs; i++)
        {
            sink_step = graph_Job_Info[i].number_steps + 2;
            for (j = 0; j < sink_step; j++)
            {
                // All but the sinks are initialized
                graph_Node[i][j].visit_status[BACKWARD] = 0;
                graph_Node[i][j].cleared_status[BACKWARD] = 0;
                graph_Node[i][j].poped[BACKWARD] = 0;
                // No node has been poped yet.
            }
            graph_Node[i][sink_step].visit_status[BACKWARD] = 0;
            graph_Node[i][sink_step].cleared_status[BACKWARD] = 0;
            graph_Node[i][sink_step].poped[BACKWARD] = 0;
            // No node has been poped yet.
        }
    }

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
                    max2(graph_Job_Info[i].completion_time, graph_Job_Info[i].due_date));
            }
            break;

        case COMPLETION:
            for (i = 0; i < total_num_jobs; i++)
            {
                sink_step = graph_Job_Info[i].number_steps + 2;
                graph_Node[i][sink_step].due_time[i].Set(graph_Job_Info[i].completion_time);
            }
            break;
    }
}

// Get_Backward_Windows( ):This call scans all the sinks backwards.
void Graph::Get_Backward_Windows(int delay_constraints)
{
    int i;
    int sink_step;
    int dead_lock;
    // for the Depth First Search

    if (updated_backward != TRUE)
    {
        updated_backward = TRUE;

        this_par_delay_constraints = delay_constraints;
        // This is a global variable for this file.

        Stack<Node> stack_Nodes;

        Initialize_Backward();

        for (i = 0; i < total_num_jobs; i++)
        {
            sink_step = graph_Job_Info[i].number_steps + 2;
            stack_Nodes.Push(&(graph_Node[i][sink_step]));
            // Push the backward source (sink) steps for every job.
        }
        Get_Start_Windows(&dead_lock, &stack_Nodes, BACKWARD);
    }
}

int Graph::Get_Makespan()
{
    int i;

    makespan = INT_MIN;
    for (i = 0; i < total_num_jobs; i++)
    {
        if (graph_Job_Info[i].completion_time > makespan)
        {
            makespan = graph_Job_Info[i].completion_time;
        }
    }
    return (makespan);
}

int Graph::Get_Weighted_Tardiness()
{
    int i;

    total_weighted_tardiness = 0;
    for (i = 0; i < total_num_jobs; i++)
    {
        total_weighted_tardiness = total_weighted_tardiness +
                                   graph_Job_Info[i].completion_time_penalty *
                                       pos(graph_Job_Info[i].completion_time - graph_Job_Info[i].due_date);
    }

#ifdef DETAIL_DEBUG
    fprintf(par_output_file, "\n");
#endif
#ifdef FLOW_DEBUG
    fprintf(par_output_file, "TOTAL WEIGHTED TARDINESS = %d\n ", total_weighted_tardiness);
#endif

    return (total_weighted_tardiness);
}

int Graph::Get_Weighted_Completion()
{
    int i;

    total_weighted_completion = 0;
    for (i = 0; i < total_num_jobs; i++)
    {
        total_weighted_completion = total_weighted_completion + graph_Job_Info[i].completion_time_penalty *
                                                                    graph_Job_Info[i].completion_time;
    }
    return (total_weighted_completion);
}

int Graph::Get_Objective()
{
    int dead_lock = FALSE;

    if (objective_updated == FALSE)
    {
        objective_updated = TRUE;

        if (updated_forward == FALSE) Get_Forward_Windows(&dead_lock);

#ifdef CHECK_DEBUG
        assert(dead_lock == FALSE);
#endif

        switch (par_objective)
        {
            case MAKESPAN:
                objective = Get_Makespan();
                break;

            case TARDINESS:
                objective = Get_Weighted_Tardiness();
                break;

            case COMPLETION:
                objective = Get_Weighted_Completion();
                break;
        }
    }
    return (objective);
}

void Graph::Set_Objective(int input_objective)
{
    objective_updated = TRUE;
    objective = input_objective;

    switch (par_objective)
    {
        case MAKESPAN:
            makespan = input_objective;
            break;

        case TARDINESS:
            total_weighted_tardiness = input_objective;
            break;

        case COMPLETION:
            total_weighted_completion = input_objective;
            break;
    }
}

void Graph::Update_Nodes()
{
    int i, j;

    for (i = 0; i < total_num_jobs; i++)
    {
        for (j = 1; j <= graph_Job_Info[i].number_steps; j++)
        {
            graph_Node[i][j].Lateness_Tardiness(number_due_times, completion_time_penalty);
        }
    }
}

int Graph::Tardiness_Node(int job, int step)
{
#ifdef CHECK_DEBUG
    assert(job >= 0 && job < total_num_jobs);
    assert(step >= 1 && step <= graph_Job_Info[job].number_steps);
    assert(updated_forward == TRUE);
    assert(updated_backward == TRUE);
#endif

    return (graph_Node[job][step].tardiness);
}

int Graph::Estimate_Delta(int job, int step, int estimate_release_time)
{
#ifdef CHECK_DEBUG
    assert(job >= 0 && job < total_num_jobs);
    assert(step >= 1 && step <= graph_Job_Info[job].number_steps);
    //                                              assert( updated_forward == TRUE ) ;
    //                                              assert( updated_backward == TRUE ) ;
#endif

    return (graph_Node[job][step].Estimate_Delta(
        estimate_release_time, number_due_times, completion_time_penalty));
}

int Graph::Lateness_Node(int job, int step)
{
#ifdef CHECK_DEBUG
    assert(job >= 0 && job < total_num_jobs);
    assert(step >= 1 && step <= graph_Job_Info[job].number_steps);
#endif

    return (graph_Node[job][step].lateness);
}

int Graph::Slack_Node(int job, int step)
{
#ifdef CHECK_DEBUG
    assert(job >= 0 && job < total_num_jobs);
    assert(step >= 1 && step <= graph_Job_Info[job].number_steps);
#endif

    return (graph_Node[job][step].release_time - graph_Node[job][step].min_release);
}

void Graph::Print_Freezable()
{
    int i, j;
    FILE* this_file = open_file("frozen", "w");

    for (i = 0; i < total_num_jobs; i++)
    {
        for (j = 1; j <= graph_Job_Info[i].number_steps; j++)
        {
            fprintf(this_file, "%d ", Is_Freezable(i, j));
        }
        fprintf(this_file, "\n");
    }
    fclose(this_file);
}

/*
//Mach_Pred_Is_Adjacent(): returns TRUE if the machine predecessor is adjacent.
//returns false otherwise, or if there is no predecessor. The predecessor operation in in &job &step.
int Graph::Mach_Pred_Is_Adjacent( int *p_job , int *p_step )
{
                                        #ifdef CHECK_DEBUG
                                                assert(         graph_Node[*p_job][*p_step].From_Exists() == 1
|| graph_Node[*p_job][*p_step].From_Exists() == 0 ) ; #endif if ( graph_Node[*p_job][*p_step].From_Exists() )
{ int start_time = graph_Node[*p_job][*p_step].Get_Release_Time() ; int pred_job =
graph_Node[*p_job][*p_step].From_Job() ; int pred_step = graph_Node[*p_job][*p_step].From_Step() ;

                                        #ifdef CHECK_DEBUG
                                                assert( graph_Node[pred_job][pred_step].Get_Finishing_Time()
<= start_time ) ; #endif

                if ( graph_Node[pred_job][pred_step].Get_Finishing_Time() == start_time ) {
                        *p_job = pred_job ;
                        *p_step = pred_step ;
                        return( TRUE ) ;
                } else
                        return( FALSE ) ;
        } else {
                return( FALSE ) ;
        }
}

//Mach_Succ_Is_Adjacent(): returns TRUE if the machine successor is adjacent.
//returns false otherwise, or if there is no successor. The successor operation in in &job &step.
int Graph::Mach_Succ_Is_Adjacent( int *p_job , int *p_step )
{
                                        #ifdef CHECK_DEBUG
                                                assert(         graph_Node[*p_job][*p_step].Arc_Exists() == 1
|| graph_Node[*p_job][*p_step].Arc_Exists() == 0 ) ; #endif if (
graph_Node[*p_job][*p_step].disjunctive[FORWARD][0].Exist() ) { int completion_time =
graph_Node[*p_job][*p_step].Get_Finishing_Time() ; int succ_job =
graph_Node[*p_job][*p_step].disjunctive[FORWARD][0].Job() ; int succ_step =
graph_Node[*p_job][*p_step].disjunctive[FORWARD][0].Step() ;

                                        #ifdef CHECK_DEBUG
                                                assert( graph_Node[succ_job][succ_step].Get_Release_Time() >=
completion_time ) ; #endif

                if ( graph_Node[succ_job][succ_step].Get_Release_Time() == completion_time ) {
                        *p_job = succ_job ;
                        *p_step = succ_step ;
                        return( TRUE ) ;
                } else
                        return( FALSE ) ;
        } else {
                return( FALSE ) ;
        }
}

int Graph::There_Is_Mach_Pred( int *p_mach_pred_job , int *p_mach_pred_step , int job , int step )
{
                                        #ifdef CHECK_DEBUG
                                                assert(         graph_Node[job][step].From_Exists() == 1 ||
                                                                graph_Node[job][step].From_Exists() == 0 ) ;
                                        #endif
        if ( graph_Node[job][step].From_Exists() ) {
                *p_mach_pred_job = graph_Node[job][step].From_Job() ;
                *p_mach_pred_step = graph_Node[job][step].From_Step() ;
                return( TRUE ) ;
        } else
                return( FALSE ) ;
}

int Graph::There_Is_Mach_Succ( int *p_mach_succ_job , int *p_mach_succ_step , int job , int step )
{
                                        #ifdef CHECK_DEBUG
                                                assert(         graph_Node[job][step].Arc_Exists() == 1 ||
                                                                graph_Node[job][step].Arc_Exists() == 0 ) ;
                                        #endif
        if ( graph_Node[job][step].disjunctive[FORWARD][0].Exist() ) {
                *p_mach_succ_job = graph_Node[job][step].disjunctive[FORWARD][0].Job() ;
                *p_mach_succ_step = graph_Node[job][step].disjunctive[FORWARD][0].Step() ;
                return( TRUE ) ;
        } else
                return( FALSE ) ;
}

int Graph::Job_Pred_Is_Adjacent( int *p_job_pred_center_id , int job , int step , int sigma )
{
        if ( step == 1 )
                return( FALSE ) ;

        if ( graph_Node[job][step - 1].Get_Finishing_Time() + sigma >=
graph_Node[job][step].Get_Release_Time() ) { *p_job_pred_center_id = graph_Node[job][step - 1].work_center ;
                return( TRUE ) ;
        } else
                return( FALSE ) ;
}

*/
