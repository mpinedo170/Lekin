#include "StdAfx.h"

void borrar_archivos()
{
    _unlink("gantt");
    _unlink("gantt_final");
    _unlink("input.dec");
    _unlink("output");
    _unlink("par_gen");
    _unlink("par_heu");
    _unlink("par_rest");
    _unlink("results");
    _unlink("stats");
    _unlink("vent0");
    _unlink("vent1");
}
