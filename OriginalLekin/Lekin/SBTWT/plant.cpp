#include "StdAfx.h"

#include "plant.h"

Plant::Plant()
{
    number_work_centers = 0;
    //      plant_Work_Centers = NULL ;
    cluster_work_center = NULL;
}

/*
void Plant::Initialize( char *name_file , Graph *p_Graph )
{

        number_work_centers = 0;
        Read_File(name_file);
}
*/

void Plant::Terminate()
{
    int i;

    if (cluster_work_center != NULL)
    {
        for (i = 0; i < number_work_centers; i++)
        {
            cluster_work_center[i].Terminate();
        }
        delete[] cluster_work_center;
    }
}

// Writes the input for CUISE.
void Plant::Show_Gantt_Chart(char* name_file, Graph* p_Graph)
{
    // int i , j , k , l , m;
    int i, j, k, l;
    int job, step;
    int min_job, min_step;  // Job and steps with the minimum release time in the current iteration.
    int min_release_time;   // Minimum release time of the current iteration.
    int old_minimum;        // Minimum release time of the last operation to be printed.
    int finishing_time;

    int number_operations_machine;  // Number of operation scheduled in the machine of the current iteration.
    FILE* this_file;

    // This part prints the information about the jobs.
    int total_num_jobs;
    total_num_jobs = p_Graph->Get_Number_Jobs();

    this_file = fopen(name_file, "w");
    fprintf(this_file, "%d \n", total_num_jobs);
    for (i = 0; i < total_num_jobs; i++)
    {
        fprintf(this_file, "    %d     %d      %d   %d        %d    %d \n", i,
            p_Graph->graph_Job_Info[i].number_steps,

            p_Graph->graph_Job_Info[i].release_time, p_Graph->graph_Job_Info[i].due_date,

            p_Graph->graph_Job_Info[i].completion_time_penalty, p_Graph->graph_Job_Info[i].completion_time);
    }

    // This part prints the information about the operations.
    fprintf(this_file, "%d \n", number_work_centers);
    for (i = 0; i < number_work_centers; i++)
    {
        fprintf(this_file, "    %d \n", cluster_work_center[i].number_machines);
        for (j = 0; j < cluster_work_center[i].number_machines; j++)
        {
            number_operations_machine = 0;
            min_release_time = INT_MAX;
            finishing_time = INT_MAX;
            for (k = 0; k < cluster_work_center[i].number_operations; k++)
            {
                job = cluster_work_center[i].operation[k].Get_Job();
                step = cluster_work_center[i].operation[k].Get_Step();

                if ((p_Graph->graph_Node[job][step].machine == j) &&
                    (p_Graph->graph_Node[job][step].is_scheduled == TRUE))
                {
                    number_operations_machine++;
                    if ((p_Graph->graph_Node[job][step].release_time +
                            p_Graph->graph_Node[job][step].conjunctive[FORWARD].Length()) <= finishing_time)
                    {
                        finishing_time = p_Graph->graph_Node[job][step].release_time +
                                         p_Graph->graph_Node[job][step].conjunctive[FORWARD].Length();
                        min_job = job;
                        min_step = step;
                    }
                }
            }
            fprintf(this_file, "        %d \n", number_operations_machine);
            old_minimum = finishing_time;

            if (number_operations_machine > 0)
            {
                fprintf(this_file, "            %d  ", min_job);
                fprintf(this_file, "%d      ", min_step);
                min_release_time =
                    finishing_time - p_Graph->graph_Node[min_job][min_step].conjunctive[FORWARD].Length();
                fprintf(this_file, "%d  ", min_release_time);
                fprintf(this_file, "%d      ", finishing_time);

                fprintf(this_file, "\n");

                for (l = 0; l < (number_operations_machine - 1); l++)
                {
                    // For the remaining operations in such machine

                    min_release_time = INT_MAX;
                    finishing_time = INT_MAX;
                    for (k = 0; k < cluster_work_center[i].number_operations; k++)
                    {
                        job = cluster_work_center[i].operation[k].Get_Job();
                        step = cluster_work_center[i].operation[k].Get_Step();
                        if ((p_Graph->graph_Node[job][step].machine == j) &&
                            (p_Graph->graph_Node[job][step].is_scheduled == TRUE))
                        {
                            if (((p_Graph->graph_Node[job][step].release_time +
                                     p_Graph->graph_Node[job][step].conjunctive[FORWARD].Length()) <=
                                    finishing_time) &&
                                ((p_Graph->graph_Node[job][step].release_time +
                                     p_Graph->graph_Node[job][step].conjunctive[FORWARD].Length()) >
                                    old_minimum))
                            {
                                // It must be between the old minimum and the current minimum.

                                finishing_time = p_Graph->graph_Node[job][step].release_time +
                                                 p_Graph->graph_Node[job][step].conjunctive[FORWARD].Length();
                                min_job = job;
                                min_step = step;
                            }
                        }
                    }
                    old_minimum = finishing_time;

                    fprintf(this_file, "            %d  ", min_job);
                    fprintf(this_file, "%d      ", min_step);
                    min_release_time =
                        finishing_time - p_Graph->graph_Node[min_job][min_step].conjunctive[FORWARD].Length();
                    fprintf(this_file, "%d  ", min_release_time);
                    fprintf(this_file, "%d      ", finishing_time);
                    fprintf(this_file, "\n");
                }
            }
        }
    }

    fprintf(this_file, "%d \n ", p_Graph->Get_Weighted_Tardiness());
    fprintf(this_file, "%d \n ", p_Graph->Get_Makespan());
    fclose(this_file);
}

int Plant::Fix_Disjunctions(int* p_n_arcs_list, Arc_Backup* list, int max_possible_increase, Graph* p_Graph)
{
    int dead_lock = FALSE;
    int delta = 0;

    // First to update the graph.
    p_Graph->Get_Forward_Windows(&dead_lock);
#ifdef CHECK_DEBUG
    assert(dead_lock == FALSE);
#endif

    p_Graph->Get_Backward_Windows(FALSE);
    // Does not requiere delay precedence constraints.
    // int predecessor ;
    for (int i = 0; i < number_work_centers; i++)
    {
        if (!p_Graph->Valid_Answer_Cent(cluster_work_center[i].Get_Id())
            // TRUE if the selection of arcs is a tour.
            // A tour visits all the nones once with no cycle.
            &&
            //                        ( cluster_work_center[i].Get_Number_Machines() == 1 )
            (cluster_work_center[i].number_machines == 1))
        {
            delta =
                cluster_work_center[i].Fix_Disjunctions(p_n_arcs_list, list, max_possible_increase, p_Graph);
            // predecessor = p_Graph->Is_Optimal_Predecessor(0) ;
        }
        if (delta > 0) return (delta);
    }
    return (0);
}

void Plant::Copy_Flow_Shop_Sequence()
{
    par_optimal_sequence = new int[number_work_centers];
    for (int i = 0; i < number_work_centers; i++)
    {
        // This sequence comes from the shifting bottleneck.

        par_optimal_sequence[i] = i;
    }
}

#ifdef CUISE_CONNECTED

// Delete_Disjunctives( Graph *the_Graph ) deletes the disjunctive arcs in the backtracking process.
// For CUISE interface.
void Plant::Delete_Disjunctives_Work_Center(Graph* the_Graph, int work_center_id)
{
    int i;
    int job_from, step_from;

    for (i = 0; i < cluster_work_center[work_center_id].number_operations; i++)
    {
        job_from = cluster_work_center[work_center_id].operation[i].job;
        step_from = cluster_work_center[work_center_id].operation[i].step;

        the_Graph->Delete_Disjunctive(job_from, step_from);
    }
}

void Plant::Unset_Machine_Is_Scheduled_Work_Center(Graph* the_Graph, int work_center_id)
{
    int i;
    int job, step;

    for (i = 0; i < cluster_work_center[work_center_id].number_operations; i++)
    {
        job = cluster_work_center[work_center_id].operation[i].job;
        step = cluster_work_center[work_center_id].operation[i].step;

        the_Graph->graph_Node[job][step].Unset_Machine_Is_Scheduled();
    }
}

void Plant::Define_Disjunctives_Work_Center(
    // Output parameter.
    Class_List_Disjunctives* p_list_disjunctives,

    // Input paramenters.
    struct Manual_Solution_Struct* p_manual_reschedule)
{
    int j, k;
    int job_from, job_to;
    int work_center_id = p_manual_reschedule->work_center_id;

    p_list_disjunctives->Reset();
    // Get ready for adding arcs.

    for (j = 0; j < cluster_work_center[work_center_id].number_machines; j++)
    {
        if (p_manual_reschedule->length_solution[j] > 1)
        {
            for (k = 0; k < (p_manual_reschedule->length_solution[j] - 1); k++)
            {
                job_from = p_manual_reschedule->solution[j][k][0];
                job_to = p_manual_reschedule->solution[j][k + 1][0];

                //                              if (job_from != job_to){
                if (TRUE)
                {
                    // If it is not a "recirculation" arc.

                    p_list_disjunctives->Add_Arc(job_from, p_manual_reschedule->solution[j][k][1], job_to,
                        p_manual_reschedule->solution[j][k + 1][1]);
                }
            }
        }
    }
}

void Plant::Set_Machine_Is_Scheduled_Work_Center(
    Graph* the_Graph, struct Manual_Solution_Struct* p_manual_reschedule)
{
    int j, k;
    int job, step;
    int work_center_id = p_manual_reschedule->work_center_id;

    for (j = 0; j < cluster_work_center[work_center_id].number_machines; j++)
    {
        for (k = 0; k < p_manual_reschedule->length_solution[j]; k++)
        {
            job = p_manual_reschedule->solution[j][k][0];
            step = p_manual_reschedule->solution[j][k][1];
            the_Graph->graph_Node[job][step].Set_Machine_Is_Scheduled(j);
        }
    }
}

#endif
