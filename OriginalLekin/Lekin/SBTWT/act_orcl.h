#ifndef ACT_ORCL_H
#define ACT_ORCL_H

#include "plant.h"
#include "set_cent.h"
#include "set_clus.h"
#include "t_index.h"

//#define ANY_WORK_CENTER -1

#ifdef CUISE_CONNECTED

extern "C" {
void read_from_dec();
void CUISE_Repaint();
}

extern int flag_dec, flag_schedule, flag_machine, flag_operation;

#endif

//-------------------------------------------------------------------------------------
class Forbidden_Set
{
    // Data structures:

    int number_centers;

    int** forbidden;  // forbidden[2][4] == 3 means that when branching on machine 2,
                      // If the  last operation is 4 then operation 3 cannot be scheduled next.

public:
    Forbidden_Set(){};

    void Initialize(Cluster* p_Cluster)
    {
        int i, j;

        number_centers = p_Cluster->Get_Number_Work_Centers();
        forbidden = new type_p_int[number_centers];
        for (i = 0; i < number_centers; i++)
        {
            forbidden[i] = new int[p_Cluster->Get_Number_Operations_Work_Center(i)];
            for (j = 0; j < p_Cluster->Get_Number_Operations_Work_Center(i); j++)
                forbidden[i][j] = NULL_LOCATION;
        }
    }

    void Terminate()
    {
        for (int i = 0; i < number_centers; i++)
        {
            delete[] forbidden[i];
        }
        delete[] forbidden;
    }

    int* Back_Up(Cluster* p_Cluster, int center)
    {
        // Makes a copy of the information of the center.

        int number_operations = p_Cluster->Get_Number_Operations_Work_Center(center);
        int* backup = new int[number_operations];
        for (int i = 0; i < number_operations; i++)
        {
            backup[i] = forbidden[center][i];
            forbidden[center][i] = NULL_LOCATION;
        }
        return (backup);
    }

    void Recover_Data(int* backup, Cluster* p_Cluster, int center)
    {
        // Recover the information from the backup.

        int number_operations = p_Cluster->Get_Number_Operations_Work_Center(center);
        for (int i = 0; i < number_operations; i++)
        {
            forbidden[center][i] = backup[i];
        }
    }

    //      int Is_Allowed_Sequence( int j , int k , int center , int *backup ) {
    int Is_Allowed_Sequence(int j, int k, int* backup)
    {
        // TRUE if sequence j->k is has not been eliminated for the machine.
        // backup contains the information.
        // forbidden[center] is filled up with new information.

        if (backup[j] == k)
            return (FALSE);
        else
            return (TRUE);
    }

    void Forbid_Sequence(int j, int k, int center)
    {
        // TRUE id sequence j->k is has not been eliminated for the machine.
        forbidden[center][j] = k;
    }

    int** Get_Forbidden()
    {
        return (forbidden);
    }
};

//-------------------------------------------------------------------------------------

class Act_Oracle
{
    // Data structures:

    int max_acceptable;  // It continuos with a current solution if it is equal to
                         // or less than max_acceptable.

    int max_allowed;  // It quits the recursion once a solution greater that or
                      // equal to max_allowed is reached.

    int time_out;
    // TRUE if the recursion has stopped since a time out and not
    // because a dead lock.

    // SOLEX int cutting_time ;
    int cutting_time;
    // For controling that the process does not take too long.

    int type_oracle;  //#define HEURISTIC_ORACLE      0
                      //#define EXACT_ORACLE          1
                      //#define BOUND_ORACLE          2

    int flag_fix_arcs;

    Cluster* p_Cluster;
    Graph* p_Graph;
    Plant* p_Plant;
    Set_Clusters* p_Set_Clusters;

    ATC_Rule this_ATC_Rule;
    Forbidden_Set this_Forbidden_Set;

    // Privite functions:

    void Recursive_Call(Class_Solution_Oracle*,

        Class_Solution_Oracle*,
        Solution_Increment*,
        int,
        int,
        Set_Work_Centers**,
        int*);

    int Could_Calculate_Lower_Bound(int, int, Cluster*, Set_Work_Centers*, int);
    int Could_Fix_Disjunctives(int, int);

    int Get_Number_Unscheduled(int*, int, Cluster*);

    int Get_Bound_N(int*, int*, int, int, int, Cluster*, Graph*);

    // SOLEX int Get_Cutting_Time( int ) ;
    int Get_Cutting_Time(int);

    int Time_Out();

    int Get_Children(Beam_Indices<Solution_Increment>*, int);
    int Get_Children_Plus(Beam_Indices<Solution_Increment>*, int*, int**, int, Set_Work_Centers*);

public:
    Act_Oracle(){};

    void Initialize(int, Plant*, Set_Clusters*);

    void Optimize(Class_Solution_Oracle*, Cluster*, Graph*, int, double, int, int, Set_Work_Centers*);

    int Can_Get_Mach_Bound(int, int, int, Cluster*, Graph*);
    int Single_Mach_Bound(Set_Work_Centers**, Set_Work_Centers**, int, Cluster*, Graph*);
    int Get_Lower_Bound(Set_Work_Centers**, Set_Work_Centers**, int, int, int, Cluster*, Graph*);

    void Set_Flag_Fix_Arcs(int flag)
    {
        flag_fix_arcs = flag;
    }

    void Recursive_Call(
        int*, int, Class_Solution_Oracle*, Class_Solution_Oracle*, int, int, int, int, Cluster_Work_Center*);
    void Optimize(Class_Solution_Oracle*, Cluster*, Graph*, int, double, int, int);
    //      void Set_Fix_Precedences(int input_this_par_fix_precedence ) {
    //              this_par_fix_precedence = input_this_par_fix_precedence ;
    //      }
};

#endif
