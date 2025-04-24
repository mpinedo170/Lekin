#ifndef GRAPH_H
#define GRAPH_H

#include "globals.h"
#include "lib_func.h"
#include "list_dis.h"
#include "node.h"
#include "sol_grph.h"
#include "t_stack.h"

#define MAX_ARCS_BACKUP 1000

class Job_Info
{
    friend class Graph;
    friend class Plant;
    friend class Set_Clusters;
    // For Set_Clusters::Write_Work_Center().

    friend class Parameters;

    // Data structures:

    int number_steps;
    int release_time;
    int due_date;

    int completion_time_penalty;
    int completion_time;

public:
    Job_Info()
    {
        completion_time_penalty = 0;
        release_time = NULL_TIME;
        due_date = NULL_TIME;
        number_steps = NULL_NUMBER;
        completion_time = NULL_TIME;
    }
    void operator=(const Job_Info& from)
    {
        completion_time_penalty = from.completion_time_penalty;
        release_time = from.release_time;
        due_date = from.due_date;
        number_steps = from.number_steps;
        completion_time = from.completion_time;
    }
};

class Arc_Backup
{
public:
    int center;
    int from;
    int to;
    int value;

    void Insert(int input_center, int input_from, int input_to, int input_value)
    {
        center = input_center;
        from = input_from;
        to = input_to;
        value = input_value;
    }
};

class Graph
{
    // Friends:

    friend class Plant;
    // For Plant.Show_Gantt_Chart()

    friend class Parameters;

    // Data structures:

    //      int dummy[1000] ;

    int total_num_jobs;
    int number_due_times;

    int objective;  // May be a combination.

    int makespan;
    int total_weighted_tardiness;
    int total_weighted_completion;
    // Output of the program.

    Node** graph_Node;
    // graph_Node[i][j] is a node in the graph

    Job_Info* graph_Job_Info;
    // keeps information about every job.

    int* completion_time_penalty;
    // Cashes the completion time penalty in the graph_Job_Info.
    // For the ATC.C, so it does not handle the graph.

    int* number_operations_job;
    // Cashes the number of operations per job in the graph_Job_Info.

    int updated_forward;
    int updated_backward;
    int objective_updated;
    // Flags.

    int counter_TRUE;
    int counter_FALSE;
    int counter_NOT_VISITED;
    // Used to save the in Initialize_Forward() and Initialize_Backward().
    // Instead of having to change:
    //      graph_Node[i][j].visit_status[XX] = NOT_VISITED;
    //      graph_Node[i][j].cleared_status[XX] = FALSE;
    //      graph_Node[i][j].poped = FALSE;
    // It is checked whether those values are bigger than "counter"

    int flag_reset;  // It tells Initialize_Backward() to reset values.

    int number_disjunctives;
    int number_blanks_graph;

    int number_centers_plant;
    int* number_operations_center;
    int* number_blanks_center;
    Class_Node_Index** node_index_location;
    // node_index_location[3][2]=(5,6) means that job number 2 in center 3
    // is the job 5 and step 6.

    int*** num_disj_center_oper;
    // num_disj_center_oper[2][4][BACKWARD] = 2 means that for work center 2,
    // operation 4 has to backward disjunctive arcs labeled SELECTED.

    int*** matrix;  // Matrix containing the status SELECTED, FORBIDDEN, REDUNDANT or BLANK.
                    // matrix[3][4][1] = FORBIDDEN means that the arc 4->1 in the center 3 is FORBIDDEN.
                    //#define SELECTED              111
                    //#define FORBIDDEN             222
                    //#define REDUNDANT     333
                    //#define BLANK_FIRST   444
                    //#define BLANK_SECOND  555
                    //#define DIAGONAL              666

    // ARCS related functions:
    void Reset();
    void Add_Source_Sink();

    void Add_Conjunctive(int, int, int, int, int);

    int There_Is_Conj_Node(Node**, Node*, int);

    void Create_Matrix(int, int*);
    // Creates the matrix containing the status SELECTED, FORBIDDEN, REDUNDANT, BLANK or DIAGONAL.
    //

    // TIME related functions:
    void Set_Min_Release();

    void Set_Time_Window(Node*, Node*, int, int);
    void Choose_Set_Time_Window(Node*, Node*, int, int);
    void Scan_Node(Node*, Node*, int, int);

    void Initialize_Forward();
    void Initialize_Backward();

    void Get_Start_Windows(int*, Stack<Node>*, int);
    void Write_Complition_Times(int*);

    int Get_Makespan();
    int Get_Weighted_Tardiness();
    int Get_Weighted_Completion();
    //

public:
    // ARCS related functions:
    Graph(){};
    void operator=(const Graph&);
    int operator==(const Graph&);

    void Initialize();
    void Terminate();

    void Initialize_Solution(Solution_Graph*);
    void Set_Blank_First(Solution_Graph*);
    void Write_Solution(Solution_Graph*);
    void Read_Machines(Solution_Graph*);
    void Read_Arcs(Solution_Graph*);

    void Add_New_Disjunctives(Class_List_Disjunctives*);
    // Used by the CUISE connection:

    void Show_Gantt_Chart();
    void Show_Disjunctives();

    int Is_Empty()
    {
        return (!number_disjunctives);
    }

    // Augusto 17.10.96
    void Set_Length_Conjuntive(int, int, int);

    //      void Back_Up_Center_Matrix( int *** , int * , int * , int *** , int * , int ) ;
    //      void Back_Up_Graph_Matrix( int **** , int ** , int * , int **** , int * ) ;
    //      void Check_Back_Up_Graph_Matrix( ) ;
    //      void Restore_Center_Matrix( int ** , int , int , int ** , int , int ) ;
    void Restore_Arcs(int, Arc_Backup*);
    //      void Restore_Precedence( int , Arc_Backup * ) ;
    void Delete_Center_Matrix(int);
    //      void Restore_Graph_Matrix( int *** , int * , int , int *** , int ) ;

    //

    int Add_Disjunctive_New(int*, Arc_Backup*, int, int, int, int, int);
    int Insert_Arc(int, int, int);
    int Insert_Arc(int*, Arc_Backup*, int, int, int);
    //      int Is_Brank( int , int , int , int ) ;
    //      void Delete_Disjunctive_New( int , int , int , int );
    //      void Uninsert_Arc( int , int , int ) ;
    //      void Uninsert_All_Arcs( int , int ) ;
    int Is_There_Machine_Predecessor(int*, int, int);
    int Predecessors_Are_Scheduled(int, int);
    int Number_Predecesors(Node*, int);

    // TIME related functions:
    void Set_Obsolete_Forward()
    {
        updated_forward = FALSE;
    }
    void Set_Obsolete_Backward()
    {
        updated_backward = FALSE;
    }
    int Is_Updated()
    {
        if ((updated_forward == TRUE) && (updated_backward == TRUE) && (objective_updated == TRUE))
            return (TRUE);
        else
            return (FALSE);
    }
    int Is_Updated_Forward()
    {
        if (updated_forward == TRUE)
            return (TRUE);
        else
            return (FALSE);
    }
    int Is_Updated_Backward()
    {
        if (updated_backward == TRUE)
            return (TRUE);
        else
            return (FALSE);
    }
    void Get_Forward_Windows(int*);
    void Get_Backward_Windows(int);

    int Get_Objective();
    void Set_Objective(int);

    // Returns TRUE if all the job predecessors have been scheduled.
    int Is_Complete(int job, int step)
    {
        for (int i = 1; i < step; i++)
            if (graph_Node[job][i].is_scheduled == FALSE) return (FALSE);
        return (TRUE);
    }

    void Update_Nodes();
    int Estimate_Delta(int, int, int);
    int Tardiness_Node(int, int);
    int Lateness_Node(int, int);
    int Slack_Node(int, int);

    int Mach_Pred_Is_Adjacent(int*, int*);
    int Mach_Succ_Is_Adjacent(int*, int*);
    int There_Is_Mach_Pred(int*, int*, int, int);
    int There_Is_Mach_Succ(int*, int*, int, int);
    int Job_Pred_Is_Adjacent(int*, int, int, int);

    // DATA related functions:
    int Get_Number_Jobs()
    {
        return (total_num_jobs);
    }
    int Get_Number_Steps_Job(int job)
    {
        return (graph_Job_Info[job].number_steps);
    }
    int Get_Due_Date_Job(int job)
    {
        return (graph_Job_Info[job].due_date);
    }

    int Get_Number_Due_Times()
    {
        return (number_due_times);
    }
    int* Get_Completion_Time_Penalty()
    {
        return (completion_time_penalty);
    }
    int Length(int job, int step)
    {
        return (graph_Node[job][step].conjunctive[FORWARD].Length());
    }
    int* Get_p_Release_Time(int job, int step)
    {
        return (&(graph_Node[job][step].release_time));
    }
    Class_Due_Time* Get_p_Due_Time(int job, int step)
    {
        return (graph_Node[job][step].due_time);
    }
    int Get_Work_Center(int job, int step)
    {
        return (graph_Node[job][step].work_center);
    }
    int* Get_p_Machine(int job, int step)
    {
        return (&(graph_Node[job][step].machine));
    }
    int* Get_p_Is_Scheduled(int job, int step)
    {
        return (&(graph_Node[job][step].is_scheduled));
    }
    void Set_Location_Center(int input_location_center, int job, int step)
    {
        graph_Node[job][step].location_center = input_location_center;
    }
    void Set_Node_Index_Location(int id, int partial_num_oper, int input_job, int input_step)
    {
        node_index_location[id][partial_num_oper].Set(input_job, input_step);
    }
    Class_Node_Index* Get_Node_Index_Location(int id, int location)
    {
        return (&(node_index_location[id][location]));
    }
    int Matrix_Value(int id, int j, int k)
    {
        return (matrix[id][j][k]);
    }
    int Get_Finishing_Time(int job, int step)
    {
        return (graph_Node[job][step].release_time + graph_Node[job][step].conjunctive[FORWARD].Length());
    }
    int Get_Release_Time(int job, int step)
    {
        return (graph_Node[job][step].release_time);
    }
    int Get_Number_Blanks()
    {
        return (number_blanks_graph);
    }
    int Get_N_Blank_Centers(int center)
    {
        return (number_blanks_center[center]);
    }
    int Few_Blanks_Centers(int);
    int Max_Branks_Center();
    double Print_Route_Index();
    double Print_Bottleneck_Index();
    int Is_Optimal_Predecessor();
    void FPrint_Matrix();

    int Valid_Answer_Cent(int);
    // TRUE if the selection of arcs is a tour.
    // A tour visits all the operations once with no cycles.

    int Number_Centers_Plant()
    {
        return (number_centers_plant);
    }
    int Number_Predecessors(int center, int operation)
    {
        return (num_disj_center_oper[center][operation][BACKWARD]);
    }
    int Delay(int job_from, int step_from, int job_to, int step_to)
    {
        return (graph_Node[job_from][step_from].delay[job_to][step_to]);
    }
    void Set_Delay(int job_from, int step_from, int job_to, int step_to, int value)
    {
        graph_Node[job_from][step_from].delay[job_to][step_to] = value;
    }
    int Is_Freezable(int job, int step)
    {
        if (graph_Node[job][step].is_movable == counter_TRUE)
            return (FALSE);
        else
            return (TRUE);
    }
    void Print_Freezable();

    void Increase_Counter();

    int Number_Disjunctives()
    {
        return (number_disjunctives);
    };
    int Number_Blanks()
    {
        return (number_blanks_graph);
    };

    int* Backup_Change_Due_Dates();
    void Restore_Due_Dates(int*);
};

#endif
