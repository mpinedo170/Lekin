#include "StdAfx.h"

#include "node.h"

Node::Node()
{
    release_time = NULL_TIME;
    min_release = NULL_TIME;

    visit_status[FORWARD] = 0;
    cleared_status[FORWARD] = FALSE;
    poped[FORWARD] = FALSE;

    visit_status[BACKWARD] = 0;
    cleared_status[BACKWARD] = FALSE;
    poped[BACKWARD] = FALSE;

    work_center = NULL_WORK_CENTER;
    machine = NULL_MACHINE;
    location_center = NULL_LOCATION;
    is_scheduled = FALSE;

    is_movable = TRUE;
}

// void Node::Initialize( int input_job , int input_step , int number_jobs , int number_due_dates )
void Node::Initialize(
    int input_job, int input_step, int number_due_dates, int input_number_jobs, int* number_operations)
{
    position.job = input_job;
    position.step = input_step;
    // Allocating memory:
    due_time = new Class_Due_Time[number_due_dates];

    number_jobs = input_number_jobs;
    delay = new type_p_int[number_jobs];
    for (int i = 0; i < number_jobs; i++)
    {
        delay[i] = new int[number_operations[i] + 1];
    }
}

void Node::Terminate()
{
    position.job = NULL_JOB;
    position.step = NULL_STEP;

    // De-allocating memory:
    delete[] this->due_time;

    for (int i = 0; i < number_jobs; i++)
    {
        delete[] delay[i];
    }
    delete[] delay;
}

void Node::operator=(const Node& from)
{
    release_time = from.release_time;
    min_release = from.min_release;

    work_center = from.work_center;
    location_center = from.location_center;

    position.job = from.position.job;
    position.step = from.position.step;

    conjunctive[FORWARD] = from.conjunctive[FORWARD];
    conjunctive[BACKWARD] = from.conjunctive[BACKWARD];
}

int Node::operator==(const Node& from)
{
    if (release_time != from.release_time) return (FALSE);

    if ((work_center != from.work_center) || (machine != from.machine) ||
        (location_center != from.location_center))
        return (FALSE);

    if (!(conjunctive[FORWARD] == from.conjunctive[FORWARD]) ||
        !(conjunctive[BACKWARD] == from.conjunctive[BACKWARD]))
        return (FALSE);

    return (TRUE);
}

void Node::Lateness_Tardiness(int number_due_times, int* completion_time_penalty)
{
    int i;

    lateness = 0;
    tardiness = 0;
    for (i = 0; i < number_due_times; i++)
    {
        // For every sink node the index is calculated and added.

        if (due_time[i].Exist())
        {
            // If there is no path from the node to a sink, do not calculate
            // ist penalty.

            lateness = lateness + (release_time - due_time[i].Time()) / completion_time_penalty[i];
            tardiness = tardiness + pos(release_time - due_time[i].Time()) / completion_time_penalty[i];
        }
    }
}

// int Node::Estimate_Tardiness(         int estimate_release_time ,
int Node::Estimate_Delta(int estimate_release_time, int number_due_times, int* completion_time_penalty)
{
    int i;

    tardiness = 0;
    for (i = 0; i < number_due_times; i++)
    {
        // For every sink node the index is calculated and added.

        if (due_time[i].Exist())
        {
            // If there is no path from the node to a sink, do not calculate
            // ist penalty.

            tardiness =
                tardiness + pos(estimate_release_time - due_time[i].Time()) * completion_time_penalty[i];
        }
    }
    return (tardiness);
}

/*
void Node::Add_Conjunctive( lOng int job_to , int step_to , int length , int direction )
{
        conjunctive[direction].Create( job_to,  step_to , length , 0 );
        number_predecesors[!direction]++;
}

void Node::Add_Disjunctive( int job_to , int step_to , int length , int position , int diraction )
{
        disjunctive[direction][number_disjunctives[direction]].Create( job_to,  step_to , length , position );
        number_disjunctives[direction]++;
        number_predecesors[!direction]++;
}
*/
