#include "StdAfx.h"

#include "set_clus.h"

// Set_Clusters( ): Constructor.
Set_Clusters::Set_Clusters()
{
    cluster = NULL;
};

void Set_Clusters::Initialize(Graph* p_Graph, Plant* p_Plant)
{
    int i;

    for (i = 0; i < MAX_CLUSTERS; i++)
    {
        sequence[i] = NULL_CLUSTER;
        optimal_sequence[i] = NULL_CLUSTER;
    }
    remaining_clusters = number_clusters;
    // Since non has been scheduled yet.
}
// void Set_Clusters::Terminate( Graph *p_Graph )
void Set_Clusters::Terminate()
{
    int i;

    //      Clean_Information( p_Graph ) ;
    for (i = 0; i < number_clusters; i++)
    {
        cluster[i].Terminate();
    }
    delete[] cluster;
    cluster = NULL;
}

void Set_Clusters::Print_Unscheduled(int number_jobs, Graph* p_Graph)
{
    int i;

    for (i = 0; i < number_clusters; i++)
        if (cluster[i].is_scheduled == FALSE) cluster[i].Print(number_jobs, p_Graph);
}

void Set_Clusters::Mark_Scheduled(int cluster_id)
{
#ifdef CHECK_DEBUG
    assert(cluster_id < number_clusters);
    assert(remaining_clusters >= 1);
    assert(cluster[cluster_id].is_scheduled == FALSE);
#endif

    cluster[cluster_id].is_scheduled = TRUE;
    // Marked so it can not be scheduled again.

    // WARNING!: redundant marking.

    sequence[number_clusters - remaining_clusters] = cluster_id;
    // It is the (number_clusters - remaining_clusters)th cluster
    // to be scheduled.

    remaining_clusters--;
}

void Set_Clusters::Unmark_Scheduled(int cluster_id)
{
#ifdef CHECK_DEBUG
    assert(cluster[cluster_id].is_scheduled == TRUE);
#endif

    cluster[cluster_id].is_scheduled = FALSE;
    // Marked so it can not be scheduled again.
    sequence[number_clusters - remaining_clusters - 1] = cluster_id;
    // The scheduled cluster is in the
    //(number_clusters - remaining_clusters - 1 )th position.

    remaining_clusters++;
}

void Set_Clusters::Print_Sequence()
{
    printf("(");
    for (int i = 0; i < number_clusters; i++) printf("%d", sequence[i]);
    printf(")\n");
}

void Set_Clusters::FPrint_Optimal_Sequence()
{
    int i;
    printf("{");
    for (i = 0; i < number_clusters; i++) printf("%d", optimal_sequence[i]);
    printf("}\n");

    /*fprintf( par_output_file ,"{" ) ;
    for( i = 0 ; i < number_clusters ; i++ )
            fprintf( par_output_file ,"%d",optimal_sequence[i] ) ;
    fprintf( par_output_file ,"}" ) ;*/
}

void Set_Clusters::Copy_Optimal_Sequence()
{
    par_optimal_sequence = new int[number_clusters];
    for (int i = 0; i < number_clusters; i++)
    {
        // This sequence comes from the shifting bottleneck.

        par_optimal_sequence[i] = -1;
        // Initialize.

        par_optimal_sequence[i] = optimal_sequence[i];
        // Write.
#ifdef CHECK_DEBUG
        assert(par_optimal_sequence[i] < number_clusters);
        assert(par_optimal_sequence[i] >= 0);
#endif
    }
}

void Set_Clusters::Copy_Given_Sequence(int* given_sequence)
{
    par_optimal_sequence = new int[number_clusters];
    for (int i = 0; i < number_clusters; i++)
    {
        // This sequence comes from the shifting bottleneck.

        par_optimal_sequence[i] = -1;
        // Initialize.

        par_optimal_sequence[i] = given_sequence[i];
        // Write.
#ifdef CHECK_DEBUG
        assert(par_optimal_sequence[i] < number_clusters);
        assert(par_optimal_sequence[i] >= 0);
#endif
    }
}

void Set_Clusters::Copy_Flow_Shop_Sequence()
{
    par_optimal_sequence = new int[number_clusters];
    for (int i = 0; i < number_clusters; i++)
    {
        // This sequence comes from the shifting bottleneck.

        par_optimal_sequence[i] = i;
    }
}

void Set_Clusters::Create_Cluster(
    Cluster* p_Cluster, int* this_seq, int pos_seq, int size_shuffle, Graph* p_Graph)
{
    int i, j;
    int number_centers = 0;
    for (i = 0; i < size_shuffle; i++)
        number_centers = number_centers + cluster[this_seq[pos_seq + i]].Get_Number_Work_Centers();

    p_Cluster->Initialize(0, number_centers, p_Graph);
    for (i = 0; i < size_shuffle; i++)
        for (j = 0; j < cluster[this_seq[pos_seq + i]].Get_Number_Work_Centers(); j++)
            p_Cluster->Assign_p_Work_Center(cluster[this_seq[pos_seq + i]].Get_Cluster_Work_Center(j));
}
