/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                    D E F . H                         */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#ifndef DEF_H
#define DEF_H

/*****************          Rahmenbedingungen           *****************/

#define MAX_STAGES 30      // Max. Nr. of Stages
#define MAX_MACHINE_NR 20  // Max. Machinenr. in Stage
#define MAX_JOB_NR 100

// extern long RAND_MAX ;

/*****************            Strukturen                *****************/

/* Auftragsinformationen */
struct job
{
    int Job_Nr;
    int Release_Date;
    int Due_Date;
    int Weight;
    int Processing_Times[MAX_STAGES + 1];
    int Machine_Nr[MAX_STAGES + 1];
    int Completion_Time;
};

extern struct job JOBS[MAX_JOB_NR + 1];

/* Maschineninformationen */
struct machine
{
    int Occupied;
    int Best_Job_Order[MAX_JOB_NR + 1];
    int Act_Job_Order[MAX_JOB_NR + 1];
};

extern struct machine MACHINES[MAX_STAGES + 1][MAX_MACHINE_NR + 1];

/* Struktur eines Knoten des erweiterten disjunktiven Graphen. */
struct graph_node
{
    int Job_Nr;
    int Stage_Nr;
    int Machine_Nr;
    int Processing_Time;
    int Longest_Path;
    int SLP;
    int Input_Counter;
    int Mark;
    int Ident;
    int Pred_Job;
    int Succ_Job;
    struct graph_node* Pred_Operation_LP;
};

extern struct graph_node* GRAPH[MAX_STAGES + 1][MAX_JOB_NR];
extern int SAVE_LP[MAX_STAGES * MAX_JOB_NR];
extern struct graph_node* LAST_NODE;
extern struct graph_node* NODE1;
extern struct graph_node* NODE2;

/******************        Globale Variablen                 *****************/

/* An welche Stelle von JOBS sollen Auftragsdaten geschrieben werden */
extern int WHERE_IN_JOBS;

/* Anzahl der Bearbeitungsstationen */
extern int STAGES;

/* Anzahl der Maschinen auf den einzelnen Bearbeitungsstationen */
extern int MACHINES_ON_STAGE[MAX_STAGES + 1];

/* Auftragsanzahl */
extern int JOB_NR;

/* Anzahl der Bearbeitungen auf den einzelnen Maschinen. */
extern int OPERATIONS_ON_MACHINES[MAX_STAGES + 1][MAX_MACHINE_NR + 1];

/* Which Operation on which Machine */
extern int WORKLOAD[MAX_STAGES + 1][MAX_JOB_NR + 1];

/* Knoten, die sich auf einem längsten Weg befinden. */
extern int CRITICAL_EDGES[MAX_STAGES * MAX_JOB_NR][2];

/* Anzahl der Kanten, die sich auf einem längsten Weg befinden. */
extern int NR_OF_CRITICAL_EDGES;

/* Gewählte kritische Kante (Anfangsknoten) */
extern struct graph_node* CANDIDATE;

/* Platz der gewählten kritischen Kante in CRITICAL_EDGES [] */
extern int PLACE;

/* Zielwert */
extern long ACT_RESULT;
extern long CANDIDATE_RESULT;
extern long BEST_RESULT;

/* Temperatur */
extern double TEMPERATURE;

/* Vorgeschlagene Verschlechterungen */
extern long BAD_ITER;

/* Akzeptierte Verschlechterungen */
extern long ACCEPT;

/* Vorgegebene Verschlechterungsrate */
extern double ACCEPT_RATE;

/* Iterationsanzahl */
extern long ITERATION;

/* Nachbarschaftsstruktur */
extern int NEIGHBORHOOD;

/* Nachbarschaftsstruktur N5 */
extern int MACHINE_2;
extern int POSITION_2;
extern int MACHINE_3;
extern int POSITION_3;

/******************          Definitionen                *****************/

#define FREE 0
#define NOT_FREE 1

#define YES 0
#define NO 1
#define STOP -1

#define NO_JOB -1
#define OK 1
#define NO_MEMORY 2
#define CYCLE 3

#define SENKE NULL
#define M 32000  // M steht fuer unendlich

#endif
