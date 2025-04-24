#ifndef CLUS_CEN_H
#define CLUS_CEN_H

#include "graph.h"
#include "plant.h"

// Cluster_Operation: contais the information for every operation to solve the
// subproblem defined by the cluster.

class Cluster_Operation
{
    friend class Cluster_Work_Center;

    // Data structures:
    int job;
    int step;
    int length;

    // These elements point to the data of Node.h.
    short* p_release_time;
    Class_Due_Time* due_time;

    int work_center;
    // It is fixed.

    int* p_machine;
    // Is a pointer because it changes constantly.

    char location_recirc;
    // Stores the location (with respect to the cluster) of the recirculating
    // operation for the current operation.

    char is_available;
    // Stores whether an operation is available.
    // It is TRUE if the operation is not a sucesor.
    // It is FALSE if this is a recirculating step and the previous one
    // has not been scheduled yet.

    int* p_is_scheduled;

    //      int *p_cluster_job_pred_is_scheduled ;
    // Points to the node that corresponds to the job predecessor of this operation in the cluster.

    //      int *p_complete ;

public:
    Cluster_Operation()
    {
        job = NULL_JOB;
        step = NULL_STEP;
        length = NULL_LENGTH;
        p_release_time = NULL;
        due_time = NULL;
        work_center = NULL_WORK_CENTER;
        p_machine = NULL;
        location_recirc = NULL_LOCATION;
        is_available = TRUE;
        p_is_scheduled = NULL;
        //              p_cluster_job_pred_is_scheduled = NULL ;
        //              p_complete = NULL ;
    }

    void Set_Job_Step(int input_job, int input_step)
    {
        job = input_job;
        step = input_step;
    }

    int Get_Job()
    {
        return (job);
    }

    int Get_Step()
    {
        return (step);
    }

    int Get_Length()
    {
        return (length);
    }

    int Get_Release_Time()
    {
        return (*p_release_time);
    }

    int Get_Finishing_Time()
    {
        return (*p_release_time + length);
    }

    Class_Due_Time* Get_Due_Time(int position)
    {
        return (&(due_time[position]));
    }

    void Set_Machine_Is_Scheduled(int machine_id)
    {
        *p_machine = machine_id;
        *p_is_scheduled = TRUE;
    }

    void Unset_Machine_Is_Scheduled()
    {
        *p_machine = NULL_MACHINE;
        *p_is_scheduled = FALSE;
    }

    int Is_Available()
    {
        return (is_available);
    }

    int Is_Scheduled()
    {
        return (*p_is_scheduled);
    }

    int* Get_p_Is_Scheduled()
    {
        return (p_is_scheduled);
    }

    /*
            void Set_p_Job_Pred_Is_Sched( int *input_p_cluster_job_pred_is_scheduled )
            {
                    p_cluster_job_pred_is_scheduled = input_p_cluster_job_pred_is_scheduled ;
            }

    //Cluster_Predecessor_Is_Scheduled() returns TRUE if the job-predecessor operation in the cluster has been
    already scheduled.
    //If the operations has no job-predessors in the cluster, Cluster_Predecessor_Is_Scheduled() = TRUE.
            int Cluster_Predecessor_Is_Scheduled()
            {
                    if ( ( par_option == 2 ) || ( par_accelerator_predecessor == 0 ) )
                            return( TRUE ) ;
                                                    //If we are calculating lower bounds, every operation must
    be schedulable,
                                                    //or we are not using the corresponding accelerator.

                    if ( p_cluster_job_pred_is_scheduled == NULL ) {
                                                    //Te operation has no job predecessors in the cluster.
                            return( TRUE ) ;
                    } else {
                            return( *p_cluster_job_pred_is_scheduled ) ;
                    }
            }
    */
    int Is_Complete(Graph* p_Graph)
    {
        return (p_Graph->Is_Complete(job, step));
    }
};

typedef Cluster_Operation* type_p_Cluster_Operation;
typedef Cluster_Operation** type_p_p_Cluster_Operation;

//-----------------------------------------------------------------------------------------------

class Cluster_Work_Center
{
    friend class Cluster;

    // Data structures:

    int id;  // The id of the work center.
             // The same of the work center in the plant.

    // Augusto 12.8.96
    Work_Center* p_Work_Center;
    //        Cluster_Work_Center *p_Work_Center ;
    int number_machines;

    short number_operations;
    // number of steps assigned to this cluster.

    Cluster_Operation* operation;
    // set of operations assigned to this cluster.

    // Data structures for sequencing:

    int total_number_scheduled;
    // number of operations in the cluster already scheduled.

    // This part is redundant with Solution_Oracle
    int* machine_number_scheduled;
    // number of operations scheduled in each machine.

    int** sequence;
    // Sequence in which the operations have been scheduled.
    // sequence[1][3] = 5 means the 4th operation to be scheduled in machine 2 was 5.
    // It is used for getting rid of the Chart.
    //

    Node** pred_node;
    // Array of pointers for the operations in the work center that preceed the cluster.
    // pred_node[1] points to the operation in machine 2 that preceeds the cluster.

    // 3     Node **succ_node ;
    // Array of pointers for the operations in the work center that succed the cluster.

    int min_release_time;
    // Stores the earliest available time in the work center.
    // Helps to calculate more accurate available times for operations in the cluster.

    int* finishing_time_machine;
    // Keeps the finishing time for every machine.
    // For instance, finishing_time_machine[3] == 2 means machine 3 is available at time 2.

    // Functions:

    int Nth_Job(int n)
    // Returns the job at the nth position for the work center.
    {
        return (operation[n].job);
    };

    int Nth_Step(int n)
    // Returns the step at the nth position for the work center.
    {
        return (operation[n].step);
    };

public:
    Cluster_Work_Center();

    // Functions used by Set_Clusters:
    void Initialize(int, int, int);
    void Terminate();
    void Copy_Graph_Plant_Information(Graph*, Plant*);
    void Clean_Information(Graph*);
    void Check_Recirculation();

    void Delete_Disjunctives(Graph*);
    void Reset_Machine(Graph*, int[MAX_OPERATIONS_CLUSTER]);
    void Print(int);

    int Get_Id()
    {
        return (id);
    };
    int Get_Number_Machines()
    {
        return (number_machines);
    };
    int Get_Number_Operations()
    {
        return (number_operations);
    };
    int Get_Number_Unscheduled_Operations()
    {
        return (number_operations - total_number_scheduled);
    };
    Cluster_Operation* Get_Operation(int operation_number)
    {
#ifdef CHECK_DEBUG
        assert(operation_number >= 0 && operation_number < number_operations);
#endif
        return (&(operation[operation_number]));
    }

    void Schedule_Operation(int*, Graph*, int, int, int);

    void Unschedule_Operation(int, int, Graph*, int);

    void Next_Idle_Work_Center_Machine(int*, int*);

    void Set_Mach_Pred(Graph*, int, int, int);
    void Set_Mach_Succ(Graph*, int, int, int);

    void Update_Machines_Time(Graph*);
    void Set_Max_Starting_Time(int, int, int);

    int Cluster_Work_Center::f1(const void*, const void*);
    //      void Cluster_Work_Center::Check_Active_Schedule( Graph *p_Graph ) ;

    void Set_Min_Release_Time(int input_min_release_time)
    {
        min_release_time = input_min_release_time;
    }

    void Set_Finishing_Time_Machine(int machine, int input_finishing_time_machine)
    {
        finishing_time_machine[machine] = input_finishing_time_machine;
    }
};

#endif
