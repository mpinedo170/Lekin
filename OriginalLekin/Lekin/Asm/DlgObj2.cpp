#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgObj2.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgObj2 dialog

CDlgObj2::CDlgObj2() : CDlgA(CDlgObj2::IDD)
{
    AddDU(new TMskDataUnit(strEmpty, IDC_OBJ0, m_Select, -1, 0, objN, IDP_NO_OBJ));
}

BEGIN_MESSAGE_MAP(CDlgObj2, CDlgA)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgObj2 message handlers

BOOL CDlgObj2::OnInitDialog()
{
    CDlgA::OnInitDialog();
    FillObjectives();
    return true;
}
