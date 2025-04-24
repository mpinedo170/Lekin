#ifndef NODE_H
#define NODE_H

#include "const.h"
#include "lib_func.h"

#define FORWARD 0
#define BACKWARD 1

//-----------------------------------------------------------------------------------------

class Class_Due_Time
{
    int time;
    int exist;

public:
    Class_Due_Time()
    {
        time = NULL_TIME;
        exist = FALSE;
    }

    void Set(int input_time)
    {
        time = input_time;
        exist = TRUE;
    }
    int Time()
    {
        return (time);
    }

    int Exist()
    {
        return (exist);
    }

    void Delete()
    {
        time = NULL_TIME;
        exist = FALSE;
    }

    void operator=(const Class_Due_Time& from)
    {
        time = from.time;
        exist = from.exist;
    }

    int operator==(const Class_Due_Time& from)
    {
        if ((time != from.time) || (exist != from.exist))
        {
            return (FALSE);
        }
        else
        {
            return (TRUE);
        }
    }
};

//-----------------------------------------------------------------------------------------

class Class_Node_Index
{  // address of a node. It correspons to an operation.

public:
    int job;
    int step;

    Class_Node_Index(int input_job, int input_step)
    {
        job = input_job;
        step = input_step;
    }
    Class_Node_Index()
    {
        job = NULL_JOB;
        step = NULL_STEP;
    }
    void Initialize()
    {
        job = NULL_JOB;
        step = NULL_STEP;
    }
    void Terminate()
    {}
    void Set(int input_job, int input_step)
    {
        job = input_job;
        step = input_step;
    }
    void operator=(const Class_Node_Index& from)
    {
        job = from.job;
        step = from.step;
    }
    int operator==(const Class_Node_Index& from)
    {
        if ((job != from.job) || (step != from.step))
            return (FALSE);
        else
            return (TRUE);
    }
};
typedef Class_Node_Index* type_p_Class_Node_Index;

typedef int* type_p_int;
typedef int** type_p_p_int;
typedef int*** type_p_p_p_int;

//-----------------------------------------------------------------------------------------

class Class_Arc
{
    Class_Node_Index head;
    int length;
    int exist;  // TRUE if exists, FALSE if not.
    //      int position ;             //position in the set of arcs that this one has.

public:
    Class_Arc()
    {
        length = 0;
        exist = FALSE;
    };
    //      void Create(int job_to, int step_to , int input_length , int input_position ){
    void Create(int job_to, int step_to, int input_length)
    {
#ifdef CHECK_DEBUG
        assert(exist == FALSE);
#endif
        head.job = job_to;
        head.step = step_to;
        length = input_length;
        exist = TRUE;
        //              position = input_position ;
    }

    // Augusto 17.10.96
    void Modify(int job_to, int step_to, int input_length)
    {
#ifdef CHECK_DEBUG
        assert(exist == TRUE);
#endif
        //                head.job = job_to ;
        //                head.step = step_to ;
        length = input_length;
    }

    int Job()
    {
#ifdef CHECK_DEBUG
        assert(exist != FALSE);
#endif
        return (head.job);
    }

    int Step()
    {
        return (head.step);
    }
    int Length()
    {
#ifdef CHECK_DEBUG
        assert(exist != FALSE);
#endif
        return (length);
    }

    int Exist()
    {
#ifdef CHECK_DEBUG
        assert(exist == 1 || exist == 0);
#endif
        return (exist);
    }

    void Delete()
    {
#ifdef CHECK_DEBUG
        assert(exist == TRUE);
#endif
        //              head.Initialize();
        //              length = 0;
        exist = FALSE;
    }

    void operator=(const Class_Arc& from)
    {
        head.job = from.head.job;
        head.step = from.head.step;
        length = from.length;
        exist = from.exist;
        //              position = from.position ;
    }

    int operator==(const Class_Arc& from)
    {
        if ((head.job != from.head.job) || (head.step != from.head.step) ||
            (length != from.length)
            //              ||      ( position != from.position )
            || (exist != from.exist))
        {
            return (FALSE);
        }
        else
        {
            return (TRUE);
        }
    }
};

//-----------------------------------------------------------------------------------------------

class Node
{
    friend class Graph;
    friend class Plant;

    friend class Parameters;

    //      Class_Node_Index head_conjunctive;
    // where the conjunctive arc is pointing
    // For the time being we assume  head_conjunctive[i][j]=[i][j+1]

    // The following data structures represents the arcs.
    // They are duplicated so the function Get_Windows
    // does boph directions.
    // The forward arc i->j is the backward arc j->i
    // They are private becouse when adding or editing an arc, its twin must
    // be modifyed accordingly.

    // Arcs:

    Class_Node_Index position;
    // Location [i][j] of the node.

    Class_Arc conjunctive[2];

    //      Class_Arc *disjunctive[2];
    //      int number_disjunctives[2] ;
    //      int *p_number_disjunctives[2] ;
    // position 0 is FORWARD, position 1 is BACKWARD.

    int release_time;
    int min_release;
    // Minimum time an operation can be started.
    // This value is calculated at the very beggining, and remains constant.

    Class_Due_Time* due_time;
    // there is one due date for every job.

    int visit_status[2];
    // It can be either  NOT_VISITED or ONCE_VISITED.

    int cleared_status[2];
    // if TRUE, it can be pushed into the stack.

    int poped[2];
    // if TRUE then this node has been already poped out of the stack and
    // should not be scanned again during a given iteration. If it is, there is a cycle.

    // This data structure is for the gantt Chart.
    int work_center;
    int machine;
    // Stores the work center and machine where
    // the corresponding step is processed.

    int location_center;
    // Location of this node in the work center.

    int is_scheduled;

    int lateness;
    int tardiness;

    int number_jobs;
    int** delay;
    // For the delayed precedence constraints.

    int is_movable;
    // It is not frozable.

public:
    Node();
    void Initialize(int, int, int, int, int*);
    void Terminate();

    void operator=(const Node&);
    int operator==(const Node&);

    void Set_Machine_Is_Scheduled(int);
    void Unset_Machine_Is_Scheduled();

    int Get_Finishing_Time()
    {
        return (release_time + conjunctive[FORWARD].Length());
    }

    void Lateness_Tardiness(int, int*);
    int Estimate_Delta(int, int, int*);
};

typedef Node* type_p_Node;

#endif
