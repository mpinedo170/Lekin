/************************************************************************/
/*                                                                      */
/*  Definition aller globaler Variablen (aus öbersichtlichkeitsgrÅnden) */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "Def.h"

// Array, in dem die AuftrÑge stehen.
CSmartArray<TJob> JOBS;

// Array, in dem Informationen Åber die Maschinen stehen.
CSmartArray<TMachine> MACHINES;

// Letzter Bearbeitungsknoten des Auftrages, der die groessten Kosten
// verursacht.
TGraphNode* LAST_NODE;

// Knoten, die sich auf einem lÑngsten Weg befinden.
CSmartArray<TGraphNode> CRITICAL_EDGES;

// GewÑhlte kritische Kante (Anfangsknoten)
TGraphNode* CANDIDATE;

// Platz der gewÑhlten kritischen Kante in CRITICAL_EDGES[]
int PLACE;

// Zielwert
int ACT_RESULT;
int CANDIDATE_RESULT;
int BEST_RESULT;

// Temperatur
double TEMPERATURE;

// Nachbarschaftsstruktur
int NEIGHBORHOOD;

// Vorgeschlagene Verschlechterungen
int BAD_ITER;

// Akzeptierte Verschlechterungen
int ACCEPT;

// Vorgegebene Verschlechterungsrate
double ACCEPT_RATE;

// Iterationsanzahl
int ITERATION;

// Zeitvorgabe fÅr statisches Problem
int STAT_TIME;

// Nachbarschaftsstruktur N5
TGraphNode* NODE_2;
TGraphNode* NODE_3;

// Welches Lîsungsverfahren
TAlgorithm ALGORITHM;

// Which objective function
TObjective OBJECTIVE_FUNCTION;
