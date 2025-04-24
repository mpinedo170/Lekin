#ifndef DSJ_ORCL_H
#define DSJ_ORCL_H

#include "act_orcl.h"
#include "plant.h"
#include "set_cent.h"
#include "set_clus.h"
#include "t_index.h"

class Dsj_Oracle
{
    // Data structures:

    int max_acceptable;  // It continuos with a current solution if it is equal to
                         // or less than max_acceptable.

    int max_allowed;  // It quits the recursion once a solution greater that or
                      // equal to max_allowed is reached.

    //	int time_out ;
    // TRUE if the recursion has stopped since a time out and not
    // because a dead lock.

    //	int cutting_time ;
    // For controling that the process does not take too long.

    int type_oracle;

    Cluster* p_Cluster;
    Graph* p_Graph;
    Plant* p_Plant;
    Set_Clusters* p_Set_Clusters;

    Act_Oracle this_Act_Exact_Orcl;
    Act_Oracle this_Act_Heur_Orcl;

    // Privite functions:

    int Get_Arcs_Center(
        Class_Disjunctive_Arc*, Class_Disjunctive_Arc*, int*, Graph*, int, Set_Work_Centers*, Cluster*);

    void Recursive_Call(
        Class_Solution_Oracle*, int, Class_Disjunctive_Arc*, int, int, Set_Work_Centers**, int*);

    //	int Could_Calculate_Lower_Bound( int , int , int , int , Cluster * , Set_Work_Centers * , Graph * ) ;
    int Can_Get_Mach_Bound(int, int, Cluster*, Graph*);
    int Could_Fix_Disjunctives(int, int);
    int Switch_Active_Branching(int, Graph*);

    //	int Get_Number_Unscheduled( int * , int ) ;
    //	int Get_Bound_N( int * , int * , int , int , int , Cluster * , Graph * )  ;
    //	int Get_Cutting_Time( ) ;
    //	int Time_Out( ) ;

    //	int Get_Children( Beam_Indices<Solution_Increment> * , int ) ;
    //	int Get_Children_Plus( Beam_Indices<Solution_Increment> * , int * , int ** , int , int , int ) ;

public:
    Dsj_Oracle(){};

    void Initialize(int, Plant*, Set_Clusters*);

    void Optimize(Class_Solution_Oracle*, Cluster*, Graph*, int, int);
};

#endif
