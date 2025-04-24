#include "StdAfx.h"

#include "input_file.h"
#include "optimize.h"
#include "paramet.h"
#include "sol_grph.h"

#include "fun_w_traslape.h"

// tools.cpp, Version ventanas balanceadas segun el tiempo de procesamiento con traslape
// Luis Peña M.  22 de Octubre de 1997

void Read_Windows()
{
    printf("\n********************************************************************\n\n");
    printf("   Number of Windows : ");
    scanf("%d", &Windows);
    printf("\n\n********************************************************************\n");
}

void save_Information(char* file,
    Input_File_Jobs* p_problem1,
    Input_File_Workcenters* p_problem2,
    int* mayor,
    int* num_machines)
{
    FILE* input;
    int i, j, jobs, w, r, N_opers, workcenters, p, d;
    int N_workcenter, N_maq, list_speeds;
    double f;
    if ((input = fopen(file, "r")) == NULL)
    {
        printf(" Archivo no encontrado\n");
        exit(1);
    }

    fscanf(input, "%d", &jobs);
    (*p_problem1).put_jobs(jobs);
    (*p_problem1).tam_w(jobs);
    (*p_problem1).tam_r(jobs);
    (*p_problem1).tam_f(jobs);
    (*p_problem1).tam_N_opers(jobs);
    (*p_problem1).tam_workcenters1(jobs);
    (*p_problem1).tam_p1(jobs);
    (*p_problem1).tam_d(jobs);

    for (i = 0; i < jobs; i++)
    {
        fscanf(input, "%d", &w);
        (*p_problem1).put_w(w, i);
        fscanf(input, "%d", &r);
        (*p_problem1).put_r(r, i);
        fscanf(input, "%lf", &f);
        (*p_problem1).put_f(f, i);
        fscanf(input, "%d", &N_opers);
        (*p_problem1).put_N_opers(N_opers, i);
        (*p_problem1).tam_workcenters2(N_opers, i);
        (*p_problem1).tam_p2(N_opers, i);
        for (j = 0; j < N_opers; j++)
        {
            fscanf(input, "%d", &workcenters);
            (*p_problem1).put_workcenters(workcenters, i, j);
            fscanf(input, "%d", &p);
            (*p_problem1).put_p(p, i, j);
            if ((*mayor) < workcenters) (*mayor) = workcenters;
        }

        if (f == -1.0)
        {
            fscanf(input, "%d", &d);
            (*p_problem1).put_d(d, i);
        }
    }

    fscanf(input, "%d", &N_workcenter);
    (*p_problem2).put_N_workcenter(N_workcenter);
    (*p_problem2).tam_list_speeds1(N_workcenter);
    (*p_problem2).tam_N_maq(N_workcenter);

    for (i = 0; i < N_workcenter; i++)
    {
        fscanf(input, "%d", &N_maq);
        (*p_problem2).put_N_maq(N_maq, i);
        (*p_problem2).tam_list_speeds2(N_maq, i);

        if (N_maq == 1)
        {
            (*p_problem2).put_list_speeds(N_maq, i, 0);
            (*num_machines)++;
        }
        else
        {
            for (j = 0; j < N_maq; j++)
            {
                fscanf(input, "%d", &list_speeds);
                (*p_problem2).put_list_speeds(list_speeds, i, j);
                (*num_machines)++;
            }
        }
    }
    fclose(input);
}

void Create_Windows(Input_File_Jobs problem1,
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
    int* due_date)
{
    FILE* output;
    int max_length = 0;
    int jobs_add = 0, *reserv_op;

#ifdef V_UNIX
    char *file_output, *name = {"INPUT/resultados/vent"};
#else
    char *file_output, *name = {"vent"};
#endif

    char* s[10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

    int traslape = 0, max_machine = 0, tras = 0, i;
    double toler;
    file_output = new TCHAR[_tcslen(name) + _tcslen(s[window - 1]) + 1];
    sprintf(file_output, "%s%s", name, s[window - 1]);

    if ((output = fopen(file_output, "w")) == NULL)
    {
        printf("El archivo no puede ser creado\n");
        exit(1);
    }

    fprintf(output, "%d\n", problem1.get_jobs());

    toler = read_tolerancia();

    //  se almacena el release inicial de la primera ventana para la segunda

    if (window == 2)
    {
        for (i = 0; i < problem1.get_jobs(); i++) release_times[i][0] = problem1.get_r1(i);
    }

    // Fin almacenado de release inicial de la primera ventana

    if (p_optimal_Graph != NULL && times_machines != NULL)
    {
        max_machine = get_max_machine(times_machines, W_machines, num_machines);

        iniciate_times_machines(problem2.get_N_workcenter(), &times_machines);
        calculate_release(p_optimal_Graph, next_op, problem1.get_r2(), Tras_win, &times_machines,
            problem1.get_workcenters(), problem1.get_jobs(), finishing_times, release_times);
        calculate_release_machine(problem1.get_jobs(), next_op, problem1.get_N_opers(), &times_machines,
            problem2.get_list_speeds(), W_machines, problem1.get_workcenters(), problem1.get_p(),
            problem1.get_r2());
    }

    reserv_op = new int[problem1.get_jobs()];
    iniciate_reserv_op(&reserv_op, next_op, problem1.get_jobs());

    max_length = get_max_length(
        problem1.get_p(), problem1.get_jobs(), problem1.get_N_opers(), problem1.get_r(), *next_op);
    (*limit) = max_length / ((Windows + 1) - window);
    (*limit) = (int)((*limit) * toler);

    windows_with_traslape(&problem1, limit, sum_operations, output, window, next_op, times_machines,
        problem2.get_N_workcenter(), &jobs_add, reserv_op, due_date);
    Workcenters_of_the_Window(output, &problem2, window, problem1.get_jobs(), jobs_add);

    fclose(output);
}

void windows_Jobs(FILE* output,
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
    int N_opers,
    int* due_date)
{
    int add = 0, largo_total = 0, j, k, limite;

    if (f > 1.0)
    {
        limite = num_op + next_op;
        if (ventana == 1)
        {
            for (k = 0; k < N_opers; k++) add = add + p[i][k];
            add = add + r;
            largo_total = (int)(add * f);
            due_date[i] = largo_total;
        }
        else
        {
            largo_total = due_date[i];
        }
        for (k = limite; k < N_opers; k++) largo_total = largo_total - p[i][k];
        d = largo_total;
        f = 0;
    }
    else
        d = d * ventana;

    fprintf(output, "%d\t%d\t%.1f\t%d\t", w, r, f, num_op);
    for (j = next_op; j < (num_op + next_op); j++) fprintf(output, "%d\t%d\t", maq[i][j], p[i][j]);

    if (num_op > 0)
        fprintf(output, "%d\n", d);
    else
    {
        if (sum_operations >= N_opers)
            fprintf(output, "%d\n", d);
        else
            fprintf(output, "%d\n", r);
    }
}

void windows_Workcenter(FILE* output, int N_maq, int** list_speeds, int i)
{
    int j = 0;
    if (N_maq == 1)
        fprintf(output, "%d\t", list_speeds[i][j]);
    else
    {
        fprintf(output, "\t%d\t", N_maq);
        for (j = 0; j < N_maq; j++) fprintf(output, "%d\t", list_speeds[i][j]);
    }
    fprintf(output, "\n");
}

void Read_times_machines(int* times_machines, int* W_machines)
{
    FILE* input;
    int v[6];
    int n_jobs, n_work_center, machines, n_operations;
    int i, j, k, m, valor, valor1, valor2, valor3, total = 0;

#ifdef V_UNIX
    char* name = "INPUT/gantt/gantt";
#else
    char* name = "gantt";
#endif

    if ((input = fopen(name, "r")) == NULL)
    {
        printf("Archivo no encontrado\n");
        exit(1);
    }

    fscanf(input, "%d", &n_jobs);
    for (i = 0; i < n_jobs; i++)
        for (j = 0; j < 6; j++) fscanf(input, "%d", &v[j]);
    fscanf(input, "%d", &n_work_center);
    for (j = 0; j < n_work_center; j++)
    {
        fscanf(input, "%d", &machines);
        for (k = 0; k < machines; k++)
        {
            fscanf(input, "%d", &n_operations);
            for (m = 0; m < n_operations; m++)
            {
                fscanf(input, "%d", &valor1);
                fscanf(input, "%d", &valor2);
                fscanf(input, "%d", &valor3);
                fscanf(input, "%d", &valor);
            }
            times_machines[total] = valor;
            total++;
            valor = 0;
        }
        W_machines[j] = machines;
    }
    fclose(input);
}

int* Read_times_jobs(int* n)
{
    FILE* input;
    int v[5];
    int n_jobs;
    int i, j, valor;
    int* times_jobs;

    char* name = "gantt";

    if ((input = fopen(name, "r")) == NULL)
    {
        printf("Archivo no encontrado\n");
        exit(1);
    }

    fscanf(input, "%d", &n_jobs);
    *n = n_jobs;
    times_jobs = new int[n_jobs];
    for (i = 0; i < n_jobs; i++) times_jobs[i] = 0;
    for (i = 0; i < n_jobs; i++)
    {
        for (j = 0; j < 5; j++) fscanf(input, "%d", &v[j]);
        fscanf(input, "%d", &valor);
        times_jobs[i] = valor;
    }
    fclose(input);
    return (times_jobs);
}

int* Read_release_jobs(int* n)
{
    FILE* input;
    int v[6];
    int n_jobs;
    int i, j;
    int* release_jobs;

    char* name = "gantt";

    if ((input = fopen(name, "r")) == NULL)
    {
        printf("Archivo no encontrado\n");
        exit(1);
    }

    fscanf(input, "%d", &n_jobs);
    *n = n_jobs;
    release_jobs = new int[n_jobs];
    for (i = 0; i < n_jobs; i++) release_jobs[i] = 0;
    for (i = 0; i < n_jobs; i++)
    {
        for (j = 0; j < 6; j++)
        {
            fscanf(input, "%d", &v[j]);
            if (j == 2) release_jobs[i] = v[j];
        }
    }
    fclose(input);
    return (release_jobs);
}

void windows_faltantes(FILE* output, int* times_machines, int N_workcenter, int* jobs_add)
{
    int i, w = 10, opers = 1, r = 0;
    double f = -1.0;
    for (i = 0; i < N_workcenter; i++)
    {
        if (times_machines[i] != 0)
        {
            fprintf(output, "%d\t%d\t%.1f\t%d\t%d\t%d\t%d\n", w, r, f, opers, i, times_machines[i],
                times_machines[i]);
            (*jobs_add)++;
        }
    }
}

int get_max_length(int** p, int jobs, int* N_opers, int* r, int* next_op)
{
    int i, j, max = 0, add = 0;
    for (i = 0; i < jobs; i++)
    {
        add = add + r[i];
        for (j = next_op[i]; j < N_opers[i]; j++) add = add + p[i][j];
        if (max < add) max = add;
        add = 0;
    }
    return (max);
}

int get_max_machine(int* times_machines, int* W_machines, int num_machines)
{
    int i, max_machine = 0;
    if (times_machines != NULL)
    {
        Read_times_machines(times_machines, W_machines);
        for (i = 0; i < num_machines; i++)
        {
            if (max_machine < times_machines[i]) max_machine = times_machines[i];
        }
    }
    return (max_machine);
}

double read_tolerancia()
{
    double toler;
    toler = 1.5;
    return (toler);
}

int valida_traslape(int window, int max_machine, double Tras_win)
{
    int sw = 1, traslape;
    if (window > 1)
    {
        while (sw)
        {
            traslape = (int)(max_machine * Tras_win);
            if (traslape > max_machine)
            {
                printf("\n Error el traslape debe ser menor a la ventana anterior\n");
                exit(1);
            }
            else
                sw = 0;
        }
    }
    return (traslape);
}

/*double read_traslape()
{
    double traslape;
    printf("\n *************************************************************");
    printf("\n Ingrese el traslape de la ventana : ");
    scanf("%lf",&traslape);
    printf("\n *************************************************************\n");
    return(traslape);
}
*/
void iniciate_times_machines(int N_workcenter, int** times_machines)
{
    int i;
    for (i = 0; i < N_workcenter; i++) (*times_machines)[i] = 0;
}

// Cambios por RENE y LUIS
void calculate_release(Graph* p_optimal_Graph,
    int** next_op,
    int** r,
    double tras,
    int** times_machines,
    int** workcenters,
    int jobs,
    int** finishing_times,
    int** release_times)
{
    int i, j, k, valor = 0, valor2 = 0, valor3 = 0, save = 0, m = 0;
    int step = 0, dif = 0, dif2 = 0, rest = 0, inc = 0;
    int *times_jobs, tt = 0, n_jobs;
    int* release_jobs;
    int traslape;
    int difer = 0, max_difer = 0, max_job = 0;
    times_jobs = Read_times_jobs(&n_jobs);
    release_jobs = Read_release_jobs(&n_jobs);
    for (i = 0; i < jobs; i++)
    {
        difer = times_jobs[i] - release_jobs[i];
        if (difer > max_difer) max_difer = difer;
        if (times_jobs[i] > max_job) max_job = times_jobs[i];
    }
    traslape = max_job - ((int)(max_difer * tras));
    for (i = 0; i < jobs; i++)
    {
        valor2 = 0;
        // traslape =times_jobs[i]-((int)(max_difer*tras));
        if (p_optimal_Graph->Get_Number_Steps_Job(i) == 0 && (*next_op)[i] != 0)
        {
            for (k = 0; release_times[i][k] != -1; k++)
            {
                if (release_times[i][k] <= traslape)
                    valor2++;
                else
                    break;
                m = workcenters[i][k];
                if ((*times_machines)[m] < finishing_times[i][k])
                    (*times_machines)[m] = finishing_times[i][k];
            }
            (*r)[i] = finishing_times[i][valor2 - 1];
        }
        else if (p_optimal_Graph->Get_Number_Steps_Job(i) == 0 && (*next_op)[i] == 0)
            (*r)[i] = release_times[i][0];

        else
        {
            inc = (*next_op)[i] - p_optimal_Graph->Get_Number_Steps_Job(i);
            for (j = 1; j <= p_optimal_Graph->Get_Number_Steps_Job(i); j++)
            {
                valor = p_optimal_Graph->Get_Release_Time(i, j);
                release_times[i][inc + j - 1] = valor;
                valor3 = p_optimal_Graph->Get_Finishing_Time(i, j);
                finishing_times[i][inc + j - 1] = valor3;
                if (valor <= traslape)
                    valor2++;
                else
                    break;

                save = valor3;
                dif = p_optimal_Graph->Get_Number_Steps_Job(i) - valor2;
                rest = (*next_op)[i] - dif;
                m = workcenters[i][rest - 1];
                if ((*times_machines)[m] < save) (*times_machines)[m] = save;
            }

            step = p_optimal_Graph->Get_Number_Steps_Job(i) - valor2;
            dif2 = (*next_op)[i];
            (*next_op)[i] = dif2 - step;
            if ((*next_op)[i] != 0) (*r)[i] = save;
            if (valor2 == 0) (*r)[i] = valor;
        }
    }
    {
        int jj;
        int maq = 0;
        for (jj = jobs; jj < n_jobs; jj++)
        {
            tt = times_jobs[jj];
            if ((*times_machines)[maq] < tt) (*times_machines)[maq] = tt;
            maq++;
        }
    }
    delete[] times_jobs;
}

// Fin cambios por RENE y LUIS

void calculate_release_machine(int jobs,
    int** next_op,
    int* N_opers,
    int** times_machines,
    int** list_speeds,
    int* W_machines,
    int** workcenters,
    int** p,
    int** r)
{
    int i, machine_choose, aux, aux2;
    for (i = 0; i < jobs; i++)
    {
        if ((*next_op)[i] != N_opers[i])
        {
            aux2 = (*next_op)[i];
            if ((*next_op)[i] != 0)
            {
                machine_choose = Balance_times_machines((*times_machines), list_speeds, W_machines,
                    workcenters[i][(*next_op)[i]], p[i][(*next_op)[i]]);
                if ((*times_machines)[machine_choose] > (*r)[i])
                {
                    aux = (*times_machines)[machine_choose];
                    (*r)[i] = (*times_machines)[machine_choose];
                }
            }
        }
    }
}

int Balance_times_machines(int* times_machines, int** list_speeds, int* W_machines, int maq, int p)
{
    int i, j, lenght, min_lenght, machine = 0, save_machine;
    for (i = 0; i < maq; i++) machine = machine + W_machines[i];
    for (j = 0; j < W_machines[maq]; j++)
    {
        lenght = times_machines[machine] + (p / list_speeds[maq][j]);
        if (j == 0)
        {
            min_lenght = lenght;
            save_machine = machine;
        }
        machine++;
        if (lenght < min_lenght)
        {
            min_lenght = lenght;
            save_machine = machine;
        }
    }
    return (save_machine);
}
void iniciate_reserv_op(int** reserv_op, int** next_op, int jobs)
{
    int i;
    for (i = 0; i < jobs; i++) (*reserv_op)[i] = (*next_op)[i];
}

void windows_with_traslape(Input_File_Jobs* p_problem1,
    int* limit,
    int** sum_operations,
    FILE* output,
    int window,
    int** next_op,
    int* times_machines,
    int N_workcenter,
    int* jobs_add,
    int* reserv_op,
    int* due_date)
{
    int* operations;
    int i, k, num_op = 0, range = 0;
    operations = new int[(*p_problem1).get_jobs()];
    for (i = 0; i < (*p_problem1).get_jobs(); i++)
    {
        range = range + (*p_problem1).get_r1(i);
        for (k = (*next_op)[i]; k < (*p_problem1).get_N_opers1(i); k++)
        {
            if (range <= (*limit))
                num_op++;
            else
                break;
            range = range + (*p_problem1).get_p1(i, k);
        }

        (*sum_operations)[i] = (*sum_operations)[i] + num_op;
        operations[i] = num_op;
        windows_Jobs(output, (*p_problem1).get_w1(i), (*p_problem1).get_r1(i), (*p_problem1).get_f1(i),
            num_op, (*p_problem1).get_workcenters(), (*p_problem1).get_p(), (*p_problem1).get_d1(i) / Windows,
            window, (*next_op)[i], i, (*sum_operations)[i], (*p_problem1).get_N_opers1(i), due_date);

        if ((i + 1) == (*p_problem1).get_jobs() && window > 1)
        {
            // check_faltantes(&faltantes,(*p_problem1).get_workcenters(),(*p_problem1).get_jobs(),reserv_op,operations,N_workcenter);
            windows_faltantes(output, times_machines, N_workcenter, jobs_add);
        }

        (*next_op)[i] = (*next_op)[i] + num_op;
        range = 0;
        num_op = 0;
    }
}

void Workcenters_of_the_Window(
    FILE* output, Input_File_Workcenters* p_problem2, int window, int jobs, int jobs_add)
{
    int j;
    fprintf(output, "%d\n", (*p_problem2).get_N_workcenter());
    for (j = 0; j < (*p_problem2).get_N_workcenter(); j++)
        windows_Workcenter(output, (*p_problem2).get_N_maq1(j), (*p_problem2).get_list_speeds(), j);
    if (window > 1)
    {
        fseek(output, 0, SEEK_SET);
        fprintf(output, "%d", jobs + jobs_add);
        fprintf(output, "\t");
    }
}
