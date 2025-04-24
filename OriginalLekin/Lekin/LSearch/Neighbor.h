#pragma once

/************************************************************************/
/*                                                                      */
/*     Prozeduren zur Bestimmung der Nachbarschaft eines Zustandes.     */
/*                                                                      */
/************************************************************************/

// Bestimme die kritischen Kanten.
void GetCriticalEdges();

// Bestimme die kritischen Kanten des Auftrages, der die meisten
// Kosten verursacht.
void GetCriticalEdgesFromHighestImpactJob();

// Bestimme Zielwert von CANDIDATE
bool GetCandidateResult();

// WÑhle zufÑllig eine kritische Kante
void SelectCandidateFromCriticalEdges();

// Nachbarschaftsstruktur N1
void neighborhood_n1();

// Nachbarschaftsstruktur N2
void neighborhood_n2();

// Nachbarschaftsstruktur N4
void neighborhood_n4();
int get_left_bound();
int get_right_bound();
int get_correct_left_bound(int);
int get_correct_right_bound(int);
int get_direction();
void change_jobs_n4();
void restore_n4();

// Nachbarschaftsstruktur N5
void neighborhood_n5();
int step1();
int step2();
int step3();
int step4();
int step5();
void restore_n5();

// énderung im Graphen wieder rÅckgaÑgig machen.
void TurnBackChangedEdge();

// Besten Ablaufplan speichern
void SaveConfiguration();

// Besten Ablaufplan als aktuell betrachten.
void ReadBestPlan();

void show_machine_order(int);
void show_critical_edges();
void show_position(int);
