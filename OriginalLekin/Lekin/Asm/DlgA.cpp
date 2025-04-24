#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgA.h"

#include "Schedule.h"
#include "Workspace.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgA dialog

CDlgA::CDlgA()
{}

CDlgA::CDlgA(UINT nIDTemplate) : super(nIDTemplate)
{}

BEGIN_MESSAGE_MAP(CDlgA, super)
END_MESSAGE_MAP()

void CDlgA::ModifyLab()
{
    CString s;
    GetWindowText(s);
    s = _T("Add ") + s + _T("s");
    SetWindowText(s);
    GetDlgItemText(IDC_LAB_COUNT, s);
    s += _T(" to add");
    SetDlgItemText(IDC_LAB_COUNT, s);
}

void CDlgA::FillWorkspaces()
{
    for (int i = 0; i < 6; ++i) SetDlgItemText(IDC_WKT0 + i, WktGetLabel(TWorkspace(i)));
}

void CDlgA::FillObjectives()
{
    for (int i = 0; i < objN; ++i) SetDlgItemText(IDC_OBJ0 + i, TSchedule::GetLabel(i));
}

void CDlgA::SetStdLimitText()
{
    int arrControl[] = {IDC_RELEASE, 5, IDC_DUE, 5, IDC_WEIGHT, 5, IDC_PROCESSING, 4, IDC_STATUS, 1, IDC_ID,
        MAX_ID, IDC_JOB_ID, MAX_ID, IDC_WKC_ID, MAX_ID, IDC_MCH_ID, MAX_ID, 0, 0};

    for (int i = 0; arrControl[i] != 0; i += 2)
    {
        CEdit* pWndEdit = (CEdit*)GetDlgItem(arrControl[i]);
        if (pWndEdit == NULL) continue;
        pWndEdit->SetLimitText(arrControl[i + 1]);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgA message handlers

BOOL CDlgA::OnInitDialog()
{
    super::OnInitDialog();
    EnableToolTips(false);
    return true;
}
