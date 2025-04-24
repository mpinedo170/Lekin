#ifndef INT_CUIS_H
#define INT_CUIS_H

#include "const.h"
#include "graph.h"
#include "list_dis.h"
#include "plant.h"

struct Manual_Solution_Struct
{
    int work_center_id;
    int length_solution[MAX_MACHINES_WORK_CENTER];
    int solution[MAX_MACHINES_WORK_CENTER][MAX_OPERATIONS_WORK_CENTER][2];
};

#endif
