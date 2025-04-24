#include "StdAfx.h"

#include "AFLibProTT.h"

#include "AFLibGlobal.h"
#include "Local.h"

using namespace AFLibPrivate;
using AFLibGui::CProTT;

IMPLEMENT_DYNAMIC(CProTT, super)

CProTT::CProTT(UINT nIDTemplate) : super(nIDTemplate)
{
    m_psp.dwFlags &= ~PSP_HASHELP;
}

BEGIN_MESSAGE_MAP(CProTT, super)
END_MESSAGE_MAP()

BOOL CProTT::OnInitDialog()
{
    EnableToolTips();
    return super::OnInitDialog();
}
