#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgPrintGantt.h"

#include "MarginGantt.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgPrintGantt dialog

CDlgPrintGantt::CDlgPrintGantt(bool bSetupOnly, CWnd* pParent) : super(bSetupOnly, pParent)
{
    m_psd.lpPageSetupTemplateName =
        LPWSTR(MAKEINTRESOURCE(bSetupOnly ? IDD_PAGE_SETUP_GANTT : IDD_PRINT_GANTT));
    m_psd.hInstance = theApp.m_hInstance;
    m_bCatch = false;
}

BEGIN_MESSAGE_MAP(CDlgPrintGantt, super)
ON_EN_CHANGE(IDC_PAGES_X, OnChange2)
ON_EN_CHANGE(IDC_PAGES_Y, OnChange2)
ON_EN_CHANGE(IDC_NO_SPLIT, OnChange2)
END_MESSAGE_MAP()

TMarginGantt* CDlgPrintGantt::GetMar()
{
    return (TMarginGantt*)m_pMarginCur;
}

void CDlgPrintGantt::EnableNoSplit()
{
    CString s;
    GetDlgItemText(IDC_PAGES_Y, s);
    GetDlgItem(IDC_NO_SPLIT)->EnableWindow(s != _T("1"));
}

CString CDlgPrintGantt::GetToolTip(UINT idc)
{
    switch (idc)
    {
        case IDC_PAGES_X:
            return _T("Pages per row");
        case IDC_PAGES_Y:
            return _T("Pages per column");
        case IDC_NO_SPLIT:
            return _T("Always print a workstation on a single page");
    }

    return super::GetToolTip(idc);
}

UINT CDlgPrintGantt::SaveAdditionalData()
{
    int a = GetDlgItemInt(IDC_PAGES_X);
    if (a < 1 || a > 10) return IDC_PAGES_X;
    GetMar()->m_nX = a;

    a = GetDlgItemInt(IDC_PAGES_Y);
    if (a < 1 || a > 10) return IDC_PAGES_Y;
    GetMar()->m_nY = a;

    GetMar()->m_bNoSplit = IsDlgButtonChecked(IDC_NO_SPLIT) != 0;
    return super::SaveAdditionalData();
}

void CDlgPrintGantt::LoadAdditionalData()
{
    SetDlgItemInt(IDC_PAGES_X, GetMar()->m_nX);
    SetDlgItemInt(IDC_PAGES_Y, GetMar()->m_nY);
    CheckDlgButton(IDC_NO_SPLIT, GetMar()->m_bNoSplit);
    super::LoadAdditionalData();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgPrintGantt message handlers

void CDlgPrintGantt::OnChange2()
{
    if (!m_bCatch) return;
    EnableNoSplit();
    RedrawPage(true);
}

BOOL CDlgPrintGantt::OnInitDialog()
{
    super::OnInitDialog();

    m_spinPagesX.SubclassDlgItem(IDC_SPIN_PAGES_X, this);
    m_spinPagesY.SubclassDlgItem(IDC_SPIN_PAGES_Y, this);

    m_spinPagesX.SetRange32(1, 10);
    m_spinPagesY.SetRange32(1, 10);

    m_bCatch = true;
    EnableNoSplit();
    return true;
}
