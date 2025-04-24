#include "Stdafx.h"

#include "AppAsm.h"
#include "FrmSmart.h"

#include "DocSmart.h"
#include "DocTmpSmart.h"

/////////////////////////////////////////////////////////////////////////////
// CFrmSmart

IMPLEMENT_DYNCREATE(CFrmSmart, CMDIChildWnd)

CFrmSmart::CFrmSmart()
{
    m_pView = NULL;
}

BEGIN_MESSAGE_MAP(CFrmSmart, super)
ON_WM_CLOSE()
ON_WM_PAINT()
ON_WM_GETMINMAXINFO()
ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFrmSmart message handlers

BOOL CFrmSmart::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style &= ~FWS_ADDTOTITLE;
    cs.style |= WS_CLIPCHILDREN;
    return super::PreCreateWindow(cs);
}

void CFrmSmart::OnClose()
{
    if (IsZoomed()) ShowWindow(SW_RESTORE);
    ShowWindow(SW_HIDE);
    m_bPseudoInactive = true;
    theApp.ActivateSomething();
}

void CFrmSmart::OnUpdateFrameTitle(BOOL bAddToTitle)
{
    super::OnUpdateFrameTitle(bAddToTitle);
    CDocSmart* pDoc = (CDocSmart*)GetActiveDocument();
    if (!pDoc) return;
    ASSERT_KINDOF(CDocSmart, pDoc);

    static LPCTSTR delim = _T(" - ");
    CString old;
    GetWindowText(old);
    CString s = old;
    int i = s.Find(delim);
    if (i >= 0) s = s.Left(i);

    s += delim;
    s += pDoc->GetTitle();
    if (pDoc->IsModified2()) s += " *";
    CString tit2 = pDoc->GetTitle2();
    if (!tit2.IsEmpty())
    {
        s += " (";
        s += tit2;
        s += ")";
    }

    if (s != old) SetWindowText(s);
}

void CFrmSmart::UpdateTitle()
{
    OnUpdateFrameTitle(true);
}

void CFrmSmart::Redraw(bool bHor, bool bVer)
{
    if (bHor)
    {
        InvalidateRect(m_pView->GetHeaderRect(m_rectFrame, m_rectView, CViewSmart::Top, false), false);
        InvalidateRect(m_pView->GetHeaderRect(m_rectFrame, m_rectView, CViewSmart::Bottom, false), false);
    }
    if (bVer)
    {
        InvalidateRect(m_pView->GetHeaderRect(m_rectFrame, m_rectView, CViewSmart::Left, false), false);
        InvalidateRect(m_pView->GetHeaderRect(m_rectFrame, m_rectView, CViewSmart::Right, false), false);
    }
    if (bHor || bVer) UpdateWindow();
}

void CFrmSmart::OnPaint()
{
    CPaintDC dc(this);
    if (!m_pView) return;

    CRgn fillRgn;
    fillRgn.CreateRectRgn(m_rectFrame.left, m_rectFrame.top, m_rectFrame.right, m_rectFrame.bottom);

    for (int i = -1; i < 4; ++i)
    {
        CViewSmart::THeader hi = CViewSmart::THeader(i);
        CRect rect = m_pView->GetHeaderRect(m_rectFrame, m_rectView, CViewSmart::THeader(hi), false);
        if (rect.IsRectEmpty()) continue;

        CSize sz = rect.Size();
        CRgn rgn2;
        rgn2.CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
        fillRgn.CombineRgn(&fillRgn, &rgn2, RGN_DIFF);
        if (i == -1) continue;

        CImageAF img;
        img.CreateScreen(sz);
        img.Fill(colorNormal());
        CDC* pDCImg = img.GetCDC();

        CPoint ptOrigin(0, 0);
        if (hi == CViewSmart::Top)
            ptOrigin.x -= m_pView->GetScrollPosition().x;
        else if (hi == CViewSmart::Left)
            ptOrigin.y -= m_pView->GetScrollPosition().y;
        pDCImg->SetViewportOrg(ptOrigin);

        pDCImg->SetTextColor(colorText());
        pDCImg->SetBkMode(TRANSPARENT);
        m_pView->DrawHeader(hi, pDCImg, rect.Size(), m_pView->m_pDDPaint);
        pDCImg->SetViewportOrg(0, 0);

        img.ReleaseCDC(pDCImg);
        img.BitBlt(dc, rect.TopLeft());
    }

    dc.SelectClipRgn(&fillRgn);
    dc.FillSolidRect(&m_rectFrame, colorNormal());
    dc.Draw3dRect(&m_rectFrame, colorDark(), colorLight());
    dc.SelectClipRgn(NULL);
    m_pView->DrawMisc(&dc, m_pView->m_pDDPaint);
}

void CFrmSmart::RecalcLayout(BOOL bNotify)
{
    if (m_pView) m_pView->ShowWindow(SW_HIDE);
    super::RecalcLayout(bNotify);
    if (!m_pView) return;

    if (GetActiveView()->GetSafeHwnd() != m_pView->GetSafeHwnd()) return;
    GetClientRect(&m_rectFrame);

    if (m_wndToolBar.GetSafeHwnd())
    {
        CRect rect;
        m_wndToolBar.GetWindowRect(rect);
        m_rectFrame.top = rect.Height();
    }

    CDC* pDC = GetDC();
    m_rectView = m_pView->GetViewRect(pDC, m_rectFrame, m_pView->m_pDDPaint);
    ReleaseDC(pDC);
    m_pView->MoveWindow(m_rectView);
    m_pView->ShowWindow(SW_SHOWNORMAL);
}

void CFrmSmart::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
    super::OnGetMinMaxInfo(lpMMI);
    lpMMI->ptMinTrackSize = CPoint(180, 130);
}

void CFrmSmart::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
    super::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
    if (bActivate) m_bPseudoInactive = false;
    theApp.ScheduleMenu();
}
