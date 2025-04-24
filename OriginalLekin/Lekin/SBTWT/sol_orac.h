#ifndef SOL_ORAC_H
#define SOL_ORAC_H

#include "const.h"
#include "globals.h"
#include "node.h"

#define NULL_CLUSTER -1

//-----------------------------------------------------------------------------------------------

class Class_Solution_Oracle
{
    friend class Cluster;

    // Data structures:
    int value;     // Objective value for the solution.
    int time_out;  // TRUE if there was a time out in the Oracle .
    int cluster_id;

    int number_work_centers;
    int* number_machines_work_center;

    int** length_solution;
    int*** solution;

public:
    //      Class_Solution_Oracle( Cluster * ) ;
    Class_Solution_Oracle();
    //      ~Class_Solution_Oracle( ) ;
    void Terminate();
    //      void Compress( ) ;
    void operator=(const Class_Solution_Oracle&);
    int operator==(const Class_Solution_Oracle&);
    void Write(int, int, int);
    void Unwrite(int, int, int);
    int Nth_Operation(int, int, int);
    // Nth_Operation(): Obtain the location in the set of operations of the operation
    // with a given postition in a machine of a work center.

    void Set_Objective(int input_value)
    {
        value = input_value;
    };
    int Get_Objective()
    {
        return (value);
    };
    void Set_Time_Out(int input_time_out)
    {
        time_out = input_time_out;
    };
    int Get_Time_Out()
    {
        return (time_out);
    };
    void Write_Cluster(int input_cluster_id)
    {
        cluster_id = input_cluster_id;
    };
    int Get_Cluster_Id()
    {
        return (cluster_id);
    }

    //      void Print( Cluster * ) ;
    void Print();

    //      int *Get_Min_Release_Times() ;
    //      int *Get_Finishing_Times() ;
    int Is_Optimal_Predecessor();
};

#endif
