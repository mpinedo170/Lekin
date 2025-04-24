/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                 D E F . C P P                        */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  Definition aller globaler Variablen (aus Übersichtlichkeitsgründen) */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "def.h"

// long RAND_MAX;

/* Array, in dem die Aufträge stehen. */
struct job JOBS[MAX_JOB_NR + 1];

/* Array, in dem Informationen über die Maschinen stehen. */
struct machine MACHINES[MAX_STAGES + 1][MAX_MACHINE_NR + 1];

/* Array, in dem der erweiterte disjunktive Graph steht.  */
struct graph_node* GRAPH[MAX_STAGES + 1][MAX_JOB_NR];

/* Speichere die Weglängen ab. */
int SAVE_LP[MAX_STAGES * MAX_JOB_NR];

/* Letzter Bearbeitungsknoten des Auftrages, der die groessten Kosten */
/* verursacht.                                                        */
struct graph_node* LAST_NODE;
struct graph_node* NODE1;
struct graph_node* NODE2;

/* Anzahl der Bearbeitungsstationen */
int STAGES;

/* Anzahl der Maschinen auf den einzelnen Bearbeitungsstationen */
int MACHINES_ON_STAGE[MAX_STAGES + 1];

/* Auftragsanzahl */
int JOB_NR;

/* Anzahl der Bearbeitungen auf den einzelnen Maschinen. */
int OPERATIONS_ON_MACHINES[MAX_STAGES + 1][MAX_MACHINE_NR + 1];

/* Which Operation on which Machine */
int WORKLOAD[MAX_STAGES + 1][MAX_JOB_NR + 1];

/* Knoten, die sich auf einem längsten Weg befinden. */
int CRITICAL_EDGES[MAX_STAGES * MAX_JOB_NR][2];

/* Anzahl der Kanten, die sich auf einem längsten Weg befinden. */
int NR_OF_CRITICAL_EDGES;

/* Gewählte kritische Kante (Anfangsknoten) */
struct graph_node* CANDIDATE;

/* Platz der gewählten kritischen Kante in CRITICAL_EDGES [] */
int PLACE;

/* Zielwert */
long ACT_RESULT;
long CANDIDATE_RESULT;
long BEST_RESULT;

/* Temperatur */
double TEMPERATURE;

/* Nachbarschaftsstruktur */
int NEIGHBORHOOD;

/* Vorgeschlagene Verschlechterungen */
long BAD_ITER;

/* Akzeptierte Verschlechterungen */
long ACCEPT;

/* Vorgegebene Verschlechterungsrate */
double ACCEPT_RATE;

/* Iterationsanzahl */
long ITERATION;

/* Nachbarschaftsstruktur N5 */
int MACHINE_2;
int POSITION_2;
int MACHINE_3;
int POSITION_3;
