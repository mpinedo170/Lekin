#include "StdAfx.h"

#include "file_gantt.h"
#include "globals.h"
#include "input_file.h"
#include "optimize.h"
#include "paramet.h"
#include "sol_grph.h"

#include "fun_w_traslape.h"

int Windows;
double Read_Par_Ventanas(int);

void Leer_gantts(Input_File_gantts_p1* gantt_jobs, Input_File_gantts_p2* gantt_dist);
void unir_gantts(Input_File_gantts_p1* gantt_jobs, Input_File_gantts_p2* gantt_dist);
int buscar_job(int** lista, int job, int n_op, int j);
void guardar_gantt(Input_File_gantts_p1 gantt_jobs, Input_File_gantts_p2 gantt_dist);

void windows_with_traslape(int iter)
{
    int m, k = 0, centers = 0, num_machines = 0, limit = 0, mayor = 0;
    int v, i;

    // Clases que permiten guardar la informacion del archivo de entrada

    Input_File_Jobs problem1;
    Input_File_Workcenters problem2;

    // Clases que permiten guardar la informacion de las cartas gantts

    Input_File_gantts_p1* gantt_jobs;
    Input_File_gantts_p2* gantt_dist;

    // variables que se utilizan para volver a iterar el programa

    int *W_machines = NULL, *times_machines = NULL;
    int *next_op, *sum_operations;
    int **finishing_times, **release_times, *due_date;
    double Traslape;
    int tiempo = 0;

    int last_time;  // first_time is a global variable.

    Graph* p_optimal_Graph = NULL;  // It will store the optimal solution so far.

    Plant* p_main_Plant;  // Stores the data of work centers and clusters.

    Parameters main_Parameters;

    FILE* many_output_file = fopen("results", "w");
    int size, version, instance;
    int from_version, to_version;
    int from_instance, to_instance;
    main_Parameters.Read_General(&size, &version, &instance);
    if (version == -1)
    {
        from_version = 0;
        to_version = 8;
    }
    else
    {
        from_version = version;
        to_version = version + 1;
    }
    if (instance == -1)
    {
        from_instance = 0;
        to_instance = 22;
    }
    else
    {
        from_instance = instance;
        to_instance = instance + 1;
    }
    FILE* statistics_file = fopen("stats", "w");

    Traslape = Read_Par_Ventanas(iter);

    for (v = from_version; v < to_version; v++)
    {
        for (i = from_instance; i < to_instance; i++)
        {
            if (Windows > 1)
            {
                num_machines = 0;

                // Funcion que guarda la informacion del archivo en las clases correspondientes

                save_Information("input.dec", &problem1, &problem2, &mayor, &num_machines);

                // Creo espacio para release_times y finishing_times que almacenaran el tiempo
                // de incio y de termino de las operaciones programadas

                release_times = new int*[problem1.get_jobs()];
                finishing_times = new int*[problem1.get_jobs()];
                due_date = new int[problem1.get_jobs()];
                for (k = 0; k < problem1.get_jobs(); k++)
                {
                    release_times[k] = new int[problem1.get_N_opers1(k) + 1];
                    finishing_times[k] = new int[problem1.get_N_opers1(k) + 1];
                    {
                        int y;
                        for (y = 0; y < problem1.get_N_opers1(k) + 1; y++)
                        {
                            release_times[k][y] = -1;
                            finishing_times[k][y] = -1;
                        }
                    }
                }

                // Fin creacion espacio para release_times y finishing_times

                // Se crea espacio para la estructura next_op y sum_operations que almacena
                // la proxima operacion a ser programada

                next_op = new int[problem1.get_jobs()];
                sum_operations = new int[problem1.get_jobs()];
                for (m = 0; m < problem1.get_jobs(); m++)
                {
                    sum_operations[m] = 0;
                    next_op[m] = 0;
                }

                // Fin de creacion de espacio
                // Creacion de arreglos de cartas gantt

                gantt_jobs = new Input_File_gantts_p1[Windows];
                gantt_dist = new Input_File_gantts_p2[Windows];

                // fin creacion de arreglos

                for (k = 1; k <= Windows; k++)
                {
                    Create_Windows(problem1, problem2, mayor, p_optimal_Graph, k, &next_op, &limit,
                        &sum_operations, times_machines, W_machines, num_machines, Traslape, finishing_times,
                        release_times, due_date);
                    switch (k)
                    {
#ifdef V_UNIX
                        case 1:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent0";
                            break;
                        }
                        case 2:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent1";
                            break;
                        }
                        case 3:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent2";
                            break;
                        }
                        case 4:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent3";
                            break;
                        }
                        case 5:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent4";
                            break;
                        }
                        case 6:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent5";
                            break;
                        }
                        case 7:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent6";
                            break;
                        }
                        case 8:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent7";
                            break;
                        }
                        case 9:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent8";
                            break;
                        }
                        case 10:
                        {
                            par_name_file_static_data = "INPUT/resultados/vent9";
                            break;
                        }
#endif
#ifndef V_UNIX
                        case 1:
                        {
                            par_name_file_static_data = "vent0";
                            break;
                        }
                        case 2:
                        {
                            par_name_file_static_data = "vent1";
                            break;
                        }
                        case 3:
                        {
                            par_name_file_static_data = "vent2";
                            break;
                        }
                        case 4:
                        {
                            par_name_file_static_data = "vent3";
                            break;
                        }
                        case 5:
                        {
                            par_name_file_static_data = "vent4";
                            break;
                        }
                        case 6:
                        {
                            par_name_file_static_data = "vent5";
                            break;
                        }
                        case 7:
                        {
                            par_name_file_static_data = "vent6";
                            break;
                        }
                        case 8:
                        {
                            par_name_file_static_data = "vent7";
                            break;
                        }
                        case 9:
                        {
                            par_name_file_static_data = "vent8";
                            break;
                        }
                        case 10:
                        {
                            par_name_file_static_data = "vent9";
                            break;
                        }
#endif
                    }

                    par_upper_bound = 0;
                    dec_input(statistics_file, &p_optimal_Graph, &p_main_Plant, v);

                    last_time = int(clock()) / CLK_TCK;

                    fprintf(many_output_file, "%d  %d      ", p_optimal_Graph->Get_Objective(),
                        last_time - first_time);
                    fprintf(many_output_file, "( %d ) ", hitting_time);
                    if ((par_option != LOWER_BOUND) && (p_optimal_Graph->Get_Objective() == par_upper_bound))
                        fprintf(many_output_file, "*\n");
                    else
                        fprintf(many_output_file, "\n");

                    times_machines = new int[num_machines];

                    centers = p_optimal_Graph->Number_Centers_Plant();
                    W_machines = new int[centers];
                    p_main_Plant->Terminate();
                    delete p_main_Plant;
                    delete[] par_name_output_file;
                    tiempo = tiempo + (last_time - first_time);

                    Leer_gantts(&gantt_jobs[k - 1], &gantt_dist[k - 1]);
                }

                unir_gantts(gantt_jobs, gantt_dist);
                p_optimal_Graph->Terminate();
                delete p_optimal_Graph;
                delete[] times_machines;
                times_machines = NULL;
                tiempo = 0;
            }
            else
            {
                gantt_jobs = new Input_File_gantts_p1[1];
                gantt_dist = new Input_File_gantts_p2[1];
                par_name_file_static_data = "input.dec";
                dec_input(statistics_file, &p_optimal_Graph, &p_main_Plant, v);
                last_time = int(clock()) / CLK_TCK;
                fprintf(many_output_file, "%d  %d      ", p_optimal_Graph->Get_Objective(),
                    last_time - first_time);
                fprintf(many_output_file, "( %d ) ", hitting_time);
                if ((par_option != LOWER_BOUND) && (p_optimal_Graph->Get_Objective() == par_upper_bound))
                    fprintf(many_output_file, "*\n");
                else
                    fprintf(many_output_file, "\n");

                p_main_Plant->Terminate();
                delete p_main_Plant;
                delete[] par_name_output_file;
                p_optimal_Graph->Terminate();
                delete p_optimal_Graph;
                Leer_gantts(&gantt_jobs[0], &gantt_dist[0]);
                unir_gantts(gantt_jobs, gantt_dist);
            }
        }
    }

    if (par_option == HEURISTIC)
    {
        fprintf(many_output_file, "\n");
        fprintf(many_output_file, "Number_Branches_Main:      %d\n", par_max_branches_main);
        fprintf(many_output_file, "Threshold_Main:            %1.3lf\n", par_threshold_main);
        fprintf(many_output_file, "Number_Reoptimized:        %d\n", par_reoptimization);
        fprintf(many_output_file, "Number_Branches_Oracle:    %d\n", par_max_branches_oracle);
        fprintf(many_output_file, "Threshold_Oracle:          %1.3lf\n", par_threshold_oracle);
        fprintf(many_output_file, "Slack_Active:                      %d\n", par_slack_active);
    }
    else if (par_option == LOWER_BOUND)
    {
        fprintf(many_output_file, "\n");
        fprintf(
            many_output_file, "Transition_Speed(0->Disjunctive;1000->Active): %d\n", par_transition_speed);
        fprintf(many_output_file, "Selection(0->Early;1->Estimate;2->Bottle;3->Bound):    %d\n",
            par_selection_rule);
    }

    fclose(many_output_file);
    fclose(statistics_file);
    if (par_option == 5) printf("\n");
}

// Funciones para generar la carta gantt final

// Esta funcion permite estblecer la cantidad de ventanas y el traslape a utilizar

double Read_Par_Ventanas(int iter)
{
    int Num_windows[] = {1};
    double List_traslapes[] = {0.5};
    Windows = Num_windows[iter];
    return (List_traslapes[iter]);
}

// Esta funcion permite leer las cartas gantts generadas

void Leer_gantts(Input_File_gantts_p1* gantt_jobs, Input_File_gantts_p2* gantt_dist)
{
    FILE* input;
    int n_jobs, n_workcenters, op_mac;
    int i, j, aux;

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

    // Resumen de infortmacion de los Jobs

    fscanf(input, "%d", &n_jobs);
    (*gantt_jobs).put_n_jobs(n_jobs);

    (*gantt_jobs).tam_N_opers1(n_jobs);
    (*gantt_jobs).tam_r1(n_jobs);
    (*gantt_jobs).tam_d1(n_jobs);
    (*gantt_jobs).tam_w1(n_jobs);
    (*gantt_jobs).tam_c1(n_jobs);

    for (i = 0; i < n_jobs; i++)
    {
        fscanf(input, "%d", &aux);
        fscanf(input, "%d", &aux);
        (*gantt_jobs).put_N_opers(aux, i);
        fscanf(input, "%d", &aux);
        (*gantt_jobs).put_r(aux, i);
        fscanf(input, "%d", &aux);
        (*gantt_jobs).put_d(aux, i);
        fscanf(input, "%d", &aux);
        (*gantt_jobs).put_w(aux, i);
        fscanf(input, "%d", &aux);
        (*gantt_jobs).put_c(aux, i);
    }

    // Distribucion de las operaciones en las maquinas

    fscanf(input, "%d", &n_workcenters);
    (*gantt_dist).put_workcenters(n_workcenters);

    (*gantt_dist).tam_num_machines1(n_workcenters);
    (*gantt_dist).tam_opers_mac1(n_workcenters);
    (*gantt_dist).tam_job1(n_workcenters);
    (*gantt_dist).tam_step1(n_workcenters);
    (*gantt_dist).tam_i1(n_workcenters);
    (*gantt_dist).tam_f1(n_workcenters);

    for (i = 0; i < n_workcenters; i++)
    {
        fscanf(input, "%d", &aux);
        (*gantt_dist).put_num_machines(aux, i);

        fscanf(input, "%d", &op_mac);
        (*gantt_dist).put_opers_mac(op_mac, i);

        (*gantt_dist).tam_job2(op_mac, i);
        (*gantt_dist).tam_step2(op_mac, i);
        (*gantt_dist).tam_i2(op_mac, i);
        (*gantt_dist).tam_f2(op_mac, i);

        for (j = 0; j < op_mac; j++)
        {
            fscanf(input, "%d", &aux);
            (*gantt_dist).put_job(aux, i, j);
            fscanf(input, "%d", &aux);
            (*gantt_dist).put_step(aux, i, j);
            fscanf(input, "%d", &aux);
            (*gantt_dist).put_i(aux, i, j);
            fscanf(input, "%d", &aux);
            (*gantt_dist).put_f(aux, i, j);
        }
    }
    fclose(input);
}

// En esta funcion se unen las cartas gantts

void unir_gantts(Input_File_gantts_p1* gantt_jobs, Input_File_gantts_p2* gantt_dist)
{
    int i, j, k, jj;

    Input_File_gantts_p1 gantt_final_jobs;
    Input_File_gantts_p2 gantt_final_dist;

    // Resumen de informacion de los Jobs

    gantt_final_jobs.tam_N_opers1(gantt_jobs[0].get_n_jobs());
    gantt_final_jobs.tam_r1(gantt_jobs[0].get_n_jobs());
    gantt_final_jobs.tam_d1(gantt_jobs[0].get_n_jobs());
    gantt_final_jobs.tam_w1(gantt_jobs[0].get_n_jobs());
    gantt_final_jobs.tam_c1(gantt_jobs[0].get_n_jobs());

    // fin de resumen

    // Distribucion de las operaciones en las maquinas

    gantt_final_dist.tam_num_machines1(gantt_dist[0].get_workcenters());
    gantt_final_dist.tam_opers_mac1(gantt_dist[0].get_workcenters());
    gantt_final_dist.tam_job1(gantt_dist[0].get_workcenters());
    gantt_final_dist.tam_step1(gantt_dist[0].get_workcenters());
    gantt_final_dist.tam_i1(gantt_dist[0].get_workcenters());
    gantt_final_dist.tam_f1(gantt_dist[0].get_workcenters());

    for (i = 0; i < gantt_dist[0].get_workcenters(); i++)
    {
        gantt_final_dist.tam_job2(gantt_jobs[0].get_n_jobs(), i);
        gantt_final_dist.tam_step2(gantt_jobs[0].get_n_jobs(), i);
        gantt_final_dist.tam_i2(gantt_jobs[0].get_n_jobs(), i);
        gantt_final_dist.tam_f2(gantt_jobs[0].get_n_jobs(), i);
    }

    // Inicializacion de los jobs de la distribucion

    for (i = 0; i < gantt_dist[0].get_workcenters(); i++)
    {
        for (j = 0; j < gantt_jobs[0].get_n_jobs(); j++)
        {
            gantt_final_dist.put_job(-1, i, j);
        }
    }

    // fin creacion de la distribucion

    // Llenado de tabla de resumen de la gantt
    gantt_final_jobs.put_n_jobs(gantt_jobs[0].get_n_jobs());
    for (i = 0; i < gantt_final_jobs.get_n_jobs(); i++)
    {
        gantt_final_jobs.put_N_opers(gantt_jobs[0].get_n_jobs(), i);
        gantt_final_jobs.put_r(gantt_jobs[0].get_r(i), i);
        gantt_final_jobs.put_d(gantt_jobs[Windows - 1].get_d(i), i);
        gantt_final_jobs.put_w(gantt_jobs[0].get_w(i), i);
        gantt_final_jobs.put_c(gantt_jobs[Windows - 1].get_c(i), i);
    }

    // fin llenado tabla de resumen

    // Llenado de la distribucion en la planta

    gantt_final_dist.put_workcenters(gantt_dist[0].get_workcenters());
    for (i = 0; i < gantt_dist[0].get_workcenters(); i++)
    {
        gantt_final_dist.put_num_machines(gantt_dist[0].get_num_machines(i), i);
        gantt_final_dist.put_opers_mac(gantt_jobs[0].get_n_jobs(), i);
        jj = gantt_final_dist.get_opers_mac(i) - 1;
        for (k = Windows - 1; k >= 0; k--)
        {
            for (j = gantt_dist[k].get_opers_mac(i) - 1; j > 0; j--)
            {
                if (buscar_job(gantt_final_dist.get_job2(), gantt_dist[k].get_job(i, j),
                        gantt_final_dist.get_opers_mac(i), i) == 0)
                {
                    gantt_final_dist.put_job(gantt_dist[k].get_job(i, j), i, jj);
                    gantt_final_dist.put_step(gantt_dist[k].get_step(i, j), i, jj);
                    gantt_final_dist.put_i(gantt_dist[k].get_i(i, j), i, jj);
                    gantt_final_dist.put_f(gantt_dist[k].get_f(i, j), i, jj);
                    jj--;
                }
            }
        }
        gantt_final_dist.put_job(gantt_dist[0].get_job(i, j), i, jj);
        gantt_final_dist.put_step(gantt_dist[0].get_step(i, j), i, jj);
        gantt_final_dist.put_i(gantt_dist[0].get_i(i, j), i, jj);
        gantt_final_dist.put_f(gantt_dist[0].get_f(i, j), i, jj);
    }

    // Fin llenado de la planta de produccion

    guardar_gantt(gantt_final_jobs, gantt_final_dist);
}

// Esta funcion permite verificar si el job ha sido incluido en la carta gantt

int buscar_job(int** lista, int job, int n_op, int i)
{
    int j;
    for (j = n_op - 1; j >= 0; j--)
    {
        if (lista[i][j] == job) return (1);
    }
    return (0);
}

// Esta funcion permite generar la carta gantt final

void guardar_gantt(Input_File_gantts_p1 gantt_jobs, Input_File_gantts_p2 gantt_dist)
{
    FILE* output;
    int i, j;

#ifdef V_UNIX
    char* name = "INPUT/gantt/gantt_final";
#else
    char* name = "gantt_final";
#endif
    if ((output = fopen(name, "w")) == NULL)
    {
        printf(" El archivo no se puede crear\n");
        exit(1);
    }

    // Guarda el resumen de la gantt final

    fprintf(output, "%d\n", gantt_jobs.get_n_jobs());
    for (i = 0; i < gantt_jobs.get_n_jobs(); i++)
    {
        fprintf(output, "%d     %d     %d     %d     %d     %d\n", i, gantt_jobs.get_N_opers(i),
            gantt_jobs.get_r(i), gantt_jobs.get_d(i), gantt_jobs.get_w(i), gantt_jobs.get_c(i));
    }

    // Guarda la distribucion de la gantt final

    fprintf(output, "%d\n", gantt_dist.get_workcenters());
    for (i = 0; i < gantt_dist.get_workcenters(); i++)
    {
        fprintf(output, "   %d\n", gantt_dist.get_num_machines(i));
        fprintf(output, "       %d\n", gantt_dist.get_opers_mac(i));
        for (j = 0; j < gantt_dist.get_opers_mac(i); j++)
        {
            fprintf(output, "             %d     %d     %d    %d\n", gantt_dist.get_job(i, j),
                gantt_dist.get_step(i, j), gantt_dist.get_i(i, j), gantt_dist.get_f(i, j));
        }
    }

    fclose(output);
}
