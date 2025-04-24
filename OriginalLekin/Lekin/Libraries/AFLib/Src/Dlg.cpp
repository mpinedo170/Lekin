#include "StdAfx.h"

#include "AFLibDlg.h"

#include "AFLibDataUnit.h"
#include "Local.h"

using namespace AFLibGui;

IMPLEMENT_DYNAMIC(CDlg, super)

CDlg::CDlg()
{
    m_pDlgSD = NULL;
}

CDlg::CDlg(UINT nIDTemplate, CWnd* pWndParent) : super(nIDTemplate, pWndParent)
{
    m_pDlgSD = NULL;
}

void CDlg::AddCheck(UINT idcCheck, UINT idcEdit, UINT idcOther, int count)
{
    TTranslationUnit tu = {idcCheck, idcEdit, idcOther, count};
    m_arrCheck.Add(tu);
}

void CDlg::ProcessCheck(UINT idcCheck, bool bActivate)
{
    for (int i = m_arrCheck.GetSize(); --i >= 0;)
    {
        TTranslationUnit& tu = m_arrCheck[i];
        if (tu.ContainsIdcFrom(idcCheck))
        {
            PrivateProcessCheck(tu, bActivate ? idcCheck : 0);
            break;
        }
    }
}

void CDlg::ProcessAllChecks()
{
    for (int i = 0; i < m_arrCheck.GetSize(); ++i) PrivateProcessCheck(m_arrCheck[i], 0);
}

void CDlg::PrivateProcessCheck(const TTranslationUnit& tu, UINT idc)
{  // for all check/radio buttons in the set
    for (int i = 0; i < tu.m_count; ++i)
    {
        int idcCheck = tu.m_idcFrom + i;
        bool bEnable = IsDlgItemEC(idcCheck);

        // get corresponding "other" box window, and enable/disable it
        if (tu.m_idcTo2 != 0)
        {
            CWnd* pWndOther = GetDlgItem(tu.m_idcTo2 + i);
            if (pWndOther != NULL) pWndOther->EnableWindow(bEnable);
        }

        // get corresponding edit box window
        CWnd* pWndEdit = GetDlgItem(tu.m_idcTo + i);
        if (pWndEdit == NULL) continue;

        // enable or disable edit box
        pWndEdit->EnableWindow(bEnable);
        if (idc != idcCheck) continue;

        // if this check box was just clicked, set input focus to the corresponding edit box
        pWndEdit->SetFocus();
        pWndEdit->SendMessage(EM_SETSEL, 0, -1);
    }
}

void CDlg::AddSetButton(UINT idcButton, UINT idcDest, int count)
{
    const TTranslationUnit tu = {idcButton, idcDest, 0, count};
    m_arrSetButton.Add(tu);
}

void CDlg::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDXData(pDX, 0, GetDUCount() - 1, pDX->m_bSaveAndValidate != 0);
}

BOOL CALLBACK CDlg::AdjustMarginProc(HWND hWnd, LPARAM lParam)
{  // if the child window is an edit box, set its left and right margins to 1 pixel
    ::SendMessage(hWnd, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(1, 1));
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Data Manipulation

bool CDlg::IsDUNeeded(const TDataUnit* pDu)
{
    const UINT idc = pDu->m_idc;
    TTranslationUnit tu;

    // check if this data unit has a corresponding check box
    int i = m_arrCheck.GetSize();
    while (--i >= 0)
    {
        tu = m_arrCheck[i];
        if (tu.ContainsIdcTo(idc)) break;
    }
    if (i < 0) return true;

    // found a check/edit unit.  Does the check belong to a data unit?
    int index = idc - tu.m_idcTo;
    const TDataUnit* pDu2 = FindDU(tu.m_idcFrom + index);
    return pDu2 == NULL ? true : pDu2->PeerNeeded(index);
}

BOOL CDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{  // redefined to process check sets without interfering with message handling chain
    if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED) ProcessCheck(LOWORD(wParam), true);
    return super::OnWndMsg(message, wParam, lParam, pResult);
}

CString CDlg::PrivateGetToolTip(UINT idc, bool bTranslate)
{
    const TDataUnit* pDu = FindDU(idc);

    if (pDu != NULL && pDu->m_idsHelp != 0) return AFLib::AfxString(pDu->m_idsHelp);

    if (bTranslate)
    {
        for (int i = m_arrSetButton.GetSize(); --i >= 0;)
        {
            const TTranslationUnit& tu = m_arrSetButton[i];
            if (!tu.ContainsIdcFrom(idc)) continue;

            int index = idc - tu.m_idcFrom;
            CString s = PrivateGetToolTip(tu.m_idcTo + index, false);
            if (!s.IsEmpty()) return AFLibPrivate::LocalAfxString(IDS_SET, s);
            break;
        }
    }

    return super::GetToolTip(idc);
}

CString CDlg::GetToolTip(UINT idc)
{
    return PrivateGetToolTip(idc, true);
}

BEGIN_MESSAGE_MAP(CDlg, super)
END_MESSAGE_MAP()

BOOL CDlg::OnInitDialog()
{
    super::OnInitDialog();

    // adjust all edit box controls
    EnumChildWindows(m_hWnd, AdjustMarginProc, 0);
    ProcessAllChecks();
    return true;
}

void CDlg::OnOK()
{
    if (!UpdateData()) return;
    SaveData();
    ValidateData();
    EndDialog(IDOK);
}
