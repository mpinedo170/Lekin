/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                         M E T R O P O L . H                          */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#ifndef METROPOL_H
#define METROPOL_H

/************************************************************************/
/*                                                                      */
/*           Implementierung des Metropolis-Algorithmusses              */
/*                                                                      */
/************************************************************************/

/* Optimierung mittels Metropolis-Algorithmus */
void metropolis();

/* Wird neue Konfiguration akzeptiert? */
int accept_new_configuration();

/* Aktualisiere Zielwert bei einer Akzeptanz */
void compare_act_result_with_best_result();

/* Lokale Suche */
void local_search();

/* Eigenes Verfahren */
void lsrw();

#endif
