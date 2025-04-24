#ifndef PLANT_H
#define PLANT_H

#include "center.h"
#include "globals.h"
#include "graph.h"
#include "int_cuis.h"

#define ALL_WORK_CENTERS -1

#define REOPTIMIZING -1

class Plant
{
    friend class Cluster_Work_Center;
    friend class Parameters;
    friend class Set_Clusters;

    // Data structures for work centers.

    int number_work_centers;
    // number of work centers in the plant.

    //      Work_Center *plant_Work_Centers ;
    // set of work centers in the plant.

    Cluster_Work_Center* cluster_work_center;
    // Set of work centers in the cluster.

    // Three functions called by Plant::Plant().
    void Read_File(char*);

public:
    Plant();

    void Initialize(char*, Graph*);
    void Terminate();

    void Show_Gantt_Chart(char*, Graph*);
    // Writes the input for CUISE.

    int Fix_Disjunctions(int*, Arc_Backup*, int, Graph*);

    int Get_Number_Work_Centers()
    {
        return (number_work_centers);
    }
    void Copy_Flow_Shop_Sequence();

/*      int Get_Number_Machines_Work_Center( int work_center_id )
        {
                return( cluster_work_center[ work_center_id ].number_machines ) ;
        }
*/
#ifdef CUISE_CONNECTED
    // For CUISE interface.

    void Delete_Disjunctives_Work_Center(Graph*, int);
    void Unset_Machine_Is_Scheduled_Work_Center(Graph*, int);
    void Define_Disjunctives_Work_Center(Class_List_Disjunctives*, struct Manual_Solution_Struct*);
    void Set_Machine_Is_Scheduled_Work_Center(Graph*, struct Manual_Solution_Struct*);

#endif
};

#endif
