#ifndef LIST_DIS_H
#define LIST_DIS_H

#include "globals.h"

class Class_Disjunctive_Arc
{
public:
    int job_from, step_from;
    int job_to, step_to;

    Class_Disjunctive_Arc()
    {
        job_from = NULL_JOB;
        step_from = NULL_STEP;
        job_to = NULL_JOB;
        step_to = NULL_STEP;
    }
};

class Class_List_Disjunctives
{
    int number_disjunctives;
    int current_position;
    Class_Disjunctive_Arc disjunctive_arc[MAX_OPERATIONS_CLUSTER];

public:
    Class_List_Disjunctives()
    {
        number_disjunctives = 0;
        current_position = 0;
    };

    void Reset()
    {
        number_disjunctives = 0;
        current_position = 0;
    };

    void Add_Arc(int input_job_from, int input_step_from, int input_job_to, int input_step_to)
    {
#ifdef CHECK_DEBUG
        assert(current_position < MAX_OPERATIONS_CLUSTER);
#endif

        disjunctive_arc[current_position].job_from = input_job_from;
        disjunctive_arc[current_position].step_from = input_step_from;
        disjunctive_arc[current_position].job_to = input_job_to;
        disjunctive_arc[current_position].step_to = input_step_to;

        number_disjunctives++;
        current_position++;
    }
    int Get_Number_Disjunctives()
    {
        return (number_disjunctives);
    }
    int Get_Nth_Job_From(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < number_disjunctives);
#endif

        return (disjunctive_arc[position].job_from);
    }

    int Get_Nth_Step_From(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < number_disjunctives);
#endif

        return (disjunctive_arc[position].step_from);
    }

    int Get_Nth_Job_To(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < number_disjunctives);
#endif

        return (disjunctive_arc[position].job_to);
    }

    int Get_Nth_Step_To(int position)
    {
#ifdef CHECK_DEBUG
        assert(position < number_disjunctives);
#endif

        return (disjunctive_arc[position].step_to);
    }

    void Print()
    {
        /*
            for ( i = 0 ; i < number_disjunctives ; i++ ){
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
