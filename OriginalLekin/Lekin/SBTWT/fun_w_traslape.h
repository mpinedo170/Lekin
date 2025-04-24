#ifndef FUN_W_TRASLAPE_H
#define FUN_W_TRASLAPE_H

/* Declaracion de la funciones externas */

extern void dec_input(FILE*, Graph**, Plant**, int version);
extern void save_Information(char*, Input_File_Jobs*, Input_File_Workcenters*, int*, int*);
extern void Create_Windows(Input_File_Jobs problem1,
    Input_File_Workcenters problem2,
    int mayor,
    Graph* p_optimal_Graph,
    int window,
    int** next_op,
    int* limit,
    int** sum_operations,
    int* times_machines,
    int* W_machines,
    int num_machines,
    double Tras_win,
    int** finishing_times,
    int** release_times,
    int* due_date);
extern void Read_Windows();
extern void salvar_resultados(
    char* problem, int window, double traslape, double tolerancia, int twt, int time);

extern void windows_Jobs(FILE* output,
    int w,
    int r,
    double f,
    int num_op,
    int** maq,
    int** p,
    int d,
    int ventana,
    int next_op,
    int i,
    int sum_operations,
    int N_opers);
extern void windows_Workcenter(FILE*, int, int**, int);
extern int Balance_times_machines(int*, int**, int*, int, int);
extern void windows_faltantes(FILE*, int*, int, int*);
extern int get_max_length(int** p, int jobs, int* N_opers, int* r, int*);
extern void Read_times_machines(int*, int*);
extern int get_max_machine(int* times_machines, int* W_machines, int num_machines);
extern double read_tolerancia();
extern double read_traslape();
extern int valida_traslape(int window, int max_machine, double Tras_win);
extern void iniciate_times_machines(int N_workcenter, int** times_machines);
extern void calculate_release(Graph* p_optimal_Graph,
    int** next_op,
    int** r,
    double tras,
    int** times_machines,
    int** workcenters,
    int jobs,
    int**,
    int**);
extern void calculate_release_machine(int jobs,
    int** next_op,
    int* N_opers,
    int** times_machines,
    int** list_speeds,
    int* W_machines,
    int** workcenters,
    int** p,
    int** r);
extern void iniciate_reserv_op(int** reserv_op, int** next_op, int jobs);
extern void windows_with_traslape(Input_File_Jobs* p_problem1,
    int* limit,
    int** sum_operations,
    FILE* output,
    int window,
    int** next_op,
    int* times_machines,
    int N_workcenter,
    int* jobs_add,
    int* reserv_op,
    int* due_date);
extern void Workcenters_of_the_Window(
    FILE* output, Input_File_Workcenters* p_problem2, int window, int jobs, int jobs_add);
extern int* Read_release_jobs(int* n);

#endif
