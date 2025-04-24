#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgLoadSetup.h"

#include "Misc.h"
#include "Workcenter.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadSetup dialog

CDlgLoadSetup::CDlgLoadSetup() : super(CDlgLoadSetup::IDD)
{}

void CDlgLoadSetup::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;
    if (m_setup.GetDim() == 0)
    {
        LekinBeep();
        pDX->Fail();
    }
}

BEGIN_MESSAGE_MAP(CDlgLoadSetup, super)
ON_LBN_DBLCLK(IDC_WKC_ID, OnDblclkWkcId)
ON_LBN_SELCHANGE(IDC_WKC_ID, OnSelchangeWkcId)
END_MESSAGE_MAP()

TWorkcenter* CDlgLoadSetup::GetWkc(int i)
{
    return i < 0 ? NULL : (TWorkcenter*)m_lstWkc.GetItemDataPtr(i);
}

void CDlgLoadSetup::SaveSelection()
{
    m_setup.SetDim(0);
    TWorkcenter* pWkc = GetWkc(m_lstWkc.GetCurSel());
    if (!pWkc) return;
    m_setup = pWkc->m_setup;
}

void CDlgLoadSetup::LoadSelection()
{
    int i = m_lstWkc.GetCount();
    while (--i >= 0)
        if (GetWkc(i)->m_setup == m_setup) break;

    m_lstWkc.SetCurSel(i);
    EnableDlgItem(IDOK, i >= 0);
}

void CDlgLoadSetup::FillListBox()
{
    m_lstWkc.ResetContent();
    EnumerateWkc if (pWkc->m_setup.GetDim() < 1) continue;
    if (m_bNoDup)
        for (int i = m_lstWkc.GetCount(); --i >= 0;)
            if (GetWkc(i)->m_setup == pWkc->m_setup) goto NEXT;
    m_lstWkc.AddData(pWkc);
NEXT:;
    EnumerateEnd LoadSelection();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadSetup message handlers

BOOL CDlgLoadSetup::OnInitDialog()
{
    m_lstWkc.SubclassDlgItem(IDC_WKC_ID, this);
    super::OnInitDialog();

    if (m_lstWkc.GetCount() < 1)
    {
        AfxMB(IDP_NO_SETUP);
        EndDialog(IDCANCEL);
    }
    m_lstWkc.SetFocus();
    return false;
}

void CDlgLoadSetup::OnDblclkWkcId()
{
    OnOK();
}

void CDlgLoadSetup::OnSelchangeWkcId()
{
    EnableDlgItem(IDOK, true);
}
