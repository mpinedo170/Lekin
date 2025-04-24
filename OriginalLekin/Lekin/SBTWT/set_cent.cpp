#include "StdAfx.h"

#include "set_cent.h"

//#define ANY_WORK_CENTER -1

#ifdef CUISE_CONNECTED

extern "C" {
void read_from_dec();
void CUISE_Repaint();
}

extern int flag_dec, flag_schedule, flag_machine, flag_operation;

#endif

//-------------------------------------------------------------------------------------

void Set_Work_Centers::Initialize(int input_number_centers_cluster)
{
    int i;
    number_centers_cluster = input_number_centers_cluster;

    list_work_centers.Initialize(number_centers_cluster,  // Number of elements.
        number_centers_cluster,                           // Maximum branches.
        0);                                               // Threshold.

    for (i = 0; i < 5; i++)
    {
        machine_effective[i] = FALSE;
        machine[i] = -1;
    }
}

void Set_Work_Centers::Terminate()
{
    list_work_centers.Terminate();
}

void Set_Work_Centers::Add(
    int input_center_id, int input_center_result, int input_number_unscheduled, int location)
{
    Work_Centers_Info* p_Work_Centers_Info =
        new Work_Centers_Info(input_center_id, input_center_result, input_number_unscheduled);

    list_work_centers.Add_Element(
        double(pos(input_center_result - input_number_unscheduled)), p_Work_Centers_Info, location);
}

// void Set_Work_Centers::operator=(const Set_Work_Centers& from )
void Set_Work_Centers::Get_Sequence(Set_Work_Centers* p_Set_Work_Centers)
{
    int i;
    Work_Centers_Info* p_Work_Centers_Info;

    for (i = 0; i < p_Set_Work_Centers->list_work_centers.Number_Elements_List(); i++)
    {
        p_Work_Centers_Info = p_Set_Work_Centers->list_work_centers.Get_p_ith_Element(i);
        Add(p_Work_Centers_Info->id, p_Work_Centers_Info->result, p_Work_Centers_Info->number_unscheduled,
            p_Work_Centers_Info->id);
    }
}

int Set_Work_Centers::Get_Number_Bounds_One(int bound_number,  // Position in the WHILE.
    int max_number_bounds_one)
{
    if (par_max_number_unscheduled_one < 1) return (0);

    if (bound_number == 0)
    {  // There is no previous sequence or
       // It is the first sibling for which a lower bound is calculated.

        //              return( number_centers_cluster  ) ;
        return (min2(number_centers_cluster, max_number_bounds_one));
    }
    else
    {
        if (machine_effective[0] == TRUE)
        {
#ifdef CHECK_DEBUG
            assert(machine_effective[1] == FALSE);
            assert(machine_effective[2] == FALSE);
            assert(machine_effective[3] == FALSE);
            assert(machine_effective[4] == FALSE);
#endif

            //                      return( 1 ) ;   //Only the previous effective bound is tried.
            return (2);  // Only the previous effective bound is tried
                         // and/or the branching center.
        }
        else
        {
            return (0);
        }
    }
}

int Set_Work_Centers::Get_Number_Bounds_Two(int bound_number,  // Position in the WHILE.
    int max_number_bounds_two)
{
    if (par_max_number_unscheduled_two < 1) return (0);

    if ((bound_number == 0)  // It is the first sibling for which a lower bound is calculated.
        || ((machine_effective[0] == TRUE)
               // One machine effective.
               && (list_work_centers.Number_Elements_List() ==
                      number_centers_cluster))  // The work centers have not been ranked.
    )
    {
        if ((number_centers_cluster <= 3) || (number_centers_cluster == 5))
            return (0);  // Allows only 4, 6, 7, 8, ....
        else
            return (min2(Number_Combinations_Two(), max_number_bounds_two));
    }
    else if (machine_effective[1] == TRUE)
    {
        // Two machine effective.

        return (1);  // Try the one that was effective before.
    }
    else
    {
        return (0);
    }
}

int Set_Work_Centers::Get_Number_Bounds_Three(int bound_number,  // Position in the WHILE.
    int max_number_bounds_three)
{
    if (par_max_number_unscheduled_three < 1) return (0);

    if ((bound_number == 0)  // It is the first sibling for which a lower bound is calculated.
        || (((machine_effective[0] == TRUE)
                // One machine effective.
                || (machine_effective[1] == TRUE)
                // Two machine effective.
                ) &&
               (list_work_centers.Number_Elements_List() ==
                   number_centers_cluster))  // The work centers have not been ranked.
    )
    {
        if (number_centers_cluster < 5)
            return (0);  // Allows only 5, 6, 7, 8, ....
        else
            return (min2(Number_Combinations_Three(), max_number_bounds_three));
    }
    else if (machine_effective[2] == TRUE)
    {
        // Three machine effective.

        return (1);  // Try the one that was effective before.
    }
    else
    {
        return (0);
    }
}

int Set_Work_Centers::Get_Number_Bounds_Four(int bound_number,  // Position in the WHILE.
    int max_number_bounds_four)
{
    if (par_max_number_unscheduled_four < 1) return (0);

    if ((bound_number == 0)  // It is the first sibling for which a lower bound is calculated.
        || (((machine_effective[0] == TRUE)
                // One machine effective.
                || (machine_effective[1] == TRUE)
                // Two machine effective.
                || (machine_effective[2] == TRUE)
                // Three machine effective.
                ) &&
               (list_work_centers.Number_Elements_List() ==
                   number_centers_cluster))  // The work centers have not been ranked.
    )
    {
        if (number_centers_cluster <= 5)
            return (0);  // Allows only 6, 7, 8, ....
        else
            return (min2(Number_Combinations_Four(), max_number_bounds_four));
    }
    else if (machine_effective[3] == TRUE)
    {
        // Four machine effective.

        return (1);  // Try the one that was effective before.
    }
    else
    {
        return (0);
    }
}

int Set_Work_Centers::Get_Number_Bounds_Five(int bound_number,  // Position in the WHILE.
    int max_number_bounds_five)
{
    if (par_max_number_unscheduled_five < 1) return (0);

    if ((bound_number == 0)  // It is the first sibling for which a lower bound is calculated.
        || (((machine_effective[0] == TRUE)
                // One machine effective.
                || (machine_effective[1] == TRUE)
                // Two machine effective.
                || (machine_effective[2] == TRUE)
                // Three machine effective.
                || (machine_effective[3] == TRUE)
                // Four machine effective.
                ) &&
               (list_work_centers.Number_Elements_List() ==
                   number_centers_cluster))  // The work centers have not been ranked.
    )
    {
        if (number_centers_cluster <= 6)
            return (0);  // Allows only 7, 8, ....
        else
            return (min2(Number_Combinations_Five(), max_number_bounds_five));
    }
    else if (machine_effective[4] == TRUE)
    {
        // Five machine effective.

        return (1);  // Try the one that was effective before.
    }
    else
    {
        return (0);
    }
}

void Set_Work_Centers::Get_Highest_Locations_One(
    int* best_location_one, int* p_result, int* p_number_unscheduled, int priority)
{
    static int sorted_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    if (list_work_centers.Number_Elements_List() == number_centers_cluster)
    {
        // If is a set with elements.
        Work_Centers_Info* p_Work_Centers_Info = list_work_centers.Get_p_ith_Element(priority);
        best_location_one[0] = p_Work_Centers_Info->id;
        *p_result = p_Work_Centers_Info->result;
        *p_number_unscheduled = p_Work_Centers_Info->number_unscheduled;
    }
    else
    {
        // There is no prevoius sequence.
        best_location_one[0] = sorted_array[priority];
        *p_result = -1;
        *p_number_unscheduled = -1;
    }
}

void Set_Work_Centers::Get_Just_Highest_Locations_One(
    int* best_location_one, int* p_result, int* p_number_unscheduled, int priority, int branching_center)
{
    static int sorted_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    if (priority == 0)
    {
        best_location_one[0] = branching_center;
        *p_result = -1;
        *p_number_unscheduled = -1;
    }
    else
    {  // priority > 0

        if (list_work_centers.Number_Elements_List() == number_centers_cluster)
        {
            // If is a set with elements.
            Work_Centers_Info* p_Work_Centers_Info = list_work_centers.Get_p_ith_Element(priority - 1);
            if (p_Work_Centers_Info->id != branching_center)
            {
                best_location_one[0] = p_Work_Centers_Info->id;
                *p_result = p_Work_Centers_Info->result;
                *p_number_unscheduled = p_Work_Centers_Info->number_unscheduled;
            }
            else
            {
                Work_Centers_Info* p_Work_Centers_Info = list_work_centers.Get_p_ith_Element(priority);
                best_location_one[0] = p_Work_Centers_Info->id;
                *p_result = p_Work_Centers_Info->result;
                *p_number_unscheduled = p_Work_Centers_Info->number_unscheduled;
            }
        }
        else
        {
            // There is no prevoius sequence.
            best_location_one[0] = sorted_array[priority - 1];
            if (best_location_one[0] == branching_center) best_location_one[0] = sorted_array[priority];
            *p_result = -1;
            *p_number_unscheduled = -1;
        }
    }
}

// Number_Combinations_Two : returns the number of combinations of two machines
// given the number of elements in the list.
int Set_Work_Centers::Number_Combinations_Two()
{
    switch (list_work_centers.Number_Non_Zero_Elements())
    {
        case 0:
        case 1:
            return (0);
        case 2:
            return (1);
        case 3:
            return (3);
        default:
            return (5);  // No more than 5 bounds are allowed.
    }
}

// ith_Best_Positions_Two: gets the ith best two positions in the list.
void Set_Work_Centers::ith_Best_Positions_Two(int* position, int ith)
{
    switch (ith)
    {
        case 0:
            position[0] = 0;
            position[1] = 1;
            break;
        case 1:
            position[0] = 0;
            position[1] = 2;
            break;
        case 2:
            position[0] = 1;
            position[1] = 2;
            break;
        case 3:
            position[0] = 0;
            position[1] = 3;
            break;
        case 4:
            position[0] = 1;
            position[1] = 3;
            break;
    }
}

void Set_Work_Centers::Get_Highest_Locations_Two(int* location, int* p_sum_values, int position)
{
    int best_position[2];
    Work_Centers_Info* p_Work_Centers_Info[2];

#ifdef CHECK_DEBUG
    assert(position < Number_Combinations_Two());
#endif

    if (machine_effective[1] == TRUE)
    {
        // Two machine effective.
        // Is due to a previous effective lower bound.

#ifdef CHECK_DEBUG
        assert(position == 0);
#endif

        location[0] = machine[0];
        location[1] = machine[1];

        *p_sum_values = INT_MAX;
    }
    else
    {
#ifdef CHECK_DEBUG
        if (list_work_centers.Number_Elements_List() != number_centers_cluster)

            assert(list_work_centers.Number_Elements_List() == number_centers_cluster);
#endif

        ith_Best_Positions_Two(best_position, position);

        p_Work_Centers_Info[0] = list_work_centers.Get_p_ith_Element(best_position[0]);
        p_Work_Centers_Info[1] = list_work_centers.Get_p_ith_Element(best_position[1]);

        location[0] = p_Work_Centers_Info[0]->id;
        location[1] = p_Work_Centers_Info[1]->id;

        *p_sum_values = p_Work_Centers_Info[0]->result + p_Work_Centers_Info[1]->result;
    }
}

// Number_Combinations_Three : returns the number of combinations of two machines
// given the number of elements in the list.
int Set_Work_Centers::Number_Combinations_Three()
{
    switch (list_work_centers.Number_Non_Zero_Elements())
    {
        case 0:
        case 1:
        case 2:
            return (0);
        case 3:
            return (1);
        default:
            return (4);  // No more than 4 bounds are allowed.
    }
}

// ith_Best_Positions_Three: gets the ith best three positions in the list.
void Set_Work_Centers::ith_Best_Positions_Three(int* position, int ith)
{
    switch (ith)
    {
        case 0:
            position[0] = 0;
            position[1] = 1;
            position[2] = 2;
            break;
        case 1:
            position[0] = 0;
            position[1] = 1;
            position[2] = 3;
            break;
        case 2:
            position[0] = 0;
            position[1] = 2;
            position[2] = 3;
            break;
        case 3:
            position[0] = 1;
            position[1] = 2;
            position[2] = 3;
            break;
    }
}

void Set_Work_Centers::Get_Highest_Locations_Three(int* location, int* p_sum_values, int position)
{
    int best_position[3];
    Work_Centers_Info* p_Work_Centers_Info[3];

#ifdef CHECK_DEBUG
    assert(position < Number_Combinations_Three());
#endif

    if (machine_effective[2] == TRUE)
    {
        // Is due to a previous effective lower bound.

#ifdef CHECK_DEBUG
        assert(position == 0);
#endif

        location[0] = machine[0];
        location[1] = machine[1];
        location[2] = machine[2];

        *p_sum_values = INT_MAX;
    }
    else
    {
#ifdef CHECK_DEBUG
        assert(list_work_centers.Number_Elements_List() == number_centers_cluster);
#endif

        ith_Best_Positions_Three(best_position, position);

        p_Work_Centers_Info[0] = list_work_centers.Get_p_ith_Element(best_position[0]);
        p_Work_Centers_Info[1] = list_work_centers.Get_p_ith_Element(best_position[1]);
        p_Work_Centers_Info[2] = list_work_centers.Get_p_ith_Element(best_position[2]);

        location[0] = p_Work_Centers_Info[0]->id;
        location[1] = p_Work_Centers_Info[1]->id;
        location[2] = p_Work_Centers_Info[2]->id;

        *p_sum_values =
            p_Work_Centers_Info[0]->result + p_Work_Centers_Info[1]->result + p_Work_Centers_Info[2]->result;
    }
}

// Number_Combinations_Four : returns the number of combinations of four machines
// given the number of elements in the list.
int Set_Work_Centers::Number_Combinations_Four()
{
    switch (list_work_centers.Number_Non_Zero_Elements())
    {
        case 0:
        case 1:
        case 2:
        case 3:
            return (0);
        case 4:
            return (1);
        default:
            return (4);  // No more than 4 bounds are allowed.
    }
}

// ith_Best_Positions_Four: gets the ith best three positions in the list.
void Set_Work_Centers::ith_Best_Positions_Four(int* position, int ith)
{
    switch (ith)
    {
        case 0:
            position[0] = 0;
            position[1] = 1;
            position[2] = 2;
            position[3] = 3;
            break;
        case 1:
            position[0] = 0;
            position[1] = 1;
            position[2] = 2;
            position[3] = 4;
            break;
        case 2:
            position[0] = 0;
            position[1] = 2;
            position[2] = 3;
            position[3] = 4;
            break;
        case 3:
            position[0] = 1;
            position[1] = 2;
            position[2] = 3;
            position[3] = 4;
            break;
    }
}

void Set_Work_Centers::Get_Highest_Locations_Four(int* location, int* p_sum_values, int position)
{
    int best_position[4];
    Work_Centers_Info* p_Work_Centers_Info[4];

#ifdef CHECK_DEBUG
    assert(position < Number_Combinations_Four());
#endif

    if (machine_effective[3] == TRUE)
    {
        // Is due to a previous effective lower bound.

#ifdef CHECK_DEBUG
        assert(position == 0);
#endif

        location[0] = machine[0];
        location[1] = machine[1];
        location[2] = machine[2];
        location[3] = machine[3];

        *p_sum_values = INT_MAX;
    }
    else
    {
#ifdef CHECK_DEBUG
        assert(list_work_centers.Number_Elements_List() == number_centers_cluster);
#endif

        ith_Best_Positions_Four(best_position, position);

        p_Work_Centers_Info[0] = list_work_centers.Get_p_ith_Element(best_position[0]);
        p_Work_Centers_Info[1] = list_work_centers.Get_p_ith_Element(best_position[1]);
        p_Work_Centers_Info[2] = list_work_centers.Get_p_ith_Element(best_position[2]);
        p_Work_Centers_Info[3] = list_work_centers.Get_p_ith_Element(best_position[3]);

        location[0] = p_Work_Centers_Info[0]->id;
        location[1] = p_Work_Centers_Info[1]->id;
        location[2] = p_Work_Centers_Info[2]->id;
        location[3] = p_Work_Centers_Info[3]->id;

        *p_sum_values = p_Work_Centers_Info[0]->result + p_Work_Centers_Info[1]->result +
                        p_Work_Centers_Info[2]->result + p_Work_Centers_Info[3]->result;
    }
}

// Number_Combinations_Five : returns the number of combinations of five machines
// given the number of elements in the list.
int Set_Work_Centers::Number_Combinations_Five()
{
    switch (list_work_centers.Number_Non_Zero_Elements())
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            return (0);
        default:
            return (4);  // No more than 4 bounds are allowed.
    }
}

// ith_Best_Positions_Five: gets the ith best three positions in the list.
void Set_Work_Centers::ith_Best_Positions_Five(int* position, int ith)
{
    switch (ith)
    {
        case 0:
            position[0] = 0;
            position[1] = 1;
            position[2] = 2;
            position[3] = 3;
            position[4] = 4;
            break;
        case 1:
            position[0] = 0;
            position[1] = 1;
            position[2] = 2;
            position[3] = 3;
            position[4] = 5;
            break;
        case 2:
            position[0] = 0;
            position[1] = 1;
            position[2] = 2;
            position[3] = 4;
            position[4] = 5;
            break;
        case 3:
            position[0] = 0;
            position[1] = 2;
            position[2] = 3;
            position[3] = 4;
            position[4] = 5;
            break;
    }
}

void Set_Work_Centers::Get_Highest_Locations_Five(int* location, int* p_sum_values, int position)
{
    int best_position[5];
    Work_Centers_Info* p_Work_Centers_Info[5];

#ifdef CHECK_DEBUG
    assert(position < Number_Combinations_Five());
#endif

    if (machine_effective[4] == TRUE)
    {
        // Is due to a previous effective lower bound.

#ifdef CHECK_DEBUG
        assert(position == 0);
#endif

        location[0] = machine[0];
        location[1] = machine[1];
        location[2] = machine[2];
        location[3] = machine[3];
        location[4] = machine[4];

        *p_sum_values = INT_MAX;
    }
    else
    {
#ifdef CHECK_DEBUG
        assert(list_work_centers.Number_Elements_List() == number_centers_cluster);
#endif

        ith_Best_Positions_Five(best_position, position);

        p_Work_Centers_Info[0] = list_work_centers.Get_p_ith_Element(best_position[0]);
        p_Work_Centers_Info[1] = list_work_centers.Get_p_ith_Element(best_position[1]);
        p_Work_Centers_Info[2] = list_work_centers.Get_p_ith_Element(best_position[2]);
        p_Work_Centers_Info[3] = list_work_centers.Get_p_ith_Element(best_position[3]);
        p_Work_Centers_Info[4] = list_work_centers.Get_p_ith_Element(best_position[4]);

        location[0] = p_Work_Centers_Info[0]->id;
        location[1] = p_Work_Centers_Info[1]->id;
        location[2] = p_Work_Centers_Info[2]->id;
        location[3] = p_Work_Centers_Info[3]->id;
        location[4] = p_Work_Centers_Info[4]->id;

        *p_sum_values = p_Work_Centers_Info[0]->result + p_Work_Centers_Info[1]->result +
                        p_Work_Centers_Info[2]->result + p_Work_Centers_Info[3]->result +
                        p_Work_Centers_Info[4]->result;
    }
}

void Set_Work_Centers::Set_N_Effective(int* location, int number_machines)
{
#ifdef CHECK_DEBUG
    assert(number_machines <= 5);
#endif
    int i;
    for (i = 0; i < 5; i++) machine_effective[i] = FALSE;

    machine_effective[number_machines - 1] = TRUE;

    for (i = 0; i < number_machines; i++) machine[i] = location[i];
}

void Set_Work_Centers::Get_N_Effective(int* location, int number_machines)
{
#ifdef CHECK_DEBUG
    assert(machine_effective[number_machines - 1] == TRUE);
#endif
    int i;
    for (i = 0; i < number_machines; i++) location[i] = machine[i];
}

int Set_Work_Centers::N_Machine_Effective(int number_machines)
{
#ifdef CHECK_DEBUG
    assert(number_machines > 0 && number_machines < 6);
#endif
    return (machine_effective[number_machines - 1]);
}

int Set_Work_Centers::Some_Machine_Effective()
{
    int i;
    int answer = 0;
    for (i = 0; i < 5; i++) answer = answer + machine_effective[i];

#ifdef CHECK_DEBUG
    if ((answer != 1) && (answer != 0))
    {
        assert(answer == 1 || answer == 0);
    }
#endif
    return (answer);
}

int Set_Work_Centers::Highest_Bound_Center(int location)
{
    if (list_work_centers.Number_Elements_List() == number_centers_cluster)
    {
        // If is a set with elements.
        return ((list_work_centers.Get_p_ith_Element(location))->id);
    }
    else
    {
        // There is no prevoius sequence.
        return (-1);
    }
}
