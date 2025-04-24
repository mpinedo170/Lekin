#ifndef SOL_INCR_H
#define SOL_INCR_H

// Contains the increment of the solution of the oracle.
class Solution_Increment
{
    friend class Act_Oracle;  // For Beam_Oracle::Recursive_Call().

    // Data structures:
    int location_work_center;
    int machine;
    int location_operation;
    int finishing_time;

public:
    Solution_Increment(int input_location_work_center,
        int input_machine,
        int input_location_operation,
        int input_finishing_time)
    {
        location_work_center = input_location_work_center;
        machine = input_machine;
        location_operation = input_location_operation;
        finishing_time = input_finishing_time;
    }
    Solution_Increment()
    {
        location_work_center = NULL_WORK_CENTER;
        machine = NULL_MACHINE;
        location_operation = NULL_LOCATION;
        finishing_time = NULL_TIME;
    }

    void Terminate(){};

    void Set(int input_location_work_center,
        int input_machine,
        int input_location_operation,
        int input_finishing_time)
    {
        location_work_center = input_location_work_center;
        machine = input_machine;
        location_operation = input_location_operation;
        finishing_time = input_finishing_time;
    };

    void operator=(const Solution_Increment& from)
    {
        location_work_center = from.location_work_center;
        machine = from.machine;
        location_operation = from.location_operation;
        finishing_time = from.finishing_time;
    }
    int operator==(const Solution_Increment& from)
    {
        if ((location_work_center != from.location_work_center) || (machine != from.machine) ||
            (location_operation != from.location_operation) || (finishing_time != from.finishing_time))
            return (FALSE);
        else
            return (TRUE);
    }
    int Get_Location_Operation()
    {
        return (location_operation);
    }
};

#endif
