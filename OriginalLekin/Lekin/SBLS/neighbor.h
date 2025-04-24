/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                           N E I G H B O R . H                        */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#ifndef NEIGHBOR_H
#define NEIGHBOR_H

/************************************************************************/
/*                                                                      */
/*     Prozeduren zur Bestimmung der Nachbarschaft eines Zustandes.     */
/*                                                                      */
/************************************************************************/

/* Bestimme die kritischen Kanten. */
void get_critical_edges();

/* Bestimme die kritischen Kanten des Auftrages, der die meisten */
/* Kosten verursacht.                                            */
void get_critical_edges_from_highest_impact_job();

/* Bestimme Zielwert von CANDIDATE */
int get_candidate_result();

/* Wähle zufällig eine kritische Kante */
void select_candidate_from_critical_edges();

/* Nachbarschaftsstruktur N1 */
void neighborhood_n1();

/* Nachbarschaftsstruktur N5 */
void neighborhood_n5();
int step1();
int step2();
int step3();
int step4();
int step5();
void restore_n5();

/* Änderung im Graphen wieder rückgaägig machen. */
void turn_back_changed_edge();

/* Besten Ablaufplan speichern */
void save_configuration();

/* Besten Ablaufplan als aktuell betrachten. */
void read_in_best_plan();

void show_machine_order(int);
void show_critical_edges();
void show_position(int);

#endif
