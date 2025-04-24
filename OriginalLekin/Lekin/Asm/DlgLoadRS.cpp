#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgLoadRS.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadRS dialog

CDlgLoadRS::CDlgLoadRS(UINT nIDTemplate) : super(nIDTemplate)
{
    m_section = secView;
    AddDU(new TChkDataUnit(_T("NoDup"), IDC_NO_DUP, m_bNoDup, false, false));
}

void CDlgLoadRS::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    if (!pDX->m_bSaveAndValidate) return;
    SaveSelection();
}

BEGIN_MESSAGE_MAP(CDlgLoadRS, super)
ON_BN_CLICKED(IDC_NO_DUP, OnNoDup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLoadRS message handlers

BOOL CDlgLoadRS::OnInitDialog()
{
    super::OnInitDialog();
    FillListBox();
    return true;
}

void CDlgLoadRS::OnNoDup()
{
    if (!UpdateData(true)) return;
    SaveData();
    SaveSelection();
    FillListBox();
}
