#include "StdAfx.h"

#include "AppAsm.h"
#include "DocGantt.h"

#include "Schedule.h"

IMPLEMENT_DYNCREATE(CDocGantt, super)

BEGIN_MESSAGE_MAP(CDocGantt, super)
END_MESSAGE_MAP()

CDocGantt::CDocGantt()
{}

CString CDocGantt::GetTitle2()
{
    return pSchActive != NULL ? pSchActive->m_id : strEmpty;
}

bool CDocGantt::IsEmpty()
{
    return pSchActive == NULL;
}
