#include "StdAfx.h"

#include "int_cuis.h"

extern void dec_input(Graph**, Plant**);

extern "C" {

// struct Graph *p_optimal_Graph ;                       //It will store the optimal solution so far.
// struct Plant *p_main_Plant ;                  //Stores the data of work centers.
class Graph* p_optimal_Graph;  // It will store the optimal solution so far.
class Plant* p_main_Plant;     // Stores the data of work centers.

struct Manual_Solution_Struct this_Manual_Solution_Struct;
}

// int flag_dec, flag_schedule , flag_machine , flag_operation ;

void Manual_Reschedule(Graph* p_Graph, Plant* p_Plant, Manual_Solution_Struct* p_Manual_Solution_Struct)
{
#ifdef CUISE_CONNECTED
    // For CUISE interface.

    Class_List_Disjunctives list_disjunctives;

    int work_center_id = p_Manual_Solution_Struct->work_center_id;
    int dead_lock;
    Graph backup_Graph;

    backup_Graph.Allocate_Memory(p_Graph);
    backup_Graph = *p_Graph;

    p_Plant->Delete_Disjunctives_Work_Center(p_Graph, work_center_id);
    p_Plant->Unset_Machine_Is_Scheduled_Work_Center(p_Graph, work_center_id);

    p_Plant->Define_Disjunctives_Work_Center(&list_disjunctives, p_Manual_Solution_Struct);
    p_Plant->Set_Machine_Is_Scheduled_Work_Center(p_Graph, p_Manual_Solution_Struct);
    p_Graph->Add_New_Disjunctives(&list_disjunctives);

    p_Graph->Get_Forward_Windows(&dead_lock);
    p_Graph->Get_Backward_Windows();
    if (dead_lock == TRUE)
    {
        p_Graph->Terminate();
        p_Graph->Allocate_Memory(&backup_Graph);
        *p_Graph = backup_Graph;
        printf("DEAD LOCK: SCHEDULE INFEASIBLE\n");
    }
    backup_Graph.Terminate();
    p_Plant->Show_Gantt_Chart("gantt", p_Graph);

#endif
}

extern "C" {
void DEC_Optimize(Graph** p_p_Graph, Plant** p_p_Plant)
{
    //              dec_input( p_p_Graph , p_p_Plant );
}
void DEC_Manual_Reschedule(Graph* p_Graph, Plant* p_Plant, Manual_Solution_Struct* p_Manual_Solution_Struct)
{
    Manual_Reschedule(p_Graph, p_Plant, p_Manual_Solution_Struct);
}
}
