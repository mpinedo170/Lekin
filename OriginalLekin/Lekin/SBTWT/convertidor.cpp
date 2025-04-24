#include "StdAfx.h"

#define MAX 1000

typedef struct
{
    int release, due, weigth, opers;
    int workcenter[MAX];
    int p[MAX];
} job;

job instance[MAX];

int Leer_jobs();
int Leer_machines();
void construir_entrada(int jobs, int workcenters);
void construir_par_heu(int workcenters);
void construir_par_rest();
void construir_par_gen(int workcenters);

void convertidor()
{
    int jobs, workcenters;
    jobs = Leer_jobs();
    workcenters = Leer_machines();
    construir_entrada(jobs, workcenters);
    construir_par_heu(workcenters);
    construir_par_rest();
    construir_par_gen(workcenters);
}

int Leer_jobs()
{
    FILE* input;
    int c = 0;
    char w[10], p[10];
    char s[256], workcenter[256];
    int valor, i, j, k;
    int cont_jobs = 0, cont_opers = 0;

    if ((input = fopen("_user.job", "r")) == NULL)
    {
        printf(" No se puede abrir el archivo \n");
        exit(1);
    }
    while (c != EOF)
    {
        while ((_tcscmp(s, "Release:")) != 0) fscanf(input, "%s", s);
        fscanf(input, "%d", &valor);
        instance[cont_jobs].release = valor;
        fscanf(input, "%s", s);
        fscanf(input, "%d", &valor);
        instance[cont_jobs].due = valor;
        fscanf(input, "%s", s);
        fscanf(input, "%d", &valor);
        instance[cont_jobs].weigth = valor;
        while ((_tcscmp(s, "Job:")) != 0 && c != EOF)
        {
            fscanf(input, "%s", s);
            fscanf(input, "%s", workcenter);
            c = getc(input);
            if ((_tcscmp(s, "Job:")) == 0 || c == EOF) break;
            for (i = 3, j = 0; workcenter[i] != ';'; i++, j++) w[j] = workcenter[i];
            w[j] = '\0';
            instance[cont_jobs].workcenter[cont_opers] = atol(w);
            for (k = i + 1, j = 0; workcenter[k] != ';'; k++, j++) p[j] = workcenter[k];
            p[j] = '\0';
            instance[cont_jobs].p[cont_opers] = atol(p);

            cont_opers++;
        }
        instance[cont_jobs].opers = cont_opers;
        cont_opers = 0;
        cont_jobs++;
    }
    return (cont_jobs);
}

int Leer_machines()
{
    FILE* input;
    int c = ' ';
    char s[15], s1[15];
    int cont_workcenters = 0;
    if ((input = fopen("_user.mch", "r")) == NULL)
    {
        printf(" No se puede abrir el archivo \n");
        exit(1);
    }
    fscanf(input, "%s", s);
    while (c != EOF)
    {
        while ((_tcscmp(s, "Workcenter:")) != 0 && (_tcscmp(s1, "Workcenter:")) != 0)
        {
            fscanf(input, "%s", s);
            c = getc(input);
            if (c == EOF) break;
            fscanf(input, "%s", s1);
        }
        cont_workcenters++;
        s[0] = ' ';
        s1[0] = ' ';
    }
    return (cont_workcenters - 1);
}

void construir_entrada(int jobs, int workcenters)
{
    FILE* output;
    int i, j;
    double f;
    output = fopen("input.dec", "w");
    fprintf(output, "%d\n", jobs);
    for (i = 0; i < jobs; i++)
    {
        f = -1.0;
        fprintf(output, "%d\t%d\t%.1f\t%d", instance[i].weigth, instance[i].release, f, instance[i].opers);
        for (j = 0; j < instance[i].opers; j++)
        {
            fprintf(output, "\t%d\t%d", instance[i].workcenter[j], instance[i].p[j]);
        }
        fprintf(output, "\t%d", instance[i].due);

        fprintf(output, "\n");
    }
    fprintf(output, "%d\n", workcenters);

    for (i = 0; i < workcenters; i++) fprintf(output, "\t%d\n", 1);

    fclose(output);
}

void construir_par_heu(int workcenters)
{
    FILE* output;
    int i;
    output = fopen("par_heu", "w");
    fprintf(output, "Clusters_Description:\n");
    fprintf(output, "%d\n", workcenters);
    for (i = 0; i < workcenters; i++) fprintf(output, "1\t%d\n", i);
    fprintf(output, "Number_Branches_Main:\t2\n");
    fprintf(output, "Threshold_Main:\t0.0\n");
    fprintf(output, "Number_Reoptimized:\t%d\n", workcenters);
    fprintf(output, "Number_Branches_Oracle:\t%d\n", workcenters);
    fprintf(output, "Threshold_Oracle:\t0.0\n");
    fprintf(output, "Number_Skips_Graph:\t0\n");
    fprintf(output, "Upper_Bound_Factor:\t-1\n");
    fclose(output);
}

void construir_par_rest()
{
    FILE* output;
    output = fopen("par_rest", "w");
    fprintf(output, "Output_File:\t\t output \n");
    fprintf(output, "K(0.00001->MS;5.0->ATC;99999.0->WSPT;-1->automatic):\t -1 \n");
    fprintf(output, "Slack_Active(0->active):\t\t0\n");
    fprintf(output, "Look_Ahead(0->non_delay):\t\t1500\n");
    fprintf(output, "Print(1=YES,0=NO):\t\t0\n");
    fprintf(output, "Animation_Schedule_Machine(1=YES,0=NO):\t\t0\n");
    fprintf(output, "Animation_Reoptimization(1=YES,0=NO):\t\t0\n");
    fprintf(output, "Animation_Schedule_Operation(1=YES,0=NO):\t\t0\n");

    fclose(output);
}

void construir_par_gen(int workcenters)
{
    FILE* output;
    output = fopen("par_gen", "w");
    fprintf(output, "Option(0=Heuristic,1=Branch&Bound,2=Local_Search):\t0\n");
    fprintf(output, "Objective(0=Makespan,1=Tardiness,2=Completion):\t1\n");
    fprintf(output, "Size(1->1x1;2->2x2;3->3x3;4->4x4;5->5x5;6->6x6;7->7x7;8->8x8;9->9x9;10->10x10):\t%d\n",
        workcenters);
    fprintf(output, "Version(-1->all):\t\t0\n");
    fprintf(output, "Instance(-1->all):\t\t0\n\n");
    fprintf(output, "ABZ5\t\t: 0\n");
    fprintf(output, "ABZ6\t\t: 1\n");
    fprintf(output, "LA16\t\t: 2\n");
    fprintf(output, "LA17\t\t: 3\n");
    fprintf(output, "LA18\t\t: 4\n");
    fprintf(output, "LA19\t\t: 5\n");
    fprintf(output, "LA20\t\t: 6\n");
    fprintf(output, "LA21\t\t: 7\n");
    fprintf(output, "LA22\t\t: 8\n");
    fprintf(output, "LA23\t\t: 9\n");
    fprintf(output, "LA24\t\t: 10\n");
    fprintf(output, "MT10\t\t: 11\n");
    fprintf(output, "ORB1\t\t: 12\n");
    fprintf(output, "ORB2\t\t: 13\n");
    fprintf(output, "ORB3\t\t: 14\n");
    fprintf(output, "ORB4\t\t: 15\n");
    fprintf(output, "ORB5\t\t: 16\n");
    fprintf(output, "ORB6\t\t: 17\n");
    fprintf(output, "ORB7\t\t: 18\n");
    fprintf(output, "ORB8\t\t: 19\n");
    fprintf(output, "ORB9\t\t: 20\n");
    fprintf(output, "ORB10\t\t: 21\n");

    fclose(output);
}
