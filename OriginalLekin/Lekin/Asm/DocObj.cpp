#include "StdAfx.h"

#include "AppAsm.h"
#include "DocObj.h"

#include "Schedule.h"

IMPLEMENT_DYNCREATE(CDocObj, super)

BEGIN_MESSAGE_MAP(CDocObj, super)
END_MESSAGE_MAP()

CDocObj::CDocObj()
{}

bool CDocObj::IsEmpty()
{
    return pSchActive == NULL;
}
