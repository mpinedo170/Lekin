#ifndef SOL_GRPH_H
#define SOL_GRPH_H

#include "list_dis.h"

class Solution_Graph
{
    int objective;  // Keeps the objective function of the graph
                    // with this list of disjuctives.

    int time_out;  // TRUE if there is a time out.

    int number_jobs;
    int* completion_time;
    int** machine;

    int current_number_disj;
    int max_number_arcs;
    Class_Disjunctive_Arc* disjunctive_arc;

public:
    Solution_Graph()
    {
        objective = INT_MAX;
        time_out = FALSE;
        completion_time = NULL;
        disjunctive_arc = NULL;
    }

    void Initialize(int input_number_jobs, int* number_steps)
    {
#ifdef CHECK_DEBUG
        assert(disjunctive_arc == NULL);
#endif
        objective = INT_MAX;

        number_jobs = input_number_jobs;
        completion_time = new int[number_jobs];
        machine = new type_p_int[number_jobs];
        for (int i = 0; i < number_jobs; i++) machine[i] = new int[number_steps[i] + 1];
    }

    void Prepare_Write(int input_max_number_arcs)
    {
#ifdef CHECK_DEBUG
        assert(disjunctive_arc == NULL);
#endif
        current_number_disj = 0;
        max_number_arcs = input_max_number_arcs;
        disjunctive_arc = new Class_Disjunctive_Arc[max_number_arcs];
    }

    void Delete()
    {
        objective = -1;

        current_number_disj = 0;
        max_number_arcs = -1;
        if (disjunctive_arc != NULL)
        {
            delete[] disjunctive_arc;
            disjunctive_arc = NULL;
        }
    }

    void Terminate()
    {
        Delete();
        for (int i = 0; i < number_jobs; i++) delete[] machine[i];
        delete[] machine;
        delete[] completion_time;
    }

    void operator=(const Solution_Graph& from)
    {
        int i;
        objective = from.objective;
        time_out = from.time_out;

#ifdef CHECK_DEBUG
        assert(number_jobs == from.number_jobs);
#endif
        for (i = 0; i < number_jobs; i++) completion_time[i] = from.completion_time[i];

#ifdef CHECK_DEBUG
        assert(max_number_arcs == from.max_number_arcs);
#endif
        current_number_disj = from.current_number_disj;
        for (i = 0; i < current_number_disj; i++)
        {
            disjunctive_arc[i].job_to = from.disjunctive_arc[i].job_to;
            disjunctive_arc[i].step_to = from.disjunctive_arc[i].step_to;
            disjunctive_arc[i].job_from = from.disjunctive_arc[i].job_from;
            disjunctive_arc[i].step_from = from.disjunctive_arc[i].step_from;
        }
    }

    int operator==(const Solution_Graph& from)
    {
        int i;
        if (objective != from.objective) return (FALSE);
        for (i = 0; i < number_jobs; i++)
        {
            if (completion_time[i] != from.completion_time[i]) return (FALSE);
        }
        if (current_number_disj != from.current_number_disj) return (FALSE);
        for (i = 0; i < current_number_disj; i++)
        {
            if ((disjunctive_arc[i].job_to != from.disjunctive_arc[i].job_to) ||
                (disjunctive_arc[i].step_to != from.disjunctive_arc[i].step_to) ||
                (disjunctive_arc[i].job_from != from.disjunctive_arc[i].job_from) ||
                (disjunctive_arc[i].step_from != from.disjunctive_arc[i].step_from))
                return (FALSE);
        }
        return (TRUE);
    }

    void Set_Objective(int input_objective)
    {
        objective = input_objective;
    }

    int Get_Objective()
    {
#ifdef CHECK_DEBUG
        assert(objective >= 0);
#endif
        return (objective);
    }

    void Set_Time_Out(int input_time_out)
    {
        time_out = input_time_out;
    }

    int Get_Time_Out()
    {
        return (time_out);
    }

    void Write_Completion_Time(int position, int input_completion_time)
    {
#ifdef CHECK_DEBUG
        assert(position < number_jobs);
#endif

        completion_time[position] = input_completion_time;
    }

    void Set_Machine(int input_machine, int job, int step)
    {
#ifdef CHECK_DEBUG
        assert(job >= 0 && job <= number_jobs);
        assert(step >= 1);
#endif

        machine[job][step] = input_machine;
    }

    int Get_Machine(int job, int step)
    {
#ifdef CHECK_DEBUG
        assert(job >= 0 && job <= number_jobs);
        assert(step >= 1);
#endif

        return (machine[job][step]);
    }

    void Add_Arc(int input_job_from, int input_step_from, int input_job_to, int input_step_to)
    {
#ifdef CHECK_DEBUG
        if (current_number_disj >= max_number_arcs) assert(current_number_disj < max_number_arcs);
#endif

        disjunctive_arc[current_number_disj].job_from = input_job_from;
        disjunctive_arc[current_number_disj].step_from = input_step_from;
        disjunctive_arc[current_number_disj].job_to = input_job_to;
        disjunctive_arc[current_number_disj].step_to = input_step_to;

        current_number_disj++;
    }
    void Add_Arc(Class_Disjunctive_Arc* p_disjunctive_arc)
    {
#ifdef CHECK_DEBUG
        assert(current_number_disj < max_number_arcs);
#endif

        disjunctive_arc[current_number_disj].job_from = p_disjunctive_arc->job_from;
        disjunctive_arc[current_number_disj].step_from = p_disjunctive_arc->step_from;
        disjunctive_arc[current_number_disj].job_to = p_disjunctive_arc->job_to;
        disjunctive_arc[current_number_disj].step_to = p_disjunctive_arc->step_to;

        current_number_disj++;
    }
    void Delete_Arc()
    {
#ifdef CHECK_DEBUG
        assert(current_number_disj > 0);
#endif

        current_number_disj++;
    }
    int Get_Number_Disjunctives()
    {
        return (current_number_disj);
    }
    int Get_Nth_Job_From(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < current_number_disj);
#endif

        return (disjunctive_arc[position].job_from);
    }

    int Get_Nth_Step_From(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < current_number_disj);
#endif

        return (disjunctive_arc[position].step_from);
    }

    int Get_Nth_Job_To(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < current_number_disj);
#endif

        return (disjunctive_arc[position].job_to);
    }

    int Get_Nth_Step_To(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < current_number_disj);
#endif

        return (disjunctive_arc[position].step_to);
    }

    void Print()
    {
        /*
            int i ;
            for ( i = 0 ; i < current_number_disj ; i++ ){
                fprintf( par_output_file ," (%d,%d)->(%d,%d) ; ",
                        disjunctive_arc[i].job_from ,
                        disjunctive_arc[i].step_from ,
                        disjunctive_arc[i].job_to ,
                        disjunctive_arc[i].step_to
                );
            }
            fprintf( par_output_file ,"\n\n");
        */
    }
};

#endif
