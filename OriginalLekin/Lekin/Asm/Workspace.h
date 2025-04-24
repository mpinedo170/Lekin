#pragma once

enum TWorkspace
{
    wktSingle,
    wktParallel,
    wktFlow,
    wktFlexFlow,
    wktJob,
    wktFlexJob,
    wktLoad,
    wktExit
};

bool GotWkc();
int GetTotalMchCount();
int GetTotalMchMax();
TWorkspace WktGet();
CString WktGetLabel(TWorkspace wkt);
bool WktIsFlexible(TWorkspace wkt);
TShop WktGetShop(TWorkspace wkt);
