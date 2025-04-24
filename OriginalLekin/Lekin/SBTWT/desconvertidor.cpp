#include "StdAfx.h"

#include "file_gantt.h"

void Leer_gantts_2(Input_File_gantts_p1* gantt_jobs, Input_File_gantts_p2* gantt_dist);
void nuevo_formato(Input_File_gantts_p2* gantt_dist);

void desconvertidor()
{
    Input_File_gantts_p1 gantt_jobs;
    Input_File_gantts_p2 gantt_dist;

    Leer_gantts_2(&gantt_jobs, &gantt_dist);
    nuevo_formato(&gantt_dist);
}

void Leer_gantts_2(Input_File_gantts_p1* gantt_jobs, Input_File_gantts_p2* gantt_dist)
{
    FILE* input;
    int n_jobs, n_workcenters, op_mac;
    int i, j, aux;

    char* name = "gantt_final";
    input = fopen(name, "r");

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

void nuevo_formato(Input_File_gantts_p2* gantt_dist)
{
    FILE* output;
    double cont = 0.0;
    int i, j;
    output = fopen("_user.seq", "w");
    fprintf(output, "Schedule\tShifting Bottleneck / sum(wT)\n");
    for (i = 0; i < (*gantt_dist).get_workcenters(); i++)
    {
        fprintf(output, "Machine:\t%.1lf\n", cont);
        for (j = 0; j < (*gantt_dist).get_opers_mac(i); j++)
            fprintf(output, "Oper:\t\t%d\n", (*gantt_dist).get_job(i, j));
        cont++;
    }
    fclose(output);
}
