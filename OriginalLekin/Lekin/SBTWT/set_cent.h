#ifndef SET_CENT_H
#define SET_CENT_H

#include "center.h"
#include "globals.h"
#include "t_index.h"

//#define ANY_WORK_CENTER -1

#ifdef CUISE_CONNECTED

extern "C" {
void read_from_dec();
void CUISE_Repaint();
}

extern int flag_dec, flag_schedule, flag_machine, flag_operation;

#endif

//----------------------------------------------------------------------------
class Work_Centers_Info
{
    friend class Set_Work_Centers;

    // Data structures:

    int id;
    int result;
    int number_unscheduled;

    // Private functions:

public:
    Work_Centers_Info()
    {}

    Work_Centers_Info(int input_work_center_id, int input_work_center_result, int input_number_unscheduled)
    {
        id = input_work_center_id;
        result = input_work_center_result;
        number_unscheduled = input_number_unscheduled;
    }
    void Terminate()
    {}
    void operator=(const Work_Centers_Info& from)
    {
        id = from.id;
        result = from.result;
        number_unscheduled = from.number_unscheduled;
    }
    int operator==(const Work_Centers_Info& from)
    {
        if ((id != from.id) || (result != from.result) || (number_unscheduled != from.number_unscheduled))
            return (FALSE);
        else
            return (TRUE);
    }
};

//---------------------------------------------------------------------------
class Set_Work_Centers
{
    // Data structures:

    // Private functions:

    Beam_Indices<Work_Centers_Info> list_work_centers;

    int number_centers_cluster;

    int machine_effective[5];

    int machine[5];

    // Private functions:

    int Number_Combinations_Two();
    int Number_Combinations_Three();
    int Number_Combinations_Four();
    int Number_Combinations_Five();

    void ith_Best_Positions_Two(int*, int);
    void ith_Best_Positions_Three(int*, int);
    void ith_Best_Positions_Four(int*, int);
    void ith_Best_Positions_Five(int*, int);

public:
    Set_Work_Centers(){};

    void Initialize(int);
    void Terminate();
    void Add(int, int, int, int);
    //	void operator=(const Set_Work_Centers& ) ;
    void Get_Sequence(Set_Work_Centers*);

    void Get_Highest_Locations_One(int*, int*, int*, int);
    void Get_Just_Highest_Locations_One(int*, int*, int*, int, int);
    void Get_Highest_Locations_Two(int*, int*, int);
    void Get_Highest_Locations_Three(int*, int*, int);
    void Get_Highest_Locations_Four(int*, int*, int);
    void Get_Highest_Locations_Five(int*, int*, int);

    int Get_Number_Bounds_One(int, int);
    int Get_Number_Bounds_Two(int, int);
    int Get_Number_Bounds_Three(int, int);
    int Get_Number_Bounds_Four(int, int);
    int Get_Number_Bounds_Five(int, int);

    void Set_N_Effective(int*, int);
    void Get_N_Effective(int*, int);
    int N_Machine_Effective(int);
    int Some_Machine_Effective();

    int Highest_Bound_Center(int);
};

#endif
