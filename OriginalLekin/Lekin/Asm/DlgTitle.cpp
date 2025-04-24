#include "StdAfx.h"

#include "AppAsm.h"
#include "DlgTitle.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgTitle dialog

CDlgTitle::CDlgTitle() : super(IDD)
{
    m_bBlinking = false;
}

void CDlgTitle::DoDataExchange(CDataExchange* pDX)
{
    super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STERN, m_wndStern);
    DDX_Control(pDX, IDC_BLINK, m_wndBlink);
}

BEGIN_MESSAGE_MAP(CDlgTitle, super)
ON_WM_TIMER()
ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTitle message handlers

BOOL CDlgTitle::OnInitDialog()
{
    super::OnInitDialog();

    m_imgStern.LoadBitmap(IDB_STERN);
    m_wndStern.SetBitmap(m_imgStern);

    CRect rectStern;
    m_wndStern.GetWindowRect(rectStern);
    ScreenToClient(rectStern);

    CRect rect;
    m_wndStern.GetClientRect(rect);

    CSize szImg = m_imgStern.GetSize();
    CSize szBrd = rectStern.Size() - rectStern.Size();

    rectStern.SetRect(rectStern.TopLeft(), rectStern.TopLeft() + szImg + szBrd);
    m_wndStern.MoveWindow(rectStern, false);

    CRect rectDlg;
    GetWindowRect(rectDlg);
    GetClientRect(rect);
    szBrd = rectDlg.Size() - rect.Size();

    CSize szMar = CSize(rectStern.TopLeft());
    rectDlg.SetRect(rectDlg.TopLeft(), rectDlg.TopLeft() + rectStern.Size() + szMar + szMar + szBrd);

    if (ColorsOk(this))
        m_wndBlink.ShowWindow(SW_HIDE);
    else
    {
        CRect rectBlink;
        m_wndBlink.GetWindowRect(rectBlink);
        ScreenToClient(rectBlink);

        rectBlink.MoveToY(rectStern.bottom + szMar.cy);
        rectBlink.right = rectStern.right;
        m_wndBlink.MoveWindow(rectBlink);

        rectDlg.bottom += rectBlink.Height() + szMar.cy;
        m_bBlinking = true;
    }

    MoveWindow(rectDlg, false);
    SetTimer(timerUpdate, 500, NULL);

#ifdef _DEBUG
    m_ticks = 4;
#else
    m_ticks = 10;
#endif

    return true;
}

void CDlgTitle::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == timerUpdate)
    {
        if (m_bBlinking) m_wndBlink.EnableWindow(!m_wndBlink.IsWindowEnabled());
        if (--m_ticks <= 0) OnCancel();
    }
}

void CDlgTitle::OnCancel()
{
    m_ticks = 0;
    EndDialog(IDOK);
}

LRESULT CDlgTitle::OnCommandHelp(WPARAM, LPARAM)
{
    return 1;
}
