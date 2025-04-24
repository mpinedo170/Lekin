#ifndef SET_CLUS_H
#define SET_CLUS_H

#include "cluster.h"
#include "plant.h"

#define ALL_CLUSTERS -1

class Set_Clusters
{
    friend class Optimizer;
    friend class Parameters;

    // Data structures:
    int number_clusters;
    // It will decrease as more clusters are solved.

    // Data structures for sequencing:
    int sequence[MAX_CLUSTERS];
    // Keeps the sequence in which the clusters have been scheduled.
    // sequence[3] = 2 means the 4th cluster to be scheduled was 2.
    // It is used in the reoptimization only.

    int optimal_sequence[MAX_CLUSTERS];
    // Keeps the best sequence..
    // It is used in as an input for the B&B.

    int remaining_clusters;  // Number clusters yet to be scheduled.

    Cluster* cluster;
    //"p_cluster" is an array of pointers to Cluster.
    // The memory for every cluster has been allocated in Plant.h.

    // Functions:
    void Write_Work_Center(Plant*);
    void Check_Recirculation();

    void Clean_Information(Graph*);

public:
    // Functions.
    Set_Clusters();

    void Read_File(char*, Plant*);
    void Initialize(Graph*, Plant*);
    void Terminate();

    void Print_Unscheduled(int, Graph*);
    void Mark_Scheduled(int);
    void Unmark_Scheduled(int);
    int Number_Clusters()
    {
        return (number_clusters);
    }
    int Number_Scheduled()
    {
        return (number_clusters - remaining_clusters);
    }
    int Number_Remaining()
    {
        return (remaining_clusters);
    }
    Cluster* Nth_Scheduled_Cluster(int nth_cluster)
    {
        return (&(cluster[sequence[nth_cluster]]));
    };
    Cluster* Get_Cluster(int cluster_id)
    {
#ifdef CHECK_DEBUG
        assert((cluster_id >= 0) && (cluster_id < number_clusters));
#endif

        return (&(cluster[cluster_id]));
    };
    void Write_Optimal_Sequence()
    {
        for (int i = 0; i < number_clusters; i++) optimal_sequence[i] = sequence[i];
    }
    void Print_Sequence();
    void FPrint_Optimal_Sequence();
    void Copy_Optimal_Sequence();
    void Copy_Given_Sequence(int*);
    void Copy_Flow_Shop_Sequence();
    void Create_Cluster(Cluster*, int*, int, int, Graph*);
    int* Get_Sequence()
    {
        return (sequence);
    }
    int* Get_Optimal_Sequence()
    {
        return (optimal_sequence);
    }
};

#endif
