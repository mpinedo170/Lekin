#pragma once

/************************************************************************/
/*                                                                      */
/*           Implementierung des Metropolis-Algorithmusses              */
/*                                                                      */
/************************************************************************/

// Optimierung mittels Metropolis-Algorithmus
void metropolis();

// Wird neue Konfiguration akzeptiert?
bool AcceptNewConfiguration();

// öberprÅfe, ob Kandidat zulÑssig ist.
bool CheckNeighborhood();

// Aktualisiere Zielwert bei einer Akzeptanz
void AcceptCandidate();

// Lokale Suche vorbereiten.
void PrepareLocalSearch();

// Lokale Suche
void LocalSearch();

// Eigenes Verfahren
void psg();

void psg_tabu();
void tabu_search_psg();
