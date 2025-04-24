#include "StdAfx.h"

#include "AFLibWndAFSplitter.h"

#include "AFLibViewSplitted.h"

using AFLibGui::CViewSplitted;
using AFLibGui::CWndAFSplitter;

IMPLEMENT_DYNAMIC(CWndAFSplitter, super)

CWndAFSplitter::CWndAFSplitter()
{
    m_bDrag = false;
    m_xStart = 0;
    m_rectSaved.SetRectEmpty();
    m_hCursor = LoadCursor(NULL, IDC_SIZEWE);
}

void CWndAFSplitter::StopDragging()
{
    m_bDrag = false;
    ReleaseCapture();
    ClipCursor(NULL);
}

CViewSplitted* CWndAFSplitter::GetParent()
{
    return dynamic_cast<CViewSplitted*>(super::GetParent());
}

BEGIN_MESSAGE_MAP(CWndAFSplitter, super)
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDBLCLK()
ON_WM_CAPTURECHANGED()
ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CWndAFSplitter::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!super::PreCreateWindow(cs)) return false;

    WNDCLASS wndcls;
    GetClassInfo(AfxGetApp()->m_hInstance, cs.lpszClass, &wndcls);
    cs.lpszClass = wndcls.lpszClassName = _T("AFLibSplitter");
    AfxRegisterClass(&wndcls);
    return true;
}

void CWndAFSplitter::OnLButtonDown(UINT nFlags, CPoint point)
{
    super::OnLButtonDown(nFlags, point);

    CViewSplitted* pView = GetParent();
    if (!pView->m_bSplitterOn) return;

    CRect rect;
    pView->GetClientRect(rect);
    pView->ClientToScreen(rect);
    rect.left += pView->m_xLeftInset;
    rect.right -= pView->m_xRightInset;
    if (rect.IsRectEmpty()) return;

    ::ClipCursor(rect);
    GetWindowRect(m_rectSaved);
    ClientToScreen(&point);
    m_xStart = m_rectSaved.left - point.x;

    GetParent()->ScreenToClient(m_rectSaved);
    SetFocus();
    SetCapture();
    m_bDrag = true;
}

void CWndAFSplitter::OnLButtonUp(UINT nFlags, CPoint point)
{
    super::OnLButtonUp(nFlags, point);
    if (!m_bDrag) return;
    StopDragging();
    GetParent()->Split();
}

void CWndAFSplitter::OnMouseMove(UINT nFlags, CPoint point)
{
    super::OnMouseMove(nFlags, point);
    if (!m_bDrag) return;
    ClientToScreen(&point);
    CViewSplitted* pView = GetParent();

    pView->ScreenToClient(&point);
    SetWindowPos(NULL, point.x + m_xStart, pView->m_ySplit, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}

BOOL CWndAFSplitter::PreTranslateMessage(MSG* pMsg)
{
    if (m_bDrag && pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_ESCAPE)
        {
            StopDragging();
            AndrewMoveWindow(this, m_rectSaved);
        }
        return true;
    }
    return super::PreTranslateMessage(pMsg);
}

void CWndAFSplitter::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CViewSplitted* pView = GetParent();
    if (pView->m_bSplitterOn) pView->Fit();
}

void CWndAFSplitter::OnCaptureChanged(CWnd* pWnd)
{
    super::OnCaptureChanged(pWnd);
    if (!m_bDrag) return;
    StopDragging();
    AndrewMoveWindow(this, m_rectSaved);
}

BOOL CWndAFSplitter::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (!GetParent()->m_bSplitterOn) return super::OnSetCursor(pWnd, nHitTest, message);
    SetCursor(m_hCursor);
    return true;
}
