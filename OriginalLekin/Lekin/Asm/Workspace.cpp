#include "StdAfx.h"

#include "AppAsm.h"
#include "Workspace.h"

#include "Job.h"
#include "Workcenter.h"

int GetTotalMchCount()
{
    int s = 0;
    EnumerateWkc s += pWkc->GetMchCount();
    EnumerateEnd return s;
}

int GetTotalMchMax()
{
    int s = 0;
    EnumerateWkc s = max(s, pWkc->GetMchCount());
    EnumerateEnd return s;
}

TWorkspace WktGet()
{
    return TWorkspace(int(TJob::GetShop()) * 2 + (TWorkcenter::IsFlexible() ? 1 : 0));
}

CString WktGetLabel(TWorkspace wkt)
{
    CString s(MAKEINTRESOURCE(IDS_WKT0 + int(wkt)));
    return s;
}

bool WktIsFlexible(TWorkspace wkt)
{
    return (int(wkt) & 1) != 0;
}

TShop WktGetShop(TWorkspace wkt)
{
    return TShop(int(wkt) / 2);
}

bool GotWkc()
{
    return arrWorkcenter.GetSize() > 0;
}
