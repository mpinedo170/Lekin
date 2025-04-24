#ifndef PARAMET_H
#define PARAMET_H

#include "graph.h"
#include "lib_func.h"
#include "plant.h"
#include "set_clus.h"

#define LENGTH_INPUT_CHAR 100

class Parameters
{
    char* Get_Char_Parameter(char*, FILE*);
    int Get_Int_Parameter(char*, FILE*);
    double Get_Float_Parameter(char*, FILE*);

public:
    Parameters(){};
    void Read_General(int*, int*, int*);
    void Read_Result(int, int, int);
    void Read_General(char*);
    void Read_Other();
    void Read_Static_Data(Graph*, Plant*, int);
    void Read_Heuristic_Data(Graph*, Plant*, Set_Clusters*);
    void Read_Lower_Bound_Data(Graph*, Plant*, Set_Clusters*);
};

#endif
