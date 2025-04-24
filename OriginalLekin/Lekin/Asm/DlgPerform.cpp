#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgPerform.h"

#include "Schedule.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPerform dialog

CDlgPerform::CDlgPerform()
{
    Create(CDlgPerform::IDD, NULL);
    FillObjectives();
    SetIcon(theApp.LoadIcon(IDI_PERFORM), true);
}

void CDlgPerform::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    if (pDX->m_bSaveAndValidate || !pSchActive) return;

    for (int i = 0; i < objN; ++i) DDX_Text(pDX, IDC_EOBJ0 + i, pSchActive->m_arrObj[i]);
    SetDlgItemText(IDC_ID, pSchActive->m_id);
}

BEGIN_MESSAGE_MAP(CDlgPerform, super)
END_MESSAGE_MAP()

void CDlgPerform::FillData()
{
    UpdateData(false);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPerform message handlers

void CDlgPerform::OnCancel()
{
    theApp.CloseDlgPerform();
}
