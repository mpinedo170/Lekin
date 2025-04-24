#include "StdAfx.h"

#include "AppAsm.h"
#include "Dlg3Button.h"

/////////////////////////////////////////////////////////////////////////////
// CDlg3Button dialog

CDlg3Button::CDlg3Button(int idd) : super(idd)
{
    m_bNormal = true;
}

BEGIN_MESSAGE_MAP(CDlg3Button, super)
ON_BN_CLICKED(IDNO, OnNo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlg3Button message handlers

BOOL CDlg3Button::OnInitDialog()
{
    super::OnInitDialog();
    CString s;
    GetDlgItemText(IDC_PROMPT, s);
    if (s == strStar)
    {
        AfxFormatString1(s, IDS_CLEAR_SEQ, m_bNormal ? _T("save") : _T("discard"));
        SetDlgItemText(IDC_PROMPT, s);
    }

    if (m_bNormal)
        GetDlgItem(IDOK)->SetFocus();
    else
    {
        EnableDlgItem(IDOK, false);
        SetDefID(IDNO);
        GetDlgItem(IDNO)->SetFocus();
    }
    return false;
}

void CDlg3Button::OnNo()
{
    EndDialog(IDNO);
}
