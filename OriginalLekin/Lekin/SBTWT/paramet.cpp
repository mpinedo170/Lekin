#include "StdAfx.h"

#include "paramet.h"

// General parameters:

int par_option;
int par_objective;

char* par_name_file_static_data;
char* par_name_file_heuristic;
char* par_name_file_lower_bound;
char* par_name_file_results;

// Other parameters:

char* par_name_output_file;

int par_slack_active;
int par_look_ahead;

int par_print;
int par_scheduling_machine;
int par_animation_reoptimization;
int par_scheduling_operation;

// int par_accelerator_complete ;
// int par_min_machine ;
// int par_active_schedule ;

// Heuristic parameters:

int par_max_branches_main;
double par_threshold_main;
int par_reoptimization;

int par_max_branches_oracle;
double par_threshold_oracle;

double par_K = -1;
double global_K = -1;

int par_number_skips = 0;
int par_upper_bound_factor;

// Lower bound parameters:

int par_number_machines_spot_check;
int par_upper_bound;
int par_transition_speed;
int par_selection_rule;
int* par_optimal_sequence;
double par_lower_bound;
int par_max_number_unscheduled_one;
int par_max_number_bounds_one;
int par_max_number_unscheduled_two;
int par_max_number_bounds_two;
int par_max_number_unscheduled_three;
int par_max_number_bounds_three;
int par_max_number_unscheduled_four;
int par_max_number_bounds_four;
int par_max_number_unscheduled_five;
int par_max_number_bounds_five;

// Local Search parameters:
int par_sigma;
int par_max_number_centers;
int par_size_cluster_list;

// Global variables:

FILE* n;
int first_time;
int hitting_time;

#ifdef CUISE_CONNECTED

// Graph optimal_Graph ;                 //It will store the optimal solution so far.

// Plant main_Plant ;                    //Stores the data of work centers.

// struct Manual_Solution_Struct this_Manual_Solution_Struct ;

#endif

// Parameters for SLOW_INPUT:

char* par_name_file_graph;
char* par_name_file_plant;
char* par_name_file_clusters;

char* Parameters::Get_Char_Parameter(char* header, FILE* p_file)
{
    char str[LENGTH_INPUT_CHAR];
    char* str_out;

    fscanf(p_file, "%s", str);
    if (_tcscmp(header, str) != 0)
    {
        printf("ERROR the header is incorrect\n");
        exit(1);
    }
    fscanf(p_file, "%s", str);
    if (_tcslen(str) == 0)
    {
        printf("ERROR there in no paramenter\n");
        exit(1);
    }
    //      str_out = (char *)malloc(_tcslen(str) + 1);
    str_out = new char[_tcslen(str) + 1];
    _tcscpy(str_out, str);
    return (str_out);
}

int Parameters::Get_Int_Parameter(char* header, FILE* p_file)
{
    char str[LENGTH_INPUT_CHAR];
    int input_int;

    fscanf(p_file, "%s", str);
    if (_tcscmp(header, str) != 0)
    {
        printf("ERROR the header is incorrect\n");
        exit(1);
    }
    fscanf(p_file, "%d", &input_int);
    return (input_int);
}

double Parameters::Get_Float_Parameter(char* header, FILE* p_file)
{
    char str[LENGTH_INPUT_CHAR];
    double input_float;

    fscanf(p_file, "%s", str);
    if (_tcscmp(header, str) != 0)
    {
        printf("ERROR the header is incorrect\n");
        exit(1);
    }
    fscanf(p_file, "%lf", &input_float);
    return (input_float);
}

void Parameters::Read_General(int* p_size, int* p_version, int* p_instance)
{
    FILE* p_file;

#ifdef V_UNIX
    static char* par_heu[5] = {"INPUT/p_4x4/par_heu", "INPUT/p_6x6/par_heu", "INPUT/p_7x7/par_heu",
        "INPUT/p_8x8/par_heu", "INPUT/p_10x10/par_heu"};
    static char* par_low[5] = {"INPUT/p_4x4/par_low", "INPUT/p_6x6/par_low", "INPUT/p_7x7/par_low",
        "INPUT/p_8x8/par_low", "INPUT/p_10x10/par_low"};
    static char* results[5] = {"INPUT/p_4x4/results", "INPUT/p_6x6/results", "INPUT/p_7x7/results",
        "INPUT/p_8x8/results", "INPUT/p_10x10/results"};
    static char* res_flow[5] = {"INPUT/p_4x4/res_flow", "INPUT/p_6x6/res_flow", "INPUT/p_7x7/res_flow",
        "INPUT/p_8x8/res_flow", "INPUT/p_10x10/res_flow"};

#endif

#ifndef V_UNIX
    static char* par_heu[10] = {"par_heu", "par_heu", "par_heu", "par_heu", "par_heu", "par_heu", "par_heu",
        "par_heu", "par_heu", "par_heu"};
    static char* par_low[5] = {
        "INPUT\\p_4x4\\par_low", "p_6x6\\par_low", "p_7x7\\par_low", "p_8x8\\par_low", "p_10x10\\par_low"};
    static char* results[5] = {
        "INPUT\\p_4x4\\results", "p_6x6\\results", "p_7x7\\results", "p_8x8\\results", "p_10x10\\results"};
    static char* res_flow[5] = {"INPUT\\p_4x4\\res_flow", "p_6x6\\res_flow", "p_7x7\\res_flow",
        "p_8x8\\res_flow", "p_10x10\\res_flow"};

#endif

    // General parameters:
    // p_file = fopen("par_gen", "r");
    if ((p_file = fopen("par_gen", "r")) == NULL)
    {
        printf("Archivo no encontrado\n");
        exit(1);
    }

    //"par_gen" is the name of the file with the general paramenters.

    par_option = Get_Int_Parameter("Option(0=Heuristic,1=Branch&Bound,2=Local_Search):", p_file);

    par_objective = Get_Int_Parameter("Objective(0=Makespan,1=Tardiness,2=Completion):", p_file);

    *p_size = Get_Int_Parameter(
        "Size(1->1x1;2->2x2;3->3x3;4->4x4;5->5x5;6->6x6;7->7x7;8->8x8;9->9x9;10->10x10):", p_file);
    assert(*p_size == 1 || *p_size == 2 || *p_size == 3 || *p_size == 4 || *p_size == 5 || *p_size == 6 ||
           *p_size == 7 || *p_size == 8 || *p_size == 9 || *p_size == 10);
    switch (*p_size)
    {
        case 1:
            *p_size = 0;
            break;
        case 2:
            *p_size = 1;
            break;
        case 3:
            *p_size = 2;
            break;
        case 4:
            *p_size = 3;
            break;
        case 5:
            *p_size = 4;
            break;
        case 6:
            *p_size = 5;
            break;
        case 7:
            *p_size = 6;
            break;
        case 8:
            *p_size = 7;
            break;
        case 9:
            *p_size = 8;
            break;
        case 10:
            *p_size = 9;
            break;
    }
    par_name_file_heuristic = par_heu[*p_size];
    // par_name_file_lower_bound = par_low[*p_size] ;

    *p_version = Get_Int_Parameter("Version(-1->all):", p_file);
    assert(*p_version < NUMBER_VERSIONS);

    *p_instance = Get_Int_Parameter("Instance(-1->all):", p_file);
    assert(*p_instance < NUMBER_INSTANCES);

    fclose(p_file);  // Close "par_gen".
}

void Parameters::Read_Result(int version, int instance, int size)
{
    int i, v;
    FILE* p_file;
    char name_instance[10];
    // Other parameters:

    if ((p_file = fopen(par_name_file_results, "r")) == NULL)
    {
        printf("Archivo results no encontrado\n");
        exit(1);
    }

    for (i = 0; i < instance; i++)
    {
        fscanf(p_file, "%s", &name_instance);
        for (v = 0; v < NUMBER_VERSIONS; v++) fscanf(p_file, "%d", &par_upper_bound);
    }
    fscanf(p_file, "%s", &name_instance);
    for (v = 0; v < version + 1; v++) fscanf(p_file, "%d", &par_upper_bound);
    fclose(p_file);
    printf("%s[%d] ", &name_instance, par_upper_bound);
}

void Parameters::Read_Other()
{
    FILE* p_file;

    // Other parameters:
    if ((p_file = fopen("par_rest", "r")) == NULL)
    {
        printf("Archivo par_rest no encontrado\n");
        exit(1);
    }

    par_name_output_file = Get_Char_Parameter("Output_File:", p_file);

    par_K = Get_Float_Parameter("K(0.00001->MS;5.0->ATC;99999.0->WSPT;-1->automatic):", p_file);
    if ((par_K <= 0.0) && (par_K != -1))
    {
        printf("ERROR: wrong value par_K \n");
        exit(1);
    }

    global_K = -1;

    par_slack_active = Get_Int_Parameter("Slack_Active(0->active):", p_file);
    if ((par_slack_active < 0) || (par_slack_active > INT_MAX / 2))
    {
        printf("ERROR: (par_slack_active < 0) || (par_slack_active > INT_MAX/2) \n");
        exit(1);
    }

    par_look_ahead = Get_Int_Parameter("Look_Ahead(0->non_delay):", p_file);
    if ((par_look_ahead < 0) || (par_look_ahead > INT_MAX / 2))
    {
        printf("ERROR: (par_look_ahead < 0) || (par_look_ahead > INT_MAX/2) \n");
        exit(1);
    }

    par_print = Get_Int_Parameter("Print(1=YES,0=NO):", p_file);
    par_scheduling_machine = Get_Int_Parameter("Animation_Schedule_Machine(1=YES,0=NO):", p_file);
    par_animation_reoptimization = Get_Int_Parameter("Animation_Reoptimization(1=YES,0=NO):", p_file);
    par_scheduling_operation = Get_Int_Parameter("Animation_Schedule_Operation(1=YES,0=NO):", p_file);
    fclose(p_file);  //"par_rest"
}

void Parameters::Read_Static_Data(Graph* p_Graph, Plant* p_Plant, int version)

{
    FILE* this_file;
    int i, j;  // counters.
    int input_number_work_centers, input_number_machines, input_total_num_jobs, input_completion_time_penalty,
        input_release_time, input_number_steps, input_work_center, input_length_conjunctive;
    int input_speed_machines;
    // input variables.

    int total_length;

    double input_due_date_factor;

    // The following double loop reads the values of the conjunctive arcs.
    if ((this_file = fopen(par_name_file_static_data, "r")) == NULL)
    {
        printf("Archivo par_name_file_static_data no encontrado\n");
        exit(1);
    }

    // From here the jobs.
    fscanf(this_file, "%d", &input_total_num_jobs);

#ifdef CHECK_DEBUG
    assert(input_total_num_jobs <= MAX_JOBS);
#endif

    p_Graph->total_num_jobs = input_total_num_jobs;

    switch (par_objective)
    {
        case MAKESPAN:
            p_Graph->number_due_times = 1;
            break;

        case TARDINESS:
        case COMPLETION:
            p_Graph->number_due_times = input_total_num_jobs;
            break;
    }
    // Allocating memory:
    p_Graph->graph_Node = new type_p_Node[p_Graph->total_num_jobs];
    // There is a typedef Node* p_Node in Node.h since
    // ANSI C++ forbids array dimensions with parenthesized type.

    p_Graph->graph_Job_Info = new Job_Info[p_Graph->total_num_jobs];
    p_Graph->completion_time_penalty = new int[p_Graph->total_num_jobs];
    p_Graph->number_operations_job = new int[p_Graph->total_num_jobs];
    for (i = 0; i < p_Graph->total_num_jobs; i++)
    {
        fscanf(this_file, "%d", &input_completion_time_penalty);
        p_Graph->graph_Job_Info[i].completion_time_penalty = input_completion_time_penalty;
        p_Graph->completion_time_penalty[i] = input_completion_time_penalty;
        // Cashing the completion time panlty.

        if (par_objective == MAKESPAN)
        {
            p_Graph->graph_Job_Info[i].completion_time_penalty = 1;
            p_Graph->completion_time_penalty[i] = 1;
            // Cashing the completion time panlty.
        }

        fscanf(this_file, "%d", &input_release_time);
        p_Graph->graph_Job_Info[i].release_time = input_release_time;

        fscanf(this_file, "%lf", &input_due_date_factor);

        switch (version)
        {
            case 0:  // What is in the file.
                break;
            case 1:
                input_due_date_factor = 1.1;
                break;
            case 2:
                input_due_date_factor = 1.2;
                break;
            case 3:
                input_due_date_factor = 1.3;
                break;
            case 4:
                input_due_date_factor = 1.4;
                break;
            case 5:
                input_due_date_factor = 1.5;
                break;
            case 6:
                input_due_date_factor = 1.6;
                break;
            case 7:
                input_due_date_factor = 1.7;
                break;
        }
        fscanf(this_file, "%d", &input_number_steps);
        p_Graph->graph_Job_Info[i].number_steps = input_number_steps;
        p_Graph->number_operations_job[i] = input_number_steps;

        // Allocating memory:
        p_Graph->graph_Node[i] = new Node[p_Graph->graph_Job_Info[i].number_steps + 3];
        ;
        total_length = input_release_time;
        for (j = 0; j < p_Graph->graph_Job_Info[i].number_steps; j++)
        {
            fscanf(this_file, "%d", &input_work_center);
            fscanf(this_file, "%d", &input_length_conjunctive);
            assert(input_length_conjunctive >= 0);
            p_Graph->Add_Conjunctive(i, j + 1, input_length_conjunctive, i, j + 2);

            p_Graph->graph_Node[i][j + 1].work_center = input_work_center;
            total_length = total_length + input_length_conjunctive;
        }
        p_Graph->graph_Job_Info[i].due_date = (int)(total_length * input_due_date_factor);

        // Augusto 9.10.96 Lectura y asignacion del due date si corresponde
        if (input_due_date_factor == -1.0)
        {
            int input_due_date;
            fscanf(this_file, "%d", &input_due_date);
            p_Graph->graph_Job_Info[i].due_date = (int)(input_due_date);
        }
    }

    // This part writes the position of the nodes and creates the due dates.
    for (i = 0; i < p_Graph->total_num_jobs; i++)
    {
        for (j = 0; j < (p_Graph->graph_Job_Info[i].number_steps + 3); j++)
        {
            p_Graph->graph_Node[i][j].Initialize(
                i, j, p_Graph->number_due_times, p_Graph->total_num_jobs, p_Graph->number_operations_job);
            //"number_due_dates" sets the size of the composed due date.
        }
        int k, l;
        for (k = 0; k < p_Graph->total_num_jobs; k++)
        {
            for (l = 1; l <= p_Graph->number_operations_job[k]; l++)
            {
                p_Graph->graph_Node[i][p_Graph->number_operations_job[i] + 1].delay[k][l] = -1;
            }
        }
        p_Graph->graph_Node[i][p_Graph->number_operations_job[i] + 1]
            .delay[i][p_Graph->number_operations_job[i]] = 0;
    }

    // From here the work centers
    fscanf(this_file, "%d", &input_number_work_centers);

#ifdef CHECK_DEBUG
    assert(input_number_work_centers <= MAX_PLANT_WORK_CENTER);
#endif

    p_Plant->number_work_centers = input_number_work_centers;

    // Allocationg memory:
    p_Plant->cluster_work_center = new Cluster_Work_Center[p_Plant->number_work_centers];

    int* n_operations_work_center = new int[p_Plant->number_work_centers];
    // Counts the number of operations in each work center,

    for (i = 0; i < p_Plant->number_work_centers; i++)
    {
        n_operations_work_center[i] = 0;
    }

    for (i = 0; i < p_Graph->total_num_jobs; i++)
    {
        for (j = 0; j < p_Graph->graph_Job_Info[i].number_steps; j++)
        {
            n_operations_work_center[p_Graph->graph_Node[i][j + 1].work_center]++;
        }
    }

    p_Graph->Create_Matrix(p_Plant->number_work_centers, n_operations_work_center);
    // Creates the matrix containing the status SELECTED, FORBIDDEN, REDUNDANT or BLANK.

    for (i = 0; i < p_Plant->number_work_centers; i++)
    {
        fscanf(this_file, "%d", &input_number_machines);
#ifdef CHECK_DEBUG
        assert(input_number_machines <= MAX_MACHINES_WORK_CENTER);
#endif

        p_Plant->cluster_work_center[i].Initialize(
            i, n_operations_work_center[i], input_number_machines, NULL, NULL, p_Graph);

        // Augusto 9.10.96 ingreso de veloc. de maq.
        if (input_number_machines > 1)
        {
            for (j = 0; j < input_number_machines; j++)
            {
                fscanf(this_file, "%d", &input_speed_machines);
                //                                    p_Plant->cluster_work_center[i].speed[j] =
                //                                    input_speed_machines;
                p_Plant->cluster_work_center[i].Ini_veloc(j, input_speed_machines);
            }
        }
        else
            //                               p_Plant->cluster_work_center[i].speed[0] = 1.0;
            p_Plant->cluster_work_center[i].Ini_veloc(0, 1);
    }
    delete[] n_operations_work_center;
    fclose(this_file);

    // Adds operations to each cluster:
    for (i = 0; i < p_Graph->total_num_jobs; i++)
    {
        for (j = 0; j < p_Graph->graph_Job_Info[i].number_steps; j++)
        {
            // Checks every operation in the graph.

            p_Plant->cluster_work_center[(p_Graph->graph_Node[i][j + 1].work_center)].Add_Operation(
                i, j + 1, p_Graph);
            // It writes the position of the operation in the center in each corresponding node.
#ifdef CHECK_DEBUG
            assert(i == p_Graph->graph_Node[i][j + 1].position.job);
            assert(j + 1 == p_Graph->graph_Node[i][j + 1].position.step);
#endif
        }
    }
}

void Parameters::Read_Heuristic_Data(Graph* p_Graph, Plant* p_Plant, Set_Clusters* p_Set_Clusters)

{
    FILE* this_file;
    int i, j;  // counters.
    int input_number_clusters, input_number_work_centers, input_work_center_id;

    // int agregated_number_operations ;

    if ((this_file = fopen(par_name_file_heuristic, "r")) == NULL)
    {
        printf("Archivo par_name_file_heuristic no encontrado\n");
        exit(1);
    }

    input_number_clusters = Get_Int_Parameter("Clusters_Description:", this_file);

#ifdef CHECK_DEBUG
    assert(input_number_clusters <= MAX_CLUSTERS);
    assert(p_Set_Clusters->cluster == NULL);
#endif

    p_Set_Clusters->number_clusters = input_number_clusters;

    p_Set_Clusters->cluster = new Cluster[p_Set_Clusters->number_clusters];
    // Allocationg memory:

    for (i = 0; i < p_Set_Clusters->number_clusters; i++)
    {
        fscanf(this_file, "%d", &input_number_work_centers);

#ifdef CHECK_DEBUG
        assert(input_number_work_centers <= MAX_WORK_CENTERS_CLUSTER);
#endif

        p_Set_Clusters->cluster[i].Initialize(i, input_number_work_centers, p_Graph);
        // i is the id of the cluster.
        // Here the memory is allocated.

        //              agregated_number_operations = 0 ;

        for (j = 0; j < input_number_work_centers; j++)
        {
            fscanf(this_file, "%d", &input_work_center_id);

            //                      agregated_number_operations = agregated_number_operations +
            //                                              p_Plant->cluster_work_center[input_work_center_id].Get_Number_Operations()
            //                                              ;

            p_Set_Clusters->cluster[i].Assign_p_Work_Center(
                &(p_Plant->cluster_work_center[input_work_center_id]));
        }
    }

    par_max_branches_main = Get_Int_Parameter("Number_Branches_Main:", this_file);
    if ((par_max_branches_main < 0) || (par_max_branches_main > INT_MAX / 2))
    {
        printf("ERROR: (par_max_branches_main < 0) || (par_max_branches_main > INT_MAX/2) \n");
        exit(1);
    }

    par_threshold_main = Get_Float_Parameter("Threshold_Main:", this_file);

    par_reoptimization = Get_Int_Parameter("Number_Reoptimized:", this_file);
    if ((par_reoptimization < 0) || (par_reoptimization > INT_MAX / 2))
    {
        printf("ERROR: (par_reoptimization < 0) || (par_reoptimization > INT_MAX/2) \n");
        exit(1);
    }

    par_max_branches_oracle = Get_Int_Parameter("Number_Branches_Oracle:", this_file);
    if ((par_max_branches_oracle < 0) || (par_max_branches_oracle > INT_MAX / 2))
    {
        printf("ERROR: (par_max_branches_oracle < 0) || (par_max_branches_oracle > INT_MAX/2) \n");
        exit(1);
    }

    par_threshold_oracle = Get_Float_Parameter("Threshold_Oracle:", this_file);

    fclose(this_file);

    //      par_transition_speed = 1000 ;
    //      par_selection_rule = EARLY ;
}

void Parameters::Read_Lower_Bound_Data(Graph* p_Graph, Plant* p_Plant, Set_Clusters* p_Set_Clusters)

{
    printf(" Entro a lower bound \n");

    FILE* this_file;
    int i, j;  // counters.
    int input_number_clusters, input_number_work_centers, input_work_center_id;

    // int agregated_number_operations ;

    if ((this_file = fopen(par_name_file_lower_bound, "r")) == NULL)
    {
        printf(" Archivo par_name_file_lower_bound no encontrado\n");
        exit(1);
    }

    int temp_par_upper_bound = Get_Int_Parameter("Upper_Bound(-1->search;Otherwise->given):", this_file);
    if (temp_par_upper_bound == -1) par_upper_bound = -1;

    if (((par_upper_bound < 0) || (par_upper_bound > INT_MAX / 2)) && (par_upper_bound != -1))
    {
        printf("ERROR: (par_upper_bound < 0)   || (par_upper_bound > INT_MAX/2) \n");
        exit(1);
    }

    par_transition_speed = Get_Int_Parameter("Transition_Speed(0->Disjunctive;1000->Active):", this_file);

    par_selection_rule = Get_Int_Parameter("Selection(0->Early;1->Estimate;2->Bottle;3->Bound):", this_file);
    assert(par_selection_rule >= 2 && par_selection_rule <= 4);

    if (par_selection_rule == 4) par_transition_speed = 0;

    par_lower_bound = Get_Float_Parameter("Threshold_Lower_Bound:", this_file);
    if ((par_lower_bound < 0) || (par_lower_bound > 1))
    {
        printf("ERROR: (par_lower_bound < 0) || (par_lower_bound > 1) \n");
        exit(1);
    }

    par_max_number_unscheduled_one = Get_Int_Parameter("Max_Number_Unsched_One:", this_file);
    par_max_number_bounds_one = Get_Int_Parameter("Max_Number_Bounds_One:", this_file);
    par_max_number_unscheduled_two = Get_Int_Parameter("Max_Number_Unsched_Two:", this_file);
    par_max_number_bounds_two = Get_Int_Parameter("Max_Number_Bounds_Two:", this_file);
    //      par_max_number_unscheduled_three = Get_Int_Parameter("Max_Number_Unsched_Three:", this_file ) ;
    //      par_max_number_bounds_three = Get_Int_Parameter("Max_Number_Bounds_Three:", this_file ) ;
    //      par_max_number_unscheduled_four = Get_Int_Parameter("Max_Number_Unsched_Four:", this_file ) ;
    //      par_max_number_bounds_four = Get_Int_Parameter("Max_Number_Bounds_Four:", this_file ) ;
    //      par_max_number_unscheduled_five = Get_Int_Parameter("Max_Number_Unsched_Five:", this_file ) ;
    //      par_max_number_bounds_five = Get_Int_Parameter("Max_Number_Bounds_Five:", this_file ) ;

    input_number_clusters =
        Get_Int_Parameter("Clusters_Description(1->set_given;>1->size_choose_set):", this_file);

    if (input_number_clusters == 1)
    {
        par_number_machines_spot_check = NULL_NUMBER;

#ifdef CHECK_DEBUG
        assert(input_number_clusters <= MAX_CLUSTERS);
        assert(p_Set_Clusters->cluster == NULL);
#endif
        p_Set_Clusters->number_clusters = input_number_clusters;

        p_Set_Clusters->cluster = new Cluster[p_Set_Clusters->number_clusters];
        // Allocationg memory:

        for (i = 0; i < p_Set_Clusters->number_clusters; i++)
        {
            fscanf(this_file, "%d", &input_number_work_centers);

#ifdef CHECK_DEBUG
            assert(input_number_work_centers <= MAX_WORK_CENTERS_CLUSTER);
#endif

            p_Set_Clusters->cluster[i].Initialize(i, input_number_work_centers, p_Graph);
            // i is the id of the cluster.
            // Here the memory is allocated.

            //                      agregated_number_operations = 0 ;

            for (j = 0; j < input_number_work_centers; j++)
            {
                fscanf(this_file, "%d", &input_work_center_id);

                //                              agregated_number_operations = agregated_number_operations +
                //                                                      p_Plant->cluster_work_center[input_work_center_id].Get_Number_Operations()
                //                                                      ;

                //                              p_Set_Clusters->cluster[i].p_work_center[j] =
                //                                                                      &(
                //                                                                      p_Plant->cluster_work_center[input_work_center_id]
                //                                                                      ) ;

                p_Set_Clusters->cluster[i].Assign_p_Work_Center(
                    &(p_Plant->cluster_work_center[input_work_center_id]));

                //                              p_Set_Clusters->cluster[i].work_center[j].Initialize(
                //                              input_work_center_id ,
                //                                                              p_Plant->cluster_work_center[input_work_center_id].number_operations
                //                                                              ,
                //                                                              p_Plant->Get_Number_Machines_Work_Center(
                //                                                              input_work_center_id ) );
                // Here the memory is allocated.

                /*
                                                for( k = 0 ; k <
                   p_Plant->plant_Work_Centers[input_work_center_id].number_operations ; k++){
                                                        p_Set_Clusters->cluster[i].
                                                                work_center[j].
                                                                        Get_Operation(k)->
                                                                                Set_Job_Step(
                   p_Plant->plant_Work_Centers[input_work_center_id]. steps_Work_Center[k].job ,
                                                                                                        p_Plant->plant_Work_Centers[input_work_center_id].
                                                                                                                steps_Work_Center[k].step ) ;
                                                }
                */
            }

            //                      p_Set_Clusters->cluster[i].number_operations = agregated_number_operations
            //                      ;
        }

        par_max_branches_main = 1;

        par_threshold_main = 0;

        par_reoptimization = 0;

        par_max_branches_oracle = INT_MAX;

        par_threshold_oracle = 0;

        par_slack_active = 0;
        par_look_ahead = INT_MAX / 2;

        par_print = 0;
        par_scheduling_machine = 0;
        par_animation_reoptimization = 0;
        //              par_scheduling_operation  = 0 ;
    }
    else
    {
#ifdef CHECK_DEBUG
        assert(input_number_clusters <= MAX_CLUSTERS);
#endif
        par_number_machines_spot_check = input_number_clusters;
    }
    fclose(this_file);
}
