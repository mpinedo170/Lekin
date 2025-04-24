#include "StdAfx.h"

#include "DlgListHelper.h"

#include "AFLibGuiGlobal.h"
#include "Local.h"

using AFLibPrivate::CDlgListHelper;
using namespace AFLibGui;

CDlgListHelper::CDlgListHelper(CWnd* pWndParent) : m_pWndParent(pWndParent)
{
    m_index = 0;
}

void CDlgListHelper::Create(const CRect& rect)
{
    m_rect = rect;
    super::Create(IDD_LIST_HELPER, m_pWndParent);
}

BEGIN_MESSAGE_MAP(CDlgListHelper, super)
ON_WM_ACTIVATE()
END_MESSAGE_MAP()

BOOL CDlgListHelper::OnInitDialog()
{
    super::OnInitDialog();
    m_wndMain.SubclassDlgItem(IDC_EDIT_LIST, this);
    m_wndMain.SetWindowText(m_data);

    CFont* pFont = m_pWndParent->GetFont();
    SetFont(pFont);
    m_wndMain.SetFont(pFont);

    CRect rectWin;
    CRect rectCli;

    GetWindowRect(rectWin);
    GetClientRect(rectCli);

    CSize szOutBorder((rectWin.Width() - rectCli.Width()) / 2, (rectWin.Height() - rectCli.Height()) / 2);

    m_wndMain.GetWindowRect(rectWin);
    m_wndMain.GetClientRect(rectCli);

    CSize szInBorder((rectWin.Width() - rectCli.Width()) / 2, (rectWin.Height() - rectCli.Height()) / 2);

    rectWin = m_rect;
    rectWin.InflateRect(szInBorder + szOutBorder);
    MoveWindow(rectWin);

    rectWin = m_rect;
    rectWin.InflateRect(szInBorder);
    rectWin.MoveToXY(0, 0);
    AndrewMoveWindow(&m_wndMain, rectWin);
    return false;
}

// destroy window on enter, cancel, or lost focus
void CDlgListHelper::OnOK()
{
    m_wndMain.GetWindowText(m_data);
    m_pWndParent->SendMessage(WM_TIMER, timerCommitEdit);
    DestroyWindow();
}

void CDlgListHelper::OnCancel()
{
    DestroyWindow();
}

void CDlgListHelper::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    super::OnActivate(nState, pWndOther, bMinimized);
    m_wndMain.GetWindowText(m_data);
    m_pWndParent->SendMessage(WM_TIMER, timerCommitEdit);
    if (nState == WA_INACTIVE) PostMessage(WM_CLOSE);
}
