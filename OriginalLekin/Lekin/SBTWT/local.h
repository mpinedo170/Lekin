#ifndef LOCAL_H
#define LOCAL_H

#include "act_orcl.h"
#include "dsj_orcl.h"
#include "plant.h"
#include "t_index.h"

class Local
{
    // External data structures:
    Plant* p_Plant;
    Set_Clusters* p_Set_Clusters;

    //	Oracle this_Oracle_Reopt ;
    Act_Oracle this_Oracle_Local;

    // Internal data structures:
    Class_List_Disjunctives list_new_disjunctives;

    Class_List_Disjunctives list_deleted_disjunctives;
    // Records the deleted disjunctive arcs in the reoptimization,
    // so they can be added if the reoptimization does not
    // provide a better value.

    int list_old_machines[MAX_OPERATIONS_CLUSTER];
    // Records the machine of each operation in the reoptimization,
    // so they can be reset if the reoptimization does not
    // provide a better value.

    // Functions:

    void Recursive_Call(int, int, int, int, int, Cluster*, Graph*);
    void Rank_Operations(Beam_Indices<Class_Node_Index>*, Graph*);
    void Get_Cluster(int, int, Cluster*, Graph*);
    int Get_Cluster_Index(Cluster*, int, int);
    void Rank_Clusters(Beam_Indices<Cluster>*, Solution_Graph*, Graph*);
    void Search_Neighborhood(Beam_Indices<Cluster>*, Solution_Graph*, Graph*);
    double Choose_Parameter_K(int, Act_Oracle*, Graph*);

public:
    Local(){};

    void Initialize(Plant*, Set_Clusters*);

    int Reoptimize(Graph*, Cluster*, Act_Oracle*, Dsj_Oracle*, int*);

    void Run(Solution_Graph*, Graph*, Plant*);

    void Shuffle(Solution_Graph*, Graph*);
};

#endif
