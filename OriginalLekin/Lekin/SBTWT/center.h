#ifndef CENTER_H
#define CENTER_H

#include "graph.h"
#include "sol_incr.h"
#include "sol_orac.h"
#include "t_index.h"

//-----------------------------------------------------------------------------------------------
class Info_Operation
{
public:
    int job;
    int step;
    Node* p_Node;
};

typedef Info_Operation* type_p_Info_Operation;

//-----------------------------------------------------------------------------------------------

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
    int* p_release_time;
    Class_Due_Time* due_time;

    // Augusto 25.10.96
    int flag_ficticio;
    int machine_pre_set;

    int work_center;
    // It is fixed.

    int* p_machine;
    // Is a pointer because it changes constantly.

    int* p_is_scheduled;

    int is_marked;
    int marked_release_time;
    // Operations can be scheduled either directly in the graph (done when there
    // are many machines) or independently from the graph (when there is only one machine
    // per center). is_marked and marked_release_time keep data regarding to the secoind case.
    // is_marked== TRUE means that the operation was scheduled

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
        p_is_scheduled = NULL;
        is_marked = FALSE;
        marked_release_time = -1;
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

    int Get_Marked_Release_Time()
    {
#ifdef CHECK_DEBUG
        assert(*p_is_scheduled == FALSE);
#endif
        return (marked_release_time);
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
#ifdef CHECK_DEBUG
        assert(is_marked == FALSE);
        //                                              assert( marked_release_time == -1 ) ;
#endif
        *p_machine = machine_id;
        *p_is_scheduled = TRUE;
    }

    void Set_Marked(int input_marked_release_time, int machine_id)
    {
#ifdef CHECK_DEBUG
        assert(*p_is_scheduled == FALSE);
#endif

        *p_machine = machine_id;
        is_marked = TRUE;
        marked_release_time = input_marked_release_time;
    }

    void Unset_Marked()
    {
#ifdef CHECK_DEBUG
        assert(*p_is_scheduled == FALSE);
#endif

        *p_machine = -1;
        is_marked = FALSE;
        marked_release_time = -1;
    }

    void Unset_Machine_Is_Scheduled()
    {
        *p_machine = NULL_MACHINE;
        *p_is_scheduled = FALSE;
    }

    int Is_Scheduled()
    {
        return (*p_is_scheduled);
    }

    int* Get_p_Is_Scheduled()
    {
        return (p_is_scheduled);
    }

    int Is_Complete(Graph* p_Graph)
    {
        return (p_Graph->Is_Complete(job, step));
    }
    int Is_Almost_Complete(Graph* p_Graph)
    {
        if (step <= 2) return (TRUE);
        return (p_Graph->Is_Complete(job, step - 1));
    }
    int Is_Final_Operation(Graph* p_Graph)
    {
        if (step == p_Graph->Get_Number_Steps_Job(job))
            return (1);
        else
            return (0);
    }
    //      int Get_Release_Time_Job_Successor( Graph *p_Graph ) {
    //              Node **graph_node = p_Graph->Get_Graph_Node();
    //              return( graph_node[ job ][ step + 1 ].Get_Release_Time() ) ;
    //      }
    int Get_Due_Date(Graph* p_Graph)
    {
        return (p_Graph->Get_Due_Date_Job(job));
    }
};

typedef Cluster_Operation* type_p_Cluster_Operation;
typedef Cluster_Operation** type_p_p_Cluster_Operation;

//-------------------------------------------------------------------------------------

class ATC_Rule
{
    int number_due_times;  // Number of jobs in the job shop.
    int* completion_time_penalty;

    double step_length_average;
    double this_K;

public:
    ATC_Rule(){};
    void Initialize(int, int*, double, double);

    double Value_Index(Cluster_Operation*, int);
    void Insert_Operation(Beam_Indices<Solution_Increment>*, Cluster_Operation*, int, int, int, int, int);
};

//-----------------------------------------------------------------------------------------------

class Cluster_Work_Center
{
    friend class Cluster;
    friend class Plant;

    // Data structures:

    int id;  // The id of the work center.
             // The same of the work center in the plant.

    int number_machines;

    int* speed;  // stores the speed of each machine.

    int number_operations;
    // number of steps assigned to this cluster.

    int partial_num_oper;
    // Used to add operations to the cluster.

    Cluster_Operation* operation;
    // set of operations assigned to this cluster.

    // Data structures for sequencing:

    int total_number_scheduled;
    // number of operations in the center already scheduled.

    // This part is redundant with Solution_Oracle
    int* machine_number_scheduled;
    // number of operations scheduled in each machine.

    int** sequence;
    // Sequence in which the operations have been scheduled.
    // sequence[1][3] = 5 means the 4th operation to be scheduled in machine 2 was 5.
    // It is used for getting rid of the Chart.
    //

    Class_Node_Index* pred_node;
    // Array of nodes that precede each machine.
    // pred_node[1] points to the operation in machine 2 that preceeds the cluster.

    Class_Node_Index* succ_node;
    // Array of nodes that succede each machine.

    int min_release_time;
    // Stores the earliest available time in the work center.
    // Helps to calculate more accurate available times for operations in the cluster.

    int min_machine;
    // Machine that achieves the value min_release_time.

    int* finishing_time_machine;
    // Keeps the finishing time for every machine.
    // For instance, finishing_time_machine[3] == 2 means machine 3 is available at time 2.

    int completion_star;
    int position_star;
    int Omega_i;

    int earliest_release[2];
    int position_release[2];

    // Functions:

    int Nth_Job(int n)
    {
        // Returns the job at the nth position for the work center.

        return (operation[n].job);
    };

    int Nth_Step(int n)
    {
        // Returns the step at the nth position for the work center.

        return (operation[n].step);
    };

    //      void Get_Available_Time_Marked( int * , int * , Graph * ) ;
    int There_Is_Star(Graph*);
    //      void Calculate_Release_Times( Graph * , int * ) ;
    void Calculate_Release_Times();

    int Complete(Graph*);

public:
    Cluster_Work_Center();

    // Functions used by Set_Clusters:
    void Initialize(int, int, int, Class_Node_Index*, Class_Node_Index*, Graph*);
    // Augusto 9.10.96
    void Ini_veloc(int, int);

    void Terminate();
    int operator==(const Cluster_Work_Center&);

    void Add_Operation(int, int, Graph*);

    void Delete_Disjunctives(Graph*);
    void Print(int, Graph*);

    int Get_Id()
    {
        return (id);
    }
    int Get_Number_Machines()
    {
        return (number_machines);
    }
    int Get_Number_Operations()
    {
        return (number_operations);
    }
    int Get_Number_Unscheduled_Operations()
    {
        return (number_operations - total_number_scheduled);
    }
    Cluster_Operation* Get_Operation(int operation_number)
    {
#ifdef CHECK_DEBUG
        assert(operation_number >= 0 && operation_number < number_operations);
#endif
        return (&(operation[operation_number]));
    }

    // Augusto 17.10.96
    //       _length = Get_operation_length(location_operation, location_work_center)
    int Get_Length_operation(int id_operation)
    {
        return (operation[id_operation].Get_Length());
    }

    // Augusto 22.10.96
    int Get_Length_Longest_Operation()
    {
        int i, max, aux;
        max = 0;
        for (i = 0; i < number_operations; i++)
        {
            if (!operation[i].Is_Scheduled() && operation[i].Get_Length() > max)
            {
                aux = i;
                max = operation[i].Get_Length();
            }
        }
        return (max);
    }

    // Augusto 22.10.96
    int Get_Fastest_Machine()
    {
        int i, max, aux;
        max = 0;
        for (i = 0; i < number_machines; i++)
        {
            if (speed[i] > max)
            {
                aux = i;
                max = speed[i];
            }
        }
        return (aux);
    }

    // marcos 19.11.96
    int Get_Time_Earliest_Finishing_Machine(Graph* p_Graph)
    {
        int location_operation, this_machine_finishing_time, earliest_machine = -1;
        int earliest_finishing_time = INT_MAX;
        for (int i = 0; i < number_machines; i++)
        {
            if (machine_number_scheduled[i] == 0) return (0);
            // If there are no operations scheduled then this is
            // the earliest finishing time

            location_operation = sequence[i][machine_number_scheduled[i] - 1];
            // Assigns the id of the operation that was scheduled last.

            this_machine_finishing_time = p_Graph->Get_Finishing_Time(
                operation[location_operation].job, operation[location_operation].step);
            // Returns the finishing time of the [job,step].

            if (this_machine_finishing_time < earliest_finishing_time)
            {
                earliest_machine = i;
                earliest_finishing_time = this_machine_finishing_time;
            }
        }
        return (earliest_finishing_time);
    }

    int Get_Likely_ATC_Operation(Graph*, ATC_Rule*, int);
    void Obtain_Machine(int*, int*, Graph*, Cluster_Operation*);

    int Schedule_Operation(Graph*, int, int);
    int Schedule_Operation(int*, Arc_Backup*, Graph*, int, int, int*, Class_Solution_Oracle*, int);
    void Unschedule_Operation(int, int, Graph*);
    // Augusto 17.10.96
    //        void Unschedule_Operation( int , int , Graph * , Class_Solution_Oracle * , int  ) ;
    void Unschedule_Operation(int, int, Graph*, Class_Solution_Oracle*, int, int);
    //      void Unschedule_Operation( int , int , Graph * ) ;

    void Get_Available_Time(int*, int*, Graph*);
    // Augusto 22.10.96
    //        void Get_Available_Time_Length_Operation(   int * , int * , Graph * , int );

    int Earliest_Finishing(Graph*);
    // Obtains the earliest timing when an operation that has not been scheduled yet would finish.

    //      int Earliest_Completion( Graph * ) ;
    void Clean_Star()
    {
        completion_star = INT_MAX;
        position_star = NULL_LOCATION;
        Omega_i = 0;
    }
    //      int Completion_Star( Graph * , int * , int ) ;
    int Completion_Star(Graph*, int);
    //      int Get_Completion_Star() {
    //              return( completion_star ) ;
    //      }
    int Get_Omega_i()
    {
        return (Omega_i);
    }
    //      int Get_Number_Children( Graph * , int * ) ;
    //      int Mean_Lateness( int * , Graph * , int * ) ;
    //      double Mean_AATC( int * , Graph * , ATC_Rule * , int * ) ;
    //      int Mean_Increase( int * , Graph * , int * , int , int ) ;
    //      int Mean_Estimated_Increase( int * , Graph * , int * ) ;

    int Filter_Rank_Operations(Beam_Indices<Solution_Increment>*, int, Graph*, ATC_Rule*, int, int**, int*);

    //      int **Insert_Arc( Graph * , Class_Disjunctive_Arc * , int ) ;
    // Insert_Arc() : inserts a->b

    //      void Uninsert_Arc( int ** , Graph * , Class_Disjunctive_Arc * , int ) ;

    void Get_Index(int*, int*, Class_Disjunctive_Arc*, Class_Disjunctive_Arc*, Graph*);
    // Get_Index(): returns the index.

    int Fix_Disjunctions(int*, class Arc_Backup*, int, class Graph*);

    int Completion_Star_Marked(Graph*);
    int Predecessors_Are_Scheduled_or_Marked(int, Graph*);
    int Filter_Rank_Operations(Beam_Indices<Solution_Increment>*, int, Graph*, ATC_Rule*);
    int Try_Operation(int*, int**, int, int, Graph*);
    int Fix_Precedences(int*, int**, int**, int*, int, Graph*);
    void Untry_Operation(int);
    int Complete_or_Marked(Graph*);
    //      void Back_Up( int ** , int *** , Graph * ) ;
    void Back_Up(int**);
    //      void Restore( int * , int ** , Graph * ) ;
    void Restore(int*, int, int*, int*, Graph*);
    //      void Restore( int * ) ;
    void Copy_Release_Times();
    void Uncopy_Release_Times();
};

typedef Cluster_Work_Center* type_p_Cluster_Work_Center;

#endif
