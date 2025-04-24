#ifndef OPTIMIZE_H
#define OPTIMIZE_H

#include "local.h"

#ifdef CUISE_CONNECTED
extern "C" {
void read_from_dec();
void CUISE_Repaint();
}
extern int flag_dec, flag_schedule, flag_machine, flag_operation;
#endif

class Optimizer
{
    // External data structures:

    Local* p_Local;
    Plant* p_Plant;
    Set_Clusters* p_Set_Clusters;

    // Internal data structures:
    Class_List_Disjunctives list_new_disjunctives;

    //	Branch this_Branch;
    Act_Oracle this_Act_Exact_Orcl;
    Act_Oracle this_Act_Heur_Orcl;
    // Optimizer of each cluster.
    Dsj_Oracle this_Disj_Exact_Orcl;
    Dsj_Oracle this_Disj_Heur_Orcl;

    int cutting_time;
    int highest_machine_bound;  // Highest one machine lower bound.
                                // If equal to the optimal solution, then we quit.

    // Functions:

    int Get_Cutting_Time(int);
    int Time_Out();
    double Choose_Parameter_K(Graph*, int);

    void Rank_Work_Centers(Cluster*, Graph*);
    void Rank_Clusters(int*, Beam_Indices<Class_Solution_Oracle>*, Graph*, int);
    void Re_Rank_Clusters(int*, Beam_Indices<Class_Solution_Oracle>*, Graph*);

    void Get_Better_Solution(Class_Solution_Oracle*, Cluster*, Graph*, int, double);

    void Reoptimize(Graph*, int*);
    void Reoptimize_Plus(Graph*, int*);
    void Reoptimize_All(Graph*, int*);

    void Recursive_Call(Solution_Graph*, Graph*, int);

public:
    Optimizer(){};

    void Initialize(Plant*, Set_Clusters*);
    int Single_Lower_Bound(Graph*);
    void Optimize(Solution_Graph*, Graph*, Local*);
};

#endif
