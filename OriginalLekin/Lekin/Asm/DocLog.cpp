#include "StdAfx.h"

#include "AppAsm.h"
#include "DocLog.h"

#include "Schedule.h"

IMPLEMENT_DYNCREATE(CDocLog, super)

BEGIN_MESSAGE_MAP(CDocLog, super)
END_MESSAGE_MAP()

CDocLog::CDocLog()
{}

bool CDocLog::IsEmpty()
{
    return pSchActive == NULL;
}

bool CDocLog::IsModified2()
{
    CDocSmart* pDoc = theApp.GetDoc(tvwSeq);
    return pDoc ? pDoc->IsModified2() : false;
}
