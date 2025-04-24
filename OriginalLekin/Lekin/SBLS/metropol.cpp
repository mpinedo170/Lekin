/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                         M E T R O P O L . C P P                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/*           Implementierung des Metropolis-Algorithmuss                */
/*                                                                      */
/************************************************************************/

#include "StdAfx.h"

#include "def.h"
#include "graph.h"
#include "metropol.h"
#include "neighbor.h"

extern long COUNT2;
extern long COUNT3;

extern double Random();

/************************************************************************/
/*                                                                      */
/* Aufgabe: Akzeptiere neuen Zustand und speichere Bestwerte ab.        */
/*                                                                      */
/************************************************************************/

void compare_act_result_with_best_result()
{
    save_lp();
    ACT_RESULT = CANDIDATE_RESULT;
    /* Neuer bester Zielwert */
    if (ACT_RESULT < BEST_RESULT)
    {
        BEST_RESULT = ACT_RESULT;
        save_actual_schedule();
        // printf("\nLarge Step: %lu", BEST_RESULT);
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Optimierung mittels Metropolis-Algorithmus                  */
/*                                                                      */
/************************************************************************/

void metropolis()
{
    int ok, change, abbruch = NO;
    double multi;

    ITERATION = ACCEPT = BAD_ITER = 0;
    save_lp();

    while (abbruch == NO)
    {
        ITERATION++;

        /* Wähle zufälligen Nachbarszustand */
        select_candidate_from_critical_edges();
        ok = get_candidate_result();

        if (ok == CYCLE)
        {
            printf("\nZyklus!!!!!!\n");
            exit(0);
        }
        else
            /* Wird neue Konfiguration akzeptiert? */
            change = accept_new_configuration();

        /* Nein: Aenderung zuruecknehmen */
        if (change == NO) turn_back_changed_edge();
        /* Ja:   Aenderung akzeptieren */
        else
        {
            compare_act_result_with_best_result();
            /* Neuer Graph liegt vor -> bestimme kritische Kanten */
            get_critical_edges_from_highest_impact_job();
        }

        if ((ITERATION / 100) * 100 == ITERATION)
        {
            /* Temperatur an gewünschte Akzeptanzrate angleichen. */
            if ((ACCEPT == 0) || (BAD_ITER == 0))
                multi = 1.0;
            else
                multi = ACCEPT_RATE / (double(ACCEPT) / BAD_ITER);
            TEMPERATURE = TEMPERATURE * multi;
            ACCEPT = BAD_ITER = 0;
            if ((ITERATION / 1000) * 1000 == ITERATION)
                printf("\nIteration %lu  Wert %lu  Temperatur %f", ITERATION, ACT_RESULT, TEMPERATURE);
        }
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Neue Loesung akzeptieren: ja oder nein                      */
/*                                                                      */
/************************************************************************/

int accept_new_configuration()
{
    double prop, u;

    /* Wahrscheinlichkeit einen schlechteren Zustand zu akzeptieren */
    if ((CANDIDATE_RESULT - ACT_RESULT) > 0)
    {
        BAD_ITER++;
        prop = exp((ACT_RESULT - CANDIDATE_RESULT) / TEMPERATURE);
        u = Random();
        //  u = rand()/RAND_MAX;
        if (u < prop)
        {
            ACCEPT++;
            return (YES);
        }
        else
            return (NO);
    }
    /* Keine Verschlechterung -> immer akzeptieren */
    else
        return (YES);
}

/************************************************************************/
/*                                                                      */
/*                Implementierung von lokaler Suche                     */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* Aufgabe: Optimierung mittels Lokaler Suche                           */
/*                                                                      */
/************************************************************************/

void local_search()
{
    int abbruch = NO, ok, nr;
    int j_nr, stage_nr;

    if (NR_OF_CRITICAL_EDGES == 0) abbruch = YES;

    while (abbruch == NO)
    {
        nr = 0;
        /* Wähle zufälligen Nachbarszustand */
        select_candidate_from_critical_edges();

        /* Suche einen Nachbarn, der zu einer Verbesserung führt. */
        while (nr <= NR_OF_CRITICAL_EDGES)
        {
            nr++;
            ok = get_candidate_result();

            if (ok == CYCLE)
            {
                printf("\nIteration %d  -> Zyklus bei Local Search!!", ITERATION);
                exit(0);
            }

            /* Wird neue Konfiguration akzeptiert? */
            /* Nein */
            if ((CANDIDATE_RESULT - ACT_RESULT) >= 0)
            {
                /* Änderung rückgängig machen */
                turn_back_changed_edge();
                /* Nächsten Kandidaten bestimmen */
                /* Gehe dazu CRITICAL_EDGES um eins weiter. */
                if (PLACE == (NR_OF_CRITICAL_EDGES - 1))
                    PLACE = 0;
                else
                    PLACE++;

                stage_nr = CRITICAL_EDGES[PLACE][0];
                j_nr = CRITICAL_EDGES[PLACE][1];

                CANDIDATE = GRAPH[stage_nr][j_nr];
            }
            /* Ja */
            else
            {
                save_lp();
                ACT_RESULT = CANDIDATE_RESULT;

                /* Neuer bester Zielwert */
                if (ACT_RESULT < BEST_RESULT)
                {
                    BEST_RESULT = ACT_RESULT;
                    save_actual_schedule();
                    // printf("\nSmall Step: %lu", BEST_RESULT);
                }
                /* Neuer Graph liegt vor -> bestimme kritische Kanten */
                get_critical_edges();
                if (NR_OF_CRITICAL_EDGES == 0) return;  // newly added line
                nr = NR_OF_CRITICAL_EDGES + 1;
            }

            if ((nr == NR_OF_CRITICAL_EDGES) || (NR_OF_CRITICAL_EDGES == 0)) abbruch = YES;
        }
    }
}

/************************************************************************/
/*                                                                      */
/* Aufgabe: Large Step Random Walk                                      */
/*                                                                      */
/************************************************************************/

void lsrw()
{
    int ok, change, iter, min_iter, max_iter, i;
    long act_best, metro_iter;

    // act_best = M;
    // BEST_RESULT = M;

    act_best = BEST_RESULT;
    save_lp();

    for (i = 1; i <= ITERATION; i++)
    {
        local_search();
        if (NR_OF_CRITICAL_EDGES == 0) return;  // newly added line

        if (ACT_RESULT < act_best) act_best = ACT_RESULT;

        if (ACT_RESULT > (act_best * 1.1))
        {
            min_iter = 10;
            max_iter = 100;
        }
        if ((ACT_RESULT <= (act_best * 1.1)) || (ACT_RESULT <= (act_best + 20)))
        {
            min_iter = 10;
            max_iter = 40;
        }
        if (ACT_RESULT == act_best)
        {
            min_iter = 5;
            max_iter = 20;
        }
        metro_iter = (long)(max_iter * Random()) + min_iter;
        // metro_iter = (long) (max_iter * rand()/RAND_MAX) + min_iter;

        iter = 0;
        while (iter < metro_iter)
        {
            /* Wähle zufälligen Nachbarszustand */
            select_candidate_from_critical_edges();
            ok = get_candidate_result();

            if (ok == CYCLE)
            {
                printf("\nCycle  -  LSRW\n");
                exit(0);
            }

            change = accept_new_configuration();

            /* Nein: Aenderung zuruecknehmen */
            if (change == NO) turn_back_changed_edge();
            /* Ja:   Aenderung akzeptieren */
            else
            {
                compare_act_result_with_best_result();
                /* Neuer Graph liegt vor -> bestimme kritische Kanten */
                get_critical_edges_from_highest_impact_job();
                if (NR_OF_CRITICAL_EDGES == 0) return;  // newly added line
            }

            iter++;
        }
    }
}
