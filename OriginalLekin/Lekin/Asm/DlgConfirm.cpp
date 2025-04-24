#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgConfirm.h"

#include "Misc.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfirm dialog

CDlgConfirm::CDlgConfirm() : super(CDlgConfirm::IDD)
{
    m_bConfirm = false;
}

void CDlgConfirm::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_PROMPT, m_prompt);
    DDX_Check(pDX, IDC_CONFIRM0, m_bConfirm);
}

BEGIN_MESSAGE_MAP(CDlgConfirm, super)
ON_BN_CLICKED(IDNO, OnNo)
ON_BN_CLICKED(IDYES, OnYes)
ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfirm message handlers

void CDlgConfirm::OnNo()
{
    UpdateData(true);
    EndDialog(IDNO);
}

void CDlgConfirm::OnYes()
{
    UpdateData(true);
    EndDialog(IDYES);
}

void CDlgConfirm::OnClose()
{
    LekinBeep();
}

BOOL CDlgConfirm::OnInitDialog()
{
    super::OnInitDialog();
    if (m_level == 0) EnableDlgItem(IDC_CONFIRM0, false);

    static LPCTSTR arrIDI[] = {IDI_ERROR, IDI_WARNING, IDI_INFORMATION};
    m_hIcon = LoadIcon(NULL, arrIDI[m_level]);
    ((CStatic*)GetDlgItem(IDC_LAB0))->SetIcon(m_hIcon);
    return true;
}
