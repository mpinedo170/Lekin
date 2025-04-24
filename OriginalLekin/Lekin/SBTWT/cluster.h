#ifndef CLUSTER_H
#define CLUSTER_H

#include "center.h"
#include "set_cent.h"
#include "sol_orac.h"

#define NULL_CLUSTER -1

//----------------------------------------------------------------------------

class List_Int
{
    int dummy;

public:
    List_Int(){};
    void Terminate(){};
    int operator==(const List_Int& nothing)
    {
        return (FALSE);
    };
};

//----------------------------------------------------------------------------

// Cluster: set of operations that represent a subproblem.
// Cluster and Subproblem are interchangeble.
class Cluster
{
    friend class Optimizer;

    friend class Set_Clusters;
    // For Chart::Schedule().

    friend class Class_Solution_Oracle;

    // Data structures:

    int id;  // The name of the cluster. Start from 0,1,2..., etc.

    int number_work_centers;
    // number of work centers the cluster has.

    Cluster_Work_Center** p_work_center;
    // Set of work centers in the cluster.

    //      int *rank_work_centers ;

    int number_operations;
    // number of steps assigned to this cluster.

    int number_machines;
    // Summation of the number of machines of each center.

    int is_scheduled;
    // TRUE if it was already scheduled.

    double step_length_average;

    //      int *set_work_centers ;
    // For the forward windows in the graph.

    int current_position;
    // Keeps the position where the next pointer to Cluster_Work_Center
    // will be stored.

    //      int **operation ;
    //                      //operation[i][j] = CHECK_TRUE if operation (i,j) belongs in the cluster.

    int number_jobs;
    // number of jobs in the problem. Helps to delete the operation[][].

    Class_Node_Index original_operation;
    // The operations that propagates the cluster.

    // Functions:

public:
    Cluster();

    // Functions used by Set_Clusters:
    void Set_Work_Center(int, int);
    void Initialize(int, int, Graph*);
    int operator==(const Cluster&);

    void Assign_p_Work_Center(Cluster_Work_Center*);

    //      int **Cluster::Get_Operations() {
    //              return( operation ) ;
    //      }

    void Calculate_Step_Length_Average();
    //      void Terminate_Set( ) ;
    void Terminate();  // Used in local search. It terminates the work centers also.
    void Initialize_Solution(Class_Solution_Oracle*);
    void Copy_Solution(Class_Solution_Oracle*);

    void Schedule_Solution(Graph*, Class_Solution_Oracle*);
    void Unschedule_Solution(Graph*);
    void Read_File(Graph*, Plant*);
    // For Local.C

    void Define_Disjunctives(Class_List_Disjunctives*, Class_Solution_Oracle*);

    //      int Earliest_Finishing_Work_Center( Graph * , int ) ;
    int Earliest_Finishing_Work_Center(Graph*);
    int Earliest_Completion_Work_Center(Graph*);
    int Least_Children_Work_Center(Graph*, int**);
    int Max_Mean_Lateness_Work_Center(Graph*, int**);
    int Max_Mean_Increase_Work_Center(Graph*, int**, int, int);
    int Max_Mean_Estimated_Increase_Center(Graph*, int**);
    //      int Bottleneck_Work_Center( Graph * , int ** ) ;
    //      int Highest_Bound_Center( Graph * , int ** , Set_Work_Centers * ) ;
    int Bottleneck_Work_Center(Graph*);
    int Highest_Bound_Center(Graph*, Set_Work_Centers*);
    //      void Rank_Centers( Graph * ) ;

    int Arcs_Max_Increase_Center(Class_Disjunctive_Arc*, Class_Disjunctive_Arc*, int*, Graph*, int);
    int Arcs_Bottleneck_Center(Class_Disjunctive_Arc*, Class_Disjunctive_Arc*, int*, Graph*, int);
    int Arcs_Max_Bound_Center(
        Class_Disjunctive_Arc*, Class_Disjunctive_Arc*, int*, Graph*, int, Set_Work_Centers*);
    //      int Get_Arcs_Center( Class_Disjunctive_Arc *, Class_Disjunctive_Arc * , int * , Graph * , int ,
    //      Set_Work_Centers * ) ;
    int Fix_Disjunctions(int*, Arc_Backup*, int, Graph*);
    void Back_Up_Matrix(int****, int**, int*, int****, int*, Graph*);
    //      void Restore_Graph_Matrix( int *** , int * , int , int *** , int , Graph * ) ;
    void Delete_Graph_Matrix(Graph*);
    void Print(int, Graph*);

    // Auxiliary functions:
    int Get_Id()
    {
        return (id);
    }
    int Get_Number_Work_Centers()
    {
        return (number_work_centers);
    }

    int Get_Number_Machines()
    {
        // Augusto 30-09-96
        //                number_machines = 0 ;
        //                for ( int i = 0 ; i < number_work_centers ; i++ ) {
        //                        number_machines = number_machines + p_work_center[ i ]->number_machines ;
        //                }
        return (number_machines);
    }

    int Get_Number_Operations()
    {
        return (number_operations);
    }
    Cluster_Work_Center* Get_Cluster_Work_Center(int location_work_center)
    {
        return (p_work_center[location_work_center]);
    }

    double Get_Step_Length_Average()
    {
        if (step_length_average == -1) Calculate_Step_Length_Average();

        return (step_length_average);
    }
    //
    int Get_Number_Scheduled_Operations();
    int Get_Number_Unscheduled_Operations();

    void Set_Original(int input_job, int input_step)
    {
        original_operation.job = input_job;
        original_operation.step = input_step;
    }
    Class_Node_Index* Get_p_Original()
    {
        return (&original_operation);
    }
    int Get_Number_Operations_Work_Center(int location_work_center)
    {
        return ((p_work_center[location_work_center])->Get_Number_Operations());
    }
    int Get_Number_Unscheduled_Work_Center(int location_work_center)
    {
        return ((p_work_center[location_work_center])->Get_Number_Unscheduled_Operations());
    }
    //      void Create_Rank_Centers() {
    //              rank_work_centers = new int[ number_work_centers ] ;
    //      }
    //      void Set_Rank_Center( int rank , int center ) {
    //                                      #ifdef CHECK_DEBUG
    //                                              assert( center < number_work_centers ) ;
    //                                      #endif
    //              rank_work_centers[ rank ] = center ;
    //      }
    int Valid_Answer(Graph*);
    // TRUE if the selection of arcs is a tour.
    // A tour visits all the nones once with no cycle.
};

#endif
